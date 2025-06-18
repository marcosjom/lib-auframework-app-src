//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrNotifs.h"
#include "AUAppGlueAndroidSecure.h"
#include "NBMngrOSTools.h"
#include "nb/crypto/NBAes256.h"
//Android and JNI headers
#include <jni.h>

typedef struct AUAppGlueAndroidSecureData_ {
	AUAppI* app;
} AUAppGlueAndroidSecureData;

//Calls

bool AUAppGlueAndroidSecure::create(AUAppI* app, STMngrOSSecureCalls* obj){
	AUAppGlueAndroidSecureData* data = (AUAppGlueAndroidSecureData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidSecureData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAndroidSecureData);
	NBMemory_setZeroSt(*obj, STMngrOSSecureCalls);
	data->app					= (AUAppI*)app;
	//
	obj->funcCreate				= create;
	obj->funcCreateParam		= data;
	obj->funcDestroy			= destroy;
	obj->funcDestroyParam		= data;
	//
	obj->funcEncWithGKey		= encWithGKey;
	obj->funcEncWithGKeyParam	= data;
	obj->funcDecWithGKey		= decWithGKey;
	obj->funcDecWithGKeyParam	= data;
	//
	return true;
}

bool AUAppGlueAndroidSecure::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidSecureData* data = (AUAppGlueAndroidSecureData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

static bool AUAppGlueAndroidSecure_getStoredGlobalKey(JNIEnv* jEnv, const jint sdkInt, const char* keyId, AUCadenaMutable8* dst){
	bool r = false;
	//-------------------------------------------------------------
	//https://developer.android.com/training/articles/keystore.html
	//Usa la KeyChain API cuando desees credenciales en todo el sistema.
	//Usa el Keystore de Android para permitir que una app individual almacene sus propias credenciales, a las cuales solo la app puede acceder.
	//-------------------------------------------------------------
	if(jEnv != NULL && keyId != NULL){
		if(keyId[0] != '\0'){
			if(sdkInt >= 0x00000017){ //API 18+ (4.3+)
				/*
				 JAVA:
				 KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
				 keyStore.load(null);
				 Object key = keyStore.getKey("myAwesomeSecretKey01", null);
				 if(key instanceof KeyStore.PrivateKeyEntry){
				 Certificate cert = ((KeyStore.PrivateKeyEntry)key).getCertificate();
				 bytes data[] = cert.getEncoded();
				 }
				 */
				jclass clsKeyStore		= jEnv->FindClass("java/security/KeyStore"); NBASSERT(clsKeyStore != NULL)
				jclass clsPrivKeyEntry	= jEnv->FindClass("java/security/KeyStore$PrivateKeyEntry"); NBASSERT(clsPrivKeyEntry != NULL)
				jclass clsCertificate	= jEnv->FindClass("java/security/cert/Certificate"); NBASSERT(clsCertificate != NULL)
				if(clsKeyStore != NULL && clsPrivKeyEntry != NULL && clsCertificate != NULL){
					PRINTF_INFO("AUAppGlueAndroidSecure_getStoredGlobalKey API 18+.\n");
					jmethodID mGetInstance	= jEnv->GetStaticMethodID(clsKeyStore, "getInstance", "(Ljava/lang/String;)Ljava/security/KeyStore;"); NBASSERT(mGetInstance != NULL)
					jmethodID mLoad			= jEnv->GetMethodID(clsKeyStore, "load", "(Ljava/security/KeyStore$LoadStoreParameter;)V"); NBASSERT(mLoad != NULL)
					jmethodID mGetEntry		= jEnv->GetMethodID(clsKeyStore, "getEntry", "(Ljava/lang/String;Ljava/security/KeyStore$ProtectionParameter;)Ljava/security/KeyStore$Entry;"); NBASSERT(mGetEntry != NULL)
					jmethodID mGetCertificate = jEnv->GetMethodID(clsPrivKeyEntry, "getCertificate", "()Ljava/security/cert/Certificate;"); NBASSERT(mGetCertificate != NULL)
					jmethodID mGetEncoded	= jEnv->GetMethodID(clsCertificate, "getEncoded", "()[B"); NBASSERT(mGetEncoded != NULL)
					if(mGetInstance != NULL && mLoad != NULL && mGetEntry != NULL && mGetCertificate != NULL && mGetEncoded != NULL){
						jstring jKeyAlias	= jEnv->NewStringUTF(keyId);
						jstring jAKeyStore	= jEnv->NewStringUTF("AndroidKeyStore");
						jobject jKs			= jEnv->CallStaticObjectMethod(clsKeyStore, mGetInstance, jAKeyStore);
						if(jKs != NULL){
							//load
							jEnv->CallVoidMethod(jKs, mLoad, NULL);
							//gEntry
							jobject jKeyEntry	= jEnv->CallObjectMethod(jKs, mGetEntry, jKeyAlias, NULL);
							if(jKeyEntry != NULL){
								NBASSERT(jEnv->IsInstanceOf(jKeyEntry, clsPrivKeyEntry))
								if(jEnv->IsInstanceOf(jKeyEntry, clsPrivKeyEntry)){
									jobject jCert	= jEnv->CallObjectMethod(jKeyEntry, mGetCertificate); NBASSERT(jCert != NULL)
									if(jCert != NULL){
										jbyteArray jArr = (jbyteArray)jEnv->CallObjectMethod(jCert, mGetEncoded); NBASSERT(jArr != NULL)
										if(jArr != NULL){
											jsize jSz = jEnv->GetArrayLength(jArr); NBASSERT(jSz > 0)
											if(jSz > 0){
												if(dst != NULL){
													jbyte* arrBytes = jEnv->GetByteArrayElements(jArr, NULL); NBASSERT(arrBytes != NULL)
													AUBase64::codificaBase64((const char*)arrBytes, (UI32)jSz, dst);
													jEnv->ReleaseByteArrayElements(jArr, arrBytes, JNI_ABORT);
												}
												r = true;
											}
										}
									}
								}
							}
						}
						//
						NBJNI_DELETE_REF_LOCAL(jEnv, jAKeyStore)
						NBJNI_DELETE_REF_LOCAL(jEnv, jKeyAlias)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsCertificate)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsPrivKeyEntry)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyStore)
				}
			} else {
				if(dst != NULL){
					AUBase64::codificaBase64((const char*)keyId, (UI32)AUCadena8::tamano(keyId), dst);
				}
				r = true;
			}
		}
	}
	return r;
}

