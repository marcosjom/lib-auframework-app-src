//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrContacts.h"

AUMngrContacts* NBMngrContacts::_instance	= NULL;

void NBMngrContacts::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrContacts();
	if(_instance != NULL) _instance->setAsMainStore();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrContacts::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrContacts::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrContacts::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::isGlued")
	bool r = AUMngrContacts::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrContacts::setGlue(AUAppI* app, PTRfuncContactsCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::setGlue")
	bool r = AUMngrContacts::setGlue(app, initCall);
	if(_instance != NULL) _instance->setAsMainStore();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

ENContactsAuthStatus NBMngrContacts::authStatus(const BOOL requestIfNecesary){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::getPhoneCount")
	const ENContactsAuthStatus r = _instance->authStatus(requestIfNecesary);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Search concact

const STNBContact* NBMngrContacts::getContactLocked(const char* contactId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::getContactLocked")
	const STNBContact* r = _instance->getContactLocked(contactId);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrContacts::returnContact(const STNBContact* cc){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::returnContact")
	_instance->returnContact(cc);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//Read contacts

const STNBContact** NBMngrContacts::getContactsLocked(UI32* dstSz){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::getContactsLocked")
	const STNBContact** r = _instance->getContactsLocked(dstSz);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrContacts::returnContacts(const STNBContact** cc){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::returnContacts")
	_instance->returnContacts(cc);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//Read thumbnail

const STNBContactThumbnailRef* NBMngrContacts::getThumbnailRetained(const char* contactId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::getThumbnailRetained")
	const STNBContactThumbnailRef* r = _instance->getThumbnailRetained(contactId);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrContacts::releaseThumbnail(const STNBContactThumbnailRef* t){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::releaseThumbnail")
	_instance->releaseThumbnail(t);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//Listeners

void NBMngrContacts::addLstnr(STNBContactsLstnr* lstnr){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::addLstnr")
	_instance->addLstnr(lstnr);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrContacts::removeLstnr(void* lstnrParam){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::removeLstnr")
	_instance->removeLstnr(lstnrParam);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrContacts::tick(const float secs){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrContacts::tick")
	_instance->tick(secs);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

