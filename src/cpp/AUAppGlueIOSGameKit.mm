//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrGameKit.h"
#include "AUAppGlueIOSGameKit.h"
//#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

//-------------------------
// AUAppGlueIOSGameKit
//-------------------------

typedef struct AUAppGlueIOSGameKitData_ {
	AUAppI* app;
} AUAppGlueIOSGameKitData;

@interface AUAppGameKitDelegate : NSObject<GKGameCenterControllerDelegate, GKLeaderboardViewControllerDelegate> {
	ENMngrGameKitAuthState		_authState;
	AUAppGlueIOSGameKitData*	_glueData;
}
+ (AUAppGameKitDelegate*)sharedInstance;
@end

@implementation AUAppGameKitDelegate

+ (AUAppGameKitDelegate*)sharedInstance {
	static AUAppGameKitDelegate* inst = nil;
	if(inst == nil){
		inst = [[AUAppGameKitDelegate alloc] init];
	}
	return inst;
}

- (void)setGlueData:(AUAppGlueIOSGameKitData*)data {
	_glueData = data;
}

- (UIViewController*)getParentViewController {
	UIViewController* r = nil;
	if(_glueData != NULL){
		if(_glueData->app != NULL){
			r = (UIViewController*)_glueData->app->getViewController();
		}
	}
	return r;
}

- (ENMngrGameKitAuthState)getAuthState {
	return _authState;
}

