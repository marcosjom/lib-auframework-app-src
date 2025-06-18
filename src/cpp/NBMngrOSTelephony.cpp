//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrOSTelephony.h"

AUMngrOSTelephony* NBMngrOSTelephony::_instance	= NULL;

void NBMngrOSTelephony::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrOSTelephony();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrOSTelephony::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrOSTelephony::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrOSTelephony::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::isGlued")
	bool r = AUMngrOSTelephony::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTelephony::setGlue(AUAppI* app, PTRfuncOSTelephonyCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::setGlue")
	bool r = AUMngrOSTelephony::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

ENTelephonyAuthStatus NBMngrOSTelephony::authStatus(const BOOL requestIfNecesary){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::getPhoneCount")
	const ENTelephonyAuthStatus r = _instance->authStatus(requestIfNecesary);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

SI32 NBMngrOSTelephony::getPhoneCount(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::getPhoneCount")
	const SI32 r = _instance->getPhoneCount();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrOSTelephony::getIMEI(const SI32 slot, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::getIMEI")
	const BOOL r = _instance->getIMEI(slot, dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTelephony::canMakeCalls(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::canMakeCalls")
	const bool r = _instance->canMakeCalls();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTelephony::getCarrierCountryISO(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTelephony::getCarrierCountryISO")
	const bool r = _instance->getCarrierCountryISO(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}
