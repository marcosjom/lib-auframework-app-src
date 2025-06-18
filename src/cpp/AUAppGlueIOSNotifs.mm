//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrNotifs.h"
#include "AUAppGlueIOSNotifs.h"
//#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h> //for UNUserNotificationCenter
//
#import <AVFoundation/AVFoundation.h>

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

//-------------------------
// AUAppGlueIOSNotifs
//-------------------------

class AUAppGlueIOSNotifsListener;

typedef enum ENAppGlueIOSNotifsRemoteTokenStatus_ {
	ENAppGlueIOSNotifsRemoteTokenStatus_Idle = 0,
	ENAppGlueIOSNotifsRemoteTokenStatus_RequestingSettings,
	ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken,
	ENAppGlueIOSNotifsRemoteTokenStatus_Sucess,
	ENAppGlueIOSNotifsRemoteTokenStatus_Error,
	ENAppGlueIOSNotifsRemoteTokenStatus_Count
} ENAppGlueIOSNotifsRemoteTokenStatus;

typedef struct AUAppGlueIOSNotifsData_ {
	AUAppI*						app;
	AUAppGlueIOSNotifsListener* listener;
	//Auth
	struct {
		ENAppNotifAuthState		lastStateRetrieved;	//cache, last known state
		ENAppNotifAuthState		curReqState;		//current settings request state
		ENAppNotifAuthQueryMode	curReqMode;			//current settings request mode
	} auth;
	//Remote notifs
	struct {
		ENAppGlueIOSNotifsRemoteTokenStatus status;
		STNBString	token;
	} remote;
} AUAppGlueIOSNotifsData;

class AUAppGlueIOSNotifsListener: public AUAppNotifListener {
	public:
		AUAppGlueIOSNotifsListener(AUAppGlueIOSNotifsData* data){
			_data = data;
		}
		virtual ~AUAppGlueIOSNotifsListener(){
			_data = NULL;
		}
		//AUAppNotifListener
		void appLocalNotifReceived(AUAppI* app, void* notif /*UILocalNotification*/){
			AUAppGlueIOSNotifs::analyzeLocalNotification(_data, notif);
		}
	private:
		AUAppGlueIOSNotifsData* _data;
};

//New API delegate

//ToDo: why derivated from 'UIResponder', should be 'NSObject'?
@interface AUAppNotifDelegate : UIResponder <UNUserNotificationCenterDelegate> {
	//
}
+ (AUAppNotifDelegate*)sharedInstance;
@end

@implementation AUAppNotifDelegate

