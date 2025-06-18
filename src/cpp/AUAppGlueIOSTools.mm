//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSTools.h"
#include "NBMngrNotifs.h"
#include "NBMngrOSSecure.h"
#include "nb/2d/NBPng.h"
#include "nb/2d/NBJpeg.h"
//
#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#	include "nb/NBObjCMethods.h"	//for "objc_msgSend", "sel_registerName", ...
//#	include <objc/message.h>	//for "objc_msgSend"
//#	include <objc/objc.h>		//for "sel_registerName"
#endif
//

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

typedef struct AUAppGlueIOSToolsData_ AUAppGlueIOSToolsData;

//Document interaction (send-to, share file)

@interface AUIOSToolDocInteractDelegate : NSObject<UIDocumentInteractionControllerDelegate> {
	@private
		AUAppGlueIOSToolsData* _data;
}
-(id) init:(AUAppGlueIOSToolsData*)data;
- (void) dealloc;
//UIDocumentInteractionControllerDelegate
- (void)documentInteractionControllerWillBeginPreview:(UIDocumentInteractionController*) controller;
- (void)documentInteractionControllerDidEndPreview:(UIDocumentInteractionController*) controller;
- (void)documentInteractionControllerWillPresentOptionsMenu:(UIDocumentInteractionController*) controller;
- (void)documentInteractionControllerDidDismissOptionsMenu:(UIDocumentInteractionController*) controller;
- (void)documentInteractionControllerWillPresentOpenInMenu:(UIDocumentInteractionController*) controller;
- (void)documentInteractionControllerDidDismissOpenInMenu:(UIDocumentInteractionController*) controller;
//Opening Files
- (void)documentInteractionController:(UIDocumentInteractionController *)controller willBeginSendingToApplication:(NSString *)application;
- (void)documentInteractionController:(UIDocumentInteractionController *)controller didEndSendingToApplication:(NSString *)application;
@end


//File picker delegate

@interface AUIOSToolFilePickerDelegate : NSObject<UIDocumentPickerDelegate> {
@private
	AUAppGlueIOSToolsData* _data;
}
-(id) init:(AUAppGlueIOSToolsData*)data;
- (void) dealloc;
//UIDocumentPickerDelegate
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls;
- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller;
@end

//Photo picker delegate

@interface AUIOSToolImagePickerDelegate : NSObject<UINavigationControllerDelegate, UIImagePickerControllerDelegate> {
@private
	AUAppGlueIOSToolsData* _data;
}
-(id) init:(AUAppGlueIOSToolsData*)data;
- (void) dealloc;
//UINavigationControllerDelegate
- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated;
- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated;
//UIImagePickerControllerDelegate
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey, id> *)info;
- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker;
@end


//

typedef struct AUAppGlueIOSToolsData_ {
	AUAppI* app;
	ENStatusBarStyle barStyle;
	AUIOSToolDocInteractDelegate* docInteractDelegate;
	//File picker
	struct {
		STNBFilePickerData				params;
		UIDocumentPickerViewController*	contoller;
		AUIOSToolFilePickerDelegate*		delegate;
	} filePicker;
	//Photo picker
	struct {
		STNBImagePickerData				params;
		UIImagePickerController*		contoller;
		AUIOSToolImagePickerDelegate*	delegate;
	} imagePicker;
} AUAppGlueIOSToolsData;

//Calls
	
bool AUAppGlueIOSTools::create(AUAppI* app, STMngrOSToolsCalls* obj){
	AUAppGlueIOSToolsData* data 		= (AUAppGlueIOSToolsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSToolsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueIOSToolsData);
	NBMemory_setZeroSt(*obj, STMngrOSToolsCalls);
	data->app							= (AUAppI*)app;
	data->barStyle						= ENStatusBarStyle_Count;
	data->docInteractDelegate			= [[AUIOSToolDocInteractDelegate alloc] init:data];
	{
		NBMemory_setZero(data->filePicker);
		data->filePicker.contoller		= nil;
		data->filePicker.delegate		= nil;
	}
	{
		NBMemory_setZero(data->imagePicker);
		data->imagePicker.contoller		= nil;
		data->imagePicker.delegate		= nil;
	}
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
	obj->funcOpenFolder					= NULL;
	obj->funcOpenFolderParam			= NULL;
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
	obj->funcGetGetSupportsRotation		= supportsRotation;
	obj->funcGetGetSupportsRotationParam = data;
	obj->funcGetOrientation				= getOrientation;
	obj->funcGetOrientationParam		= data;
	//obj->funcSetOrientation			= setOrientation;
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

bool AUAppGlueIOSTools::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		@autoreleasepool {
			AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
			if(data->docInteractDelegate != NULL){
				[data->docInteractDelegate release];
				data->docInteractDelegate = nil;
			}
			data->app = NULL;
			NBGestorMemoria::liberarMemoria(pData);
			r = true;
		}
	}
	return r;
}

