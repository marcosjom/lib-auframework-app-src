//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAppleContacts_H
#define AUAppGlueAppleContacts_H

#include "AUMngrContacts.h"

class AUAppGlueAppleContacts {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrContactsCalls* obj);
		static bool destroy(void* data);
		//
		static ENContactsAuthStatus authStatus(void* data, const BOOL requestIfNecesary);
		//
		static void setListener(void* data, void* lstnr, STNBContactLstnrMethods* lstnrMethods);
		static void startSync(void* data, const BOOL onlyIfNecesary);
		static void	startThumbnailReq(void* data, const char* contactId);
};

#endif
