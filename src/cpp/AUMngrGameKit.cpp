//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrGameKit.h"

#define AUMNGRGAMEKIT_MUTEX_ACTIVATE	NBHILO_MUTEX_ACTIVAR(&_mutex) _mutexLocksCount++;
#define AUMNGRGAMEKIT_MUTEX_DEACTIVATE	_mutexLocksCount--; NBHILO_MUTEX_DESACTIVAR(&_mutex)

STMngrGameKitCalls AUMngrGameKit::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
};

AUMngrGameKit::AUMngrGameKit() : AUObjeto()
	, _mutexLocksCount(0)
	, _scores(this)
	, _achievs(this)
{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::AUMngrGameKit")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrGameKit")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		SI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0)
		}
	}
#	endif
	NBHILO_MUTEX_INICIALIZAR(&_mutex)
	//this->loadData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrGameKit::~AUMngrGameKit(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::~AUMngrGameKit")
	AUMNGRGAMEKIT_MUTEX_ACTIVATE
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	{
		SI32 i;
		for(i = (_scores.conteo - 1); i >= 0; i--){
			STAppScore* data = _scores.elemPtr(i);
			if(data->uniqueId != NULL) data->uniqueId->liberar(NB_RETENEDOR_THIS); data->uniqueId = NULL;
		}
		_scores.vaciar();
	}
	{
		SI32 i;
		for(i = (_achievs.conteo - 1); i >= 0; i--){
			STAppAchiev* data = _achievs.elemPtr(i);
			if(data->uniqueId != NULL) data->uniqueId->liberar(NB_RETENEDOR_THIS); data->uniqueId = NULL;
		}
		_achievs.vaciar();
	}
	AUMNGRGAMEKIT_MUTEX_DEACTIVATE
	NBHILO_MUTEX_FINALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrGameKit::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcAuthState != NULL);
}