//

bool AUAppGlueIOSTools::isPhone(void){
    bool r = FALSE;
    {
        /*UIUserInterfaceIdiomUnspecified
        UIUserInterfaceIdiomPhone
        UIUserInterfaceIdiomPad
        UIUserInterfaceIdiomTV
        UIUserInterfaceIdiomCarPlay
        UIUserInterfaceIdiomMac*/
        r = ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone);
    }
    return r;
}

//

bool AUAppGlueIOSTools::getPkgIdentifier(void* pData, AUCadenaMutable8* dst){
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

bool AUAppGlueIOSTools::canOpenUrl(void* pData, const char* url){
	bool r = false;
	if(url != NULL){
		@autoreleasepool {
			/*
			 ObjC:
			 return [[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString:url]];
			 */
			//
			//Moved to pch
			//id (*id_objc_msgSend_cstr)(id, SEL, const char*) = (id (*)(id, SEL, const char*)) objc_msgSend;
			//BOOL (*BOOL_objc_msgSend_id)(id, SEL, id) = (BOOL (*)(id, SEL, id)) objc_msgSend;
			//id (*id_objc_msgSend_id)(id, SEL, id) = (id (*)(id, SEL, id)) objc_msgSend;
			//
			id uiApp = id_objc_msgSend((id)objc_getClass("UIApplication"), sel_registerName("sharedApplication"));
			id URL = id_objc_msgSend_id((id)objc_getClass("NSURL"), sel_registerName("URLWithString:"), NB_OBJC_STR_AUTORELEASED(url));
			if(BOOL_objc_msgSend_id(uiApp, sel_registerName("canOpenURL:"), URL)){
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSTools::canOpenFolders(void* pData){
	return false;
}

bool AUAppGlueIOSTools::openUrl(void* pData, const char* url){
	bool r = false;
	if(url != NULL){
		@autoreleasepool {
			if([[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]] == YES){
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSTools::openMyStore(void* pData){
	bool r = false;
	return r;
}

bool AUAppGlueIOSTools::openMySettings(void* pData){
	bool r = false;
	@autoreleasepool {
		//"prefs:root=General&path=About"
		//"App-prefs:root=General&path=About"
		NSString* urlPath = UIApplicationOpenSettingsURLString;
		//NSString* urlPath = [NSString stringWithFormat:@"%@%s", UIApplicationOpenSettingsURLString, "root=General&path=About"];
		PRINTF_INFO("Opening settings: '%s'.\n", [urlPath UTF8String]);
		if([[UIApplication sharedApplication] openURL:[NSURL URLWithString:urlPath]] == YES){
			r = true;
		}
	}
	return r;
}

bool AUAppGlueIOSTools::shareFile(void* pData, const char* filepath, const char* optNewFilename){
	bool r = false;
	AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
	@autoreleasepool {
		UIWindow* window = (UIWindow*)data->app->getWindow(); NBASSERT(window != nil)
		if(window != NULL){
			if(filepath != NULL){
				if(filepath[0] != '\0'){
					STNBString filepathShare;
					NBString_initWithStr(&filepathShare, filepath);
					r = true;
					//Create a copy file (if necesary)
					/*{
					 NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
					 NSString* docDir = [paths objectAtIndex:0];
					 if(NBString_strStartsWith(filepathShare.str, [docDir UTF8String])){
					 PRINTF_INFO("AUAppGlueIOSTools::shareFile: file already stored in doc folder.\n");
					 } else {
					 {
					 NSString* filePath2		= [NSString stringWithUTF8String:filepath];
					 NSString* fileNameOnly	= [[filePath2 lastPathComponent] stringByDeletingPathExtension];
					 NSString* fileExtOnly	= [filePath2 pathExtension];
					 NSFileManager *fileManager = [NSFileManager defaultManager];
					 UI32 iVer = 1;
					 while(TRUE){
					 //Concat folder path
					 NBString_set(&filepathShare, [docDir UTF8String]);
					 if(filepathShare.length > 0){
					 if(filepathShare.str[filepathShare.length - 1] != '/' && filepathShare.str[filepathShare.length - 1] != '\\'){
					 NBString_concat(&filepathShare, "/");
					 }
					 }
					 //Concat filename and extension
					 NBString_concat(&filepathShare, [fileNameOnly UTF8String]);
					 if(iVer > 1){
					 NBString_concat(&filepathShare, "(");
					 NBString_concatUI32(&filepathShare, iVer);
					 NBString_concat(&filepathShare, ")");
					 }
					 NBString_concat(&filepathShare, ".");
					 NBString_concat(&filepathShare, [fileExtOnly UTF8String]);
					 //
					 if([fileManager fileExistsAtPath:[NSString stringWithUTF8String:filepathShare.str]]){
					 //try next posfix
					 iVer++;
					 } else {
					 //Use current filepath
					 break;
					 }
					 }
					 //Create copy
					 {
                         NSError* error = nil;
                         if(![fileManager copyItemAtPath:[NSString stringWithUTF8String:filepath] toPath:[NSString stringWithUTF8String:filepathShare.str] error:&error]){
                         if(error != nil){
                         PRINTF_ERROR("AUAppGlueIOSTools::shareFile failed: '%s'.\n", [[error description] UTF8String]);
                         }
                         r = false;
					 } else {
					 #									ifdef NB_CONFIG_INCLUDE_ASSERTS
					 {
                         STNBFileRef file = NBFile_alloc(NULL);
                         if(!NBFile_open(&file, filepathShare.str, ENNBFileMode_Read)){
                         NBASSERT(FALSE)
                         }
                         NBFile_release(&file);
                         }
                         #									endif
                         r = true;
                         }
                         }
                         }
                         PRINTF_INFO("AUAppGlueIOSTools::shareFile: created copy of file to doc folder, to '%s' from '%s'.\n", filepathShare.str, filepath);
					 }
					 }*/
					//Share file
					if(r){
						r = false;
						{
							const char* filePathToShare = filepathShare.str;
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
								NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePathToShare]];
								if(url){
									// Initialize Document Interaction Controller
									UIDocumentInteractionController* docController = [UIDocumentInteractionController interactionControllerWithURL:url];
									// Configure Document Interaction Controller
									[docController setDelegate:data->docInteractDelegate];
									//Retain to avoid ben released before the destination reach the file
									[docController retain];
									// Present Open In Menu
									{
										UIView* view	= [window rootViewController].view;
										CGRect rect		= [view bounds];
										rect.origin.x	+= rect.size.width / 2.0f;
										rect.origin.y	+= rect.size.height;
										rect.size		= CGSizeZero;
										//[docController presentPreviewAnimated:YES];
										if(![docController presentOptionsMenuFromRect:rect inView:view animated:YES]){
											[docController release];
										} else {
											//Will be released by the delegate
											r = true;
										}
										/*if([docController presentOpenInMenuFromRect:[window frame] inView:[window rootViewController].view animated:YES]){
										 r = true;
										 }*/
									}
								}
							}
							NBString_release(&strNewPath);
						}
					}
					NBString_release(&filepathShare);
				}
			}
		}
	}
	return r;
}

float AUAppGlueIOSTools::getWindowTopPaddingPxs(void* pData){
	float r = 0.0f;
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		@autoreleasepool {
			UIWindow* win = (UIWindow*)data->app->getWindow(); NBASSERT(win != nil)
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
			}
		}
	}
	return r;
}

float AUAppGlueIOSTools::getWindowBtmPaddingPxs(void* pData){
	float r = 0.0f;
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		@autoreleasepool {
			UIWindow* win = (UIWindow*)data->app->getWindow(); NBASSERT(win != nil)
			if(win != nil){
				//After iOS 11 sareArea was defined (iPhoneX)
				if (@available(iOS 11.0, *)) {
					const float contentScale	= [[win rootViewController].view contentScaleFactor]; NBASSERT(contentScale > 0.0f)
					const float logicHeight		= (float)win.safeAreaInsets.bottom;
					r = (logicHeight * contentScale);
				}
			}
		}
	}
	return r;
}

ENStatusBarStyle AUAppGlueIOSTools::getBarStyle(void* pData){
	ENStatusBarStyle r = ENStatusBarStyle_Count;
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		r = data->barStyle;
	}
	return r;
}

void AUAppGlueIOSTools::setBarStyle(void* pData, const ENStatusBarStyle style){
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		data->barStyle = style;
		@autoreleasepool {
			UIWindow* win = (UIWindow*)data->app->getWindow(); NBASSERT(win != nil)
			if(win != nil){
				[[win rootViewController] setNeedsStatusBarAppearanceUpdate];
			}
		}
	}
}

BOOL AUAppGlueIOSTools::concatDeviceName(void* pData, STNBString* dst){
	BOOL r = FALSE;
	if(pData != NULL){
		//AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		@autoreleasepool {
			UIDevice* dev = [UIDevice currentDevice];
			if(dev != nil){
				NSString* dName = [dev name];
				if(dName != nil){
					NBString_set(dst, [dName UTF8String]);
					r = TRUE;
				}
			}
		}
	}
	return r;
}

