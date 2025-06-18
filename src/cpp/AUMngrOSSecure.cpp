//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrOSSecure.h"

STMngrOSSecureCalls AUMngrOSSecure::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
};

AUMngrOSSecure::AUMngrOSSecure() : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSSecure::AUMngrOSSecure")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrOSSecure")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		SI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0)
		}
	}
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrOSSecure::~AUMngrOSSecure(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSSecure::~AUMngrOSSecure")
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrOSSecure::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSSecure::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrOSSecure::setGlue(AUAppI* app, PTRfuncOSSecureCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSSecure::setGlue")
	bool r = false;
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	//Init
	if(initCall != NULL){
		if(!(*initCall)(app, &_calls)){
			NBASSERT(false)
		} else {
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool AUMngrOSSecure::encWithGKey(const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSSecure::encWithGKey")
	bool r = false;
	if(_calls.funcEncWithGKey != NULL){
		r = (*_calls.funcEncWithGKey)(_calls.funcEncWithGKeyParam, plainData, plainDataSz, salt, saltSz, iterations, dstCryptData);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSSecure::decWithGKey(const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSSecure::decWithGKey")
	bool r = false;
	if(_calls.funcDecWithGKey != NULL){
		r = (*_calls.funcDecWithGKey)(_calls.funcDecWithGKeyParam, cryptdData, cryptdDataSz, salt, saltSz, iterations, dstPlainData);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrOSSecure)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrOSSecure, "AUMngrOSSecure")
AUOBJMETODOS_CLONAR_NULL(AUMngrOSSecure)
