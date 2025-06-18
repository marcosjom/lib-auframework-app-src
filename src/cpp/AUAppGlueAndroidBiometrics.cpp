//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueAndroidBiometrics.h"
//
#include "NBMngrBiometrics.h"
//
#include "AUAppGlueAndroidJNI.h"

#define NB_ANDROID_BIOMETRICS_KEYS_ALIAS	"NBBiometrics"

typedef struct AUAppGlueAndroidBiometricsData_ {
	AUAppI*					app;
	//
	STNBThreadMutex			mutex;
	BOOL					keystoreInited;
	ENBiometricsAuthStatus	status;
	UI64					timeLastAuth;	//Last success auth
	//Current authentitcation flow
	struct {
		jobject				jMngr;
		jobject				jCipher;
		jobject				jLstnr;
		jobject				jCancelSig;
	} authCur;
} AUAppGlueAndroidBiometricsData;

void AUAppGlueAndroidBiometrics_keystoreInit(AUAppGlueAndroidBiometricsData* data);

//Calls
	
bool AUAppGlueAndroidBiometrics::create(AUAppI* app, STMngrBiometricsCalls* obj){
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidBiometricsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAndroidBiometricsData);
	NBMemory_setZeroSt(*obj, STMngrBiometricsCalls);
	data->app							= (AUAppI*)app;
	NBThreadMutex_init(&data->mutex);
	data->keystoreInited				= FALSE;
	data->status						= ENBiometricsAuthStatus_NotDetermined;
	data->timeLastAuth					= 0;
	NBMemory_setZero(data->authCur);
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
	//Create Key
	//ToDo: remove to allow a quicker app-launch
	//if(!data->keystoreInited){
	//	AUAppGlueAndroidBiometrics_keystoreInit(data);
	//}
	//
	return true;
}

bool AUAppGlueAndroidBiometrics::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)pData;
		NBThreadMutex_lock(&data->mutex);
		//
		{
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			if(data->authCur.jCancelSig != NULL){
				jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
				data->authCur.jCancelSig = NULL;
			}
			if(data->authCur.jLstnr != NULL){
				jEnv->DeleteGlobalRef(data->authCur.jLstnr);
				data->authCur.jLstnr = NULL;
			}
			if(data->authCur.jCipher != NULL){
				jEnv->DeleteGlobalRef(data->authCur.jCipher);
				data->authCur.jCipher = NULL;
			}
			if(data->authCur.jMngr != NULL){
				jEnv->DeleteGlobalRef(data->authCur.jMngr);
				data->authCur.jMngr = NULL;
			}
		}
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

void AUAppGlueAndroidBiometrics::getTypeName(void* pData, const ENBiometricsType type, STNBString* dstName){
	//Generic name
	NBString_set(dstName, "Biometric");
}

BOOL AUAppGlueAndroidBiometrics_canAuthenticateAPI29_(AUAppGlueAndroidBiometricsData* data, JNIEnv* jEnv, jobject jContext, const SI32 curAPI, const ENBiometricsType type, STNBString* dstError, BOOL* dstEnrollIsRequired){
    BOOL r = FALSE, enrollIsRequired = FALSE;
    jclass clsBioMngr = jEnv->FindClass("androidx/biometric/BiometricManager");
    if(clsBioMngr == NULL){
        //Requires API level 29
        if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
    } else {
        jmethodID mFrom        = jEnv->GetStaticMethodID(clsBioMngr, "from", "(Landroid.content.Context;)Landroidx/biometric/BiometricManager;"); NBASSERT(mFrom != NULL) //PRINTF_INFO("mGetCount.\n");
        jmethodID mCanAuth    = jEnv->GetMethodID(clsBioMngr, "canAuthenticate", "(I)I"); NBASSERT(mCanAuth != NULL) //PRINTF_INFO("mGetCount.\n");
        if(mFrom != NULL && mCanAuth != NULL){
            jobject jBioMngr = jEnv->CallStaticObjectMethod(clsBioMngr, mFrom, jContext);
            if(jBioMngr != NULL){
                const jint authTypes = (curAPI >= 28 ? 255 /*BIOMETRIC_WEAK*/ : 0) | (curAPI >= 30 ? 15 /*BIOMETRIC_STRONG*/ : 0);
                const jint canAuth = jEnv->CallIntMethod(jBioMngr, mCanAuth, authTypes);
                switch(canAuth){
                    case 0x00000000: //BIOMETRIC_SUCCESS
                        //No error detected.
                        r = TRUE;
                        break;
                    case 0x00000001: //BIOMETRIC_ERROR_HW_UNAVAILABLE
                        //The hardware is unavailable. Try again later.
                        if(dstError != NULL){
                            NBString_set(dstError, "Biometric features are currently unavailable. Try again later.");
                        }
                        r = FALSE;
                        break;
                    case 0x0000000b: //BIOMETRIC_ERROR_NONE_ENROLLED
                        //The user does not have any biometrics enrolled.
                        if(dstError != NULL){
                            NBString_set(dstError, "User hasn't associated any biometric credentials with their account.");
                        }
                        enrollIsRequired = TRUE;
                        r = FALSE;
                        break;
                    case 0x0000000c: //BIOMETRIC_ERROR_NO_HARDWARE
                        //There is no biometric hardware.
                        if(dstError != NULL){
                            NBString_set(dstError, "No biometric features available on this device.");
                        }
                        r = FALSE;
                        break;
                    default:
                        break;
                }
            }
            NBJNI_DELETE_REF_LOCAL(jEnv, jBioMngr)
        }
    }
    NBJNI_DELETE_REF_LOCAL(jEnv, clsBioMngr)
    if(dstEnrollIsRequired != NULL){
        *dstEnrollIsRequired = enrollIsRequired;
    }
    return r;
}
    