//Orientation

BOOL AUAppGlueIOSTools::supportsRotation(void* data){
	return TRUE;
}
	
ENAppOrientationBit AUAppGlueIOSTools::getOrientation(void* data){
	ENAppOrientationBit r = (ENAppOrientationBit)0;
	UIInterfaceOrientation orient = [[UIApplication sharedApplication] statusBarOrientation];
	switch (orient) {
		case UIInterfaceOrientationPortrait:
			r = ENAppOrientationBit_Portrait;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			r = ENAppOrientationBit_LandscapeLeftBtn;
			break;
		case UIInterfaceOrientationLandscapeRight:
			r = ENAppOrientationBit_LandscapeRightBtn;
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			r = ENAppOrientationBit_PortraitInverted;
			break;
		default:
			break;
	}
	return r;
}

//Not safe! Not calling event-methods.
/*void AUAppGlueIOSTools::setOrientation(void* data, const ENAppOrientationBit orientation){
	if(orientation & ENAppOrientationBit_Portrait){
		[[UIDevice currentDevice] setValue:[NSNumber numberWithInteger: UIInterfaceOrientationPortrait] forKey:@"orientation"];
	} else if(orientation & ENAppOrientationBit_LandscapeLeftBtn){
		[[UIDevice currentDevice] setValue:[NSNumber numberWithInteger: UIInterfaceOrientationLandscapeLeft] forKey:@"orientation"];
	} else if(orientation & ENAppOrientationBit_LandscapeRightBtn){
		[[UIDevice currentDevice] setValue:[NSNumber numberWithInteger: UIInterfaceOrientationLandscapeRight] forKey:@"orientation"];
	} else if(orientation & ENAppOrientationBit_PortraitInverted){
		[[UIDevice currentDevice] setValue:[NSNumber numberWithInteger: UIInterfaceOrientationPortraitUpsideDown] forKey:@"orientation"];
	}
}*/

