//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueAppleContacts.h"
#include "NBMngrContacts.h"
#include "NBMngrOSSecure.h"
#include "nb/core/NBStruct.h"
//
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#	include "nb/NBObjCMethods.h"	//for "objc_msgSend", "sel_registerName", ...
//#	include <objc/message.h>	//for "objc_msgSend"
//#	include <objc/objc.h>		//for "sel_registerName"
#endif

//

#include <Contacts/Contacts.h>

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

/*#ifdef __APPLE__
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
		//iOS simulator
#	elif TARGET_OS_IPHONE
		//iOS
#	else
		//OSX
#	endif
#endif*/

struct AUAppGlueAppleContactsData_;

BOOL AUAppGlueAppleContacts_startSyncAll(void* pData);

//Changes listener
@interface AUAppContactNotifDelegate : NSObject {
@private
	struct AUAppGlueAppleContactsData_* _data;
}
- (id)initWithData:(struct AUAppGlueAppleContactsData_*)data;
- (void)contactDidChange:(NSNotification*)notif;
@end

typedef struct AUAppGlueAppleContactsData_ {
	AUAppI* 					app;
	CNContactStore*				store;
	AUAppContactNotifDelegate*	changeDelegate;
	BOOL						requestingAuth;
	//
	BOOL						lstnrSynced;
	void*						lstnr;
	STNBContactLstnrMethods		lstnrMethods;
} AUAppGlueAppleContactsData;

//Calls
	
bool AUAppGlueAppleContacts::create(AUAppI* app, STMngrContactsCalls* obj){
	AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAppleContactsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAppleContactsData);
	NBMemory_setZeroSt(*obj, STMngrContactsCalls);
	data->app					= (AUAppI*)app;
	data->store					= NULL;
	data->changeDelegate		= NULL;
	{
		Class clsCStore = NSClassFromString(@"CNContactStore");
		if(clsCStore){
			data->store			= [[CNContactStore alloc] init];
			data->changeDelegate = [[AUAppContactNotifDelegate alloc] initWithData:data];
		}
	}
	data->requestingAuth		= FALSE;
	//
	data->lstnrSynced			= FALSE;
	data->lstnr					= NULL;
	NBMemory_setZero(data->lstnrMethods);
	//
	obj->funcCreate				= create;
	obj->funcCreateParam		= data;
	obj->funcDestroy			= destroy;
	obj->funcDestroyParam		= data;
	//
	obj->funcAuthStatus			= authStatus;
	obj->funcAuthStatusParam	= data;
	//
	obj->funcSetListener		= setListener;
	obj->funcSetListenerParam	= data;
	obj->funcStartSync			= startSync;
	obj->funcStartSyncParam		= data;
	obj->funcStartThumbnailReq	= startThumbnailReq;
	obj->funcStartThumbnailReqParam	= data;
	//Observer
	if(data->changeDelegate != NULL){
		[[NSNotificationCenter defaultCenter] addObserver:data->changeDelegate selector:@selector(contactDidChange:) name:CNContactStoreDidChangeNotification object:nil];
	}
	return true;
}

bool AUAppGlueAppleContacts::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)pData;
		{
			data->lstnrSynced = FALSE;
			data->lstnr = NULL;
			NBMemory_setZero(data->lstnrMethods);
		}
		if(data->changeDelegate != NULL){
			//Observer
			[[NSNotificationCenter defaultCenter] removeObserver:data->changeDelegate];
			[data->changeDelegate release];
			data->changeDelegate = nil;
		}
		if(data->store != nil){
			[data->store release];
			data->store = nil;
		}
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

