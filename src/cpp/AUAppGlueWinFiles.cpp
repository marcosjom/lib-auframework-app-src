//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueWinFiles.h"
#include "AUAppI.h"
#include <Shlobj.h> //for 'SHGetFolderPath'

//-------------------------
// AUAppGlueWinFiles
//-------------------------

#if defined(_WIN32) || defined(_WIN32)
	//is windows
#endif

typedef struct AUAppGlueWinFilesData_ {
	AUAppI* app;
} AUAppGlueWinFilesData;

//Callbacks

bool AUAppGlueWinFiles::create(void* app, STMngrFilesCalls* obj){
	AUAppGlueWinFilesData* data = (AUAppGlueWinFilesData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueWinFilesData), ENMemoriaTipo_General);
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

bool AUAppGlueWinFiles::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueWinFilesData* data = (AUAppGlueWinFilesData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueWinFiles::getPathPrefix(void* pData, const ENMngrFilesPathType type, AUCadenaMutable8* dst){
	bool r = false;
	{
		
		switch (type) {
			case ENMngrFilesPathType_Pkg:
				dst->establecer("./");
				r = true;
				break;
			case ENMngrFilesPathType_Doc:
				{
					CHAR strPath[MAX_PATH + 1];
					HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL /*FOLDERID_Documents*/, NULL, SHGFP_TYPE_CURRENT, strPath);
					if (result == S_OK){
						dst->establecer(strPath);
						r = true;
					}
				}
				break;
			case ENMngrFilesPathType_Lib:
				{
					CHAR strPath[MAX_PATH + 1];
					HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA /*FOLDERID_RoamingAppData*/, NULL, SHGFP_TYPE_CURRENT, strPath);
					if (result == S_OK){
						dst->establecer(strPath);
						r = true;
					}
				}
				break;
			case ENMngrFilesPathType_Cache:
				{
					CHAR strPath[MAX_PATH + 1];
					if(GetTempPath(MAX_PATH, strPath) != 0){
						dst->establecer(strPath);
						r = true;
					}
				}
				break;
			default:
				NBASSERT(false)
				break;
		}
	}
	return r;
}