//Pasteboard

UI64 AUAppGlueIOSTools::pasteboardChangeCount(void* pData){
	UI64 r = 0;
	@autoreleasepool {
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		if(pasteboard != nil){
			r = [pasteboard changeCount];
		}
	}
	return r;
}

bool AUAppGlueIOSTools::pasteboardIsFilled(void* pData){
	bool r = FALSE;
	@autoreleasepool {
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		if(pasteboard != nil){
			r = [pasteboard hasStrings];
		}
	}
	return r;
}

bool AUAppGlueIOSTools::pasteboardGetString(void* pData, STNBString* dst){
	bool r = FALSE;
	@autoreleasepool {
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		if(pasteboard != nil){
			if([pasteboard hasStrings]){
				NSString* value = [pasteboard string];
				if(value != nil){
					if(dst != NULL){
						NBString_set(dst, [value UTF8String]);
					}
					r = TRUE;
				}
			}
		}
	}
	return r;
}

bool AUAppGlueIOSTools::pasteboardSetString(void* data, const char* value){
	bool r = FALSE;
	@autoreleasepool {
		UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
		if(pasteboard != nil){
			[pasteboard setString:[NSString stringWithUTF8String:value]];
			r = TRUE;
		}
	}
	return r;
}

//File picker

BOOL AUAppGlueIOSTools::filePickerStart(void* pData, const STNBFilePickerData* pickerParams){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		@autoreleasepool {
			if(data->filePicker.contoller == nil && data->filePicker.delegate == nil){
				if(data->app != NULL){
					UIViewController* vc = (UIViewController*)data->app->getViewController();
					if(vc != nil){
						UIDocumentPickerViewController* controller = nil;
						if(pickerParams != NULL){
							if(pickerParams->fileTypes != NULL && pickerParams->fileTypesSz > 0){
								//Filetypes to filter
								NSMutableArray* arrTypes = [NSMutableArray arrayWithCapacity:(NSUInteger)pickerParams->fileTypesSz];
								SI32 i; for(i = 0; i < pickerParams->fileTypesSz; i++){
									NSString* str = [NSString stringWithUTF8String:pickerParams->fileTypes[i]];
									[arrTypes addObject:str];
								}
								controller = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:arrTypes inMode:UIDocumentPickerModeImport];
								//[arrTypes release]; //Known-leak: releasing the array here produces a crash.
							}
						}
						if(controller == nil){
							//No filetype filter
						}
						if(controller != nil){
							AUIOSToolFilePickerDelegate* delegate = [[AUIOSToolFilePickerDelegate alloc] init:data];
							controller.delegate = delegate;
							controller.modalPresentationStyle = UIModalPresentationFormSheet; //UIModalPresentationFullScreen
							//Set as current
							{
								{
									NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &data->filePicker.params, sizeof(data->filePicker.params));
									if(pickerParams != NULL){
										NBStruct_stClone(NBFilePickerData_getSharedStructMap(), pickerParams, sizeof(*pickerParams), &data->filePicker.params, sizeof(data->filePicker.params));
									}
								}
								data->filePicker.contoller	= controller;
								data->filePicker.delegate	= delegate;
							}
							[vc presentViewController:controller animated:YES completion:nil];
							/*UIModalPresentationFullScreen = 0,
							 UIModalPresentationPageSheet NS_ENUM_AVAILABLE_IOS(3_2) __TVOS_PROHIBITED, //partially covers the underlying content.
							 UIModalPresentationFormSheet NS_ENUM_AVAILABLE_IOS(3_2) __TVOS_PROHIBITED, //displays the content centered in the screen.
							 UIModalPresentationCurrentContext NS_ENUM_AVAILABLE_IOS(3_2),
							 UIModalPresentationCustom NS_ENUM_AVAILABLE_IOS(7_0),
							 UIModalPresentationOverFullScreen NS_ENUM_AVAILABLE_IOS(8_0),
							 UIModalPresentationOverCurrentContext NS_ENUM_AVAILABLE_IOS(8_0),
							 UIModalPresentationPopover NS_ENUM_AVAILABLE_IOS(8_0) __TVOS_PROHIBITED,
							 UIModalPresentationBlurOverFullScreen __TVOS_AVAILABLE(11_0) __IOS_PROHIBITED __WATCHOS_PROHIBITED,
							 UIModalPresentationNone NS_ENUM_AVAILABLE_IOS(7_0) = -1,*/
							r = TRUE;
						}
					}
				}
			}
		}
	}
	return r;
}

