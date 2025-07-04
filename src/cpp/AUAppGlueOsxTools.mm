//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueOsxTools.h"
#include "nb/core/NBMemory.h"
#include "NBMngrNotifs.h"
#include "NBMngrOSSecure.h"
#include <AppKit/AppKit.h>
#include <SystemConfiguration/SystemConfiguration.h> //For "SCDynamicStoreCopyComputerName"
//
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#	include "nb/NBObjCMethods.h"	//for "objc_msgSend", "sel_registerName", ...
//#	include <objc/message.h>	//for "objc_msgSend"
//#	include <objc/objc.h>		//for "sel_registerName"
#endif


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

typedef struct AUAppGlueOsxToolsData_ {
	AUAppI* app;
	ENStatusBarStyle barStyle;
} AUAppGlueOsxToolsData;

//Calls
	
bool AUAppGlueOsxTools::create(AUAppI* app, STMngrOSToolsCalls* obj){
	AUAppGlueOsxToolsData* data 		= (AUAppGlueOsxToolsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueOsxToolsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueOsxToolsData);
	NBMemory_setZeroSt(*obj, STMngrOSToolsCalls);
	data->app							= (AUAppI*)app;
	data->barStyle						= ENStatusBarStyle_Count;
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcGetPkgId					= getPkgIdentifier;
	obj->funcGetPkgIdParam				= data;
	obj->funcCanOpenUrl					= canOpenUrl;
	obj->funcCanOpenUrlParam			= data;
	obj->funcCanOpenFolders				= canOpenFolders;
	obj->funcCanOpenFoldersParam		= data;
	obj->funcOpenUrl					= openUrl;
	obj->funcOpenUrlParam				= data;
	obj->funcOpenFolder					= openFolder;
	obj->funcOpenFolderParam			= data;
	obj->funcOpenMyStore				= openMyStore;
	obj->funcOpenMyStoreParam			= data;
	obj->funcOpenMySettings				= openMySettings;
	obj->funcOpenMySettingsParam		= data;
	obj->funcSetContentProviderAuthority = NULL;
	obj->funcSetContentProviderAuthorityParam = NULL;
	obj->funcShareFile					= shareFile;
	obj->funcShareFileParam				= data;
	obj->funcGetTopPaddingPxs			= getWindowTopPaddingPxs;
	obj->funcGetTopPaddingPxsParam		= data;
	obj->funcGetBtmPaddingPxs			= getWindowBtmPaddingPxs;
	obj->funcGetBtmPaddingPxsParam		= data;
	obj->funcGetBarStyle				= getBarStyle;
	obj->funcGetBarStyleParam			= data;
	obj->funcSetBarStyle				= setBarStyle;
	obj->funcSetBarStyleParam			= data;
	obj->funcConcatDeviceName			= concatDeviceName;
	obj->funcConcatDeviceNameParam		= data;
	//Device orientation
	obj->funcGetGetSupportsRotation		= NULL;
	obj->funcGetGetSupportsRotationParam = data;
	obj->funcGetOrientation				= NULL;
	obj->funcGetOrientationParam		= data;
	//obj->funcSetOrientation			= NULL;
	//obj->funcSetOrientationParam		= data;
	obj->funcSetOrientationsMask		= NULL;
	obj->funcSetOrientationsMaskParam	= NULL;
	//Pasteboard
	obj->funcPasteboardChangeCount		= pasteboardChangeCount;
	obj->funcPasteboardChangeCountParam	= data;
	obj->funcPasteboardIsFilled			= pasteboardIsFilled;
	obj->funcPasteboardIsFilledParam	= data;
	obj->funcPasteboardGetString		= pasteboardGetString;
	obj->funcPasteboardGetStringParam	= data;
	obj->funcPasteboardSetString		= pasteboardSetString;
	obj->funcPasteboardSetStringParam	= data;
	//File picker
	obj->funcFilePickerStart			= filePickerStart;
	obj->funcFilePickerStartParam		= data;
	//Photo picker
	obj->funcImagePickerIsAvailable		= imagePickerIsAvailable;
	obj->funcImagePickerIsAvailableParam = data;
	obj->funcImagePickerStart			= imagePickerStart;
	obj->funcImagePickerStartParam		= data;
	//Wallpaper
	obj->funcWallpaperCanBeSet			= NULL;
	obj->funcWallpaperCanBeSetParam		= NULL;
	obj->funcWallpaperSet				= NULL;
	obj->funcWallpaperSetParam			= NULL;
	//
	return true;
}

