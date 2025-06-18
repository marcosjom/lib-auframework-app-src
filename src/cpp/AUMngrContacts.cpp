//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrContacts.h"
#include "AUMngrOSSecure.h"
#include "nb/core/NBStruct.h"
#include "nb/core/NBMngrStructMaps.h"

#define NBMNGR_CONTACTS_SECS_WAIT_TO_NOTIFY_CHANGES	0.30f

//Contact data

STNBStructMapsRec NBContact_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBContact_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBContact_sharedStructMap);
	if(NBContact_sharedStructMap.map == NULL){
		STNBContact s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBContact);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, uid);
		NBStructMap_addUIntM(map, s, sequential);
		NBStructMap_addStrPtrM(map, s, givenName);
		NBStructMap_addStrPtrM(map, s, familyName);
		NBStructMap_addStrPtrM(map, s, organizationName);
		NBStructMap_addPtrToArrayOfStrPtrM(map, s, phoneNumbers, phoneNumbersSz, ENNBStructMapSign_Signed);
		NBStructMap_addPtrToArrayOfStrPtrM(map, s, emailAddresses, emailAddressesSz, ENNBStructMapSign_Signed);
		//NBStructMap_addStrPtrM(map, s, thumbnailImagePath);
		//NBStructMap_addPtrToArrayOfBytesM(map, s, thumbnailImageData, thumbnailImageDataSz, ENNBStructMapSign_Unsigned);
		NBContact_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBContact_sharedStructMap);
	return NBContact_sharedStructMap.map;
}

BOOL NBCompare_STNBContact(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBContact))
	if(dataSz == sizeof(STNBContact)){
		const STNBContact* d1 = (const STNBContact*)data1;
		const STNBContact* d2 = (const STNBContact*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(d1->uid, d2->uid);
			case ENCompareMode_Lower:
				return NBString_strIsLower(d1->uid, d2->uid);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(d1->uid, d2->uid);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(d1->uid, d2->uid);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(d1->uid, d2->uid);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//Contacts data

STNBStructMapsRec NBContacts_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBContacts_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBContacts_sharedStructMap);
	if(NBContacts_sharedStructMap.map == NULL){
		STNBContacts s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBContacts);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfStructM(map, s, contacts, contactsSz, ENNBStructMapSign_Unsigned, NBContact_getSharedStructMap());
		NBContacts_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBContacts_sharedStructMap);
	return NBContacts_sharedStructMap.map;
}


//Contact thumbnail

STNBStructMapsRec NBContactImg_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBContactImg_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBContactImg_sharedStructMap);
	if(NBContactImg_sharedStructMap.map == NULL){
		STNBContactImg s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBContactImg);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addStrPtrM(map, s, uid);
		NBStructMap_addUIntM(map, s, sequential);
		NBStructMap_addPtrToArrayOfBytesM(map, s, imgData, imgDataSz, ENNBStructMapSign_Unsigned);
		NBContactImg_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBContactImg_sharedStructMap);
	return NBContactImg_sharedStructMap.map;
}

//

BOOL NBCompare_STNBContactIdx(const ENCompareMode mode, const void* data1, const void* data2, const UI32 dataSz){
	NBASSERT(dataSz == sizeof(STNBContactIdx))
	if(dataSz == sizeof(STNBContactIdx)){
		const STNBContactIdx* d1 = (const STNBContactIdx*)data1;
		const STNBContactIdx* d2 = (const STNBContactIdx*)data2;
		switch (mode) {
			case ENCompareMode_Equal:
				return NBString_strIsEqual(*d1->uid, *d2->uid);
			case ENCompareMode_Lower:
				return NBString_strIsLower(*d1->uid, *d2->uid);
			case ENCompareMode_LowerOrEqual:
				return NBString_strIsLowerOrEqual(*d1->uid, *d2->uid);
			case ENCompareMode_Greater:
				return NBString_strIsGreater(*d1->uid, *d2->uid);
			case ENCompareMode_GreaterOrEqual:
				return NBString_strIsGreaterOrEqual(*d1->uid, *d2->uid);
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return FALSE;
}

//

STMngrContactsCalls AUMngrContacts::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
};

AUMngrContacts::AUMngrContacts() : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrContacts::AUMngrContacts")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrContacts")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		UI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0);
		}
	}