//Photo picker

BOOL AUAppGlueIOSTools::imagePickerIsAvailable(void* pData, const ENPhotoSource src){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		@autoreleasepool {
			//UIImagePickerControllerSourceTypePhotoLibrary
			//Specifies the device’s photo library as the source for the image picker controller.
			//
			//UIImagePickerControllerSourceTypeCamera
			//Specifies the device’s built-in camera as the source for the image picker controller. Indicate the specific camera you want (front or rear, as available) by using the cameraDevice property.
			//
			//UIImagePickerControllerSourceTypeSavedPhotosAlbum
			//Specifies the device’s Camera Roll album as the source for the image picker controller. If the device does not have a camera, specifies the Saved Photos album as the source.
			//
			switch(src) {
				case ENPhotoSource_Library:
					r = ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypePhotoLibrary] ? TRUE : FALSE);
					break;
				case ENPhotoSource_Camera:
					r = ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera] ? TRUE : FALSE);
					break;
				default:
					PRINTF_WARNING("AUAppGlueIOSTools, imagePickerIsAvailable, unsupported ENPhotoSource(%d).\n", src);
					break;
			}
		}
	}
	return r;
}

BOOL AUAppGlueIOSTools::imagePickerStart(void* pData, const ENPhotoSource src, const STNBImagePickerData* pickerParams){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueIOSToolsData* data = (AUAppGlueIOSToolsData*)pData;
		@autoreleasepool {
			if(data->imagePicker.contoller == nil && data->imagePicker.delegate == nil){
				if(data->app != NULL){
					UIViewController* vc = (UIViewController*)data->app->getViewController();
					if(vc != nil){
						UIImagePickerController* controller = nil;
						switch(src) {
							case ENPhotoSource_Library:
								controller					= [[UIImagePickerController alloc] init];
								controller.sourceType		= UIImagePickerControllerSourceTypePhotoLibrary;
								controller.allowsEditing	= NO;
								//extern const CFStringRef kUTTypeJPEG		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
								//extern const CFStringRef kUTTypeJPEG2000	__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
								//extern const CFStringRef kUTTypePNG		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_3_0);
								break;
							case ENPhotoSource_Camera:
								controller					= [[UIImagePickerController alloc] init];
								controller.sourceType		= UIImagePickerControllerSourceTypeCamera;
								controller.allowsEditing	= NO;
								break;
							default:
								PRINTF_WARNING("AUAppGlueIOSTools, imagePickerStart, unsupported ENPhotoSource(%d).\n", src);
								break;
						}
						if(controller != nil){
							AUIOSToolImagePickerDelegate* delegate = [[AUIOSToolImagePickerDelegate alloc] init:data];
							controller.delegate = delegate;
							//Set as current
							{
								{
									NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &data->imagePicker.params, sizeof(data->imagePicker.params));
									if(pickerParams != NULL){
										NBStruct_stClone(NBImagePickerData_getSharedStructMap(), pickerParams, sizeof(*pickerParams), &data->imagePicker.params, sizeof(data->imagePicker.params));
									}
								}
								data->imagePicker.contoller	= controller;
								data->imagePicker.delegate	= delegate;
							}
							[vc presentViewController:controller animated:YES completion:nil];
							r = TRUE;
						}
					}
				}
			}
		}
	}
	return r;
}

