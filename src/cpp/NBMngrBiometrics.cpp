//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrBiometrics.h"

AUMngrBiometrics* NBMngrBiometrics::_instance	= NULL;

void NBMngrBiometrics::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrBiometrics();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrBiometrics::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrBiometrics::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrBiometrics::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::isGlued")
	bool r = AUMngrBiometrics::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrBiometrics::setGlue(AUAppI* app, PTRfuncBiometricsCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::setGlue")
	bool r = AUMngrBiometrics::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void NBMngrBiometrics::getTypeName(const ENBiometricsType type, STNBString* dstName){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::getTypeName")
	_instance->getTypeName(type, dstName);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

BOOL NBMngrBiometrics::canAuthenticate(const ENBiometricsType type, STNBString* dstError){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::canAuthenticate")
	const BOOL r = _instance->canAuthenticate(type, dstError);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrBiometrics::showsOwnGui(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::showsOwnGui")
	const BOOL r = _instance->showsOwnGui();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrBiometrics::startAuthentication(const char* reasonTitle, const char* cancelTitle){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::startAuthentication")
	const BOOL r = _instance->startAuthentication(reasonTitle, cancelTitle);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrBiometrics::cancelAuthentication(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::cancelAuthentication")
	_instance->cancelAuthentication();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}


ENBiometricsAuthStatus NBMngrBiometrics::authStatus(const UI64 secsLastValid){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrBiometrics::getPhoneCount")
	const ENBiometricsAuthStatus r = _instance->authStatus(secsLastValid);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

