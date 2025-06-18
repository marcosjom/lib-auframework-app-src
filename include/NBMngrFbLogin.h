//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrFbLogin_h
#define NBMngrFbLogin_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrFbLogin.h"

class NBMngrFbLogin {
	public:
		static void init(AUAppI* app);
		static void finish();
		static bool isInited();
		//
		static bool	addListener(IFbLoginListener* itf);
		static bool	removeListener(IFbLoginListener* itf);
		//
		static bool	loadStoredData();
		static void	setConfig(const char* appClientId, const char* webClientId, const char* webSecret, const char* webRedirUri);
		static bool	loginStart();
		static bool	logout();
		static bool	getUser(SI32* dstSeq, AUCadenaMutable8* dstId, AUCadenaMutable8* dstEmail, AUCadenaMutable8* dstFirstNames, AUCadenaMutable8* dstLastNames, AUCadenaMutable8* dstGender);
		static bool	getUserPic(const ENFbPicSize picSize, SI32* dstSeq, AUCadenaMutable8* dstPath);
	private:
		static AUMngrFbLogin* _instance;
};

#endif
