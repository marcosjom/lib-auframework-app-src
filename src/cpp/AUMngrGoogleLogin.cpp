//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrGoogleLogin.h"
//
#include "nb/core/NBThread.h"
#include "nb/core/NBJson.h"
#include "nb/net/NBOAuthClient.h"
#include "nb/net/NBUrl.h"
//
#include "NBMngrOSTools.h"
#include "NBMngrOSSecure.h"
//
#define NB_GOOGLE_FILE_NAME				"googleLogin.bin"
#define NB_GOOGLE_CYPHER_SALT			"NBGoogleLogin"
#define NB_GOOGLE_CYPHER_ITERATIONS		1020

typedef struct STMngrGoogleLoginState_ {
	STNBOAuthClient	oauth;
	BOOL			threadRuning;
} STMngrGoogleLoginState;

AUMngrGoogleLogin::AUMngrGoogleLogin(AUAppI* app) : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::AUMngrGoogleLogin")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrGoogleLogin")
	{
		this->_app	= app;
		this->_app->addAppOpenUrlListener(this);
	}
	{
		STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)NBGestorMemoria::reservarMemoria(sizeof(STMngrGoogleLoginState), ENMemoriaTipo_General);
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
		_curUser.gender		= NULL;
		_curUser.picUri		= NULL;
		SI32 i; for(i = 0; i < ENGooglePicSize_Count; i++){
			_curUser.pics[i].sequential = _listnrsUserPicsSeq[i] = -1;
			_curUser.pics[i].filepath = NULL;
		}
	}
	NBHILO_MUTEX_INICIALIZAR(&_listnrsNotifyingMutex)
	_listnrsNotifying	= false;
	_listeners			= new(this) AUArregloNativoMutableP<IGoogleLoginListener*>();
	_listenersToAdd		= new(this) AUArregloNativoMutableP<IGoogleLoginListener*>();
	_listenersToRemove	= new(this) AUArregloNativoMutableP<IGoogleLoginListener*>();
	NBGestorAnimadores::agregarAnimador(this, this);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrGoogleLogin::~AUMngrGoogleLogin(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::~AUMngrGoogleLogin")
	if(this->_app != NULL){
		this->_app->removeAppOpenUrlListener(this);
		this->_app = NULL;
	}
	if(this->_loginState != NULL){
		STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)this->_loginState;
		//Wait for thread
		while(data->threadRuning){
			NBThread_mSleep(100);
		}
		//
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
		if(_curUser.picUri != NULL) _curUser.picUri->liberar(NB_RETENEDOR_THIS); _curUser.picUri = NULL;
		SI32 i; for(i = 0; i < ENGooglePicSize_Count; i++){
			_curUser.pics[i].sequential = _listnrsUserPicsSeq[i] = -1;
			if(_curUser.pics[i].filepath != NULL) _curUser.pics[i].filepath->liberar(NB_RETENEDOR_THIS); _curUser.pics[i].filepath = NULL;
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	NBHILO_MUTEX_FINALIZAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrGoogleLogin::addListener(IGoogleLoginListener* itf){
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

bool AUMngrGoogleLogin::removeListener(IGoogleLoginListener* itf){
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

bool AUMngrGoogleLogin::loadStoredData(){
	bool r = false;
	AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnLibreria(NB_GOOGLE_FILE_NAME), ENArchivoModo_SoloLectura, ENArchivoRetainMode_Retain);
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
			if(!NBMngrOSSecure::decWithGKey((const BYTE*)strCrypt->str(), strCrypt->tamano(), (const BYTE*)NB_GOOGLE_CYPHER_SALT, AUCadenaLarga8::tamano(NB_GOOGLE_CYPHER_SALT), NB_GOOGLE_CYPHER_ITERATIONS, strJson)){
				PRINTF_ERROR("GoogleLogin, unable to decrypt data.\n");
			} else {
				STNBJson json;
				NBJson_init(&json);
				if(!NBJson_loadFromStr(&json, strJson->str())){
					PRINTF_ERROR("GoogleLogin, unable to parse decrypted data.\n");
				} else {
					const STNBJsonNode* rootNode = NBJson_rootMember(&json);
					if(rootNode == NULL){
						PRINTF_ERROR("GoogleLogin, decrypted data has no root node.\n");
					} else {
						const STNBJsonNode* googleLogin = NBJson_childNode(&json, "googleLogin", rootNode, NULL); NBASSERT(googleLogin != NULL)
						if(googleLogin != NULL){
							//Auth
							const STNBJsonNode* auth = NBJson_childNode(&json, "auth", googleLogin, NULL);
							if(auth != NULL){
								const char* code = NBJson_childStr(&json, "code", NULL, auth, NULL); NBASSERT(code != NULL)
								STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)this->_loginState;
								NBOAuthClient_setAuth(&data->oauth, code);
							}
							//profData
							const char* profData = NBJson_childStr(&json, "profData", NULL, googleLogin, NULL); NBASSERT(profData != NULL)
							if(profData != NULL){
								AUCadenaMutable8* strJson = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
								AUBase64::decodificaBase64(profData, AUCadenaLarga8::tamano(profData), strJson); NBASSERT(strJson->tamano() > 0)
								if(strJson->tamano() <= 0){
									PRINTF_ERROR("GoogleLogin, decrypted profData is empty.\n");
								} else {
									if(!this->parseProfData(strJson->str())){
										PRINTF_ERROR("GoogleLogin, decrypted profData could not be parsed.\n");
									} else {
										PRINTF_INFO("GoogleLogin, profData decrypted and loaded.\n");
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

void AUMngrGoogleLogin::setOAuthConfig(const char* clientId, const char* secret, const char* redirUri){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::setOAuthConfig")
	STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)this->_loginState;
	NBOAuthClient_setAuthServer(&data->oauth, "accounts.google.com", 443, TRUE, redirUri);
	NBOAuthClient_setAuthServerCmdAuth(&data->oauth, "/o/oauth2/v2/auth");
	NBOAuthClient_setTokenServer(&data->oauth, "www.googleapis.com", 443, TRUE, redirUri);
	NBOAuthClient_setTokenServerCmdToken(&data->oauth, "/oauth2/v4/token");
	NBOAuthClient_setClient(&data->oauth, clientId, secret);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool AUMngrGoogleLogin::loginStart(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::loginStart")
	bool r = false;
	{
		STNBString strUrl;
		NBString_init(&strUrl);
		STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)this->_loginState;
		if(NBOAuthClient_getAuthURI(&data->oauth, "profile email", &strUrl)){
			PRINTF_INFO("GoogleLogin, opening URL: '%s'.\n", strUrl.str);
			r = NBMngrOSTools::openUrl(strUrl.str);
		}
		NBString_release(&strUrl);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGoogleLogin::logout(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::logout")
	bool r = false;
	{
		//Delete file
		NBGestorArchivos::eliminarArchivo(NBGestorArchivos::rutaHaciaRecursoEnLibreria(NB_GOOGLE_FILE_NAME));
		//Unset auth and token
		STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)this->_loginState;
		NBOAuthClient_setAuth(&data->oauth, NULL);
		NBOAuthClient_setTokenAndRefreshToken(&data->oauth, NULL, NULL, NULL);
		//
		this->setCurrentUser(NULL, NULL, NULL, NULL, NULL, NULL);
		r = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrGoogleLogin::getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::getUser")
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

bool AUMngrGoogleLogin::getUserPic(const ENGooglePicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::getUserPic")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	if(dstSeq != NULL){
		if(picSize < ENGooglePicSize_Count) *dstSeq = _curUser.pics[picSize].sequential;
		else *dstSeq = 0;
	}
	if(dstPath != NULL){
		if(picSize >= ENGooglePicSize_Count) dstPath->vaciar();
		else if(_curUser.pics[picSize].filepath != NULL) dstPath->establecer(_curUser.pics[picSize].filepath->str());
		else dstPath->vaciar();
	}
	NBHILO_MUTEX_DESACTIVAR(&_curUserMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return true;
}

bool AUMngrGoogleLogin::setCurrentUser(const char* userId, const char* email, const char* firstNames, const char* lastNames, const char* gender, const char* picUri){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::setCurrentUser")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	//Remove previous user
	{
		if(_curUser.userId != NULL) _curUser.userId->liberar(NB_RETENEDOR_THIS); _curUser.userId = NULL;
		if(_curUser.email != NULL) _curUser.email->liberar(NB_RETENEDOR_THIS); _curUser.email = NULL;
		if(_curUser.firstNames != NULL) _curUser.firstNames->liberar(NB_RETENEDOR_THIS); _curUser.firstNames = NULL;
		if(_curUser.lastNames != NULL) _curUser.lastNames->liberar(NB_RETENEDOR_THIS); _curUser.lastNames = NULL;
		if(_curUser.gender != NULL) _curUser.gender->liberar(NB_RETENEDOR_THIS); _curUser.gender = NULL;
		if(_curUser.picUri != NULL) _curUser.picUri->liberar(NB_RETENEDOR_THIS); _curUser.picUri = NULL;
		SI32 i; for(i = 0; i < ENGooglePicSize_Count; i++){
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
			_curUser.picUri			= new(this) AUCadena8(picUri != NULL ? picUri : "");
			AUCadenaMutable8* dstPath = new(this) AUCadenaMutable8();
			SI32 i; for(i = 0; i < ENGooglePicSize_Count; i++){
				dstPath->establecer(NBGestorArchivos::rutaHaciaRecursoEnCache("googlePics"));
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

bool AUMngrGoogleLogin::setUserPicData(const char* userId, const ENGooglePicSize picSize, const BYTE* picData, const SI32 picDataSize){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrGoogleLogin::setUserPicData")
	NBHILO_MUTEX_ACTIVAR(&_curUserMutex);
	bool r = false;
	NBASSERT(userId != NULL)
	if(userId != NULL){
		NBASSERT(userId[0] != '\0')
		if(userId[0] != '\0'){
			if(picSize >= 0 && picSize < ENGooglePicSize_Count){
				if(picData != NULL && picDataSize > 0){
					AUCadenaMutable8* dstPath = new(this) AUCadenaMutable8(NBGestorArchivos::rutaHaciaRecursoEnCache("googlePics"));
					if(!NBGestorArchivos::existeCarpeta(dstPath->str())){
						NBGestorArchivos::crearCarpeta(dstPath->str());
					}
					dstPath->agregar('/');
					AUBase64::codificaBase64(userId, AUCadena8::tamano(userId), dstPath);
					switch(picSize) {
						case ENGooglePicSize_Small: dstPath->agregar("_s.jpg"); break;
						case ENGooglePicSize_Normal: dstPath->agregar("_n.jpg"); break;
						case ENGooglePicSize_Large: dstPath->agregar("_L.jpg"); break;
						default: NBASSERT(FALSE); dstPath->agregar(".jpg"); break;
					}
					AUArchivo* dstFile = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, dstPath->str(), ENArchivoModo_SoloEscritura); NBASSERT(dstFile != NULL)
					if(dstFile != NULL){
						dstFile->lock();
						dstFile->escribir((const void*)picData, 1, picDataSize, dstFile);
						dstFile->unlock();
						dstFile->cerrar();
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

void AUMngrGoogleLogin::tickAnimacion(float segsTranscurridos){
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
				IGoogleLoginListener* itf = _listeners->elem(i);
				itf->googleLoginChanged(_curUser.sequential, (_curUser.userId == NULL ? NULL : _curUser.userId->str()), (_curUser.email == NULL ? NULL : _curUser.email->str()), (_curUser.firstNames == NULL ? NULL : _curUser.firstNames->str()), (_curUser.lastNames == NULL ? NULL : _curUser.lastNames->str()), (_curUser.gender == NULL ? NULL : _curUser.gender->str()));
				SI32 i; for(i = 0; i < ENGooglePicSize_Count; i++){
					itf->googleLoginPicChanged((_curUser.userId == NULL ? NULL : _curUser.userId->str()), (_curUser.email == NULL ? NULL : _curUser.email->str()), (ENGooglePicSize)i, _curUser.pics[i].sequential, (_curUser.pics[i].filepath == NULL ? NULL : _curUser.pics[i].filepath->str()));
				}
			}
			_listnrsUserSeq = _curUser.sequential;
			for(i = 0; i < ENGooglePicSize_Count; i++){
				_listnrsUserPicsSeq[i] = _curUser.pics[i].sequential;
			}
		} else if(_curUser.userId != NULL){
			//User pic changed?
			SI32 i; for(i = 0; i < ENGooglePicSize_Count; i++){
				if(_listnrsUserPicsSeq[i] != _curUser.pics[i].sequential){
					SI32 j; for(j = _listeners->conteo - 1; j >= 0; j--){
						IGoogleLoginListener* itf = _listeners->elem(j);
						itf->googleLoginPicChanged((_curUser.userId == NULL ? NULL : _curUser.userId->str()), (_curUser.email == NULL ? NULL : _curUser.email->str()), (ENGooglePicSize)i, _curUser.pics[i].sequential, (_curUser.pics[i].filepath == NULL ? NULL : _curUser.pics[i].filepath->str()));
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

bool AUMngrGoogleLogin::appOpenUrl(AUAppI* app, const STNBUrl* url, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	if(url) {
		STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)this->_loginState;
		//PRINTF_INFO("GoogleLogin, received: '%s'.\n", plainUrl);
		AUCadenaMutable8* myScheme = new AUCadenaMutable8();
		NBMngrOSTools::getPkgIdentifier(myScheme);
		if(NBString_strIsEqual(NBUrl_getScheme(url, NULL), myScheme->str())){
			if(NBString_strIsEqual(NBUrl_getHost(url, NULL), "googleAuth")){
				const char* codeStr = NBUrl_getQueryValue(url, "code", NULL);
				if(codeStr != NULL){
					NBOAuthClient_setAuth(&data->oauth, codeStr);
					//Start token negotiation
					NBASSERT(!data->threadRuning)
					STNBThread* thread = NBMemory_allocType(STNBThread);
					NBThread_init(thread);
					NBThread_setIsJoinable(thread, FALSE);
					data->threadRuning = TRUE;
					if(!NBThread_start(thread, AUMngrGoogleLogin::runThreadAuth, this, NULL)){
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
		myScheme->liberar(NB_RETENEDOR_THIS);
	}
	return r;
}

bool AUMngrGoogleLogin::appOpenUrlImage(AUAppI* app, const STNBUrl* url, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	return false;
}

bool AUMngrGoogleLogin::appOpenFileData(AUAppI* app, const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz){
	return false;
}

bool AUMngrGoogleLogin::appOpenFileImageData(AUAppI* app, const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	return false;
}

bool AUMngrGoogleLogin::appOpenBitmap(AUAppI* app, const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	return false;
}

bool AUMngrGoogleLogin::downloadUserImage(const char* userId, const char* picUri, const char* picSzStr, const ENGooglePicSize imgSize, STNBHttpClient* httpClient){
	bool r = false;
	//https://lh4.googleusercontent.com/-x2iVF3AjOBU/AAAAAAAAAAI/AAAAAAAAAGE/ABCDEFGHIJ/photo.jpg
	const SI32 iPosProtocol = NBString_strIndexOf(picUri, "://", 0);
	if(iPosProtocol > 0){
		STNBString strServer, strPath;
		NBString_init(&strServer);
		NBString_init(&strPath);
		const SI32 iPosRes = NBString_strIndexOf(picUri, "/", iPosProtocol + 3);
		if(iPosRes == -1){
			NBString_concat(&strServer, &picUri[iPosProtocol + 3]);
			NBString_concat(&strPath, "/");
		} else {
			NBString_concatBytes(&strServer, &picUri[iPosProtocol + 3], iPosRes - iPosProtocol - 3);
			NBString_concat(&strPath, &picUri[iPosRes]);
		}
		STNBHttpRequest req;
		NBHttpRequest_init(&req);
		NBHttpRequest_addParamGET(&req, "sz", picSzStr);
		NBHttpRequest_addParamGET(&req, "type", "square");
		STNBHttpResponse* resp = NBHttpClient_executeSync(httpClient, strServer.str, 443, strPath.str, TRUE, &req, NULL, ENNBHttpRedirMode_FollowAllways);
		if(resp == NULL){
			PRINTF_ERROR("GoogleLogin, could not request user's pic('%s') data.\n", picSzStr);
		} else if(resp->code != 200){
			PRINTF_ERROR("GoogleLogin, user's pic('%s') data request returned code(%d): '%s'.\n", picSzStr, resp->code, resp->body.str);
		} else if(resp->body.length == 0){
			PRINTF_ERROR("GoogleLogin, user's pic('%s') data request returned empty body.\n", picSzStr);
		} else {
			PRINTF_INFO("GoogleLogin, getUserImage('%s'/'%s') returned code '%d' (%d bytes body).\n", userId, picSzStr, resp->code, resp->body.length);
			this->setUserPicData(userId, imgSize, (const BYTE*)resp->body.str, resp->body.length);
		}
		NBHttpRequest_release(&req);
		NBString_release(&strServer);
		NBString_release(&strPath);
	}
	return r;
}

bool AUMngrGoogleLogin::parseProfData(const char* profData){
	bool r = false;
	/*{
	 "id": "123456789123456789123",
	 "email": "email@gmail.com",
	 "verified_email": true,
	 "name": "Marcos Ortega",
	 "given_name": "Marcos",
	 "family_name": "Ortega",
	 "link": "https://plus.google.com/123456789123456789123",
	 "picture": "https://lh4.googleusercontent.com/-x2iVF3AjOBU/AAAAAAAAAAI/AAAAAAAAAGE/ABCDEFGHIJ/photo.jpg",
	 "gender": "male",
	 "locale": "es-419"
	 }*/
	STNBJson json;
	NBJson_init(&json);
	if(!NBJson_loadFromStr(&json, profData)){
		PRINTF_ERROR("GoogleLogin, user's public_profile data could not be parsed: '%s'.\n", profData);
	} else {
		const STNBJsonNode* rootNode = NBJson_rootMember(&json);
		if(rootNode == NULL){
			PRINTF_ERROR("GoogleLogin, user's public_profile data has no root node: '%s'.\n", profData);
		} else {
			//PRINTF_INFO("GoogleLogin, returned: '%s'.\n", resp->body.str);
			const char* userId		= NBJson_childStr(&json, "id", "", rootNode, NULL);
			const char* email		= NBJson_childStr(&json, "email", "", rootNode, NULL);
			const char* given_name	= NBJson_childStr(&json, "given_name", "", rootNode, NULL);
			const char* family_name	= NBJson_childStr(&json, "family_name", "", rootNode, NULL);
			const char* gender		= NBJson_childStr(&json, "gender", "", rootNode, NULL);
			const char* picUri		= NBJson_childStr(&json, "picture", "", rootNode, NULL);
			//Set user
			this->setCurrentUser(userId, email, given_name, family_name, gender, picUri);
			//
			r = true;
		}
	}
	NBJson_release(&json);
	return r;
}

SI64 AUMngrGoogleLogin::runThreadAuth(STNBThread* thread, void* param){
	//Request a new token
	AUMngrGoogleLogin* obj = (AUMngrGoogleLogin*)param;
	STMngrGoogleLoginState* data = (STMngrGoogleLoginState*)obj->_loginState;
	{
		BOOL r = FALSE;
		if(!NBOAuthClient_getToken(&data->oauth)){
			PRINTF_ERROR("GoogleLogin, could not get token.\n");
		} else {
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
				NBHttpRequest_addParamGET(&req, "alt", "json");
				NBHttpRequest_addHeader(&req, "Authorization", strToken.str);
				STNBHttpResponse* resp = NBHttpClient_executeSync(&httpClient, "www.googleapis.com", 443, "/oauth2/v1/userinfo", TRUE, &req, NULL, ENNBHttpRedirMode_None);
				if(resp == NULL){
					PRINTF_ERROR("GoogleLogin, could not request user's public_profile data.\n");
				} else if(resp->code != 200){
					PRINTF_ERROR("GoogleLogin, user's public_profile data request returned code(%d): '%s'.\n", resp->code, resp->body.str);
				} else {
					if(obj->parseProfData(resp->body.str)){
						//Save data
						{
							AUCadenaLargaMutable8* strCrypt = new(ENMemoriaTipo_Temporal) AUCadenaLargaMutable8();
							AUCadenaMutable8* strJson = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
							strJson->agregar("{");
							strJson->agregar("\"googleLogin\":{");
							strJson->agregar("\"auth\":{\"code\":\""); strJson->agregar(&data->oauth.strs.str[data->oauth.auth.code]); strJson->agregar("\"}");
							strJson->agregar(", \"profData\":\""); AUBase64::codificaBase64(resp->body.str, resp->body.length, strJson); strJson->agregar("\"");
							strJson->agregar("}");
							strJson->agregar("}");
							if(!NBMngrOSSecure::encWithGKey((const BYTE*)strJson->str(), strJson->tamano(), (const BYTE*)NB_GOOGLE_CYPHER_SALT, AUCadenaLarga8::tamano(NB_GOOGLE_CYPHER_SALT), NB_GOOGLE_CYPHER_ITERATIONS, strCrypt)){
								PRINTF_ERROR("GoogleLogin, unable to encrypt data.\n");
							} else {
								AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnLibreria(NB_GOOGLE_FILE_NAME), ENArchivoModo_SoloEscritura, ENArchivoRetainMode_Retain);
								if(file == NULL){
									PRINTF_ERROR("GoogleLogin, unable to write to local file.\n");
								} else {
									file->lock();
									if(file->escribir(strCrypt->str(), strCrypt->tamano(), 1, file) != 1){
										PRINTF_ERROR("GoogleLogin, unable to write to local file (%d bytes).\n", strCrypt->tamano());
									} else {
										PRINTF_INFO("GoogleLogin, data saved to local file (%d bytes).\n", strCrypt->tamano());
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
							obj->downloadUserImage(obj->_curUser.userId->str(), obj->_curUser.picUri->str(), "32", ENGooglePicSize_Small, &httpClient);
							//Normal pic (50 x 50)
							obj->downloadUserImage(obj->_curUser.userId->str(), obj->_curUser.picUri->str(), "50", ENGooglePicSize_Normal, &httpClient);
							//Large pic (not fixed dimensions)
							obj->downloadUserImage(obj->_curUser.userId->str(), obj->_curUser.picUri->str(), "512", ENGooglePicSize_Large, &httpClient);
						}
						r = TRUE;
					}
				}
				NBHttpRequest_release(&req);
			}
			NBHttpClient_release(&httpClient);
			NBString_release(&strToken);
		}
		//Manage login error
		if(!r){
			obj->setCurrentUser(NULL, NULL, NULL, NULL, NULL, NULL);
			//TEMPORAL: para poder usar el APPsin internet
			//NBGestorRefranero::usuarioAsegurarDatosActual(ENUsuarioLoginOrigen_Google, "123", "marcosjom@hotmail.com", "Chipote", "Chillón", "male");
			//NBGestorRefranero::usuarioActualEstablecerEstadoLogeado(ENUsuarioLoginOrigen_Google, true);
		}
	}
	//Release thread
	if(thread != NULL){
		NBThread_release(thread);
		NBMemory_free(thread);
		thread = NULL;
	}
	return 0;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrGoogleLogin)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrGoogleLogin, "AUMngrGoogleLogin")
AUOBJMETODOS_CLONAR_NULL(AUMngrGoogleLogin)
