//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrOSSecure_h
#define NBMngrOSSecure_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrOSSecure.h"

class NBMngrOSSecure {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool	isGlued();
		static bool setGlue(AUAppI* app, PTRfuncOSSecureCreate initCall);
		//
		static bool	encWithGKey(const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData);
		static bool	decWithGKey(const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData);
	private:
		static AUMngrOSSecure* _instance;
};

#endif