+ (AUAppNotifDelegate*)sharedInstance {
	static AUAppNotifDelegate* inst = nil;
	if(inst == nil){
		inst = [[AUAppNotifDelegate alloc] init];
	}
	return inst;
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center willPresentNotification:(UNNotification *)notification withCompletionHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler {
	@autoreleasepool {
		BOOL showNotification = FALSE;
		BOOL consumed = FALSE;
		{
			//Called when a notification is delivered to a foreground app.
			//If not defined, the system ignores de notification.
			if(notification){
				// The user launched the app.
				UNNotificationRequest* nReq = notification.request;
				if(nReq){
					UNNotificationContent* nContent = nReq.content;
					if(nContent){
						NSDictionary* info = nContent.userInfo;
						if(info != nil){
							BOOL isLocal = FALSE;
							{
								NSString* strIsLocal = [info objectForKey:@"isLocal"];
								if(strIsLocal != nil){
									isLocal = ([strIsLocal isEqualToString:@"1"] ? TRUE : FALSE);
								}
							}
							NSString* strUid = [info objectForKey:@"uid"];
							NSString* strGrp = [info objectForKey:@"grpId"];
							NSString* strLid = [info objectForKey:@"ntfId"];
							NSString* strData = [info objectForKey:@"data"];
							if(strUid != nil && strGrp != nil && strLid != nil){
								const char* data = NULL;
								if(strData != nil){
									data = [strData UTF8String];
								}
								NBMngrNotifs::addNotifRcvd(isLocal ? ENNotifType_Local : ENNotifType_Remote, [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
								PRINTF_INFO("AUAppGlueIOSNotifs, willPresentNotification (foreground-app) processed: uid(%d) '%s'::%d: '%s'.\n", [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
								consumed = TRUE;
							}
							if(isLocal){
								showNotification = TRUE;
							}
						}
					}
				}
				if(!consumed){
					PRINTF_ERROR("AUAppGlueIOSNotifs, willPresentNotification (foreground-app) not-processed.\n");
				}
			}
		}
		//Execute completionHandler
		if(completionHandler){
			if(showNotification){
				completionHandler(UNNotificationPresentationOptionBadge + UNNotificationPresentationOptionSound + UNNotificationPresentationOptionAlert);
			} else {
				completionHandler(UNNotificationPresentationOptionNone);
			}
		}
	}
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center didReceiveNotificationResponse:(UNNotificationResponse *)response withCompletionHandler:(void (^)(void))completionHandler {
	@autoreleasepool {
		//Called to let your app know which action was selected by the user for a given notification.
		if ([response.actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier]) {
			// The user dismissed the notification without taking action.
			PRINTF_INFO("AUAppGlueIOSNotifs, didReceiveNotificationResponse (not-foreground-app) ignored (user dismissed the notification).\n");
		} else if ([response.actionIdentifier isEqualToString:UNNotificationDefaultActionIdentifier]) {
			// The user launched the app.
			bool r = false;
			UNNotification* notif = response.notification;
			if(notif){
				UNNotificationRequest* nReq = notif.request;
				if(nReq){
					UNNotificationContent* nContent = nReq.content;
					if(nContent){
						NSDictionary* info = nContent.userInfo;
						if(info != nil){
							BOOL isLocal = FALSE;
							{
								NSString* strIsLocal = [info objectForKey:@"isLocal"];
								if(strIsLocal != nil){
									isLocal = ([strIsLocal isEqualToString:@"1"] ? TRUE : FALSE);
								}
							}
							NSString* strUid = [info objectForKey:@"uid"];
							NSString* strGrp = [info objectForKey:@"grpId"];
							NSString* strLid = [info objectForKey:@"ntfId"];
							NSString* strData = [info objectForKey:@"data"];
							if(strUid != nil && strGrp != nil && strLid != nil){
								const char* data = NULL;
								if(strData != nil){
									data = [strData UTF8String];
								}
								NBMngrNotifs::setLaunchNotif(isLocal ? ENNotifType_Local : ENNotifType_Remote, [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
								PRINTF_INFO("AUAppGlueIOSNotifs, didReceiveNotificationResponse (not-foreground-app) processed: uid(%d) '%s'::%d: '%s'.\n", [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
								r = true;
							}
						}
					}
				}
			}
			if(!r){
				PRINTF_ERROR("AUAppGlueIOSNotifs, didReceiveNotificationResponse (not-foreground-app) not-processed.\n");
			}
		} else {
			PRINTF_WARNING("AUAppGlueIOSNotifs, didReceiveNotificationResponse (not-foreground-app) unknown actionIdentifier '%s'.\n", [response.actionIdentifier UTF8String]);
		}
		//Execute completionHandler
		if(completionHandler){
			completionHandler();
		}
	}
}

@end

//


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

//

bool AUAppGlueIOSNotifs::analyzeLaunchOptions(void* pData, void* pLaunchOptions /*NSDictionary*/){
	bool r = false;
	@autoreleasepool {
		if(pLaunchOptions != NULL){
			NSDictionary* launchOptions = (NSDictionary*)pLaunchOptions;
			//
			//Old API (launched by a local notification)
			if ([UIApplication instancesRespondToSelector:@selector(scheduleLocalNotification:)]){
				UILocalNotification* notif = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
				if (notif != nil) {
					NSDictionary* info = notif.userInfo;
					if(info != nil){
						BOOL isLocal = FALSE;
						{
							NSString* strIsLocal = [info objectForKey:@"isLocal"];
							if(strIsLocal != nil){
								isLocal = ([strIsLocal isEqualToString:@"1"] ? TRUE : FALSE);
							}
						}
						NSString* strUid = [info objectForKey:@"uid"];
						NSString* strGrp = [info objectForKey:@"grpId"];
						NSString* strLid = [info objectForKey:@"ntfId"];
						NSString* strData = [info objectForKey:@"data"];
						if(strUid != nil && strGrp != nil && strLid != nil){
							const char* data = NULL;
							if(strData != nil){
								data = [strData UTF8String];
							}
							NBMngrNotifs::setLaunchNotif(isLocal ? ENNotifType_Local : ENNotifType_Remote, [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
							PRINTF_INFO("AUAppGlueIOSNotifs, launchOptions with localNotification record: uid(%d) '%s'::%d: '%s'.\n", [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
						}
					}
				} else {
					PRINTF_INFO("AUAppGlueIOSNotifs, launchOptions with no localNotification record.\n");
				}
			}
		}
	}
	return r;
}

bool AUAppGlueIOSNotifs::analyzeLocalNotification(void* pData, void* pLocalNotif /*UILocalNotification*/){
	bool r = false;
	if(pData != NULL && pLocalNotif != NULL){
		@autoreleasepool {
			AUAppGlueIOSNotifsData* data = (AUAppGlueIOSNotifsData*)pData;
			UIApplication* application = (UIApplication*)data->app->getApplication(); NBASSERT(application != NULL)
			if(application != NULL){
				UILocalNotification* notif = (UILocalNotification*)pLocalNotif;
				if(application.applicationState == UIApplicationStateInactive){
					PRINTF_INFO("AUAppGlueIOSNotifs, analyzeLocalNotification while app is INACTIVE.\n");
				} else if(application.applicationState == UIApplicationStateActive){
					PRINTF_INFO("AUAppGlueIOSNotifs, analyzeLocalNotification while app is ACTIVE.\n");
				} else if(application.applicationState == UIApplicationStateBackground){
					PRINTF_INFO("AUAppGlueIOSNotifs, analyzeLocalNotification while app is in BACKGROUND.\n");
				} else {
					PRINTF_INFO("AUAppGlueIOSNotifs, analyzeLocalNotification while app in UNKNOWN state: '%d'.\n", (SI32)application.applicationState);
				}
				//Process notification
				bool r = false;
				if(notif){
					NSDictionary* info = notif.userInfo;
					if(info != nil){
						BOOL isLocal = FALSE;
						{
							NSString* strIsLocal = [info objectForKey:@"isLocal"];
							if(strIsLocal != nil){
								isLocal = ([strIsLocal isEqualToString:@"1"] ? TRUE : FALSE);
							}
						}
						NSString* strUid = [info objectForKey:@"uid"];
						NSString* strGrp = [info objectForKey:@"grpId"];
						NSString* strLid = [info objectForKey:@"ntfId"];
						NSString* strData = [info objectForKey:@"data"];
						if(strUid != nil && strGrp != nil && strLid != nil){
							const char* data = NULL;
							if(strData != nil){
								data = [strData UTF8String];
							}
							NBMngrNotifs::addNotifRcvd(isLocal ? ENNotifType_Local : ENNotifType_Remote, [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
							PRINTF_INFO("AUAppGlueIOSNotifs, analyzeLocalNotification processed: uid(%d) '%s'::%d: '%s'.\n", [strUid intValue], [strGrp UTF8String], [strLid intValue], data);
							r = true;
						}
					}
				}
				if(!r){
					PRINTF_ERROR("AUAppGlueIOSNotifs, analyzeLocalNotification not-processed.\n");
				}
				//
				[application cancelLocalNotification: notif];
			}
		}
	}
	return r;
}

//Calls

bool AUAppGlueIOSNotifs::create(AUAppI* app, STMngrNotifsCalls* obj) {
	AUAppGlueIOSNotifsData* data = (AUAppGlueIOSNotifsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSNotifsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueIOSNotifsData);
	NBMemory_setZeroSt(*obj, STMngrNotifsCalls);
	data->app		= app;
	data->listener	= new AUAppGlueIOSNotifsListener(data);
	data->app->addAppNotifListener(data->listener);
	//Auth
	{
		data->auth.lastStateRetrieved	= ENAppNotifAuthState_Count;
		data->auth.curReqState			= ENAppNotifAuthState_Count;
		data->auth.curReqMode			= ENAppNotifAuthQueryMode_Count;
	}
	//Remote
	{
		data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_Idle;
		NBString_init(&data->remote.token);
	}
	//
	obj->funcCreate				= create;
	obj->funcCreateParam		= data;
	obj->funcDestroy			= destroy;
	obj->funcDestroyParam		= data;
	//
	obj->funcLoadData			= loadData;
	obj->funcLoadDataParam		= data;
	obj->funcSaveData			= saveData;
	obj->funcSaveDataParam		= data;
	obj->funcSetBadgeNumber		= setBadgeNumber;
	obj->funcSetBadgeNumberParam = data;
	obj->funcGetAuthStatus		= getAuthStatus;
	obj->funcGetAuthStatusParam = data;
	//local notifications
	obj->funcLocalRescheduleAll	= localRescheduleAll;
	obj->funcLocalRescheduleAllParam = data;
	obj->funcLocalEnable		= localEnable;
	obj->funcLocalEnableParam	= data;
	obj->funcLocalAdd			= localAdd;
	obj->funcLocalAddParam		= data;
	obj->funcLocalCancel		= localCancel;
	obj->funcLocalCancelParam	= data;
	obj->funcLocalCancelGrp		= localCancelGrp;
	obj->funcLocalCancelGrpParam = data;
	obj->funcLocalCancelAll		= localCancelAll;
	obj->funcLocalCancelAllParam = data;
	//remote notifications
	obj->funcRemoteGetToken		= remoteGetToken;
	obj->funcRemoteGetTokenParam = data;
	obj->funcRemoteSetToken		= remoteSetToken;
	obj->funcRemoteSetTokenParam = data;
	//
	@autoreleasepool {
		//New API
		if (NSClassFromString(@"UNUserNotificationCenter") != nil) {
			UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
			[center setDelegate:[AUAppNotifDelegate sharedInstance]];
		}
	}
	//Analyze launch options
	{
		void* launchOps = data->app->getLaunchOptions();
		if(launchOps == NULL){
			PRINTF_WARNING("AUAppGlueIOSNotifs, app's launch options is not set yet, canot determine if the app was launched by touching a notification (call AUApp::setLaunchOptions before linking to this glue).\n");
		} else {
			AUAppGlueIOSNotifs::analyzeLaunchOptions(data, launchOps);
		}
	}
	return true;
}

bool AUAppGlueIOSNotifs::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSNotifsData* data = (AUAppGlueIOSNotifsData*)pData;
		if(data->listener != NULL){
			data->app->removeAppNotifListener(data->listener);
			delete data->listener;
			data->listener = NULL;
		}
		//Remote
		{
			NBString_release(&data->remote.token);
		}
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueIOSNotifs::loadData(void* pData){
	bool r = false;
	AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnCache("_notifsData.bin"), ENArchivoModo_SoloLectura);
	if(file != NULL){
		AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		AUArchivoCrypt* fileCrypt = new(ENMemoriaTipo_Temporal) AUArchivoCrypt(file, "AUAppGlueIOSNotifs");
		fileCrypt->lock();
		char buff[4096];
		do {
			const SI32 read = fileCrypt->leer(buff, sizeof(char), 4096, fileCrypt);
			if(read <= 0) break;
			str->agregar(buff, read);
		} while(1);
		//
		if(NBMngrNotifs::lockedLoadFromJSON(str->str())){
			r = true;
		}
		//
		fileCrypt->unlock();
		fileCrypt->cerrar();
		fileCrypt->liberar(NB_RETENEDOR_NULL);
		file->cerrar();
		str->liberar(NB_RETENEDOR_NULL);
	}
	return r;
}

bool AUAppGlueIOSNotifs::saveData(void* pData){
	bool r = false;
	AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	if(NBMngrNotifs::lockedSaveToJSON(str)){
		AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnCache("_notifsData.bin"), ENArchivoModo_SoloEscritura);
		if(file != NULL){
			AUArchivoCrypt* fileCrypt = new(ENMemoriaTipo_Temporal) AUArchivoCrypt(file, "AUAppGlueIOSNotifs");
			fileCrypt->lock();
			fileCrypt->escribir(str->str(), sizeof(char), str->tamano(), fileCrypt);
			fileCrypt->unlock();
			fileCrypt->cerrar();
			fileCrypt->liberar(NB_RETENEDOR_NULL);
			file->cerrar();
			r = true;
		}
	}
	str->liberar(NB_RETENEDOR_NULL);
	return r;
}

bool AUAppGlueIOSNotifs::setBadgeNumber(void* pData, const SI32 num){
	@autoreleasepool {
		[UIApplication sharedApplication].applicationIconBadgeNumber = num;
	}
	return true;
}

ENAppNotifAuthState AUAppGlueIOSNotifs::getAuthStatus(void* pData, const ENAppNotifAuthQueryMode reqMode){
	ENAppNotifAuthState r = ENAppNotifAuthState_Denied;
	if(pData != NULL){
		@autoreleasepool {
			AUAppGlueIOSNotifsData* data = (AUAppGlueIOSNotifsData*)pData;
			//Set return value
			r = (reqMode == ENAppNotifAuthQueryMode_CurrentState ? data->auth.curReqState : data->auth.lastStateRetrieved);
			//Start settings request
			{
				const BOOL isFirstSettingsUpdate = (data->auth.lastStateRetrieved == ENAppNotifAuthState_Count);
				const BOOL isRequesting = (data->auth.curReqState == ENAppNotifAuthState_QueringSettings || data->auth.curReqState == ENAppNotifAuthState_Requesting);
				if(!isRequesting && (isFirstSettingsUpdate || reqMode == ENAppNotifAuthQueryMode_UpdateCache || reqMode == ENAppNotifAuthQueryMode_UpdateCacheRequestIfNecesary)){
					BOOL reqStarted = FALSE;
					//Start request settings
					if(NSClassFromString(@"UNUserNotificationCenter") != nil) {
						UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
						if(center != nil){
							reqStarted				= TRUE;
							data->auth.curReqState	= ENAppNotifAuthState_QueringSettings;
							data->auth.curReqMode	= reqMode;
							if(isFirstSettingsUpdate){
								data->auth.lastStateRetrieved = data->auth.curReqState;
							}
							//Set return value
							r = (reqMode == ENAppNotifAuthQueryMode_CurrentState ? data->auth.curReqState : data->auth.lastStateRetrieved);
							//
							[center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings* settings){
								const UNAuthorizationStatus status = [settings authorizationStatus];
								switch (status) {
									case UNAuthorizationStatusDenied:
										data->auth.lastStateRetrieved = data->auth.curReqState = ENAppNotifAuthState_Denied;
										break;
									case UNAuthorizationStatusAuthorized:
									case UNAuthorizationStatusProvisional:
										data->auth.lastStateRetrieved = data->auth.curReqState = ENAppNotifAuthState_Authorized;
										break;
									case UNAuthorizationStatusNotDetermined:
										//Start auth request
										if(data->auth.curReqMode != ENAppNotifAuthQueryMode_UpdateCacheRequestIfNecesary){
											data->auth.lastStateRetrieved = data->auth.curReqState = ENAppNotifAuthState_NotDetermined;
										} else {
											data->auth.lastStateRetrieved	= ENAppNotifAuthState_NotDetermined;
											data->auth.curReqState			= ENAppNotifAuthState_Requesting;
											if(isFirstSettingsUpdate){
												data->auth.lastStateRetrieved = data->auth.curReqState;
											}
											[center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert + UNAuthorizationOptionBadge + UNAuthorizationOptionSound)
																  completionHandler:^(BOOL granted, NSError * _Nullable error) {
																	  // Enable or disable features based on authorization.
																	  if(granted){
																		  PRINTF_INFO("AUAppGlueIOSNotifs, notifications permission granted.\n");
																		  data->auth.lastStateRetrieved = data->auth.curReqState = ENAppNotifAuthState_Authorized;
																	  } else {
																		  data->auth.lastStateRetrieved = data->auth.curReqState = ENAppNotifAuthState_Denied;
																		  if (error != nil) {
																			  PRINTF_ERROR("AUAppGlueIOSNotifs, notifications permission NOT-granted: '%s'.\n", [error.localizedDescription UTF8String]);
																		  } else {
																			  PRINTF_ERROR("AUAppGlueIOSNotifs, notifications permission NOT-granted.\n");
																		  }
																	  }
																  }];
										}
										break;
									default:
										break;
								}
							}];
						}
					}
					//Error result
					if(!reqStarted){
						data->auth.curReqState	= ENAppNotifAuthState_Denied;
						if(isFirstSettingsUpdate){
							r = data->auth.lastStateRetrieved = ENAppNotifAuthState_Denied;
						}
					}
				}
			}
		}
	}
	return r;
}

//Local notifications

bool AUAppGlueIOSNotifs::localRescheduleAll(void* pData){
	//Is automatic on iOS?
	return true;
}

bool AUAppGlueIOSNotifs::localEnable(void* pData){
	bool r = false;
	@autoreleasepool {
		//New API
		if(!r){
			if (NSClassFromString(@"UNUserNotificationCenter") != nil) {
				UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
				[center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert + UNAuthorizationOptionBadge + UNAuthorizationOptionSound)
									  completionHandler:^(BOOL granted, NSError * _Nullable error) {
										  // Enable or disable features based on authorization.
										  if(granted){
											  PRINTF_INFO("AUAppGlueIOSNotifs, local notifications permission granted.\n");
										  } else {
											  if (error != nil) {
												  PRINTF_ERROR("AUAppGlueIOSNotifs, local notifications permission NOT-granted: '%s'.\n", [error.localizedDescription UTF8String]);
											  } else {
												  PRINTF_ERROR("AUAppGlueIOSNotifs, local notifications permission NOT-granted.\n");
											  }
										  }
									  }];
				PRINTF_INFO("AUAppGlueIOSNotifs, localEnable using new API.\n");
				r = true;
			}
		}
		//Old API
		if(!r){
			if ([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)]){
				[[UIApplication sharedApplication] registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert | UIUserNotificationTypeBadge | UIUserNotificationTypeSound categories:nil]];
				PRINTF_INFO("AUAppGlueIOSNotifs, localEnable using old API.\n");
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSNotifs::localAdd(void* pData, const SI32 uniqueId, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData){
	bool r = false;
	@autoreleasepool {
		//New API
		if(!r && grpId != NULL){
			if (NSClassFromString(@"UNUserNotificationCenter") != nil) {
				NSString* objData	= @""; if(notifData != NULL) objData = [NSString stringWithUTF8String:notifData];
				UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
				UNMutableNotificationContent* nContent = [[UNMutableNotificationContent alloc] init];
				if(title != NULL){
					if(title[0] != '\0'){
						nContent.title	= [NSString stringWithUTF8String:title];
					}
				}
				if(content != NULL){
					if(content[0] != '\0'){
						nContent.body	= [NSString stringWithUTF8String:content];
					}
				}
				nContent.userInfo	= [NSDictionary dictionaryWithObjectsAndKeys: @"1", @"isLocal", [NSString stringWithFormat:@"%d", uniqueId], @"uid", [NSString stringWithUTF8String:grpId], @"grpId", [NSString stringWithFormat:@"%d", notifId], @"ntfId", objData, @"data", nil];
				nContent.sound		= [UNNotificationSound defaultSound];
				nContent.badge		= [NSNumber numberWithInt:1];
				// Configure the trigger
				NSDateComponents* comps = [[NSDateComponents alloc] init];
				[comps setSecond:secsFromNow];
				NSDate* date0 = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:[NSDate date] options:0];
				NSDateComponents* date = [[NSCalendar currentCalendar] components:(NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond) fromDate: date0];
				//NSTimeInterval tzSecs = [[NSTimeZone localTimeZone] secondsFromGMT]; //Current timeZone secs
				//NSDate* date0		= [NSDate dateWithTimeIntervalSinceNow:(secsFromNow /*+ tzSecs*/)]; //Add timeZone secs
				//NSDateComponents* date = [[NSCalendar currentCalendar] components:NSCalendarUnitYear + NSCalendarUnitMonth + NSCalendarUnitDay + NSCalendarUnitHour + NSCalendarUnitMinute + NSCalendarUnitSecond fromDate: date0];
				UNCalendarNotificationTrigger* trigger = [UNCalendarNotificationTrigger triggerWithDateMatchingComponents:date repeats:NO];
				// Create the request object.
				UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier:[NSString stringWithFormat:@"%s::%d", (grpId != NULL ? grpId : ""), notifId] content:nContent trigger:trigger];
				[center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
					if (error != nil) {
						PRINTF_ERROR("AUAppGlueIOSNotifs, could not add local notification: '%s'.\n", [error.localizedDescription UTF8String]);
					} else {
						PRINTF_INFO("AUAppGlueIOSNotifs, local notification added.\n");
					}
				}];
				PRINTF_INFO("AUAppGlueIOSNotifs, localAdd using new API: +%d secs, date(utc:%d-%d-%d %d:%d:%d, local: '%s') '%s'::%d: '%s', '%s', '%s'.\n", (SI32)secsFromNow, (SI32)date.year, (SI32)date.month, (SI32)date.day, (SI32)date.hour, (SI32)date.minute, (SI32)date.second, [[date0 description] UTF8String], (grpId != NULL ? grpId : ""), notifId, title, content, notifData);
				//NSLog(@"AUAppGlueIOSNotifs, localAdd using new API: date(%d-%d-%d %d:%d:%d, '%s') '%s'::%d: '%s', '%s', '%s'.\n", (SI32)date.year, (SI32)date.month, (SI32)date.day, (SI32)date.hour, (SI32)date.minute, (SI32)date.second, [[date0 description] UTF8String], (grpId != NULL ? grpId : ""), notifId, title, content, notifData);
				[comps release];
				r = true;
			}
		}
		//Old API
		if(!r && grpId != NULL){
			if ([UIApplication instancesRespondToSelector:@selector(scheduleLocalNotification:)]){
				UILocalNotification *notification = [[UILocalNotification alloc] init];
				notification.fireDate		= [NSDate dateWithTimeIntervalSinceNow:secsFromNow];
				notification.timeZone		= [NSTimeZone defaultTimeZone];
				//notification.alertAction	= @"Leer";
				if(title != NULL){
					if(title[0] != '\0'){
						if ([UILocalNotification instancesRespondToSelector:@selector(setAlertTitle:)]){
							notification.alertTitle = [NSString stringWithUTF8String:title];
						}
					}
				}
				if(content != NULL){
					if(content[0] != '\0'){
						if ([UILocalNotification instancesRespondToSelector:@selector(setAlertBody:)]){
							notification.alertBody	= [NSString stringWithUTF8String:content];
						}
					}
				}
				NSString* objData			= @""; if(notifData != NULL) objData = [NSString stringWithUTF8String:notifData];
				notification.userInfo		= [NSDictionary dictionaryWithObjectsAndKeys: @"1", @"isLocal", [NSString stringWithFormat:@"%d", uniqueId], @"uid", [NSString stringWithUTF8String:grpId], @"grpId", [NSString stringWithFormat:@"%d", notifId], @"ntfId", objData, @"data", nil];
				notification.soundName		= UILocalNotificationDefaultSoundName;
				notification.applicationIconBadgeNumber = 1;
				[[UIApplication sharedApplication] scheduleLocalNotification:notification];
				PRINTF_INFO("AUAppGlueIOSNotifs, localAdd using old API: '%s'::%d: '%s', '%s', '%s'.\n", (grpId != NULL ? grpId : ""), notifId, title, content, notifData);
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSNotifs::localCancel(void* pData, const char* grpId, const SI32 notifId){
	bool r = false;
	@autoreleasepool {
		//New API
		if(!r){
			if (NSClassFromString(@"UNUserNotificationCenter") != nil) {
				UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
				[center removePendingNotificationRequestsWithIdentifiers:[NSArray arrayWithObjects:[NSString stringWithFormat:@"%s::%d", (grpId != NULL ? grpId : ""), notifId], nil]];
				PRINTF_INFO("AUAppGlueIOSNotifs, local notifications with id '%s::%d' removed using new API.\n", (grpId != NULL ? grpId : ""), notifId);
				r = true;
			}
		}
		//Old API
		if(!r && grpId != NULL){
			if ([UIApplication instancesRespondToSelector:@selector(cancelLocalNotification:)]){
				NSString* nsGrupo = [NSString stringWithUTF8String:grpId];
				NSArray* localNotifs  = [[UIApplication sharedApplication] scheduledLocalNotifications];
				SI32 i; const SI32 conteo = (SI32)localNotifs.count;
				for(i = 0; i < conteo; i++){
					UILocalNotification* notif = (UILocalNotification*)[localNotifs objectAtIndex: i];
					NSDictionary* info = notif.userInfo;
					if(info != nil){
						NSString* strGrp = [info objectForKey:@"grpId"];
						NSString* strLid = [info objectForKey:@"ntfId"];
						if(strGrp != nil && strLid != nil){
							if([strGrp isEqualToString:nsGrupo] && [strLid intValue] == notifId){
								[[UIApplication sharedApplication] cancelLocalNotification:notif];
								//PRINTF_INFO("AUAppGlueIOSNotifs, notificacion especifica cancelada: '%s':%d.\n", grupo, idInterno);
							}
						}
					}
				}
				PRINTF_INFO("AUAppGlueIOSNotifs, %d de %d local notifications canceled with old API (specific search).\n", conteo - (SI32)[[UIApplication sharedApplication] scheduledLocalNotifications].count, conteo);
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSNotifs::localCancelGrp(void* pData, const char* grpId){
	bool r = false;
	@autoreleasepool {
		//New API
		if(!r){
			if (NSClassFromString(@"UNUserNotificationCenter") != nil) {
				UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
				[center getPendingNotificationRequestsWithCompletionHandler:^(NSArray<UNNotificationRequest *> * _Nonnull reqs) {
					if(reqs != nil){
						//Build list
						NSMutableArray* reqsToRemove = [[NSMutableArray alloc] init];
						SI32 i; NSString* strStart = [NSString stringWithFormat:@"%s::", (grpId != NULL ? grpId : "")];
						for(i = ((SI32)reqs.count - 1); i >= 0; i--){
							UNNotificationRequest* req = [reqs objectAtIndex:(NSUInteger)i];
							if(req != NULL){
								NSRange rng = [req.identifier rangeOfString:strStart];
								if(rng.location == 0 && rng.length != 0){
									[reqsToRemove addObject:[NSString stringWithString:req.identifier]];
								}
							}
						}
						//Remove
						if(reqsToRemove.count > 0){
							[center removePendingNotificationRequestsWithIdentifiers:reqsToRemove];
						}
						PRINTF_INFO("AUAppGlueIOSNotifs, %d local notifications with id '%s::*' removed using new API.\n", (SI32)reqsToRemove.count, (grpId != NULL ? grpId : ""));
					}
				}];
				r = true;
			}
		}
		//Old API
		if(!r && grpId != NULL){
			if ([UIApplication instancesRespondToSelector:@selector(cancelLocalNotification:)]){
				NSString* nsGrupo = [NSString stringWithUTF8String:grpId];
				NSArray* localNotifs  = [[UIApplication sharedApplication] scheduledLocalNotifications];
				SI32 i; const SI32 conteo = (SI32)localNotifs.count;
				for(i = 0; i < conteo; i++){
					UILocalNotification* notif = (UILocalNotification*)[localNotifs objectAtIndex: i];
					NSDictionary* info = notif.userInfo; NBASSERT(info != nil)
					if(info != nil){
						NSString* strGrp = [info objectForKey:@"grpId"]; NBASSERT(strGrp != nil)
						if(strGrp != nil){
							if([strGrp isEqualToString:nsGrupo]){
								[[UIApplication sharedApplication] cancelLocalNotification:notif];
								//PRINTF_INFO("AUAppGlueIOSNotifs, notificacion de grupo cancelada: '%s'.\n", grupo);
							}
						}
					}
				}
				PRINTF_INFO("AUAppGlueIOSNotifs, %d de %d local notifications canceled with old API (group search).\n", conteo - (SI32)[[UIApplication sharedApplication] scheduledLocalNotifications].count, conteo);
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSNotifs::localCancelAll(void* pData){
	bool r = false;
	@autoreleasepool {
		//New API
		if(!r){
			if (NSClassFromString(@"UNUserNotificationCenter") != nil) {
				UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
				[center removeAllPendingNotificationRequests];
				PRINTF_INFO("AUAppGlueIOSNotifs, ALL local notifications canceled with new API.\n");
				r = true;
			}
		}
		//Old API
		if(!r){
			if ([UIApplication instancesRespondToSelector:@selector(cancelAllLocalNotifications)]){
				[[UIApplication sharedApplication] cancelAllLocalNotifications];
				PRINTF_INFO("AUAppGlueIOSNotifs, ALL local notifications canceled with old API.\n");
				r = true;
			}
		}
	}
	return r;
}

//remote notifications

bool AUAppGlueIOSNotifs::remoteGetToken(void* pData, const ENAppNotifTokenQueryMode mode, STNBString* dst){
	bool r = false;
	if(pData != NULL){
		@autoreleasepool {
			AUAppGlueIOSNotifsData* data = (AUAppGlueIOSNotifsData*)pData;
			{
				if(dst != NULL){
					NBString_setBytes(dst, data->remote.token.str, data->remote.token.length);
				}
				r = TRUE;
			}
			//Start request
			if(data->auth.lastStateRetrieved == ENAppNotifAuthState_Authorized){
				if(mode == ENAppNotifTokenQueryMode_ForceNewRequest || (mode == ENAppNotifTokenQueryMode_CacheAndRequestIfNecesary && data->remote.token.length <= 0)){
					if(data->remote.status != ENAppGlueIOSNotifsRemoteTokenStatus_RequestingSettings && data->remote.status != ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken){
						//Start token request
						UIApplication* app = (UIApplication*)data->app->getApplication();
						if(app == nil){
							//Error, cannot be requested
							PRINTF_ERROR("AUAppGlueIOSNotifs, app not set.\n");
							data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_Error;
						} else {
							data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken;
							PRINTF_INFO("AUAppGlueIOSNotifs, calling registerForRemoteNotifications.\n");
							[app registerForRemoteNotifications];
						}
					}
				}
			}
		}
	}
	return r;
}

void AUAppGlueIOSNotifs::remoteSetToken(void* pData, const void* token, const UI32 tokenSz){
	if(pData != NULL){
		@autoreleasepool {
			AUAppGlueIOSNotifsData* data = (AUAppGlueIOSNotifsData*)pData;
			if(token == NULL || tokenSz <= 0){
				if(data->remote.status == ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken){
					data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_Error;
				}
			} else {
				PRINTF_INFO("AUAppGlueIOSNotifs, remoteSetToken(%d bytes).\n", tokenSz);
				NBString_setBytes(&data->remote.token, (const char*)token, tokenSz);
				if(data->remote.status == ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken){
					data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_Sucess;
				}
				
			}
		}
	}
}
