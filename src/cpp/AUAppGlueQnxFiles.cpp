//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueQnxFiles.h"
#include "AUAppI.h"

//-------------------------
// AUAppGlueQnxFiles
//-------------------------

#if defined(__QNX__)
	//is QNX (BB10)
#endif

typedef struct AUAppGlueQNXFilesData_ {
	AUAppI* app;
} AUAppGlueQNXFilesData;

//Callbacks

bool AUAppGlueQnxFiles::create(void* app, STMngrFilesCalls* obj){
	AUAppGlueQNXFilesData* data = (AUAppGlueQNXFilesData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueQNXFilesData), ENMemoriaTipo_General);
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

bool AUAppGlueQnxFiles::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueQNXFilesData* data = (AUAppGlueQNXFilesData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueQnxFiles::getPathPrefix(void* pData, const ENMngrFilesPathType type, AUCadenaMutable8* dst){
	bool r = false;
	{
		
		switch (type) {
			case ENMngrFilesPathType_Pkg:
				dst->establecer("./app/native/assets/");
				r = true;
				break;
			case ENMngrFilesPathType_Doc:
				dst->establecer("./data/");
				r = true;
				break;
			case ENMngrFilesPathType_Lib:
				dst->establecer("./data/");
				r = true;
				break;
			case ENMngrFilesPathType_Cache:
				dst->establecer("./tmp/");
				r = true;
				break;
			default:
				NBASSERT(false)
				break;
		}
	}
	return r;
}

