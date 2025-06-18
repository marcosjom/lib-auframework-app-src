//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include <windows.h>
#include <shellapi.h>
#include "AUAppGlueWinTools.h"
//
#include "nb/core/NBThread.h"
//
#include <shobjidl.h> //for IFileOpenDialog
#include <Shlobj.h> //for ILCreateFromPath()

class AUAppGlueWinToolsAppListener;

typedef struct AUAppGlueWinToolsData_ {
	AUAppI*				app;
	//
	STNBFilePickerData	filePickerParams;
	STNBImagePickerData	imgPickerParams;
} AUAppGlueWinToolsData;

//Calls

bool AUAppGlueWinTools::create(AUAppI* app, STMngrOSToolsCalls* obj){
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueWinToolsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueWinToolsData);
	NBMemory_setZeroSt(*obj, STMngrOSToolsCalls);
	data->app					= (AUAppI*)app;
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcGetPkgId					= getPkgIdentifier;
	obj->funcGetPkgIdParam				= data;
	obj->funcCanOpenUrl					= NULL;
	obj->funcCanOpenUrlParam			= NULL;
	obj->funcCanOpenFolders				= NULL;
	obj->funcCanOpenFoldersParam		= NULL;
	obj->funcOpenUrl					= openUrl;
	obj->funcOpenUrlParam				= data;
	obj->funcOpenFolder					= NULL;
	obj->funcOpenFolderParam			= NULL;
	obj->funcOpenMyStore				= openMyStore;
	obj->funcOpenMyStoreParam			= data;
	obj->funcOpenMySettings				= openMySettings;
	obj->funcOpenMySettingsParam		= data;
	obj->funcSetContentProviderAuthority = setContentProviderAuthority;
	obj->funcSetContentProviderAuthorityParam = data;
	obj->funcShareFile					= shareFile;
	obj->funcShareFileParam				= data;
	obj->funcGetTopPaddingPxs			= NULL;
	obj->funcGetTopPaddingPxsParam		= NULL;
	obj->funcGetBtmPaddingPxs			= NULL;
	obj->funcGetBtmPaddingPxsParam		= NULL;
	obj->funcGetBarStyle				= NULL;
	obj->funcGetBarStyleParam			= NULL;
	obj->funcSetBarStyle				= NULL;
	obj->funcSetBarStyleParam			= NULL;
	obj->funcConcatDeviceName			= concatDeviceName;
	obj->funcConcatDeviceNameParam		= data;
	//Device orientation
	obj->funcGetGetSupportsRotation		= supportsRotation;
	obj->funcGetGetSupportsRotationParam = data;
	obj->funcGetOrientation				= getOrientation;
	obj->funcGetOrientationParam		= data;
	//obj->funcSetOrientation			= setOrientation;
	//obj->funcSetOrientationParam		= data;
	obj->funcSetOrientationsMask		= setOrientationsMask;
	obj->funcSetOrientationsMaskParam	= data;
	//Pasteboard
	obj->funcPasteboardChangeCount		= NULL;
	obj->funcPasteboardChangeCountParam	= NULL;
	obj->funcPasteboardIsFilled			= NULL;
	obj->funcPasteboardIsFilledParam	= NULL;
	obj->funcPasteboardGetString		= NULL;
	obj->funcPasteboardGetStringParam	= NULL;
	obj->funcPasteboardSetString		= NULL;
	obj->funcPasteboardSetStringParam	= NULL;
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

bool AUAppGlueWinTools::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
		//
		{
			NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &data->filePickerParams, sizeof(data->filePickerParams));
			NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &data->imgPickerParams, sizeof(data->imgPickerParams));
		}
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

BOOL AUAppGlueWinTools::concatDeviceName(void* pData, STNBString* dst){
	BOOL r = FALSE;
	if(pData != NULL && dst != NULL){
		AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
		char buffer[MAX_COMPUTERNAME_LENGTH + 1] = "";
		DWORD size = sizeof(buffer);
		if (GetComputerName(buffer, &size)){
			PRINTF_INFO("AUAppGlueWinTools, computerName: %s\n", buffer);
			NBString_concat(dst, buffer);
			
		}
	}
	return r;
}


//

bool AUAppGlueWinTools::getPkgIdentifier(void* pData, AUCadenaMutable8* dst){
	bool r = false;
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	if(data != NULL){
		//
	}
	return r;
}

