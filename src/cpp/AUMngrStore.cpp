//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrStore.h"

STMngrStoreCalls AUMngrStore::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//
};

AUMngrStore::AUMngrStore() : AUObjeto()
	, _mutexLocksCount(0)
	, _storeState(ENStoreResult_Count)
	, _actionState(ENStoreResult_Count)
	, _actionProdId(NULL)
	, _storeProds(this)
{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::AUMngrStore")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrStore")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		SI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0)
		}
	}
#	endif
	NBHILO_MUTEX_INICIALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrStore::~AUMngrStore(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::~AUMngrStore")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	{
		SI32 i;
		for(i = (_storeProds.conteo - 1); i >= 0; i--){
			STAppStoreProd* data = _storeProds.elemPtr(i);
			if(data->prodId != NULL) data->prodId->liberar(NB_RETENEDOR_THIS); data->prodId = NULL;
			if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
			if(data->name != NULL) data->name->liberar(NB_RETENEDOR_THIS); data->name = NULL;
			if(data->desc != NULL) data->desc->liberar(NB_RETENEDOR_THIS); data->desc = NULL;
			if(data->price != NULL) data->price->liberar(NB_RETENEDOR_THIS); data->price = NULL;
			data->owned	= false;
		}
		_storeProds.vaciar();
	}
	if(_actionProdId != NULL) _actionProdId->liberar(NB_RETENEDOR_THIS); _actionProdId = NULL;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	NBHILO_MUTEX_FINALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrStore::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrStore::setGlue(AUAppI* app, PTRfuncStoreCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::setGlue")
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

ENStoreResult AUMngrStore::storeState() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::storeState")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _storeState;
}

ENStoreResult AUMngrStore::curActionState() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::storeState")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _actionState;
}

