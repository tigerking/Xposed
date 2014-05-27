/**
 * This file includes functions called directly from app_main.cpp during startup.
 */

#define LOG_TAG "Xposed"

#include "xposed.h"

#include <stdlib.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

extern int RUNNING_PLATFORM_SDK_VERSION;

namespace xposed {

////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////

XposedShared* xposed = new XposedShared;


////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////

/** Initialize Xposed (unless it is disabled). */
bool initialize(bool zygote, const char* className, int argc, char* const argv[]) {
    if (!zygote && access(XPOSED_ENABLE_FOR_TOOLS, F_OK) != 0)
        return false;

    if (zygote && !isSafemodeDisabled() && detectSafemodeTrigger(shouldSkipSafemodeDelay()))
        disableXposed();

    printRomInfo();
    enforceDalvik();

    if (isDisabled() || shouldIgnoreCommand(className, argc, argv))
        return false;

    return (loadRuntimeLibrary() && addJarToClasspath(zygote));
}

/** Print information about the used ROM into the log */
void printRomInfo() {
    char release[PROPERTY_VALUE_MAX];
    char sdk[PROPERTY_VALUE_MAX];
    char manufacturer[PROPERTY_VALUE_MAX];
    char model[PROPERTY_VALUE_MAX];
    char rom[PROPERTY_VALUE_MAX];
    char fingerprint[PROPERTY_VALUE_MAX];
    
    property_get("ro.build.version.release", release, "n/a");
    property_get("ro.build.version.sdk", sdk, "n/a");
    property_get("ro.product.manufacturer", manufacturer, "n/a");
    property_get("ro.product.model", model, "n/a");
    property_get("ro.build.display.id", rom, "n/a");
    property_get("ro.build.fingerprint", fingerprint, "n/a");
    
    ALOGD("Starting Xposed binary version %s, compiled for SDK %d", XPOSED_VERSION, PLATFORM_SDK_VERSION);
    ALOGD("Phone: %s (%s), Android version %s (SDK %s)", model, manufacturer, release, sdk);
    ALOGD("ROM: %s", rom);
    ALOGD("Build fingerprint: %s", fingerprint);
}

/** Make sure that the runtime is Dalvik (and not ART), changing the setting if necessary. */
void enforceDalvik() {
    if (RUNNING_PLATFORM_SDK_VERSION < 19)
        return;

    char runtime[PROPERTY_VALUE_MAX];
    property_get("persist.sys.dalvik.vm.lib", runtime, "");
    if (strcmp(runtime, "libdvm.so") != 0) {
        ALOGE("Unsupported runtime library %s, setting to libdvm.so", runtime);
        property_set("persist.sys.dalvik.vm.lib", "libdvm.so");
    }
}

/** Check whether Xposed is disabled by a flag file */
bool isDisabled() {
    if (access(XPOSED_LOAD_BLOCKER, F_OK) == 0) {
        ALOGE("Found %s, not loading Xposed", XPOSED_LOAD_BLOCKER);
        return true;
    }
    return false;
}

/** Create a flag file to disable Xposed. */
void disableXposed() {
    int fd;
    fd = open(XPOSED_LOAD_BLOCKER, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd >= 0)
        close(fd);
}

/** Check whether safemode is disabled. */
bool isSafemodeDisabled() {
    if (access(XPOSED_SAFEMODE_DISABLE, F_OK) == 0)
        return true;
    else
        return false;
}

/** Check whether the delay for safemode should be skipped. */
bool shouldSkipSafemodeDelay() {
    if (access(XPOSED_SAFEMODE_NODELAY, F_OK) == 0)
        return true;
    else
        return false;
}

/** Ignore the broadcasts by various Superuser implementations to avoid spamming the Xposed log. */
bool shouldIgnoreCommand(const char* className, int argc, const char* const argv[]) {
    if (className == NULL || argc < 4 || strcmp(className, "com.android.commands.am.Am") != 0)
        return false;
        
    if (strcmp(argv[2], "broadcast") != 0 && strcmp(argv[2], "start") != 0)
        return false;
        
    bool mightBeSuperuser = false;
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "com.noshufou.android.su.RESULT") == 0
         || strcmp(argv[i], "eu.chainfire.supersu.NativeAccess") == 0)
            return true;
            
        if (mightBeSuperuser && strcmp(argv[i], "--user") == 0)
            return true;
            
        char* lastComponent = strrchr(argv[i], '.');
        if (!lastComponent)
            continue;
        
        if (strcmp(lastComponent, ".RequestActivity") == 0
         || strcmp(lastComponent, ".NotifyActivity") == 0
         || strcmp(lastComponent, ".SuReceiver") == 0)
            mightBeSuperuser = true;
    }
    
    return false;
}

/** Load the libxposed_*.so library for the currently active runtime. */
bool loadRuntimeLibrary() {
    const char *error;
    void *xposedlib = dlopen(XPOSED_LIB, RTLD_NOW);
    if (!xposedlib) {
        ALOGE("Could not load libxposed: %s", dlerror());
        return false;
    }

    // Clear previous errors
    dlerror();

    bool (*xposedInitLib)(XposedShared* shared) = NULL;
    *(void **) (&xposedInitLib) = dlsym(xposedlib, "xposedInitLib");
    if (!xposedInitLib)  {
        ALOGE("Could not find function xposedInitLib");
        return false;
    }

    return xposedInitLib(xposed);
}

/** Add XposedBridge.jar to the Java classpath. */
bool addJarToClasspath(bool zygote) {
    ALOGI("-----------------");

    // Do we have a new version and are (re)starting zygote? Then load it!
    if (zygote && access(XPOSED_JAR_NEWVERSION, R_OK) == 0) {
        ALOGI("Found new Xposed jar version, activating it");
        if (rename(XPOSED_JAR_NEWVERSION, XPOSED_JAR) != 0) {
            ALOGE("Could not move %s to %s", XPOSED_JAR_NEWVERSION, XPOSED_JAR);
            return false;
        }
    }

    if (access(XPOSED_JAR, R_OK) == 0) {
        char* oldClassPath = getenv("CLASSPATH");
        if (oldClassPath == NULL) {
            setenv("CLASSPATH", XPOSED_JAR, 1);
        } else {
            char classPath[4096];
            int neededLength = snprintf(classPath, sizeof(classPath), "%s:%s", XPOSED_JAR, oldClassPath);
            if (neededLength >= (int)sizeof(classPath)) {
                ALOGE("ERROR: CLASSPATH would exceed %d characters", sizeof(classPath));
                return false;
            }
            setenv("CLASSPATH", classPath, 1);
        }
        ALOGI("Added Xposed (%s) to CLASSPATH", XPOSED_JAR);
        return true;
    } else {
        ALOGE("ERROR: Could not access Xposed jar '%s'", XPOSED_JAR);
        return false;
    }
}

}  // namespace xposed
