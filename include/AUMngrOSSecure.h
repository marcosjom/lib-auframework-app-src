//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrOSSecure_h
#define AUMngrOSSecure_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

typedef struct STMngrOSSecureCalls_ STMngrOSSecureCalls;

//Callbacks
typedef bool (*PTRfuncOSSecureCreate)(AUAppI* app, STMngrOSSecureCalls* obj);
typedef bool (*PTRfuncOSSecureDestroy)(void* obj);
typedef bool (*PTRfuncOSSecureEncWithGKey)(void* obj, const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData);
typedef bool (*PTRfuncOSSecureDecWithGKey)(void* obj, const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData);


//

typedef struct STMngrOSSecureCalls_ {
	PTRfuncOSSecureCreate		funcCreate;
	void*						funcCreateParam;
	PTRfuncOSSecureDestroy		funcDestroy;
	void*						funcDestroyParam;
	//
	PTRfuncOSSecureEncWithGKey	funcEncWithGKey;
	void*						funcEncWithGKeyParam;
	PTRfuncOSSecureDecWithGKey	funcDecWithGKey;
	void*						funcDecWithGKeyParam;
} STMngrOSSecureCalls;

//

class AUMngrOSSecure : public AUObjeto {
	public:
		AUMngrOSSecure();
		virtual ~AUMngrOSSecure();
		//
		static bool	isGlued();
		static bool	setGlue(AUAppI* app, PTRfuncOSSecureCreate initCall);
		//
		bool		encWithGKey(const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData);
		bool		decWithGKey(const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrOSSecureCalls _calls;
};

#endif
