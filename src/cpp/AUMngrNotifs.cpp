//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrNotifs.h"

#define AUMNGRNOTIFS_MUTEX_ACTIVATE		NBHILO_MUTEX_ACTIVAR(&_mutex) _mutexLocksCount++;
#define AUMNGRNOTIFS_MUTEX_DEACTIVATE	_mutexLocksCount--; NBHILO_MUTEX_DESACTIVAR(&_mutex)

STMngrNotifsCalls AUMngrNotifs::_calls = {
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

AUMngrNotifs::AUMngrNotifs() : AUObjeto()
	, _mutexLocksCount(0)
	, _lastUniqueId(0)
	, _launchNotif(NULL)
	, _notifsQueued(this)
	, _notifsRcvd(this)
{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::AUMngrNotifs")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrNotifs")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		SI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0)
		}
	}
#	endif
	NBHILO_MUTEX_INICIALIZAR(&_mutex)
	this->loadData();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrNotifs::~AUMngrNotifs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::~AUMngrNotifs")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	{
		SI32 i;
		for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
			STAppNotif* data = _notifsQueued.elemPtr(i);
			if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
			if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
		}
		_notifsQueued.vaciar();
	}
	{
		SI32 i;
		for(i = (_notifsRcvd.conteo - 1); i >= 0; i--){
			STAppNotif* data = _notifsRcvd.elemPtr(i);
			if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
			if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
		}
		_notifsRcvd.vaciar();
	}
	if(_launchNotif != NULL){
		if(_launchNotif->grpId != NULL) _launchNotif->grpId->liberar(NB_RETENEDOR_THIS); _launchNotif->grpId = NULL;
		if(_launchNotif->data != NULL) _launchNotif->data->liberar(NB_RETENEDOR_THIS); _launchNotif->data = NULL;
		NBGestorMemoria::liberarMemoria(_launchNotif);
		_launchNotif = NULL;
	}
	_lastUniqueId = 0;
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	NBHILO_MUTEX_FINALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrNotifs::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcLocalEnable != NULL);
}

