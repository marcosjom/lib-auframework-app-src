//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSGameKit_H
#define AUAppGlueIOSGameKit_H

#include "AUMngrGameKit.h"
#include "AUAppI.h"

class AUAppGlueIOSGameKit {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrGameKitCalls* obj);
		static bool destroy(void* data);
		static bool loadData(void* data);
		static bool saveData(void* data);
		static ENMngrGameKitAuthState authenticationState(void* data);
		static bool startAuthentication(void* data);
		static bool getLocalPlayer(void* data, AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName);
		static bool sendScoreAndReport(void* data, const char* scoreId, const SI64 value);
		static bool sendAchievProgressAndReport(void* data, const char* achievId, const SI8 prog100);
		static bool showCenter(void* data);
		static bool showLeaderboard(void* data);
};

#endif
