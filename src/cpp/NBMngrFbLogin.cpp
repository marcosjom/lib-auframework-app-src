//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrFbLogin.h"

AUMngrFbLogin* NBMngrFbLogin::_instance	= NULL;

void NBMngrFbLogin::init(AUAppI* app){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrFbLogin(app);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrFbLogin::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrFbLogin::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrFbLogin::addListener(IFbLoginListener* itf){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::addListener")
	const bool r = _instance->addListener(itf);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrFbLogin::removeListener(IFbLoginListener* itf){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::removeListener")
	const bool r = _instance->removeListener(itf);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool NBMngrFbLogin::loadStoredData(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::loadStoredData")
	const bool r = _instance->loadStoredData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrFbLogin::setConfig(const char* appClientId, const char* webClientId, const char* webSecret, const char* webRedirUri){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::setConfig")
	_instance->setConfig(appClientId, webClientId, webSecret, webRedirUri);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrFbLogin::loginStart(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::loginStart")
	const bool r = _instance->loginStart();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrFbLogin::logout(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::logout")
	const bool r = _instance->logout();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrFbLogin::getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::getUser")
	const bool r = _instance->getUser(dstSeq, dstId, dstEmail, dstFirstNames, dstLastNames, dstGender);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrFbLogin::getUserPic(const ENFbPicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrFbLogin::getUserPic")
	const bool r = _instance->getUserPic(picSize, dstSeq, dstPath);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

