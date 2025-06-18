//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrNotifs.h"

AUMngrNotifs* NBMngrNotifs::_instance	= NULL;

void NBMngrNotifs::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrNotifs();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrNotifs::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrNotifs::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrNotifs::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::isGlued")
	const bool r = AUMngrNotifs::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


bool NBMngrNotifs::setGlue(AUAppI* app, PTRfuncNotifsCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::setGlue")
	const bool r = AUMngrNotifs::setGlue(app, initCall);
	if(_instance != NULL) _instance->loadData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

ENAppNotifAuthState NBMngrNotifs::getAuthStatus(const ENAppNotifAuthQueryMode reqMode){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::getAuthStatus")
	ENAppNotifAuthState r = _instance->getAuthStatus(reqMode);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Local notifications

bool NBMngrNotifs::localEnable(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::localEnable")
	bool r = _instance->localEnable();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrNotifs::localAdd(const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::localAdd")
	bool r = _instance->localAdd(grpId, notifId, secsFromNow, title, content, notifData);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrNotifs::localCancel(const char* grpId, const SI32 notifId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::localCancel")
	bool r = _instance->localCancel(grpId, notifId);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrNotifs::localCancelGrp(const char* grpId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::localCancelGrp")
	bool r = _instance->localCancelGrp(grpId);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrNotifs::localCancelAll(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::localCancelAll")
	bool r = _instance->localCancelAll();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Remote notifications

bool NBMngrNotifs::remoteGetToken(const ENAppNotifTokenQueryMode mode, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::remoteGetToken")
	bool r = _instance->remoteGetToken(mode, dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrNotifs::remoteSetToken(const void* token, const UI32 tokenSz){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::remoteSetToken")
	_instance->remoteSetToken(token, tokenSz);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

void NBMngrNotifs::clearRcvdNotifs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::clearRcvdNotifs")
	_instance->clearRcvdNotifs();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool NBMngrNotifs::loadData(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::loadData")
	bool r = _instance->loadData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrNotifs::setLaunchNotif(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::setLaunchNotif")
	_instance->setLaunchNotif(type, uid, grpId, notifId, data);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrNotifs::addNotifRcvd(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::processNew")
	bool r = _instance->addNotifRcvd(type, uid, grpId, notifId, data);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void NBMngrNotifs::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::lock")
	_instance->lock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrNotifs::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::unlock")
	_instance->unlock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const STAppNotif* NBMngrNotifs::lockedGetLaunchNotif(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::lockedGetLaunchNotif")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _instance->lockedGetLaunchNotif();
}

void NBMngrNotifs::lockedClearLaunchNotif(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::lockedClearLaunchNotif")
	_instance->lockedClearLaunchNotif();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const STAppNotif* NBMngrNotifs::lockedGetNotifsQueue(SI32* dstSize){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::lockedGetNotifsQueue")
	const STAppNotif* r = _instance->lockedGetNotifsQueue(dstSize);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool NBMngrNotifs::lockedLoadFromJSON(const char* json){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::lockedLoadFromJSON")
	bool r = _instance->lockedLoadFromJSON(json);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrNotifs::lockedSaveToJSON(AUCadenaMutable8* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrNotifs::lockedSaveToJSON")
	bool r = _instance->lockedSaveToJSON(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