BOOL AUAppGlueAndroidBiometrics::canAuthenticate(void* pData, const ENBiometricsType type, STNBString* dstError){
	BOOL r = FALSE;
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		const SI32 curAPI	= AUAppGlueAndroidJNI::getAPICurrent(jEnv);
        //BIOMETRIC_WEAK = 255, API 28+
        //BIOMETRIC_STRONG = 15, API 30+
		/*if(curAPI >= 29 && (type == ENBiometricsType_Unknown || type == ENBiometricsType_Count)){
            r = AUAppGlueAndroidBiometrics_canAuthenticateAPI29_(data, jEnv, jContext, curAPI, type, dstError, NULL);
		} else*/ if(curAPI >= 23 && (type == ENBiometricsType_Unknown || type == ENBiometricsType_Finger || type == ENBiometricsType_Count)){
			jclass clsFngrMngr = jEnv->FindClass("android/hardware/fingerprint/FingerprintManager");
			if(clsFngrMngr == NULL){
				//Requires API level 23
				if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
			} else {
				jobject jMngr = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "FINGERPRINT_SERVICE");
				if(jMngr != NULL){
					jmethodID mIsHrdw	= jEnv->GetMethodID(clsFngrMngr, "isHardwareDetected", "()Z"); NBASSERT(mIsHrdw != NULL)
					jmethodID mHasEnrol	= jEnv->GetMethodID(clsFngrMngr, "hasEnrolledFingerprints", "()Z"); NBASSERT(mHasEnrol != NULL)
					if(!jEnv->CallBooleanMethod(jMngr, mIsHrdw)){
						if(dstError != NULL){
							NBString_set(dstError, "No biometric features available on this device.");
						}
						PRINTF_INFO("AUAppGlueAndroidBiometrics, No biometric features available on this device.");
					} else if(!jEnv->CallBooleanMethod(jMngr, mHasEnrol)){
						if(dstError != NULL){
							NBString_set(dstError, "User hasn't associated any biometric credentials with their account.");
						}
						PRINTF_INFO("AUAppGlueAndroidBiometrics, User hasn't associated any biometric credentials with their account.");
					} else {
						PRINTF_INFO("AUAppGlueAndroidBiometrics, hardware and enroll present.");
						r = TRUE;
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, jMngr)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsFngrMngr)
		}
	}
	return r;
}

BOOL AUAppGlueAndroidBiometrics::showsOwnGui(void* pData){
	BOOL r = FALSE;
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		const SI32 curAPI	= AUAppGlueAndroidJNI::getAPICurrent(jEnv);
        /*if(curAPI >= 29){
            //using 'BiometricManager'
            r = TRUE;
        } else*/ {
            //using 'FingerprintManager'
            r = FALSE;
        }
	}
	return r;
}

