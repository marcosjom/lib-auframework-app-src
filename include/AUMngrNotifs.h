//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrNotifs_h
#define AUMngrNotifs_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

//Auth state

typedef enum ENAppNotifAuthState_ {
	ENAppNotifAuthState_NotDetermined = 0, //Not requested yet
	ENAppNotifAuthState_QueringSettings, //Quering settings
	ENAppNotifAuthState_Requesting,		//Requesting permission
	ENAppNotifAuthState_Denied,			//Explicit denied
	ENAppNotifAuthState_Authorized,		//Explicit authorized
	ENAppNotifAuthState_Count
} ENAppNotifAuthState;

//Auth request mode

typedef enum ENAppNotifAuthQueryMode_ {
	ENAppNotifAuthQueryMode_CacheOnly = 0,	//Not requested yet
	ENAppNotifAuthQueryMode_CurrentState,	//Current action value
	ENAppNotifAuthQueryMode_UpdateCache,	//Quering settings
	ENAppNotifAuthQueryMode_UpdateCacheRequestIfNecesary, //Requesting permission
	ENAppNotifAuthQueryMode_Count
} ENAppNotifAuthQueryMode;

//Token request mode

typedef enum ENAppNotifTokenQueryMode_ {
	ENAppNotifTokenQueryMode_CacheOnly = 0,
	ENAppNotifTokenQueryMode_CacheAndRequestIfNecesary,
	ENAppNotifTokenQueryMode_ForceNewRequest,
	ENAppNotifTokenQueryMode_Count,
} ENAppNotifTokenQueryMode;

//

typedef enum ENAppNotifType_ {
	ENNotifType_Local = 0,
	ENNotifType_Remote,
	ENNotifType_Count
} ENAppNotifType;

//Callbacks

typedef struct STMngrNotifsCalls_ STMngrNotifsCalls;

typedef bool (*PTRfuncNotifsCreate)(AUAppI* app, STMngrNotifsCalls* obj);
typedef bool (*PTRfuncNotifsDestroy)(void* obj);
typedef bool (*PTRfuncNotifsLoadData)(void* obj);
typedef bool (*PTRfuncNotifsSaveData)(void* obj);
typedef bool (*PTRfuncNotifsSetBadgeNumber)(void* obj, const SI32 number);
typedef ENAppNotifAuthState (*PTRfuncNotifsGetAuthStatus)(void* obj, const ENAppNotifAuthQueryMode reqMode);
//Local notifications
typedef bool (*PTRfuncNotifsLocalRescheduleAll)(void* obj);
typedef bool (*PTRfuncNotifsLocalEnable)(void* obj);
typedef bool (*PTRfuncNotifsLocalAdd)(void* obj, const SI32 uniqueId, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* data);
typedef bool (*PTRfuncNotifsLocalCancel)(void* obj, const char* grpId, const SI32 notifId);
typedef bool (*PTRfuncNotifsLocalCancelGrp)(void* obj, const char* grpId);
typedef bool (*PTRfuncNotifsLocalCancelAll)(void* obj);
//Remote notifications
typedef bool (*PTRfuncNotifsRemoteGetToken)(void* obj, const ENAppNotifTokenQueryMode mode, STNBString* dst);
typedef void (*PTRfuncNotifsRemoteSetToken)(void* obj, const void* token, const UI32 tokenSz);

//
typedef struct STMngrNotifsCalls_ {
	PTRfuncNotifsCreate			funcCreate;
	void*						funcCreateParam;
	PTRfuncNotifsDestroy		funcDestroy;
	void*						funcDestroyParam;
	//
	PTRfuncNotifsLoadData		funcLoadData;
	void*						funcLoadDataParam;
	PTRfuncNotifsSaveData		funcSaveData;
	void*						funcSaveDataParam;
	PTRfuncNotifsSetBadgeNumber funcSetBadgeNumber;
	void*						funcSetBadgeNumberParam;
	PTRfuncNotifsGetAuthStatus	funcGetAuthStatus;
	void*						funcGetAuthStatusParam;
	//Local notifications
	PTRfuncNotifsLocalRescheduleAll	funcLocalRescheduleAll;
	void*						funcLocalRescheduleAllParam;
	PTRfuncNotifsLocalEnable	funcLocalEnable;
	void*						funcLocalEnableParam;
	PTRfuncNotifsLocalAdd		funcLocalAdd;
	void*						funcLocalAddParam;
	PTRfuncNotifsLocalCancel	funcLocalCancel;
	void*						funcLocalCancelParam;
	PTRfuncNotifsLocalCancelGrp	funcLocalCancelGrp;
	void*						funcLocalCancelGrpParam;
	PTRfuncNotifsLocalCancelAll funcLocalCancelAll;
	void*						funcLocalCancelAllParam;
	//Remote
	PTRfuncNotifsRemoteGetToken	funcRemoteGetToken;
	void*						funcRemoteGetTokenParam;
	PTRfuncNotifsRemoteSetToken	funcRemoteSetToken;
	void*						funcRemoteSetTokenParam;
} STMngrNotifsCalls;

//

typedef struct STAppNotif_ {
	SI32			uniqueId;
	ENAppNotifType	type;
	AUCadena8*		grpId;
	SI32			notifId;
	AUCadena8*		data;
} STAppNotif;

class AUMngrNotifs : public AUObjeto {
	public:
		AUMngrNotifs();
		virtual ~AUMngrNotifs();
		//
		static bool			isGlued();
		static bool			setGlue(AUAppI* app, PTRfuncNotifsCreate initCall);
		//
		ENAppNotifAuthState		getAuthStatus(const ENAppNotifAuthQueryMode reqMode);
		//Local notifications
		bool				localEnable();
		bool				localAdd(const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData);
		bool				localCancel(const char* grpId, const SI32 notifId);
		bool				localCancelGrp(const char* grpId);
		bool				localCancelAll();
		//Remote notifications
		bool				remoteGetToken(const ENAppNotifTokenQueryMode mode, STNBString* dst);
		void				remoteSetToken(const void* token, const UI32 tokenSz);
		//
		void				clearRcvdNotifs();
		//
		bool				loadData();
		void				setLaunchNotif(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data);
		bool				addNotifRcvd(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data);
		//
		void				lock();
		void				unlock();
		const STAppNotif*	lockedGetLaunchNotif() const;
		void				lockedClearLaunchNotif();
		const STAppNotif*	lockedGetNotifsQueue(SI32* dstSize) const;
		bool				lockedLoadFromJSON(const char* str);
		bool				lockedSaveToJSON(AUCadenaMutable8* dst) const;
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrNotifsCalls _calls;
		//
		NBHILO_MUTEX_CLASE		_mutex;
		SI32					_mutexLocksCount;			//Depth of calling ->lock() and unlock().
		//
		SI32					_lastUniqueId;				//Records unique id generator
		STAppNotif*				_launchNotif;				//Notification that launched or focus the app
		AUArregloNativoMutableP<STAppNotif> _notifsQueued;	//Send to queue
		AUArregloNativoMutableP<STAppNotif> _notifsRcvd;	//Received
};

#endif
