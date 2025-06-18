//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrGameKit_h
#define NBMngrGameKit_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrGameKit.h"

class NBMngrGameKit {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool isGlued();
		static bool setGlue(AUAppI* app, PTRfuncGameKitCreate initCall);
		//
		static ENMngrGameKitAuthState authenticationState();
		static bool startAuthentication();
		static bool getLocalPlayer(AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName);
		static bool setScore(const char* scoreId, const SI64 value);
		static bool setAchievProgress(const char* achievId, const SI8 prog100);
		static bool showCenter();
		static bool showLeaderboard();
		//
		static void lock();
		static void unlock();
		static bool lockedLoadFromJSON(const char* str);
		static bool lockedSaveToJSON(AUCadenaMutable8* dst);
		static bool lockedScoreReportingResult(const char* scoreId, const SI64 value, const ENMngrGameKitSrc src, const bool success);
		static bool lockedAchievReportingResult(const char* achievId, const SI8 prog100, const ENMngrGameKitSrc src, const bool success);
	private:
		static AUMngrGameKit* _instance;
};

#endif
