//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrNotifs.h"
#include "AUAppGlueAndroidTelephony.h"
//Android and JNI headers
#include <jni.h>

#ifdef __ANDROID__
//is android
#endif

#define NB_TELEPHONY_PERM_ID		"android.permission.READ_PHONE_STATE"
#define NB_TELEPHONY_PREF_NAME		"NB_TELEPHONY"
#define NB_TELEPHONY_PREF_DATA_NAME	"android.permission.READ_PHONE_STATE.requested"

//

class AUAppGlueAndroidTelephonyListener;

typedef struct AUAppGlueAndroidTelephonyData_ {
	AUAppI* app;
	//
	BOOL requestingAuth;
	AUAppGlueAndroidTelephonyListener* listener;
} AUAppGlueAndroidTelephonyData;

class AUAppGlueAndroidTelephonyListener: public AUAppReqPermResultListener {
public:
	AUAppGlueAndroidTelephonyListener(AUAppGlueAndroidTelephonyData* data){
		_data = data;
	}
	virtual ~AUAppGlueAndroidTelephonyListener(){
		_data = NULL;
	}
	//AUAppReqPermResultListener
	void appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* data /*jintArray*/);
private:
	AUAppGlueAndroidTelephonyData* _data;
};

//Calls

bool AUAppGlueAndroidTelephony::create(AUAppI* app, STMngrOSTelephonyCalls* obj){
	AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidTelephonyData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAndroidTelephonyData);
	NBMemory_setZeroSt(*obj, STMngrOSTelephonyCalls);
	data->app = (AUAppI*)app;
	data->requestingAuth	= FALSE;
	data->listener			= new AUAppGlueAndroidTelephonyListener(data);
	data->app->addReqPermResultListener(data->listener);
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcAuthStatus					= authStatus;
	obj->funcAuthStatusParam			= data;
	//
	obj->funcGetPhoneCount				= getPhoneCount;
	obj->funcGetPhoneCountParam			= data;
	obj->funcGetIMEI					= getIMEI;
	obj->funcGetIMEIParam				= data;
	obj->funcCanMakeCalls				= canMakeCalls;
	obj->funcCanMakeCallsParam			= data;
	obj->funcGetCarrierCountryISO		= getCarrierCountryISO;
	obj->funcGetCarrierCountryISOParam	= data;
	//
	return true;
}

bool AUAppGlueAndroidTelephony::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)pData;
		if(data->listener != NULL){
			data->app->removeReqPermResultListener(data->listener);
			delete data->listener;
			data->listener = NULL;
		}
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

