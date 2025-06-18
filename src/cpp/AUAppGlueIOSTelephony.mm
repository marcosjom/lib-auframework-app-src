//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSTelephony.h"
#include "NBMngrNotifs.h"
#include "NBMngrOSSecure.h"
//
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#	include "nb/NBObjCMethods.h"	//for "objc_msgSend", "sel_registerName", ...
//#	include <objc/message.h>	//for "objc_msgSend"
//#	include <objc/objc.h>		//for "sel_registerName"
#endif
//

#include <CoreTelephony/CTTelephonyNetworkInfo.h>
#include <CoreTelephony/CTCarrier.h>

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

typedef struct AUAppGlueIOSTelephonyData_ {
	AUAppI* app;
} AUAppGlueIOSTelephonyData;

//Calls
	
bool AUAppGlueIOSTelephony::create(AUAppI* app, STMngrOSTelephonyCalls* obj){
	AUAppGlueIOSTelephonyData* data = (AUAppGlueIOSTelephonyData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSTelephonyData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueIOSTelephonyData);
	NBMemory_setZeroSt(*obj, STMngrOSTelephonyCalls);
	data->app = (AUAppI*)app;
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcAuthStatus					= authStatus;
	obj->funcAuthStatusParam			= data;
	//
	obj->funcGetPhoneCount				= getPhoneCount;
	obj->funcGetPhoneCountParam			= data;
	obj->funcGetIMEI					= getIMEI;
	obj->funcGetIMEIParam				= data;
	obj->funcCanMakeCalls				= canMakeCalls;
	obj->funcCanMakeCallsParam			= data;
	obj->funcGetCarrierCountryISO		= getCarrierCountryISO;
	obj->funcGetCarrierCountryISOParam	= data;
	//
	return true;
}

bool AUAppGlueIOSTelephony::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSTelephonyData* data = (AUAppGlueIOSTelephonyData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

ENTelephonyAuthStatus AUAppGlueIOSTelephony::authStatus(void* data, const BOOL requestIfNecesary){
	return ENTelephonyAuthStatus_Denied;
}

//

SI32 AUAppGlueIOSTelephony::getPhoneCount(void* data){	//sims supported by device
	SI32 r = 0;
	@autoreleasepool {
		if(NSClassFromString(@"CTTelephonyNetworkInfo") != nil){
			r = 1;
		}
	}
	return r;
}

BOOL AUAppGlueIOSTelephony::getIMEI(void* data, const SI32 slot, STNBString* dst){
	return FALSE;
}

bool AUAppGlueIOSTelephony::canMakeCalls(void* data){
	bool r = FALSE;
	@autoreleasepool {
		if (NSClassFromString(@"CTTelephonyNetworkInfo") != nil){
			CTTelephonyNetworkInfo* netinfo = [[CTTelephonyNetworkInfo alloc] init];
			if (@available(iOS 12.0, *)) {
				NSDictionary<NSString *,CTCarrier *>* arr = [netinfo serviceSubscriberCellularProviders];
				if(arr != nil){
					for(NSString* key in arr) {
						CTCarrier* carrier = arr[key];
						if(carrier != nil){
							NSString* code = [carrier mobileNetworkCode];
							NSString* name = [carrier carrierName];
							NSString* country = [carrier isoCountryCode];
							if(code != nil && name != nil){
								PRINTF_INFO("Carrier key('%s') code('%s') name('%s') country('%s').\n", [key UTF8String], [code UTF8String], [name UTF8String], [country UTF8String]);
								r = TRUE;
								break;
							}
						}
					}
				}
			} else {
				CTCarrier* carrier = [netinfo subscriberCellularProvider];
				if(carrier != nil){
					NSString* code = [carrier mobileNetworkCode];
					NSString* name = [carrier carrierName];
					NSString* country = [carrier isoCountryCode];
					if(code != nil && name != nil){
						PRINTF_INFO("Carrier code('%s') name('%s') country('%s').\n", [code UTF8String], [name UTF8String], [country UTF8String]);
						r = TRUE;
					}
				}
			}
			[netinfo release];
		}
	}
	return r;
}

bool AUAppGlueIOSTelephony::getCarrierCountryISO(void* data, STNBString* dst){
	bool r = FALSE;
	@autoreleasepool {
		if (NSClassFromString(@"CTTelephonyNetworkInfo") != nil){
			CTTelephonyNetworkInfo* netinfo = [[CTTelephonyNetworkInfo alloc] init];
			if (@available(iOS 12.0, *)) {
				NSDictionary<NSString *,CTCarrier *>* arr = [netinfo serviceSubscriberCellularProviders];
				if(arr != nil){
					BOOL homeFound = FALSE;
					for(NSString* key in arr) {
						CTCarrier* carrier = arr[key];
						if(carrier != nil){
							NSString* code = [carrier mobileNetworkCode];
							NSString* name = [carrier carrierName];
							NSString* country = [carrier isoCountryCode];
							if(country != nil){
								PRINTF_INFO("Carrier key('%s') code('%s') name('%s') country('%s').\n", [key UTF8String], [code UTF8String], [name UTF8String], [country UTF8String]);
								if(!homeFound && dst != NULL){
									NBString_set(dst, [country UTF8String]);
								}
								//
								r = TRUE;
								//
								if([key isEqualToString: @"home"]){
									homeFound = TRUE;
								}
							}
						}
					}
				}
			} else {
				CTCarrier* carrier = [netinfo subscriberCellularProvider];
				if(carrier != nil){
					NSString* code = [carrier mobileNetworkCode];
					NSString* name = [carrier carrierName];
					NSString* country = [carrier isoCountryCode];
					if(country != nil){
						PRINTF_INFO("Carrier code('%s') name('%s') country('%s').\n", [code UTF8String], [name UTF8String], [country UTF8String]);
						if(dst != NULL){
							NBString_set(dst, [country UTF8String]);
						}
						r = TRUE;
					}
				}
			}
			[netinfo release];
		}
	}
	return r;
}



