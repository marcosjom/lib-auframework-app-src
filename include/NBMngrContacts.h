//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrContacts_h
#define NBMngrContacts_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrContacts.h"

class NBMngrContacts {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool	isGlued();
		static bool setGlue(AUAppI* app, PTRfuncContactsCreate initCall);
		//
		static ENContactsAuthStatus authStatus(const BOOL requestIfNecesary);
		//Search concact
		static const STNBContact*	getContactLocked(const char* contactId);
		static void					returnContact(const STNBContact* cc);
		//Read contacts
		static const STNBContact**	getContactsLocked(UI32* dstSz);
		static void					returnContacts(const STNBContact** cc);
		//Read thumbnail
		static const STNBContactThumbnailRef* getThumbnailRetained(const char* contactId);
		static void					releaseThumbnail(const STNBContactThumbnailRef* t);
		//Listeners
		static void	addLstnr(STNBContactsLstnr* lstnr);
		static void	removeLstnr(void* lstnrParam);
		static void	tick(const float secs);
	private:
		static AUMngrContacts* _instance;
};

#endif
