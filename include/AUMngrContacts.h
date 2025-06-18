//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrContacts_h
#define AUMngrContacts_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"
#include "nb/core/NBStructMap.h"
#include "nb/core/NBArraySorted.h"

typedef enum ENContactsAuthStatus_ {
	ENContactsAuthStatus_NotDetermined = 0,
	ENContactsAuthStatus_Authorized,
	ENContactsAuthStatus_Requesting,
	ENContactsAuthStatus_Denied,
	ENContactsAuthStatus_Restricted,
	ENContactsAuthStatus_Unavailable,
	//
	ENContactsAuthStatus_Count
} ENContactsAuthStatus;

//Contact data

typedef struct STNBContact_ {
	char*	uid;			//unique-id
	UI32	sequential;		//changes sequential
	char*	givenName;
	char*	familyName;
	char*	organizationName;
	char**	phoneNumbers;
	SI32	phoneNumbersSz;
	char**	emailAddresses;
	SI32	emailAddressesSz;
	//char*	thumbnailImagePath;		//in-library folder
	//BYTE*	thumbnailImageData;
	//UI32	thumbnailImageDataSz;
} STNBContact;

const STNBStructMap* NBContact_getSharedStructMap(void);

BOOL NBCompare_STNBContact(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

//Contacts data

typedef struct STNBContacts_ {
	STNBContact*	contacts;
	UI32			contactsSz;
} STNBContacts;

const STNBStructMap* NBContacts_getSharedStructMap(void);

//Contact thumbnail

typedef struct STNBContactImg_ {
	char*	uid;			//unique-id
	UI32	sequential;		//changes sequential
	BYTE*	imgData;
	UI32	imgDataSz;
} STNBContactImg;

const STNBStructMap* NBContactImg_getSharedStructMap(void);

//

typedef struct STNBContactThumbnailRef_ {
	char*			uid;			//unique-id
	STNBContactImg*	thumbnail;		//NULL if request has no returned yet
	SI32			retainCount;
} STNBContactThumbnailRef;

//

typedef struct STNBContactIdx_ {
	char**	uid;			//unique-id
	UI32	iPos;
} STNBContactIdx;

BOOL NBCompare_STNBContactIdx(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz);

//

typedef struct STNBContactsLstnr_ {
	void*	param;
	void	(*retain)(void* obj);
	void	(*release)(void* obj);
	void	(*contactsChanged)(void* obj);
} STNBContactsLstnr;

//

typedef struct STNBContactLstnrMethods_ {
	void		(*contactFound)(void* param, const STNBContact* contact);
	void		(*thumbnailFound)(void* param, const STNBContactImg* thumbnail);
} STNBContactLstnrMethods;

//

typedef struct STMngrContactsCalls_ STMngrContactsCalls;

//Callbacks
typedef bool (*PTRfuncContactsCreate)(AUAppI* app, STMngrContactsCalls* obj);
typedef bool (*PTRfuncContactsDestroy)(void* obj);
//
typedef ENContactsAuthStatus (*PTRFuncContactsAuthStatus)(void* obj, const BOOL requestIfNecesary);
//
typedef void (*PTRFuncContactsSetListener)(void* obj, void* lstnr, STNBContactLstnrMethods* lstnrMethods);
typedef void (*PTRFuncContactsStartSync)(void* obj, const BOOL onlyIfNecesary);
//Thumbnail
typedef void (*PTRFuncContactsStartThumbnailReq)(void* obj, const char* contactId);

typedef struct STMngrContactsCalls_ {
	PTRfuncContactsCreate				funcCreate;
	void*								funcCreateParam;
	PTRfuncContactsDestroy				funcDestroy;
	void*								funcDestroyParam;
	//
	PTRFuncContactsAuthStatus			funcAuthStatus;
	void*								funcAuthStatusParam;
	//
	PTRFuncContactsSetListener			funcSetListener;
	void*								funcSetListenerParam;
	PTRFuncContactsStartSync			funcStartSync;
	void*								funcStartSyncParam;
	//Thumbnail
	PTRFuncContactsStartThumbnailReq	funcStartThumbnailReq;
	void*								funcStartThumbnailReqParam;
} STMngrContactsCalls;

//

class AUMngrContacts : public AUObjeto {
	public:
		AUMngrContacts();
		virtual ~AUMngrContacts();
		//
		static bool	isGlued();
		static bool	setGlue(AUAppI* app, PTRfuncContactsCreate initCall);
		//
		void					setAsMainStore();
		ENContactsAuthStatus	authStatus(const BOOL requestIfNecesary);
		//Search concact
		const STNBContact*		getContactLocked(const char* contactId);
		void					returnContact(const STNBContact* cc);
		//Read contacts
		const STNBContact**		getContactsLocked(UI32* dstSz);
		void					returnContacts(const STNBContact** cc);
		//Read thumbnail
		const STNBContactThumbnailRef* getThumbnailRetained(const char* contactId);
		void					releaseThumbnail(const STNBContactThumbnailRef* t);
		//Listeners
		void					addLstnr(STNBContactsLstnr* lstnr);
		void					removeLstnr(void* lstnrParam);
		void					tick(const float secs);
		//Listening glue
		static void				contactFound(void* param, const STNBContact* contact);
		static void				thumbnailFound(void* param, const STNBContactImg* contact);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrContactsCalls _calls;
		//Contacts
		struct {
			UI64			sequential;	//changes count
			STNBThreadMutex	mutex;
			STNBArray		array;		//STNBContact*
			STNBArraySorted	srchIdx;	//STNBContactIdx
			//Incoming (syncing)
			struct {
				float			secsSinceChange;
				UI64			sequential;	//changes count
				STNBThreadMutex	mutex;
				STNBArray		array;		//STNBContact*
				STNBArraySorted	srchIdx;	//STNBContactIdx
			} incoming;
		} _contacts;
		//Contacts thumbnails
		struct {
			UI64			sequential;	//changes count
			STNBThreadMutex	mutex;
			STNBArray		array;		//STNBContactThumbnailRef*
			STNBArraySorted	srchIdx;	//STNBContactIdx
		} _thumbnails;
		//Listeners
		struct {
			UI64			sequential;	//compare against "_contacts.sequential"
			STNBThreadMutex	mutex;
			STNBArray		array;	//STNBContactsLstnr
		} _lstnrs;
};

#endif
