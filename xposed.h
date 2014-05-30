#ifndef XPOSED_H_
#define XPOSED_H_

#include "xposed_shared.h"
#include "xposed_safemode.h"

#define XPOSED_LIB_DALVIK XPOSED_DIR "bin/libxposed_dalvik.so"
#ifdef XPOSED_WITH_ART
    #define XPOSED_LIB_ART XPOSED_DIR "bin/libxposed_art.so"
#endif
#define XPOSED_JAR XPOSED_DIR "bin/XposedBridge.jar"
#define XPOSED_JAR_NEWVERSION XPOSED_DIR "bin/XposedBridge.jar.newversion"
#define XPOSED_LOAD_BLOCKER XPOSED_DIR "conf/disabled"
#define XPOSED_ENABLE_FOR_TOOLS XPOSED_DIR "conf/enable_for_tools"
#define XPOSED_SAFEMODE_NODELAY XPOSED_DIR "conf/safemode_nodelay"
#define XPOSED_SAFEMODE_DISABLE XPOSED_DIR "conf/safemode_disable"

#define XPOSED_CLASS_DOTS_ZYGOTE "de.robv.android.xposed.XposedBridge"
#define XPOSED_CLASS_DOTS_TOOLS  "de.robv.android.xposed.XposedBridge$ToolEntryPoint"

namespace xposed {

bool handleOptions(int argc, char* const argv[]);
bool initialize(bool zygote, const char* className, int argc, char* const argv[]);
void printRomInfo();
bool isDisabled();
void disableXposed();
bool isSafemodeDisabled();
bool shouldSkipSafemodeDelay();
bool shouldIgnoreCommand(const char* className, int argc, const char* const argv[]);
bool addJarToClasspath(bool zygote);
void onVmCreated(JNIEnv* env, const char* className);

}  // namespace xposed

#endif  // XPOSED_H_