#	endif
	//Contacts
	{
		_contacts.sequential = 0;
		NBThreadMutex_init(&_contacts.mutex);
		NBArray_init(&_contacts.array, sizeof(STNBContact*), NULL);
		NBArraySorted_init(&_contacts.srchIdx, sizeof(STNBContactIdx), NBCompare_STNBContactIdx);
		//Incoming (syncing)
		{
			_contacts.incoming.secsSinceChange	= 0.0f;
			_contacts.incoming.sequential		= 0;
			NBThreadMutex_init(&_contacts.incoming.mutex);
			NBArray_init(&_contacts.incoming.array, sizeof(STNBContact*), NULL);
			NBArraySorted_init(&_contacts.incoming.srchIdx, sizeof(STNBContactIdx), NBCompare_STNBContactIdx);
		}
	}
	//Contacts thumbnails
	{
		_thumbnails.sequential = 0;
		NBThreadMutex_init(&_thumbnails.mutex);
		NBArray_init(&_thumbnails.array, sizeof(STNBContactThumbnailRef*), NULL);
		NBArraySorted_init(&_thumbnails.srchIdx, sizeof(STNBContactIdx), NBCompare_STNBContactIdx);
	}
	//Listeners
	{
		_lstnrs.sequential = 0;
		NBThreadMutex_init(&_lstnrs.mutex);
		NBArray_init(&_lstnrs.array, sizeof(STNBContactsLstnr), NULL);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrContacts::~AUMngrContacts(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrContacts::~AUMngrContacts")
	//Finish
	if(_calls.funcSetListener != NULL){
		(*_calls.funcSetListener)(_calls.funcSetListenerParam, NULL, NULL);
		_calls.funcSetListener = NULL;
	}
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	//Contacts
	{
		NBThreadMutex_lock(&_contacts.mutex);
		{
			const STNBStructMap* cMap = NBContact_getSharedStructMap();
			SI32 i; for(i = 0; i < _contacts.array.use; i++){
				STNBContact* c = NBArray_itmValueAtIndex(&_contacts.array, STNBContact*, i);
				NBStruct_stRelease(cMap, c, sizeof(*c));
				NBMemory_free(c);
				c = NULL;
			}
			NBArray_empty(&_contacts.array);
			NBArray_release(&_contacts.array);
			NBArraySorted_release(&_contacts.srchIdx);
		}
		NBThreadMutex_unlock(&_contacts.mutex);
		NBThreadMutex_release(&_contacts.mutex);
		//Incoming
		{
			NBThreadMutex_lock(&_contacts.incoming.mutex);
			{
				const STNBStructMap* cMap = NBContact_getSharedStructMap();
				SI32 i; for(i = 0; i < _contacts.incoming.array.use; i++){
					STNBContact* c = NBArray_itmValueAtIndex(&_contacts.incoming.array, STNBContact*, i);
					NBStruct_stRelease(cMap, c, sizeof(*c));
					NBMemory_free(c);
					c = NULL;
				}
				NBArray_empty(&_contacts.incoming.array);
				NBArray_release(&_contacts.incoming.array);
				NBArraySorted_release(&_contacts.incoming.srchIdx);
			}
			NBThreadMutex_unlock(&_contacts.incoming.mutex);
			NBThreadMutex_release(&_contacts.incoming.mutex);
		}
	}
	//Contacts thumbnails
	{
		NBThreadMutex_lock(&_thumbnails.mutex);
		{
			const STNBStructMap* cMap = NBContactImg_getSharedStructMap();
			SI32 i; for(i = 0; i < _thumbnails.array.use; i++){
				STNBContactThumbnailRef* c = NBArray_itmValueAtIndex(&_thumbnails.array, STNBContactThumbnailRef*, i);
				if(c->thumbnail != NULL){
					NBStruct_stRelease(cMap, c, sizeof(*c));
					NBMemory_free(c->thumbnail);
					c->thumbnail = NULL;
				}
				NBMemory_free(c);
				c = NULL;
			}
			NBArray_empty(&_thumbnails.array);
			NBArray_release(&_thumbnails.array);
			NBArraySorted_release(&_thumbnails.srchIdx);
		}
		NBThreadMutex_unlock(&_thumbnails.mutex);
		NBThreadMutex_release(&_thumbnails.mutex);
	}
	//Listeners
	{
		NBThreadMutex_lock(&_lstnrs.mutex);
		{
			/*SI32 i; for(i = 0; i < _lstnrs.array.use; i++){
				STNBContactsLstnr* c = NBArray_itmPtrAtIndex(&_lstnrs.array, STNBContactsLstnr, i);
				if(c->release != NULL){
					(*c->release)(c->param);
				}
			}*/
			NBArray_empty(&_lstnrs.array);
			NBArray_release(&_lstnrs.array);
		}
		NBThreadMutex_unlock(&_lstnrs.mutex);
		NBThreadMutex_release(&_lstnrs.mutex);
		
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrContacts::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrContacts::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrContacts::setGlue(AUAppI* app, PTRfuncContactsCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrContacts::setGlue")
	bool r = false;
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	//Init
	if(initCall != NULL){
		if(!(*initCall)(app, &_calls)){
			NBASSERT(false)
		} else {
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

void AUMngrContacts::setAsMainStore(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrContacts::setAsMainStore")
	//Set listener
	if(_calls.funcSetListener != NULL){
		STNBContactLstnrMethods mthds;
		NBMemory_setZeroSt(mthds, STNBContactLstnrMethods);
		mthds.contactFound		= AUMngrContacts::contactFound;
		mthds.thumbnailFound	= AUMngrContacts::thumbnailFound;
		(*_calls.funcSetListener)(_calls.funcSetListenerParam, this, &mthds);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

ENContactsAuthStatus AUMngrContacts::authStatus(const BOOL requestIfNecesary){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrContacts::authStatus")
	ENContactsAuthStatus r = ENContactsAuthStatus_Unavailable;
	if(_calls.funcAuthStatus != NULL){
		r = (*_calls.funcAuthStatus)(_calls.funcAuthStatusParam, requestIfNecesary);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Listening glue

void AUMngrContacts::contactFound(void* param, const STNBContact* contact){
	AUMngrContacts* obj = (AUMngrContacts*)param;
	NBThreadMutex_lock(&obj->_contacts.incoming.mutex);
	{
		const STNBStructMap* cMap = NBContact_getSharedStructMap();
		STNBContact* fnd = NULL;
		SI32 i; for(i = 0; i < obj->_contacts.incoming.array.use; i++){
			STNBContact* c = NBArray_itmValueAtIndex(&obj->_contacts.incoming.array, STNBContact*, i);
			if(NBString_strIsEqual(c->uid, contact->uid)){
				fnd = c;
				break;
			}
		}
		if(fnd == NULL){
			//Add new
			STNBContact* c = NBMemory_allocType(STNBContact);
			NBMemory_setZeroSt(*c, STNBContact);
			NBStruct_stClone(cMap, contact, sizeof(*contact), c, sizeof(*c));
			//Release thumbnail data
			c->sequential = 1;
			//Add idx
			{
				STNBContactIdx idx;
				idx.uid		= &c->uid;
				idx.iPos	= obj->_contacts.incoming.array.use;
				NBArraySorted_add(&obj->_contacts.incoming.srchIdx, &idx, sizeof(idx));
			}
			//Add record
			NBArray_addValue(&obj->_contacts.incoming.array, c);
			//PRINTF_INFO("Contact '%s' added with seq(%d).\n", c->givenName, c->sequential);
			obj->_contacts.incoming.sequential++;
			obj->_contacts.incoming.secsSinceChange = 0.0f;
		} else {
			BOOL changed = TRUE;
			//Detect changes
			{
				//ToDo: implement
			}
			//Update current
			if(changed){
				const UI32 newSeq = fnd->sequential + 1;
				NBStruct_stRelease(cMap, fnd, sizeof(*fnd));
				NBMemory_setZeroSt(*fnd, STNBContact);
				NBStruct_stClone(cMap, contact, sizeof(*contact), fnd, sizeof(*fnd));
				fnd->sequential = newSeq;
				//PRINTF_INFO("Contact '%s' updated with seq(%d).\n", fnd->givenName, fnd->sequential);
				obj->_contacts.incoming.sequential++;
				obj->_contacts.incoming.secsSinceChange = 0.0f;
			}
		}
	}
	NBThreadMutex_unlock(&obj->_contacts.incoming.mutex);
}

void AUMngrContacts::thumbnailFound(void* param, const STNBContactImg* contact){
	AUMngrContacts* obj = (AUMngrContacts*)param;
	//PRINTF_INFO("AUMngrContacts::thumbnailFound('%s') bytes(%d).\n", contact->uid, contact->imgDataSz);
	{
		NBThreadMutex_lock(&obj->_thumbnails.mutex);
		{
			STNBContactIdx idx;
			idx.uid		= (char**)&contact->uid;
			idx.iPos	= 0;
			{
				const SI32 iFnd = NBArraySorted_indexOf(&obj->_thumbnails.srchIdx, &idx, sizeof(idx), NULL);
				if(iFnd >= 0){
					//Retrieve record
					const STNBContactIdx* idx = NBArraySorted_itmPtrAtIndex(&obj->_thumbnails.srchIdx, STNBContactIdx, iFnd);
					NBASSERT(idx->iPos >= 0 && idx->iPos < obj->_thumbnails.array.use);
					STNBContactThumbnailRef* t = NBArray_itmValueAtIndex(&obj->_thumbnails.array, STNBContactThumbnailRef*, idx->iPos);
					NBASSERT(t->retainCount > 0)
					//Release previous
					if(t->thumbnail != NULL){
						NBStruct_stRelease(NBContactImg_getSharedStructMap(), t->thumbnail, sizeof(*t->thumbnail));
						NBMemory_free(t->thumbnail);
						t->thumbnail = NULL;
					}
					//Add new
					{
						t->thumbnail = NBMemory_allocType(STNBContactImg);
						NBMemory_setZeroSt(*t->thumbnail, STNBContactImg);;
						NBStruct_stClone(NBContactImg_getSharedStructMap(), contact, sizeof(*contact), t->thumbnail, sizeof(*t->thumbnail));
					}
				}
			}
		}
		NBThreadMutex_unlock(&obj->_thumbnails.mutex);
	}
}

//Search concact
const STNBContact* AUMngrContacts::getContactLocked(const char* contactId){
	const STNBContact* r = NULL;
	NBThreadMutex_lock(&_contacts.mutex);
	{
		STNBContactIdx idx;
		idx.uid		= (char**)&contactId;
		idx.iPos	= 0;
		{
			const SI32 iFnd = NBArraySorted_indexOf(&_contacts.srchIdx, &idx, sizeof(idx), NULL);
			if(iFnd < 0){
				NBThreadMutex_unlock(&_contacts.mutex);
			} else {
				const STNBContactIdx* idx = NBArraySorted_itmPtrAtIndex(&_contacts.srchIdx, STNBContactIdx, iFnd);
				NBASSERT(idx->iPos >= 0 && idx->iPos < _contacts.array.use);
				r = NBArray_itmValueAtIndex(&_contacts.array, STNBContact*, idx->iPos);
			}
		}
	}
	return r;
}

void AUMngrContacts::returnContact(const STNBContact* cc){
	NBThreadMutex_unlock(&_contacts.mutex);
}

//Read contacts

const STNBContact** AUMngrContacts::getContactsLocked(UI32* dstSz){
	const STNBContact** r = NULL;
	NBThreadMutex_lock(&_contacts.mutex);
	if(dstSz != NULL){
		*dstSz = _contacts.array.use;
	}
	if(_contacts.array.use > 0){
		r = (const STNBContact**)NBArray_data(&_contacts.array);
	} else {
		NBThreadMutex_unlock(&_contacts.mutex);
	}
	return r;
}

void AUMngrContacts::returnContacts(const STNBContact** cc){
	NBASSERT(cc == NBArray_data(&_contacts.array));
	NBThreadMutex_unlock(&_contacts.mutex);
}

//Read thumbnail

const STNBContactThumbnailRef* AUMngrContacts::getThumbnailRetained(const char* contactId){
	const STNBContactThumbnailRef* r = NULL;
	{
		NBThreadMutex_lock(&_thumbnails.mutex);
		NBASSERT(_thumbnails.srchIdx.use == _thumbnails.array.use)
		{
			STNBContactIdx idx;
			idx.uid		= (char**)&contactId;
			idx.iPos	= 0;
			{
				const SI32 iFnd = NBArraySorted_indexOf(&_thumbnails.srchIdx, &idx, sizeof(idx), NULL);
				if(iFnd >= 0){
					//Retrieve record
					const STNBContactIdx* idx = NBArraySorted_itmPtrAtIndex(&_thumbnails.srchIdx, STNBContactIdx, iFnd);
					NBASSERT(idx->iPos >= 0 && idx->iPos < _thumbnails.array.use);
					STNBContactThumbnailRef* t = NBArray_itmValueAtIndex(&_thumbnails.array, STNBContactThumbnailRef*, idx->iPos);
					NBASSERT(t->retainCount > 0)
					t->retainCount++;
					r = t;
				} else if(_calls.funcStartThumbnailReq != NULL){
					//Add record
					STNBContactThumbnailRef* c = NBMemory_allocType(STNBContactThumbnailRef);
					NBMemory_setZeroSt(*c, STNBContactThumbnailRef);
					NBString_strFreeAndNewBuffer(&c->uid, contactId);
					c->retainCount	= 1;
					//Add idx
					{
						STNBContactIdx idx;
						idx.uid		= &c->uid;
						idx.iPos	= _thumbnails.array.use;
						NBArraySorted_add(&_thumbnails.srchIdx, &idx, sizeof(idx));
					}
					//Add record
					NBArray_addValue(&_thumbnails.array, c);
					//Trigger load (unlocked)
					{
						NBThreadMutex_unlock(&_thumbnails.mutex);
						{
							(*_calls.funcStartThumbnailReq)(_calls.funcStartThumbnailReqParam, contactId);
						}
						NBThreadMutex_lock(&_thumbnails.mutex);
					}
					r = c;
				}
			}
		}
		NBASSERT(_thumbnails.srchIdx.use == _thumbnails.array.use)
		NBThreadMutex_unlock(&_thumbnails.mutex);
	}
	return r;
}

void AUMngrContacts::releaseThumbnail(const STNBContactThumbnailRef* t){
	NBThreadMutex_lock(&_thumbnails.mutex);
	NBASSERT(_thumbnails.srchIdx.use == _thumbnails.array.use)
	{
		BOOL fnd = FALSE;
		SI32 i; for(i = 0; i < _thumbnails.array.use; i++){
			STNBContactThumbnailRef* c = NBArray_itmValueAtIndex(&_thumbnails.array, STNBContactThumbnailRef*, i);
			NBASSERT(_thumbnails.srchIdx.use == _thumbnails.array.use)
			if(c == t){
				NBASSERT(c->retainCount > 0)
				c->retainCount--;
				//PRINTF_INFO("AUMngrContacts::releaseThumbnail remains %d retainCount.\n", c->retainCount);
				if(c->retainCount == 0){
					//Remove index
					{
						STNBContactIdx idx;
						idx.uid		= (char**)&c->uid;
						idx.iPos	= 0;
						{
							const SI32 iFnd = NBArraySorted_indexOf(&_thumbnails.srchIdx, &idx, sizeof(idx), NULL);
							NBASSERT(iFnd >= 0)
							if(iFnd >= 0){
								NBArraySorted_removeItemAtIndex(&_thumbnails.srchIdx, iFnd);
							}
						}
					}
					//Release uid
					if(c->uid != NULL) NBMemory_free(c->uid); c->uid = NULL;
					//Release record
					{
						const STNBStructMap* cMap = NBContactImg_getSharedStructMap();
						if(c->thumbnail != NULL){
							NBStruct_stRelease(cMap, c->thumbnail, sizeof(*c->thumbnail));
							NBMemory_free(c->thumbnail);
							c->thumbnail = NULL;
						}
						NBMemory_free(c);
						c = NULL;
					}
					NBArray_removeItemAtIndex(&_thumbnails.array, i);
					//
					//PRINTF_INFO("AUMngrContacts, thumbnail removed (%d remains).\n", _thumbnails.array.use);
				}
				fnd = TRUE;
				break;
			}
			NBASSERT(_thumbnails.srchIdx.use == _thumbnails.array.use)
		} NBASSERT(fnd) //Must be found
	}
	NBASSERT(_thumbnails.srchIdx.use == _thumbnails.array.use)
	NBThreadMutex_unlock(&_thumbnails.mutex);
}

//Listeners

void AUMngrContacts::addLstnr(STNBContactsLstnr* lstnr){
	if(lstnr != NULL){
		NBThreadMutex_lock(&_lstnrs.mutex);
		{
			/*if(lstnr->retain != NULL){
				(*lstnr->retain)(lstnr->param);
			}*/
			NBArray_addValue(&_lstnrs.array, *lstnr);
		}
		NBThreadMutex_unlock(&_lstnrs.mutex);
	}
}

void AUMngrContacts::removeLstnr(void* lstnrParam){
	if(lstnrParam != NULL){
		NBThreadMutex_lock(&_lstnrs.mutex);
		{
			BOOL found = FALSE;
			SI32 i; for(i = 0; i < _lstnrs.array.use; i++){
				STNBContactsLstnr* c = NBArray_itmPtrAtIndex(&_lstnrs.array, STNBContactsLstnr, i);
				if(c->param == lstnrParam){
					/*if(c->release != NULL){
						(*c->release)(c->param);
					}*/
					NBArray_removeItemAtIndex(&_lstnrs.array, i);
					found = TRUE;
					break;
				}
			} NBASSERT(found)
		}
		NBThreadMutex_unlock(&_lstnrs.mutex);
	}
}

void AUMngrContacts::tick(const float secs){
	_contacts.incoming.secsSinceChange += secs;
	//Analyze incoming
	if(_contacts.incoming.secsSinceChange >= NBMNGR_CONTACTS_SECS_WAIT_TO_NOTIFY_CHANGES){
		_contacts.incoming.secsSinceChange = NBMNGR_CONTACTS_SECS_WAIT_TO_NOTIFY_CHANGES;
		//Apply incoming contacts
		if(_contacts.incoming.sequential > 0){
			_contacts.incoming.sequential = 0;
			//Apply incoming as database (swap)
			NBThreadMutex_lock(&_contacts.mutex);
			NBThreadMutex_lock(&_contacts.incoming.mutex);
			{
				STNBArray		array		= _contacts.incoming.array;
				STNBArraySorted	srchIdx		= _contacts.incoming.srchIdx;
				_contacts.incoming.array	= _contacts.array;
				_contacts.incoming.srchIdx	= _contacts.srchIdx;
				_contacts.array				= array;
				_contacts.srchIdx			= srchIdx;
				//Reset incoming array
				{
					const STNBStructMap* cMap = NBContact_getSharedStructMap();
					SI32 i; for(i = 0; i < _contacts.incoming.array.use; i++){
						STNBContact* c = NBArray_itmValueAtIndex(&_contacts.incoming.array, STNBContact*, i);
						NBStruct_stRelease(cMap, c, sizeof(*c));
						NBMemory_free(c);
						c = NULL;
					}
					NBArray_empty(&_contacts.incoming.array);
					NBArraySorted_empty(&_contacts.incoming.srchIdx);
				}
				_contacts.sequential++;
			}
			NBThreadMutex_unlock(&_contacts.incoming.mutex);
			NBThreadMutex_unlock(&_contacts.mutex);
		}
		//Notify listeners
		{
			NBThreadMutex_lock(&_lstnrs.mutex);
			if(_lstnrs.sequential != _contacts.sequential){
				if(_lstnrs.array.use > 0){
					STNBArray array;
					NBArray_init(&array, sizeof(STNBContactsLstnr), NULL);
					//Acumulate notifications (locked)
					{
						SI32 i; for(i = 0; i < _lstnrs.array.use; i++){
							STNBContactsLstnr* c = NBArray_itmPtrAtIndex(&_lstnrs.array, STNBContactsLstnr, i);
							if(c->retain != NULL){
								(*c->retain)(c->param);
							}
						}
						NBArray_addItems(&array, NBArray_data(&_lstnrs.array), sizeof(STNBContactsLstnr), _lstnrs.array.use);
					}
					//Notify (unlocked)
					if(array.use > 0){
						//PRINTF_INFO("Contacts, notifying %d listeners.\n", array.use);
						NBThreadMutex_unlock(&_lstnrs.mutex);
						{
							SI32 i; for(i = 0; i < array.use; i++){
								STNBContactsLstnr* c = NBArray_itmPtrAtIndex(&array, STNBContactsLstnr, i);
								if(c->contactsChanged != NULL){
									(*c->contactsChanged)(c->param);
								}
								if(c->release != NULL){
									(*c->release)(c->param);
								}
							}
						}
						NBThreadMutex_lock(&_lstnrs.mutex);
					}
					NBArray_release(&array);
				}
				_lstnrs.sequential = _contacts.sequential;
			}
			NBThreadMutex_unlock(&_lstnrs.mutex);
		}
		//Sync (if necesary)
		if(_calls.funcStartSync != NULL){
			(*_calls.funcStartSync)(_calls.funcStartSyncParam, TRUE /*onlyIfNecesary*/);
		}
	}
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrContacts)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrContacts, "AUMngrContacts")
AUOBJMETODOS_CLONAR_NULL(AUMngrContacts)
