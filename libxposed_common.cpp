/**
 * This file includes functions shared by different runtimes.
 */

#define LOG_TAG "Xposed"

#include "libxposed_common.h"

#define private public
#if PLATFORM_SDK_VERSION == 15
#include <utils/ResourceTypes.h>
#else
#include <androidfw/ResourceTypes.h>
#endif
#undef private

namespace xposed {

////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////

bool xposedLoadedSuccessfully = false;
xposed::XposedShared* xposed = NULL;
const char* startClassName = NULL;
jclass xposedClass = NULL;
jclass xresourcesClass = NULL;

static jmethodID xresourcesTranslateResId = NULL;
static jmethodID xresourcesTranslateAttrId = NULL;


////////////////////////////////////////////////////////////
// Utility methods
////////////////////////////////////////////////////////////

/** Read an integer value from a configuration file. */
int readIntConfig(const char* fileName, int defaultValue) {
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
        return defaultValue;

    int result;
    int success = fscanf(fp, "%i", &result);
    fclose(fp);

    return (success >= 1) ? result : defaultValue;
}


////////////////////////////////////////////////////////////
// JNI methods
////////////////////////////////////////////////////////////

jobject jni_XposedBridge_getStartClassName(JNIEnv* env, jclass clazz) {
    return env->NewStringUTF(startClassName);
}

jboolean jni_XposedBridge_initNative(JNIEnv* env, jclass clazz) {
    if (!xposedLoadedSuccessfully) {
        ALOGE("Not initializing Xposed because of previous errors");
        return false;
    }

    if (!jni_callback_XposedBridge_initNative(env))
        return false;

    xresourcesClass = env->FindClass(XRESOURCES_CLASS);
    xresourcesClass = reinterpret_cast<jclass>(env->NewGlobalRef(xresourcesClass));
    if (xresourcesClass == NULL) {
        ALOGE("Error while loading XResources class '%s':", XRESOURCES_CLASS);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }
    if (register_natives_XResources(env) != JNI_OK) {
        ALOGE("Could not register natives for '%s'", XRESOURCES_CLASS);
        env->ExceptionClear();
        return false;
    }

    xresourcesTranslateResId = env->GetStaticMethodID(xresourcesClass, "translateResId",
        "(ILandroid/content/res/XResources;Landroid/content/res/Resources;)I");
    if (xresourcesTranslateResId == NULL) {
        ALOGE("ERROR: could not find method %s.translateResId(int, Resources, Resources)", XRESOURCES_CLASS);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    xresourcesTranslateAttrId = env->GetStaticMethodID(xresourcesClass, "translateAttrId",
        "(Ljava/lang/String;Landroid/content/res/XResources;)I");
    if (xresourcesTranslateAttrId == NULL) {
        ALOGE("ERROR: could not find method %s.findAttrId(String, Resources, Resources)", XRESOURCES_CLASS);
        logExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    return true;
}

void jni_XResources_rewriteXmlReferencesNative(JNIEnv* env, jclass clazz,
            jint parserPtr, jobject origRes, jobject repRes) {

    using namespace android;

    ResXMLParser* parser = (ResXMLParser*)parserPtr;
    const ResXMLTree& mTree = parser->mTree;
    uint32_t* mResIds = (uint32_t*)mTree.mResIds;
    ResXMLTree_attrExt* tag;
    int attrCount;
    
    if (parser == NULL)
        return;
    
    do {
        switch (parser->next()) {
            case ResXMLParser::START_TAG:
                tag = (ResXMLTree_attrExt*)parser->mCurExt;
                attrCount = dtohs(tag->attributeCount);
                for (int idx = 0; idx < attrCount; idx++) {
                    ResXMLTree_attribute* attr = (ResXMLTree_attribute*)
                        (((const uint8_t*)tag)
                         + dtohs(tag->attributeStart)
                         + (dtohs(tag->attributeSize)*idx));
                         
                    // find resource IDs for attribute names
                    int32_t attrNameID = parser->getAttributeNameID(idx);
                    // only replace attribute name IDs for app packages
                    if (attrNameID >= 0 && (size_t)attrNameID < mTree.mNumResIds && dtohl(mResIds[attrNameID]) >= 0x7f000000) {
                        size_t attNameLen;
                        const char16_t* attrName = mTree.mStrings.stringAt(attrNameID, &attNameLen);
                        jint attrResID = env->CallStaticIntMethod(xresourcesClass, xresourcesTranslateAttrId,
                            env->NewString((const jchar*)attrName, attNameLen), origRes);
                        if (env->ExceptionCheck())
                            goto leave;
                            
                        mResIds[attrNameID] = htodl(attrResID);
                    }
                    
                    // find original resource IDs for reference values (app packages only)
                    if (attr->typedValue.dataType != Res_value::TYPE_REFERENCE)
                        continue;

                    jint oldValue = dtohl(attr->typedValue.data);
                    if (oldValue < 0x7f000000)
                        continue;
                    
                    jint newValue = env->CallStaticIntMethod(xresourcesClass, xresourcesTranslateResId,
                        oldValue, origRes, repRes);
                    if (env->ExceptionCheck())
                        goto leave;

                    if (newValue != oldValue)
                        attr->typedValue.data = htodl(newValue);
                }
                continue;
            case ResXMLParser::END_DOCUMENT:
            case ResXMLParser::BAD_DOCUMENT:
                goto leave;
            default:
                continue;
        }
    } while (true);
    
    leave:
    parser->restart();
}

}  // namespace xposed
