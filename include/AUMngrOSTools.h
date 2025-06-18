//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrOSTools_h
#define AUMngrOSTools_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

typedef enum ENAppOrientationBit_ {
	ENAppOrientationBit_Portrait			= 1,
	ENAppOrientationBit_LandscapeLeftBtn	= 2,
	ENAppOrientationBit_LandscapeRightBtn	= 4,
	ENAppOrientationBit_PortraitInverted	= 8,
	ENAppOrientationBits_All				= (ENAppOrientationBit_Portrait | ENAppOrientationBit_LandscapeLeftBtn | ENAppOrientationBit_LandscapeRightBtn | ENAppOrientationBit_PortraitInverted)
} ENAppOrientationBit;

typedef enum ENStatusBarStyle_ {
	ENStatusBarStyle_Dark = 0,
	ENStatusBarStyle_Light,
	ENStatusBarStyle_Count
} ENStatusBarStyle;


typedef enum ENPhotoSource_ {
	ENPhotoSource_Library = 0,
	ENPhotoSource_Camera,
	ENPhotoSource_Count
} ENPhotoSource;

typedef enum ENWallpaperBit_ {
	ENWallpaperBit_System = 1,
	ENWallpaperBit_Lock = 2,
} ENWallpaperBit;

//

typedef struct STNBFilePickerData_ {
	BOOL	cantChooseDir;
	BOOL	cantChooseFile;
	BOOL	cantMultipleSelection;
	char*	title;
	char*	message;
	char**	fileTypes;
	UI32	fileTypesSz;
	char**	fileExts;
	UI32	fileExtsSz;
	char**	mimeTypes;
	UI32	mimeTypesSz;
	BYTE*	userData;
	UI32	userDataSz;
} STNBFilePickerData;

const STNBStructMap* NBFilePickerData_getSharedStructMap(void);

typedef struct STNBImagePickerData_ {
	BYTE*	userData;
	UI32	userDataSz;
} STNBImagePickerData;

const STNBStructMap* NBImagePickerData_getSharedStructMap(void);

//

typedef struct STMngrOSToolsCalls_ STMngrOSToolsCalls;

//Callbacks
typedef bool (*PTRfuncOSToolCreate)(AUAppI* app, STMngrOSToolsCalls* obj);
typedef bool (*PTRfuncOSToolDestroy)(void* obj);
typedef bool (*PTRfuncOSToolGetPkgId)(void* obj, AUCadenaMutable8* dst);
typedef bool (*PTRfuncOSToolcCantOpenUrl)(void* obj, const char* url);
typedef bool (*PTRfuncOSToolCanOpenFolders)(void* obj);
typedef bool (*PTRfuncOSToolOpenUrl)(void* obj, const char* url);
typedef bool (*PTRfuncOSToolOpenFolder)(void* obj, const char* floderPath);
typedef bool (*PTRfuncOSToolOpenMyStore)(void* obj);
typedef bool (*PTRfuncOSToolOpenMySettings)(void* obj);
typedef void (*PTRfuncOSToolSetContentProviderAuth)(void* obj, const char* authority);
typedef bool (*PTRfuncOSToolShareFile)(void* obj, const char* filePath, const char* optFileName);
typedef float (*PTRfuncOSToolGetTopPaddingPxs)(void* obj);
typedef float (*PTRfuncOSToolGetBtmPaddingPxs)(void* obj);
typedef ENStatusBarStyle (*PTRfuncOSToolGetBarStyle)(void* obj);
typedef void (*PTRfuncOSToolSetBarStyle)(void* obj, const ENStatusBarStyle style);
typedef BOOL (*PTRfuncOSToolConcatDeviceName)(void* obj, STNBString* dst);
//Device orientation
typedef BOOL (*PTRfuncOSToolGetSupportsRotation)(void* obj);
typedef ENAppOrientationBit (*PTRfuncOSToolGetOrientation)(void* obj);
//typedef void (*PTRfuncOSToolSetOrientation)(void* obj, const ENAppOrientationBit orientBit);
typedef BOOL (*PTRfuncOSToolSetOrientationsMask)(void* obj, const UI32 orientMsk);
//Pasteboard
typedef UI64 (*PTRfuncOSToolPasteboardChangeCount)(void* obj);
typedef bool (*PTRfuncOSToolPasteboardIsFilled)(void* obj);
typedef bool (*PTRfuncOSToolPasteboardGetString)(void* obj, STNBString* dst);
typedef bool (*PTRfuncOSToolPasteboardSetString)(void* obj, const char* value);
//File picker
typedef BOOL (*PTRfuncOSToolFilePickerStart)(void* obj, const STNBFilePickerData* pickerParams);
//Photo picker
typedef BOOL (*PTRfuncOSToolImagePickerIsAvailable)(void* obj, const ENPhotoSource src);
typedef BOOL (*PTRfuncOSToolImagePickerStart)(void* obj, const ENPhotoSource src, const STNBImagePickerData* pickerParams);
//Wallpaper
typedef BOOL (*PTRfuncOSToolWallpaperCanBeSet)(void* obj);
typedef BOOL (*PTRfuncOSToolWallpaperSet)(void* obj, const char* path, const UI32 wallsMask /*ENWallpaperBit*/);

//