//
//Document interaction (send-to, share file)
//

@implementation AUIOSToolDocInteractDelegate
- (id) init:(AUAppGlueIOSToolsData*)data {
	_data = data;
	return self;
}
- (void) dealloc {
	_data = NULL;
	[super dealloc];
}
//UIDocumentInteractionControllerDelegate
- (void)documentInteractionControllerWillBeginPreview:(UIDocumentInteractionController*) controller {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::documentInteractionControllerWillBeginPreview.\n");
}
- (void)documentInteractionControllerDidEndPreview:(UIDocumentInteractionController*) controller {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::documentInteractionControllerDidEndPreview.\n");
	[controller release]; //retained at creation
}
- (void)documentInteractionControllerWillPresentOptionsMenu:(UIDocumentInteractionController*) controller {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::documentInteractionControllerWillPresentOptionsMenu.\n");
}
- (void)documentInteractionControllerDidDismissOptionsMenu:(UIDocumentInteractionController*) controller {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::documentInteractionControllerDidDismissOptionsMenu.\n");
	[controller release]; //retained at creation
}
- (void)documentInteractionControllerWillPresentOpenInMenu:(UIDocumentInteractionController*) controller {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::documentInteractionControllerWillPresentOpenInMenu.\n");
}
- (void)documentInteractionControllerDidDismissOpenInMenu:(UIDocumentInteractionController*) controller {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::documentInteractionControllerDidDismissOpenInMenu.\n");
	[controller release]; //retained at creation
}
//Opening Files
- (void)documentInteractionController:(UIDocumentInteractionController *)controller willBeginSendingToApplication:(NSString *)application {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::willBeginSendingToApplication '%s'.\n", [application UTF8String]);
}
- (void)documentInteractionController:(UIDocumentInteractionController *)controller didEndSendingToApplication:(NSString *)application {
	PRINTF_INFO("AUIOSToolDocInteractDelegate::didEndSendingToApplication: '%s'.\n", [application UTF8String]);
}
@end

//
//File picker delegate
//

@implementation AUIOSToolFilePickerDelegate
- (id) init:(AUAppGlueIOSToolsData*)data {
	_data = data;
	return self;
}
- (void) removeFromData {
	if(_data != NULL){
		NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &_data->filePicker.params, sizeof(_data->filePicker.params));
		if(_data->filePicker.delegate != nil){
			[_data->filePicker.delegate release];
			_data->filePicker.delegate = nil;
		}
		if(_data->filePicker.contoller != nil){
			[_data->filePicker.contoller release];
			_data->filePicker.contoller = nil;
		}
		_data = NULL;
	}
}
- (void) dealloc{
	[self removeFromData];
	[super dealloc];
}
//UIDocumentPickerDelegate
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
	if(_data != NULL){
		AUAppI* app = _data->app;
		STNBFilePickerData params;
		NBMemory_setZeroSt(params, STNBFilePickerData);
		NBStruct_stClone(NBFilePickerData_getSharedStructMap(), &_data->filePicker.params, sizeof(_data->filePicker.params), &params, sizeof(params));
		//Remove myself before the events
		{
			[self removeFromData];
		}
		//Trigger file selections
		if(app != NULL){
			if(urls != nil){
				SI32 i; const SI32 count = (SI32)urls.count;
				for(i = 0;  i < count; i++){
					NSURL* url = [urls objectAtIndex:i];
					if(url != nil){
						PRINTF_INFO("AUIOSToolFilePickerDelegate::document selected: '%s'.\n", [[url absoluteString] UTF8String]);
						if(app != NULL){
							app->broadcastOpenUrl([[url absoluteString] UTF8String], params.userData, params.userDataSz);
						}
					}
				}
			}
		}
		NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &params, sizeof(params));
	}
}
- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller{
	if(_data != NULL){
		[self removeFromData];
	}
}
@end


