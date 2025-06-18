//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSNotifs_H
#define AUAppGlueIOSNotifs_H

#include "AUAppI.h"
#include "AUMngrNotifs.h"

class AUAppGlueIOSNotifs {
	public:
		static bool	analyzeLaunchOptions(void* data, void* launchOptions /*NSDictionary*/);
		static bool analyzeLocalNotification(void* data, void* localNotif /*UILocalNotification*/);
		//Calls
		static bool create(AUAppI* app, STMngrNotifsCalls* obj);
		static bool destroy(void* data);
		static bool loadData(void* data);
		static bool saveData(void* data);
		static bool setBadgeNumber(void* data, const SI32 num);
		static ENAppNotifAuthState getAuthStatus(void* data, const ENAppNotifAuthQueryMode reqMode);
		//local notifications
		static bool localRescheduleAll(void* data);
		static bool localEnable(void* data);
		static bool localAdd(void* data, const SI32 uniqueId, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData);
		static bool localCancel(void* data, const char* grpId, const SI32 notifId);
		static bool localCancelGrp(void* data, const char* grpId);
		static bool localCancelAll(void* data);
		//remote notifications
		static bool remoteGetToken(void* data, const ENAppNotifTokenQueryMode mode, STNBString* dst);
		static void remoteSetToken(void* data, const void* token, const UI32 tokenSz);
};

#endif
