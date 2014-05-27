/**
 * These declarations are needed for both app_process and the libraries.
 */

#ifndef XPOSED_SHARED_H_
#define XPOSED_SHARED_H_

#include "cutils/log.h"
#ifndef ALOGD
#define ALOGD LOGD
#define ALOGE LOGE
#define ALOGI LOGI
#define ALOGV LOGV
#endif

#include "jni.h"

#define XPOSED_VERSION "58"
#define XPOSED_DIR "/data/data/de.robv.android.xposed.installer/"

namespace xposed {

struct XposedShared {
    // Provided by runtime-specific library, used by executable
    void     (*onVmCreated)(JNIEnv* env, const char* className);
};

}

#endif // XPOSED_SHARED_H_
