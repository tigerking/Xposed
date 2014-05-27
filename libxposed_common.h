#ifndef LIBXPOSED_COMMON_H_
#define LIBXPOSED_COMMON_H_

#include "xposed_shared.h"

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

namespace xposed {

#define XPOSED_CLASS "de/robv/android/xposed/XposedBridge"
#define XRESOURCES_CLASS "android/content/res/XResources"
#define MIUI_RESOURCES_CLASS "android/content/res/MiuiResources"
#define XTYPEDARRAY_CLASS "android/content/res/XResources$XTypedArray"

/////////////////////////////////////////////////////////////////
// Provided by common part, used by runtime-specific implementation
/////////////////////////////////////////////////////////////////
extern bool xposedLoadedSuccessfully;
extern xposed::XposedShared* xposed;
extern jclass xposedClass;
extern jclass xresourcesClass;
extern const char* startClassName;

extern int readIntConfig(const char* fileName, int defaultValue);

extern jboolean jni_XposedBridge_initNative(JNIEnv* env, jclass clazz);
extern void jni_XResources_rewriteXmlReferencesNative(JNIEnv* env, jclass clazz, jint parserPtr, jobject origRes, jobject repRes);
extern jobject jni_XposedBridge_getStartClassName(JNIEnv* env, jclass clazz);

/////////////////////////////////////////////////////////////////
// To be provided by runtime-specific implementation
/////////////////////////////////////////////////////////////////
extern "C" bool xposedInitLib(xposed::XposedShared* shared);
extern void onVmCreated(JNIEnv* env, const char* className);
extern void logExceptionStackTrace();

extern jboolean jni_callback_XposedBridge_initNative(JNIEnv* env);

extern int register_natives_XposedBridge(JNIEnv* env);
extern int register_natives_XResources(JNIEnv* env);

}  // namespace xposed

#endif  // LIBXPOSED_COMMON_H_
