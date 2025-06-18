//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrStore.h"

AUMngrStore* NBMngrStore::_instance	= NULL;

void NBMngrStore::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrStore();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrStore::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrStore::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrStore::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::isGlued")
	bool r = AUMngrStore::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::setGlue(AUAppI* app, PTRfuncStoreCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::setGlue")
	bool r = AUMngrStore::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

ENStoreResult NBMngrStore::storeState(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::storeState")
	const ENStoreResult r = _instance->storeState();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

ENStoreResult NBMngrStore::curActionState(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::curActionState")
	const ENStoreResult r = _instance->curActionState();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrStore::addProdId(const char* prodId, const ENStoreProdType type, const char* grpId, const UI32 actions /*ENStorePurchaseActionBit*/, const float actionsSecsWait, const float actionsSecsRetry){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::addProdId")
	_instance->addProdId(prodId, type, grpId, actions, actionsSecsWait, actionsSecsRetry);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrStore::startStoreSync(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::startStoreSync")
	const bool r = _instance->startStoreSync();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::startRestoringReceipt(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::startRestoringReceipt")
	const bool r = _instance->startRestoringReceipt();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::startRestoringPurchases(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::startRestoringPurchases")
	const bool r = _instance->startRestoringPurchases();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::startPurchase(const char* prodId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::startPurchase")
	const bool r = _instance->startPurchase(prodId);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::getProdProps(const char* prodId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDesc, AUCadenaMutable8* dstPrice, bool* dstOwned){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::getProdProps")
	const bool r = _instance->getProdProps(prodId, dstName, dstDesc, dstPrice, dstOwned);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::isOwned(const char* prodId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::isOwned")
	const bool r = _instance->isOwned(prodId);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

BOOL NBMngrStore::concatLocalReceiptPayload(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::concatLocalReceiptPayload")
	const BOOL r = _instance->concatLocalReceiptPayload(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool NBMngrStore::loadData(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::loadData")
	const bool r = _instance->loadData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::setProdData(const char* prodId, const bool owned){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::setProdData")
	const bool r = _instance->setProdData(prodId, owned);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::setProdData(const char* prodId, const char* name, const char* desc, const char* price){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::setProdData")
	const bool r = _instance->setProdData(prodId, name, desc, price);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::setProdData(const char* prodId, const char* name, const char* desc, const char* price, const bool owned){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::setProdData")
	const bool r = _instance->setProdData(prodId, name, desc, price, owned);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::endStoreSync(const ENStoreResult result){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::endStoreSync")
	const bool r = _instance->endStoreSync(result);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::endStoreAction(const ENStoreResult result){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::endStoreAction")
	const bool r = _instance->endStoreAction(result);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void NBMngrStore::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::lock")
	_instance->lock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrStore::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::unlock")
	_instance->unlock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrStore::lockedLoadFromJSON(const char* json){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::lockedLoadFromJSON")
	const bool r = _instance->lockedLoadFromJSON(json);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrStore::lockedSaveToJSON(AUCadenaMutable8* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrStore::lockedSaveToJSON")
	const bool r = _instance->lockedSaveToJSON(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}




