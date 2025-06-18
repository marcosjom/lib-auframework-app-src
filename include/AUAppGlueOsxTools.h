//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueOsxTools_H
#define AUAppGlueOsxTools_H

#include "AUMngrOSTools.h"

class AUAppGlueOsxTools {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrOSToolsCalls* obj);
		static bool destroy(void* data);
		//
		static bool getPkgIdentifier(void* data, AUCadenaMutable8* dst);
		static bool canOpenUrl(void* data, const char* url);
		static bool canOpenFolders(void* data);
		static bool openUrl(void* data, const char* url);
		static bool openFolder(void* data, const char* folderPath);
		static bool openMyStore(void* data);
		static bool openMySettings(void* data);
		static bool shareFile(void* data, const char* filepath, const char* optNewFilename);
		static float getWindowTopPaddingPxs(void* data);
		static float getWindowBtmPaddingPxs(void* data);
		static ENStatusBarStyle getBarStyle(void* data);
		static void setBarStyle(void* data, const ENStatusBarStyle style);
		static BOOL concatDeviceName(void* data, STNBString* dst);
		//Pasteboard
		static UI64 pasteboardChangeCount(void* data);
		static bool pasteboardIsFilled(void* data);
		static bool pasteboardGetString(void* data, STNBString* dst);
		static bool pasteboardSetString(void* data, const char* value);
		//File picker
		static BOOL	filePickerStart(void* data, const STNBFilePickerData* pickerParams);
		//Photo picker
		static BOOL	imagePickerIsAvailable(void* data, const ENPhotoSource src);
		static BOOL	imagePickerStart(void* data, const ENPhotoSource src, const STNBImagePickerData* pickerParams);
};

#endif
