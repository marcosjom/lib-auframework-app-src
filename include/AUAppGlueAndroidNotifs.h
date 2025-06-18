//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAndroidNotifs_H
#define AUAppGlueAndroidNotifs_H

#include "AUAppI.h"
#include "NBMngrNotifs.h"

class AUAppGlueAndroidNotifs {
	public:
		static bool analyzeIntent(void* data, void* jIntent /*jobject*/);
		static bool setAppBadgeNumber(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, const SI32 number);
		static bool notifSchedule(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, const SI32 uid, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* data);
		static bool notifShow(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, const SI32 uid, const char* grpId /*jstring*/, const SI32 notifId /*jint*/, const char* title /*jstring*/, const char* content /*jstring*/, const char* data /*jstring*/);
		static bool notifCancel(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, void* jAlarmMngr /*jobject*/, const SI32 uid);
		//Calls
		static bool create(AUAppI* app, STMngrNotifsCalls* obj);
		static bool destroy(void* data);
		static bool loadData(void* data);
		static bool saveData(void* data);
		static bool setBadgeNumber(void* data, const SI32 num);
		static ENAppNotifAuthState getAuthStatus(void* pData, const ENAppNotifAuthQueryMode reqMode);
		//Local notifications
		static bool localRescheduleAll(void* data);
		static bool localEnable(void* data);
		static bool localAdd(void* data, const SI32 uniqueId, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData);
		static bool localCancel(void* data, const char* grpId, const SI32 notifId);
		static bool localCancelGrp(void* data, const char* grpId);
		static bool localCancelAll(void* data);
		//remote notifications
		static bool remoteGetToken(void* data, const ENAppNotifTokenQueryMode mode, STNBString* dst);
		//static void remoteSetToken(void* data, const void* token, const UI32 tokenSz);
};

#endif
