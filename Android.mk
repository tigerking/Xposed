##########################################################
# Customized app_process executable
##########################################################

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    app_main.cpp \
    xposed.cpp \
    xposed_safemode.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libbinder \
    libandroid_runtime \
    libdl

LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

ifeq ($(XPOSED_WITH_ART),true)
    LOCAL_CFLAGS += -DXPOSED_WITH_ART
endif

LOCAL_MODULE := xposed
LOCAL_MODULE_STEM := app_process_xposed_sdk$(PLATFORM_SDK_VERSION)
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

##########################################################
# Library for Dalvik-specific functions
##########################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    libxposed_common.cpp \
    libxposed_dalvik.cpp

LOCAL_C_INCLUDES += \
    dalvik \
    dalvik/vm \
    external/stlport/stlport \
    bionic \
    bionic/libstdc++/include

LOCAL_SHARED_LIBRARIES := \
    libdvm \
    liblog \
    libdl

ifeq ($(PLATFORM_SDK_VERSION),15)
    LOCAL_SHARED_LIBRARIES += libutils
else
    LOCAL_SHARED_LIBRARIES += libandroidfw
endif

LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_MODULE := libxposed_dalvik
# LOCAL_MODULE_STEM := libxposed_dalvik_sdk$(PLATFORM_SDK_VERSION)
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

##########################################################
# Library for ART-specific functions
##########################################################

ifeq ($(XPOSED_WITH_ART),true)
    include frameworks/base/cmds/xposed/ART.mk
endif
