//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrOSTools_h
#define NBMngrOSTools_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrOSTools.h"

class NBMngrOSTools {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool	isGlued();
		static bool setGlue(AUAppI* app, PTRfuncOSToolCreate initCall);
		//
		static bool	getPkgIdentifier(AUCadenaMutable8* dst);
		static bool	canOpenUrl(const char* url);
		static bool	canOpenFolders();
		static bool	openUrl(const char* url);
		static bool	openFolder(const char* folderPath);
		static bool	openMyStore();
		static bool	openMySettings();
		static void	setContentProviderAuthority(const char* authority);
		static bool	shareFile(const char* filepath, const char* optNewFilename);
		static float getWindowTopPaddingPxs();
		static float getWindowBtmPaddingPxs();
		static ENStatusBarStyle getBarStyle();
		static void setBarStyle(const ENStatusBarStyle style);
		static BOOL	concatDeviceName(STNBString* dst);
		//Device orientation
		static BOOL supportsRotation();
		static BOOL	canAutorotate();
		static void	setCanAutorotate(const BOOL canAutorotate);
		static UI32	orientationsMask();
		static void	setOrientationsMask(const UI32 mask); //ENAppOrientationBit
		static ENAppOrientationBit	getOrientationPrefered();
		static void					setOrientationPrefered(const ENAppOrientationBit orientation);
		static ENAppOrientationBit	getOrientation();
		//static void				setOrientation(const ENAppOrientationBit orientation);
		//Pasteboard
		static UI64 pasteboardChangeCount();
		static bool pasteboardIsFilled();
		static bool pasteboardGetString(STNBString* dst);
		static bool pasteboardSetString(const char* value);
		//File picker
		static BOOL	filePickerStart(const STNBFilePickerData* pickerParams);
		//Photo picker
		static BOOL	imagePickerIsAvailable(const ENPhotoSource src);
		static BOOL	imagePickerStart(const ENPhotoSource src, const STNBImagePickerData* pickerParams);
		//Wallpaper
		static BOOL	wallpaperCanBeSet();
		static BOOL	wallpaperSet(const char* filepath, const UI32 wallsMask /*ENWallpaperBit*/);
	private:
		static AUMngrOSTools* _instance;
};

#endif
