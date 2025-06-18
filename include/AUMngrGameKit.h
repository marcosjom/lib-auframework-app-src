//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrGameKit_h
#define AUMngrGameKit_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

//Callbacks

typedef enum ENMngrGameKitAuthState_ {
	ENMngrGameKitAuthState_LogedOut = 0,
	ENMngrGameKitAuthState_LogedIn,
	ENMngrGameKitAuthState_Quering
} ENMngrGameKitAuthState;

typedef enum ENMngrGameKitSrc_ {
	ENMngrGameKitSrc_OSApi = 0,	//OS API (if available)
	ENMngrGameKitSrc_MyServer,	//My custom server (if available)
	ENMngrGameKitSrc_Count
} ENMngrGameKitSrc;

typedef struct STMngrGameKitCalls_ STMngrGameKitCalls;

typedef bool (*PTRfuncGameKitCreate)(AUAppI* app, STMngrGameKitCalls* obj);
typedef bool (*PTRfuncGameKitDestroy)(void* obj);
typedef bool (*PTRfuncGameKitLoadData)(void* obj);
typedef bool (*PTRfuncGameKitSaveData)(void* obj);
typedef ENMngrGameKitAuthState (*PTRfuncGameKitAuthedState)(void* obj);
typedef bool (*PTRfuncGameKitAuthenticate)(void* obj);
typedef bool (*PTRfuncGameKitGetLocalPlayer)(void* obj, AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName);
typedef bool (*PTRfuncGameKitSendScore)(void* obj, const char* scoreId, const SI64 value);
typedef bool (*PTRfuncGameKitSendAchiev)(void* obj, const char* achievId, const SI8 prog100);
typedef bool (*PTRfuncGameKitShowCenter)(void* obj);
typedef bool (*PTRfuncGameKitShowLeaderbrd)(void* obj);
//
typedef struct STMngrGameKitCalls_ {
	PTRfuncGameKitCreate		funcCreate;
	void*						funcCreateParam;
	PTRfuncGameKitDestroy		funcDestroy;
	void*						funcDestroyParam;
	//
	PTRfuncGameKitLoadData		funcLoadData;
	void*						funcLoadDataParam;
	PTRfuncGameKitSaveData		funcSaveData;
	void*						funcSaveDataParam;
	//
	PTRfuncGameKitAuthedState	funcAuthState;
	void*						funcAuthStateParam;
	PTRfuncGameKitAuthenticate	funcAuthenticate;
	void*						funcAuthenticateParam;
	PTRfuncGameKitGetLocalPlayer funcGetLocalPlayer;
	void*						funcGetLocalPlayerParam;
	PTRfuncGameKitShowCenter	funcShowCenter;
	void*						funcShowCenterParam;
	PTRfuncGameKitShowLeaderbrd	funcShowLeaderbrd;
	void*						funcShowLeaderbrdParam;
	PTRfuncGameKitSendScore		funcSendScore;
	void*						funcSendScoreParam;
	PTRfuncGameKitSendAchiev	funcSendAchiev;
	void*						funcSendAchievParam;
} STMngrGameKitCalls;

//

typedef struct STAppScoreVal_ {
	SI64			value;			//current value
	SI64			valSending;		//value currently sending
	SI64			valSent;		//value already sent
} STAppScoreVal;

typedef struct STAppScore_ {
	AUCadena8*		uniqueId;
	STAppScoreVal	val[ENMngrGameKitSrc_Count];
} STAppScore;

typedef struct STAppAchievVal_ {
	SI8			prog100;		//current value
	SI8			progSending;	//value currently sending
	SI8			progSent;		//value already sent
} STAppAchievVal;

typedef struct STAppAchiev_ {
	AUCadena8*		uniqueId;
	STAppAchievVal	val[ENMngrGameKitSrc_Count];
} STAppAchiev;

class AUMngrGameKit : public AUObjeto {
	public:
		AUMngrGameKit();
		virtual ~AUMngrGameKit();
		//
		static bool			isGlued();
		static bool			setGlue(AUAppI* app, PTRfuncGameKitCreate initCall);
		//
		ENMngrGameKitAuthState authenticationState();
		bool				startAuthentication();
		bool				getLocalPlayer(AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName);
		bool				setScore(const char* scoreId, const SI64 value);
		bool				setAchievProgress(const char* achievId, const SI8 prog100);
		bool				showCenter();
		bool				showLeaderboard();
		//
		void				lock();
		void				unlock();
		bool				lockedLoadFromJSON(const char* str);
		bool				lockedSaveToJSON(AUCadenaMutable8* dst) const;
		bool				lockedScoreReportingResult(const char* scoreId, const SI64 value, const ENMngrGameKitSrc src, const bool success);
		bool				lockedAchievReportingResult(const char* achievId, const SI8 prog100, const ENMngrGameKitSrc src, const bool success);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		static STMngrGameKitCalls _calls;
		//
		NBHILO_MUTEX_CLASE		_mutex;
		SI32					_mutexLocksCount;			//Depth of calling ->lock() and unlock().
		AUArregloNativoMutableP<STAppScore>		_scores;
		AUArregloNativoMutableP<STAppAchiev>	_achievs;
		//
		STAppScore*		privLockedGetScore(const char* scoreId);
		STAppAchiev*	privLockedGetAchiev(const char* achievId);
		STAppScore*		privLockedAddOrUpdateScore(const char* scoreId, const SI64 value, const ENMngrGameKitSrc src);
		STAppAchiev*	privLockedAddOrUpdateAchiev(const char* achievId, const SI8 prog100, const ENMngrGameKitSrc src);
};

#endif