bool AUMngrNotifs::setGlue(AUAppI* app, PTRfuncNotifsCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::setGlue")
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

ENAppNotifAuthState AUMngrNotifs::getAuthStatus(const ENAppNotifAuthQueryMode reqMode){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::getAuthStatus")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	ENAppNotifAuthState r = ENAppNotifAuthState_Denied;
	if(_calls.funcGetAuthStatus != NULL){
		r = (*_calls.funcGetAuthStatus)(_calls.funcGetAuthStatusParam, reqMode);
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool AUMngrNotifs::localEnable(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::localEnable")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcLocalEnable != NULL){
		r = (*_calls.funcLocalEnable)(_calls.funcLocalEnableParam);
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrNotifs::localAdd(const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::localAdd")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcLocalAdd != NULL){
		const SI32 uid = ++_lastUniqueId;
		if((*_calls.funcLocalAdd)(_calls.funcLocalAddParam, uid, grpId, notifId, secsFromNow, title, content, notifData)){
			//Add to queued
			STAppNotif itm;
			itm.uniqueId = uid;
			itm.type	= ENNotifType_Local;
			itm.grpId	= new(this) AUCadena8((grpId != NULL ? grpId : ""));
			itm.notifId	= notifId;
			itm.data	= new(this) AUCadena8((notifData != NULL ? notifData : ""));
			_notifsQueued.agregarElemento(itm);
			//Save data
			if(_calls.funcSaveData != NULL){
				if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
					//
				}
			}
			//
			r = true;
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrNotifs::localCancel(const char* grpId, const SI32 notifId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::localCancel")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcLocalCancel != NULL){
		if((*_calls.funcLocalCancel)(_calls.funcLocalCancelParam, grpId, notifId)){
			//Remove from queued
			{
				SI32 i;
				for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
					STAppNotif* data = _notifsQueued.elemPtr(i);
					if(data->notifId == notifId){
						if(data->grpId->esIgual(grpId)){
							if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
							if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
							_notifsQueued.quitarElementoEnIndice(i);
						}
					}
				}
			}
			//Save data
			if(_calls.funcSaveData != NULL){
				if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
					//
				}
			}
			r = true;
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrNotifs::localCancelGrp(const char* grpId){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::localCancelGrp")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcLocalCancelGrp != NULL){
		if((*_calls.funcLocalCancelGrp)(_calls.funcLocalCancelGrpParam, grpId)){
			//Remove from queued
			{
				SI32 i;
				for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
					STAppNotif* data = _notifsQueued.elemPtr(i);
					if(data->grpId->esIgual(grpId)){
						if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
						if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
						_notifsQueued.quitarElementoEnIndice(i);
					}
				}
			}
			//Save data
			if(_calls.funcSaveData != NULL){
				if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
					//
				}
			}
			r = true;
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrNotifs::localCancelAll(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::localCancelAll")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcLocalCancelAll != NULL){
		if((*_calls.funcLocalCancelAll)(_calls.funcLocalCancelAllParam)){
			//Remove from queued
			{
				SI32 i;
				for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
					STAppNotif* data = _notifsQueued.elemPtr(i);
					if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
					if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
				}
				_notifsQueued.vaciar();
			}
			//Save data
			if(_calls.funcSaveData != NULL){
				if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
					//
				}
			}
			r = true;
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Remote notifications

bool AUMngrNotifs::remoteGetToken(const ENAppNotifTokenQueryMode mode, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::remoteGetToken")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcRemoteGetToken != NULL){
		r = (*_calls.funcRemoteGetToken)(_calls.funcRemoteGetTokenParam, mode, dst);
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrNotifs::remoteSetToken(const void* token, const UI32 tokenSz){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::remoteSetToken")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	if(_calls.funcRemoteSetToken != NULL){
		(*_calls.funcRemoteSetToken)(_calls.funcRemoteSetTokenParam, token, tokenSz);
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrNotifs::loadData(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::loadData")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcLoadData != NULL){
		if((*_calls.funcLoadData)(_calls.funcLoadDataParam)){
			r = true;
			//Update app's icon badge
			if(_calls.funcSetBadgeNumber != NULL){
				(*_calls.funcSetBadgeNumber)(_calls.funcSetBadgeNumberParam, _notifsRcvd.conteo);
			}
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrNotifs::setLaunchNotif(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::addNotifRcvd")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	//Remove prev
	if(_launchNotif != NULL){
		if(_launchNotif->grpId != NULL) _launchNotif->grpId->liberar(NB_RETENEDOR_THIS); _launchNotif->grpId = NULL;
		if(_launchNotif->data != NULL) _launchNotif->data->liberar(NB_RETENEDOR_THIS); _launchNotif->data = NULL;
		NBGestorMemoria::liberarMemoria(_launchNotif);
		_launchNotif = NULL;
	}
	//Set new
	{
		_launchNotif			= (STAppNotif*)NBGestorMemoria::reservarMemoria(sizeof(STAppNotif), ENMemoriaTipo_General);
		_launchNotif->uniqueId	= uid;
		_launchNotif->type		= type;
		_launchNotif->grpId		= new(this) AUCadena8((grpId != NULL ? grpId : ""));
		_launchNotif->notifId	= notifId;
		_launchNotif->data		= new(this) AUCadena8((data != NULL ? data : ""));
		//Remove from queue
		{
			SI32 i;
			for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
				STAppNotif* data = _notifsQueued.elemPtr(i);
				if(data->uniqueId == uid){
					if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
					if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
					_notifsQueued.quitarElementoEnIndice(i);
				}
			}
		}
		//Save data
		if(_calls.funcSaveData != NULL){
			if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
				//
			}
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrNotifs::clearRcvdNotifs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::addNotifRcvd")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	//Empty rcvd list
	{
		SI32 i;
		for(i = (_notifsRcvd.conteo - 1); i >= 0; i--){
			STAppNotif* data = _notifsRcvd.elemPtr(i);
			if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
			if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
		}
		_notifsRcvd.vaciar();
	}
	//Update app's icon badge
	if(_calls.funcSetBadgeNumber != NULL){
		(*_calls.funcSetBadgeNumber)(_calls.funcSetBadgeNumberParam, _notifsRcvd.conteo);
	}
	//Save data
	if(_calls.funcSaveData != NULL){
		if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
			//
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool AUMngrNotifs::addNotifRcvd(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::addNotifRcvd")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	bool r = false;
	//Add to received
	{
		STAppNotif itm;
		itm.uniqueId	= uid;
		itm.type		= type;
		itm.grpId		= new(this) AUCadena8((grpId != NULL ? grpId : ""));
		itm.notifId	= notifId;
		itm.data		= new(this) AUCadena8((data != NULL ? data : ""));
		_notifsRcvd.agregarElemento(itm);
	}
	//Remove from queued
	{
		SI32 i;
		for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
			STAppNotif* data = _notifsQueued.elemPtr(i);
			if(data->uniqueId == uid){
				if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
				if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
				_notifsQueued.quitarElementoEnIndice(i);
			}
		}
	}
	//Update app's icon badge
	if(_calls.funcSetBadgeNumber != NULL){
		(*_calls.funcSetBadgeNumber)(_calls.funcSetBadgeNumberParam, _notifsRcvd.conteo);
	}
	//Save data
	if(_calls.funcSaveData != NULL){
		if((*_calls.funcSaveData)(_calls.funcSaveDataParam)){
			//
		}
	}
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void AUMngrNotifs::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::lock")
	AUMNGRNOTIFS_MUTEX_ACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrNotifs::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::unlock")
	NBASSERT(_mutexLocksCount > 0)
	AUMNGRNOTIFS_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const STAppNotif* AUMngrNotifs::lockedGetLaunchNotif() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::lockedGetLaunchNotif")
	const STAppNotif* r = NULL;
	NBASSERT(_mutexLocksCount > 0)
	if(_mutexLocksCount > 0){
		r = _launchNotif;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrNotifs::lockedClearLaunchNotif(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::lockedClearLaunchNotif")
	NBASSERT(_mutexLocksCount > 0)
	if(_mutexLocksCount > 0){
		//Remove
		if(_launchNotif != NULL){
			if(_launchNotif->grpId != NULL) _launchNotif->grpId->liberar(NB_RETENEDOR_THIS); _launchNotif->grpId = NULL;
			if(_launchNotif->data != NULL) _launchNotif->data->liberar(NB_RETENEDOR_THIS); _launchNotif->data = NULL;
			NBGestorMemoria::liberarMemoria(_launchNotif);
			_launchNotif = NULL;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const STAppNotif* AUMngrNotifs::lockedGetNotifsQueue(SI32* dstSize) const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::lockedGetNotifsQueue")
	const STAppNotif* r = NULL;
	NBASSERT(_mutexLocksCount > 0)
	if(_mutexLocksCount > 0){
		if(dstSize != NULL) *dstSize = _notifsQueued.conteo;
		r = _notifsQueued.arreglo();
	} else {
		if(dstSize != NULL) *dstSize = 0;
		r = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool AUMngrNotifs::lockedLoadFromJSON(const char* str){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::lockedLoadFromJSON")
	bool r = false;
	NBASSERT(_mutexLocksCount > 0)
	if(str != NULL && _mutexLocksCount > 0){
		//PRINTF_INFO("AUMngrNotifs, loading data from JSON: '%s'.\n", str);
		BOOL isEmpty = TRUE;
		//Analyze content
		{
			UI32 i = 0;
			while(str[i] != '\0'){
				if(str[i] != '{' && str[i] != '}' && str[i] != ' '){
					isEmpty = FALSE;
					break;
				}
				i++;
			}
		}
		//Process content
		if(isEmpty){
			r = TRUE;
		} else {
			AUDatosJSONMutable8* json = new AUDatosJSONMutable8();
			if(!json->cargaDatosJsonDesdeCadena(str)){
				PRINTF_WARNING("AUMngrNotifs, json could not be parsed.\n");
			} else {
				//--------------------
				//- Empty destinations
				//--------------------
				{
					_lastUniqueId = 0;
					//Remove received
					{
						SI32 i;
						for(i = (_notifsRcvd.conteo - 1); i >= 0; i--){
							STAppNotif* data = _notifsRcvd.elemPtr(i);
							if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
							if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
						}
						_notifsRcvd.vaciar();
					}
					//Remove queue
					{
						SI32 i;
						for(i = (_notifsQueued.conteo - 1); i >= 0; i--){
							STAppNotif* data = _notifsQueued.elemPtr(i);
							if(data->grpId != NULL) data->grpId->liberar(NB_RETENEDOR_THIS); data->grpId = NULL;
							if(data->data != NULL) data->data->liberar(NB_RETENEDOR_THIS); data->data = NULL;
						}
						_notifsQueued.vaciar();
					}
				}
				//-----------
				//- Load data
				//-----------
				const STJsonNode* nRoot = json->nodoHijo();
				if(nRoot == NULL){
					PRINTF_WARNING("AUMngrNotifs, json is empty.\n");
				} else {
					AUCadenaMutable8* strTmp64 = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
					AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
					//Notif received
					{
						const STJsonNode* notifs = json->nodoHijo("notifsRcvd", nRoot);
						if(notifs != NULL){
							//Load new
							const STJsonNode* notif = json->nodoHijo("notifsRcvd", notifs);
							while(notif != NULL){
								const SI32 uid = json->nodoHijo("uid", 0, notif);
								const SI32 type = json->nodoHijo("type", -1, notif);
								const SI32 notifId = json->nodoHijo("notifId", 0, notif);
								if(uid > 0 && type >= 0 && type <= ENNotifType_Count){
									STAppNotif itm;
									itm.uniqueId = uid; if(_lastUniqueId < uid) _lastUniqueId = uid;
									itm.notifId	= notifId;
									itm.type	= ENNotifType_Local;
									//
									strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("grpId", strTmp64, "", notif);
									if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
									itm.grpId	= new(this) AUCadena8(strTmp->str());
									//
									strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("data", strTmp64, "", notif);
									if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
									itm.data	= new(this) AUCadena8(strTmp->str());
									//
									_notifsRcvd.agregarElemento(itm);
								}
								//Next
								notif = json->nodoHijo("notifsRcvd", notifs, notif);
							}
						}
					}
					//Notif queue
					{
						const STJsonNode* notifs = json->nodoHijo("notifsQueue", nRoot);
						if(notifs != NULL){
							//Load new
							const STJsonNode* notif = json->nodoHijo("notifsQueue", notifs);
							while(notif != NULL){
								const SI32 uid = json->nodoHijo("uid", 0, notif);
								const SI32 type = json->nodoHijo("type", -1, notif);
								const SI32 notifId = json->nodoHijo("notifId", 0, notif);
								if(uid > 0 && type >= 0 && type <= ENNotifType_Count){
									STAppNotif itm;
									itm.uniqueId = uid; if(_lastUniqueId < uid) _lastUniqueId = uid;
									itm.notifId	= notifId;
									itm.type	= ENNotifType_Local;
									//
									strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("grpId", strTmp64, "", notif);
									if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
									itm.grpId	= new(this) AUCadena8(strTmp->str());
									//
									strTmp64->vaciar(); strTmp->vaciar(); json->nodoHijo("data", strTmp64, "", notif);
									if(strTmp64->tamano() > 0) AUBase64::decodificaBase64(strTmp64->str(), strTmp64->tamano(), strTmp);
									itm.data	= new(this) AUCadena8(strTmp->str());
									//
									_notifsQueued.agregarElemento(itm);
								}
								//Next
								notif = json->nodoHijo("notifsQueue", notifs, notif);
							}
						}
					}
					//Update app's icon badge
					if(_calls.funcSetBadgeNumber != NULL){
						(*_calls.funcSetBadgeNumber)(_calls.funcSetBadgeNumberParam, _notifsRcvd.conteo);
					}
					strTmp->liberar(NB_RETENEDOR_THIS);
					strTmp64->liberar(NB_RETENEDOR_THIS);
				}
				PRINTF_INFO("AUMngrNotifs, notifications loaded with %d at queue and %d received.\n", _notifsQueued.conteo, _notifsRcvd.conteo);
				r = true;
			}
			json->liberar(NB_RETENEDOR_THIS);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrNotifs::lockedSaveToJSON(AUCadenaMutable8* dst) const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrNotifs::lockedSaveToJSON")
	NBASSERT(_mutexLocksCount > 0)
	bool r = false;
	if(dst != NULL && _mutexLocksCount > 0){
		dst->agregar("{ ");
		SI32 qArrs = 0;
		//Notif received
		if(_notifsRcvd.conteo > 0){
			qArrs++; if(qArrs != 1) dst->agregar(" ,");
			dst->agregar(" \"notifsRcvd\": [");
			SI32 i; const SI32 count = _notifsRcvd.conteo;
			for(i = 0; i < count; i++){
				STAppNotif* data = _notifsRcvd.elemPtr(i);
				if(i != 0) dst->agregar(",");
				dst->agregar("{ ");
				dst->agregar(" \"uid\": \""); dst->agregarNumerico((SI32)data->uniqueId); dst->agregar("\"");
				dst->agregar(" , \"type\": \""); dst->agregarNumerico((SI32)data->type); dst->agregar("\"");
				dst->agregar(" , \"grpId\": \""); AUBase64::codificaBase64(data->grpId->str(), data->grpId->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"notifId\": \""); dst->agregarNumerico(data->notifId); dst->agregar("\"");
				dst->agregar(" , \"data\": \""); AUBase64::codificaBase64(data->data->str(), data->data->tamano(), dst); dst->agregar("\"");
				dst->agregar(" }");
			}
			dst->agregar("] ");
		}
		//Notif queue
		if(_notifsQueued.conteo > 0){
			qArrs++; if(qArrs != 1) dst->agregar(" ,");
			dst->agregar(" \"notifsQueue\": [");
			SI32 i; const SI32 count = _notifsQueued.conteo;
			for(i = 0; i < count; i++){
				STAppNotif* data = _notifsQueued.elemPtr(i);
				if(i != 0) dst->agregar(",");
				dst->agregar("{ ");
				dst->agregar(" \"uid\": \""); dst->agregarNumerico((SI32)data->uniqueId); dst->agregar("\"");
				dst->agregar(" , \"type\": \""); dst->agregarNumerico((SI32)data->type); dst->agregar("\"");
				dst->agregar(" , \"grpId\": \""); AUBase64::codificaBase64(data->grpId->str(), data->grpId->tamano(), dst); dst->agregar("\"");
				dst->agregar(" , \"notifId\": \""); dst->agregarNumerico(data->notifId); dst->agregar("\"");
				dst->agregar(" , \"data\": \""); AUBase64::codificaBase64(data->data->str(), data->data->tamano(), dst); dst->agregar("\"");
				dst->agregar(" }");
			}
			dst->agregar("]");
		}
		dst->agregar("}");
		//PRINTF_INFO("AUMngrNotifs, loading data to JSON: '%s'.\n", dst->str());
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrNotifs)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrNotifs, "AUMngrNotifs")
AUOBJMETODOS_CLONAR_NULL(AUMngrNotifs)
