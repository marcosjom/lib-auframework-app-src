//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrGoogleLogin.h"

AUMngrGoogleLogin* NBMngrGoogleLogin::_instance	= NULL;

void NBMngrGoogleLogin::init(AUAppI* app){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrGoogleLogin(app);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrGoogleLogin::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrGoogleLogin::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrGoogleLogin::addListener(IGoogleLoginListener* itf){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::addListener")
	const bool r = _instance->addListener(itf);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGoogleLogin::removeListener(IGoogleLoginListener* itf){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::removeListener")
	const bool r = _instance->removeListener(itf);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool NBMngrGoogleLogin::loadStoredData(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::loadStoredData")
	const bool r = _instance->loadStoredData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrGoogleLogin::setOAuthConfig(const char* clientId, const char* secret, const char* redirUri){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::setOAuthConfig")
	_instance->setOAuthConfig(clientId, secret, redirUri);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrGoogleLogin::loginStart(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::loginStart")
	const bool r = _instance->loginStart();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGoogleLogin::logout(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::logout")
	const bool r = _instance->logout();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGoogleLogin::getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::getUser")
	const bool r = _instance->getUser(dstSeq, dstId, dstEmail, dstFirstNames, dstLastNames, dstGender);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGoogleLogin::getUserPic(const ENGooglePicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGoogleLogin::getUserPic")
	const bool r = _instance->getUserPic(picSize, dstSeq, dstPath);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