void AUMngrStore::addProdId(const char* prodId, const ENStoreProdType type, const char* grpId, const UI32 actions /*ENStorePurchaseActionBit*/, const float actionsSecsWait, const float actionsSecsRetry){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::addProdId")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	if(prodId != NULL){
		if(prodId[0] != '\0'){
			//Search current record
			SI32 i; const SI32 count = _storeProds.conteo;
			for(i = 0; i < count; i++){
				STAppStoreProd* data = _storeProds.elemPtr(i);
				if(data->prodId->esIgual(prodId)){
					break;
				}
			}
			//Add new record
			if(i == count){
				STAppStoreProd data;
				NBMemory_setZeroSt(data, STAppStoreProd);
				data.prodId	= new(this) AUCadena8(prodId);
				data.type	= type;
				data.grpId	= new(this) AUCadena8(grpId);
				data.name	= new(this) AUCadenaMutable8();
				data.desc	= new(this) AUCadenaMutable8();
				data.price	= new(this) AUCadenaMutable8();
				data.owned	= false;
				data.actions = actions;
				data.actionsSecsWait	= actionsSecsWait;
				data.actionsSecsRetry	= actionsSecsRetry;
				_storeProds.agregarElemento(data);
			}
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}


bool AUMngrStore::startStoreSync(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::startStoreSync")
	bool r = false;
	//Load data
	/*if(!this->loadData()){
		//NBASSERT(false) //will fail the first time, when no file has been saved.
	}*/
	//Sync with store
	{
		NBHILO_MUTEX_ACTIVAR(&_mutex)
		_mutexLocksCount++;
		if(_calls.funcStartRefresh == NULL){
			_storeState = ENStoreResult_Error;
			//PRINTF_ERROR("AUMngrStore, funcStartRefresh is NULL.\n");
		} else {
			const SI32 count = _storeProds.conteo;
			if(count > 0 && _storeState != ENStoreResult_Busy){
				STAppStoreProdId* prodIds = NBMemory_allocTypes(STAppStoreProdId, count);
				{
					SI32 i;
					for(i = 0; i < count; i++){
						STAppStoreProd* data = _storeProds.elemPtr(i);
						//PRINTF_INFO("Store: #%d prodId('%s').\n", (i + 1), data->prodId->str());
						NBMemory_setZero(prodIds[i]);
						prodIds[i].prodId	= (char*)data->prodId->str();
						prodIds[i].type		= data->type;
						prodIds[i].grpId	= (char*)data->grpId->str();
						prodIds[i].actions	= data->actions;
						prodIds[i].actionsSecsWait	= data->actionsSecsWait;
						prodIds[i].actionsSecsRetry	= data->actionsSecsRetry;
					}
				}
				_storeState = ENStoreResult_Busy;
				//Execute unlocked
				{
					_mutexLocksCount--;
					NBHILO_MUTEX_DESACTIVAR(&_mutex)
					{
						if((*_calls.funcStartRefresh)(_calls.funcStartRefreshParam, prodIds, count)){
							r = true;
						} else {
							_storeState = ENStoreResult_Error;
						}
					}
					NBHILO_MUTEX_ACTIVAR(&_mutex)
					_mutexLocksCount++;
				}
				//Release
				NBMemory_free(prodIds);
				prodIds = NULL;
			}
		}
		//
		_mutexLocksCount--;
		NBHILO_MUTEX_DESACTIVAR(&_mutex)
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::startRestoringReceipt(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::startRestoringReceipt")
	bool r = false;
	bool execute = false;
	{
		NBHILO_MUTEX_ACTIVAR(&_mutex)
		_mutexLocksCount++;
		if(_calls.funcStartRestore != NULL && _storeState == ENStoreResult_Success && _actionState != ENStoreResult_Busy && _actionProdId == NULL){
			NBASSERT(_actionProdId == NULL)
			_actionState = ENStoreResult_Busy;
			execute = true;
		}
		_mutexLocksCount--;
		NBHILO_MUTEX_DESACTIVAR(&_mutex)
	}
	//Execute outside the mutex
	if(execute){
		if((*_calls.funcStartReceiptRefresh)(_calls.funcStartReceiptRefreshParam)){
			r = true;
		} else {
			_actionState = ENStoreResult_Error;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::startRestoringPurchases(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::startRestoringPurchases")
	bool r = false;
	bool execute = false;
	{
		NBHILO_MUTEX_ACTIVAR(&_mutex)
		_mutexLocksCount++;
		if(_calls.funcStartRestore != NULL && _storeState == ENStoreResult_Success && _actionState != ENStoreResult_Busy && _actionProdId == NULL){
			NBASSERT(_actionProdId == NULL)
			_actionState = ENStoreResult_Busy;
			execute = true;
		}
		_mutexLocksCount--;
		NBHILO_MUTEX_DESACTIVAR(&_mutex)
	}
	//Execute outside the mutex
	if(execute){
		if((*_calls.funcStartRestore)(_calls.funcStartRestoreParam)){
			r = true;
		} else {
			_actionState = ENStoreResult_Error;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::startPurchase(const char* prodId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::startPurchase")
	bool r = false;
	bool execute = false;
	{
		NBHILO_MUTEX_ACTIVAR(&_mutex)
		_mutexLocksCount++;
		if(_calls.funcStartPruchase != NULL && _storeState == ENStoreResult_Success && _actionState != ENStoreResult_Busy && _actionProdId == NULL){
			//Search current record
			SI32 i; const SI32 count = _storeProds.conteo;
			for(i = 0; i < count; i++){
				STAppStoreProd* data = _storeProds.elemPtr(i);
				if(data->prodId->esIgual(prodId)){
					break;
				}
			}
			if(i < count){
				NBASSERT(_actionProdId == NULL)
				_actionProdId = new(this) AUCadena8(_storeProds.elemPtr(i)->prodId->str());
				_actionState = ENStoreResult_Busy;
				execute = true;
			}
		}
		_mutexLocksCount--;
		NBHILO_MUTEX_DESACTIVAR(&_mutex)
	}
	//Execute outside the mutex
	if(execute){
		if(!(*_calls.funcStartPruchase)(_calls.funcStartPruchaseParam, _actionProdId->str())){
			if(_actionProdId != NULL) _actionProdId->liberar(NB_RETENEDOR_THIS); _actionProdId = NULL;
			_actionState = ENStoreResult_Error;
		} else {
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::getProdProps(const char* prodId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDesc, AUCadenaMutable8* dstPrice, bool* dstOwned){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::getProdProps")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	/*if(_storeState == ENStoreResult_Success)*/{
		//Search current record
		SI32 i; const SI32 count = _storeProds.conteo;
		for(i = 0; i < count; i++){
			STAppStoreProd* data = _storeProds.elemPtr(i);
			if(data->prodId->esIgual(prodId)){
				if(dstName != NULL) dstName->establecer(data->name->str());
				if(dstDesc != NULL) dstDesc->establecer(data->desc->str());
				if(dstPrice != NULL) dstPrice->establecer(data->price->str());
				if(dstOwned != NULL) *dstOwned = data->owned;
				r = true;
				break;
			}
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::isOwned(const char* prodId) {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::isOwned")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	/*if(_storeState == ENStoreResult_Success)*/{
		//Search current record
		SI32 i; const SI32 count = _storeProds.conteo;
		for(i = 0; i < count; i++){
			STAppStoreProd* data = _storeProds.elemPtr(i);
			if(data->prodId->esIgual(prodId)){
				r = data->owned;
				break;
			}
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

BOOL AUMngrStore::concatLocalReceiptPayload(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::concatLocalReceiptPayload")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	BOOL r = FALSE;
	if(_calls.funcConcatLocalReceipt != NULL){
		if((*_calls.funcConcatLocalReceipt)(_calls.funcConcatLocalReceiptParam, dst)){
			r = TRUE;
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::loadData(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::loadData")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	if(_calls.funcLoadData != NULL){
		if((*_calls.funcLoadData)(_calls.funcLoadDataParam)){
			r = true;
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::setProdData(const char* prodId, const bool owned){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::setProdData")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	//NBASSERT(_storeState == ENStoreResult_Busy || _actionState == ENStoreResult_Busy)
	if(_storeState == ENStoreResult_Busy || _actionState == ENStoreResult_Busy){
		SI32 i; const SI32 count = _storeProds.conteo;
		for(i = 0; i < count; i++){
			STAppStoreProd* data = _storeProds.elemPtr(i);
			if(data->prodId->esIgual(prodId)){
				data->owned	= owned;
				r = true;
				break;
			}
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::setProdData(const char* prodId, const char* name, const char* desc, const char* price){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::setProdData")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	//PRINTF_INFO("AUMngrStore::setProdData('%s')-name('%s').\n", prodId, name);
	//NBASSERT(_storeState == ENStoreResult_Busy || _actionState == ENStoreResult_Busy)
	if(_storeState == ENStoreResult_Busy || _actionState == ENStoreResult_Busy){
		SI32 i; const SI32 count = _storeProds.conteo;
		for(i = 0; i < count; i++){
			STAppStoreProd* data = _storeProds.elemPtr(i);
			if(data->prodId->esIgual(prodId)){
				data->name->establecer(name);
				data->desc->establecer(desc);
				data->price->establecer(price);
				r = true;
				break;
			}
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::setProdData(const char* prodId, const char* name, const char* desc, const char* price, const bool owned){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::setProdData")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	//NBASSERT(_storeState == ENStoreResult_Busy || _actionState == ENStoreResult_Busy)
	if(_storeState == ENStoreResult_Busy || _actionState == ENStoreResult_Busy){
		SI32 i; const SI32 count = _storeProds.conteo;
		for(i = 0; i < count; i++){
			STAppStoreProd* data = _storeProds.elemPtr(i);
			if(data->prodId->esIgual(prodId)){
				data->name->establecer(name);
				data->desc->establecer(desc);
				data->price->establecer(price);
				data->owned	= owned;
				r = true;
				break;
			}
		}
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::endStoreSync(const ENStoreResult result){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::endStoreSync")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	//NBASSERT(_storeState == ENStoreResult_Busy && (result == ENStoreResult_Success || result == ENStoreResult_NoChanges || result == ENStoreResult_Error))
	if(_storeState == ENStoreResult_Busy && (result == ENStoreResult_Success || result == ENStoreResult_NoChanges || result == ENStoreResult_Error)){
		_storeState = result;
		//Save data
		if(_calls.funcSaveData != NULL){
			if(!(*_calls.funcSaveData)(_calls.funcSaveDataParam)){
				NBASSERT(false)
			}
		}
		r = true;
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::endStoreAction(const ENStoreResult result){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::endStoreAction")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	bool r = false;
	//NBASSERT(_actionState == ENStoreResult_Busy && (result == ENStoreResult_Success || result == ENStoreResult_NoChanges || result == ENStoreResult_Error))
	if(_actionState == ENStoreResult_Busy && (result == ENStoreResult_Success || result == ENStoreResult_NoChanges || result == ENStoreResult_Error)){
		_actionState = result;
		if(_actionProdId != NULL) _actionProdId->liberar(NB_RETENEDOR_THIS); _actionProdId = NULL;
		//Save data
		if(_calls.funcSaveData != NULL){
			if(!(*_calls.funcSaveData)(_calls.funcSaveDataParam)){
				NBASSERT(false)
			}
		}
		r = true;
	}
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void AUMngrStore::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::lock")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
	_mutexLocksCount++;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrStore::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::unlock")
	NBASSERT(_mutexLocksCount > 0)
	_mutexLocksCount--;
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrStore::lockedLoadFromJSON(const char* str){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::lockedLoadFromJSON")
	bool r = false;
	NBASSERT(_mutexLocksCount > 0)
	if(str != NULL && _mutexLocksCount > 0){
		//PRINTF_INFO("AUMngrStore, loading prods from JSON: '%s'.\n", str);
		AUDatosJSONMutable8* json = new AUDatosJSONMutable8();
		if(!json->cargaDatosJsonDesdeCadena(str)){
			PRINTF_ERROR("AUMngrStore, json could not be parsed.\n");
		} else {
			//--------------------
			//- Empty destinations
			//--------------------
			{
				SI32 i;
				for(i = (_storeProds.conteo - 1); i >= 0; i--){
					STAppStoreProd* data = _storeProds.elemPtr(i);
					if(data->prodId != NULL) data->prodId->liberar(NB_RETENEDOR_THIS); data->prodId = NULL;
					if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
					if(data->name != NULL) data->name->liberar(NB_RETENEDOR_THIS); data->name = NULL;
					if(data->desc != NULL) data->desc->liberar(NB_RETENEDOR_THIS); data->desc = NULL;
					if(data->price != NULL) data->price->liberar(NB_RETENEDOR_THIS); data->price = NULL;
					data->owned	= false;
				}
				_storeProds.vaciar();
			}
			//-----------
			//- Load data
			//-----------
			const STJsonNode* nRoot = json->nodoHijo();
			if(nRoot == NULL){
				PRINTF_WARNING("AUMngrStore, json is empty.\n");
			} else {
				AUCadenaMutable8* strTmp64 = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				//Products
				{
					const STJsonNode* prods = json->nodoHijo("prods", nRoot);
					if(prods != NULL){
						//Load new
						const STJsonNode* prod = json->nodoHijo("prods", prods);
						while(prod != NULL){
							STAppStoreProd itm;
							//
							strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("id", strTmp64, "", prod);
							if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
							if(strTmp->tamano() > 0){
								itm.prodId	= new(this) AUCadena8(strTmp->str());
								//
								strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("name", strTmp64, "", prod);
								if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
								itm.name	= new(this) AUCadenaMutable8(strTmp->str());
								//
								strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("desc", strTmp64, "", prod);
								if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
								itm.desc	= new(this) AUCadenaMutable8(strTmp->str());
								//
								strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("price", strTmp64, "", prod);
								if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
								itm.price	= new(this) AUCadenaMutable8(strTmp->str());
								//
								itm.owned	= json->nodoHijo("owned", false, prod);
								//
								_storeProds.agregarElemento(itm);
							}
							//Next
							prod = json->nodoHijo("prods", prods, prod);
						}
					}
				}
				strTmp->liberar(NB_RETENEDOR_THIS);
				strTmp64->liberar(NB_RETENEDOR_THIS);
			}
			PRINTF_INFO("AUMngrStore, products loaded with %d items.\n", _storeProds.conteo);
			r = true;
		}
		json->liberar(NB_RETENEDOR_THIS);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrStore::lockedSaveToJSON(AUCadenaMutable8* dst) const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrStore::lockedSaveToJSON")
	NBASSERT(_mutexLocksCount > 0)
	bool r = false;
	if(dst != NULL && _mutexLocksCount > 0){
		dst->agregar(" { ");
		SI32 qArrs = 0;
		//Prods
		if(_storeProds.conteo > 0){
			qArrs++; if(qArrs != 1) dst->agregar(" ,");
			dst->agregar(" \"prods\": [");
			SI32 i; const SI32 count = _storeProds.conteo;
			for(i = 0; i < count; i++){
				STAppStoreProd* data = _storeProds.elemPtr(i);
				if(i != 0) dst->agregar(",");
				dst->agregar(" { ");
				dst->agregar(" \"id\": \""); AUBase64::codificaBase64(data->prodId->str(), data->prodId->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"name\": \""); AUBase64::codificaBase64(data->name->str(), data->name->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"desc\": \""); AUBase64::codificaBase64(data->desc->str(), data->desc->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"price\": \""); AUBase64::codificaBase64(data->price->str(), data->price->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"owned\": \""); dst->agregarNumerico((SI32)(data->owned ? 1 : 0)); dst->agregar("\"");
				dst->agregar(" } ");
			}
			dst->agregar("] ");
		}
		dst->agregar("}");
		//PRINTF_INFO("AUMngrStore, loading prods to JSON: '%s'.\n", dst->str());
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrStore)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrStore, "AUMngrStore")
AUOBJMETODOS_CLONAR_NULL(AUMngrStore)
