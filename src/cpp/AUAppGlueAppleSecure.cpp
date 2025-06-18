//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrNotifs.h"
#include "AUAppGlueAppleSecure.h"
#include "NBMngrOSTools.h"
#include "nb/crypto/NBAes256.h"
//
#include <Security/Security.h>
//

typedef struct AUAppGlueAppleSecureData_ {
	AUAppI* app;
} AUAppGlueAppleSecureData;

//Calls

bool AUAppGlueAppleSecure::create(AUAppI* app, STMngrOSSecureCalls* obj){
	AUAppGlueAppleSecureData* data = (AUAppGlueAppleSecureData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAppleSecureData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAppleSecureData);
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

bool AUAppGlueAppleSecure::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAppleSecureData* data = (AUAppGlueAppleSecureData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

static bool AUAppGlueAppleSecure_getGlobalKey(AUCadenaMutable8* dst){
	bool r = false;
	AUCadenaMutable8* appTag = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	if(!NBMngrOSTools::getPkgIdentifier(appTag)){
		PRINTF_ERROR("Could not get the app's package identifier.\n");
	} else {
		appTag->agregar(".global.key");
		//Retreive private key from keychain
		{
			CFStringRef appTag2	= CFStringCreateWithCString(CFAllocatorGetDefault(), appTag->str(), kCFStringEncodingUTF8);
			CFMutableDictionaryRef attribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionarySetValue(attribs, kSecClass, kSecClassKey);
			CFDictionarySetValue(attribs, kSecAttrKeyType, kSecAttrKeyTypeRSA);
			CFDictionarySetValue(attribs, kSecAttrApplicationTag, appTag2);
			CFDictionarySetValue(attribs, kSecReturnRef, kCFBooleanTrue);
			SecKeyRef seckey = NULL;
			OSStatus res = SecItemCopyMatching(attribs, (CFTypeRef*)&seckey);
			if(res != errSecSuccess){
				PRINTF_ERROR("Could not retrieve key in keychain, SecItemCopyMatching returned error code: %d.\n", (SI32)res);
			} else {
				CFErrorRef err = nil;
				CFDataRef keydata2 = SecKeyCopyExternalRepresentation(seckey, &err);
				if(keydata2 != nil){
					AUCadenaMutable8* keydata = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
					AUBase64::codificaBase64((const char*)CFDataGetBytePtr(keydata2), (UI32)CFDataGetLength(keydata2), keydata);
					/*PRINTF_ERROR("----------------------------------------.\n");
					 PRINTF_ERROR("-- ToDo: remove this mesage.\n");
					 PRINTF_ERROR("-- Secret key retrieved from keychain (%d bytes):\n", keydata->tamano());
					 PRINTF_ERROR("-- %s\n", keydata->str());
					 PRINTF_ERROR("-- ToDo: remove this mesage.\n");
					 PRINTF_ERROR("----------------------------------------.\n");*/
					if(dst != NULL) dst->establecer(keydata->str());
					keydata->liberar(NB_RETENEDOR_NULL);
					r = true;
				}
			}
			CFRelease(attribs);
			CFRelease(appTag2);
		}
		//Generate and store private key at keychain
		if(!r){
			const SI32 keySz = 2048;
			CFNumberRef keySz2 = CFNumberCreate(CFAllocatorGetDefault(), kCFNumberSInt32Type, &keySz);
			CFStringRef appTag2	= CFStringCreateWithCString(CFAllocatorGetDefault(), appTag->str(), kCFStringEncodingUTF8);
			CFMutableDictionaryRef privKeyAttribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionarySetValue(privKeyAttribs, kSecAttrIsPermanent, kCFBooleanTrue);	//When 'true' the key will be automatically added to the OS's keychain.
			CFDictionarySetValue(privKeyAttribs, kSecAttrApplicationTag, appTag2);
			{
				CFMutableDictionaryRef attribs = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				CFDictionarySetValue(attribs, kSecAttrKeyType, kSecAttrKeyTypeRSA);
				CFDictionarySetValue(attribs, kSecAttrKeySizeInBits, keySz2);
				CFDictionarySetValue(attribs, kSecPrivateKeyAttrs, privKeyAttribs);
				CFErrorRef err = nil;
				SecKeyRef seckey = SecKeyCreateRandomKey(attribs, &err);
				if(seckey == nil){
					PRINTF_ERROR("Could not generate secret key, SecKeyCreateRandomKey returned nil.\n");
				} else {
					CFErrorRef err = nil;
					CFDataRef keydata2 = SecKeyCopyExternalRepresentation(seckey, &err);
					if(keydata2 != nil){
						AUCadenaMutable8* keydata = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
						AUBase64::codificaBase64((const char*)CFDataGetBytePtr(keydata2), (UI32)CFDataGetLength(keydata2), keydata);
						/*PRINTF_ERROR("----------------------------------------.\n");
						 PRINTF_ERROR("-- ToDo: remove this mesage.\n");
						 PRINTF_ERROR("-- Secret key generated from keychain (%d bytes):\n", keydata->tamano());
						 PRINTF_ERROR("-- %s\n", keydata->str());
						 PRINTF_ERROR("-- ToDo: remove this mesage.\n");
						 PRINTF_ERROR("----------------------------------------.\n");*/
						if(dst != NULL) dst->establecer(keydata->str());
						keydata->liberar(NB_RETENEDOR_NULL);
						r = true;
					}
				}
				CFRelease(attribs);
			}
			CFRelease(appTag2);
			CFRelease(keySz2);
			CFRelease(privKeyAttribs);
		}
	}
	appTag->liberar(NB_RETENEDOR_NULL);
	return r;
}

//

bool AUAppGlueAppleSecure::encWithGKey(void* data, const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData){
	bool r = false;
	if(plainDataSz > 0){
		AUCadenaMutable8* globalKey = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		if(!AUAppGlueAppleSecure_getGlobalKey(globalKey)){
			PRINTF_ERROR("encWithGKey failed, could not retrieve the global key (link to OSSecure).\n");
		} else {
			STNBString tmp;
			NBString_initWithSz(&tmp, NBAes256_encryptedSize(plainDataSz), 1024 * 64, 1.1f);
			if(NBAes256_aesEncrypt(plainData, plainDataSz, globalKey->str(), globalKey->tamano(), salt, saltSz, iterations, &tmp)){
				dstCryptData->agregar(tmp.str, tmp.length);
				r = TRUE;
			}
			NBString_release(&tmp);
		}
		globalKey->liberar(NB_RETENEDOR_NULL);
	}
	return r;
}

bool AUAppGlueAppleSecure::decWithGKey(void* data, const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData){
	BOOL r = false;
	if(cryptdDataSz > 0){
		AUCadenaMutable8* globalKey = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		if(!AUAppGlueAppleSecure_getGlobalKey(globalKey)){
			PRINTF_ERROR("encWithGKey failed, could not retrieve the global key (link to OSSecure).\n");
		} else {
			STNBString tmp;
			NBString_initWithSz(&tmp, cryptdDataSz + NBAes256_blockSize(), 1024 * 64, 1.1f);
			if(NBAes256_aesDecrypt(cryptdData, cryptdDataSz, globalKey->str(), globalKey->tamano(), salt, saltSz, iterations, &tmp)){
				dstPlainData->agregar(tmp.str, tmp.length);
				r = TRUE;
			}
			NBString_release(&tmp);
		}
		globalKey->liberar(NB_RETENEDOR_NULL);
	}
	return r;
}