bool AUAppGlueWinTools::openUrl(void* pData, const char* url){
	bool r = false;
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	if(data != NULL){
		if((SI32)ShellExecute(0, 0, url, 0, 0 , SW_SHOW) <= 32){
			PRINTF_ERROR("AUAppGlueWinTools, could not open URL: %s\n", url);
		} else {
			r = true;
		}
	}
	return r;
}

bool AUAppGlueWinTools::openMyStore(void* pData){
	bool r = false;
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	if(data != NULL){
		//
	}
	return r;
}

bool AUAppGlueWinTools::openMySettings(void* pData){
	bool r = false;
	return r;
}

void AUAppGlueWinTools::setContentProviderAuthority(void* pData, const char* authority){
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	if(data != NULL){
		//
	}
}

//

typedef struct STAppGlueWinToolsFolderOpener_ {
	AUAppGlueWinToolsData*	data;
	STNBString				selFilepath;
} STAppGlueWinToolsFolderOpener;

SI64 AUAppGlueWinTools_FolderOpenerRunAsync_(STNBThread* t, void* pParam){
	SI64 r = 0;
	if(pParam != NULL){
		STAppGlueWinToolsFolderOpener* params = (STAppGlueWinToolsFolderOpener*)pParam;
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (!SUCCEEDED(hr)) {
			PRINTF_ERROR("AUAppGlueWinTools, 'CoInitializeEx' error.\n");
		} else {
			{
				ITEMIDLIST* pidl = ILCreateFromPath(params->selFilepath.str);
				if(!pidl){
					PRINTF_ERROR("AUAppGlueWinTools, ILCreateFromPath failed for: '%s'.\n", params->selFilepath.str);
				} else {
					if(SHOpenFolderAndSelectItems(pidl, 0, 0, 0) != S_OK){
						PRINTF_ERROR("AUAppGlueWinTools, SHOpenFolderAndSelectItems failed for: '%s'.\n", params->selFilepath.str);
					} else {
						PRINTF_INFO("AUAppGlueWinTools, SHOpenFolderAndSelectItems success for: '%s'.\n", params->selFilepath.str);
					}
					ILFree(pidl);
				}
			}
			CoUninitialize();
		}
		//Release params
		{
			NBString_release(&params->selFilepath);
			NBMemory_free(params);
			params = NULL;
		}
	}
	//Release thread
	if(t != NULL){
		NBThread_release(t);
		NBMemory_free(t);
		r = NULL;
	}
	return r;
}

