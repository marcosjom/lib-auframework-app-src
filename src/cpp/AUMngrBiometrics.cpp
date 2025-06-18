//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrBiometrics.h"
#include "AUMngrOSSecure.h"

STMngrBiometricsCalls AUMngrBiometrics::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
};

AUMngrBiometrics::AUMngrBiometrics() : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::AUMngrBiometrics")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrBiometrics")
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

AUMngrBiometrics::~AUMngrBiometrics(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::~AUMngrBiometrics")
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrBiometrics::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrBiometrics::setGlue(AUAppI* app, PTRfuncBiometricsCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::setGlue")
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

void AUMngrBiometrics::getTypeName(const ENBiometricsType type, STNBString* dstName){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::getTypeName")
	if(_calls.funcGetTypeName != NULL){
		(*_calls.funcGetTypeName)(_calls.funcGetTypeNameParam, type, dstName);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

BOOL AUMngrBiometrics::canAuthenticate(const ENBiometricsType type, STNBString* dstError){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::canAuthenticate")
	BOOL r = FALSE;
	if(_calls.funcCanAuthenticate != NULL){
		r = (*_calls.funcCanAuthenticate)(_calls.funcCanAuthenticateParam, type, dstError);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrBiometrics::showsOwnGui(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::showsOwnGui")
	BOOL r = FALSE;
	if(_calls.funcShowsOwnGui != NULL){
		r = (*_calls.funcShowsOwnGui)(_calls.funcShowsOwnGuiParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrBiometrics::startAuthentication(const char* reasonTitle, const char* cancelTitle){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::startAuthentication")
	BOOL r = FALSE;
	if(_calls.funcStartAuthentication != NULL){
		r = (*_calls.funcStartAuthentication)(_calls.funcStartAuthenticationParam, reasonTitle, cancelTitle);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrBiometrics::cancelAuthentication(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::cancelAuthentication")
	if(_calls.funcCancelAuthentication != NULL){
		(*_calls.funcCancelAuthentication)(_calls.funcCancelAuthenticationParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

ENBiometricsAuthStatus AUMngrBiometrics::authStatus(const UI64 secsLastValid){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrBiometrics::authStatus")
	ENBiometricsAuthStatus r = ENBiometricsAuthStatus_Failed;
	if(_calls.funcAuthStatus != NULL){
		r = (*_calls.funcAuthStatus)(_calls.funcAuthStatusParam, secsLastValid);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrBiometrics)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrBiometrics, "AUMngrBiometrics")
AUOBJMETODOS_CLONAR_NULL(AUMngrBiometrics)
