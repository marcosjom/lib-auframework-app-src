LOCAL_PATH 		:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    	:= auframework-app

#---------------------
# Optimization options
#---------------------
#-fvisibility=hidden:   removes not-public functions (non-JNIEXPORT) from the symbols table.
#-ffunction-sections:   place each FUCNTION item into its own section in the output file (needed for "--gc-sections").
#-fdata-sections:       place each DATA item into its own section in the output file (needed for "--gc-sections").
LOCAL_CPPFLAGS  += -ffunction-sections -fdata-sections #-fvisibility=hidden
LOCAL_CFLAGS    += -ffunction-sections -fdata-sections #-fvisibility=hidden

LOCAL_C_INCLUDES	:= \
$(LOCAL_PATH)/../../include \
$(LOCAL_PATH)/../../../../sys-nbframework/lib-nbframework-src/include \
$(LOCAL_PATH)/../../../../sys-nbframework/lib-nbframework-src/src/ext/quirc \
$(LOCAL_PATH)/../../../lib-box2d-src/include \
$(LOCAL_PATH)/../../../lib-auframework-src/include \
$(LOCAL_PATH)/../../../lib-auframework-media-src/include

SRC_AUF_APP_PATH := ../../src/cpp

LOCAL_SRC_FILES 	:= ../../../../../actualizarTodos.cpp \
$(SRC_AUF_APP_PATH)/AUAppNucleoPrecompilado.cpp \
$(SRC_AUF_APP_PATH)/AUAppNucleoRegistro.cpp \
$(SRC_AUF_APP_PATH)/AUApp.cpp \
$(SRC_AUF_APP_PATH)/AUAppEscena.cpp \
$(SRC_AUF_APP_PATH)/AUAppEscenasAdmin.cpp \
$(SRC_AUF_APP_PATH)/AUAppEscenasAdminSimple.cpp \
$(SRC_AUF_APP_PATH)/AUAppEscenasResumenDebug.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidJNI.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidFiles.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidKeyboard.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidTools.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidSecure.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidAVCapture.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidStore.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidTelephony.cpp \
$(SRC_AUF_APP_PATH)/AUAppGlueAndroidNotifs.cpp \
$(SRC_AUF_APP_PATH)/AUAppTransicion.cpp \
$(SRC_AUF_APP_PATH)/AUAppTransicionConCaptura.cpp \
$(SRC_AUF_APP_PATH)/AUAppTransicionConColorSolido.cpp \
$(SRC_AUF_APP_PATH)/AUAppTransicionConCortina.cpp \
$(SRC_AUF_APP_PATH)/AUAppTransicionConFondo.cpp \
$(SRC_AUF_APP_PATH)/AUMngrAVCapture.cpp \
$(SRC_AUF_APP_PATH)/AUMngrFbLogin.cpp \
$(SRC_AUF_APP_PATH)/AUMngrGoogleLogin.cpp \
$(SRC_AUF_APP_PATH)/AUMngrNotifs.cpp \
$(SRC_AUF_APP_PATH)/AUMngrOSSecure.cpp \
$(SRC_AUF_APP_PATH)/AUMngrOSTools.cpp \
$(SRC_AUF_APP_PATH)/AUMngrStore.cpp \
$(SRC_AUF_APP_PATH)/AUMngrOSTelephony.cpp \
$(SRC_AUF_APP_PATH)/NBMngrAVCapture.cpp \
$(SRC_AUF_APP_PATH)/NBMngrFbLogin.cpp \
$(SRC_AUF_APP_PATH)/NBMngrGoogleLogin.cpp \
$(SRC_AUF_APP_PATH)/NBMngrNotifs.cpp \
$(SRC_AUF_APP_PATH)/NBMngrOSSecure.cpp \
$(SRC_AUF_APP_PATH)/NBMngrOSTools.cpp \
$(SRC_AUF_APP_PATH)/NBMngrStore.cpp \
$(SRC_AUF_APP_PATH)/NBMngrOSTelephony.cpp \
$(SRC_AUF_APP_PATH)/com_auframework_AppNative.cpp

#----------------------------------
#- Compilar libreria
#----------------------------------
include $(BUILD_STATIC_LIBRARY)


