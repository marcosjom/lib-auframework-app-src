//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrNotifs_h
#define NBMngrNotifs_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrNotifs.h"

class NBMngrNotifs {
	public:
		static void		init();
		static void		finish();
		static bool		isInited();
		//
		static bool		isGlued();
		static bool		setGlue(AUAppI* app, PTRfuncNotifsCreate initCall);
		//
		static ENAppNotifAuthState getAuthStatus(const ENAppNotifAuthQueryMode reqMode);
		//Local notifications
		static bool		localEnable();
		static bool		localAdd(const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData);
		static bool		localCancel(const char* grpId, const SI32 notifId);
		static bool		localCancelGrp(const char* grpId);
		static bool		localCancelAll();
		//Remote notifications
		static bool		remoteGetToken(const ENAppNotifTokenQueryMode mode, STNBString* dst);
		static void		remoteSetToken(const void* token, const UI32 tokenSz);
		//
		static void		clearRcvdNotifs();
		//
		static bool		loadData();
		static void		setLaunchNotif(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data);
		static bool		addNotifRcvd(const ENAppNotifType type, const SI32 uid, const char* grpId, const SI32 notifId, const char* data);
		//
		static void					lock();
		static void					unlock();
		static const STAppNotif*	lockedGetLaunchNotif();
		static void					lockedClearLaunchNotif();
		static const STAppNotif*	lockedGetNotifsQueue(SI32* dstSize);
		static bool					lockedLoadFromJSON(const char* json);
		static bool					lockedSaveToJSON(AUCadenaMutable8* dst);
	private:
		static AUMngrNotifs* _instance;
};

#endif