BOOL AUAppGlueAppleContacts_startSyncAll(void* pData){ //STNBContact
	BOOL r = FALSE;
	AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)pData;
	if(data->store != NULL && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
		@autoreleasepool {
			CNAuthorizationStatus status = [CNContactStore authorizationStatusForEntityType:CNEntityTypeContacts];
			if(status == CNAuthorizationStatusAuthorized){
				//keys with fetching properties
				NSArray* keys = [NSArray arrayWithObjects:
								 CNContactIdentifierKey
								 , CNContactFamilyNameKey
								 , CNContactGivenNameKey
								 , CNContactOrganizationNameKey
								 , CNContactPhoneNumbersKey
								 , CNContactEmailAddressesKey
								 //, CNContactImageDataAvailableKey
								 //, CNContactImageDataKey
								 //, CNContactThumbnailImageDataKey
								 , nil];
				CNContactFetchRequest *request = [[CNContactFetchRequest alloc] initWithKeysToFetch:keys];
				{
					NSError *error = nil;
					r = [data->store enumerateContactsWithFetchRequest:request error:&error usingBlock:^(CNContact * _Nonnull contact, BOOL * _Nonnull stop) {
						if(contact != nil && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
							@autoreleasepool {
								STNBContact c;
								NBMemory_setZeroSt(c, STNBContact);
								//Populate
								{
									//PRINTF_INFO("Contact enumerated:\n");
									if([contact isKeyAvailable:CNContactIdentifierKey]){
										//PRINTF_INFO("identifier: '%s'.\n", [contact.identifier UTF8String]);
										NBString_strFreeAndNewBuffer(&c.uid, [contact.identifier UTF8String]);
									}
									if([contact isKeyAvailable:CNContactGivenNameKey]){
										//PRINTF_INFO("givenName: '%s'.\n", [contact.givenName UTF8String]);
										NBString_strFreeAndNewBuffer(&c.givenName, [contact.givenName UTF8String]);
									}
									if([contact isKeyAvailable:CNContactFamilyNameKey]){
										//PRINTF_INFO("familyName: '%s'.\n", [contact.familyName UTF8String]);
										NBString_strFreeAndNewBuffer(&c.familyName, [contact.familyName UTF8String]);
									}
									if([contact isKeyAvailable:CNContactOrganizationNameKey]){
										//PRINTF_INFO("organizationName: '%s'.\n", [contact.organizationName UTF8String]);
										NBString_strFreeAndNewBuffer(&c.organizationName, [contact.organizationName UTF8String]);
									}
									if([contact isKeyAvailable:CNContactPhoneNumbersKey]){
										if(contact.phoneNumbers.count > 0){
											c.phoneNumbers		= NBMemory_allocTypes(char*, contact.phoneNumbers.count);
											c.phoneNumbersSz	= (SI32)contact.phoneNumbers.count;
											SI32 i; for(i = 0 ; i < contact.phoneNumbers.count; i++){
												c.phoneNumbers[i] = NULL;
												{
													CNLabeledValue* lbl = contact.phoneNumbers[i];
													if(lbl != nil){
														CNPhoneNumber* num = lbl.value;
														if(num != NULL){
															NSString* str = num.stringValue;
															if(str != nil){
																//PRINTF_INFO("Phone: '%s'.\n", [str UTF8String]);
																NBString_strFreeAndNewBuffer(&c.phoneNumbers[i], [str UTF8String]);
															}
														}
													}
												}
											}
										}
									}
									if([contact isKeyAvailable:CNContactEmailAddressesKey]){
										if(contact.emailAddresses.count > 0){
											c.emailAddresses	= NBMemory_allocTypes(char*, contact.emailAddresses.count);
											c.emailAddressesSz	= (SI32)contact.emailAddresses.count;
											SI32 i; for(i = 0 ; i < contact.emailAddresses.count; i++){
												c.emailAddresses[i] = NULL;
												{
													CNLabeledValue* lbl = contact.emailAddresses[i];
													if(lbl != nil){
														NSString* str = lbl.value;
														if(str != nil){
															//PRINTF_INFO("Email: '%s'.\n", [str UTF8String]);
															NBString_strFreeAndNewBuffer(&c.emailAddresses[i], [str UTF8String]);
														}
													}
												}
											}
										}
									}
									//Images
									/*if([contact isKeyAvailable:CNContactThumbnailImageDataKey]){
									 if(contact.thumbnailImageData != nil){
									 NSData* imgData		= contact.thumbnailImageData;
									 const void* data	= [imgData bytes];
									 const UI32 dataSz	= (UI32)[imgData length];
									 if(data != NULL && dataSz > 0){
									 c.thumbnailImageDataSz = dataSz;
									 NBString_strFreeAndNewBufferBytes((char**)&c.thumbnailImageData, (const char*)data, dataSz);
									 }
									 }
									 }*/
								}
								//Notify
								(*data->lstnrMethods.contactFound)(data->lstnr, &c);
								//Tmp: simulating more contacts
								/*{
									STNBString str;
									NBString_init(&str);
									{
										char* orgId = c.uid;
										SI32 i; for(i = 0; i < 8; i++){
											NBString_empty(&str);
											NBString_concat(&str, [contact.identifier UTF8String]);
											NBString_concat(&str, "_");
											NBString_concatSI32(&str, i);
											c.uid = str.str;
											//Notify
											(*data->lstnrMethods.contactFound)(data->lstnr, &c);
										}
										c.uid = orgId;
									}
									NBString_release(&str);
								}*/
								//Clear data
								NBStruct_stRelease(NBContact_getSharedStructMap(), &c, sizeof(c));
							}
						}
					}];
					if(r){
						PRINTF_INFO("Contacts, started contacts enumeration.\n");
					} else {
						NSLog(@"Contacts, error requesting enumation of contacts: '%@'.\n", error);
					}
				}
				[request release];
				request = nil;
			}
		}
	}
	return r;
}

