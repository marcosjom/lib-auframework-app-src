//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrOSTools.h"

AUMngrOSTools* NBMngrOSTools::_instance	= NULL;

void NBMngrOSTools::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrOSTools();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrOSTools::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrOSTools::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrOSTools::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::isGlued")
	bool r = AUMngrOSTools::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::setGlue(AUAppI* app, PTRfuncOSToolCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::setGlue")
	bool r = AUMngrOSTools::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

bool NBMngrOSTools::getPkgIdentifier(AUCadenaMutable8* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::getPkgIdentifier")
	const bool r = _instance->getPkgIdentifier(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::canOpenUrl(const char* url){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::canOpenUrl")
	const bool r = _instance->canOpenUrl(url);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::canOpenFolders(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::canOpenFolders")
	const bool r = _instance->canOpenFolders();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::openUrl(const char* url){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::openUrl")
	const bool r = _instance->openUrl(url);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::openFolder(const char* folderPath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::openFolder")
	const bool r = _instance->openFolder(folderPath);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::openMyStore(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::openMyStore")
	const bool r = _instance->openMyStore();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::openMySettings(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::openMySettings")
	const bool r = _instance->openMySettings();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrOSTools::setContentProviderAuthority(const char* authority){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::shareFile")
	_instance->setContentProviderAuthority(authority);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrOSTools::shareFile(const char* filepath, const char* optNewFilename){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::shareFile")
	const bool r = _instance->shareFile(filepath, optNewFilename);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

float NBMngrOSTools::getWindowTopPaddingPxs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::getWindowTopPaddingPxs")
	const float r = _instance->getWindowTopPaddingPxs();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

float NBMngrOSTools::getWindowBtmPaddingPxs(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::getWindowBtmPaddingPxs")
	const float r = _instance->getWindowBtmPaddingPxs();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

ENStatusBarStyle NBMngrOSTools::getBarStyle(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::getBarStyle")
	ENStatusBarStyle r = _instance->getBarStyle();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrOSTools::setBarStyle(const ENStatusBarStyle style){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::setBarStyle")
	_instance->setBarStyle(style);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

BOOL NBMngrOSTools::concatDeviceName(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::concatDeviceName")
	BOOL r = _instance->concatDeviceName(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Device orientation

BOOL NBMngrOSTools::supportsRotation(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::supportsRotation")
	BOOL r = _instance->supportsRotation();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrOSTools::canAutorotate(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::canAutorotate")
	BOOL r = _instance->canAutorotate();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrOSTools::setCanAutorotate(const BOOL canAutorotate){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::setCanAutorotate")
	_instance->setCanAutorotate(canAutorotate);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

UI32 NBMngrOSTools::orientationsMask(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::orientationsMask")
	UI32 r = _instance->orientationsMask();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrOSTools::setOrientationsMask(const UI32 mask){ //ENAppOrientationBit
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::setOrientationsMask")
	_instance->setOrientationsMask(mask);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

ENAppOrientationBit NBMngrOSTools::getOrientationPrefered(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::getOrientationPrefered")
	ENAppOrientationBit r = _instance->getOrientationPrefered();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrOSTools::setOrientationPrefered(const ENAppOrientationBit orientation){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::setOrientationPrefered")
	_instance->setOrientationPrefered(orientation);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

ENAppOrientationBit NBMngrOSTools::getOrientation(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::getOrientation")
	ENAppOrientationBit r = _instance->getOrientation();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

/*void NBMngrOSTools::setOrientation(const ENAppOrientationBit orientation){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::setOrientation")
	_instance->setOrientation(orientation);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}*/

//Pasteboard

UI64 NBMngrOSTools::pasteboardChangeCount(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::pasteboardChangeCount")
	const UI64 r = _instance->pasteboardChangeCount();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::pasteboardIsFilled(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::pasteboardIsFilled")
	const bool r = _instance->pasteboardIsFilled();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::pasteboardGetString(STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::pasteboardGetString")
	const bool r = _instance->pasteboardGetString(dst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrOSTools::pasteboardSetString(const char* value){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::pasteboardSetString")
	const bool r = _instance->pasteboardSetString(value);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//File picker

BOOL NBMngrOSTools::filePickerStart(const STNBFilePickerData* pickerParams){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::filePickerStart")
	const BOOL r = _instance->filePickerStart(pickerParams);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Photo picker

BOOL NBMngrOSTools::imagePickerIsAvailable(const ENPhotoSource src){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::imagePickerIsAvailable")
	const BOOL r = _instance->imagePickerIsAvailable(src);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrOSTools::imagePickerStart(const ENPhotoSource src, const STNBImagePickerData* pickerParams){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::imagePickerStart")
	const BOOL r = _instance->imagePickerStart(src, pickerParams);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Wallpaper

BOOL NBMngrOSTools::wallpaperCanBeSet(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::wallpaperCanBeSet")
	const BOOL r = _instance->wallpaperCanBeSet();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrOSTools::wallpaperSet(const char* filepath, const UI32 wallsMask /*ENWallpaperBit*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrOSTools::wallpaperSet")
	const BOOL r = _instance->wallpaperSet(filepath, wallsMask);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}
