//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSFiles.h"
#include "AUAppI.h"
#import <Foundation/Foundation.h>

//-------------------------
// AUAppGlueIOSFiles
//-------------------------

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

typedef struct AUAppGlueIOSFilesData_ {
	AUAppI* app;
} AUAppGlueIOSFilesData;

//Callbacks

bool AUAppGlueIOSFiles::create(void* app, STMngrFilesCalls* obj){
	AUAppGlueIOSFilesData* data = (AUAppGlueIOSFilesData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSFilesData), ENMemoriaTipo_General);
	data->app			= (AUAppI*)app;
	//
	obj->funcCreate			= create;
	obj->funcCreateParam	= data;
	obj->funcDestroy		= destroy;
	obj->funcDestroyParam	= data;
	obj->funcGetPathPrefix	= getPathPrefix;
	obj->funcGetPathPrefixParam	= data;
	obj->funcOpenFile		= NULL;
	obj->funcOpenFileParam	= NULL;
	//
	return true;
}

bool AUAppGlueIOSFiles::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSFilesData* data = (AUAppGlueIOSFilesData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueIOSFiles::getPathPrefix(void* pData, const ENMngrFilesPathType type, AUCadenaMutable8* dst){
	bool r = false;
	@autoreleasepool {
		NSBundle* bundle = [NSBundle mainBundle];
		if(bundle != nil){
			switch (type) {
				case ENMngrFilesPathType_Pkg:
					{
						NSString* path = [bundle bundlePath];
						if(path != nil){
							dst->establecer([path UTF8String]);
							r = true;
						}
					}
					break;
				case ENMngrFilesPathType_Doc:
					dst->establecer([[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String]);
					r = true;
					break;
				case ENMngrFilesPathType_Lib:
					dst->establecer([[NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String]);
					r = true;
					break;
				case ENMngrFilesPathType_Cache:
					dst->establecer([[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String]);
					r = true;
					break;
				default:
					NBASSERT(false)
					break;
			}
		}
	}
	return r;
}