static bool AUAppGlueAndroidSecure_getGlobalKey(AUAppGlueAndroidSecureData* data, AUCadenaMutable8* dst){
	bool r = false;
	AUCadenaMutable8* appTag = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	if(!NBMngrOSTools::getPkgIdentifier(appTag)){
		PRINTF_ERROR("AndroidSecure, could not get the app's package identifier.\n");
	} else {
		appTag->agregar(".global.key");
		//-------------------------------------------------------------
		//https://developer.android.com/training/articles/keystore.html
		//Usa la KeyChain API cuando desees credenciales en todo el sistema.
		//Usa el Keystore de Android para permitir que una app individual almacene sus propias credenciales, a las cuales solo la app puede acceder.
		//-------------------------------------------------------------
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		if(jniGlue != NULL){
			JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv();
			jobject jActivity	= (jobject)jniGlue->jActivity();
			if(jEnv != NULL && jActivity != NULL){
				jclass clsBuildVer = jEnv->FindClass("android/os/Build$VERSION"); NBASSERT(clsBuildVer != NULL)
				if(clsBuildVer != NULL){
					jfieldID fSdkInt	= jEnv->GetStaticFieldID(clsBuildVer, "SDK_INT", "I"); NBASSERT(fSdkInt != NULL)
					jint sdkInt 		= jEnv->GetStaticIntField(clsBuildVer, fSdkInt); NBASSERT(sdkInt > 0)
					//Retreive private key from Keystore
					if(AUAppGlueAndroidSecure_getStoredGlobalKey(jEnv, sdkInt, appTag->str(), dst)){
						r = true;
					}
					//Generate and store private key at Keystore
					if(!r){
						if(sdkInt >= 23) {	//API 23 (6.0)
							PRINTF_INFO("AUAppGlueAndroidSecure_getGlobalKey API 23+.\n");
							//JAVA: only API 23+ (6.0+)
							/*import java.security.KeyPair;
							import java.security.KeyPairGenerator;
							import java.security.spec.RSAKeyGenParameterSpec;
							import android.security.keystore.KeyProperties;
							import android.security.keystore.KeyGenParameterSpec;
							//
							try {
								KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA"/ *KeyProperties.KEY_ALGORITHM_RSA* /, "AndroidKeyStore");
								KeyGenParameterSpec.Builder builder = new KeyGenParameterSpec.Builder(alias, 0x00000001 / *KeyProperties.PURPOSE_ENCRYPT* / | 0x00000002 / *KeyProperties.PURPOSE_DECRYPT* /);
								builder.setAlgorithmParameterSpec(new RSAKeyGenParameterSpec(1024, F4));
								builder.setBlockModes("CBC" / *KeyProperties.BLOCK_MODE_CBC* /);
								builder.setEncryptionPaddings("PKCS1Padding" / *KeyProperties.ENCRYPTION_PADDING_RSA_PKCS1* /);
								builder.setDigests("SHA-256" / *KeyProperties.DIGEST_SHA256* /, "SHA-384" / *KeyProperties.DIGEST_SHA384* /, "SHA-512" / *KeyProperties.DIGEST_SHA512* /);
								// Only permit the private key to be used if the user authenticated within the last five minutes:
								builder.setUserAuthenticationRequired(requireAuth);
								keyPairGenerator.initialize(builder.build());
								KeyPair keyPair = keyPairGenerator.generateKeyPair();
							} catch (NoSuchProviderException | NoSuchAlgorithmException | InvalidAlgorithmParameterException e) {
								throw new RuntimeException(e);
							}*/
							jclass clsBigInteger	= jEnv->FindClass("java/math/BigInteger"); NBASSERT(clsBigInteger != NULL)
							jclass clsKeyStore		= jEnv->FindClass("java/security/KeyStore"); NBASSERT(clsKeyStore != NULL)
							jclass clsKeyPair		= jEnv->FindClass("java/security/KeyPair"); NBASSERT(clsKeyPair != NULL)
							jclass clsKeyPairGen	= jEnv->FindClass("java/security/KeyPairGenerator"); NBASSERT(clsKeyPairGen != NULL)
							jclass clsRSAKeyGenSpec	= jEnv->FindClass("java/security/spec/RSAKeyGenParameterSpec"); NBASSERT(clsRSAKeyGenSpec != NULL)
							jclass clsKeySpecBldr	= jEnv->FindClass("android/security/keystore/KeyGenParameterSpec$Builder"); NBASSERT(clsKeySpecBldr != NULL)
							if(clsBigInteger != NULL && clsKeyStore != NULL && clsKeyPair != NULL && clsKeyPairGen != NULL && clsRSAKeyGenSpec != NULL && clsKeySpecBldr != NULL){
								jmethodID mGetInstance		= jEnv->GetStaticMethodID(clsKeyPairGen, "getInstance", "(Ljava/lang/String;Ljava/lang/String;)Ljava/security/KeyPairGenerator;"); NBASSERT(mGetInstance != NULL)
								jmethodID mBigIntInit		= jEnv->GetMethodID(clsBigInteger, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mBigIntInit != NULL)
								jmethodID mParamSpecInit	= jEnv->GetMethodID(clsKeySpecBldr, "<init>", "(Ljava/lang/String;I)V"); NBASSERT(mParamSpecInit != NULL)
								jmethodID mRSAKeySpecInit	= jEnv->GetMethodID(clsRSAKeyGenSpec, "<init>", "(ILjava/math/BigInteger;)V"); NBASSERT(mRSAKeySpecInit != NULL)
								jmethodID mSetAlParamSpec	= jEnv->GetMethodID(clsKeySpecBldr, "setAlgorithmParameterSpec", "(Ljava/security/spec/AlgorithmParameterSpec;)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetAlParamSpec != NULL)
								jmethodID mSetBlockModes	= jEnv->GetMethodID(clsKeySpecBldr, "setBlockModes", "([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetBlockModes != NULL)
								jmethodID mSetEncPadding	= jEnv->GetMethodID(clsKeySpecBldr, "setEncryptionPaddings", "([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetEncPadding != NULL)
								jmethodID mSetDiggests		= jEnv->GetMethodID(clsKeySpecBldr, "setDigests", "([Ljava/lang/String;)Landroid/security/keystore/KeyGenParameterSpec$Builder;"); NBASSERT(mSetDiggests != NULL)
								jmethodID mBuild			= jEnv->GetMethodID(clsKeySpecBldr, "build", "()Landroid/security/keystore/KeyGenParameterSpec;"); NBASSERT(mBuild != NULL)
								jmethodID mInitialize		= jEnv->GetMethodID(clsKeyPairGen, "initialize", "(Ljava/security/spec/AlgorithmParameterSpec;)V"); NBASSERT(mInitialize != NULL)
								jmethodID mGeneratePair		= jEnv->GetMethodID(clsKeyPairGen, "generateKeyPair", "()Ljava/security/KeyPair;"); NBASSERT(mGeneratePair != NULL)
								if(mGetInstance != NULL && mBigIntInit != NULL && mParamSpecInit != NULL && mRSAKeySpecInit != NULL && mSetAlParamSpec != NULL && mSetBlockModes != NULL && mSetEncPadding != NULL && mSetDiggests != NULL && mBuild != NULL && mInitialize != NULL && mGeneratePair != NULL){
									jstring jKeyAlias	= jEnv->NewStringUTF(appTag->str());
									jstring j65537		= jEnv->NewStringUTF("65537"); //RSAKeyGenParameterSpec.F4
									jstring jAKeyStore	= jEnv->NewStringUTF("AndroidKeyStore");
									jstring jRSA		= jEnv->NewStringUTF("RSA");
									jstring jCBC		= jEnv->NewStringUTF("CBC");
									jstring jPKCS1Padd	= jEnv->NewStringUTF("PKCS1Padding");
									jstring jSHA256		= jEnv->NewStringUTF("SHA-256");
									jstring jSHA384		= jEnv->NewStringUTF("SHA-384");
									jstring jSHA512		= jEnv->NewStringUTF("SHA-512");
									jobject jKeysGen	= jEnv->CallStaticObjectMethod(clsKeyPairGen, mGetInstance, jRSA, jAKeyStore); NBASSERT(jKeysGen != NULL)
									if(jKeysGen != NULL){
										jobject f4BigInt = jEnv->NewObject(clsBigInteger, mBigIntInit, j65537); NBASSERT(f4BigInt != NULL)
										jobject specBld	= jEnv->NewObject(clsKeySpecBldr, mParamSpecInit, jKeyAlias, (0x00000001 | 0x00000002)); NBASSERT(specBld != NULL)
										jobject rsaSpec	= jEnv->NewObject(clsRSAKeyGenSpec, mRSAKeySpecInit, 1024, f4BigInt); NBASSERT(rsaSpec != NULL)
										jEnv->CallObjectMethod(specBld, mSetAlParamSpec, rsaSpec);
										jEnv->CallObjectMethod(specBld, mSetBlockModes, jEnv->NewObjectArray(1, jEnv->FindClass("java/lang/String"), jCBC));
										jEnv->CallObjectMethod(specBld, mSetEncPadding, jEnv->NewObjectArray(1, jEnv->FindClass("java/lang/String"), jPKCS1Padd));
										{
											jobjectArray arrDigs = (jobjectArray)jEnv->NewObjectArray(3, jEnv->FindClass("java/lang/String"), jSHA256);
											jEnv->SetObjectArrayElement(arrDigs, 0, jSHA256);
											jEnv->SetObjectArrayElement(arrDigs, 1, jSHA384);
											jEnv->SetObjectArrayElement(arrDigs, 2, jSHA512);
											jEnv->CallObjectMethod(specBld, mSetDiggests, arrDigs);
										}
										// Only permit the private key to be used if the user authenticated within the last five minutes:
										// builder.setUserAuthenticationRequired(requireAuth);
										jobject jParamSpec	= jEnv->CallObjectMethod(specBld, mBuild); NBASSERT(jParamSpec != NULL)
										jEnv->CallVoidMethod(jKeysGen, mInitialize, jParamSpec);
										jobject keyPair		= jEnv->CallObjectMethod(jKeysGen, mGeneratePair); NBASSERT(keyPair != NULL)
										//Retreive private key from Keystore
										if(!AUAppGlueAndroidSecure_getStoredGlobalKey(jEnv, sdkInt, appTag->str(), dst)){
											PRINTF_ERROR("AndroidSecure, could not retrieve key after generating it (API 23+, 6.0+).\n");
											NBASSERT(false)
										} else {
											r = true;
										}
									}
									//
									NBJNI_DELETE_REF_LOCAL(jEnv, jSHA512)
									NBJNI_DELETE_REF_LOCAL(jEnv, jSHA384)
									NBJNI_DELETE_REF_LOCAL(jEnv, jSHA256)
									NBJNI_DELETE_REF_LOCAL(jEnv, jPKCS1Padd)
									NBJNI_DELETE_REF_LOCAL(jEnv, jCBC)
									NBJNI_DELETE_REF_LOCAL(jEnv, jRSA)
									NBJNI_DELETE_REF_LOCAL(jEnv, jAKeyStore)
									NBJNI_DELETE_REF_LOCAL(jEnv, j65537)
									NBJNI_DELETE_REF_LOCAL(jEnv, jKeyAlias)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeySpecBldr)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsRSAKeyGenSpec)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyPairGen)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyPair)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyStore)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsBigInteger)
							}
						} else if(sdkInt >= 18){ //API 18 (4.3)
							//JAVA: only API 18+ (4.3+)
							/*import java.math.BigInteger;
							import java.security.KeyPair;
							import java.security.KeyPairGenerator;
							import java.util.Calendar;
							import java.util.GregorianCalendar;
							import javax.security.auth.x500.X500Principal;
							import android.security.KeyPairGeneratorSpec;
							//
							Calendar start = new GregorianCalendar();
							Calendar end = new GregorianCalendar();
							end.add(Calendar.YEAR, 30);
							KeyPairGeneratorSpec.Builder bldr = KeyPairGeneratorSpec.Builder(context);
							bldr.setAlias(alias)
							bldr.setSubject(new X500Principal("CN=" + alias))
							bldr.setSerialNumber(BigInteger.valueOf(Math.abs(alias.hashCode())))
							// Date range of validity for the generated pair.
							bldr.setStartDate(start.getTime());
							bldr.setEndDate(end.getTime());
							KeyPairGeneratorSpec spec = bldr.build();
							KeyPairGenerator kpGenerator = KeyPairGenerator.getInstance("RSA", "AndroidKeyStore");
							kpGenerator.initialize(spec);
							KeyPair keyPair = kpGenerator.generateKeyPair();*/
							PRINTF_INFO("AUAppGlueAndroidSecure_getGlobalKey API 18+.\n");
							jclass clsString		= jEnv->FindClass("java/lang/String"); NBASSERT(clsString != NULL)
							jclass clsBigInteger	= jEnv->FindClass("java/math/BigInteger"); NBASSERT(clsBigInteger != NULL)
							jclass clsKeyPair		= jEnv->FindClass("java/security/KeyPair"); NBASSERT(clsKeyPair != NULL)
							jclass clsKeyPairGen	= jEnv->FindClass("java/security/KeyPairGenerator"); NBASSERT(clsKeyPairGen != NULL)
							jclass clsCalendar		= jEnv->FindClass("java/util/Calendar"); NBASSERT(clsCalendar != NULL)
							jclass clsGregorianCal	= jEnv->FindClass("java/util/GregorianCalendar"); NBASSERT(clsGregorianCal != NULL)
							jclass clsX500			= jEnv->FindClass("javax/security/auth/x500/X500Principal"); NBASSERT(clsX500 != NULL)
							jclass clsKeySpecBldr	= jEnv->FindClass("android/security/KeyPairGeneratorSpec$Builder"); NBASSERT(clsKeySpecBldr != NULL)
							if(clsString != NULL && clsBigInteger != NULL && clsKeyPair != NULL && clsKeyPairGen != NULL  && clsCalendar != NULL  && clsGregorianCal != NULL && clsX500 != NULL && clsKeySpecBldr != NULL){
								jmethodID mGregCalInit	= jEnv->GetMethodID(clsGregorianCal, "<init>", "()V"); NBASSERT(mGregCalInit != NULL)
								jmethodID mGregCalAdd	= jEnv->GetMethodID(clsGregorianCal, "add", "(II)V"); NBASSERT(mGregCalAdd != NULL)
								jmethodID mGetDate		= jEnv->GetMethodID(clsGregorianCal, "getTime", "()Ljava/util/Date;"); NBASSERT(mGetDate != NULL)
								jmethodID mX500Init		= jEnv->GetMethodID(clsX500, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mX500Init != NULL)
								jmethodID mSpecBldrInit	= jEnv->GetMethodID(clsKeySpecBldr, "<init>", "(Landroid/content/Context;)V"); NBASSERT(mSpecBldrInit != NULL)
								jmethodID mSetAlias		= jEnv->GetMethodID(clsKeySpecBldr, "setAlias", "(Ljava/lang/String;)Landroid/security/KeyPairGeneratorSpec$Builder;"); NBASSERT(mSetAlias != NULL)
								jmethodID mSetSubject	= jEnv->GetMethodID(clsKeySpecBldr, "setSubject", "(Ljavax/security/auth/x500/X500Principal;)Landroid/security/KeyPairGeneratorSpec$Builder;"); NBASSERT(mSetSubject != NULL)
								jmethodID mSetSerialNum	= jEnv->GetMethodID(clsKeySpecBldr, "setSerialNumber", "(Ljava/math/BigInteger;)Landroid/security/KeyPairGeneratorSpec$Builder;"); NBASSERT(mSetSerialNum != NULL)
								jmethodID mSetStartDate	= jEnv->GetMethodID(clsKeySpecBldr, "setStartDate", "(Ljava/util/Date;)Landroid/security/KeyPairGeneratorSpec$Builder;"); NBASSERT(mSetStartDate != NULL)
								jmethodID mSetEndDate	= jEnv->GetMethodID(clsKeySpecBldr, "setEndDate", "(Ljava/util/Date;)Landroid/security/KeyPairGeneratorSpec$Builder;"); NBASSERT(mSetEndDate != NULL)
								jmethodID mBuild		= jEnv->GetMethodID(clsKeySpecBldr, "build", "()Landroid/security/KeyPairGeneratorSpec;"); NBASSERT(mBuild != NULL)
								jmethodID mHashCode		= jEnv->GetMethodID(clsString, "hashCode", "()I"); NBASSERT(mHashCode != NULL)
								jmethodID mValueOf		= jEnv->GetStaticMethodID(clsBigInteger, "valueOf", "(J)Ljava/math/BigInteger;"); NBASSERT(mValueOf != NULL)
								jmethodID mGetInstance	= jEnv->GetStaticMethodID(clsKeyPairGen, "getInstance", "(Ljava/lang/String;Ljava/lang/String;)Ljava/security/KeyPairGenerator;"); NBASSERT(mGetInstance != NULL)
								jmethodID mInitialize	= jEnv->GetMethodID(clsKeyPairGen, "initialize", "(Ljava/security/spec/AlgorithmParameterSpec;)V"); NBASSERT(mInitialize != NULL)
								jmethodID mGeneratePair	= jEnv->GetMethodID(clsKeyPairGen, "generateKeyPair", "()Ljava/security/KeyPair;"); NBASSERT(mGeneratePair != NULL)
								if(mGregCalInit != NULL && mGregCalAdd != NULL && mGetDate != NULL && mX500Init != NULL && mSpecBldrInit != NULL && mSetAlias != NULL && mSetSubject != NULL && mSetSerialNum != NULL && mSetStartDate != NULL && mSetEndDate != NULL && mBuild != NULL && mHashCode != NULL && mValueOf != NULL && mGetInstance != NULL && mInitialize != NULL && mGeneratePair != NULL){
									AUCadenaMutable8* strTmp = new AUCadenaMutable8();
									strTmp->agregar("CN="); strTmp->agregar(appTag->str());
									jstring jKeyAlias	= jEnv->NewStringUTF(appTag->str());
									jstring jX500Alias	= jEnv->NewStringUTF(strTmp->str());
									jstring jAKeyStore	= jEnv->NewStringUTF("AndroidKeyStore");
									jstring jRSA		= jEnv->NewStringUTF("RSA");
									jobject jKeysGen	= jEnv->CallStaticObjectMethod(clsKeyPairGen, mGetInstance, jRSA, jAKeyStore); NBASSERT(jKeysGen != NULL)
									if(jKeysGen != NULL){
										jobject jStart	= jEnv->NewObject(clsGregorianCal, mGregCalInit); NBASSERT(jStart != NULL)
										jobject jEnd	= jEnv->NewObject(clsGregorianCal, mGregCalInit); NBASSERT(jEnd != NULL)
										jEnv->CallVoidMethod(jEnd, mGregCalAdd, 0x00000001 /*Calendar.YEAR*/, 30);
										jobject jStart2	= jEnv->CallObjectMethod(jStart, mGetDate); NBASSERT(jStart2 != NULL)
										jobject jEnd2	= jEnv->CallObjectMethod(jEnd, mGetDate); NBASSERT(jEnd2 != NULL)
										jobject jX500	= jEnv->NewObject(clsX500, mX500Init, jX500Alias); NBASSERT(jX500 != NULL)
										jobject jBldr	= jEnv->NewObject(clsKeySpecBldr, mSpecBldrInit, jActivity); NBASSERT(jBldr != NULL)
										jint jHashCode	= jEnv->CallIntMethod(jKeyAlias, mHashCode); if(jHashCode < 0) jHashCode = -jHashCode;
										jobject jHash2	= jEnv->CallStaticObjectMethod(clsBigInteger, mValueOf, (jlong)jHashCode);
										//
										jEnv->CallObjectMethod(jBldr, mSetAlias, jKeyAlias);
										jEnv->CallObjectMethod(jBldr, mSetSubject, jX500);
										jEnv->CallObjectMethod(jBldr, mSetSerialNum, jHash2);
										jEnv->CallObjectMethod(jBldr, mSetStartDate, jStart2);
										jEnv->CallObjectMethod(jBldr, mSetEndDate, jEnd2);
										//
										jobject jParamSpec	= jEnv->CallObjectMethod(jBldr, mBuild);
										jEnv->CallVoidMethod(jKeysGen, mInitialize, jParamSpec);
										jobject keyPair		= jEnv->CallObjectMethod(jKeysGen, mGeneratePair); NBASSERT(keyPair != NULL)
										//Retreive private key from Keystore
										if(!AUAppGlueAndroidSecure_getStoredGlobalKey(jEnv, sdkInt, appTag->str(), dst)){
											PRINTF_ERROR("AndroidSecure, could not retrieve key after generating it (API 18-22, 4.3-5.1).\n");
											NBASSERT(false)
										} else {
											r = true;
										}
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jRSA)
									NBJNI_DELETE_REF_LOCAL(jEnv, jAKeyStore)
									NBJNI_DELETE_REF_LOCAL(jEnv, jX500Alias)
									NBJNI_DELETE_REF_LOCAL(jEnv, jKeyAlias)
									strTmp->liberar(NB_RETENEDOR_THIS);
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeySpecBldr)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsX500)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsGregorianCal)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsCalendar)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyPairGen)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyPair)
								NBJNI_DELETE_REF_LOCAL(jEnv, clsBigInteger)
							}
						} else {
							NBASSERT(false)
						}
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsBuildVer)
				}
			}
		}
	}
	appTag->liberar(NB_RETENEDOR_THIS);
	return r;
}