- (void)startAuthentication {
	if ([GKLocalPlayer localPlayer].authenticated == NO) {
		_authState = ENMngrGameKitAuthState_Quering;
		[[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:nil];
	} else {
		_authState = ENMngrGameKitAuthState_LogedIn;
	}
}

- (void)authenticationChanged {
	if ([GKLocalPlayer localPlayer].isAuthenticated) {
		PRINTF_INFO("Player authenticated in GameCenter.\n");
		_authState = ENMngrGameKitAuthState_LogedIn;
		//Consultar progreso de logros desde GameCenter
		/*if((NSClassFromString(@"GKAchievement"))){
			if(![GKAchievement respondsToSelector:@selector(loadAchievementsWithCompletionHandler:)]){
				PRINTF_WARNING("No se pueden consultar LOGROS desde GameCenter (clase no responde a selector).\n");
			} else {
				[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error) {
					if (error == nil && achievements != nil) {
						for (GKAchievement* achievement in achievements){
							NBGestorJuego::establecerProgresoLogro([[achievement identifier] UTF8String], [achievement percentComplete], ENOrigenDatos_GameCenter);
						}
					} else {
						PRINTF_ERROR("Al consultar LOGROS desde GameCenter.\n");
					}
				}];
			}
		}*/
		//Consultar scores desde GameCenter
		/*if((NSClassFromString(@"GKLeaderboard"))){
			if(![GKLeaderboard respondsToSelector:@selector(loadLeaderboardsWithCompletionHandler:)]){
				PRINTF_WARNING("No se pueden consultar SCORES desde GameCenter (clase no responde a selector).\n");
			} else {
				[GKLeaderboard loadLeaderboardsWithCompletionHandler:^(NSArray *leaderboards, NSError *error) {
					if(error == nil && leaderboards != nil){
						for (GKLeaderboard *score in leaderboards){
							if([score respondsToSelector:@selector(identifier)]){
								NBGestorJuego::procesaScore((UI16)[[score localPlayerScore] value], [[score identifier] UTF8String], ENOrigenDatos_GameCenter);
							} else {
								NBGestorJuego::procesaScore((UI16)[[score localPlayerScore] value], [[score category] UTF8String], ENOrigenDatos_GameCenter);
							}
						}
					} else {
						PRINTF_ERROR("Al consultar SCORES desde GameCenter.\n");
					}
				}];
			}
		}*/
		PRINTF_INFO("--------------------------------------\n");
	} else {
		PRINTF_INFO("Player loged-out from GameCenter.\n");
		_authState = ENMngrGameKitAuthState_LogedOut;
	}
}

- (void)gameCenterViewControllerDidFinish:(GKGameCenterViewController *)gameCenterViewController {
	[gameCenterViewController dismissViewControllerAnimated:YES completion:nil];
	//[self crearTimers];
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController {
	[viewController dismissViewControllerAnimated:YES completion:nil];
	//[self crearTimers];
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

bool AUAppGlueIOSGameKit::create(AUAppI* app, STMngrGameKitCalls* obj){
	AUAppGlueIOSGameKitData* data = (AUAppGlueIOSGameKitData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSGameKitData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueIOSGameKitData);
	NBMemory_setZeroSt(*obj, STMngrGameKitCalls);
	data->app = (AUAppI*)app;
	//
	obj->funcCreate				= create;
	obj->funcCreateParam		= NULL;
	obj->funcDestroy			= destroy;
	obj->funcDestroyParam		= NULL;
	//
	obj->funcLoadData			= loadData;
	obj->funcLoadDataParam		= NULL;
	obj->funcSaveData			= saveData;
	obj->funcSaveDataParam		= NULL;
	obj->funcAuthState			= authenticationState;
	obj->funcAuthStateParam		= NULL;
	obj->funcAuthenticate		= startAuthentication;
	obj->funcAuthenticateParam	= NULL;
	obj->funcGetLocalPlayer		= getLocalPlayer;
	obj->funcGetLocalPlayerParam = NULL;
	obj->funcShowCenter			= showCenter;
	obj->funcShowCenterParam	= NULL;
	obj->funcShowLeaderbrd		= showLeaderboard;
	obj->funcShowLeaderbrdParam	= NULL;
	obj->funcSendScore			= sendScoreAndReport;
	obj->funcSendScoreParam		= NULL;
	obj->funcSendAchiev			= sendAchievProgressAndReport;
	obj->funcSendAchievParam	= NULL;
	//Configure
	{
		AUAppGameKitDelegate* delegate = [AUAppGameKitDelegate sharedInstance];
		[delegate setGlueData:data];
		//Register as listener
		NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
		[nc removeObserver:delegate];
		[nc addObserver:delegate
			   selector:@selector(authenticationChanged)
				   name:GKPlayerAuthenticationDidChangeNotificationName
				 object:nil];
	}
	//
	return true;
}

bool AUAppGlueIOSGameKit::destroy(void* pData){
	bool r = false;
	//Remove as listener
	{
		AUAppGameKitDelegate* delegate = [AUAppGameKitDelegate sharedInstance];
		NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
		[nc removeObserver:delegate];
	}
	if(pData != NULL){
		AUAppGlueIOSGameKitData* data = (AUAppGlueIOSGameKitData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return true;
}

bool AUAppGlueIOSGameKit::loadData(void* pData){
	bool r = false;
	AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnCache("_gameKitData.bin"), ENArchivoModo_SoloLectura);
	if(file != NULL){
		AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		AUArchivoCrypt* fileCrypt = new(ENMemoriaTipo_Temporal) AUArchivoCrypt(file, "AUAppGlueIOSGameKit");
		fileCrypt->lock();
		char buff[4096];
		do {
			const SI32 read = fileCrypt->leer(buff, sizeof(char), 4096, fileCrypt);
			if(read <= 0) break;
			str->agregar(buff, read);
		} while(1);
		//
		if(NBMngrGameKit::lockedLoadFromJSON(str->str())){
			r = true;
		}
		//
		fileCrypt->unlock();
		fileCrypt->cerrar();
		fileCrypt->liberar(NB_RETENEDOR_THIS);
		file->cerrar();
		str->liberar(NB_RETENEDOR_THIS);
	}
	return r;
}

bool AUAppGlueIOSGameKit::saveData(void* pData){
	bool r = false;
	AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	if(NBMngrGameKit::lockedSaveToJSON(str)){
		AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnCache("_gameKitData.bin"), ENArchivoModo_SoloEscritura);
		if(file != NULL){
			AUArchivoCrypt* fileCrypt = new(ENMemoriaTipo_Temporal) AUArchivoCrypt(file, "AUAppGlueIOSGameKit");
			fileCrypt->lock();
			fileCrypt->escribir(str->str(), sizeof(char), str->tamano(), fileCrypt);
			fileCrypt->unlock();
			fileCrypt->cerrar();
			fileCrypt->liberar(NB_RETENEDOR_THIS);
			file->cerrar();
			r = true;
		}
	}
	str->liberar(NB_RETENEDOR_THIS);
	return r;
}

ENMngrGameKitAuthState AUAppGlueIOSGameKit::authenticationState(void* pData){
	AUAppGameKitDelegate* delegate = [AUAppGameKitDelegate sharedInstance];
	return [delegate getAuthState];
}

bool AUAppGlueIOSGameKit::startAuthentication(void* pData){
	AUAppGameKitDelegate* delegate = [AUAppGameKitDelegate sharedInstance];
	[delegate startAuthentication];
	return true;
}

bool AUAppGlueIOSGameKit::getLocalPlayer(void* pData, AUCadenaMutable8* dstId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDisplayName){
	bool r = false;
	GKLocalPlayer* localPlayer = [GKLocalPlayer localPlayer];
	if(localPlayer != nil){
		if(dstId != NULL){
			NSString* v = localPlayer.playerID;
			if(v != nil) dstId->establecer([v UTF8String]);
			else dstId->vaciar();
		}
		if(dstName != NULL){
			NSString* v = localPlayer.alias;
			if(v != nil) dstName->establecer([v UTF8String]);
			else dstName->vaciar();
		}
		if(dstDisplayName != NULL){
			NSString* v = localPlayer.displayName;
			if(v != nil) dstDisplayName->establecer([v UTF8String]);
			else dstDisplayName->vaciar();
		}
		r = true;
	}
	return r;
}

bool AUAppGlueIOSGameKit::sendScoreAndReport(void* pData, const char* scoreId, const SI64 value){
	bool r = false;
	GKScore* scoreReporter = [GKScore alloc];
	if([scoreReporter respondsToSelector:@selector(initWithLeaderboardIdentifier:)] && [GKScore respondsToSelector:@selector(reportScores:withCompletionHandler:)]){
		PRINTF_INFO("Setting score (iOS_7+).\n");
		scoreReporter = [scoreReporter initWithLeaderboardIdentifier:[NSString stringWithUTF8String:scoreId]];
		if(scoreReporter){
			scoreReporter.value = value;
			{
				NSArray* scores = [NSArray arrayWithObjects:scoreReporter, nil];
				[GKScore reportScores:scores withCompletionHandler:^(NSError *error) {
					NBMngrGameKit::lockedScoreReportingResult(scoreId, value, ENMngrGameKitSrc_OSApi, (error == nil));
				}];
			}
			r = true;
		}
	} else {
		PRINTF_INFO("Setting score (iOS_6-).\n");
		scoreReporter = [scoreReporter initWithCategory:[NSString stringWithUTF8String:scoreId]];
		if(scoreReporter){
			scoreReporter.value		= value;
			scoreReporter.context	= 0;
			[scoreReporter reportScoreWithCompletionHandler:^(NSError *error) {
				NBMngrGameKit::lockedScoreReportingResult(scoreId, value, ENMngrGameKitSrc_OSApi, (error == nil));
			}];
			r = true;
		}
	}
	return r;
}

bool AUAppGlueIOSGameKit::sendAchievProgressAndReport(void* pData, const char* achievId, const SI8 prog100){
	bool r = false;
	GKAchievement *achievement = [[GKAchievement alloc] initWithIdentifier:[NSString stringWithUTF8String:achievId]];
	if (achievement){
		achievement.showsCompletionBanner = YES;
		achievement.percentComplete = prog100;
		[achievement reportAchievementWithCompletionHandler:^(NSError *error) {
			NBMngrGameKit::lockedAchievReportingResult(achievId, prog100, ENMngrGameKitSrc_OSApi, (error == nil));
		}];
		r = true;
	}
	return r;
}

bool AUAppGlueIOSGameKit::showCenter(void* pData){
	bool r = false;
	Class gcClass = (NSClassFromString(@"GKGameCenterViewController")); //iOS 6 en adelante
	if(gcClass){
		GKGameCenterViewController *gameCenterController = [[GKGameCenterViewController alloc] init];
		if (gameCenterController != nil) {
			AUAppGameKitDelegate* delegate = [AUAppGameKitDelegate sharedInstance];
			UIViewController* vc = [delegate getParentViewController];
			if(vc != nil){
				//[self detenerTimers];
				gameCenterController.gameCenterDelegate = delegate;
				[vc presentViewController: gameCenterController animated: YES completion:nil];
				r = true;
			}
		}
	} else {
		r = AUAppGlueIOSGameKit::showLeaderboard(pData);
	}
	return r;
}

bool AUAppGlueIOSGameKit::showLeaderboard(void* pData){
	bool r = false;
	Class gcClass = (NSClassFromString(@"GKLeaderboardViewController")); //iOS 4 hasta 6
	if(gcClass){
		GKLeaderboardViewController *leaderboardController = [[GKLeaderboardViewController alloc] init];
		if (leaderboardController != nil) {
			AUAppGameKitDelegate* delegate = [AUAppGameKitDelegate sharedInstance];
			UIViewController* vc = [delegate getParentViewController];
			if(vc != nil){
				//[self detenerTimers];
				leaderboardController.leaderboardDelegate = delegate;
				[vc presentViewController: leaderboardController animated: YES completion:nil];
				r = true;
			}
		}
	}
	return r;
}