//
//Photo picker delegate
//

@implementation AUIOSToolImagePickerDelegate
- (id) init:(AUAppGlueIOSToolsData*)data {
	_data = data;
	return self;
}
- (void) removeFromData {
	if(_data != NULL){
		NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &_data->imagePicker.params, sizeof(_data->imagePicker.params));
		if(_data->imagePicker.delegate != nil){
			[_data->imagePicker.delegate release];
			_data->imagePicker.delegate = nil;
		}
		if(_data->imagePicker.contoller != nil){
			[_data->imagePicker.contoller release];
			_data->imagePicker.contoller = nil;
		}
		_data = NULL;
	}
}
- (void) dealloc{
	[self removeFromData];
	[super dealloc];
}

//UINavigationControllerDelegate

- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated {
	//Nothing
}

- (void)navigationController:(UINavigationController *)navigationController didShowViewController:(UIViewController *)viewController animated:(BOOL)animated {
	//Nothing
}

//UIImagePickerControllerDelegate

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<UIImagePickerControllerInfoKey, id> *)info {
	//Process
	if(_data != NULL){
		AUAppI* app = _data->app;
		STNBImagePickerData params;
		NBMemory_setZeroSt(params, STNBImagePickerData);
		NBStruct_stClone(NBImagePickerData_getSharedStructMap(), &_data->imagePicker.params, sizeof(_data->imagePicker.params), &params, sizeof(params));
		//Remove myself before the events
		{
			[self removeFromData];
		}
		//NSLog(@"AUIOSToolImagePickerDelegate, didFinishPickingMediaWithInfo: '%@'.\n", info);
		//Trigger file selections
		if(app != NULL){
			//Open
			{
				BOOL sent = FALSE;
				SI32 rotDeg = 0;
				//Detect rotation
				{
					NSDictionary* meta = [info objectForKey:UIImagePickerControllerMediaMetadata];
					if(meta != nil){
						//NSLog(@"UIImagePickerControllerMediaMetadata: %@.\n", meta);
						CFNumberRef orient = (CFNumberRef)[meta objectForKey:(NSString*)kCGImagePropertyOrientation];
						if(orient != nil){
							SI32 oValue = 0;
							if(CFNumberGetValue(orient, kCFNumberSInt32Type, &oValue)){
								switch(oValue){
									case kCGImagePropertyOrientationUp:
										//The encoded image data matches the image's intended display orientation.
										break;
									case kCGImagePropertyOrientationUpMirrored:
										//The encoded image data is horizontally flipped from the image's intended display orientation.
										break;
									case kCGImagePropertyOrientationDown:
										//The encoded image data is rotated 180° from the image's intended display orientation.
										rotDeg = 180;
										break;
									case kCGImagePropertyOrientationDownMirrored:
										//The encoded image data is vertically flipped from the image's intended display orientation.
										break;
									case kCGImagePropertyOrientationLeftMirrored:
										//The encoded image data is horizontally flipped and rotated 90° counter-clockwise from the image's intended display orientation.
										break;
									case kCGImagePropertyOrientationRight:
										//The encoded image data is rotated 90° clockwise from the image's intended display orientation.
										rotDeg = 90;
										break;
									case kCGImagePropertyOrientationRightMirrored:
										//The encoded image data is horizontally flipped and rotated 90° clockwise from the image's intended display orientation.
										break;
									case kCGImagePropertyOrientationLeft:
										//The encoded image data is rotated 90° clockwise from the image's intended display orientation.
										rotDeg = 270;
										break;
								}
							}
						}
					} else {
						UIImage* img = (UIImage*)[info objectForKey:UIImagePickerControllerOriginalImage];
						if(img != nil){
							const UIImageOrientation orient = [img imageOrientation];
							switch(orient){
								case UIImageOrientationUp:
									// default orientation
									break;
								case UIImageOrientationDown:
									// 180 deg rotation
									rotDeg = 180;
									break;
								case UIImageOrientationLeft:
									// 90 deg CCW
									rotDeg = 270;
									break;
								case UIImageOrientationRight:
									// 90 deg CW
									rotDeg = 90;
									break;
								case UIImageOrientationUpMirrored:
									// as above but image mirrored along other axis. horizontal flip
									break;
								case UIImageOrientationDownMirrored:
									// horizontal flip
									break;
								case UIImageOrientationLeftMirrored:
									// vertical flip
									break;
								case UIImageOrientationRightMirrored:
									// vertical flip
									break;
								default:
									rotDeg = 0;
									break;
							}
						}
					}
					//Normalize rotDeg value
					{
						while(rotDeg >= 360) rotDeg -= 360;
						while(rotDeg < 0) rotDeg += 360;
						NBASSERT(rotDeg >= 0 && rotDeg < 360)
					}
				}
				//URL
				if(!sent){
					NSURL* url = (NSURL*)[info objectForKey:UIImagePickerControllerImageURL];
					if(url != nil){
						//Image already as file
						//PRINTF_INFO("AUIOSToolImagePickerDelegate::image selected: '%s'.\n", [[url absoluteString] UTF8String]);
						//Send original
						if(!sent && app != NULL){
							app->broadcastOpenUrlImage([[url absoluteString] UTF8String], rotDeg, params.userData, params.userDataSz);
							sent = TRUE;
						}
					}
				}
				//Data
				if(!sent){
					UIImage* img = (UIImage*)[info objectForKey:UIImagePickerControllerOriginalImage];
					if(img != nil){
						//Image only in memory
						NSData* jpgData	= UIImageJPEGRepresentation(img, 0.85f);
						//PRINTF_INFO("AUIOSToolImagePickerDelegate::imageData selected (%u bytes).\n", (UI32)[jpgData length]);
						if(app != NULL){
							const void* dataPtr	= [jpgData bytes];
							const UI32 dataSz	= (UI32)[jpgData length];
							//Send original
							if(!sent){
								app->broadcastOpenFileImageData(dataPtr, dataSz, rotDeg, params.userData, params.userDataSz);
							}
						}
					} else {
						PRINTF_ERROR("AUIOSToolImagePickerDelegate, imagePickerController did not provided a NSURL nor an UIImage.\n");
					}
				}
			}
			/*if(urls != nil){
				SI32 i; const SI32 count = (SI32)urls.count;
				for(i = 0;  i < count; i++){
					NSURL* url = [urls objectAtIndex:i];
					if(url != nil){
						PRINTF_INFO("AUIOSToolFilePickerDelegate::document selected: '%s'.\n", [[url absoluteString] UTF8String]);
						if(app != NULL){
							app->broadcastOpenUrl([[url absoluteString] UTF8String], params.userData, params.userDataSz);
						}
					}
				}
			}*/
		}
		NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &params, sizeof(params));
	}
	//Hide iterface
	[picker dismissViewControllerAnimated:YES completion:nil];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
	if(_data != NULL){
		/*if(data->app != NULL){
			UIViewController* vc = (UIViewController*)data->app->getViewController();
			if(vc != nil){
				//
			}
		}*/
		[self removeFromData];
	}
	//Hide iterface
	[picker dismissViewControllerAnimated:YES completion:nil];
}

//UIDocumentPickerDelegate
/*- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
	if(_data != NULL){
		AUAppI* app = _data->app;
		STNBImagePickerData params;
		NBMemory_setZeroSt(params, STNBImagePickerData);
		NBStruct_stClone(NBImagePickerData_getSharedStructMap(), &_data->imagePicker.params, sizeof(_data->imagePicker.params), &params, sizeof(params));
		//Remove myself before the events
		{
			[self removeFromData];
		}
		//Trigger file selections
		if(app != NULL){
			if(urls != nil){
				SI32 i; const SI32 count = (SI32)urls.count;
				for(i = 0;  i < count; i++){
					NSURL* url = [urls objectAtIndex:i];
					if(url != nil){
						PRINTF_INFO("AUIOSToolFilePickerDelegate::document selected: '%s'.\n", [[url absoluteString] UTF8String]);
						if(app != NULL){
							app->broadcastOpenUrl([[url absoluteString] UTF8String], params.userData, params.userDataSz);
						}
					}
				}
			}
		}
		NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &params, sizeof(params));
	}
}*/
@end