ENTelephonyAuthStatus AUAppGlueAndroidTelephony::authStatus(void* pData, const BOOL requestIfNecesary){
	ENTelephonyAuthStatus r = ENTelephonyAuthStatus_Denied;
	AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)pData;
	if(data->requestingAuth){
		r = ENTelephonyAuthStatus_Requesting;
	} else {
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		//Query permission
		if(AUAppGlueAndroidJNI::isPermissionGranted(jEnv, jContext, NB_TELEPHONY_PERM_ID)){
			r = ENTelephonyAuthStatus_Authorized;
		} else if(AUAppGlueAndroidJNI::getAPICurrent(jEnv) < 23){
			//API 23 and before permissions were granted at install time (and cannot be requested)
			//PRINTF_INFO("Telephony, no permission '%s' and cannot request it (API 22-or-less)...\n", NB_TELEPHONY_PERM_ID);
			r = ENTelephonyAuthStatus_Denied;
		} else {
			//API 23+
			{
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(!AUAppGlueAndroidJNI::loadDataFromSharedPrefs(jEnv, jContext, NB_TELEPHONY_PREF_NAME, NB_TELEPHONY_PREF_DATA_NAME, str)){
					r = ENTelephonyAuthStatus_NotDetermined;
				} else {
					if(str->tamano() > 0){
						r = ENTelephonyAuthStatus_Denied;
					} else {
						r = ENTelephonyAuthStatus_NotDetermined;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
			//Start request
			if(r == ENTelephonyAuthStatus_NotDetermined && requestIfNecesary){
				PRINTF_INFO("Telephony, starting permission request...\n");
				data->requestingAuth = TRUE;
				const char* permId = NB_TELEPHONY_PERM_ID;
				if(!AUAppGlueAndroidJNI::requestPermissions(jEnv, jContext, &permId, 1)){
					PRINTF_ERROR("Telephony, ... could not start permission request.\n");
					data->requestingAuth = FALSE;
					r = ENTelephonyAuthStatus_Denied;
				} else {
					PRINTF_INFO("Telephony, ... started permission request.\n");
				}
			}
		}
	}
	return r;
}

//

SI32 AUAppGlueAndroidTelephony::getPhoneCount(void* pData){	//sims supported by device
	SI32 r = 0;
	if(pData != NULL){
		AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv();
		jobject jContext	= (jobject)jniGlue->jActivity();
		jobject mngr		= (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "TELEPHONY_SERVICE");
		if(mngr != NULL){
			jclass clsMngr	= jEnv->FindClass("android/telephony/TelephonyManager"); NBASSERT(clsMngr != NULL)
			if(clsMngr != NULL){
				jmethodID mGet = jEnv->GetMethodID(clsMngr, "getPhoneCount", "()I"); //API 23 (Android 6.0)
				if(mGet == NULL){
					if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
					r = 1; //Default (prev to API 23)
				} else {
					r = (SI32)jEnv->CallIntMethod(mngr, mGet);
					PRINTF_INFO("getPhoneCount: '%d'.\n", r);
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsMngr)
			}
		}
	}
	return r;
}

BOOL AUAppGlueAndroidTelephony::getIMEI(void* pData, const SI32 slot, STNBString* dst){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv();
		jobject jContext	= (jobject)jniGlue->jActivity();
		jobject mngr		= (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "TELEPHONY_SERVICE");
		if(mngr != NULL){
			jclass clsMngr	= jEnv->FindClass("android/telephony/TelephonyManager"); NBASSERT(clsMngr != NULL)
			if(clsMngr != NULL){
				jstring jStr = NULL;
				//Get str
				{
					jmethodID mGet = jEnv->GetMethodID(clsMngr, "getImei", "(I)Ljava/lang/String;"); //API 26 (Android 8.0)
					if(mGet != NULL){
						jStr = (jstring)jEnv->CallObjectMethod(mngr, mGet, (jint)slot);
						if(jStr == NULL){
							PRINTF_ERROR("API getImei(int) API.26+ returned NULL (permission?).\n");
							if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
						} else {
							PRINTF_INFO("API allowed getImei(int) API.26+.\n");
						}
					} else {
						if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
						//
						jmethodID mGet = jEnv->GetMethodID(clsMngr, "getDeviceId", "(I)Ljava/lang/String;"); //API 23 (Android 6.0)
						if(mGet != NULL){
							jStr = (jstring)jEnv->CallObjectMethod(mngr, mGet, (jint)slot);
							if(jStr == NULL){
								PRINTF_ERROR("API getDeviceId(int) API.23+ returned NULL (permission?).\n");
								if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
							} else {
								PRINTF_INFO("API allowed getDeviceId(int) API.23+.\n");
							}
						} else {
							if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
							jmethodID mGet = jEnv->GetMethodID(clsMngr, "getDeviceId", "()Ljava/lang/String;"); //API 1
							if(mGet != NULL){
								jStr = (jstring)jEnv->CallObjectMethod(mngr, mGet);
								if(jStr == NULL){
									PRINTF_ERROR("API getDeviceId() API.1+ returned NULL (permission?).\n");
									if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
								} else {
									PRINTF_INFO("API allowed getDeviceId() API.1+.\n");
								}
							}
						}
					}
				}
				//Process str
				if(jStr != NULL){
					const char* utfStr = jEnv->GetStringUTFChars(jStr, 0);
					PRINTF_INFO("getImei: '%s'.\n", utfStr);
					if(utfStr[0] != '\0'){
						if(dst != NULL) NBString_set(dst, utfStr);
						r = TRUE;
					}
					jEnv->ReleaseStringUTFChars(jStr, utfStr);
					NBJNI_DELETE_REF_LOCAL(jEnv, jStr)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsMngr)
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidTelephony::canMakeCalls(void* pData){
	bool r = FALSE;
	if(pData != NULL){
		AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv();
		jobject jContext	= (jobject)jniGlue->jActivity();
		jobject mngr		= (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "TELEPHONY_SERVICE");
		if(mngr != NULL){
			jclass clsMngr	= jEnv->FindClass("android/telephony/TelephonyManager"); NBASSERT(clsMngr != NULL)
			if(clsMngr != NULL){
				jmethodID mGet = jEnv->GetMethodID(clsMngr, "getNetworkOperatorName", "()Ljava/lang/String;"); NBASSERT(mGet != NULL)
				if(mGet != NULL){
					jstring jStr = (jstring)jEnv->CallObjectMethod(mngr, mGet);
					if(jStr != NULL){
						const char* utfStr = jEnv->GetStringUTFChars(jStr, 0);
						PRINTF_INFO("getNetworkOperatorName: '%s'.\n", utfStr);
						if(utfStr[0] != '\0'){
							r = TRUE;
						}
						jEnv->ReleaseStringUTFChars(jStr, utfStr);
						NBJNI_DELETE_REF_LOCAL(jEnv, jStr)
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsMngr)
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidTelephony::getCarrierCountryISO(void* pData, STNBString* dst){
	bool r = FALSE;
	if(pData != NULL){
		AUAppGlueAndroidTelephonyData* data = (AUAppGlueAndroidTelephonyData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv();
		jobject jContext	= (jobject)jniGlue->jActivity();
		jobject mngr		= (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "TELEPHONY_SERVICE");
		if(mngr != NULL){
			jclass clsMngr	= jEnv->FindClass("android/telephony/TelephonyManager"); NBASSERT(clsMngr != NULL)
			if(clsMngr != NULL){
				jmethodID mGet = jEnv->GetMethodID(clsMngr, "getNetworkCountryIso", "()Ljava/lang/String;"); NBASSERT(mGet != NULL)
				if(mGet != NULL){
					jstring jStr = (jstring)jEnv->CallObjectMethod(mngr, mGet);
					if(jStr != NULL){
						const char* utfStr = jEnv->GetStringUTFChars(jStr, 0);
						PRINTF_INFO("getNetworkCountryIso: '%s'.\n", utfStr);
						if(utfStr[0] != '\0'){
							if(dst != NULL) NBString_set(dst, utfStr);
							r = TRUE;
						}
						jEnv->ReleaseStringUTFChars(jStr, utfStr);
						NBJNI_DELETE_REF_LOCAL(jEnv, jStr)
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsMngr)
			}
		}
	}
	return r;
}


// Listener methods

void AUAppGlueAndroidTelephonyListener::appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* grantsResults /*jintArray*/){
	PRINTF_INFO("AUAppGlueAndroidTelephonyListener::appReqPermResult.\n");
	AUAppGlueAndroidJNI* jniGlue = _data->app->getGlueJNI();
	JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
	jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
	{
		jobjectArray jPerms	= (jobjectArray)perms;
		jintArray jGrants	= (jintArray)grantsResults;
		jint* iGrants		= jEnv->GetIntArrayElements(jGrants, 0);
		SI32 i; const SI32 count = (SI32)jEnv->GetArrayLength(jPerms); NBASSERT(count == (SI32)jEnv->GetArrayLength(jGrants))
		for(i = 0; i < count; i++){
			jobject jPerm	= jEnv->GetObjectArrayElement(jPerms, i);
			jint granted	= iGrants[i];
			const char* utf8 = jEnv->GetStringUTFChars((jstring)jPerm, 0);
			if(NBString_strIsEqual(utf8, NB_TELEPHONY_PERM_ID)){
				//Save "already asked" value
				AUAppGlueAndroidJNI::saveDataToSharedPrefs(jEnv, jContext, NB_TELEPHONY_PREF_NAME, NB_TELEPHONY_PREF_DATA_NAME, "YES");
				//
				_data->requestingAuth = FALSE;
			}
			PRINTF_INFO("AUAppGlueAndroidTelephony, Perm #%d / %d: '%s' (%s).\n", (i + 1), count, utf8, granted ? "GRANTED" : "DENIED");
			jEnv->ReleaseStringUTFChars((jstring)jPerm, utf8);
		}
	}
}