bool AUAppGlueOsxTools::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		@autoreleasepool {
			AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
			data->app = NULL;
			NBGestorMemoria::liberarMemoria(pData);
			r = true;
		}
	}
	return r;
}

//

bool AUAppGlueOsxTools::getPkgIdentifier(void* pData, AUCadenaMutable8* dst){
	if(dst != NULL) dst->establecer([[[NSBundle mainBundle] bundleIdentifier] UTF8String]);
	return true;
}

/*
 https://developer.apple.com/library/content/documentation/General/Conceptual/CocoaTouch64BitGuide/ConvertingYourAppto64-Bit/ConvertingYourAppto64-Bit.html
 Note: For 64 bits, all objc_msgSend(id, SEL, ...) call must be casted
 to explicit return and paramList, if not the extra params will be treated as NULL.
 By example:
 //normal call (no extra params)
 {
 id inst = objc_msgSend((id)objc_getClass("NSURLComponents"), sel_registerName("alloc"));
 inst = objc_msgSend(inst, sel_registerName("init"));
 }
 //casted call (extra native param)
 {
 id (*objc_msgSend_cstr)(id, SEL, const char*) = (id (*)(id, SEL, const char*)) objc_msgSend;
 id inst = objc_msgSend_cstr((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), "myNativeCStr");
 }
 //casted call (extra id param)
 {
 id (*objc_msgSend_id)(id, SEL, id) = (id (*)(id, SEL, id)) objc_msgSend;
 id inst = objc_msgSend_id((id)objc_getClass("NSString"), sel_registerName("stringWithString:"), idOtherStr);
 }
 */

#define NB_OBJC_STR_AUTORELEASED(CSTR)	((id (*)(id, SEL, const char*)) objc_msgSend)((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), CSTR)

bool AUAppGlueOsxTools::canOpenUrl(void* pData, const char* url){
	bool r = false;
	if(url != NULL){
		@autoreleasepool {
			/*
			 ObjC:
			 return [[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:url]];
			 */
			//
			/*
			//id (*id_objc_msgSend_cstr)(id, SEL, const char*) = (id (*)(id, SEL, const char*)) objc_msgSend;
			BOOL (*BOOL_objc_msgSend_id)(id, SEL, id) = (BOOL (*)(id, SEL, id)) objc_msgSend;
			id (*id_objc_msgSend_id)(id, SEL, id) = (id (*)(id, SEL, id)) objc_msgSend;
			//
			id URL = id_objc_msgSend_id((id)objc_getClass("NSURL"), sel_registerName("URLWithString:"), NB_OBJC_STR_AUTORELEASED(url));
			if(BOOL_objc_msgSend_id(objc_msgSend((id)objc_getClass("UIApplication"), sel_registerName("sharedApplication")), sel_registerName("canOpenURL:"), URL)){
				r = true;
			}*/
		}
	}
	return r;
}

bool AUAppGlueOsxTools::canOpenFolders(void* pData){
	return true;
}


