//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrFbLogin_h
#define AUMngrFbLogin_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"
#include "nb/net/NBHttpClient.h"

typedef enum ENFbPicSize_ {
	ENFbPicSize_Small = 0,
	ENFbPicSize_Normal,
	ENFbPicSize_Large,
	//
	ENFbPicSize_Count
} ENFbPicSize;

typedef struct STMngrFbLoginPic_ {
	SI32		sequential;
	AUCadena8*	filepath;
} STMngrFbLoginPic;

typedef struct STMngrFbLoginUser_ {
	SI32		sequential;
	AUCadena8*	userId;
	AUCadena8*	email;
	AUCadena8*	firstNames;
	AUCadena8*	lastNames;
	AUCadena8*	gender;
	//
	STMngrFbLoginPic pics[ENFbPicSize_Count];
} STMngrFbLoginUser;

class IFbLoginListener {
	public:
		virtual void fbLoginChanged(const SI32 seq, const char* userId, const char* email, const char* firstNames, const char* lastNames, const char* gender) = 0;
		virtual void fbLoginPicChanged(const char* userId, const char* email, const ENFbPicSize picSize, const SI32 seq, const char* filepath) = 0;
};

class AUMngrFbLogin : public AUObjeto, public NBAnimador, public AUAppOpenUrlListener {
	public:
		AUMngrFbLogin(AUAppI* app);
		virtual ~AUMngrFbLogin();
		//
		bool	addListener(IFbLoginListener* itf);
		bool	removeListener(IFbLoginListener* itf);
		//
		bool	loadStoredData();
		void	setConfig(const char* appClientId, const char* webClientId, const char* webSecret, const char* webRedirUri);
		bool	loginStart();
		bool	logout();
		bool	getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender);
		bool	getUserPic(const ENFbPicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath);
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
		STMngrFbLoginUser	_curUser;
		//Listeners
		SI32				_listnrsUserSeq;
		SI32				_listnrsUserPicsSeq[ENFbPicSize_Count];
		bool				_listnrsNotifying;
		NBHILO_MUTEX_CLASE	_listnrsNotifyingMutex;
		AUArregloNativoMutableP<IFbLoginListener*>* _listeners;
		AUArregloNativoMutableP<IFbLoginListener*>* _listenersToAdd;	//Added during notification cycle
		AUArregloNativoMutableP<IFbLoginListener*>* _listenersToRemove; //Removed during notification cycle
		//
		static SI64			runThreadAuth(STNBThread* thread, void* param);
		static SI64			runThreadLoadProf(STNBThread* thread, void* param);
		bool				parseProfData(const char* profData);
		bool				downloadUserImage(const char* userId, const char* imgType, const ENFbPicSize imgSize, STNBHttpClient* httpClient);
		bool				setCurrentUser(const char* userId, const char* email, const char* firstNames, const char* lastNames, const char* gender);
		bool				setUserPicData(const char* userId, const ENFbPicSize picSize, const BYTE* picData, const SI32 picDataSize);
};

#endif
