//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include <Windows.h>
#include <wincrypt.h>
#include "NBMngrNotifs.h"
#include "AUAppGlueWinSecure.h"
#include "NBMngrOSTools.h"
#include "nb/crypto/NBAes256.h"

#pragma comment(lib, "Crypt32.lib")


typedef struct AUAppGlueWinSecureData_ {
	AUAppI* app;
} AUAppGlueWinSecureData;

//Calls

bool AUAppGlueWinSecure::create(AUAppI* app, STMngrOSSecureCalls* obj){
	AUAppGlueWinSecureData* data = (AUAppGlueWinSecureData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueWinSecureData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueWinSecureData);
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

bool AUAppGlueWinSecure::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueWinSecureData* data = (AUAppGlueWinSecureData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

bool AUAppGlueWinSecure_getGlobalKey(AUAppGlueWinSecureData* data, AUCadenaMutable8* dstGlobalKey){
	bool r = false;
	STNBFilesystem* fs = NBGestorArchivos::getFilesystem();
	const char* keyFilepath = "nbSecure.bin";
	STNBString gkeyEnc;
	NBString_init(&gkeyEnc);
	{
		//Load key
		if (NBFilesystem_readFromFilepathAtRoot(fs, ENNBFilesystemRoot_Lib, keyFilepath, &gkeyEnc)) {
			DATA_BLOB bIn, bOut;
			bIn.cbData = gkeyEnc.length + 1;
			bIn.pbData = (BYTE*)gkeyEnc.str;
			if (!CryptUnprotectData(&bIn, NULL, NULL, NULL, NULL, 0, &bOut)) {
				PRINTF_ERROR("AUAppGlueWinSecure, CryptUnprotectData failed.\n");
			} else {
				dstGlobalKey->vaciar();
				dstGlobalKey->agregar((const char*)bOut.pbData, bOut.cbData - 1);
				LocalFree(bOut.pbData);
				r = true;
			}
		}
		//Create and save key
		if (!r) {
			//Create key
			{
				const char* cs = "abcdefghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
				const SI32 csSz = NBString_strLenBytes(cs);
				SI32 i, total = 32;
				dstGlobalKey->vaciar();
				for (i = 0; i < total; i++) {
					dstGlobalKey->agregar(cs[rand() % csSz]);
				}
				//Save key
				{
					DATA_BLOB bIn, bOut;
					bIn.cbData = dstGlobalKey->tamano() + 1;
					bIn.pbData = (BYTE*)dstGlobalKey->str();
					if (!CryptProtectData(&bIn, NULL, NULL, NULL, NULL, 0, &bOut)) {
						PRINTF_ERROR("AUAppGlueWinSecure, CryptProtectData failed.\n");
					} else {
						if (!NBFilesystem_writeToFilepathAtRoot(fs, ENNBFilesystemRoot_Lib, keyFilepath, bOut.pbData, bOut.cbData)) {
							PRINTF_ERROR("AUAppGlueWinSecure, NBFilesystem_writeToFilepathAtRoot failed.\n");
						} else {
							r = true;
						}
						LocalFree(bOut.pbData);
					}
				}
			}
		}
	}
	NBString_release(&gkeyEnc);
	return r;
}

//

bool AUAppGlueWinSecure::encWithGKey(void* pData, const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueWinSecureData* data = (AUAppGlueWinSecureData*)pData;
		if(plainDataSz > 0){
			AUCadenaMutable8* globalKey = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			if(!AUAppGlueWinSecure_getGlobalKey(data, globalKey)){
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

bool AUAppGlueWinSecure::decWithGKey(void* pData, const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueWinSecureData* data = (AUAppGlueWinSecureData*)pData;
		if(cryptdDataSz > 0){
			AUCadenaMutable8* globalKey = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			if(!AUAppGlueWinSecure_getGlobalKey(data, globalKey)){
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