bool AUAppGlueWinTools::shareFile(void* pData, const char* filepathh, const char* optNewFilename){
	bool r = false;
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	if(data != NULL){
		STNBString strPathNorm;
		NBString_initWithStr(&strPathNorm, filepathh);
		//Normalize path
		{
			NBString_replaceByte(&strPathNorm, '/', '\\');	//Unix slash for windows slash
			NBString_replace(&strPathNorm, "\\\\", "\\");	//Double-slash for single-one
		}
		//Clone file
		{
			const char* filePathToShare = strPathNorm.str;
			STNBString strNewPath;
			NBString_init(&strNewPath);
			//Create link (if necesary)
			if(!NBString_strIsEmpty(optNewFilename)){
				{
					const SI32 strLen = NBString_strLenBytes(filePathToShare);
					const SI32 iLastSlash0 = NBString_strLastIndexOf(filePathToShare, "\\", strLen);
					const SI32 iLastSlash1 = NBString_strLastIndexOf(filePathToShare, "/", strLen);
					const SI32 iLastSlash = (iLastSlash0 > iLastSlash1 ? iLastSlash0 : iLastSlash1);
					if(iLastSlash >= 0){
						NBString_concatBytes(&strNewPath, filePathToShare, (iLastSlash + 1));
					}
					NBString_concat(&strNewPath, optNewFilename);
				}
				{
					BOOL fileExists = FALSE;
					{
                        STNBFileRef stream = NBFile_alloc(NULL);
						if(NBFile_open(&stream, strNewPath.str, ENNBFileMode_Read)){
							fileExists = TRUE;
							NBFile_close(stream);
						}
						NBFile_release(&stream);
					}
					if(fileExists){
						PRINTF_INFO("AUAppGlueWinTools, symlink already exists from '%s' to '%s'.\n", filePathToShare, strNewPath.str);
						filePathToShare = strNewPath.str;
					} else {
						//Copy file
                        STNBFileRef stream = NBFile_alloc(NULL);
						if(!NBFile_open(stream, filePathToShare, ENNBFileMode_Read)){
							PRINTF_ERROR("AUAppGlueWinTools, could not create a copy from '%s' to '%s'.\n", filePathToShare, strNewPath.str);
						} else {
							NBFile_lock(stream);
							{
                                STNBFileRef stream2 = NBFile_alloc(NULL);
								if(NBFile_open(stream2, strNewPath.str, ENNBFileMode_Write)){
									NBFile_lock(stream2);
									{
										BYTE buff[406];
										while(TRUE){
											const SI32 read = NBFile_read(stream, buff, sizeof(buff));
											if(read <= 0){
												break;
											} else {
												if(NBFile_write(stream2, buff, read) != read){
													PRINTF_ERROR("AUAppGlueWinTools, could not write from '%s' to '%s'.\n", filePathToShare, strNewPath.str);
													break;
												}
											}
										};
										filePathToShare = strNewPath.str;
									}
									NBFile_unlock(stream2);
									NBFile_close(stream2);
								}
								NBFile_release(&stream2);
							}
							NBFile_unlock(stream);
							NBFile_close(stream);
						}
						NBFile_release(&stream);
					}
				}
			}
			//Documented method
			/*{
				ITEMIDLIST* pidl = ILCreateFromPath(filePathToShare);
				if(!pidl){
					PRINTF_ERROR("AUAppGlueWinTools, ILCreateFromPath failed for: '%s'.\n", filePathToShare);
				} else {
					if(SHOpenFolderAndSelectItems(pidl,0,0,0) != S_OK){
						PRINTF_ERROR("AUAppGlueWinTools, SHOpenFolderAndSelectItems failed for: '%s'.\n", filePathToShare);
					} else {
						PRINTF_INFO("AUAppGlueWinTools, SHOpenFolderAndSelectItems success for: '%s'.\n", filePathToShare);
					}
					ILFree(pidl);
				}
			}*/
			{
				//Start thread
				STAppGlueWinToolsFolderOpener* params = NBMemory_allocType(STAppGlueWinToolsFolderOpener);
				NBMemory_setZeroSt(*params, STAppGlueWinToolsFolderOpener);
				params->data = data;
				NBString_initWithStr(&params->selFilepath, filePathToShare);
				{
					STNBThread* t = NBMemory_allocType(STNBThread);
					NBThread_init(t);
					NBThread_setIsJoinable(t, FALSE);
					if (NBThread_start(t, AUAppGlueWinTools_FolderOpenerRunAsync_, params, NULL)) {
						//pFileOpen = NULL;
						params = NULL;
						t = NULL;
					}
					//Release thread (if not consumed)
					if (t != NULL) {
						NBThread_release(t);
						NBMemory_free(t);
						r = NULL;
					}
				}
				//Release param (if not consumed)
				if (params != NULL) {
					NBString_release(&params->selFilepath);
					NBMemory_free(params);
					params = NULL;
				}
			}
			NBString_release(&strNewPath);
		}
		NBString_release(&strPathNorm);
	}
	return r;
}

//

typedef struct STAppGlueWinToolsFilePicker_ {
	AUAppGlueWinToolsData*	data;
	STNBFilePickerData		pickerParams;
} STAppGlueWinToolsFilePicker;