void AUAppGlueAndroidBiometrics_keystoreInit(AUAppGlueAndroidBiometricsData* data){
	if(!data->keystoreInited){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		const SI32 curAPI	= AUAppGlueAndroidJNI::getAPICurrent(jEnv);
		if(curAPI >= 23){
			jclass clsFngrMngr = jEnv->FindClass("android/hardware/fingerprint/FingerprintManager");
			if(clsFngrMngr == NULL){
				//Requires API level 23
				if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
			} else {
				jobject jMngr = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "FINGERPRINT_SERVICE");
				if(jMngr != NULL){
					jmethodID mIsHrdw	= jEnv->GetMethodID(clsFngrMngr, "isHardwareDetected", "()Z"); NBASSERT(mIsHrdw != NULL)
					jmethodID mHasEnrol	= jEnv->GetMethodID(clsFngrMngr, "hasEnrolledFingerprints", "()Z"); NBASSERT(mHasEnrol != NULL)
					if(!jEnv->CallBooleanMethod(jMngr, mIsHrdw)){
						PRINTF_INFO("AUAppGlueAndroidBiometrics, No biometric features available on this device.");
					} else if(!jEnv->CallBooleanMethod(jMngr, mHasEnrol)){
						PRINTF_INFO("AUAppGlueAndroidBiometrics, User hasn't associated any biometric credentials with their account.");
					} else {
						//KeyGenParameterSpec requires API 23
						jclass clsString	= jEnv->FindClass("java/lang/String"); NBASSERT(clsString != NULL)
						jclass clsKeyStore	= jEnv->FindClass("java/security/KeyStore"); NBASSERT(clsKeyStore != NULL)
						jclass clsKeyGen	= jEnv->FindClass("javax/crypto/KeyGenerator"); NBASSERT(clsKeyGen != NULL)
						jclass clsKeySpecs	= jEnv->FindClass("android/security/keystore/KeyGenParameterSpec"); NBASSERT(clsKeySpecs != NULL)
						jclass clsSpecsBldr	= jEnv->FindClass("android/security/keystore/KeyGenParameterSpec$Builder"); NBASSERT(clsSpecsBldr != NULL)
						jmethodID mGetInst	= jEnv->GetStaticMethodID(clsKeyStore, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;"); NBASSERT(mGetInst != NULL)
						jmethodID mGetInst2	= jEnv->GetStaticMethodID(clsKeyGen, "getInstance", "(Ljava/lang/String;Ljava/lang/String;)Ljavax/crypto/KeyGenerator;"); NBASSERT(mGetInst2 != NULL)
						jmethodID mKGenInit	= jEnv->GetMethodID(clsKeyGen, "init", "(Ljava/security/spec/AlgorithmParameterSpec;)V"); NBASSERT(mKGenInit != NULL)
						jmethodID mKGenKey	= jEnv->GetMethodID(clsKeyGen, "generateKey", "()Ljavax/crypto/SecretKey;"); NBASSERT(mKGenKey != NULL)
						jmethodID mLoad		= jEnv->GetMethodID(clsKeyStore, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V"); NBASSERT(mLoad != NULL)
						jmethodID mBldrInit	= jEnv->GetMethodID(clsSpecsBldr, "<init>", "(Ljava/lang/String;I)V"); NBASSERT(mBldrInit != NULL)
						jmethodID mSetBMode	= jEnv->GetMethodID(clsSpecsBldr, "setBlockModes", "([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetBMode != NULL)
						jmethodID mSetAReq	= jEnv->GetMethodID(clsSpecsBldr, "setUserAuthenticationRequired", "(Z)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetAReq != NULL)
						jmethodID mSetEPad	= jEnv->GetMethodID(clsSpecsBldr, "setEncryptionPaddings", "([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetEPad != NULL)
						jmethodID mSpecBld	= jEnv->GetMethodID(clsSpecsBldr, "build", "()Landroid/security/keystore/KeyGenParameterSpec;"); NBASSERT(mSpecBld != NULL)
						jstring jStrEmpty	= jEnv->NewStringUTF("");
						jstring jAes		= jEnv->NewStringUTF("AES"); NBASSERT(jAes != NULL) //android.security.keystore.KeyProperties.KEY_ALGORITHM_AES = "AES"
						jstring jCbc		= jEnv->NewStringUTF("CBC"); NBASSERT(jCbc != NULL) //android.security.keystore.KeyProperties.BLOCK_MODE_CBC = "CBC"
						jobjectArray jCbcs	= (jobjectArray)jEnv->NewObjectArray(1, clsString, jStrEmpty); jEnv->SetObjectArrayElement(jCbcs, 0, jCbc);
						jstring jPKCS7		= jEnv->NewStringUTF("PKCS7Padding"); NBASSERT(jPKCS7 != NULL) //android.security.keystore.KeyProperties.ENCRYPTION_PADDING_PKCS7 = "PKCS7Padding"
						jobjectArray jPKCS7s = (jobjectArray)jEnv->NewObjectArray(1, clsString, jStrEmpty); jEnv->SetObjectArrayElement(jPKCS7s, 0, jPKCS7);
						jstring jType		= jEnv->NewStringUTF("AndroidKeyStore"); NBASSERT(jType != NULL)
						jstring jKeySAlias	= jEnv->NewStringUTF(NB_ANDROID_BIOMETRICS_KEYS_ALIAS); NBASSERT(jKeySAlias != NULL)
						jobject jKeyStore	= jEnv->CallStaticObjectMethod(clsKeyStore, mGetInst, jType); NBASSERT(jKeyStore != NULL)
						if(jKeyStore != NULL){
							jEnv->CallVoidMethod(jKeyStore, mLoad, NULL);
							{
								jobject jKeyGen = jEnv->CallStaticObjectMethod(clsKeyGen, mGetInst2, jAes, jType); NBASSERT(jKeyGen != NULL)
								if(jKeyGen != NULL){
									//PURPOSE_ENCRYPT = 0x00000001; PURPOSE_DECRYPT = 0x00000002;
									jobject jBuilder = jEnv->NewObject(clsSpecsBldr, mBldrInit, jKeySAlias, (jint)(0x00000001 | 0x00000002));
									if(jBuilder != NULL){
										jobject jSpecs = NULL;
										jEnv->CallObjectMethod(jBuilder, mSetBMode, jCbcs);
										jEnv->CallObjectMethod(jBuilder, mSetAReq, (jboolean)TRUE);
										jEnv->CallObjectMethod(jBuilder, mSetEPad, jPKCS7s);
										jSpecs = jEnv->CallObjectMethod(jBuilder, mSpecBld); NBASSERT(jSpecs != NULL)
										if(jSpecs != NULL){
											jobject jCipher = NULL;
											jEnv->CallVoidMethod(jKeyGen, mKGenInit, jSpecs);
											jEnv->CallObjectMethod(jKeyGen, mKGenKey);
											PRINTF_INFO("AUAppGlueAndroidBiometrics, generateKey success.\n");
											data->keystoreInited = TRUE;
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, jSpecs)
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jBuilder)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jKeyGen)
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jKeyStore)
						NBJNI_DELETE_REF_LOCAL(jEnv, jKeySAlias)
						NBJNI_DELETE_REF_LOCAL(jEnv, jType)
						NBJNI_DELETE_REF_LOCAL(jEnv, jPKCS7s)
						NBJNI_DELETE_REF_LOCAL(jEnv, jPKCS7)
						NBJNI_DELETE_REF_LOCAL(jEnv, jCbcs)
						NBJNI_DELETE_REF_LOCAL(jEnv, jCbc)
						NBJNI_DELETE_REF_LOCAL(jEnv, jAes)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrEmpty)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyStore)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyGen)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsKeySpecs)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsSpecsBldr)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsString)
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, jMngr)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsFngrMngr)
		}
	}
}