bool AUAppGlueOsxTools::openUrl(void* pData, const char* url){
	bool r = false;
	if(url != NULL){
		@autoreleasepool {
			NSWorkspace* ws = [NSWorkspace sharedWorkspace];
			if(ws != nil){
				[ws openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
			}
		}
	}
	return r;
}

bool AUAppGlueOsxTools::openFolder(void* data, const char* folderPath){
	bool r = false;
	{
		NSWorkspace* wsp = [NSWorkspace sharedWorkspace];
		if(wsp != nil){
			NSString* rootPath = [NSString stringWithUTF8String:folderPath];
			if([wsp selectFile:nil inFileViewerRootedAtPath:rootPath]){
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueOsxTools::openMyStore(void* pData){
	bool r = false;
	return r;
}

bool AUAppGlueOsxTools::openMySettings(void* pData){
	bool r = false;
	@autoreleasepool {
		//
	}
	return r;
}

bool AUAppGlueOsxTools::shareFile(void* pData, const char* filepath, const char* optNewFilename){
	bool r = false;
	AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
	@autoreleasepool {
		const char* filePathToShare = filepath;
		STNBString strNewPath;
		NBString_init(&strNewPath);
		//Create link (if necesary)
		if(!NBString_strIsEmpty(optNewFilename)){
			{
				const SI32 iLastSlash = NBString_strLastIndexOf(filePathToShare, "/", NBString_strLenBytes(filePathToShare));
				if(iLastSlash >= 0){
					NBString_concatBytes(&strNewPath, filePathToShare, (iLastSlash + 1));
				}
				NBString_concat(&strNewPath, optNewFilename);
			}
			if([[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithUTF8String:strNewPath.str]]){
				filePathToShare = strNewPath.str;
			} else {
				NSError *error = nil;
				if([[NSFileManager defaultManager] linkItemAtPath:[NSString stringWithUTF8String:filePathToShare] toPath:[NSString stringWithUTF8String:strNewPath.str] error:&error]){
					filePathToShare = strNewPath.str;
				}
			}
		}
		//Share
		{
			/*NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePathToShare]];
			if(url){
			 //
			}*/
			NSWorkspace* wsp = [NSWorkspace sharedWorkspace];
			if(wsp != nil){
				STNBString strFolderPath;
				NBString_initWithStr(&strFolderPath, filePathToShare);
				{
					const SI32 iLastSlash = NBString_strLastIndexOf(filePathToShare, "/", NBString_strLenBytes(filePathToShare));
					if(iLastSlash < 0){
						NBString_set(&strFolderPath, ".");
					} else {
						NBASSERT(iLastSlash >= 0)
						NBString_setBytes(&strFolderPath, filePathToShare, iLastSlash + (iLastSlash <= 0 ? 1 : 0));
					}
					//Open file
					{
						NSString* filePath = [NSString stringWithUTF8String:filePathToShare];
						NSString* rootPath = [NSString stringWithUTF8String:strFolderPath.str];
						if([wsp selectFile:filePath inFileViewerRootedAtPath:rootPath]){
							r = true;
						}
					}
				}
				NBString_release(&strFolderPath);
			}
		}
		NBString_release(&strNewPath);
	}
	return r;
}

float AUAppGlueOsxTools::getWindowTopPaddingPxs(void* pData){
	float r = 0.0f;
	if(pData != NULL){
		AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		@autoreleasepool {
			/*UIWindow* win = (UIWindow*)data->app->getWindow(); NBASSERT(win != nil)
			if(win != nil){
				//After iOS7 the status bar went on top of the app window.
				if([[[UIDevice currentDevice] systemVersion] floatValue] >= 7.0f){
					if (!([UIApplication sharedApplication].isStatusBarHidden)) {
						const float contentScale	= [[win rootViewController].view contentScaleFactor]; NBASSERT(contentScale > 0.0f)
						const CGSize statusBarSize	= [[UIApplication sharedApplication] statusBarFrame].size;
						const float logicHeight		= (statusBarSize.width < statusBarSize.height ? statusBarSize.width : statusBarSize.height);
						r =  (logicHeight * contentScale);
					}
				}
			}*/
		}
	}
	return r;
}

float AUAppGlueOsxTools::getWindowBtmPaddingPxs(void* pData){
	float r = 0.0f;
	if(pData != NULL){
		AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		@autoreleasepool {
			/*UIWindow* win = (UIWindow*)data->app->getWindow(); NBASSERT(win != nil)
			if(win != nil){
				//After iOS 11 sareArea was defined (iPhoneX)
				if (@available(iOS 11.0, *)) {
					const float contentScale	= [[win rootViewController].view contentScaleFactor]; NBASSERT(contentScale > 0.0f)
					const float logicHeight		= (float)win.safeAreaInsets.bottom;
					r =  (logicHeight * contentScale);
				}
			}*/
		}
	}
	return r;
}

ENStatusBarStyle AUAppGlueOsxTools::getBarStyle(void* pData){
	ENStatusBarStyle r = ENStatusBarStyle_Count;
	if(pData != NULL){
		AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		r = data->barStyle;
	}
	return r;
}

void AUAppGlueOsxTools::setBarStyle(void* pData, const ENStatusBarStyle style){
	if(pData != NULL){
		AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		data->barStyle = style;
		@autoreleasepool {
			/*UIWindow* win = (UIWindow*)data->app->getWindow(); NBASSERT(win != nil)
			if(win != nil){
				[[win rootViewController] setNeedsStatusBarAppearanceUpdate];
			}*/
		}
	}
}

BOOL AUAppGlueOsxTools::concatDeviceName(void* pData, STNBString* dst){
	BOOL r = FALSE;
	if(pData != NULL){
        /*AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		@autoreleasepool {
			NSString* dName = (NSString *)SCDynamicStoreCopyComputerName(NULL, NULL);
			if(dName != nil){
				NBString_set(dst, [dName UTF8String]);
				r = TRUE;
				[dName release];
			}
		}*/
	}
	return r;
}

//Pasteboard

UI64 AUAppGlueOsxTools::pasteboardChangeCount(void* pData){
	UI64 r = 0;
	@autoreleasepool {
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		if(pasteboard != nil){
			r = [pasteboard changeCount];
		}
	}
	return r;
}

bool AUAppGlueOsxTools::pasteboardIsFilled(void* pData){
	bool r = FALSE;
	@autoreleasepool {
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		if(pasteboard != nil){
			//Asume allways filled
			r = TRUE;
		}
	}
	return r;
}

bool AUAppGlueOsxTools::pasteboardGetString(void* pData, STNBString* dst){
	bool r = FALSE;
	@autoreleasepool {
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		if(pasteboard != nil){
			NSString* value = [pasteboard stringForType:NSPasteboardTypeString];
			if(value != nil){
				if(dst != NULL){
					NBString_set(dst, [value UTF8String]);
				}
				r = TRUE;
			}
		}
	}
	return r;
}

bool AUAppGlueOsxTools::pasteboardSetString(void* data, const char* value){
	bool r = FALSE;
	@autoreleasepool {
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		if(pasteboard != nil){
			[pasteboard clearContents];
			[pasteboard setString:[NSString stringWithUTF8String:value] forType:NSPasteboardTypeString];
			r = TRUE;
		}
	}
	return r;
}

//File picker

BOOL AUAppGlueOsxTools::filePickerStart(void* pData, const STNBFilePickerData* pPickerParams){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		//Clone params (block variable)
		__block STNBFilePickerData* cpyParams = NULL;
		if(pPickerParams != NULL){
			cpyParams = NBMemory_allocType(STNBFilePickerData);
			NBMemory_setZeroSt(*cpyParams, STNBFilePickerData);
			NBStruct_stClone(NBFilePickerData_getSharedStructMap(), pPickerParams, sizeof(*pPickerParams), cpyParams, sizeof(*cpyParams));
		}
		//Run at main thread
		dispatch_async(dispatch_get_main_queue(), ^{
			@autoreleasepool {
				NSOpenPanel* panel = [NSOpenPanel openPanel];
				[panel retain];
				{
					// This method displays the panel and returns immediately.
					// The completion handler is called when the user selects an
					// item or cancels the panel.
					if(cpyParams == NULL){
						[panel setCanChooseDirectories:YES];
						[panel setCanChooseFiles:YES];
						[panel setResolvesAliases:YES];
						[panel setAllowsMultipleSelection:YES];
						[panel setTitle:nil];
						[panel setMessage:nil];
						[panel setAllowedFileTypes:nil];
					} else {
						[panel setCanChooseDirectories:(cpyParams->cantChooseDir ? NO : YES)];
						[panel setCanChooseFiles:(cpyParams->cantChooseFile ? NO : YES)];
						[panel setResolvesAliases: YES];
						[panel setAllowsMultipleSelection:(cpyParams->cantMultipleSelection ? NO : TRUE)];
						if(NBString_strIsEmpty(cpyParams->title)){
							[panel setTitle:@""];
						} else {
							[panel setTitle:[NSString stringWithUTF8String:cpyParams->title]];
						}
						if(NBString_strIsEmpty(cpyParams->message)){
							[panel setMessage:@""];
						} else {
							[panel setMessage:[NSString stringWithUTF8String:cpyParams->message]];
						}
						if(cpyParams->fileTypes == NULL || cpyParams->fileTypesSz <= 0){
							[panel setAllowedFileTypes:nil];
						} else {
							//Filetypes to filter
							NSMutableArray* arrTypes = [NSMutableArray arrayWithCapacity:(NSUInteger)cpyParams->fileTypesSz];
							SI32 i; for(i = 0; i < cpyParams->fileTypesSz; i++){
								NSString* str = [NSString stringWithUTF8String:cpyParams->fileTypes[i]];
								[arrTypes addObject:str];
							}
							[panel setAllowedFileTypes:arrTypes];
							//[arrTypes release]; //Known-leak: releasing the array here produces a crash.
						}
					}
					//Open
					{
						if(TRUE){
							//Run modal
							const NSModalResponse result = [panel runModal];
							if(result == NSFileHandlingPanelOKButton) {
								NSArray<NSURL *>* urls = [panel URLs];
								void* usrData = NULL; UI32 usrDataSz = 0;
								if(cpyParams != NULL){
									usrData		= cpyParams->userData;
									usrDataSz	= cpyParams->userDataSz;
								}
								if(urls != nil){
									const SI32 count = (SI32)[urls count];
									SI32 i; for(i = 0; i < count; i++){
										NSURL* url = [urls objectAtIndex:i];
										//Open the document
										if(data != NULL){
											if(data->app != NULL){
												const char* plainUrl = [[url absoluteString] UTF8String];
												data->app->broadcastOpenUrl(plainUrl, usrData, usrDataSz);
												PRINTF_INFO("Opened: '%s'.\n", plainUrl);
											}
										}
									}
								}
							}
							if(cpyParams != NULL){
								NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), cpyParams, sizeof(*cpyParams));
								NBMemory_free(cpyParams);
								cpyParams = NULL;
							}
						} else {
							//Run not modal
							[panel beginWithCompletionHandler:^(NSInteger result){
								if(result == NSFileHandlingPanelOKButton) {
									NSArray<NSURL *>* urls = [panel URLs];
									void* usrData = NULL; UI32 usrDataSz = 0;
									if(cpyParams != NULL){
										usrData		= cpyParams->userData;
										usrDataSz	= cpyParams->userDataSz;
									}
									if(urls != nil){
										const SI32 count = (SI32)[urls count];
										SI32 i; for(i = 0; i < count; i++){
											NSURL* url = [urls objectAtIndex:i];
											//Open the document
											if(data != NULL){
												if(data->app != NULL){
													const char* plainUrl = [[url absoluteString] UTF8String];
													data->app->broadcastOpenUrl(plainUrl, usrData, usrDataSz);
													PRINTF_INFO("Opened: '%s'.\n", plainUrl);
												}
											}
										}
									}
								}
								if(cpyParams != NULL){
									NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), cpyParams, sizeof(*cpyParams));
									NBMemory_free(cpyParams);
									cpyParams = NULL;
								}
							}];
						}
					}
				}
				[panel release];
			}
		});
		r = TRUE;
	}
	return r;
}

