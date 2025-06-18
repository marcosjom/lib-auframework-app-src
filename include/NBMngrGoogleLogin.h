//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrGoogleLogin_h
#define NBMngrGoogleLogin_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrGoogleLogin.h"

class NBMngrGoogleLogin {
	public:
		static void init(AUAppI* app);
		static void finish();
		static bool isInited();
		//
		static bool	addListener(IGoogleLoginListener* itf);
		static bool	removeListener(IGoogleLoginListener* itf);
		//
		static bool	loadStoredData();
		static void	setOAuthConfig(const char* clientId, const char* secret, const char* redirUri);
		static bool	loginStart();
		static bool	logout();
		static bool	getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender);
		static bool	getUserPic(const ENGooglePicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath);
	private:
		static AUMngrGoogleLogin* _instance;
};

#endif
