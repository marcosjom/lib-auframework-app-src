//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSBiometrics.h"
//
#include "NBMngrBiometrics.h"
#include <LocalAuthentication/LocalAuthentication.h>

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

typedef struct AUAppGlueIOSBiometricsData_ {
	AUAppI*		app;
	//
	STNBThreadMutex			mutex;
	ENBiometricsAuthStatus	status;
	UI64					timeLastAuth;	//Last success auth
	LAContext*				context;
} AUAppGlueIOSBiometricsData;

//Calls
	
bool AUAppGlueIOSBiometrics::create(AUAppI* app, STMngrBiometricsCalls* obj){
	AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSBiometricsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueIOSBiometricsData);
	NBMemory_setZeroSt(*obj, STMngrBiometricsCalls);
	data->app							= (AUAppI*)app;
	NBThreadMutex_init(&data->mutex);
	data->status						= ENBiometricsAuthStatus_NotDetermined;
	data->timeLastAuth					= 0;
	data->context						= [[LAContext alloc] init];
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcGetTypeName				= getTypeName;
	obj->funcGetTypeNameParam			= data;
	obj->funcCanAuthenticate			= canAuthenticate;
	obj->funcCanAuthenticateParam		= data;
	obj->funcShowsOwnGui				= showsOwnGui;
	obj->funcShowsOwnGuiParam			= data;
	obj->funcStartAuthentication		= startAuthentication;
	obj->funcStartAuthenticationParam	= data;
	obj->funcCancelAuthentication		= cancelAuthentication;
	obj->funcCancelAuthenticationParam	= data;
	obj->funcAuthStatus					= authStatus;
	obj->funcAuthStatusParam			= data;
	//
	return true;
}

bool AUAppGlueIOSBiometrics::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)pData;
		NBThreadMutex_lock(&data->mutex);
		{
			data->app = NULL;
		}
		NBThreadMutex_unlock(&data->mutex);
		NBThreadMutex_release(&data->mutex);
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

void AUAppGlueIOSBiometrics::getTypeName(void* data, const ENBiometricsType type, STNBString* dstName){
	switch (type) {
		case ENBiometricsType_Unknown:
			if(dstName != NULL) NBString_set(dstName, "Biometric");
			break;
		case ENBiometricsType_Face:
			if(dstName != NULL) NBString_set(dstName, "Face ID");
			break;
		case ENBiometricsType_Finger:
			if(dstName != NULL) NBString_set(dstName, "Touch ID");
			break;
			if(dstName != NULL) NBString_set(dstName, "Biometrics");
		default:
			break;
	}
}

BOOL AUAppGlueIOSBiometrics::canAuthenticate(void* pData, const ENBiometricsType type, STNBString* dstError){
	BOOL r = FALSE;
	AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)pData;
	if(data != NULL){
		@autoreleasepool {
			NSError *error = nil;
			if(![data->context canEvaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics error:&error]){
				PRINTF_ERROR("AUAppGlueIOSBiometrics, canEvaluatePolicy returned 'NO': %s.\n", [[error description] UTF8String]);
				if(dstError != NULL && error != nil){
					NBString_set(dstError, [[error localizedDescription] UTF8String]);
				}
			} else {
				if(type == ENBiometricsType_Count){
					r = TRUE; //any
				} else {
					//"biometryType" is available at iOS 11.0
					if([data->context respondsToSelector:@selector(biometryType)]){
						if([data->context biometryType] == LABiometryTypeFaceID){
							r = (type == ENBiometricsType_Face);
						} else if([data->context biometryType] == LABiometryTypeTouchID){
							r = (type == ENBiometricsType_Finger);
						}
					} else {
						r = (type == ENBiometricsType_Finger);
					}
				}
			}
		}
	}
	return r;
}

BOOL AUAppGlueIOSBiometrics::showsOwnGui(void* pData){
	BOOL r = FALSE;
	AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)pData;
	if(data != NULL){
		r = TRUE;
	}
	return r;
}

BOOL AUAppGlueIOSBiometrics::startAuthentication(void* pData, const char* reasonTitle, const char* cancelTitle){
	BOOL r = FALSE;
	AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)pData;
	if(data != NULL){
		@autoreleasepool {
			NBThreadMutex_lock(&data->mutex);
			if(data->status != ENBiometricsAuthStatus_Requesting){
				data->status = ENBiometricsAuthStatus_Requesting;
				NBThreadMutex_unlock(&data->mutex);
				{
					[data->context setLocalizedCancelTitle:[NSString stringWithUTF8String:cancelTitle]];
					[data->context evaluatePolicy:LAPolicyDeviceOwnerAuthenticationWithBiometrics localizedReason:[NSString stringWithUTF8String:reasonTitle] reply:^(BOOL success, NSError * _Nullable error) {
						NBThreadMutex_lock(&data->mutex);
						//Process result
						if(success){
							PRINTF_INFO("AUAppGlueIOSBiometrics, user biometrics authenticated.\n");
							data->timeLastAuth	= NBDatetime_getCurUTCTimestamp();
							data->status		= ENBiometricsAuthStatus_Authenticated;
						} else {
							PRINTF_ERROR("AUAppGlueIOSBiometrics, user biometrics failed: %s.\n", [[error description] UTF8String]);
							data->status        = ENBiometricsAuthStatus_Failed;
						}
						//Invalidate and create new context
						{
							[data->context invalidate];
							[data->context release];
							data->context = [[LAContext alloc] init];
						}
						NBThreadMutex_unlock(&data->mutex);
					}];
					r = TRUE;
				}
				NBThreadMutex_lock(&data->mutex);
			}
			NBThreadMutex_unlock(&data->mutex);
		}
	}
	return r;
}

void AUAppGlueIOSBiometrics::cancelAuthentication(void* pData){
	AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)pData;
	if(data != NULL){
		//
	}
}

ENBiometricsAuthStatus AUAppGlueIOSBiometrics::authStatus(void* pData, const UI64 secsLastValid){
	ENBiometricsAuthStatus r = ENBiometricsAuthStatus_Failed;
	AUAppGlueIOSBiometricsData* data = (AUAppGlueIOSBiometricsData*)pData;
	if(data != NULL){
		//@autoreleasepool {
			NBThreadMutex_lock(&data->mutex);
			r = data->status;
			//Validate duration
			if(r == ENBiometricsAuthStatus_Authenticated){
				if((data->timeLastAuth + secsLastValid) < NBDatetime_getCurUTCTimestamp()){
					r = ENBiometricsAuthStatus_NotDetermined;
				}
			}
			NBThreadMutex_unlock(&data->mutex);
		//}
	}
	return r;
}