BOOL AUAppGlueAndroidBiometrics::startAuthentication(void* pData, const char* reasonTitle, const char* cancelTitle){
	BOOL r = FALSE;
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)pData;
	if(data != NULL){
		{
			NBThreadMutex_lock(&data->mutex);
			if(data->status != ENBiometricsAuthStatus_Requesting){
				data->status = ENBiometricsAuthStatus_Requesting;
				//Init key (while locked)
				if(/*curAPI <= 29 &&*/ !data->keystoreInited){
					AUAppGlueAndroidBiometrics_keystoreInit(data);
				}
				NBThreadMutex_unlock(&data->mutex);
				{
					AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
					JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
					jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
					const SI32 curAPI	= AUAppGlueAndroidJNI::getAPICurrent(jEnv);
					/*if(curAPI >= 29){
						jclass clsBioPrmpt = jEnv->FindClass("android/hardware/biometrics/BiometricPrompt");
						if(clsBioPrmpt == NULL){
							//Requires API level 29
							if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
						} else {
							jclass clsBioBldr		= jEnv->FindClass("android/hardware/biometrics/BiometricPrompt$Builder"); NBASSERT(clsBioBldr != NULL)
							jmethodID mBldrInit		= jEnv->GetMethodID(clsBioBldr, "<init>", "(Landroid/content/Context;)V"); NBASSERT(mBldrInit != NULL)
							jmethodID mSetTitle		= jEnv->GetMethodID(clsBioBldr, "setTitle", "(Ljava.lang.CharSequence;)Landroid/hardware/biometrics/BiometricPrompt$Builder;"); NBASSERT(mSetTitle != NULL)
							jmethodID mSetSubtitle	= jEnv->GetMethodID(clsBioBldr, "setSubtitle", "(Ljava.lang.CharSequence;)Landroid/hardware/biometrics/BiometricPrompt$Builder;"); NBASSERT(mSetSubtitle != NULL)
							jmethodID mSetNegText	= jEnv->GetMethodID(clsBioBldr, "setNegativeButtonText", "(Ljava.lang.CharSequence;)Landroid/hardware/biometrics/BiometricPrompt$Builder;"); NBASSERT(mSetNegText != NULL)
							jmethodID mSetConfReq	= jEnv->GetMethodID(clsBioBldr, "setConfirmationRequired", "(Z)Landroid/hardware/biometrics/BiometricPrompt$Builder;"); NBASSERT(mSetConfReq != NULL)
							jmethodID mBuild		= jEnv->GetMethodID(clsBioBldr, "build", "()Landroid/hardware/biometrics/BiometricPrompt;"); NBASSERT(mBuild != NULL)
							jobject jBuilder		= jEnv->NewObject(clsBioBldr, mBldrInit, jContext);
							if(jBuilder != NULL){
								jstring jTitle = jEnv->NewStringUTF(reasonTitle); NBASSERT(reasonTitle != NULL)
								jstring jCancelTitle = jEnv->NewStringUTF(cancelTitle); NBASSERT(cancelTitle != NULL)
								//
								jEnv->CallObjectMethod(jBuilder, mSetTitle, jTitle);
								//jEnv->CallObjectMethod(jBuilder, mSetSubtitle, jTitle);
								jEnv->CallObjectMethod(jBuilder, mSetNegText, jCancelTitle);
								jEnv->CallObjectMethod(jBuilder, mSetConfReq, (jboolean)FALSE);
								{
									jobject jPrompt = jEnv->CallObjectMethod(jBuilder, mBuild);
                                    if(jPrompt != NULL){
                                        //Start enrollment
                                        {
                                            BOOL enrollIsRequired = FALSE;
                                            if(!AUAppGlueAndroidBiometrics_canAuthenticateAPI29_(data, jEnv, jContext, curAPI, ENBiometricsType_Count, NULL, &enrollIsRequired)){
                                                if(enrollIsRequired){
                                                    
                                                }
                                            }
                                        }
                                        NBJNI_DELETE_REF_LOCAL(jEnv, jPrompt)
                                    }
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jTitle)
								NBJNI_DELETE_REF_LOCAL(jEnv, jCancelTitle)
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jBuilder)
							NBJNI_DELETE_REF_LOCAL(jEnv, clsBioBldr)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsBioPrmpt)
                    } else*/ if(curAPI >= 23 && data->keystoreInited){
						jclass clsFngrMngr = jEnv->FindClass("android/hardware/fingerprint/FingerprintManager");
						if(clsFngrMngr == NULL){
							//Requires API level 23
							if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
						} else {
							jobject jMngr = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "FINGERPRINT_SERVICE");
							if(jMngr != NULL){
								jmethodID mIsHrdw	= jEnv->GetMethodID(clsFngrMngr, "isHardwareDetected", "()Z"); NBASSERT(mIsHrdw != NULL)
								jmethodID mHasEnrol	= jEnv->GetMethodID(clsFngrMngr, "hasEnrolledFingerprints", "()Z"); NBASSERT(mHasEnrol != NULL)
								if(!jEnv->CallBooleanMethod(jMngr, mIsHrdw)){
									PRINTF_INFO("AUAppGlueAndroidBiometrics, No biometric features available on this device.");
								} else if(!jEnv->CallBooleanMethod(jMngr, mHasEnrol)){
									PRINTF_INFO("AUAppGlueAndroidBiometrics, User hasn't associated any biometric credentials with their account.");
								} else {
									jclass clsLstnr			= jEnv->FindClass("com/auframework/AppNative$FingerAuthListener"); NBASSERT(clsLstnr != NULL)
									jclass clsFngrCryptObj	= jEnv->FindClass("android/hardware/fingerprint/FingerprintManager$CryptoObject"); NBASSERT(clsFngrCryptObj != NULL)
									jclass clsKeyStore		= jEnv->FindClass("java/security/KeyStore"); NBASSERT(clsKeyStore != NULL)
									jclass clsCipher		= jEnv->FindClass("javax/crypto/Cipher"); NBASSERT(clsCipher != NULL)
									jmethodID mInitLstr		= jEnv->GetMethodID(clsLstnr, "<init>", "(J)V"); NBASSERT(mInitLstr != NULL)
									jmethodID mInit2		= jEnv->GetMethodID(clsFngrCryptObj, "<init>", "(Ljavax/crypto/Cipher;)V"); NBASSERT(mInit2 != NULL)
									jmethodID mGetInst		= jEnv->GetStaticMethodID(clsKeyStore, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;"); NBASSERT(mGetInst != NULL)
									jmethodID mLoad			= jEnv->GetMethodID(clsKeyStore, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V"); NBASSERT(mLoad != NULL)
									jmethodID mGetInst3		= jEnv->GetStaticMethodID(clsCipher, "getInstance", "(Ljava/lang/String;)Ljavax/crypto/Cipher;"); NBASSERT(mGetInst3 != NULL)
									jmethodID mGetKey		= jEnv->GetMethodID(clsKeyStore, "getKey", "(Ljava/lang/String;[C)Ljava/security/Key;"); NBASSERT(mGetKey != NULL)
									jmethodID mInit			= jEnv->GetMethodID(clsCipher, "init", "(ILjava/security/Key;)V"); NBASSERT(mInit != NULL)
									jstring jType			= jEnv->NewStringUTF("AndroidKeyStore"); NBASSERT(jType != NULL)
									jobject jKeyStore		= jEnv->CallStaticObjectMethod(clsKeyStore, mGetInst, jType);
									if(jKeyStore == NULL){
										//Validate exception
										if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
									} else {
										jEnv->CallVoidMethod(jKeyStore, mLoad, NULL);
										{
											jstring jTransf		= jEnv->NewStringUTF("AES/CBC/PKCS7Padding"); NBASSERT(jTransf != NULL)
											jobject jCipher 	= jEnv->CallStaticObjectMethod(clsCipher, mGetInst3, jTransf); NBASSERT(jCipher != NULL)
											if(jCipher != NULL){
												jstring jKeySAlias	= jEnv->NewStringUTF(NB_ANDROID_BIOMETRICS_KEYS_ALIAS); NBASSERT(jKeySAlias != NULL)
												jobject jKey		= jEnv->CallObjectMethod(jKeyStore, mGetKey, jKeySAlias, NULL);
												if(jKey == NULL){
													//Validate exception
													if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); }
												} else {
													//Cipher.ENCRYPT_MODE = 0x00000001
													jEnv->CallVoidMethod(jCipher, mInit, (jint)0x00000001, jKey);
													//Validate exception
													if(jEnv->ExceptionCheck()){
														jEnv->ExceptionDescribe();
														jEnv->ExceptionClear();
														NBJNI_DELETE_REF_LOCAL(jEnv, jKey)
													} else {
														jclass clsCancelSig		= jEnv->FindClass("android/os/CancellationSignal"); NBASSERT(clsCancelSig != NULL)
														jmethodID mCancelInit	= jEnv->GetMethodID(clsCancelSig, "<init>", "()V"); NBASSERT(mCancelInit != NULL)
														jobject jCancelSig		= jEnv->NewObject(clsCancelSig, mCancelInit); NBASSERT(jCancelSig != NULL)
														jobject jCryptObj		= jEnv->NewObject(clsFngrCryptObj, mInit2, jCipher); NBASSERT(jCryptObj != NULL)
														jobject jLstnr 			= jEnv->NewObject(clsLstnr, mInitLstr, (jlong)data); NBASSERT(jLstnr != NULL)
														if(jCancelSig != NULL && jCryptObj != NULL && jLstnr != NULL){
															jmethodID mAuth		= jEnv->GetMethodID(clsFngrMngr, "authenticate", "(Landroid/hardware/fingerprint/FingerprintManager$CryptoObject;Landroid/os/CancellationSignal;ILandroid/hardware/fingerprint/FingerprintManager$AuthenticationCallback;Landroid/os/Handler;)V"); NBASSERT(mAuth != NULL)
															jEnv->CallVoidMethod(jMngr, mAuth, jCryptObj, jCancelSig, 0, jLstnr, NULL);
															//Validate exception
															if(jEnv->ExceptionCheck()){
																jEnv->ExceptionDescribe();
																jEnv->ExceptionClear();
															} else {
																PRINTF_INFO("AUAppGlueAndroidBiometrics, started.\n");
																r = TRUE;
																//Set global refs
																{
																	if(data->authCur.jCancelSig != NULL){
																		jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
																		data->authCur.jCancelSig = NULL;
																	}
																	if(data->authCur.jLstnr != NULL){
																		jEnv->DeleteGlobalRef(data->authCur.jLstnr);
																		data->authCur.jLstnr = NULL;
																	}
																	if(data->authCur.jCipher != NULL){
																		jEnv->DeleteGlobalRef(data->authCur.jCipher);
																		data->authCur.jCipher = NULL;
																	}
																	if(data->authCur.jMngr != NULL){
																		jEnv->DeleteGlobalRef(data->authCur.jMngr);
																		data->authCur.jMngr = NULL;
																	}
																}
																data->authCur.jMngr			= jEnv->NewGlobalRef(jMngr);
																data->authCur.jCipher		= jEnv->NewGlobalRef(jCipher);
																data->authCur.jLstnr		= jEnv->NewGlobalRef(jLstnr);
																data->authCur.jCancelSig	= jEnv->NewGlobalRef(jCancelSig);
															}
														}
														NBJNI_DELETE_REF_LOCAL(jEnv, jLstnr)
														NBJNI_DELETE_REF_LOCAL(jEnv, jCryptObj)
														NBJNI_DELETE_REF_LOCAL(jEnv, jCancelSig)
														NBJNI_DELETE_REF_LOCAL(jEnv, clsCancelSig)
													}
												}
												NBJNI_DELETE_REF_LOCAL(jEnv, jKey)
												NBJNI_DELETE_REF_LOCAL(jEnv, jKeySAlias)
											}
											NBJNI_DELETE_REF_LOCAL(jEnv, jCipher)
											NBJNI_DELETE_REF_LOCAL(jEnv, jTransf)
										}
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jKeyStore)
									NBJNI_DELETE_REF_LOCAL(jEnv, jType)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsCipher)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyStore)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsFngrCryptObj)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsLstnr)
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jMngr)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsFngrMngr)
					}
				}
				NBThreadMutex_lock(&data->mutex);
				if(!r){
					data->status = ENBiometricsAuthStatus_Failed;
				}
			}
			NBThreadMutex_unlock(&data->mutex);
		}
	}
	return r;
}

