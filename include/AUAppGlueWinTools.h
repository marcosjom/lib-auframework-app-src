//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueWinTools_H
#define AUAppGlueWinTools_H

#include "AUMngrOSTools.h"

class AUAppGlueWinTools {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrOSToolsCalls* obj);
		static bool destroy(void* data);
		//
		static BOOL concatDeviceName(void* data, STNBString* dst);
		//
		static bool getPkgIdentifier(void* data, AUCadenaMutable8* dst);
		static bool openUrl(void* data, const char* url);
		static bool openMyStore(void* data);
		static bool openMySettings(void* data);
		static void setContentProviderAuthority(void* data, const char* authority);
		static bool shareFile(void* data, const char* filepath, const char* optNewFilename);
		//Orientation
		static BOOL					supportsRotation(void* data);
		static ENAppOrientationBit	getOrientation(void* data);
		//static void				setOrientation(void* data, const ENAppOrientationBit orientation);
		static BOOL					setOrientationsMask(void* data, const UI32 orientMask);
		//File picker
		static BOOL	filePickerStart(void* data, const STNBFilePickerData* pickerParams);
		//Photo picker
		static BOOL	imagePickerIsAvailable(void* data, const ENPhotoSource src);
		static BOOL	imagePickerStart(void* data, const ENPhotoSource src, const STNBImagePickerData* pickerParams);
};

#endif
