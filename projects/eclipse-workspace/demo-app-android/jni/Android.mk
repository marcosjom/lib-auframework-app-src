#----------------------------------
#- Respaldar ruta actual 
#- (cambia con cada invocacion a mk)
#----------------------------------
NB_LOCAL_PATH := $(call my-dir)

#----------------------------------
#- Compilar dependencias
#----------------------------------
#include $(NB_LOCAL_PATH)/../../../../../../lib-box2d/lib-box2d/projects/android/lib-box2d-android.mk
include $(NB_LOCAL_PATH)/../../../../../../lib-auframework/lib-auframework/projects/android/lib-auframework-android.mk
include $(NB_LOCAL_PATH)/../../../../../../lib-auframework-media/lib-auframework-media/projects/android/lib-auframework-media-android.mk
include $(NB_LOCAL_PATH)/../../../../../../lib-auframework-app/lib-auframework-app/projects/android/lib-auframework-app-nucleo-android.mk
include $(NB_LOCAL_PATH)/../../../android/lib-sereneh-core-android.mk

#----------------------------------
#- Compilar libreria para Android
#----------------------------------
include $(CLEAR_VARS)
LOCAL_PATH              := $(NB_LOCAL_PATH)
LOCAL_MODULE            := audemos
LOCAL_STATIC_LIBRARIES  := audemos-core auframework-app-nucleo auframework-media auframework #box2d       
LOCAL_LDLIBS			:= -llog -lOpenSLES -lGLESv1_CM -landroid #-lOpenSLES -landroid (AssetsManager)

LOCAL_C_INCLUDES		:= \
$(LOCAL_PATH)/../../../../include/lib-sereneh-core \
$(LOCAL_PATH)/../../../../../../../CltNicaraguaBinary/lib-auframework/lib-auframework/include \
$(LOCAL_PATH)/../../../../../../../CltNicaraguaBinary/lib-auframework-app/lib-auframework-app/include/lib-auframework-app-nucleo \
$(LOCAL_PATH)/../../../../../../../CltNicaraguaBinary/lib-auframework-media/lib-auframework-media/include
#$(LOCAL_PATH)/../../../../../../../CltNicaraguaBinary/lib-nixtla-audio/lib-nixtla-audio/include
#$(LOCAL_PATH)/../../../../../../../CltNicaraguaBinary/lib-box2d/lib-box2d/include

LOCAL_SRC_FILES         := com_serenehearts_android_AppNative.cpp
include $(BUILD_SHARED_LIBRARY)