void AUAppGlueAndroidBiometrics::cancelAuthentication(void* pData){
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)pData;
	if(data != NULL){
		NBThreadMutex_lock(&data->mutex);
		//Cancel
		if(data->authCur.jCancelSig != NULL){
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			//Cancel
			{
				jclass clsCancelSig		= jEnv->FindClass("android/os/CancellationSignal"); NBASSERT(clsCancelSig != NULL)
				if(clsCancelSig != NULL){
					jmethodID mCancel	= jEnv->GetMethodID(clsCancelSig, "cancel", "()V"); NBASSERT(mCancel != NULL)
					jEnv->CallVoidMethod(data->authCur.jCancelSig, mCancel);
					data->status		= ENBiometricsAuthStatus_Failed;
					PRINTF_ERROR("AUAppGlueAndroidBiometrics, user biometrics canceled.\n");
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsCancelSig)
			}
			//Release
			{
				if(data->authCur.jCancelSig != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
					data->authCur.jCancelSig = NULL;
				}
				if(data->authCur.jLstnr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jLstnr);
					data->authCur.jLstnr = NULL;
				}
				if(data->authCur.jCipher != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCipher);
					data->authCur.jCipher = NULL;
				}
				if(data->authCur.jMngr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jMngr);
					data->authCur.jMngr = NULL;
				}
			}
		}
		NBThreadMutex_unlock(&data->mutex);
	}
}
	
