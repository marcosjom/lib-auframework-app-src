//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrOSSecure.h"

AUMngrOSSecure* NBMngrOSSecure::_instance	= NULL;

void NBMngrOSSecure::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrOSSecure();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrOSSecure::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrOSSecure::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrOSSecure::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::isGlued")
	bool r = AUMngrOSSecure::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSSecure::setGlue(AUAppI* app, PTRfuncOSSecureCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::setGlue")
	bool r = AUMngrOSSecure::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool NBMngrOSSecure::encWithGKey(const BYTE* plainData, const UI32 plainDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstCryptData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::encWithGKey")
	const bool r = _instance->encWithGKey(plainData, plainDataSz, salt, saltSz, iterations, dstCryptData);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSSecure::decWithGKey(const BYTE* cryptdData, const UI32 cryptdDataSz, const BYTE* salt, const UI32 saltSz, const UI16 iterations, AUCadenaLargaMutable8* dstPlainData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSSecure::decWithGKey")
	const bool r = _instance->decWithGKey(cryptdData, cryptdDataSz, salt, saltSz, iterations, dstPlainData);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}




