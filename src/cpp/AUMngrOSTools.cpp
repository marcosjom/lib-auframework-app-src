//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrOSTools.h"
#include "AUMngrOSSecure.h"
#include "nb/core/NBMngrStructMaps.h"

//

STNBStructMapsRec NBFilePickerData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBFilePickerData_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBFilePickerData_sharedStructMap);
	if(NBFilePickerData_sharedStructMap.map == NULL){
		STNBFilePickerData s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBFilePickerData);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addBoolM(map, s, cantChooseDir);
		NBStructMap_addBoolM(map, s, cantChooseFile);
		NBStructMap_addBoolM(map, s, cantMultipleSelection);
		NBStructMap_addStrPtrM(map, s, title);
		NBStructMap_addStrPtrM(map, s, message);
		NBStructMap_addPtrToArrayOfStrPtrM(map, s, fileTypes, fileTypesSz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfStrPtrM(map, s, fileExts, fileExtsSz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfStrPtrM(map, s, mimeTypes, mimeTypesSz, ENNBStructMapSign_Unsigned);
		NBStructMap_addPtrToArrayOfBytesM(map, s, userData, userDataSz, ENNBStructMapSign_Unsigned);
		NBFilePickerData_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBFilePickerData_sharedStructMap);
	return NBFilePickerData_sharedStructMap.map;
}

STNBStructMapsRec NBImagePickerData_sharedStructMap = STNBStructMapsRec_empty;

const STNBStructMap* NBImagePickerData_getSharedStructMap(void){
	NBMngrStructMaps_lock(&NBImagePickerData_sharedStructMap);
	if(NBImagePickerData_sharedStructMap.map == NULL){
		STNBImagePickerData s;
		STNBStructMap* map = NBMngrStructMaps_allocTypeM(STNBImagePickerData);
		NBStructMap_init(map, sizeof(s));
		NBStructMap_addPtrToArrayOfBytesM(map, s, userData, userDataSz, ENNBStructMapSign_Unsigned);
		NBImagePickerData_sharedStructMap.map = map;
	}
	NBMngrStructMaps_unlock(&NBImagePickerData_sharedStructMap);
	return NBImagePickerData_sharedStructMap.map;
}

//

STMngrOSToolsCalls AUMngrOSTools::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//Device orientation
	, NULL, NULL
	, NULL, NULL
	//, NULL, NULL
	, NULL, NULL
	//Pasteboard
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//File picker
	, NULL, NULL
	//Photo picker
	, NULL, NULL
	, NULL, NULL
};

AUMngrOSTools::AUMngrOSTools() : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::AUMngrOSTools")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrOSTools")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		UI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0);
		}
	}
