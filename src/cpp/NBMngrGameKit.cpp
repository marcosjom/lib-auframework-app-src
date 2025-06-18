//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrGameKit.h"

AUMngrGameKit* NBMngrGameKit::_instance	= NULL;

void NBMngrGameKit::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrGameKit();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrGameKit::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrGameKit::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrGameKit::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::isGlued")
	const bool r = AUMngrGameKit::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::setGlue(AUAppI* app, PTRfuncGameKitCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::setGlue")
	const bool r = AUMngrGameKit::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

ENMngrGameKitAuthState NBMngrGameKit::authenticationState(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::authenticationState")
	ENMngrGameKitAuthState r = _instance->authenticationState();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


bool NBMngrGameKit::startAuthentication(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::startAuthentication")
	bool r = _instance->startAuthentication();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::getLocalPlayer(AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::getLocalPlayer")
	bool r = _instance->getLocalPlayer(dstId, dstName, dstDisplayName);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::setScore(const char* scoreId, const SI64 value){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::setScore")
	bool r = _instance->setScore(scoreId, value);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::setAchievProgress(const char* achievId, const SI8 prog100){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::setAchievProgress")
	bool r = _instance->setAchievProgress(achievId, prog100);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::showCenter(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::showCenter")
	bool r = _instance->showCenter();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::showLeaderboard(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::showLeaderboard")
	bool r = _instance->showLeaderboard();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void NBMngrGameKit::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::lock")
	_instance->lock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrGameKit::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::unlock")
	_instance->unlock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrGameKit::lockedLoadFromJSON(const char* str){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::lockedLoadFromJSON")
	bool r = _instance->lockedLoadFromJSON(str);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::lockedSaveToJSON(AUCadenaMutable8* dst) {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::lockedSaveToJSON")
	bool r = _instance->lockedSaveToJSON(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::lockedScoreReportingResult(const char* scoreId, const SI64 value, const ENMngrGameKitSrc src, const bool success){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::lockedScoreReportingResult")
	bool r = _instance->lockedScoreReportingResult(scoreId, value, src, success);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrGameKit::lockedAchievReportingResult(const char* achievId, const SI8 prog100, const ENMngrGameKitSrc src, const bool success){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrGameKit::lockedAchievReportingResult")
	bool r = _instance->lockedAchievReportingResult(achievId, prog100, src, success);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


