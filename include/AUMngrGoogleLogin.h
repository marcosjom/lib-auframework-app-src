//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrGoogleLogin_h
#define AUMngrGoogleLogin_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"
#include "nb/net/NBHttpClient.h"

typedef enum ENGooglePicSize_ {
	ENGooglePicSize_Small = 0,
	ENGooglePicSize_Normal,
	ENGooglePicSize_Large,
	//
	ENGooglePicSize_Count
} ENGooglePicSize;

typedef struct STMngrGoogleLoginPic_ {
	SI32		sequential;
	AUCadena8*	filepath;
} STMngrGoogleLoginPic;

typedef struct STMngrGoogleLoginUser_ {
	SI32		sequential;
	AUCadena8*	userId;
	AUCadena8*	email;
	AUCadena8*	firstNames;
	AUCadena8*	lastNames;
	AUCadena8*	gender;
	AUCadena8*	picUri;
	//
	STMngrGoogleLoginPic pics[ENGooglePicSize_Count];
} STMngrGoogleLoginUser;

class IGoogleLoginListener {
	public:
		virtual void googleLoginChanged(const SI32 seq, const char* userId, const char* email, const char* firstNames, const char* lastNames, const char* gender) = 0;
		virtual void googleLoginPicChanged(const char* userId, const char* email, const ENGooglePicSize picSize, const SI32 seq, const char* filepath) = 0;
};

class AUMngrGoogleLogin : public AUObjeto, public NBAnimador, public AUAppOpenUrlListener {
	public:
		AUMngrGoogleLogin(AUAppI* app);
		virtual ~AUMngrGoogleLogin();
		//
		bool	addListener(IGoogleLoginListener* itf);
		bool	removeListener(IGoogleLoginListener* itf);
		//
		bool	loadStoredData();
		void	setOAuthConfig(const char* clientId, const char* secret, const char* redirUri);
		bool	loginStart();
		bool	logout();
		bool	getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender);
		bool	getUserPic(const ENGooglePicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath);
		//
		void	tickAnimacion(float segsTranscurridos);
		//AUAppOpenUrlListener
		bool	appOpenUrl(AUAppI* app, const STNBUrl* url, const void* usrData, const UI32 usrDataSz);
		bool	appOpenUrlImage(AUAppI* app, const STNBUrl* url, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz);
		bool	appOpenFileData(AUAppI* app, const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz);
		bool	appOpenFileImageData(AUAppI* app, const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz);
		bool	appOpenBitmap(AUAppI* app, const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		AUAppI*				_app;
		void*				_loginState;	//opaque
		//User data
		NBHILO_MUTEX_CLASE	_curUserMutex;
		STMngrGoogleLoginUser	_curUser;
		//Listeners
		SI32				_listnrsUserSeq;
		SI32				_listnrsUserPicsSeq[ENGooglePicSize_Count];
		bool				_listnrsNotifying;
		NBHILO_MUTEX_CLASE	_listnrsNotifyingMutex;
		AUArregloNativoMutableP<IGoogleLoginListener*>* _listeners;
		AUArregloNativoMutableP<IGoogleLoginListener*>* _listenersToAdd;	//Added during notification cycle
		AUArregloNativoMutableP<IGoogleLoginListener*>* _listenersToRemove; //Removed during notification cycle
		//
		static SI64			runThreadAuth(STNBThread* thread, void* param);
		bool				parseProfData(const char* profData);
		bool				downloadUserImage(const char* userId, const char* picUri, const char* picSzStr, const ENGooglePicSize imgSize, STNBHttpClient* httpClient);
		bool				setCurrentUser(const char* userId, const char* email, const char* firstNames, const char* lastNames, const char* gender, const char* picUri);
		bool				setUserPicData(const char* userId, const ENGooglePicSize picSize, const BYTE* picData, const SI32 picDataSize);
};

#endif