#	endif
	//Orientation
	_canAutorotate				= TRUE;
	_orientationsMaskApplied	= FALSE;
	_orientationsMask			= ENAppOrientationBits_All;
	_orientationPrefered		= ENAppOrientationBit_Portrait;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrOSTools::~AUMngrOSTools(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::~AUMngrOSTools")
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrOSTools::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrOSTools::setGlue(AUAppI* app, PTRfuncOSToolCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::setGlue")
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

bool AUMngrOSTools::getPkgIdentifier(AUCadenaMutable8* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getPkgIdentifier")
	bool r = false;
	if(_calls.funcGetPkgId != NULL){
		r = (*_calls.funcGetPkgId)(_calls.funcGetPkgIdParam, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::canOpenUrl(const char* url){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::canOpenUrl")
	bool r = false;
	if(_calls.funcCanOpenUrl != NULL){
		r = (*_calls.funcCanOpenUrl)(_calls.funcCanOpenUrlParam, url);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::canOpenFolders(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::canOpenFolders")
	bool r = false;
	if(_calls.funcCanOpenFolders != NULL){
		r = (*_calls.funcCanOpenFolders)(_calls.funcCanOpenFoldersParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::openUrl(const char* url){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::openUrl")
	bool r = false;
	if(_calls.funcOpenUrl != NULL){
		r = (*_calls.funcOpenUrl)(_calls.funcOpenUrlParam, url);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::openFolder(const char* folderPath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::openFolder")
	bool r = false;
	if(_calls.funcOpenFolder != NULL){
		r = (*_calls.funcOpenFolder)(_calls.funcOpenFolderParam, folderPath);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::openMyStore(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::openMyStore")
	bool r = false;
	if(_calls.funcOpenMyStore != NULL){
		r = (*_calls.funcOpenMyStore)(_calls.funcOpenMyStoreParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::openMySettings(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::openMySettings")
	bool r = false;
	if(_calls.funcOpenMySettings != NULL){
		r = (*_calls.funcOpenMySettings)(_calls.funcOpenMySettingsParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrOSTools::setContentProviderAuthority(const char* authority){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::shareFile")
	if(_calls.funcSetContentProviderAuthority != NULL){
		(*_calls.funcSetContentProviderAuthority)(_calls.funcSetContentProviderAuthorityParam, authority);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool AUMngrOSTools::shareFile(const char* filepath, const char* optNewFilename){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::shareFile")
	bool r = false;
	if(_calls.funcShareFile != NULL){
		r = (*_calls.funcShareFile)(_calls.funcShareFileParam, filepath, optNewFilename);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

float AUMngrOSTools::getWindowTopPaddingPxs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getWindowTopPadding")
	float r = 0.0f;
	if(_calls.funcGetTopPaddingPxs != NULL){
		r = (*_calls.funcGetTopPaddingPxs)(_calls.funcGetTopPaddingPxsParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

float AUMngrOSTools::getWindowBtmPaddingPxs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getWindowBtmPadding")
	float r = 0.0f;
	if(_calls.funcGetBtmPaddingPxs != NULL){
		r = (*_calls.funcGetBtmPaddingPxs)(_calls.funcGetBtmPaddingPxsParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

ENStatusBarStyle AUMngrOSTools::getBarStyle(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getBarStyle")
	ENStatusBarStyle r = ENStatusBarStyle_Count;
	if(_calls.funcGetBarStyle != NULL){
		r = (*_calls.funcGetBarStyle)(_calls.funcGetBarStyleParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrOSTools::setBarStyle(const ENStatusBarStyle style){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::setBarStyle")
	if(_calls.funcSetBarStyle != NULL){
		(*_calls.funcSetBarStyle)(_calls.funcSetBarStyleParam, style);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

BOOL AUMngrOSTools::concatDeviceName(STNBString* dst){
	BOOL r = FALSE;
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::setBarStyle")
	if(_calls.funcConcatDeviceName != NULL){
		r = (*_calls.funcConcatDeviceName)(_calls.funcConcatDeviceNameParam, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Device orientation

BOOL AUMngrOSTools::supportsRotation(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::supportsRotation")
	BOOL r = FALSE;
	if(_calls.funcGetGetSupportsRotation != NULL){
		r = (*_calls.funcGetGetSupportsRotation)(_calls.funcGetGetSupportsRotationParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrOSTools::canAutorotate(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::canAutorotate")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _canAutorotate;
}

void AUMngrOSTools::setCanAutorotate(const BOOL canAutorotate){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::setCanAutorotate")
	_canAutorotate = canAutorotate;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

UI32 AUMngrOSTools::orientationsMask(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::orientationsMask")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _orientationsMask;
}

void AUMngrOSTools::setOrientationsMask(const UI32 mask){ //ENAppOrientationBit
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::setOrientationsMask")
	const UI32 mask2 = (mask & ENAppOrientationBits_All);
	if(_orientationsMask != mask2 || !_orientationsMaskApplied){
		_orientationsMask = mask2;
		//Apply orientations rule
		if(_calls.funcSetOrientationsMask != NULL){
			if((*_calls.funcSetOrientationsMask)(_calls.funcSetOrientationsMaskParam, mask2)){
				_orientationsMaskApplied	= TRUE;
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

ENAppOrientationBit AUMngrOSTools::getOrientationPrefered(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getOrientationPrefered")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _orientationPrefered;
}

void AUMngrOSTools::setOrientationPrefered(const ENAppOrientationBit orientation){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::setOrientationPrefered")
	_orientationPrefered = orientation;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

ENAppOrientationBit AUMngrOSTools::getOrientation(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getOrientation")
	ENAppOrientationBit r = (ENAppOrientationBit)0;
	if(_calls.funcGetOrientation != NULL){
		r = (*_calls.funcGetOrientation)(_calls.funcGetOrientationParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

/*void AUMngrOSTools::setOrientation(const ENAppOrientationBit orientation){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::getOrientation")
	if(_calls.funcSetOrientation != NULL){
		(*_calls.funcSetOrientation)(_calls.funcSetOrientationParam, orientation);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}*/

//Pasteboard

UI64 AUMngrOSTools::pasteboardChangeCount(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::pasteboardChangeCount")
	UI64 r = 0;
	if(_calls.funcPasteboardChangeCount != NULL){
		r = (*_calls.funcPasteboardChangeCount)(_calls.funcPasteboardChangeCountParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::pasteboardIsFilled(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::pasteboardIsFilled")
	bool r = FALSE;
	if(_calls.funcPasteboardIsFilled != NULL){
		r = (*_calls.funcPasteboardIsFilled)(_calls.funcPasteboardIsFilledParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


bool AUMngrOSTools::pasteboardGetString(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::pasteboardGetString")
	bool r = FALSE;
	if(_calls.funcPasteboardGetString != NULL){
		r = (*_calls.funcPasteboardGetString)(_calls.funcPasteboardGetStringParam, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrOSTools::pasteboardSetString(const char* value){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::pasteboardSetString")
	bool r = FALSE;
	if(_calls.funcPasteboardSetString != NULL){
		r = (*_calls.funcPasteboardSetString)(_calls.funcPasteboardSetStringParam, value);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//File picker

BOOL AUMngrOSTools::filePickerStart(const STNBFilePickerData* pickerParams){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::filePickerStart")
	BOOL r = FALSE;
	if(_calls.funcFilePickerStart != NULL){
		r = (*_calls.funcFilePickerStart)(_calls.funcFilePickerStartParam, pickerParams);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrOSTools::imagePickerIsAvailable(const ENPhotoSource src){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::imagePickerIsAvailable")
	BOOL r = FALSE;
	if(_calls.funcImagePickerIsAvailable != NULL){
		r = (*_calls.funcImagePickerIsAvailable)(_calls.funcImagePickerIsAvailableParam, src);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrOSTools::imagePickerStart(const ENPhotoSource src, const STNBImagePickerData* pickerParams){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::imagePickerStart")
	BOOL r = FALSE;
	if(_calls.funcImagePickerStart != NULL){
		r = (*_calls.funcImagePickerStart)(_calls.funcImagePickerStartParam, src, pickerParams);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Wallpaper

BOOL AUMngrOSTools::wallpaperCanBeSet(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::wallpaperCanBeSet")
	BOOL r = FALSE;
	if(_calls.funcWallpaperCanBeSet != NULL){
		r = (*_calls.funcWallpaperCanBeSet)(_calls.funcWallpaperCanBeSetParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrOSTools::wallpaperSet(const char* filepath, const UI32 wallsMask /*ENWallpaperBit*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrOSTools::wallpaperSet")
	BOOL r = FALSE;
	if(_calls.funcWallpaperSet != NULL){
		r = (*_calls.funcWallpaperSet)(_calls.funcWallpaperSetParam, filepath, wallsMask);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrOSTools)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrOSTools, "AUMngrOSTools")
AUOBJMETODOS_CLONAR_NULL(AUMngrOSTools)
