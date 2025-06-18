//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAppleSecure_H
#define AUAppGlueAppleSecure_H

#include "AUMngrOSSecure.h"
#include "NBMngrOSSecure.h"

class AUAppGlueAppleSecure {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrOSSecureCalls* obj);
		static bool destroy(void* data);
		//
		static bool encWithGKey(void* data, const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData);
		static bool decWithGKey(void* data, const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData);
};

#endif
