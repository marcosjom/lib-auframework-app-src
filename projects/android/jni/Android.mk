
LOCAL_PATH             := $(call my-dir)

#Configure
NB_CFG_PRINT_INTERNALS := 1
NB_CFG_PRINT_INFO      := 0
NB_CFG_HOST            := AndroidNdkBuild

#Import functions
include $(LOCAL_PATH)/../../../../../sys-nbframework/sys-nbframework-src/MakefileFuncs.mk

#Init workspace
$(eval $(call nbCall,nbInitWorkspace))

#Import projects
include $(LOCAL_PATH)/../../../MakefileProject.mk

#ToDo: remove
#include $(LOCAL_PATH)/../lib-auframework-app-android.mk
