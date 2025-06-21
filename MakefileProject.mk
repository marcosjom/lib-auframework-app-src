
#-------------------------
# PROJECT
#-------------------------

$(eval $(call nbCall,nbInitProject))

NB_PROJECT_NAME_VAR       := auframework_app

NB_PROJECT_NAME           := auframework-app

NB_PROJECT_CFLAGS         := 

NB_PROJECT_CXXFLAGS       := -std=c++11

NB_PROJECT_INCLUDES       := \
   include \
   ../../sys-nbframework/sys-nbframework-src/ext-libs/include \
   ../../sys-nbframework/sys-nbframework-src/include \
   ../lib-auframework-src/include \
   ../lib-auframework-media-src/include

#-------------------------
# TARGET
#-------------------------

$(eval $(call nbCall,nbInitTarget))

NB_TARGET_NAME_VAR       := auframework_app

NB_TARGET_NAME           := auframework-app

NB_TARGET_PREFIX         := lib

NB_TARGET_SUFIX          := .a

NB_TARGET_TYPE           := static

#-------------------------
# CODE GRP
#-------------------------

$(eval $(call nbCall,nbInitCodeGrp))

NB_CODE_GRP_NAME_VAR  := core

NB_CODE_GRP_NAME      := core

NB_CODE_GRP_SRCS      := \
   src/cpp/AUApp.cpp \
   src/cpp/AUAppEscena.cpp \
   src/cpp/AUAppEscenasAdmin.cpp \
   src/cpp/AUAppEscenasAdminSimple.cpp \
   src/cpp/AUAppEscenasResumenDebug.cpp \
   src/cpp/AUAppNucleoPrecompilado.cpp \
   src/cpp/AUAppNucleoRegistro.cpp \
   src/cpp/AUAppTransicion.cpp \
   src/cpp/AUAppTransicionConCaptura.cpp \
   src/cpp/AUAppTransicionConColorSolido.cpp \
   src/cpp/AUAppTransicionConCortina.cpp \
   src/cpp/AUAppTransicionConFondo.cpp \
   src/cpp/AUMngrAVCapture.cpp \
   src/cpp/AUMngrBiometrics.cpp \
   src/cpp/AUMngrContacts.cpp \
   src/cpp/AUMngrFbLogin.cpp \
   src/cpp/AUMngrGameKit.cpp \
   src/cpp/AUMngrGoogleLogin.cpp \
   src/cpp/AUMngrNotifs.cpp \
   src/cpp/AUMngrOSSecure.cpp \
   src/cpp/AUMngrOSTelephony.cpp \
   src/cpp/AUMngrOSTools.cpp \
   src/cpp/AUMngrPdfKit.cpp \
   src/cpp/AUMngrStore.cpp \
   src/cpp/NBMngrAVCapture.cpp \
   src/cpp/NBMngrBiometrics.cpp \
   src/cpp/NBMngrContacts.cpp \
   src/cpp/NBMngrFbLogin.cpp \
   src/cpp/NBMngrGameKit.cpp \
   src/cpp/NBMngrGoogleLogin.cpp \
   src/cpp/NBMngrNotifs.cpp \
   src/cpp/NBMngrOSSecure.cpp \
   src/cpp/NBMngrOSTelephony.cpp \
   src/cpp/NBMngrOSTools.cpp \
   src/cpp/NBMngrPdfKit.cpp \
   src/cpp/NBMngrStore.cpp

#Specific OS files
ifneq (,$(findstring Android,$(NB_CFG_HOST)))
  #Android
  NB_CODE_GRP_SRCS += \
      src/cpp/com_auframework_AppNative.cpp \
      src/cpp/AUAppGlueAndroidJNI.cpp \
      src/cpp/AUAppGlueAndroidFiles.cpp \
      src/cpp/AUAppGlueAndroidKeyboard.cpp \
      src/cpp/AUAppGlueAndroidTools.cpp \
      src/cpp/AUAppGlueAndroidSecure.cpp \
      src/cpp/AUAppGlueAndroidAVCapture.cpp \
      src/cpp/AUAppGlueAndroidContacts.cpp \
      src/cpp/AUAppGlueAndroidBiometrics.cpp \
      src/cpp/AUAppGlueAndroidStore.cpp \
      src/cpp/AUAppGlueAndroidTelephony.cpp \
      src/cpp/AUAppGlueAndroidNotifs.cpp \
      src/cpp/AUAppGluePdfium.cpp
else
ifeq ($(OS),Windows_NT)
  #Windows
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    #Linux
  endif
  ifeq ($(UNAME_S),Darwin)
    #OSX
  endif
endif
endif


$(eval $(call nbCall,nbBuildCodeGrpRules))

#-------------------------
# TARGET RULES
#-------------------------

$(eval $(call nbCall,nbBuildTargetRules))