SI64 AUAppGlueWinTools_filePickerRunAsync_(STNBThread* t, void* pParam){
	SI64 r = 0;
	if(pParam != NULL){
		STAppGlueWinToolsFilePicker* params = (STAppGlueWinToolsFilePicker*)pParam;
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (!SUCCEEDED(hr)) {
			PRINTF_ERROR("AUAppGlueWinTools, 'CoInitializeEx' error.\n");
		} else {
			// Create the FileOpenDialog object.
			IFileOpenDialog* pFileOpen = NULL;
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
			if (!SUCCEEDED(hr)) {
				PRINTF_ERROR("AUAppGlueWinTools, CoCreateInstance error.\n");
			} else {
				if (pFileOpen != NULL) {
					COMDLG_FILTERSPEC* filters = NULL;
					UI32 filtersSz = 0;
					//Build filters
					if (params->pickerParams.fileExts != NULL && params->pickerParams.fileExtsSz > 0) {
						STNBString strList;
						NBString_init(&strList);
						{
							UI32 i; for (i = 0; i < params->pickerParams.fileExtsSz; i++) {
								const char* ext = params->pickerParams.fileExts[i];
								UI32 iStartExtOnly = 0;
								if (ext[0] == '*') {
									iStartExtOnly = (ext[1] == '.' ? 2 : 1);
								} else if (ext[0] == '.') {
									iStartExtOnly = 1;
								}
								//Add
								if (!NBString_strIsEmpty(&ext[iStartExtOnly])) {
									PRINTF_INFO("AUAppGlueWinTools, adding extension('%s').\n", &ext[iStartExtOnly]);
									if (strList.length > 0) NBString_concatAsUtf16(&strList, ";");
									NBString_concatAsUtf16(&strList, "*.");
									NBString_concatAsUtf16(&strList, &ext[iStartExtOnly]);
								}
							}
							if (strList.length > 0) {
								//Add last byte char
								NBString_concatByte(&strList, '\0');
								{
									filters = NBMemory_allocType(COMDLG_FILTERSPEC);
									filtersSz = 1;
									//
									NBMemory_setZero(filters[0]);
									filters[0].pszName = (LPCWSTR)NBString_strNewBufferBytes(strList.str, strList.length);
									filters[0].pszSpec = (LPCWSTR)NBString_strNewBufferBytes(strList.str, strList.length);
									{
										STNBString str8;
										NBString_init(&str8);
										NBString_concatUtf16(&str8, (const UI16*)strList.str);
										PRINTF_INFO("AUAppGlueWinTools, filters('%s').\n", str8.str);
										NBString_release(&str8);
									}
								}
							}
						}
						NBString_release(&strList);
					}
					//Set filters
					if (filters != NULL && filtersSz > 0) {
						hr = pFileOpen->SetFileTypes(filtersSz, filters);
						if (!SUCCEEDED(hr)) {
							PRINTF_INFO("AUAppGlueWinTools, could not add %d filters.\n", filtersSz);
						}
					}
					// Show the Open dialog box.
					hr = pFileOpen->Show(params->data->app->getWindowHandle());
					// Get the file name from the dialog box.
					if (!SUCCEEDED(hr)) {
						PRINTF_ERROR("AUAppGlueWinTools, Show error.\n");
					} else {
						IShellItem* pItem = NULL;
						hr = pFileOpen->GetResult(&pItem);
						if (!SUCCEEDED(hr)) {
							PRINTF_ERROR("AUAppGlueWinTools, GetResult error.\n");
						} else {
							PWSTR pszFilePath = NULL;
							HRESULT hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							// Display the file name to the user.
							if (!SUCCEEDED(hr)) {
								PRINTF_ERROR("AUAppGlueWinTools, GetDisplayName error (%d).\n", hr);
							} else {
								STNBString filepath8;
								NBString_init(&filepath8);
								NBString_concatUtf16(&filepath8, (const UI16*)pszFilePath);
								{
									//MessageBox(NULL, filepath8.str, "File Path", MB_OK);
									params->data->app->broadcastOpenUrl(filepath8.str, params->pickerParams.userData, params->pickerParams.userDataSz);
								}
								NBString_release(&filepath8);
								//
								CoTaskMemFree(pszFilePath);
							}
							pItem->Release();
							pItem = NULL;
						}
					}
					//Release filters
					if (filters != NULL) {
						SI32 i; for (i = 0; i < filtersSz; i++) {
							COMDLG_FILTERSPEC* f = &filters[i];
							if (f->pszName != NULL) NBMemory_free((void*)f->pszName); f->pszName = NULL;
							if (f->pszSpec != NULL) NBMemory_free((void*)f->pszSpec); f->pszSpec = NULL;
						}
						NBMemory_free(filters);
						filters = NULL;
					}
				}
				//Release dialog
				pFileOpen->Release();
				pFileOpen = NULL;
			}
			CoUninitialize();
		}
		//Release params
		{
			NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &params->pickerParams, sizeof(params->pickerParams));
			NBMemory_free(params);
			params = NULL;
		}
	}
	//Release thread
	if(t != NULL){
		NBThread_release(t);
		NBMemory_free(t);
		r = NULL;
	}
	return r;
}

