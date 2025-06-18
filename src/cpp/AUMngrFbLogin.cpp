//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrFbLogin.h"
//
#include "nb/net/NBOAuthClient.h"
#include "nb/net/NBUrl.h"
#include "nb/core/NBThread.h"
#include "nb/core/NBJson.h"
#include "nb/crypto/NBBase64.h"
//
#include "NBMngrOSTools.h"
#include "NBMngrOSSecure.h"
//
#define NB_FB_FILE_NAME				"fbLogin.bin"
#define NB_FB_CYPHER_SALT			"NBFbLogin"
#define NB_FB_CYPHER_ITERATIONS		1230

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#	include "nb/NBObjCMethods.h"	//for "objc_msgSend", "sel_registerName", ...
//#	include <objc/message.h>	//for "objc_msgSend"
//#	include <objc/objc.h>		//for "sel_registerName"
#endif

typedef struct STMngrFbLoginState_ {
	STNBString		appClientId;
	STNBOAuthClient	oauth;
	BOOL			threadRuning;
} STMngrFbLoginState;

AUMngrFbLogin::AUMngrFbLogin(AUAppI* app) : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::AUMngrFbLogin")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrFbLogin")
	{
		this->_app	= app;
		this->_app->addAppOpenUrlListener(this);
	}
	{
		STMngrFbLoginState* data = (STMngrFbLoginState*)NBGestorMemoria::reservarMemoria(sizeof(STMngrFbLoginState), ENMemoriaTipo_General);
		NBString_init(&data->appClientId);
		data->threadRuning	= FALSE;
		NBOAuthClient_init(&data->oauth);
		_loginState = data;
	}
	NBHILO_MUTEX_INICIALIZAR(&_curUserMutex);
	{
		_curUser.sequential	= _listnrsUserSeq = -1;
		_curUser.userId		= NULL;
		_curUser.email		= NULL;
		_curUser.firstNames	= NULL;
		_curUser.lastNames	= NULL;
		_curUser.gender	= NULL;
		SI32 i; for(i = 0; i < ENFbPicSize_Count; i++){
			_curUser.pics[i].sequential = _listnrsUserPicsSeq[i] = -1;
			_curUser.pics[i].filepath = NULL;
		}
	}
	NBHILO_MUTEX_INICIALIZAR(&_listnrsNotifyingMutex)
	_listnrsNotifying	= false;
	_listeners			= new(this) AUArregloNativoMutableP<IFbLoginListener*>();
	_listenersToAdd		= new(this) AUArregloNativoMutableP<IFbLoginListener*>();
	_listenersToRemove	= new(this) AUArregloNativoMutableP<IFbLoginListener*>();
	NBGestorAnimadores::agregarAnimador(this, this);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrFbLogin::~AUMngrFbLogin(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::~AUMngrFbLogin")
	if(this->_app != NULL){
		this->_app->removeAppOpenUrlListener(this);
		this->_app = NULL;
	}
	if(this->_loginState != NULL){
		STMngrFbLoginState* data = (STMngrFbLoginState*)this->_loginState;
		//Wait for thread
		while(data->threadRuning){
			NBThread_mSleep(100);
		}
		//
		NBString_release(&data->appClientId);
		NBOAuthClient_release(&data->oauth);
		NBGestorMemoria::liberarMemoria(data);
		this->_loginState = NULL;
	}
	NBGestorAnimadores::quitarAnimador(this);
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	//Listeners
	{
		NBHILO_MUTEX_ACTIVAR(&_listnrsNotifyingMutex)
		if(_listenersToAdd != NULL){
			_listenersToAdd->vaciar();
			_listenersToAdd->liberar(NB_RETENEDOR_THIS);
			_listenersToAdd = NULL;
		}
		if(_listenersToRemove != NULL){
			_listenersToRemove->vaciar();
			_listenersToRemove->liberar(NB_RETENEDOR_THIS);
			_listenersToRemove = NULL;
		}
		if(_listeners != NULL){
			_listeners->vaciar();
			_listeners->liberar(NB_RETENEDOR_THIS);
			_listeners = NULL;
			_listnrsNotifying = false;
		}
		NBHILO_MUTEX_DESACTIVAR(&_listnrsNotifyingMutex)
		NBHILO_MUTEX_FINALIZAR(&_listnrsNotifyingMutex)
	}
	//User
	{
		_curUser.sequential = _listnrsUserSeq = -1;
		if(_curUser.userId != NULL) _curUser.userId->liberar(NB_RETENEDOR_THIS); _curUser.userId = NULL;
		if(_curUser.email != NULL) _curUser.email->liberar(NB_RETENEDOR_THIS); _curUser.email = NULL;
		if(_curUser.firstNames != NULL) _curUser.firstNames->liberar(NB_RETENEDOR_THIS); _curUser.firstNames = NULL;
		if(_curUser.lastNames != NULL) _curUser.lastNames->liberar(NB_RETENEDOR_THIS); _curUser.lastNames = NULL;
		if(_curUser.gender != NULL) _curUser.gender->liberar(NB_RETENEDOR_THIS); _curUser.gender = NULL;
		SI32 i; for(i = 0; i < ENFbPicSize_Count; i++){
			_curUser.pics[i].sequential = _listnrsUserPicsSeq[i] = -1;
			if(_curUser.pics[i].filepath != NULL) _curUser.pics[i].filepath->liberar(NB_RETENEDOR_THIS); _curUser.pics[i].filepath = NULL;
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	NBHILO_MUTEX_FINALIZAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrFbLogin::addListener(IFbLoginListener* itf){
	NBHILO_MUTEX_ACTIVAR(&_listnrsNotifyingMutex);
	if(_listnrsNotifying){
		//Add to queue (currently notifying other listeners)
		if(_listenersToRemove->indiceDe(itf) != -1){
			//cancel previous removing command
			_listenersToRemove->quitarElemento(itf);
		} else if(_listenersToAdd->indiceDe(itf) == -1){
			//queue to be added
			_listenersToAdd->agregarElemento(itf);
		}
	} else {
		//Add directly
		if(_listeners->indiceDe(itf) == -1){
			_listeners->agregarElemento(itf);
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&_listnrsNotifyingMutex);
	return true;
}

bool AUMngrFbLogin::removeListener(IFbLoginListener* itf){
	NBHILO_MUTEX_ACTIVAR(&_listnrsNotifyingMutex);
	if(_listnrsNotifying){
		//Add to queue (currently notifying other listeners)
		if(_listenersToAdd->indiceDe(itf) != -1){
			//cancel previous adding command
			_listenersToAdd->quitarElemento(itf);
		} else if(_listenersToRemove->indiceDe(itf) == -1){
			//queue to be removed
			_listenersToRemove->agregarElemento(itf);
		}
	} else {
		//Remove directly
		const SI32 idx = _listeners->indiceDe(itf);
		if(idx != -1){
			_listeners->quitarElementoEnIndice(idx);
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&_listnrsNotifyingMutex);
	return true;
}

//

bool AUMngrFbLogin::loadStoredData(){
	bool r = false;
	AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnLibreria(NB_FB_FILE_NAME), ENArchivoModo_SoloLectura, ENArchivoRetainMode_Retain);
	if(file != NULL){
		AUCadenaLargaMutable8* strCrypt = new(ENMemoriaTipo_Temporal) AUCadenaLargaMutable8();
		//Load file's content
		{
			char buff[4096];
			file->lock();
			while(1){
				const SI32 read = file->leer(buff, 1, sizeof(buff), file);
				if(read <= 0) break;
				strCrypt->agregar(buff, read);
			}
			file->unlock();
		}
		file->cerrar();
		file->liberar(NB_RETENEDOR_THIS);
		file = NULL;
		//Parse data
		{
			AUCadenaLargaMutable8* strJson = new(ENMemoriaTipo_Temporal) AUCadenaLargaMutable8();
			if(!NBMngrOSSecure::decWithGKey((const BYTE*)strCrypt->str(), strCrypt->tamano(), (const BYTE*)NB_FB_CYPHER_SALT, AUCadenaLarga8::tamano(NB_FB_CYPHER_SALT), NB_FB_CYPHER_ITERATIONS, strJson)){
				PRINTF_ERROR("FBLogin, unable to decrypt data.\n");
			} else {
				STNBJson json;
				NBJson_init(&json);
				if(!NBJson_loadFromStr(&json, strJson->str())){
					PRINTF_ERROR("FBLogin, unable to parse decrypted data.\n");
				} else {
					const STNBJsonNode* rootNode = NBJson_rootMember(&json);
					if(rootNode == NULL){
						PRINTF_ERROR("FBLogin, decrypted data has no root node.\n");
					} else {
						const STNBJsonNode* fbLogin = NBJson_childNode(&json, "fbLogin", rootNode, NULL); NBASSERT(fbLogin != NULL)
						if(fbLogin != NULL){
							//Auth
							const STNBJsonNode* auth = NBJson_childNode(&json, "auth", fbLogin, NULL);
							if(auth != NULL){
								const char* code = NBJson_childStr(&json, "code", NULL, auth, NULL); NBASSERT(code != NULL)
								STMngrFbLoginState* data = (STMngrFbLoginState*)this->_loginState;
								NBOAuthClient_setAuth(&data->oauth, code);
							}
							//profData
							const char* profData = NBJson_childStr(&json, "profData", NULL, fbLogin, NULL); NBASSERT(profData != NULL)
							if(profData != NULL){
								AUCadenaMutable8* strJson = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
								AUBase64::decodificaBase64(profData, AUCadenaLarga8::tamano(profData), strJson); NBASSERT(strJson->tamano() > 0)
								if(strJson->tamano() <= 0){
									PRINTF_ERROR("FBLogin, decrypted profData is empty.\n");
								} else {
									if(!this->parseProfData(strJson->str())){
										PRINTF_ERROR("FBLogin, decrypted profData could not be parsed.\n");
									} else {
										PRINTF_INFO("FBLogin, profData decrypted and loaded.\n");
										r = true;
									}
								}
								strJson->liberar(NB_RETENEDOR_THIS);
							}
						}
					}
				}
				NBJson_release(&json);
			}
			strJson->liberar(NB_RETENEDOR_THIS);
		}
		strCrypt->liberar(NB_RETENEDOR_THIS);
	}
	return r;
}

void AUMngrFbLogin::setConfig(const char* appClientId, const char* webClientId, const char* webSecret, const char* webRedirUri){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::setConfig")
	STMngrFbLoginState* data = (STMngrFbLoginState*)this->_loginState;
	//Configure app client
	NBString_set(&data->appClientId, appClientId);
	//Configure OAuthClient
	NBOAuthClient_setAuthServer(&data->oauth, "www.facebook.com", 443, TRUE, webRedirUri);
	NBOAuthClient_setAuthServerCmdAuth(&data->oauth, "/v2.12/dialog/oauth");
	NBOAuthClient_setTokenServer(&data->oauth, "graph.facebook.com", 443, TRUE, webRedirUri);
	NBOAuthClient_setTokenServerCmdToken(&data->oauth, "/v2.12/oauth/access_token");
	NBOAuthClient_setClient(&data->oauth, webClientId, webSecret);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}


bool AUMngrFbLogin::loginStart(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::loginStart")
	bool r = false;
	STMngrFbLoginState* data = (STMngrFbLoginState*)this->_loginState;
	//Try to authenticate using the facebook's app
	if(data->appClientId.length > 0){
		if(NBMngrOSTools::canOpenUrl("fbauth2://")){
			//PRINTF_INFO("FBApp is locally installed.\n");
			/*fbauth://authorize/
			 ?auth_type=rerequest
			 &client_id=638541819586758
			 &default_audience=friends
			 &display=touch
			 &e2e=%7B%22init%22%3A75508.961769040994%7D
			 &fbapp_pres=1
			 &legacy_override=v2.11
			 &redirect_uri=fbconnect%3A%2F%2Fsuccess
			 &response_type=token%2Csigned_request
			 &return_scopes=true
			 &scope=email%2Cpublic_profile
			 &sdk=ios
			 &sdk_version=4.31.1
			 &state=%7B%22challenge%22%3A%22cAxx4Jn2vAjIbIlqQ%252F0BWKEtXDo%253D%22%2C%220_auth_logger_id%22%3A%228A3AE95F-F1E8-4986-B6C8-26FC12C1B579%22%2C%22com.facebook.sdk_client_state%22%3Atrue%2C%223_method%22%3A%22fb_application_web_auth%22%7D*/
			STNBString strUrl;
			NBString_init(&strUrl);
			NBString_concat(&strUrl, "fbauth://authorize/");
			NBString_concat(&strUrl, "?auth_type=rerequest");
			NBString_concat(&strUrl, "&client_id="); NBString_concat(&strUrl, data->appClientId.str);
			NBString_concat(&strUrl, "&default_audience=friends");
			NBString_concat(&strUrl, "&display=touch");
			//"&e2e={"init":75508.961769040994}"
			//NBString_concat(&strUrl, "&e2e=%7B%22init%22%3A75508.961769040994%7D");
			NBString_concat(&strUrl, "&fbapp_pres=1");
			NBString_concat(&strUrl, "&legacy_override=v2.11");
			NBString_concat(&strUrl, "&redirect_uri=fbconnect%3A%2F%2Fsuccess");
			NBString_concat(&strUrl, "&response_type=token%2Csigned_request");
			NBString_concat(&strUrl, "&return_scopes=true");
			NBString_concat(&strUrl, "&scope=email%2Cpublic_profile");
			NBString_concat(&strUrl, "&sdk=ios");
			NBString_concat(&strUrl, "&sdk_version=4.31.1");
			//&state={"challenge":"cAxx4Jn2vAjIbIlqQ%2F0BWKEtXDo%3D","0_auth_logger_id":"8A3AE95F-F1E8-4986-B6C8-26FC12C1B579","com.facebook.sdk_client_state":true,"3_method":"fb_application_web_auth"}
			NBString_concat(&strUrl, "&state=%7B%22challenge%22%3A%22cAxx4Jn2vAjIbIlqQ%252F0BWKEtXDo%253D%22%2C%220_auth_logger_id%22%3A%228A3AE95F-F1E8-4986-B6C8-26FC12C1B579%22%2C%22com.facebook.sdk_client_state%22%3Atrue%2C%223_method%22%3A%22fb_application_web_auth%22%7D");
			r = NBMngrOSTools::openUrl(strUrl.str);
			NBString_release(&strUrl);
		}
	}
	//Try to authenticate using the web
	if(!r){
		STNBString strUrl;
		NBString_init(&strUrl);
		if(NBOAuthClient_getAuthURI(&data->oauth, "public_profile email", &strUrl)){
			PRINTF_INFO("FBLogin, opening URL: '%s'.\n", strUrl.str);
			r = NBMngrOSTools::openUrl(strUrl.str);
		}
		NBString_release(&strUrl);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrFbLogin::logout(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::logout")
	bool r = false;
	{
		//Delete file
		NBGestorArchivos::eliminarArchivo(NBGestorArchivos::rutaHaciaRecursoEnLibreria(NB_FB_FILE_NAME));
		//Unset auth and token
		STMngrFbLoginState* data = (STMngrFbLoginState*)this->_loginState;
		NBOAuthClient_setAuth(&data->oauth, NULL);
		NBOAuthClient_setTokenAndRefreshToken(&data->oauth, NULL, NULL, NULL);
		//
		this->setCurrentUser(NULL, NULL, NULL, NULL, NULL);
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrFbLogin::getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::getUser")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	if(dstSeq != NULL){
		*dstSeq = _curUser.sequential;
	}
	if(dstId != NULL){
		if(_curUser.userId != NULL) dstId->establecer(_curUser.userId->str());
		else dstId->vaciar();
	}
	if(dstEmail != NULL){
		if(_curUser.email != NULL) dstEmail->establecer(_curUser.email->str());
		else dstEmail->vaciar();
	}
	if(dstFirstNames != NULL){
		if(_curUser.firstNames != NULL) dstFirstNames->establecer(_curUser.firstNames->str());
		else dstFirstNames->vaciar();
	}
	if(dstLastNames != NULL){
		if(_curUser.lastNames != NULL) dstLastNames->establecer(_curUser.lastNames->str());
		else dstLastNames->vaciar();
	}
	if(dstGender != NULL){
		if(_curUser.gender != NULL) dstGender->establecer(_curUser.gender->str());
		else dstGender->vaciar();
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return true;
}

bool AUMngrFbLogin::getUserPic(const ENFbPicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::getUserPic")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	if(dstSeq != NULL){
		if(picSize < ENFbPicSize_Count) *dstSeq = _curUser.pics[picSize].sequential;
		else *dstSeq = 0;
	}
	if(dstPath != NULL){
		if(picSize >= ENFbPicSize_Count) dstPath->vaciar();
		else if(_curUser.pics[picSize].filepath != NULL) dstPath->establecer(_curUser.pics[picSize].filepath->str());
		else dstPath->vaciar();
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return true;
}

bool AUMngrFbLogin::setCurrentUser(const char* userId, const char* email, const char* firstNames, const char* lastNames, const char* gender){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::setCurrentUser")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	//Remove previous user
	{
		if(_curUser.userId != NULL) _curUser.userId->liberar(NB_RETENEDOR_THIS); _curUser.userId = NULL;
		if(_curUser.email != NULL) _curUser.email->liberar(NB_RETENEDOR_THIS); _curUser.email = NULL;
		if(_curUser.firstNames != NULL) _curUser.firstNames->liberar(NB_RETENEDOR_THIS); _curUser.firstNames = NULL;
		if(_curUser.lastNames != NULL) _curUser.lastNames->liberar(NB_RETENEDOR_THIS); _curUser.lastNames = NULL;
		if(_curUser.gender != NULL) _curUser.gender->liberar(NB_RETENEDOR_THIS); _curUser.gender = NULL;
		SI32 i; for(i = 0; i < ENFbPicSize_Count; i++){
			_curUser.pics[i].sequential = -1;
			if(_curUser.pics[i].filepath != NULL) _curUser.pics[i].filepath->liberar(NB_RETENEDOR_THIS); _curUser.pics[i].filepath = NULL;
		}
	}
	//Set new user
	if(userId != NULL){
		NBASSERT(userId[0] != '\0')
		if(userId[0] != '\0'){
			_curUser.userId			= new(this) AUCadena8(userId);
			_curUser.email			= new(this) AUCadena8(email != NULL ? email : "");
			_curUser.firstNames		= new(this) AUCadena8(firstNames != NULL ? firstNames : "");
			_curUser.lastNames		= new(this) AUCadena8(lastNames != NULL ? lastNames : "");
			_curUser.gender			= new(this) AUCadena8(gender != NULL ? gender : "");
			AUCadenaMutable8* dstPath = new(this) AUCadenaMutable8();
			SI32 i; for(i = 0; i < ENFbPicSize_Count; i++){
				dstPath->establecer(NBGestorArchivos::rutaHaciaRecursoEnCache("fbPics"));
				dstPath->agregar('/');
				AUBase64::codificaBase64(userId, AUCadena8::tamano(userId), dstPath);
				dstPath->agregar("_s.jpg");
				if(NULL != NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, dstPath->str(), ENArchivoModo_SoloLectura)){
					NBASSERT(_curUser.pics[i].sequential == -1)
					NBASSERT(_curUser.pics[i].filepath == NULL)
					_curUser.pics[i].sequential = 0;
					_curUser.pics[i].filepath = new(this) AUCadena8(dstPath->str());
				}
			}
			dstPath->liberar(NB_RETENEDOR_THIS);
		}
	}
	_curUser.sequential++;
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return true;
}

bool AUMngrFbLogin::setUserPicData(const char* userId, const ENFbPicSize picSize, const BYTE* picData, const SI32 picDataSize){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrFbLogin::setUserPicData")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	bool r = false;
	NBASSERT(userId != NULL)
	if(userId != NULL){
		NBASSERT(userId[0] != '\0')
		if(userId[0] != '\0'){
			if(picSize >= 0 && picSize < ENFbPicSize_Count){
				if(picData != NULL && picDataSize > 0){
					AUCadenaMutable8* dstPath = new(this) AUCadenaMutable8(NBGestorArchivos::rutaHaciaRecursoEnCache("fbPics"));
					if(!NBGestorArchivos::existeCarpeta(dstPath->str())){
						NBGestorArchivos::crearCarpeta(dstPath->str());
					}
					dstPath->agregar('/');
					AUBase64::codificaBase64(userId, AUCadena8::tamano(userId), dstPath);
					switch(picSize) {
						case ENFbPicSize_Small: dstPath->agregar("_s.jpg"); break;
						case ENFbPicSize_Normal: dstPath->agregar("_n.jpg"); break;
						case ENFbPicSize_Large: dstPath->agregar("_L.jpg"); break;
						default: NBASSERT(FALSE); dstPath->agregar(".jpg"); break;
					}
					AUArchivo* dstFile = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, dstPath->str(), ENArchivoModo_SoloEscritura, ENArchivoRetainMode_Retain); NBASSERT(dstFile != NULL)
					if(dstFile != NULL){
						dstFile->lock();
						dstFile->escribir((const void*)picData, 1, picDataSize, dstFile);
						dstFile->unlock();
						dstFile->cerrar();
						dstFile->liberar(NB_RETENEDOR_THIS);
						dstFile = NULL;
						//Update current user sequential
						if(_curUser.userId != NULL){
							if(_curUser.userId->esIgual(userId)){
								if(_curUser.pics[picSize].filepath != NULL) _curUser.pics[picSize].filepath->liberar(NB_RETENEDOR_THIS);
								_curUser.pics[picSize].sequential++;
								_curUser.pics[picSize].filepath = new(this) AUCadena8(dstPath->str());
							}
						}
						//
						r = true;
					}
					dstPath->liberar(NB_RETENEDOR_THIS);
				}
			}
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void AUMngrFbLogin::tickAnimacion(float segsTranscurridos){
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	//Notify listeners
	{
		//
		//Lock listeners array
		//
		NBHILO_MUTEX_ACTIVAR(&_listnrsNotifyingMutex);
		_listnrsNotifying = true;
		NBHILO_MUTEX_DESACTIVAR(&_listnrsNotifyingMutex);
		//
		//Process
		//
		if(_listnrsUserSeq != _curUser.sequential){
			//User changed
			SI32 i; for(i = _listeners->conteo - 1; i >= 0; i--){
				IFbLoginListener* itf = _listeners->elem(i);
				itf->fbLoginChanged(_curUser.sequential, (_curUser.userId == NULL ? NULL : _curUser.userId->str()), (_curUser.email == NULL ? NULL : _curUser.email->str()), (_curUser.firstNames == NULL ? NULL : _curUser.firstNames->str()), (_curUser.lastNames == NULL ? NULL : _curUser.lastNames->str()), (_curUser.gender == NULL ? NULL : _curUser.gender->str()));
				SI32 i; for(i = 0; i < ENFbPicSize_Count; i++){
					itf->fbLoginPicChanged((_curUser.userId == NULL ? NULL : _curUser.userId->str()), (_curUser.email == NULL ? NULL : _curUser.email->str()), (ENFbPicSize)i, _curUser.pics[i].sequential, (_curUser.pics[i].filepath == NULL ? NULL : _curUser.pics[i].filepath->str()));
				}
			}
			_listnrsUserSeq = _curUser.sequential;
			for(i = 0; i < ENFbPicSize_Count; i++){
				_listnrsUserPicsSeq[i] = _curUser.pics[i].sequential;
			}
		} else if(_curUser.userId != NULL){
			//User pic changed?
			SI32 i; for(i = 0; i < ENFbPicSize_Count; i++){
				if(_listnrsUserPicsSeq[i] != _curUser.pics[i].sequential){
					SI32 j; for(j = _listeners->conteo - 1; j >= 0; j--){
						IFbLoginListener* itf = _listeners->elem(j);
						itf->fbLoginPicChanged((_curUser.userId == NULL ? NULL : _curUser.userId->str()), (_curUser.email == NULL ? NULL : _curUser.email->str()), (ENFbPicSize)i, _curUser.pics[i].sequential, (_curUser.pics[i].filepath == NULL ? NULL : _curUser.pics[i].filepath->str()));
					}
					_listnrsUserPicsSeq[i] = _curUser.pics[i].sequential;
				}
			}
		}
		//
		//Process add/remove queues (listener added and removed during notification)
		//and unlock listeners array
		//
		NBHILO_MUTEX_ACTIVAR(&_listnrsNotifyingMutex);
		{
			//Process removeQueue
			{
				SI32 i; for(i = 0; i < _listenersToRemove->conteo; i++){
					_listeners->quitarElemento(_listenersToRemove->elem(i));
				}
				_listenersToRemove->vaciar();
			}
			//Process addQueue
			{
				SI32 i; for(i = 0; i < _listenersToAdd->conteo; i++){
					_listeners->agregarElemento(_listenersToAdd->elem(i));
				}
				_listenersToAdd->vaciar();
			}
		}
		_listnrsNotifying = false;
		NBHILO_MUTEX_DESACTIVAR(&_listnrsNotifyingMutex);
		//
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
}

bool AUMngrFbLogin::downloadUserImage(const char* userId, const char* imgType, const ENFbPicSize imgSize, STNBHttpClient* httpClient){
	bool r = false;
	STNBString strPath;
	NBString_init(&strPath);
	NBString_concat(&strPath, "/v2.12/");
	NBString_concat(&strPath, userId);
	NBString_concat(&strPath, "/picture");
	STNBHttpRequest req;
	NBHttpRequest_init(&req);
	NBHttpRequest_addParamGET(&req, "type", imgType);
	STNBHttpResponse* resp = NBHttpClient_executeSync(httpClient, "graph.facebook.com", 443, strPath.str, TRUE, &req, NULL, ENNBHttpRedirMode_FollowAllways);
	if(resp == NULL){
		PRINTF_ERROR("FBLogin, downloadUserImage('%s'/'%s') failed.\n", userId, imgType);
	} else if(resp->code != 200){
		PRINTF_ERROR("FBLogin, downloadUserImage('%s'/'%s') returned code '%d':\n'%s'\n'%s'.\n", userId, imgType, resp->code, resp->header.str, resp->body.str);
	} else if(resp->body.length == 0){
		PRINTF_ERROR("FBLogin, downloadUserImage('%s'/'%s') returned code '%d' and empty body.\n", userId, imgType, resp->code);
	} else {
		PRINTF_INFO("FBLogin, downloadUserImage('%s'/'%s') returned code '%d' (%d bytes body).\n", userId, imgType, resp->code, resp->body.length);
		this->setUserPicData(userId, imgSize, (const BYTE*)resp->body.str, resp->body.length);
	}
	NBHttpRequest_release(&req);
	NBString_release(&strPath);
	return r;
}

bool AUMngrFbLogin::parseProfData(const char* profData){
	bool r = false;
	STNBJson json;
	NBJson_init(&json);
	if(!NBJson_loadFromStr(&json, profData)){
		PRINTF_ERROR("FBLogin, user's public_profile data could not be parsed.\n");
	} else {
		const STNBJsonNode* rootNode = NBJson_rootMember(&json);
		if(rootNode == NULL){
			PRINTF_ERROR("FBLogin, user's public_profile data has no root node.\n");
		} else {
			//{"id":"10202453340670603","email":"email\u0040hotmail.com","name":"Marcos Ortega Morales","first_name":"Marcos","last_name":"Ortega Morales","gender":"male","picture":{"data":{"height":50,"is_silhouette":false,"url":"https:\/\/scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/22089150_10209902066324089_959700285998964546_n.jpg?oh=8993215cf3f50399190bed37ba6eae03&oe=5B0BA0AD","width":50}}}
			const char* userId		= NBJson_childStr(&json, "id", "", rootNode, NULL);
			const char* email		= NBJson_childStr(&json, "email", "", rootNode, NULL);
			const char* first_name	= NBJson_childStr(&json, "first_name", "", rootNode, NULL);
			const char* last_name	= NBJson_childStr(&json, "last_name", "", rootNode, NULL);
			const char* gender		= NBJson_childStr(&json, "gender", "", rootNode, NULL);
			//Set user
			this->setCurrentUser(userId, email, first_name, last_name, gender);
			//
			r = true;
		}
	}
	NBJson_release(&json);
	return r;
}

SI64 AUMngrFbLogin::runThreadLoadProf(STNBThread* thread, void* param){
	SI64 r = -1;
	//Request a new token
	AUMngrFbLogin* obj		= (AUMngrFbLogin*)param;
	STMngrFbLoginState* data = (STMngrFbLoginState*)obj->_loginState;
	{
		//Query user data
		STNBString strToken;
		NBString_init(&strToken);
		NBString_concat(&strToken, "Bearer" /*&data->oauth.strs.str[data->oauth.token.tokenType]*/);
		NBString_concat(&strToken, " ");
		NBString_concat(&strToken, &data->oauth.strs.str[data->oauth.token.accessToken]);
		STNBHttpClient httpClient;
		NBHttpClient_init(&httpClient);
		{
			STNBHttpRequest req;
			NBHttpRequest_init(&req);
			NBHttpRequest_addParamGET(&req, "fields", "id,email,name,first_name,last_name,gender,picture");
			NBHttpRequest_addHeader(&req, "Authorization", strToken.str);
			STNBHttpResponse* resp = NBHttpClient_executeSync(&httpClient, "graph.facebook.com", 443, "/v2.12/me", TRUE, &req, NULL, ENNBHttpRedirMode_None);
			if(resp == NULL){
				PRINTF_ERROR("FBLogin, could not request user's public_profile data.\n");
			} else if(resp->code != 200){
				PRINTF_ERROR("FBLogin, user's public_profile data request returned code(%d): '%s'.\n", resp->code, resp->body.str);
			} else {
				//{"id":"10202453340670603","email":"email\u0040hotmail.com","name":"Marcos Ortega Morales","first_name":"Marcos","last_name":"Ortega Morales","gender":"male","picture":{"data":{"height":50,"is_silhouette":false,"url":"https:\/\/scontent.xx.fbcdn.net\/v\/t1.0-1\/p50x50\/22089150_10209902066324089_959700285998964546_n.jpg?oh=8993215cf3f50399190bed37ba6eae03&oe=5B0BA0AD","width":50}}}
				if(!obj->parseProfData(resp->body.str)){
					PRINTF_ERROR("FBLogin, could not parse user's public_profile data.\n");
				} else {
					//Save data
					{
						AUCadenaLargaMutable8* strCrypt = new(ENMemoriaTipo_Temporal) AUCadenaLargaMutable8();
						AUCadenaMutable8* strJson = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
						strJson->agregar("{");
						strJson->agregar("\"fbLogin\":{");
						strJson->agregar("\"auth\":{\"code\":\""); strJson->agregar(&data->oauth.strs.str[data->oauth.auth.code]); strJson->agregar("\"}");
						strJson->agregar(", \"profData\":\""); AUBase64::codificaBase64(resp->body.str, resp->body.length, strJson); strJson->agregar("\"");
						strJson->agregar("}");
						strJson->agregar("}");
						if(!NBMngrOSSecure::encWithGKey((const BYTE*)strJson->str(), strJson->tamano(), (const BYTE*)NB_FB_CYPHER_SALT, AUCadenaLarga8::tamano(NB_FB_CYPHER_SALT), NB_FB_CYPHER_ITERATIONS, strCrypt)){
							PRINTF_ERROR("FBLogin, unable to encrypt data.\n");
						} else {
							AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnLibreria(NB_FB_FILE_NAME), ENArchivoModo_SoloEscritura, ENArchivoRetainMode_Retain);
							if(file == NULL){
								PRINTF_ERROR("FBLogin, unable to write to local file.\n");
							} else {
								file->lock();
								if(file->escribir(strCrypt->str(), strCrypt->tamano(), 1, file) != 1){
									PRINTF_ERROR("FBLogin, unable to write to local file (%d bytes).\n", strCrypt->tamano());
								} else {
									PRINTF_INFO("FBLogin, data saved to local file (%d bytes).\n", strCrypt->tamano());
								}
								file->unlock();
								file->cerrar();
								file->liberar(NB_RETENEDOR_NULL);
								file = NULL;
							}
						}
						strJson->liberar(NB_RETENEDOR_NULL);
						strCrypt->liberar(NB_RETENEDOR_NULL);
					}
					//Download pics
					{
						//Small pic (32 x 32)
						obj->downloadUserImage(obj->_curUser.userId->str(), "small", ENFbPicSize_Small, &httpClient);
						//Normal pic (50 x 50)
						obj->downloadUserImage(obj->_curUser.userId->str(), "normal", ENFbPicSize_Normal, &httpClient);
						//Large pic (not fixed dimensions)
						obj->downloadUserImage(obj->_curUser.userId->str(), "large", ENFbPicSize_Large, &httpClient);
					}
					r = 0;
				}
			}
			NBHttpRequest_release(&req);
		}
		NBHttpClient_release(&httpClient);
		NBString_release(&strToken);
	}
	//Release thread
	if(thread != NULL){
		NBThread_release(thread);
		NBMemory_free(thread);
		thread = NULL;
	}
	//
	return r;
}

SI64 AUMngrFbLogin::runThreadAuth(STNBThread* thread, void* param){
	SI64 r = -1;
	//Request a new token
	AUMngrFbLogin* obj		= (AUMngrFbLogin*)param;
	STMngrFbLoginState* data = (STMngrFbLoginState*)obj->_loginState;
	{
		if(!NBOAuthClient_getToken(&data->oauth)){
			PRINTF_ERROR("FBLogin, could not get token.\n");
		} else {
			if(AUMngrFbLogin::runThreadLoadProf(NULL, param) == 0){
				r = 0;
			}
		}
		//Manage login error
		if(r != 0){
			obj->setCurrentUser(NULL, NULL, NULL, NULL, NULL);
			//TEMPORAL: para poder usar el APPsin internet
			//NBGestorRefranero::usuarioAsegurarDatosActual(ENUsuarioLoginOrigen_Facebook, "123", "marcosjom@hotmail.com", "Chipote", "Chillón", "male");
			//NBGestorRefranero::usuarioActualEstablecerEstadoLogeado(ENUsuarioLoginOrigen_Facebook, true);
		}
	}
	//Release thread
	if(thread != NULL){
		NBThread_release(thread);
		NBMemory_free(thread);
		thread = NULL;
	}
	//
	return r;
}

bool AUMngrFbLogin::appOpenUrl(AUAppI* app, const STNBUrl* url, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	if(url) {
		STMngrFbLoginState* data = (STMngrFbLoginState*)this->_loginState;
		//PRINTF_INFO("FBLogin, received: '%s'.\n", plainUrl);
		//Consume "com.app.id://" URL
		{
			AUCadenaMutable8* myScheme = new AUCadenaMutable8();
			NBMngrOSTools::getPkgIdentifier(myScheme);
			if(NBString_strIsEqual(NBUrl_getScheme(url, NULL), myScheme->str())){
				const char* host = NBUrl_getHost(url, NULL);
				if(NBString_strIsEqual(host, "fbAuth")){
					const char* codeStr = NBUrl_getQueryValue(url, "code", NULL);
					if(codeStr != NULL){
						NBOAuthClient_setAuth(&data->oauth, codeStr);
						//Start token negotiation
						NBASSERT(!data->threadRuning)
						STNBThread* thread = NBMemory_allocType(STNBThread);
						NBThread_init(thread);
						NBThread_setIsJoinable(thread, FALSE);
						data->threadRuning = TRUE;
						if(!NBThread_start(thread, AUMngrFbLogin::runThreadAuth, this, NULL)){
							data->threadRuning	= FALSE;
							NBThread_release(thread);
							NBMemory_free(thread);
							thread = NULL;
						} else {
							r = TRUE;
						}
					}
				}
			}
			myScheme->liberar(NB_RETENEDOR_THIS);
		}
		//Consume "fb123456://" URL
		if(!r){
			/*
			 fb638541819586758://authorize
			 #access_token=...
			 &denied_scopes=
			 &e2e=%7B%22init%22%3A75508.961769040994%7D
			 &expires_in=5143871
			 &granted_scopes=email%2Cpublic_profile
			 &signed_request=hon9VTNgqkLbYKpay1jLhjxARfeXuDRut1xhfVh9zmw.eyJhbGdvcml0aG0iOiJITUFDLVNIQTI1NiIsImNvZGUiOiJBUUNmOHBobWg3NkNFVmhHLTI3NEJDNV9kSldjTVlBMHd1T0RZWVo3RjBERjZkaWd1ekNQT0FPNDRLa250WDgzWHN1bml5eDZvSUxZS0tleUdicExnTkZQbXN3S1ZXTDVNWk1nRUtJOWkzcnA2NWpGQmRzalBIMHRvOXhIUnJYamNRMnJSREsxYWpmNC04eVpoUU1uR3NrRGlIUVR1WUpHQlphMEJPYjVUX25USkRRbnIzUER3NVRSem41WkF4Qjl0RmNZbjA4d25OZFFDV2Z6QXlFY083enNpSnlQRFh1R2RjTFZZa1BSNlpBYnBidkF6blNvWktMelBrcXA3V2xyUFpaNVBoNzhyZmlVNWVhNnF2MDM5ZEtrNHN4VU9uTHVRUVNRMXlRSHNUcWdWUlF4M3FvLTNQMUQ0WU1SU1dKZEFZcjRONjFId2xLcXpvYnJNakdUWTd6TXl2NkJLUklha3A0UUxPQmhmaFBQREc5S0M1c3ZoYm1VYVJWLW9aTDczNjAiLCJpc3N1ZWRfYXQiOjE1MjE2NTU4NDUsInVzZXJfaWQiOiIxMDIwMjQ1MzM0MDY3MDYwMyJ9
			 &state=%7B%22challenge%22%3A%22cAxx4Jn2vAjIbIlqQ%252F0BWKEtXDo%253D%22%2C%220_auth_logger_id%22%3A%228A3AE95F-F1E8-4986-B6C8-26FC12C1B579%22%2C%22com.facebook.sdk_client_state%22%3Atrue%2C%223_method%22%3A%22fb_application_web_auth%22%7D'.
			 */
			AUCadenaMutable8* myScheme = new AUCadenaMutable8();
			myScheme->agregar("fb");
			myScheme->agregar(data->appClientId.str);
			if(NBString_strIsEqual(NBUrl_getScheme(url, NULL), myScheme->str())){
				const char* host = NBUrl_getHost(url, NULL);
				if(NBString_strIsEqual(host, "authorize")){
					const char* access_token	= NBUrl_getFragmentValue(url, "access_token", "");
					//const char* denied_scopes	= NBUrl_getFragmentValue(url, "denied_scopes", "");
					//const char* e2e			= NBUrl_getFragmentValue(url, "e2e", "");
					//const char* expires_in	= NBUrl_getFragmentValue(url, "expires_in", "");
					//const char* granted_scopes= NBUrl_getFragmentValue(url, "granted_scopes", "");
					const char* signed_request	= NBUrl_getFragmentValue(url, "signed_request", "");
					//const char* state			= NBUrl_getFragmentValue(url, "state", "");
					if(access_token[0] == '\0' || signed_request[0] == '\0'){
						PRINTF_ERROR("FBLogin, fbApp did not returned 'access_token' or 'signed_request'.\n")
					} else {
						//signed_request must contain a "signature.jsonData" in base 64
						/*jsonData = {
						 "algorithm":"HMAC-SHA256"
						 ,"code":"AQCf8phmh76CEVhG-274BC5_dJWcMYA0wuODYYZ7F0DF6diguzCPOAO44KkntX83Xsuniyx6oILYKKeyGbpLgNFPmswKVWL5MZMgEKI9i3rp65jFBdsjPH0to9xHRrXjcQ2rRDK1ajf4-8yZhQMnGskDiHQTuYJGBZa0BOb5T_nTJDQnr3PDw5TRzn5ZAxB9tFcYn08wnNdQCWfzAyEcO7zsiJyPDXuGdcLVYkPR6ZAbpbvAznSoZKLzPkqp7WlrPZZ5Ph78rfiU5ea6qv039dKk4sxUOnLuQQSQ1yQHsTqgVRQx3qo-3P1D4YMRSWJdAYr4N61HwlKqzobrMjGTY7zMyv6BKRIakp4QLOBhfhPPDG9KC5svhbmUaRV-oZL7360"
						 ,"issued_at":1521655845
						 ,"user_id":"10202453340670603"
						 }*/
						const SI32 sepPos = NBString_strIndexOf(signed_request, ".", 0);
						if(sepPos < 0){
							PRINTF_ERROR("FBLogin, signed_request has no separator '.'.\n");
						} else {
							SI32 endPos = NBString_strIndexOf(signed_request, ".", sepPos + 1); if(endPos < 0) endPos = NBString_strLenBytes(signed_request);
							STNBString strJson;
							NBString_init(&strJson);
							NBBase64_decodeBytes(&strJson, &signed_request[sepPos + 1], (endPos - sepPos - 1));
							STNBJson json;
							NBJson_init(&json);
							if(!NBJson_loadFromStr(&json, strJson.str)){
								PRINTF_ERROR("FBLogin, signed_request's json could not be parsed.\n");
							} else {
								const STNBJsonNode* rootNode = NBJson_rootMember(&json);
								if(rootNode == NULL){
									PRINTF_ERROR("FBLogin, signed_request's json has not root node.\n");
								} else {
									//const char* algorithm	= NBJson_childStr(&json, "algorithm", NULL, rootNode, NULL);
									const char* code		= NBJson_childStr(&json, "code", NULL, rootNode, NULL);
									//const char* issued_at	= NBJson_childStr(&json, "issued_at", NULL, rootNode, NULL);
									//const char* user_id	= NBJson_childStr(&json, "user_id", NULL, rootNode, NULL);
									if(code == NULL){
										PRINTF_ERROR("FBLogin, signed_request's json has not 'code' node.\n");
									} else {
										NBOAuthClient_setAuth(&data->oauth, code);
										NBOAuthClient_setToken(&data->oauth, "bearer", access_token);
										//Start user profile load
										NBASSERT(!data->threadRuning)
										STNBThread* thread = NBMemory_allocType(STNBThread);
										NBThread_init(thread);
										NBThread_setIsJoinable(thread, FALSE);
										data->threadRuning = TRUE;
										if(!NBThread_start(thread, AUMngrFbLogin::runThreadLoadProf, this, NULL)){
											data->threadRuning	= FALSE;
											NBThread_release(thread);
											NBMemory_free(thread);
											thread = NULL;
										} else {
											r = true;
										}
									}
								}
							}
							NBJson_release(&json);
							NBString_release(&strJson);
						}
					}
				}
			}
			myScheme->liberar(NB_RETENEDOR_THIS);
		}
	}
	return r;
}

bool AUMngrFbLogin::appOpenUrlImage(AUAppI* app, const STNBUrl* url, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	return false;
}

bool AUMngrFbLogin::appOpenFileData(AUAppI* app, const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	return r;
}

bool AUMngrFbLogin::appOpenFileImageData(AUAppI* app, const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	return false;
}

bool AUMngrFbLogin::appOpenBitmap(AUAppI* app, const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrFbLogin)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrFbLogin, "AUMngrFbLogin")
AUOBJMETODOS_CLONAR_NULL(AUMngrFbLogin)