ENContactsAuthStatus AUAppGlueAppleContacts::authStatus(void* pData, const BOOL requestIfNecesary){
	ENContactsAuthStatus r = ENContactsAuthStatus_Denied;
	AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)pData;
	if(data->requestingAuth){
		r = ENContactsAuthStatus_Requesting;
	} else if(data->store == NULL){
		r = ENContactsAuthStatus_Unavailable;
	} else {
		@autoreleasepool {
			CNAuthorizationStatus status = [CNContactStore authorizationStatusForEntityType:CNEntityTypeContacts];
			switch(status) {
				case CNAuthorizationStatusRestricted: //The user cannot change this
					PRINTF_INFO("Contacts, CNAuthorizationStatusRestricted.\n");
					r = ENContactsAuthStatus_Restricted;
					break;
				case CNAuthorizationStatusDenied:
					PRINTF_INFO("Contacts, CNAuthorizationStatusDenied.\n");
					r = ENContactsAuthStatus_Denied;
					break;
				case CNAuthorizationStatusAuthorized:
					//PRINTF_INFO("Contacts, CNAuthorizationStatusAuthorized.\n");
					r = ENContactsAuthStatus_Authorized;
					if(!data->lstnrSynced && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
						if(AUAppGlueAppleContacts_startSyncAll(data)){
							data->lstnrSynced = TRUE;
						}
					}
					break;
				default: //CNAuthorizationStatusNotDetermined
					r = ENContactsAuthStatus_NotDetermined;
					if(requestIfNecesary){
						r = ENContactsAuthStatus_Requesting;
						data->requestingAuth = TRUE;
						[data->store requestAccessForEntityType:CNEntityTypeContacts completionHandler:^(BOOL granted, NSError *error) {
							if(granted){
								PRINTF_INFO("Contacts, the user has granted authorization.\n");
								if(!data->lstnrSynced && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
									if(AUAppGlueAppleContacts_startSyncAll(data)){
										data->lstnrSynced = TRUE;
									}
								}
							} else {
								PRINTF_ERROR("Contacts, the user has denied authorization.\n");
							}
							data->requestingAuth = FALSE;
						}];
					} else {
						//PRINTF_INFO("Contacts, CNAuthorizationStatusNotDetermined.\n");
					}
					break;
			}
		}
	}
	return r;
}

//

void AUAppGlueAppleContacts::setListener(void* pData, void* lstnr, STNBContactLstnrMethods* lstnrMethods){
	AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)pData;
	//Set listener
	if(data->store != NULL){
		data->lstnr = lstnr;
		if(lstnrMethods == NULL){
			NBMemory_setZero(data->lstnrMethods);
		} else {
			data->lstnrMethods = *lstnrMethods;
			if(!data->lstnrSynced && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
				@autoreleasepool {
					CNAuthorizationStatus status = [CNContactStore authorizationStatusForEntityType:CNEntityTypeContacts];
					if(status == CNAuthorizationStatusAuthorized){
						if(AUAppGlueAppleContacts_startSyncAll(data)){
							data->lstnrSynced = TRUE;
						}
					}
				}
			}
		}
	}
}

void AUAppGlueAppleContacts::startSync(void* pData, const BOOL onlyIfNecesary){
	AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)pData;
	if(data->store != NULL && (!data->lstnrSynced || !onlyIfNecesary) && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
		if(AUAppGlueAppleContacts_startSyncAll(data)){
			data->lstnrSynced = TRUE;
		}
	}
}