BOOL AUAppGlueWinTools::filePickerStart(void* pData, const STNBFilePickerData* pickerParams){
	BOOL r = FALSE;
	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	if(data != NULL){
		//Start thread
		STAppGlueWinToolsFilePicker* params = NBMemory_allocType(STAppGlueWinToolsFilePicker);
		NBMemory_setZeroSt(*params, STAppGlueWinToolsFilePicker);
		params->data = data;
		if (pickerParams != NULL) {
			NBStruct_stClone(NBFilePickerData_getSharedStructMap(), pickerParams, sizeof(*pickerParams), &params->pickerParams, sizeof(params->pickerParams));
		}
		{
			STNBThread* t = NBMemory_allocType(STNBThread);
			NBThread_init(t);
			NBThread_setIsJoinable(t, FALSE);
			if (NBThread_start(t, AUAppGlueWinTools_filePickerRunAsync_, params, NULL)) {
				//pFileOpen = NULL;
				params = NULL;
				t = NULL;
			}
			//Release thread (if not consumed)
			if (t != NULL) {
				NBThread_release(t);
				NBMemory_free(t);
				r = NULL;
			}
		}
		//Release param (if not consumed)
		if (params != NULL) {
			NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &params->pickerParams, sizeof(params->pickerParams));
			NBMemory_free(params);
			params = NULL;
		}
	}
	return r;
}

//Orientation

BOOL AUAppGlueWinTools::supportsRotation(void* data){
	return FALSE;
}

ENAppOrientationBit AUAppGlueWinTools::getOrientation(void* pData){
	ENAppOrientationBit r = (ENAppOrientationBit)0;
	//{
	//	AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
	//}
	return r;
}

/*void AUAppGlueWinTools::setOrientation(void* data, const ENAppOrientationBit orientation){
	//
}*/

	
BOOL AUAppGlueWinTools::setOrientationsMask(void* pData, const UI32 pOrientMask){
	BOOL r = FALSE;
	//AUAppGlueWinToolsData* glue = (AUAppGlueWinToolsData*)pData;
	//
	return r;
}

//Photo picker

BOOL AUAppGlueWinTools::imagePickerIsAvailable(void* pData, const ENPhotoSource src){
	BOOL r = FALSE;
	if(pData != NULL){
		//AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
		{
			switch(src) {
				case ENPhotoSource_Library:
					r = FALSE;
					break;
				case ENPhotoSource_Camera:
					r = FALSE;
					break;
				default:
					PRINTF_WARNING("AUAppGlueWinTools, imagePickerIsAvailable, unsupported ENPhotoSource(%d).\n", src);
					break;
			}
		}
	}
	return r;
}

BOOL AUAppGlueWinTools::imagePickerStart(void* pData, const ENPhotoSource src, const STNBImagePickerData* pickerParams){
	BOOL r = FALSE;
	if(pData != NULL){
		/*AUAppGlueWinToolsData* data = (AUAppGlueWinToolsData*)pData;
		//
		{
			NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &data->imgPickerParams, sizeof(data->imgPickerParams));
			if(pickerParams != NULL){
				NBStruct_stClone(NBImagePickerData_getSharedStructMap(), pickerParams, sizeof(*pickerParams), &data->imgPickerParams, sizeof(data->imgPickerParams));
			}
		}
		//
		switch(src) {
			case ENPhotoSource_Library:
				{
					STNBFilePickerData pp;
					NBMemory_setZeroSt(pp, STNBFilePickerData);
					//Set mime
					{
						pp.mimeTypes	= NBMemory_allocTypes(char*, 1);
						pp.mimeTypes[0]	= NBString_strNewBuffer("image/*");
						pp.mimeTypesSz	= 1;
					}
					//Set user data
					if(pickerParams != NULL){
						if(pickerParams->userData != NULL && pickerParams->userDataSz > 0){
							pp.userDataSz	= pickerParams->userDataSz;
							pp.userData		= (BYTE*)NBMemory_alloc(pickerParams->userDataSz);
							NBMemory_copy(pp.userData, pickerParams->userData, pickerParams->userDataSz);
						}
					}
					//Open
					{
						r = AUAppGlueWinTools::filePickerStart(pData, jEnv, jActivity, &pp);
					}
					NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &pp, sizeof(pp));
				}
				break;
			case ENPhotoSource_Camera:
				break;
			default:
				PRINTF_WARNING("AUAppGlueWinTools, imagePickerIsAvailable, unsupported ENPhotoSource(%d).\n", src);
				break;
		}*/
	}
	return r;
}
