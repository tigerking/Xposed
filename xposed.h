#ifndef XPOSED_H_
#define XPOSED_H_

#include "xposed_shared.h"
#include "xposed_safemode.h"

#define XPOSED_LIB XPOSED_DIR "bin/libxposed_dalvik.so"
#define XPOSED_JAR XPOSED_DIR "bin/XposedBridge.jar"
#define XPOSED_JAR_NEWVERSION XPOSED_DIR "bin/XposedBridge.jar.newversion"
#define XPOSED_LOAD_BLOCKER XPOSED_DIR "conf/disabled"
#define XPOSED_ENABLE_FOR_TOOLS XPOSED_DIR "conf/enable_for_tools"
#define XPOSED_SAFEMODE_NODELAY XPOSED_DIR "conf/safemode_nodelay"
#define XPOSED_SAFEMODE_DISABLE XPOSED_DIR "conf/safemode_disable"

#define XPOSED_CLASS_DOTS "de.robv.android.xposed.XposedBridge"

namespace xposed {

extern XposedShared* xposed;

bool initialize(bool zygote, const char* className, int argc, char* const argv[]);
void printRomInfo();
void enforceDalvik();
bool isDisabled();
void disableXposed();
bool isSafemodeDisabled();
bool shouldSkipSafemodeDelay();
bool shouldIgnoreCommand(const char* className, int argc, const char* const argv[]);
bool loadRuntimeLibrary();
bool addJarToClasspath(bool zygote);

}  // namespace xposed

#endif  // XPOSED_H_