void AUAppGlueAppleContacts::startThumbnailReq(void* pData, const char* contactId){
	AUAppGlueAppleContactsData* data = (AUAppGlueAppleContactsData*)pData;
	if(data->store != NULL && data->lstnr != NULL && data->lstnrMethods.thumbnailFound != NULL){
		@autoreleasepool {
			CNAuthorizationStatus status = [CNContactStore authorizationStatusForEntityType:CNEntityTypeContacts];
			if(status == CNAuthorizationStatusAuthorized){
				//keys with fetching properties
				NSArray* keys = [NSArray arrayWithObjects:
								 CNContactIdentifierKey
								 //, CNContactFamilyNameKey
								 //, CNContactGivenNameKey
								 //, CNContactOrganizationNameKey
								 //, CNContactPhoneNumbersKey
								 //, CNContactEmailAddressesKey
								 //, CNContactImageDataAvailableKey //10.12 (avoid)
								 , CNContactImageDataKey //Required in 10.11 if asking for 'CNContactThumbnailImageDataKey'
								 , CNContactThumbnailImageDataKey
								 , nil];
				CNContactFetchRequest *request = [[CNContactFetchRequest alloc] initWithKeysToFetch:keys];
				request.predicate = [CNContact predicateForContactsWithIdentifiers:[NSArray arrayWithObjects: [NSString stringWithUTF8String:contactId], nil]];
				{
					BOOL r = FALSE;
					NSError *error = nil;
					r = [data->store enumerateContactsWithFetchRequest:request error:&error usingBlock:^(CNContact * _Nonnull contact, BOOL * _Nonnull stop) {
						@autoreleasepool {
							if(contact != nil && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
								STNBContactImg c;
								NBMemory_setZeroSt(c, STNBContactImg);
								//Populate
								{
									//PRINTF_INFO("Contact (thumbnail) enumerated:\n");
									if([contact isKeyAvailable:CNContactIdentifierKey]){
										//PRINTF_INFO("identifier: '%s'.\n", [contact.identifier UTF8String]);
										NBString_strFreeAndNewBuffer(&c.uid, [contact.identifier UTF8String]);
									}
									/*if([contact isKeyAvailable:CNContactGivenNameKey]){
										//PRINTF_INFO("givenName: '%s'.\n", [contact.givenName UTF8String]);
										NBString_strFreeAndNewBuffer(&c.givenName, [contact.givenName UTF8String]);
									}
									if([contact isKeyAvailable:CNContactFamilyNameKey]){
										//PRINTF_INFO("familyName: '%s'.\n", [contact.familyName UTF8String]);
										NBString_strFreeAndNewBuffer(&c.familyName, [contact.familyName UTF8String]);
									}
									if([contact isKeyAvailable:CNContactOrganizationNameKey]){
										//PRINTF_INFO("organizationName: '%s'.\n", [contact.organizationName UTF8String]);
										NBString_strFreeAndNewBuffer(&c.organizationName, [contact.organizationName UTF8String]);
									}
									if([contact isKeyAvailable:CNContactPhoneNumbersKey]){
										if(contact.phoneNumbers.count > 0){
											c.phoneNumbers		= NBMemory_allocTypes(char*, contact.phoneNumbers.count);
											c.phoneNumbersSz	= (SI32)contact.phoneNumbers.count;
											SI32 i; for(i = 0 ; i < contact.phoneNumbers.count; i++){
												c.phoneNumbers[i] = NULL;
												{
													CNLabeledValue* lbl = contact.phoneNumbers[i];
													if(lbl != nil){
														CNPhoneNumber* num = lbl.value;
														if(num != NULL){
															NSString* str = num.stringValue;
															if(str != nil){
																//PRINTF_INFO("Phone: '%s'.\n", [str UTF8String]);
																NBString_strFreeAndNewBuffer(&c.phoneNumbers[i], [str UTF8String]);
															}
														}
													}
												}
											}
										}
									}
									if([contact isKeyAvailable:CNContactEmailAddressesKey]){
										if(contact.emailAddresses.count > 0){
											c.emailAddresses	= NBMemory_allocTypes(char*, contact.emailAddresses.count);
											c.emailAddressesSz	= (SI32)contact.emailAddresses.count;
											SI32 i; for(i = 0 ; i < contact.emailAddresses.count; i++){
												c.emailAddresses[i] = NULL;
												{
													CNLabeledValue* lbl = contact.emailAddresses[i];
													if(lbl != nil){
														NSString* str = lbl.value;
														if(str != nil){
															//PRINTF_INFO("Email: '%s'.\n", [str UTF8String]);
															NBString_strFreeAndNewBuffer(&c.emailAddresses[i], [str UTF8String]);
														}
													}
												}
											}
										}
									}*/
									//Images
									//if(contact.imageDataAvailable){ //10.12 (avoid)
									if([contact isKeyAvailable:CNContactThumbnailImageDataKey]){
										if(contact.thumbnailImageData != nil){
											NSData* imgData		= contact.thumbnailImageData;
											const void* data	= [imgData bytes];
											const UI32 dataSz	= (UI32)[imgData length];
											if(data != NULL && dataSz > 0){
												c.imgDataSz = dataSz;
												NBString_strFreeAndNewBufferBytes((char**)&c.imgData, (const char*)data, dataSz);
											}
										}
									}
									//}
								}
								//Notify
								(*data->lstnrMethods.thumbnailFound)(data->lstnr, &c);
								//Clear data
								NBStruct_stRelease(NBContactImg_getSharedStructMap(), &c, sizeof(c));
							}
						}
					}];
					if(r){
						//PRINTF_INFO("Contacts, started contacts-thumbnail enumeration.\n");
					} else {
						NSLog(@"Contacts, error requesting enumation-thumbnail of contacts: '%@'.\n", error);
					}
				}
				[request release];
				request = nil;
			}
		}
	}
}

@implementation AUAppContactNotifDelegate

- (id)initWithData:(struct AUAppGlueAppleContactsData_*)data {
	if(self = [super init]){
		_data = data;
	}
	return self;
}

- (void)contactDidChange:(NSNotification*)notif {
	_data->lstnrSynced = FALSE;
	NSLog(@"contactDidChange: %@.\n", notif.userInfo);
}

@end
