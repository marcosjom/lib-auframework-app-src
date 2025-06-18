//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueOsxFiles.h"
#include "AUAppI.h"
#import <Foundation/Foundation.h>

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

//-------------------------
// AUAppGlueOsxFiles
//-------------------------

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

typedef struct AUAppGlueOSXFilesData_ {
	AUAppI* app;
} AUAppGlueOSXFilesData;

//Callbacks

bool AUAppGlueOsxFiles::create(void* app, STMngrFilesCalls* obj){
	AUAppGlueOSXFilesData* data = (AUAppGlueOSXFilesData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueOSXFilesData), ENMemoriaTipo_General);
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

bool AUAppGlueOsxFiles::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueOSXFilesData* data = (AUAppGlueOSXFilesData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueOsxFiles::getPathPrefix(void* pData, const ENMngrFilesPathType type, AUCadenaMutable8* dst){
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
							dst->agregar("/Contents/Resources/");
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