//Photo picker

BOOL AUAppGlueOsxTools::imagePickerIsAvailable(void* pData, const ENPhotoSource src){
	BOOL r = FALSE;
	if(pData != NULL){
		//AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		@autoreleasepool {
			switch(src) {
				case ENPhotoSource_Library:
					r = TRUE;
					break;
				case ENPhotoSource_Camera:
					r = FALSE;
					break;
				default:
					PRINTF_WARNING("AUAppGlueOsxTools, imagePickerIsAvailable, unsupported ENPhotoSource(%d).\n", src);
					break;
			}
		}
	}
	return r;
}

BOOL AUAppGlueOsxTools::imagePickerStart(void* pData, const ENPhotoSource src, const STNBImagePickerData* pickerParams){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueOsxToolsData* data = (AUAppGlueOsxToolsData*)pData;
		@autoreleasepool {
			switch(src) {
				case ENPhotoSource_Library:
					{
						STNBFilePickerData params;
						NBMemory_setZeroSt(params, STNBFilePickerData);
						{
							//Add UTIs
							{
								params.fileTypesSz	= 3;
								params.fileTypes	= NBMemory_allocTypes(char*, params.fileTypesSz);
								params.fileTypes[0]	= NBString_strNewBuffer(CFStringGetCStringPtr(kUTTypeJPEG, kCFStringEncodingUTF8));
								params.fileTypes[1]	= NBString_strNewBuffer(CFStringGetCStringPtr(kUTTypeJPEG2000, kCFStringEncodingUTF8));
								params.fileTypes[2]	= NBString_strNewBuffer(CFStringGetCStringPtr(kUTTypePNG, kCFStringEncodingUTF8));
							}
							//Clone user data
							if(pickerParams != NULL){
								if(pickerParams->userData != NULL && pickerParams->userDataSz > 0){
									NBString_strFreeAndNewBufferBytes((char**)&params.userData, (const char*)pickerParams->userData, pickerParams->userDataSz);
									params.userDataSz = pickerParams->userDataSz;
								}
							}
							//Open file
							{
								r = AUAppGlueOsxTools::filePickerStart(pData, &params);
							}
						}
						NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &params, sizeof(params));
					}
					break;
				case ENPhotoSource_Camera:
					break;
				default:
					PRINTF_WARNING("AUAppGlueOsxTools, imagePickerStart, unsupported ENPhotoSource(%d).\n", src);
					break;
			}
		}
	}
	return r;
}



