LOCAL_PATH 			:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    	:= audemos-core

LOCAL_C_INCLUDES	:= \
$(LOCAL_PATH)/../../../../lib-nixtla-audio/lib-nixtla-audio/include \
$(LOCAL_PATH)/../../../../lib-auframework/lib-auframework/include \
$(LOCAL_PATH)/../../../../lib-auframework-media/lib-auframework-media/include \
$(LOCAL_PATH)/../../include/lib-auframework-app-nucleo \
$(LOCAL_PATH)/../../src/cpp/lib-demos-nucleo

SRC_NUCLEO_PATH	:= ../../src/cpp/lib-demos-nucleo

LOCAL_SRC_FILES 	:= ../../../../../actualizarTodos.cpp \
$(SRC_NUCLEO_PATH)/AUDemosRegistro.cpp \
$(SRC_NUCLEO_PATH)/AUEscenaDemoTextRender.cpp \
$(SRC_NUCLEO_PATH)/AUEscenaDemoTextBox.cpp


#----------------------------------
#- Compilar libreria
#----------------------------------
include $(BUILD_STATIC_LIBRARY)