typedef struct STMngrOSToolsCalls_ {
	PTRfuncOSToolCreate					funcCreate;
	void*								funcCreateParam;
	PTRfuncOSToolDestroy				funcDestroy;
	void*								funcDestroyParam;
	//
	PTRfuncOSToolGetPkgId				funcGetPkgId;
	void*								funcGetPkgIdParam;
	PTRfuncOSToolOpenUrl				funcCanOpenUrl;
	void*								funcCanOpenUrlParam;
	PTRfuncOSToolCanOpenFolders			funcCanOpenFolders;
	void*								funcCanOpenFoldersParam;
	//
	PTRfuncOSToolOpenUrl				funcOpenUrl;
	void*								funcOpenUrlParam;
	PTRfuncOSToolOpenFolder				funcOpenFolder;
	void*								funcOpenFolderParam;
	PTRfuncOSToolOpenMyStore			funcOpenMyStore;
	void*								funcOpenMyStoreParam;
	PTRfuncOSToolOpenMySettings			funcOpenMySettings;
	void*								funcOpenMySettingsParam;
	PTRfuncOSToolSetContentProviderAuth	funcSetContentProviderAuthority;
	void*								funcSetContentProviderAuthorityParam;
	PTRfuncOSToolShareFile				funcShareFile;
	void*								funcShareFileParam;
	PTRfuncOSToolGetTopPaddingPxs		funcGetTopPaddingPxs;
	void*								funcGetTopPaddingPxsParam;
	PTRfuncOSToolGetBtmPaddingPxs		funcGetBtmPaddingPxs;
	void*								funcGetBtmPaddingPxsParam;
	PTRfuncOSToolGetBarStyle			funcGetBarStyle;
	void*								funcGetBarStyleParam;
	PTRfuncOSToolSetBarStyle			funcSetBarStyle;
	void*								funcSetBarStyleParam;
	PTRfuncOSToolConcatDeviceName		funcConcatDeviceName;
	void*								funcConcatDeviceNameParam;
	//Device orientation
	PTRfuncOSToolGetSupportsRotation	funcGetGetSupportsRotation;
	void*								funcGetGetSupportsRotationParam;
	PTRfuncOSToolGetOrientation			funcGetOrientation;
	void*								funcGetOrientationParam;
	//PTRfuncOSToolSetOrientation		funcSetOrientation;
	//void*								funcSetOrientationParam;
	PTRfuncOSToolSetOrientationsMask	funcSetOrientationsMask;
	void*								funcSetOrientationsMaskParam;
	//Pasteboard
	PTRfuncOSToolPasteboardChangeCount	funcPasteboardChangeCount;
	void*								funcPasteboardChangeCountParam;
	PTRfuncOSToolPasteboardIsFilled		funcPasteboardIsFilled;
	void*								funcPasteboardIsFilledParam;
	PTRfuncOSToolPasteboardGetString	funcPasteboardGetString;
	void*								funcPasteboardGetStringParam;
	PTRfuncOSToolPasteboardSetString	funcPasteboardSetString;
	void*								funcPasteboardSetStringParam;
	//File picker
	PTRfuncOSToolFilePickerStart		funcFilePickerStart;
	void*								funcFilePickerStartParam;
	//Photo picker
	PTRfuncOSToolImagePickerIsAvailable	funcImagePickerIsAvailable;
	void*								funcImagePickerIsAvailableParam;
	PTRfuncOSToolImagePickerStart		funcImagePickerStart;
	void*								funcImagePickerStartParam;
	//Wallpaper
	PTRfuncOSToolWallpaperCanBeSet		funcWallpaperCanBeSet;
	void*								funcWallpaperCanBeSetParam;
	PTRfuncOSToolWallpaperSet			funcWallpaperSet;
	void*								funcWallpaperSetParam;
} STMngrOSToolsCalls;

//

class AUMngrOSTools : public AUObjeto {
	public:
		AUMngrOSTools();
		virtual ~AUMngrOSTools();
		//
		static bool	isGlued();
		static bool	setGlue(AUAppI* app, PTRfuncOSToolCreate initCall);
		//
		bool	getPkgIdentifier(AUCadenaMutable8* dst);
		bool	canOpenUrl(const char* url);
		bool	canOpenFolders();
		bool	openUrl(const char* url);
		bool	openFolder(const char* folderPath);
		bool	openMyStore();
		bool	openMySettings();
		void	setContentProviderAuthority(const char* authority);
		bool	shareFile(const char* filepath, const char* optNewFilename);
		float	getWindowTopPaddingPxs();	//Top bar
		float	getWindowBtmPaddingPxs();	//Btn safe area
		ENStatusBarStyle getBarStyle();
		void	setBarStyle(const ENStatusBarStyle style);
		BOOL	concatDeviceName(STNBString* dst);
		//Device orientation
		BOOL	supportsRotation();
		BOOL	canAutorotate();
		void	setCanAutorotate(const BOOL canAutorotate);
		UI32	orientationsMask();
		void	setOrientationsMask(const UI32 mask); //ENAppOrientationBit
		ENAppOrientationBit	getOrientationPrefered();
		void				setOrientationPrefered(const ENAppOrientationBit orientation);
		ENAppOrientationBit	getOrientation();
		//void				setOrientation(const ENAppOrientationBit orientation);
		//Pasteboard
		UI64	pasteboardChangeCount();
		bool	pasteboardIsFilled();
		bool	pasteboardGetString(STNBString* dst);
		bool	pasteboardSetString(const char* value);
		//File picker
		BOOL	filePickerStart(const STNBFilePickerData* pickerParams);
		//Photo picker
		BOOL	imagePickerIsAvailable(const ENPhotoSource src);
		BOOL	imagePickerStart(const ENPhotoSource src, const STNBImagePickerData* pickerParams);
		//Wallpaper
		BOOL	wallpaperCanBeSet();
		BOOL	wallpaperSet(const char* filepath, const UI32 wallsMask /*ENWallpaperBit*/);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrOSToolsCalls _calls;
		//Orientation
		BOOL				_canAutorotate;
		BOOL				_orientationsMaskApplied;
		UI32				_orientationsMask;
		ENAppOrientationBit _orientationPrefered;
};

#endif