bool AUMngrGameKit::setGlue(AUAppI* app, PTRfuncGameKitCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::setGlue")
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

ENMngrGameKitAuthState AUMngrGameKit::authenticationState(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::authenticationState")
	ENMngrGameKitAuthState r = ENMngrGameKitAuthState_LogedOut;
	if(_calls.funcAuthState != NULL){
		r = (*_calls.funcAuthState)(_calls.funcAuthStateParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::startAuthentication(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::startAuthentication")
	bool r = false;
	if(_calls.funcAuthenticate != NULL){
		r = (*_calls.funcAuthenticate)(_calls.funcAuthenticateParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::getLocalPlayer(AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::getLocalPlayer")
	bool r = false;
	if(_calls.funcGetLocalPlayer != NULL){
		r = (*_calls.funcGetLocalPlayer)(_calls.funcGetLocalPlayerParam, dstId, dstName, dstDisplayName);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::setScore(const char* scoreId, const SI64 value){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::setScore")
	bool r = false;
	//Set value
	AUMNGRGAMEKIT_MUTEX_ACTIVATE
	STAppScore* score = this->privLockedAddOrUpdateScore(scoreId, value, ENMngrGameKitSrc_Count);
	AUMNGRGAMEKIT_MUTEX_DEACTIVATE
	//Send value
	if(score != NULL){
		AUMNGRGAMEKIT_MUTEX_ACTIVATE
		{
			STAppScore* score = this->privLockedGetScore(scoreId);
			if(score != NULL) score->val[ENMngrGameKitSrc_OSApi].valSending = value;
		}
		AUMNGRGAMEKIT_MUTEX_DEACTIVATE
		if(_calls.funcSendScore != NULL){
			if(!(*_calls.funcSendScore)(_calls.funcSendScoreParam, scoreId, value)){
				AUMNGRGAMEKIT_MUTEX_ACTIVATE
				{
					STAppScore* score = this->privLockedGetScore(scoreId);
					if(score != NULL) score->val[ENMngrGameKitSrc_OSApi].valSending = 0;
				}
				AUMNGRGAMEKIT_MUTEX_DEACTIVATE
			}
		}
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::setAchievProgress(const char* achievId, const SI8 prog100){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::setAchievProgress")
	bool r = false;
	//Set value
	AUMNGRGAMEKIT_MUTEX_ACTIVATE
	STAppAchiev* achiev = this->privLockedAddOrUpdateAchiev(achievId, prog100, ENMngrGameKitSrc_Count);
	AUMNGRGAMEKIT_MUTEX_DEACTIVATE
	//Send value
	if(achiev != NULL){
		AUMNGRGAMEKIT_MUTEX_ACTIVATE
		{
			STAppAchiev* achiev = this->privLockedGetAchiev(achievId);
			if(achiev != NULL) achiev->val[ENMngrGameKitSrc_OSApi].progSending = prog100;
		}
		AUMNGRGAMEKIT_MUTEX_DEACTIVATE
		if(_calls.funcSendAchiev != NULL){
			if(!(*_calls.funcSendAchiev)(_calls.funcSendAchievParam, achievId, prog100)){
				AUMNGRGAMEKIT_MUTEX_ACTIVATE
				{
					STAppAchiev* achiev = this->privLockedGetAchiev(achievId);
					if(achiev != NULL) achiev->val[ENMngrGameKitSrc_OSApi].progSending = 0;
				}
				AUMNGRGAMEKIT_MUTEX_DEACTIVATE
			}
		}
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::showCenter(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::showCenter")
	bool r = false;
	if(_calls.funcShowCenter != NULL){
		r = (*_calls.funcShowCenter)(_calls.funcShowCenterParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::showLeaderboard(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::showLeaderboard")
	bool r = false;
	if(_calls.funcShowLeaderbrd != NULL){
		r = (*_calls.funcShowLeaderbrd)(_calls.funcShowLeaderbrdParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void AUMngrGameKit::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::lock")
	AUMNGRGAMEKIT_MUTEX_ACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrGameKit::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::unlock")
	NBASSERT(_mutexLocksCount > 0)
	AUMNGRGAMEKIT_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool AUMngrGameKit::lockedLoadFromJSON(const char* str){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::lockedLoadFromJSON")
	bool r = false;
	NBASSERT(_mutexLocksCount > 0)
	if(str != NULL && _mutexLocksCount > 0){
		//PRINTF_INFO("AUMngrGameKit, loading data from JSON: '%s'.\n", str);
		AUDatosJSONMutable8* json = new AUDatosJSONMutable8();
		if(!json->cargaDatosJsonDesdeCadena(str)){
			PRINTF_WARNING("AUMngrGameKit, json could not be parsed.\n");
		} else {
			//--------------------
			//- Empty destinations
			//--------------------
			{
				{
					SI32 i;
					for(i = (_scores.conteo - 1); i >= 0; i--){
						STAppScore* data = _scores.elemPtr(i);
						if(data->uniqueId != NULL) data->uniqueId->liberar(NB_RETENEDOR_THIS); data->uniqueId = NULL;
					}
					_scores.vaciar();
				}
				{
					SI32 i;
					for(i = (_achievs.conteo - 1); i >= 0; i--){
						STAppAchiev* data = _achievs.elemPtr(i);
						if(data->uniqueId != NULL) data->uniqueId->liberar(NB_RETENEDOR_THIS); data->uniqueId = NULL;
					}
					_achievs.vaciar();
				}
			}
			//-----------
			//- Load data
			//-----------
			const STJsonNode* nRoot = json->nodoHijo();
			if(nRoot == NULL){
				PRINTF_WARNING("AUMngrGameKit, json is empty.\n");
			} else {
				AUCadenaMutable8* strTmp64 = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				//Notif received
				{
					const STJsonNode* scores = json->nodoHijo("scores", nRoot);
					if(scores != NULL){
						//Load new
						const STJsonNode* score = json->nodoHijo("scores", scores);
						while(score != NULL){
							strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("uid", strTmp64, "", score);
							if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
							if(strTmp->tamano() > 0){
								STAppScore data;
								data.uniqueId = new(ENMemoriaTipo_General) AUCadena8(strTmp->str());
								SI32 i;
								for(i = 0; i < ENMngrGameKitSrc_Count; i++){
									data.val[i].value		= 0;
									data.val[i].valSending	= 0;
									data.val[i].valSent		= 0;
								}
								//Load values
								const STJsonNode* vals = json->nodoHijo("vals", score);
								if(vals != NULL){
									const STJsonNode* val = json->nodoHijo("vals", vals);
									i = 0;
									while(val != NULL && i < ENMngrGameKitSrc_Count){
										data.val[i].value		= json->nodoHijo("v", (SI64)0, val);
										data.val[i].valSent		= json->nodoHijo("sent", data.val[i].value, val);
										val = json->nodoHijo("vals", vals, val);
										i++;
									}
								}
								_scores.agregarElemento(data);
							}
							//Next
							score = json->nodoHijo("scores", scores, score);
						}
					}
				}
				//Notif queue
				{
					const STJsonNode* achievs = json->nodoHijo("achievs", nRoot);
					if(achievs != NULL){
						//Load new
						const STJsonNode* achiev = json->nodoHijo("achievs", achievs);
						while(achiev != NULL){
							strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("uid", strTmp64, "", achiev);
							if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
							if(strTmp->tamano() > 0){
								STAppAchiev data;
								data.uniqueId = new(ENMemoriaTipo_General) AUCadena8(strTmp->str());
								SI32 i;
								for(i = 0; i < ENMngrGameKitSrc_Count; i++){
									data.val[i].prog100		= 0;
									data.val[i].progSending	= 0;
									data.val[i].progSent	= 0;
								}
								//Load values
								const STJsonNode* vals = json->nodoHijo("vals", achiev);
								if(vals != NULL){
									const STJsonNode* val = json->nodoHijo("vals", vals);
									i = 0;
									while(val != NULL && i < ENMngrGameKitSrc_Count){
										data.val[i].prog100		= json->nodoHijo("v", (SI8)0, val); NBASSERT(data.val[i].prog100 >= 0 && data.val[i].prog100 <= 100)
										data.val[i].progSent	= json->nodoHijo("sent", data.val[i].prog100, val); NBASSERT(data.val[i].progSent >= 0 && data.val[i].progSent <= 100)
										val = json->nodoHijo("vals", vals, val);
										i++;
									}
								}
								_achievs.agregarElemento(data);
							}
							//Next
							achiev = json->nodoHijo("achievs", achievs, achiev);
						}
					}
				}
				strTmp->liberar(NB_RETENEDOR_THIS);
				strTmp64->liberar(NB_RETENEDOR_THIS);
			}
			PRINTF_INFO("AUMngrGameKit, data loaded with %d scores and %d achievs.\n", _scores.conteo, _achievs.conteo);
			r = true;
		}
		json->liberar(NB_RETENEDOR_THIS);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::lockedSaveToJSON(AUCadenaMutable8* dst) const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::lockedSaveToJSON")
	NBASSERT(_mutexLocksCount > 0)
	bool r = false;
	if(dst != NULL && _mutexLocksCount > 0){
		dst->agregar(" { ");
		SI32 qArrs = 0;
		//Scores
		if(_scores.conteo > 0){
			qArrs++; if(qArrs != 1) dst->agregar(" ,");
			dst->agregar(" \"scores\": [");
			SI32 i; const SI32 count = _scores.conteo;
			for(i = 0; i < count; i++){
				STAppScore* data = _scores.elemPtr(i);
				if(i != 0) dst->agregar(",");
				dst->agregar(" { ");
				dst->agregar(" \"uid\": \""); AUBase64::codificaBase64(data->uniqueId->str(), data->uniqueId->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"vals\": [");
				SI32 i;
				for(i = 0; i < ENMngrGameKitSrc_Count; i++){
					if(i != 0) dst->agregar(",");
					dst->agregar(" {");
					dst->agregar("\"v\": \""); dst->agregarNumerico(data->val[i].value); dst->agregar("\"");
					dst->agregar(", \"sent\": \""); dst->agregarNumerico(data->val[i].valSent); dst->agregar("\"");
					dst->agregar("}");
				}
				dst->agregar(" ] ");
				dst->agregar(" } ");
			}
			dst->agregar("] ");
		}
		//Achievs
		if(_scores.conteo > 0){
			qArrs++; if(qArrs != 1) dst->agregar(" ,");
			dst->agregar(" \"achievs\": [");
			SI32 i; const SI32 count = _achievs.conteo;
			for(i = 0; i < count; i++){
				STAppAchiev* data = _achievs.elemPtr(i);
				if(i != 0) dst->agregar(",");
				dst->agregar(" { ");
				dst->agregar(" \"uid\": \""); AUBase64::codificaBase64(data->uniqueId->str(), data->uniqueId->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"vals\": [");
				SI32 i;
				for(i = 0; i < ENMngrGameKitSrc_Count; i++){
					if(i != 0) dst->agregar(",");
					dst->agregar(" {");
					dst->agregar("\"v\": \""); dst->agregarNumerico(data->val[i].prog100); dst->agregar("\"");
					dst->agregar(", \"sent\": \""); dst->agregarNumerico(data->val[i].progSent); dst->agregar("\"");
					dst->agregar("}");
				}
				dst->agregar(" ] ");
				dst->agregar(" } ");
			}
			dst->agregar("] ");
		}
		dst->agregar("}");
		//PRINTF_INFO("AUMngrGameKit, loading data to JSON: '%s'.\n", dst->str());
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::lockedScoreReportingResult(const char* scoreId, const SI64 value, const ENMngrGameKitSrc src, const bool success){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::lockedScoreReportingResult")
	NBASSERT(_mutexLocksCount > 0)
	bool r = false;
	if(_mutexLocksCount > 0){
		SI32 i; const SI32 count = _scores.conteo;
		for(i = 0; i < count; i++){
			STAppScore* data = _scores.elemPtr(i);
			if(data->uniqueId->esIgual(scoreId)){
				if(src < ENMngrGameKitSrc_Count){
					if(success){
						data->val[src].valSent = value;
					}
					if(data->val[src].valSending == value){
						data->val[src].valSending = 0;
					}
				}
				r = true;
				break;
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGameKit::lockedAchievReportingResult(const char* achievId, const SI8 prog100, const ENMngrGameKitSrc src, const bool success){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::lockedAchievReportingResult")
	NBASSERT(_mutexLocksCount > 0)
	bool r = false;
	if(_mutexLocksCount > 0){
		SI32 i; const SI32 count = _achievs.conteo;
		for(i = 0; i < count; i++){
			STAppAchiev* data = _achievs.elemPtr(i);
			if(data->uniqueId->esIgual(achievId)){
				if(src < ENMngrGameKitSrc_Count){
					if(success){
						data->val[src].progSent = prog100;
					}
					if(data->val[src].progSending == prog100){
						data->val[src].progSending = 0;
					}
				}
				r = true;
				break;
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

STAppScore* AUMngrGameKit::privLockedGetScore(const char* scoreId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::privLockedGetScore")
	STAppScore* record = NULL;
	NBASSERT(_mutexLocksCount > 0)
	if(_mutexLocksCount > 0){
		//Search for current record
		{
			SI32 i; const SI32 count = _scores.conteo;
			for(i = 0; i < count; i++){
				STAppScore* data = _scores.elemPtr(i);
				if(data->uniqueId->esIgual(scoreId)){
					record = data;
					break;
				}
			}
		}
		//Add new record
		if(record == NULL){
			STAppScore data;
			data.uniqueId = new(ENMemoriaTipo_General) AUCadena8(scoreId);
			SI32 i;
			for(i = 0; i < ENMngrGameKitSrc_Count; i++){
				data.val[i].value		= 0;
				data.val[i].valSending	= 0;
				data.val[i].valSent		= 0;
			}
			_scores.agregarElemento(data);
			record = _scores.elemPtr(_scores.conteo - 1);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return record;
}

STAppAchiev* AUMngrGameKit::privLockedGetAchiev(const char* achievId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::privLockedGetAchiev")
	STAppAchiev* record = NULL;
	NBASSERT(_mutexLocksCount > 0)
	if(_mutexLocksCount > 0){
		//Search for current record
		{
			SI32 i; const SI32 count = _achievs.conteo;
			for(i = 0; i < count; i++){
				STAppAchiev* data = _achievs.elemPtr(i);
				if(data->uniqueId->esIgual(achievId)){
					record = data;
					break;
				}
			}
		}
		//Add new record
		if(record == NULL){
			STAppAchiev data;
			data.uniqueId = new(ENMemoriaTipo_General) AUCadena8(achievId);
			SI32 i;
			for(i = 0; i < ENMngrGameKitSrc_Count; i++){
				data.val[i].prog100		= 0;
				data.val[i].progSending	= 0;
				data.val[i].progSent	= 0;
			}
			_achievs.agregarElemento(data);
			record = _achievs.elemPtr(_achievs.conteo - 1);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return record;
}

STAppScore* AUMngrGameKit::privLockedAddOrUpdateScore(const char* scoreId, const SI64 value, const ENMngrGameKitSrc src){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::privLockedAddOrUpdateScore")
	NBASSERT(_mutexLocksCount > 0)
	STAppScore* record = NULL;
	if(_mutexLocksCount > 0){
		record = this->privLockedGetScore(scoreId); NBASSERT(record != NULL)
		//Set values
		{
			SI32 i;
			for(i = 0; i < ENMngrGameKitSrc_Count; i++){
				record->val[i].value = value;
				if(src == i){
					record->val[i].valSent = value;
				}
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return record;
}

STAppAchiev* AUMngrGameKit::privLockedAddOrUpdateAchiev(const char* achievId, const SI8 prog100, const ENMngrGameKitSrc src){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGameKit::privLockedAddOrUpdateAchiev")
	STAppAchiev* record = NULL;
	NBASSERT(_mutexLocksCount > 0)
	if(_mutexLocksCount > 0){
		record = privLockedGetAchiev(achievId); NBASSERT(record != NULL)
		//Set values
		{
			SI32 i; const SI8 newVal = (prog100 < 0 ? 0 : prog100 > 100 ? 100 : prog100);
			for(i = 0; i < ENMngrGameKitSrc_Count; i++){
				record->val[i].prog100 = newVal;
				if(src == i){
					record->val[i].progSent = newVal;
				}
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return record;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrGameKit)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrGameKit, "AUMngrGameKit")
AUOBJMETODOS_CLONAR_NULL(AUMngrGameKit)