//

bool AUAppGlueAndroidSecure::encWithGKey(void* pData, const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidSecureData* data = (AUAppGlueAndroidSecureData*)pData;
		if(plainDataSz > 0){
			AUCadenaMutable8* globalKey = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			if(!AUAppGlueAndroidSecure_getGlobalKey(data, globalKey)){
				PRINTF_ERROR("AndroidSecure, encWithGKey failed, could not retrieve the global key (link to OSSecure).\n");
			} else {
				STNBString tmp;
				NBString_initWithSz(&tmp, NBAes256_encryptedSize(plainDataSz), 1024 * 64, 1.1f);
				if(NBAes256_aesEncrypt(plainData, plainDataSz, globalKey->str(), globalKey->tamano(), salt, saltSz, iterations, &tmp)){
					dstCryptData->agregar(tmp.str, tmp.length);
					r = TRUE;
				}
				NBString_release(&tmp);
			}
			globalKey->liberar(NB_RETENEDOR_THIS);
		}
	}
	return r;
}

bool AUAppGlueAndroidSecure::decWithGKey(void* pData, const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidSecureData* data = (AUAppGlueAndroidSecureData*)pData;
		if(cryptdDataSz > 0){
			AUCadenaMutable8* globalKey = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			if(!AUAppGlueAndroidSecure_getGlobalKey(data, globalKey)){
				PRINTF_ERROR("AndroidSecure, encWithGKey failed, could not retrieve the global key (link to OSSecure).\n");
			} else {
				STNBString tmp;
				NBString_initWithSz(&tmp, cryptdDataSz + NBAes256_blockSize(), 1024 * 64, 1.1f);
				if(NBAes256_aesDecrypt(cryptdData, cryptdDataSz, globalKey->str(), globalKey->tamano(), salt, saltSz, iterations, &tmp)){
					dstPlainData->agregar(tmp.str, tmp.length);
					r = TRUE;
				}
				NBString_release(&tmp);
			}
			globalKey->liberar(NB_RETENEDOR_THIS);
		}
	}
	return r;
}

