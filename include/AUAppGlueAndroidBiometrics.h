//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAndroidBiometrics_H
#define AUAppGlueAndroidBiometrics_H

#include "AUMngrBiometrics.h"
//Android and JNI headers
#include <jni.h>

class AUAppGlueAndroidBiometrics {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrBiometricsCalls* obj);
		static bool destroy(void* data);
		//
		static void	getTypeName(void* data, const ENBiometricsType type, STNBString* dstName);
		static BOOL	canAuthenticate(void* data, const ENBiometricsType type, STNBString* dstError);
		static BOOL	showsOwnGui(void* data);
		static BOOL	startAuthentication(void* data, const char* reasonTitle, const char* cancelTitle);
		static void	cancelAuthentication(void* data);
		static ENBiometricsAuthStatus authStatus(void* data, const UI64 secsLastValid);
		//Callbacks
		static void FingerAuthListener_onAuthenticationError(JNIEnv *pEnv, jobject pObj, jint errorCode, jobject errString, jlong dataPtr);
		static void FingerAuthListener_onAuthenticationFailed(JNIEnv *pEnv, jobject pObj, jlong dataPtr);
		static void FingerAuthListener_onAuthenticationHelp(JNIEnv *pEnv, jobject pObj, jint helpCode, jobject errString, jlong dataPtr);
		static void FingerAuthListener_onAuthenticationSucceeded(JNIEnv *pEnv, jobject pObj, jobject result, jlong dataPtr);
};

	
#endif