ENBiometricsAuthStatus AUAppGlueAndroidBiometrics::authStatus(void* pData, const UI64 secsLastValid){
	ENBiometricsAuthStatus r = ENBiometricsAuthStatus_Failed;
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)pData;
	if(data != NULL){
		NBThreadMutex_lock(&data->mutex);
		r = data->status;
		//Validate duration
		if(r == ENBiometricsAuthStatus_Authenticated){
			if((data->timeLastAuth + secsLastValid) < NBDatetime_getCurUTCTimestamp()){
				r = ENBiometricsAuthStatus_NotDetermined;
			}
		}
		NBThreadMutex_unlock(&data->mutex);
	}
	return r;
}

//Callbacks

void AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationError(JNIEnv *pEnv, jobject pObj, jint errorCode, jobject errString, jlong dataPtr){
	//Called when an unrecoverable error has been encountered and the operation is complete.
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)dataPtr;
	JNIEnv* jEnv = pEnv;
	if(data != NULL){
		NBThreadMutex_lock(&data->mutex);
		{
			data->status = ENBiometricsAuthStatus_Failed;
			PRINTF_ERROR("AUAppGlueAndroidBiometrics, user biometrics failed: FingerAuthListener_onAuthenticationError.\n");
			{
				if(data->authCur.jCancelSig != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
					data->authCur.jCancelSig = NULL;
				}
				if(data->authCur.jLstnr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jLstnr);
					data->authCur.jLstnr = NULL;
				}
				if(data->authCur.jCipher != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCipher);
					data->authCur.jCipher = NULL;
				}
				if(data->authCur.jMngr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jMngr);
					data->authCur.jMngr = NULL;
				}
			}
		}
		NBThreadMutex_unlock(&data->mutex);
	}
}

void AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationFailed(JNIEnv *pEnv, jobject pObj, jlong dataPtr){
	//Called when a fingerprint is valid but not recognized.
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)dataPtr;
	JNIEnv* jEnv = pEnv;
	if(data != NULL){
		NBThreadMutex_lock(&data->mutex);
		{
			data->status = ENBiometricsAuthStatus_Failed;
			PRINTF_ERROR("AUAppGlueAndroidBiometrics, user biometrics failed: FingerAuthListener_onAuthenticationFailed.\n");
			{
				if(data->authCur.jCancelSig != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
					data->authCur.jCancelSig = NULL;
				}
				if(data->authCur.jLstnr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jLstnr);
					data->authCur.jLstnr = NULL;
				}
				if(data->authCur.jCipher != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCipher);
					data->authCur.jCipher = NULL;
				}
				if(data->authCur.jMngr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jMngr);
					data->authCur.jMngr = NULL;
				}
			}
		}
		NBThreadMutex_unlock(&data->mutex);
	}
}

void AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationHelp(JNIEnv *pEnv, jobject pObj, jint helpCode, jobject errString, jlong dataPtr){
	//Called when a recoverable error has been encountered during authentication.
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)dataPtr;
	JNIEnv* jEnv = pEnv;
	if(data != NULL){
		NBThreadMutex_lock(&data->mutex);
		{
			data->status = ENBiometricsAuthStatus_Failed;
			PRINTF_ERROR("AUAppGlueAndroidBiometrics, user biometrics failed: FingerAuthListener_onAuthenticationHelp.\n");
			{
				if(data->authCur.jCancelSig != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
					data->authCur.jCancelSig = NULL;
				}
				if(data->authCur.jLstnr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jLstnr);
					data->authCur.jLstnr = NULL;
				}
				if(data->authCur.jCipher != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCipher);
					data->authCur.jCipher = NULL;
				}
				if(data->authCur.jMngr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jMngr);
					data->authCur.jMngr = NULL;
				}
			}
		}
		NBThreadMutex_unlock(&data->mutex);
	}
}

void AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationSucceeded(JNIEnv *pEnv, jobject pObj, jobject result, jlong dataPtr){
	//Called when a fingerprint is recognized.
	JNIEnv* jEnv = pEnv;
	AUAppGlueAndroidBiometricsData* data = (AUAppGlueAndroidBiometricsData*)dataPtr;
	if(data != NULL){
		NBThreadMutex_lock(&data->mutex);
		{
			data->timeLastAuth	= NBDatetime_getCurUTCTimestamp();
			data->status		= ENBiometricsAuthStatus_Authenticated;
			PRINTF_INFO("AUAppGlueAndroidBiometrics, user biometrics success.\n");
			{
				if(data->authCur.jCancelSig != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCancelSig);
					data->authCur.jCancelSig = NULL;
				}
				if(data->authCur.jLstnr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jLstnr);
					data->authCur.jLstnr = NULL;
				}
				if(data->authCur.jCipher != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jCipher);
					data->authCur.jCipher = NULL;
				}
				if(data->authCur.jMngr != NULL){
					jEnv->DeleteGlobalRef(data->authCur.jMngr);
					data->authCur.jMngr = NULL;
				}
			}
		}
		NBThreadMutex_unlock(&data->mutex);
	}
}
