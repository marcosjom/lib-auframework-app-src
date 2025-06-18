//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrOSTelephony.h"
#include "AUMngrOSSecure.h"

STMngrOSTelephonyCalls AUMngrOSTelephony::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
};

AUMngrOSTelephony::AUMngrOSTelephony() : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::AUMngrOSTelephony")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrOSTelephony")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		UI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0);
		}
	}
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrOSTelephony::~AUMngrOSTelephony(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::~AUMngrOSTelephony")
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrOSTelephony::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrOSTelephony::setGlue(AUAppI* app, PTRfuncOSTelephonyCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::setGlue")
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

ENTelephonyAuthStatus AUMngrOSTelephony::authStatus(const BOOL requestIfNecesary){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::authStatus")
	ENTelephonyAuthStatus r = ENTelephonyAuthStatus_Denied;
	if(_calls.funcAuthStatus != NULL){
		r = (*_calls.funcAuthStatus)(_calls.funcAuthStatusParam, requestIfNecesary);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

SI32 AUMngrOSTelephony::getPhoneCount(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::getPhoneCount")
	SI32 r = 0;
	if(_calls.funcGetPhoneCount != NULL){
		r = (*_calls.funcGetPhoneCount)(_calls.funcGetPhoneCountParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrOSTelephony::getIMEI(const SI32 slot, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::getIMEI")
	BOOL r = FALSE;
	if(_calls.funcGetIMEI != NULL){
		r = (*_calls.funcGetIMEI)(_calls.funcGetIMEIParam, slot, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTelephony::canMakeCalls(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::canMakeCalls")
	bool r = FALSE;
	if(_calls.funcCanMakeCalls != NULL){
		r = (*_calls.funcCanMakeCalls)(_calls.funcCanMakeCallsParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTelephony::getCarrierCountryISO(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTelephony::getCarrierCountryISO")
	bool r = FALSE;
	if(_calls.funcGetCarrierCountryISO != NULL){
		r = (*_calls.funcGetCarrierCountryISO)(_calls.funcGetCarrierCountryISOParam, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrOSTelephony)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrOSTelephony, "AUMngrOSTelephony")
AUOBJMETODOS_CLONAR_NULL(AUMngrOSTelephony)
