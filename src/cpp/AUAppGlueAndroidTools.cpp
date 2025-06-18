//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
//#include "NBMngrNotifs.h" //ToDo: remove
#include "AUAppGlueAndroidTools.h"
//Android and JNI headers
#include <jni.h>

#ifdef __ANDROID__
//is android
#endif

#define NB_FILE_SEL_INTENT_PARAMS_NAME	"NB_PICKER_PARAMS"
#define NB_FILE_SEL_INTENT_CODE			7852
#define NB_FILE_SHARE_INTENT_CODE		7857
#define NB_FILE_CAM_INTENT_CODE			9165
//

class AUAppGlueAndroidToolsAppListener;

typedef struct AUAppGlueAndroidToolsData_ {
	AUAppI*				app;
	AUAppGlueAndroidToolsAppListener* listener;
	//
	STNBString			contentProviderAuthority;
	STNBFilePickerData	filePickerParams;
	STNBImagePickerData	imgPickerParams;
} AUAppGlueAndroidToolsData;

class AUAppGlueAndroidToolsAppListener: public AUAppIntentListener, public AUAppActResultListener {
	public:
		AUAppGlueAndroidToolsAppListener(AUAppGlueAndroidToolsData* data){
			_data = data;
		}
		virtual ~AUAppGlueAndroidToolsAppListener(){
			_data = NULL;
		}
		//AUAppIntentListener
		void appIntentReceived(AUAppI* app, void* intent /*jobject::Intent*/){
			AUAppGlueAndroidTools::analyzeIntent(_data, 0, 0, intent);
		}
		//AUAppActResultListener
		void appActResultReceived(AUAppI* app, SI32 request, SI32 response, void* intent /*jobject::Intent*/){
			AUAppGlueAndroidTools::analyzeIntent(_data, request, response, intent);
		}
	private:
		AUAppGlueAndroidToolsData* _data;
};

//Calls

bool AUAppGlueAndroidTools::create(AUAppI* app, STMngrOSToolsCalls* obj){
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidToolsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAndroidToolsData);
	NBMemory_setZeroSt(*obj, STMngrOSToolsCalls);
	data->app					= (AUAppI*)app;
	data->listener = new AUAppGlueAndroidToolsAppListener(data);
	data->app->addAppIntentListener(data->listener);
	data->app->addAppActivityResultListener(data->listener);
	//
	NBString_init(&data->contentProviderAuthority);
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
	obj->funcWallpaperCanBeSet			= wallpaperCanBeSet;
	obj->funcWallpaperCanBeSetParam		= data;
	obj->funcWallpaperSet				= wallpaperSet;
	obj->funcWallpaperSetParam			= data;
	//
	return true;
}

bool AUAppGlueAndroidTools::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		//
		{
			NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &data->filePickerParams, sizeof(data->filePickerParams));
			NBStruct_stRelease(NBImagePickerData_getSharedStructMap(), &data->imgPickerParams, sizeof(data->imgPickerParams));
		}
		//
		{
			NBString_release(&data->contentProviderAuthority);
		}
		//
		if(data->listener != NULL){
			data->app->removeAppIntentListener(data->listener);
			data->app->removeAppActivityResultListener(data->listener);
			delete data->listener;
			data->listener = NULL;
		}
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

BOOL AUAppGlueAndroidTools::concatDeviceName(void* pData, STNBString* dst){
	BOOL r = FALSE;
	if(pData != NULL && dst != NULL){
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv();
		if(jEnv != NULL){
			jclass clsBuild	= jEnv->FindClass("android/os/Build"); NBASSERT(clsBuild != NULL)
			if(clsBuild != NULL){
				jfieldID fManuf	= jEnv->GetStaticFieldID(clsBuild, "MANUFACTURER", "Ljava/lang/String;"); NBASSERT(fManuf != NULL)
				jfieldID fModel	= jEnv->GetStaticFieldID(clsBuild, "MODEL", "Ljava/lang/String;"); NBASSERT(fModel != NULL)
				if(fManuf != NULL && fModel != NULL){
					NBString_empty(dst);
					//Concat manufacturer
					{
						jstring jValue = (jstring)jEnv->GetStaticObjectField(clsBuild, fManuf);
						if(jValue != NULL){
							const char* strValue = jEnv->GetStringUTFChars(jValue, 0);
							{
								PRINTF_INFO("AUAppGlueAndroidTools, manufacturer '%s'.\n", strValue);
								//Capitalize name
								{
									BOOL prevIsSpace = TRUE;
									const char* strPtr = strValue;
									while(*strPtr != '\0'){
										if(prevIsSpace){
											NBString_concatByte(dst, NBEncoding_asciiUpper(*strPtr));
										} else {
											NBString_concatByte(dst, *strPtr);
										}
										prevIsSpace = (*strPtr == ' ' || *strPtr == '\t' || *strPtr == '\r' || *strPtr == '\n' ? TRUE : FALSE);
										strPtr++;
									}
								}
							}
							jEnv->ReleaseStringUTFChars(jValue, strValue);
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jValue)
					}
					//Concat model (ignoring manufacturer start)
					{
						jstring jValue = (jstring)jEnv->GetStaticObjectField(clsBuild, fModel);
						if(jValue != NULL){
							const char* strValue = jEnv->GetStringUTFChars(jValue, 0);
							{
								PRINTF_INFO("AUAppGlueAndroidTools, model '%s'.\n", strValue);
								if(NBString_strIndexOfLike(strValue, dst->str, 0) == 0){
									//Concat remainig
									NBString_concat(dst, &strValue[dst->length]);
								} else {
									//Concat
									if(dst->length > 0) NBString_concatByte(dst, ' ');
									NBString_concat(dst, strValue);
								}
							}
							jEnv->ReleaseStringUTFChars(jValue, strValue);
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jValue)
					}
				}
				r = TRUE;
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsBuild)
		}
	}
	return r;
}


//

bool AUAppGlueAndroidTools::getPkgIdentifier(void* pData, AUCadenaMutable8* dst){
	bool r = false;
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv();
		if(jEnv != NULL){
			jstring jStrPkgName = (jstring) jniGlue->getPackageName(jEnv, jniGlue->jActivity());
			const char* bundleId = jEnv->GetStringUTFChars(jStrPkgName, 0);
			if(dst != NULL) dst->establecer(bundleId);
			jEnv->ReleaseStringUTFChars(jStrPkgName, bundleId);
			r = true;
		}
	}
	return r;
}

bool AUAppGlueAndroidTools::openUrl(void* pData, const char* url){
	bool r = false;
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		r = AUAppGlueAndroidTools::openUrl(jniGlue->curEnv(), jniGlue->jActivity(), url);
	}
	return r;
}

bool AUAppGlueAndroidTools::openMyStore(void* pData){
	bool r = false;
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv();
		jobject jActivity	= (jobject)jniGlue->jActivity();
		jstring pkgName		= (jstring)AUAppGlueAndroidJNI::getPackageName(jEnv, jActivity);
		if(pkgName != NULL){
			const char* strPkgName = jEnv->GetStringUTFChars(pkgName, 0);
			AUCadenaMutable8* strTmp = new AUCadenaMutable8();
			//Try market URL
			if(!r){
				strTmp->establecer("market://details?id=");
				strTmp->agregar(strPkgName);
				if(AUAppGlueAndroidTools::openUrl(jEnv, jActivity, strTmp->str())){
					r = true;
				}
			}
			//Try web url
			if(!r){
				strTmp->establecer("http://play.google.com/store/apps/details?id=");
				strTmp->agregar(strPkgName);
				if(AUAppGlueAndroidTools::openUrl(jEnv, jActivity, strTmp->str())){
					r = true;
				}
			}
			strTmp->liberar(NB_RETENEDOR_THIS);
			jEnv->ReleaseStringUTFChars(pkgName, strPkgName);
		}
	}
	return r;
}

bool AUAppGlueAndroidTools::openMySettings(void* pData){
	bool r = false;
	return r;
}

void AUAppGlueAndroidTools::setContentProviderAuthority(void* pData, const char* authority){
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		NBString_set(&data->contentProviderAuthority, authority);
		//PRINTF_INFO("AUAppGlueAndroidTools, set authority: '%s'.\n", data->contentProviderAuthority.str);
	}
}

bool AUAppGlueAndroidTools::shareFile(void* pData, const char* filepathh, const char* optNewFilename){
	bool r = false;
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv 		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jActivity	= (jobject)jniGlue->jActivity();
		if(jEnv != NULL && jActivity != NULL){
			jclass clsFileProv = jEnv->FindClass("androidx/core/content/FileProvider");
			if(clsFileProv == NULL){
				if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
				PRINTF_ERROR("AUAppGlueAndroidTools, Exception FileProvider class not found.\n");
			} else {
				jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
				jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
				jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
				jclass clsUriBldr	= jEnv->FindClass("android/net/Uri$Builder"); NBASSERT(clsUriBldr != NULL)
				jclass clsFile		= jEnv->FindClass("java/io/File"); NBASSERT(clsFile != NULL)
				if(clsContext != NULL && clsActivity != NULL && clsIntent != NULL && clsUriBldr != NULL && clsFile != NULL){
					jmethodID mUriBldrInit	= jEnv->GetMethodID(clsUriBldr, "<init>", "()V"); NBASSERT(mUriBldrInit != NULL)
					jmethodID mPath			= jEnv->GetMethodID(clsUriBldr, "path", "(Ljava/lang/String;)Landroid/net/Uri$Builder;"); NBASSERT(mPath != NULL)
					jmethodID mBuild		= jEnv->GetMethodID(clsUriBldr, "build", "()Landroid/net/Uri;"); NBASSERT(mBuild != NULL)
					jmethodID mCreateChooser = jEnv->GetStaticMethodID(clsIntent, "createChooser", "(Landroid/content/Intent;Ljava/lang/CharSequence;)Landroid/content/Intent;"); NBASSERT(mCreateChooser != NULL)
					jmethodID mInit			= jEnv->GetMethodID(clsIntent, "<init>", "()V"); NBASSERT(mInit != NULL)
					jmethodID mSetAct		= jEnv->GetMethodID(clsIntent, "setAction", "(Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mSetAct != NULL)
					jmethodID mSetType		= jEnv->GetMethodID(clsIntent, "setType", "(Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mSetType != NULL)
					jmethodID mPutExtra		= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;Landroid/os/Parcelable;)Landroid/content/Intent;"); NBASSERT(mPutExtra != NULL)
					jmethodID mSetData		= jEnv->GetMethodID(clsIntent, "setData", "(Landroid/net/Uri;)Landroid/content/Intent;"); NBASSERT(mSetData != NULL)
					jmethodID mSetFlags		= jEnv->GetMethodID(clsIntent, "setFlags", "(I)Landroid/content/Intent;"); NBASSERT(mSetFlags != NULL)
					jmethodID mStartAct		= jEnv->GetMethodID(clsActivity, "startActivityForResult", "(Landroid/content/Intent;I)V"); NBASSERT(mStartAct != NULL)
					jmethodID mFileInit		= jEnv->GetMethodID(clsFile, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mUriBldrInit != NULL)
					jmethodID mGetAbsPath	= jEnv->GetMethodID(clsFile, "getAbsolutePath", "()Ljava/lang/String;"); NBASSERT(mGetAbsPath != NULL)
					jmethodID mGetFilesDir	= jEnv->GetMethodID(clsContext, "getFilesDir", "()Ljava/io/File;"); NBASSERT(mGetFilesDir != NULL) //files-path
					jmethodID mGetCacheDir	= jEnv->GetMethodID(clsContext, "getCacheDir", "()Ljava/io/File;"); NBASSERT(mGetCacheDir != NULL) //cache-path
					jmethodID mGetCacheExtDir	= jEnv->GetMethodID(clsContext, "getExternalCacheDir", "()Ljava/io/File;"); NBASSERT(mGetCacheExtDir != NULL) //external-cache-path
					jmethodID mGetUriForFile	= jEnv->GetStaticMethodID(clsFileProv, "getUriForFile", "(Landroid/content/Context;Ljava/lang/String;Ljava/io/File;)Landroid/net/Uri;"); NBASSERT(mGetUriForFile != NULL)
					if(mUriBldrInit != NULL && mPath != NULL && mBuild != NULL && mCreateChooser != NULL && mInit != NULL && mSetAct != NULL && mSetType != NULL && mPutExtra != NULL && mSetData != NULL && mSetFlags != NULL && mStartAct != NULL && mFileInit != NULL && mGetAbsPath != NULL && mGetFilesDir != NULL && mGetCacheDir != NULL && mGetCacheExtDir != NULL && mGetUriForFile != NULL){
						const char* filePathToShare = filepathh;
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
									PRINTF_INFO("AUAppGlueAndroidTools, symlink already exists from '%s' to '%s'.\n", filePathToShare, strNewPath.str);
									filePathToShare = strNewPath.str;
								} else {
									//Copy file
                                    STNBFileRef stream = NBFile_alloc(NULL);
									if(!NBFile_open(stream, filePathToShare, ENNBFileMode_Read)){
										PRINTF_ERROR("AUAppGlueAndroidTools, could not create a copy from '%s' to '%s'.\n", filePathToShare, strNewPath.str);
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
																PRINTF_ERROR("AUAppGlueAndroidTools, could not write from '%s' to '%s'.\n", filePathToShare, strNewPath.str);
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
						{
							jstring jPath = jEnv->NewStringUTF(filePathToShare);
							if(jPath == NULL){
								PRINTF_ERROR("AUAppGlueAndroidTools, could not build '//content:' path for sharing: '%s'.\n", filePathToShare);
							} else {
								jobject jFile = jEnv->NewObject(clsFile, mFileInit, jPath);
								if(jFile == NULL){
									if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
									PRINTF_ERROR("AUAppGlueAndroidTools, Exception creating File for sharing: '%s'.\n", filePathToShare);
								} else {
									jstring jAuthority = jEnv->NewStringUTF(data->contentProviderAuthority.str);
									jobject jUri = jEnv->CallStaticObjectMethod(clsFileProv, mGetUriForFile, jActivity, jAuthority, jFile);
									if(jUri == NULL){
										if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
										PRINTF_ERROR("AUAppGlueAndroidTools, Exception getUriForFile authority('%s') for sharing: '%s'.\n", data->contentProviderAuthority.str, filePathToShare);
									} else {
										jstring jAct		= jEnv->NewStringUTF("android.intent.action.SEND"); //ACTION_SEND
										jstring jExtStream	= jEnv->NewStringUTF("android.intent.extra.STREAM"); //EXTRA_STREAM
										jstring jType		= jEnv->NewStringUTF("application/pdf"); //ACTION_SEND
										jobject jIntent		= jEnv->NewObject(clsIntent, mInit);
										if(jIntent != NULL){
											jEnv->CallObjectMethod(jIntent, mSetAct, jAct);
											jEnv->CallObjectMethod(jIntent, mSetType, jType);
											jEnv->CallObjectMethod(jIntent, mPutExtra, jExtStream, jUri);
											//jEnv->CallObjectMethod(jIntent, mSetData, jUri);
											jEnv->CallObjectMethod(jIntent, mSetFlags, (jint)1); //1 = Intent.FLAG_GRANT_READ_URI_PERMISSION, 2 = Intent.FLAG_GRANT_WRITE_URI_PERMISSION
											{
												jobject jIntent2 = jEnv->CallStaticObjectMethod(clsIntent, mCreateChooser, jIntent, NULL);
												if(jIntent2 != NULL){
													//jEnv->CallObjectMethod(jIntent2, mSetData, jUri);
													jEnv->CallObjectMethod(jIntent2, mSetFlags, (jint)1); //1 = Intent.FLAG_GRANT_READ_URI_PERMISSION, 2 = Intent.FLAG_GRANT_WRITE_URI_PERMISSION
													jEnv->CallVoidMethod(jActivity, mStartAct, jIntent2, (jint)NB_FILE_SHARE_INTENT_CODE);
													if(jEnv->ExceptionCheck()){
														jEnv->ExceptionClear();
													} else {
														r = TRUE;
													}
												}
												NBJNI_DELETE_REF_LOCAL(jEnv, jIntent2)
											}
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, jAct)
										NBJNI_DELETE_REF_LOCAL(jEnv, jExtStream)
										NBJNI_DELETE_REF_LOCAL(jEnv, jType)
										NBJNI_DELETE_REF_LOCAL(jEnv, jIntent)
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jUri)
									NBJNI_DELETE_REF_LOCAL(jEnv, jAuthority)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jFile)
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jPath)
						}
						//Trying manually building path
						/*{
						 jstring jPath = NULL;
						 //Analyze files mapping:
						 {
						 //getFilesDir() --> "files-path"
						 //getCacheDir() --> "cache-path"
						 //getExternalCacheDir() --> "external-cache-path"
						 jmethodID methods[] = { mGetFilesDir, mGetCacheDir, mGetCacheExtDir };
						 const char* paths[] = { "files-path", "cache-path", "external-cache-path" };
						 SI32 i; const SI32 count = (sizeof(methods) / sizeof(methods[0]));
						 NBASSERT((sizeof(methods) / sizeof(methods[0])) == (sizeof(paths) / sizeof(paths[0])))
						 for(i = 0; i < count && jPath == NULL; i++){
						 jobject jPathBase = jEnv->CallObjectMethod(jActivity, methods[i]);
						 if(jPathBase != NULL){
						 jstring jAbsPath = (jstring)jEnv->CallObjectMethod(jPathBase, mGetAbsPath);
						 if(jAbsPath != NULL){
						 const char* strAbsPath = jEnv->GetStringUTFChars(jAbsPath, 0);
						 if(NBString_strStartsWith(filePathToShare, strAbsPath)){
						 STNBString strBase;
						 NBString_init(&strBase);
						 NBString_concat(&strBase, "content://");
						 NBString_concat(&strBase, data->contentProviderAuthority.str); NBString_concat(&strBase, "/");
						 NBString_concat(&strBase, paths[i]);
						 if(strAbsPath[NBString_strLenBytes(strAbsPath) - 1] == '/'){
						 NBString_concatByte(&strBase, '/');
						 }
						 {
						 STNBString str;
						 NBString_initWithStr(&str, filePathToShare);
						 NBString_replace(&str, strAbsPath, strBase.str);
						 PRINTF_INFO("AUAppGlueAndroidTools, content path: '%s'.\n", str.str);
						 jPath = jEnv->NewStringUTF(str.str);
						 NBString_release(&str);
						 }
						 NBString_release(&strBase);
						 }
						 jEnv->ReleaseStringUTFChars(jAbsPath, strAbsPath);
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jAbsPath)
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jPathBase)
						 }
						 }
						 //Build path
						 if(jPath == NULL){
						 PRINTF_ERROR("AUAppGlueAndroidTools, could not build '//content:' path for sharing: '%s'.\n", filePathToShare);
						 } else {
						 jobject jUriBldr	= jEnv->NewObject(clsUriBldr, mUriBldrInit);
						 if(jUriBldr != NULL){
						 PRINTF_INFO("AUAppGlueAndroidTools, BLDR created.\n");
						 jstring jAct = jEnv->NewStringUTF("android.intent.action.SEND"); //ACTION_SEND
						 jstring jExtStream = jEnv->NewStringUTF("android.intent.extra.STREAM"); //EXTRA_STREAM
						 jstring jType = jEnv->NewStringUTF("application/pdf"); //ACTION_SEND
						 jEnv->CallObjectMethod(jUriBldr, mPath, jPath);
						 {
						 PRINTF_INFO("AUAppGlueAndroidTools, jStrs created.\n");
						 jobject jUri = jEnv->CallObjectMethod(jUriBldr, mBuild);
						 if(jUri != NULL){
						 PRINTF_INFO("AUAppGlueAndroidTools, jUri created.\n");
						 jobject jIntent	= jEnv->NewObject(clsIntent, mInit);
						 if(jIntent != NULL){
						 PRINTF_INFO("AUAppGlueAndroidTools, jIntent created.\n");
						 jEnv->CallObjectMethod(jIntent, mSetAct, jAct);
						 jEnv->CallObjectMethod(jIntent, mSetType, jType);
						 jEnv->CallObjectMethod(jIntent, mPutExtra, jExtStream, jUri);
						 //jEnv->CallObjectMethod(jIntent, mSetData, jUri);
						 jEnv->CallObjectMethod(jIntent, mSetFlags, (jint)1); //1 = Intent.FLAG_GRANT_READ_URI_PERMISSION, 2 = Intent.FLAG_GRANT_WRITE_URI_PERMISSION
						 {
						 PRINTF_INFO("AUAppGlueAndroidTools, jIntent configurated.\n");
						 jobject jIntent2 = jEnv->CallStaticObjectMethod(clsIntent, mCreateChooser, jIntent, NULL);
						 PRINTF_INFO("AUAppGlueAndroidTools, jIntent2 created.\n");
						 if(jIntent2 != NULL){
						 //jEnv->CallObjectMethod(jIntent2, mSetData, jUri);
						 jEnv->CallObjectMethod(jIntent2, mSetFlags, (jint)1); //1 = Intent.FLAG_GRANT_READ_URI_PERMISSION, 2 = Intent.FLAG_GRANT_WRITE_URI_PERMISSION
						 jEnv->CallVoidMethod(jActivity, mStartAct, jIntent2, (jint)NB_FILE_SHARE_INTENT_CODE);
						 PRINTF_INFO("AUAppGlueAndroidTools, start called.\n");
						 if(jEnv->ExceptionCheck()){
						 jEnv->ExceptionClear();
						 } else {
						 r = TRUE;
						 }
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jIntent2)
						 }
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jIntent)
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jUri)
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jAct)
						 NBJNI_DELETE_REF_LOCAL(jEnv, jExtStream)
						 NBJNI_DELETE_REF_LOCAL(jEnv, jType)
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jUriBldr)
						 }
						 NBJNI_DELETE_REF_LOCAL(jEnv, jPath)
						 }*/
						NBString_release(&strNewPath);
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsUriBldr)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsFile)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsFileProv)
		}
		/*
		Intent intent = new Intent();
		intent.setType("application/pdf");
		intent.setAction(Intent.ACTION_GET_CONTENT);
		startActivityForResult(Intent.createChooser(intent, "Select Pdf"), SELECT_PDF_DIALOG);
		 //
		 Intent shareIntent = new Intent();
		 shareIntent.setAction(Intent.ACTION_SEND);
		 shareIntent.putExtra(Intent.EXTRA_STREAM, uriToImage);
		 shareIntent.setType("image/jpeg");
		 startActivity(Intent.createChooser(shareIntent, getResources().getText(R.string.send_to)));
		*/
	}
	return r;
}

//

BOOL AUAppGlueAndroidTools::filePickerStart(void* pData, const STNBFilePickerData* pickerParams){
	BOOL r = FALSE;
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		r = AUAppGlueAndroidTools::filePickerStart(data, jniGlue->curEnv(), jniGlue->jActivity(), pickerParams);
	}
	return r;
}

//

bool AUAppGlueAndroidTools::analyzeIntent(void* pData, const SI32 request, const SI32 response, void* pIntent /*jobject*/){
	bool r = false;
	if(pData != NULL && pIntent != NULL){
		PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent.\n");
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			STNBString mainActClass;
			NBString_init(&mainActClass);
			{
				jstring jStrPkgName = (jstring) jniGlue->getPackageName(jEnv, jniGlue->jActivity());
				const char* bundleId = jEnv->GetStringUTFChars(jStrPkgName, 0);
				{
					NBString_concat(&mainActClass, bundleId);
					NBString_concat(&mainActClass, ".MainActivity");
					NBString_replace(&mainActClass, ".", "/");
				}
				jEnv->ReleaseStringUTFChars(jStrPkgName, bundleId);
			}
			{
				jobject jIntent = (jobject) pIntent;
				if(request == NB_FILE_SEL_INTENT_CODE){
					PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent (NB_FILE_SEL_INTENT_CODE).\n");
					jobject jActivity	= (jobject)jniGlue->jActivity();
					jclass clsMainAct	= jEnv->FindClass(mainActClass.str); NBASSERT(clsMainAct != NULL)
					if(clsMainAct != NULL){
						jmethodID mAnlz	= jEnv->GetMethodID(clsMainAct, "analizeIntentOpenable", "(IILandroid/content/Intent;)Z"); NBASSERT(mAnlz != NULL)
						jboolean rr = jEnv->CallBooleanMethod(jActivity, mAnlz, (jint)request, (jint)response, jIntent);
						r = (rr ? TRUE : FALSE);
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsMainAct)
				} else if(request == NB_FILE_SHARE_INTENT_CODE){
					PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent (NB_FILE_SHARE_INTENT_CODE).\n");
					/*jobject jActivity	= (jobject)jniGlue->jActivity();
					 jclass clsMainAct	= jEnv->FindClass(mainActClass.str); NBASSERT(clsMainAct != NULL)
					 if(clsMainAct != NULL){
					 jmethodID mAnlz	= jEnv->GetMethodID(clsMainAct, "analizeIntentOpenable", "(IILandroid/content/Intent;)Z"); NBASSERT(mAnlz != NULL)
					 jboolean rr = jEnv->CallBooleanMethod(jActivity, mAnlz, (jint)request, (jint)response, jIntent);
					 r = (rr ? TRUE : FALSE);
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsMainAct)*/
				} else if(request == NB_FILE_CAM_INTENT_CODE){
					PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent (NB_FILE_CAM_INTENT_CODE).\n");
					jclass clsBundle	= jEnv->FindClass("android/os/Bundle"); NBASSERT(clsBundle != NULL)
					jclass clsBundleB	= jEnv->FindClass("android/os/BaseBundle"); NBASSERT(clsBundleB != NULL)
					jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
					if(clsBundle != NULL && clsIntent != NULL){
						jmethodID mGetExtras	= jEnv->GetMethodID(clsIntent, "getExtras", "()Landroid/os/Bundle;"); NBASSERT(mGetExtras != NULL)
						jmethodID mGet			= jEnv->GetMethodID(clsBundleB, "get", "(Ljava/lang/String;)Ljava/lang/Object;"); NBASSERT(mGet != NULL)
						jobject jBundle			= jEnv->CallObjectMethod(jIntent, mGetExtras);
						if(jBundle != NULL){
							jstring jData		= jEnv->NewStringUTF("data");
							jobject jBmpImg		= jEnv->CallObjectMethod(jBundle, mGet, jData);
							if(jBmpImg != NULL){
								STNBString pathStr;
								NBString_initWithStr(&pathStr, NBGestorArchivos::rutaHaciaRecursoEnCache("__cameraPhoto.jpg"));
								PRINTF_INFO("pathStr: '%s'.\n", pathStr.str);
								jstring jPath	= jEnv->NewStringUTF(pathStr.str);
								{
									jclass clsFOutS		= jEnv->FindClass("java/io/FileOutputStream"); NBASSERT(clsFOutS != NULL)
									jclass clsBmp		= jEnv->FindClass("android/graphics/Bitmap"); NBASSERT(clsBmp != NULL)
									jclass clsBmpFmt	= jEnv->FindClass("android/graphics/Bitmap$CompressFormat"); NBASSERT(clsBmpFmt != NULL)
									jfieldID fJpg		= jEnv->GetStaticFieldID(clsBmpFmt, "JPEG", "Landroid/graphics/Bitmap$CompressFormat;"); NBASSERT(fJpg != NULL)
									jmethodID mFOutInit	= jEnv->GetMethodID(clsFOutS, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mFOutInit != NULL)
									jmethodID mCompress = jEnv->GetMethodID(clsBmp, "compress", "(Landroid/graphics/Bitmap$CompressFormat;ILjava/io/OutputStream;)Z"); NBASSERT(mCompress != NULL)
									jobject jJpg		= jEnv->GetStaticObjectField(clsBmpFmt, fJpg);
									jobject jFOFile		= jEnv->NewObject(clsFOutS, mFOutInit, jPath);
									if(jFOFile == NULL){
										if(jEnv->ExceptionCheck()){
											jEnv->ExceptionClear();
										}
									} else {
										jboolean rr = jEnv->CallBooleanMethod(jBmpImg, mCompress, jJpg, 80, jFOFile);
										if(!rr){
											if(jEnv->ExceptionCheck()){
												jEnv->ExceptionClear();
											}
										} else {
											data->app->broadcastOpenUrlImage(pathStr.str, 0, data->imgPickerParams.userData, data->imgPickerParams.userDataSz);
										}
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jFOFile)
									NBJNI_DELETE_REF_LOCAL(jEnv, jJpg)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsFOutS)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsBmpFmt)
									NBJNI_DELETE_REF_LOCAL(jEnv, clsBmp)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jPath)
								NBString_release(&pathStr);
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jData)
							NBJNI_DELETE_REF_LOCAL(jEnv, jBmpImg)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jBundle)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsBundleB)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsBundle)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
				} else {
					jobject jActivity	= (jobject)jniGlue->jActivity();
					jclass clsMainAct	= jEnv->FindClass(mainActClass.str); NBASSERT(clsMainAct != NULL)
					if(clsMainAct != NULL){
						jmethodID mAnlz	= jEnv->GetMethodID(clsMainAct, "analizeIntentOpenable", "(IILandroid/content/Intent;)Z"); NBASSERT(mAnlz != NULL)
						jboolean rr = jEnv->CallBooleanMethod(jActivity, mAnlz, (jint)request, (jint)-1 /*RESULT_OK*/, jIntent);
						r = (rr ? TRUE : FALSE);
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsMainAct)
					/*jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
					 jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
					 if(jContext != NULL){
					 if(pIntent != NULL){
					 jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
					 jclass clsUri		= jEnv->FindClass("android/net/Uri"); NBASSERT(clsUri != NULL)
					 if(clsIntent != NULL && clsUri != NULL){
					 jmethodID mGetData	= jEnv->GetMethodID(clsIntent, "getData", "()Landroid/net/Uri;"); NBASSERT(mGetData != NULL)
					 jmethodID mToString	= jEnv->GetMethodID(clsUri, "toString", "()Ljava/lang/String;"); NBASSERT(mToString != NULL)
					 if(mGetData != NULL && mToString != NULL){
					 jobject jUri = jEnv->CallObjectMethod(jIntent, mGetData);
					 if(jUri == NULL){
					 PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent, no Uri.\n");
					 } else {
					 jstring jStrUri = (jstring)jEnv->CallObjectMethod(jUri, mToString);
					 const char* strUri = jEnv->GetStringUTFChars(jStrUri, 0);
					 if(AUCadena8::indiceDe(strUri, "://", 0) == -1){
					 PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent, uri has no '://': '%s'\n", strUri);
					 } else {
					 if(NBString_strStartsWith(strUri, "file://")){
					 PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent, 'file://' uri '%s'.\n", strUri);
					 data->app->broadcastOpenUrl(strUri, NULL, 0);
					 } else if(NBString_strStartsWith(strUri, "content://")){
					 PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent, 'content://' uri '%s'.\n", strUri);
					 / *
					 //Copy file from content resolver
					 jclass clsContResolver	= jEnv->FindClass("android/content/ContentResolver"); NBASSERT(clsContResolver != NULL)
					 jclass clsInputStream	= jEnv->FindClass("java/io/InputStream"); NBASSERT(clsInputStream != NULL)
					 jclass clsCursor		= jEnv->FindClass("java/io/InputStream"); NBASSERT(clsCursor != NULL)
					 if(clsContResolver != NULL && clsInputStream != NULL && clsCursor != NULL){
					 jmethodID mGetContentResolver = jEnv->GetMethodID(clsContext, "getContentResolver", "()Landroid/content/ContentResolver;"); NBASSERT(mGetContentResolver != NULL)
					 jmethodID mOpenInputStream = jEnv->GetMethodID(clsContResolver, "openInputStream", "(Landroid/net/Uri;)Ljava/io/InputStream;"); NBASSERT(mOpenInputStream != NULL)
					 jmethodID mQuery		= jEnv->GetMethodID(clsContResolver, "query", "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;"); NBASSERT(mQuery != NULL)
					 jmethodID mGetColIdx 	= jEnv->GetMethodID(clsCursor, "getColumnIndex", "(Ljava/lang/String;)I"); NBASSERT(mGetColIdx != NULL)
					 jmethodID mMoveToFirst 	= jEnv->GetMethodID(clsCursor, "moveToFirst", "()Z"); NBASSERT(mMoveToFirst != NULL)
					 jmethodID mGetString 	= jEnv->GetMethodID(clsCursor, "getString", "(I)Ljava/lang/String;"); NBASSERT(mGetString != NULL)
					 jmethodID mClose	 	= jEnv->GetMethodID(clsInputStream, "close", "()V"); NBASSERT(mClose != NULL)
					 if(mGetContentResolver != NULL && mOpenInputStream != NULL && mQuery != NULL && mGetColIdx != NULL && mMoveToFirst != NULL && mGetString != NULL && mClose != NULL){
					 jobject jContentResolver = jEnv->CallObjectMethod(jContext, mGetContentResolver); NBASSERT(jContentResolver != NULL)
					 if(jContentResolver != NULL){
					 jstring jName = NULL; const char* strName = NULL;
					 //Load name
					 {
					 jobject jCursor = jEnv->CallObjectMethod(jContentResolver, mQuery, jUri, NULL, NULL, NULL, NULL);
					 if(jCursor == NULL){
					 if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
					 PRINTF_ERROR("AUAppGlueAndroidTools, analyzeIntent, query returned exception for uri.\n");
					 } else {
					 jstring jColName	= jEnv->NewStringUTF("_display_name"); //OpenableColumns.DISPLAY_NAME
					 const jint nameIdx	= jEnv->CallIntMethod(jCursor, mGetColIdx, jColName);
					 if(nameIdx == -1){
					 PRINTF_ERROR("AUAppGlueAndroidTools, analyzeIntent, cursor missing DISPLAY_NAME for uri.\n");
					 } else {
					 if(!jEnv->CallBooleanMethod(jCursor, mMoveToFirst)){
					 PRINTF_ERROR("AUAppGlueAndroidTools, analyzeIntent, cursor moveToFirst failed for uri.\n");
					 } else {
					 jName = jEnv->CallObjectMethod(jCursor, mGetString, nameIdx);
					 if(jName != NULL){
					 strName = jEnv->GetStringUTFChars(jName, 0);
					 PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent, uri with display name: '%s'.\n", strName);
					 }
					 }
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, jColName)
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
					 }
					 //Copy file
					 {
					 //if(copyFileStream(new File(outPath), uri,_activity)) {
					 }
					 //Open stream
					 / *if(!NBString_strIsEmpty(strName)){
					 jobject jInputStream = jEnv->CallObjectMethod(jContentResolver, mOpenInputStream, jUri);
					 if(jInputStream == NULL){
					 if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
					 PRINTF_ERROR("AUAppGlueAndroidTools, analyzeIntent, openInputStream returned exception for uri.\n");
					 } else {
					 PRINTF_INFO("AUAppGlueAndroidTools, analyzeIntent, inputStream opened: '%s'.\n", strName);
					 //Cursor	query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
					 //Cursor returnCursor = _activity.getContentResolver().query(returnUri, null, null, null, null);
					 //int nameIndex = returnCursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
					 //returnCursor.moveToFirst();
					 //filename = returnCursor.getString(nameIndex);
					 //Close stream
					 jEnv->CallObjectMethod(jInputStream, mClose);
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, jInputStream)
					 }* /
					 //Release jName
					 if(jName != NULL && strName != NULL){ jEnv->ReleaseStringUTFChars(jName, strName); }
					 NBJNI_DELETE_REF_LOCAL(jEnv, jName)
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, jContentResolver)
					 }
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsContResolver)
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsInputStream)
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsCursor)
					 * /
					 }
					 }
					 jEnv->ReleaseStringUTFChars(jStrUri, strUri);
					 NBJNI_DELETE_REF_LOCAL(jEnv, jStrUri)
					 }
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsUri)
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
					 }
					 }
					 }
					 NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)*/
				}
			}
			NBString_release(&mainActClass);
		}
	}
	PRINTF_INFO("AUAppGlueAndroidTools::analyzeIntent, consumed(%s).\n", r ? "YES" : "NO");
	return r;
}

/*final String appPackageName = this.getPackageName();
 try {
	startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=" + appPackageName)));
 } catch (android.content.ActivityNotFoundException anfe) {
	startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("http://play.google.com/store/apps/details?id=" + appPackageName)));
 }*/

bool AUAppGlueAndroidTools::openUrl(void* pEnv /*JNIEnv*/, void* pActivity /*jobject*/, const char* url){
	bool r = false;
	JNIEnv* jEnv		= (JNIEnv*)pEnv;
	jobject jActivity	= (jobject)pActivity;
	if(jEnv != NULL && jActivity != NULL && url != NULL){
		jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
		jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
		jclass clsUri		= jEnv->FindClass("android/net/Uri"); NBASSERT(clsUri != NULL)
		if(clsActivity != NULL && clsIntent != NULL && clsUri != NULL){
			jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V"); NBASSERT(mIntentInit != NULL)
			jfieldID fActView		= jEnv->GetStaticFieldID(clsIntent, "ACTION_VIEW", "Ljava/lang/String;"); NBASSERT(fActView != NULL)
			jmethodID mParse		= jEnv->GetStaticMethodID(clsUri, "parse", "(Ljava/lang/String;)Landroid/net/Uri;"); NBASSERT(mParse != NULL)
			jmethodID mStartAct		= jEnv->GetMethodID(clsActivity, "startActivity", "(Landroid/content/Intent;)V"); NBASSERT(mStartAct != NULL)
			if(mIntentInit != NULL && fActView != NULL && mParse != NULL && mStartAct != NULL){
				jstring jUrl	= jEnv->NewStringUTF(url);
				jobject uri		= jEnv->CallStaticObjectMethod(clsUri, mParse, jUrl);
				if(jEnv->ExceptionCheck()){
					jEnv->ExceptionClear();
				} else if(uri != NULL){
					jstring actView	= (jstring)jEnv->GetStaticObjectField(clsIntent, fActView);
					jobject intent	= jEnv->NewObject(clsIntent, mIntentInit, actView, uri);
					if(intent != NULL){
						jEnv->CallVoidMethod(jActivity, mStartAct, intent);
						if(jEnv->ExceptionCheck()){
							jEnv->ExceptionClear();
						} else {
							r = true;
						}
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, actView)
					NBJNI_DELETE_REF_LOCAL(jEnv, uri)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, jUrl)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsUri)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
		}
	}
	return r;
}


//Orientation

BOOL AUAppGlueAndroidTools::supportsRotation(void* data){
	return TRUE;
}

ENAppOrientationBit AUAppGlueAndroidTools::getOrientation(void* pData){
	ENAppOrientationBit r = (ENAppOrientationBit)0;
	{
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jActivity	= (jobject)jniGlue->jActivity();
		if(jEnv != NULL && jActivity != NULL){
			jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
			jclass clsWinMngr	= jEnv->FindClass("android/view/WindowManager"); NBASSERT(clsWinMngr != NULL)
			jclass clsDisplay	= jEnv->FindClass("android/view/Display"); NBASSERT(clsDisplay != NULL)
			if(clsActivity != NULL && clsWinMngr != NULL && clsDisplay != NULL){
				jmethodID mGetWinMngr	= jEnv->GetMethodID(clsActivity, "getWindowManager", "()Landroid/view/WindowManager;"); NBASSERT(mGetWinMngr != NULL)
				jmethodID mGetDisplay	= jEnv->GetMethodID(clsWinMngr, "getDefaultDisplay", "()Landroid/view/Display;"); NBASSERT(mGetDisplay != NULL)
				jmethodID mGetRotation	= jEnv->GetMethodID(clsDisplay, "getRotation", "()I"); NBASSERT(mGetRotation != NULL)
				if(mGetWinMngr != NULL && mGetDisplay != NULL && mGetRotation != NULL){
					jobject winMngr = jEnv->CallObjectMethod(jActivity, mGetWinMngr);
					if(winMngr != NULL){
						jobject display = jEnv->CallObjectMethod(winMngr, mGetDisplay);
						if(display != NULL){
							const jint rot = jEnv->CallIntMethod(display, mGetRotation);
							switch (rot) {
								case 0: //ROTATION_0
									r = ENAppOrientationBit_Portrait;
									break;
								case 1: //ROTATION_90
									r = ENAppOrientationBit_LandscapeLeftBtn;
									break;
								case 2: //ROTATION_180
									r = ENAppOrientationBit_PortraitInverted;
									break;
								case 3: //ROTATION_270
									r = ENAppOrientationBit_LandscapeRightBtn;
									break;
								default:
									PRINTF_WARNING("AUAppGlueAndroidTools, getDefaultDisplay().getRotation() retuned unexpeted %d value.\n", rot);
									break;
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, display)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, winMngr)
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsDisplay)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsWinMngr)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
			}
		}
	}
	return r;
}

/*void AUAppGlueAndroidTools::setOrientation(void* data, const ENAppOrientationBit orientation){
	//
}*/

typedef struct STSetOrientParam_ {
	AUAppGlueAndroidToolsData* data;
	UI32 orientMask;
} STSetOrientParam;

void AUAppGlueAndroidTools_setOrientationsMask_(void* pParam){
	STSetOrientParam* param = (STSetOrientParam*)pParam;
	if(param != NULL){
		AUAppGlueAndroidToolsData* data = param->data;
		const UI32 pOrientMask = param->orientMask;
		if(data != NULL){
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			jobject jActivity	= (jobject)jniGlue->jActivity();
			PRINTF_INFO("AUAppGlueAndroidTools::setOrientationsMask.\n");
			if(jEnv != NULL && jActivity != NULL){
				jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
				if(clsActivity != NULL){
					jmethodID mSetReqOrient = jEnv->GetMethodID(clsActivity, "setRequestedOrientation", "(I)V"); NBASSERT(mSetReqOrient != NULL)
					if(mSetReqOrient != NULL){
						const UI32 orientMask		= (pOrientMask & ENAppOrientationBits_All);
						jint orientMaskAndroid		= -1;
						const char* orientMaskAndroidName = "SCREEN_ORIENTATION_UNSPECIFIED";
						if(orientMask == 0){
							orientMaskAndroid		= -1; //SCREEN_ORIENTATION_UNSPECIFIED (API 1)
							orientMaskAndroidName	= "SCREEN_ORIENTATION_UNSPECIFIED";
						} else if(orientMask == ENAppOrientationBits_All){
							//orientMaskAndroid		= 2; //SCREEN_ORIENTATION_USER (API 1)
							//orientMaskAndroidName	= "SCREEN_ORIENTATION_USER";
							orientMaskAndroid		= 10; //SCREEN_ORIENTATION_FULL_SENSOR (API 9)
							orientMaskAndroidName	= "SCREEN_ORIENTATION_FULL_SENSOR";
							//orientMaskAndroid		= 13; //SCREEN_ORIENTATION_FULL_USER (API 18)
							//orientMaskAndroidName	= "SCREEN_ORIENTATION_FULL_USER";
						} else {
							orientMaskAndroid = 0;
							//Landscape
							if((orientMask & (ENAppOrientationBit_LandscapeLeftBtn | ENAppOrientationBit_LandscapeRightBtn)) == (ENAppOrientationBit_LandscapeLeftBtn | ENAppOrientationBit_LandscapeRightBtn)){
								//orientMaskAndroid			= 11; //SCREEN_ORIENTATION_USER_LANDSCAPE (API 18)
								//orientMaskAndroidName		= "SCREEN_ORIENTATION_USER_LANDSCAPE";
								orientMaskAndroid			= 6; //SCREEN_ORIENTATION_SENSOR_LANDSCAPE (API 9)
								orientMaskAndroidName		= "SCREEN_ORIENTATION_SENSOR_LANDSCAPE";
							} else {
								if((orientMask & ENAppOrientationBit_LandscapeLeftBtn) != 0){
									orientMaskAndroid		= 8; //SCREEN_ORIENTATION_REVERSE_LANDSCAPE (API 9)
									orientMaskAndroidName	= "SCREEN_ORIENTATION_REVERSE_LANDSCAPE";
								}
								if((orientMask & ENAppOrientationBit_LandscapeRightBtn) != 0){
									orientMaskAndroid		= 0; //SCREEN_ORIENTATION_LANDSCAPE (API 1)
									orientMaskAndroidName	= "SCREEN_ORIENTATION_LANDSCAPE";
								}
							}
							//Portrait
							if((orientMask & (ENAppOrientationBit_Portrait | ENAppOrientationBit_PortraitInverted)) == (ENAppOrientationBit_Portrait | ENAppOrientationBit_PortraitInverted)){
								//orientMaskAndroid			= 12; //SCREEN_ORIENTATION_USER_PORTRAIT (API 18)
								//orientMaskAndroidName		= "SCREEN_ORIENTATION_USER_PORTRAIT";
								orientMaskAndroid			= 7; //SCREEN_ORIENTATION_SENSOR_PORTRAIT (API 9)
								orientMaskAndroidName		= "SCREEN_ORIENTATION_SENSOR_PORTRAIT";
							} else {
								if((orientMask & ENAppOrientationBit_Portrait) != 0){
									orientMaskAndroid		= 1; //SCREEN_ORIENTATION_PORTRAIT (API 1)
									orientMaskAndroidName	= "SCREEN_ORIENTATION_PORTRAIT";
								}
								if((orientMask & ENAppOrientationBit_PortraitInverted) != 0){
									orientMaskAndroid		= 8; //SCREEN_ORIENTATION_REVERSE_LANDSCAPE (API 9)
									orientMaskAndroidName	= "SCREEN_ORIENTATION_REVERSE_LANDSCAPE";
								}
							}
						}
						//Call
						jEnv->CallVoidMethod(jActivity, mSetReqOrient, orientMaskAndroid);
						PRINTF_INFO("AUAppGlueAndroidTools, setRequestedOrientation(%d, %s).\n", orientMaskAndroid, orientMaskAndroidName);
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
			}
		}
		//Release
		NBMemory_free(param);
		param = NULL;
	}
}
	
BOOL AUAppGlueAndroidTools::setOrientationsMask(void* pData, const UI32 pOrientMask){
	BOOL r = FALSE;
	AUAppGlueAndroidToolsData* glue = (AUAppGlueAndroidToolsData*)pData;
	AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
	{
		STSetOrientParam* param = NBMemory_allocType(STSetOrientParam);
		NBMemory_setZeroSt(*param, STSetOrientParam);
		param->data			= glue;
		param->orientMask	= pOrientMask;
		if(!jniGlue->addRunableForMainThread(glue->app->getAppNative(), AUAppGlueAndroidTools_setOrientationsMask_, param)){
			NBMemory_free(param);
			param = NULL;
		} else {
			r = TRUE;
		}
	}
	return r;
}

//File picker

BOOL AUAppGlueAndroidTools::filePickerStart(void* pData, void* pEnv /*JNIEnv*/, void* pActivity /*jobject*/, const STNBFilePickerData* pickerParams){
	BOOL r = FALSE;
	AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
	if(data != NULL){
		JNIEnv* jEnv		= (JNIEnv*)pEnv;
		jobject jActivity	= (jobject)pActivity;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		PRINTF_INFO("AUAppGlueAndroidTools::filePickerStart.\n");
		if(jEnv != NULL && jActivity != NULL){
			STNBString mainActClass;
			NBString_init(&mainActClass);
			{
				jstring jStrPkgName = (jstring) jniGlue->getPackageName(jEnv, jActivity);
				const char* bundleId = jEnv->GetStringUTFChars(jStrPkgName, 0);
				{
					NBString_concat(&mainActClass, bundleId);
					NBString_concat(&mainActClass, ".MainActivity");
					NBString_replace(&mainActClass, ".", "/");
				}
				jEnv->ReleaseStringUTFChars(jStrPkgName, bundleId);
			}
			{
				jclass clsString	= jEnv->FindClass("java/lang/String"); NBASSERT(clsString != NULL)
				jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
				jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
				jclass clsUri		= jEnv->FindClass("android/net/Uri"); NBASSERT(clsUri != NULL)
				jclass clsMainAct	= jEnv->FindClass(mainActClass.str); NBASSERT(clsMainAct != NULL)
				if(clsActivity != NULL && clsIntent != NULL && clsUri != NULL && clsMainAct != NULL){
					jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mIntentInit != NULL)
					jfieldID fActGetCont	= jEnv->GetStaticFieldID(clsIntent, "ACTION_GET_CONTENT", "Ljava/lang/String;"); NBASSERT(fActGetCont != NULL)
					jfieldID fCatOpen		= jEnv->GetStaticFieldID(clsIntent, "CATEGORY_OPENABLE", "Ljava/lang/String;"); NBASSERT(fCatOpen != NULL)
					jfieldID fExtraMIMES	= jEnv->GetStaticFieldID(clsIntent, "EXTRA_MIME_TYPES", "Ljava/lang/String;"); NBASSERT(fExtraMIMES != NULL)
					jmethodID mSetType		= jEnv->GetMethodID(clsIntent, "setType", "(Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mSetType != NULL)
					jmethodID mAddCat		= jEnv->GetMethodID(clsIntent, "addCategory", "(Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mAddCat != NULL)
					jmethodID mIntPutStr	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mIntPutStr != NULL)
					jmethodID mIntPutStrs	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;[Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mIntPutStrs != NULL)
					jmethodID mCreateChooser = jEnv->GetStaticMethodID(clsIntent, "createChooser", "(Landroid/content/Intent;Ljava/lang/CharSequence;)Landroid/content/Intent;"); NBASSERT(mCreateChooser != NULL)
					jmethodID mStartAct		= jEnv->GetMethodID(clsActivity, "startActivityForResult", "(Landroid/content/Intent;I)V"); NBASSERT(mStartAct != NULL)
					jmethodID mSetPParam	= jEnv->GetMethodID(clsMainAct, "setPickerParams", "(Ljava/lang/String;)V"); NBASSERT(mSetPParam != NULL)
					if(mIntentInit != NULL && fActGetCont != NULL && fCatOpen != NULL && fExtraMIMES != NULL && mAddCat != NULL && mStartAct != NULL && mSetPParam != NULL){
						jstring jType	= jEnv->NewStringUTF("*/*");
						jstring jTitle	= jEnv->NewStringUTF("Select");
						jstring jExtraM	= (jstring)jEnv->GetStaticObjectField(clsIntent, fExtraMIMES);
						jstring actGCont = (jstring)jEnv->GetStaticObjectField(clsIntent, fActGetCont);
						jstring catOpen	= (jstring)jEnv->GetStaticObjectField(clsIntent, fCatOpen);
						jobject intent	= jEnv->NewObject(clsIntent, mIntentInit, actGCont);
						if(intent != NULL && jType != NULL){
							STNBString strParams;
							NBString_init(&strParams);
							//Convert params to json
							{
								NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &data->filePickerParams, sizeof(data->filePickerParams));
								if(pickerParams != NULL){
									NBStruct_stConcatAsJson(&strParams, NBFilePickerData_getSharedStructMap(), pickerParams, sizeof(*pickerParams));
									NBStruct_stClone(NBFilePickerData_getSharedStructMap(), pickerParams, sizeof(*pickerParams), &data->filePickerParams, sizeof(data->filePickerParams));
								}
							}
							{
								jEnv->CallObjectMethod(intent, mSetType, jType);
								jEnv->CallObjectMethod(intent, mAddCat, catOpen);
								//Extra
								if(strParams.length > 0){
									jstring jName	= jEnv->NewStringUTF(NB_FILE_SEL_INTENT_PARAMS_NAME);
									jstring jValue	= jEnv->NewStringUTF(strParams.str);
									jEnv->CallObjectMethod(intent, mIntPutStr, jName, jValue);
									NBJNI_DELETE_REF_LOCAL(jEnv, jValue)
									NBJNI_DELETE_REF_LOCAL(jEnv, jName)
								}
								//Extra
								if(pickerParams != NULL){
									//Mime types
									if(pickerParams->mimeTypes != NULL && pickerParams->mimeTypesSz > 0){
										jstring jStrEmpty	= jEnv->NewStringUTF("");
										jobjectArray jMimes	= (jobjectArray)jEnv->NewObjectArray(pickerParams->mimeTypesSz, clsString, jStrEmpty);
										//Mimetypes to filter
										{
											SI32 i; for(i = 0; i < pickerParams->mimeTypesSz; i++){
												jstring jMime = jEnv->NewStringUTF(pickerParams->mimeTypes[i]);
												jEnv->SetObjectArrayElement(jMimes, i, jMime);
												NBJNI_DELETE_REF_LOCAL(jEnv, jMime)
											}
											jEnv->CallObjectMethod(intent, mIntPutStrs, jExtraM, jMimes);
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, jMimes)
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrEmpty)
									}
								}
								//Launch intent
								{
									jobject intent2	= jEnv->CallStaticObjectMethod(clsIntent, mCreateChooser, intent, jTitle);
									//Extra, params
									if(strParams.length <= 0){
										jEnv->CallVoidMethod(jActivity, mSetPParam, NULL);
									} else {
										jstring jName	= jEnv->NewStringUTF(NB_FILE_SEL_INTENT_PARAMS_NAME);
										jstring jValue	= jEnv->NewStringUTF(strParams.str);
										jEnv->CallVoidMethod(jActivity, mSetPParam, jValue);
										jEnv->CallObjectMethod(intent2, mIntPutStr, jName, jValue);
										NBJNI_DELETE_REF_LOCAL(jEnv, jValue)
										NBJNI_DELETE_REF_LOCAL(jEnv, jName)
									}
									{
										jEnv->CallVoidMethod(jActivity, mStartAct, intent2, (jint)NB_FILE_SEL_INTENT_CODE);
										if(jEnv->ExceptionCheck()){
											jEnv->ExceptionClear();
										} else {
											r = TRUE;
										}
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, intent2)
								}
							}
							NBString_release(&strParams);
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, catOpen)
						NBJNI_DELETE_REF_LOCAL(jEnv, actGCont)
						NBJNI_DELETE_REF_LOCAL(jEnv, jExtraM)
						NBJNI_DELETE_REF_LOCAL(jEnv, jTitle)
						NBJNI_DELETE_REF_LOCAL(jEnv, jType)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsUri)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsString)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsMainAct)
			}
			NBString_release(&mainActClass);
		}
	}
	return r;
}

//Photo picker

BOOL AUAppGlueAndroidTools::imagePickerIsAvailable(void* pData, const ENPhotoSource src){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		{
			switch(src) {
				case ENPhotoSource_Library:
					r = TRUE;
					break;
				case ENPhotoSource_Camera:
					{
						jclass clsCamera = jEnv->FindClass("android/hardware/Camera");
						if(clsCamera == NULL){
							if(jEnv->ExceptionCheck()) jEnv->ExceptionClear();
						} else {
							jmethodID mGetCount = jEnv->GetStaticMethodID(clsCamera, "getNumberOfCameras", "()I"); NBASSERT(mGetCount != NULL)
							jint numCams = jEnv->CallStaticIntMethod(clsCamera, mGetCount);
							if(numCams > 0){
								r = TRUE;
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsCamera)
					}
					break;
				default:
					PRINTF_WARNING("AUAppGlueAndroidTools, imagePickerIsAvailable, unsupported ENPhotoSource(%d).\n", src);
					break;
			}
		}
	}
	return r;
}

BOOL AUAppGlueAndroidTools::imagePickerStart(void* pData, const ENPhotoSource src, const STNBImagePickerData* pickerParams){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jActivity	= (jobject)jniGlue->jActivity();
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
						r = AUAppGlueAndroidTools::filePickerStart(pData, jEnv, jActivity, &pp);
					}
					NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &pp, sizeof(pp));
				}
				break;
			case ENPhotoSource_Camera:
				{
					//
					//Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
					//if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
					//	startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
					//}
					//
					jclass clsContext	= jEnv->FindClass("android/content/Context");
					jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
					jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
					if(clsContext != NULL){
						jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mIntentInit != NULL)
						jmethodID mResolvAct	= jEnv->GetMethodID(clsIntent, "resolveActivity", "(Landroid/content/pm/PackageManager;)Landroid/content/ComponentName;"); NBASSERT(mResolvAct != NULL)
						jmethodID mGetPkgMngr	= jEnv->GetMethodID(clsContext, "getPackageManager", "()Landroid/content/pm/PackageManager;"); NBASSERT(mGetPkgMngr != NULL)
						jmethodID mStartAct		= jEnv->GetMethodID(clsActivity, "startActivityForResult", "(Landroid/content/Intent;I)V"); NBASSERT(mStartAct != NULL)
						jobject jPackMngr		= jEnv->CallObjectMethod(jActivity, mGetPkgMngr);
						if(jPackMngr != NULL){
							//MediaStore.ACTION_IMAGE_CAPTURE = "android.media.action.IMAGE_CAPTURE"
							jstring jImgCap	= jEnv->NewStringUTF("android.media.action.IMAGE_CAPTURE");
							jobject jIntent = jEnv->NewObject(clsIntent, mIntentInit, jImgCap);
							if(jIntent != NULL){
								jobject jCompName = jEnv->CallObjectMethod(jIntent, mResolvAct, jPackMngr);
								if(jCompName != NULL){
									jEnv->CallVoidMethod(jActivity, mStartAct, jIntent, (jint)NB_FILE_CAM_INTENT_CODE);
									if(jEnv->ExceptionCheck()){
										jEnv->ExceptionClear();
									} else {
										r = TRUE;
									}
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jImgCap)
							NBJNI_DELETE_REF_LOCAL(jEnv, jIntent)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jPackMngr)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
					NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
				}
				break;
			default:
				PRINTF_WARNING("AUAppGlueAndroidTools, imagePickerIsAvailable, unsupported ENPhotoSource(%d).\n", src);
				break;
		}
	}
	return r;
}

//Wallpaper

BOOL AUAppGlueAndroidTools::wallpaperCanBeSet(void* pData){
	BOOL r = FALSE;
	if(pData != NULL){
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv			= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(AUAppGlueAndroidJNI::getAPICurrent(jEnv) >= 24){
			jobject jActivity	= (jobject)jniGlue->jActivity();
			//
			jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
			jclass clsWallMngr	= jEnv->FindClass("android/app/WallpaperManager"); NBASSERT(clsWallMngr != NULL)
			if(clsContext != NULL && clsWallMngr != NULL){
				jmethodID mGetAppContext	= jEnv->GetMethodID(clsContext, "getApplicationContext", "()Landroid/content/Context;"); NBASSERT(mGetAppContext != NULL)
				jmethodID mGetInstance		= jEnv->GetStaticMethodID(clsWallMngr, "getInstance", "(Landroid/content/Context;)Landroid/app/WallpaperManager;"); NBASSERT(mGetInstance != NULL)
				jmethodID mIsSetAllowed		= jEnv->GetMethodID(clsWallMngr, "isSetWallpaperAllowed", "()Z"); //API 24
				jmethodID mIsWallSupported	= jEnv->GetMethodID(clsWallMngr, "isWallpaperSupported", "()Z"); //API 23
				if(mIsSetAllowed != NULL && mIsWallSupported != NULL){
					jobject jAppContext			= jEnv->CallObjectMethod(jActivity, mGetAppContext);
					if(jAppContext != NULL){
						jobject jInstance		= jEnv->CallStaticObjectMethod(clsWallMngr, mGetInstance, jAppContext);
						if(jInstance != NULL){
							jboolean isAlowed	= jEnv->CallBooleanMethod(jInstance, mIsWallSupported);
							if(isAlowed){
								jboolean canBeSet	= jEnv->CallBooleanMethod(jInstance, mIsSetAllowed);
								if(canBeSet){
									PRINTF_INFO("wallpaperCanBeSet is TRUE.\n");
									r = TRUE;
								}
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jInstance)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jAppContext)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsWallMngr)
		}
	}
	return r;
}

BOOL AUAppGlueAndroidTools::wallpaperSet(void* pData, const char* filepath, const UI32 wallsMask /*ENWallpaperBit*/){
	BOOL r = FALSE;
	if(pData != NULL && (wallsMask & (ENWallpaperBit_System | ENWallpaperBit_Lock)) != 0){
		AUAppGlueAndroidToolsData* data = (AUAppGlueAndroidToolsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv			= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(AUAppGlueAndroidJNI::getAPICurrent(jEnv) >= 24){
			jobject jActivity	= (jobject)jniGlue->jActivity();
			//
			jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
			jclass clsWallMngr	= jEnv->FindClass("android/app/WallpaperManager"); NBASSERT(clsWallMngr != NULL)
			jclass clsBmpFact	= jEnv->FindClass("android/graphics/BitmapFactory"); NBASSERT(clsBmpFact != NULL)
			if(clsContext != NULL && clsWallMngr != NULL && clsBmpFact != NULL){
				jmethodID mGetAppContext	= jEnv->GetMethodID(clsContext, "getApplicationContext", "()Landroid/content/Context;"); NBASSERT(mGetAppContext != NULL)
				jmethodID mGetInstance		= jEnv->GetStaticMethodID(clsWallMngr, "getInstance", "(Landroid/content/Context;)Landroid/app/WallpaperManager;"); NBASSERT(mGetInstance != NULL)
				jmethodID mIsSetAllowed		= jEnv->GetMethodID(clsWallMngr, "isSetWallpaperAllowed", "()Z"); //API 24
				jmethodID mIsWallSupported	= jEnv->GetMethodID(clsWallMngr, "isWallpaperSupported", "()Z"); //API 23
				if(mIsSetAllowed != NULL && mIsWallSupported != NULL){
					jmethodID mSetBitmap		= jEnv->GetMethodID(clsWallMngr, "setBitmap", "(Landroid/graphics/Bitmap;Landroid/graphics/Rect;ZI)I"); NBASSERT(mSetBitmap != NULL)
					jmethodID mDecodeFile		= jEnv->GetStaticMethodID(clsBmpFact, "decodeFile", "(Ljava/lang/String;)Landroid/graphics/Bitmap;"); NBASSERT(mDecodeFile != NULL)
					jobject jAppContext			= jEnv->CallObjectMethod(jActivity, mGetAppContext);
					if(jAppContext != NULL){
						jobject jInstance		= jEnv->CallStaticObjectMethod(clsWallMngr, mGetInstance, jAppContext);
						if(jInstance != NULL){
							jboolean isAlowed	= jEnv->CallBooleanMethod(jInstance, mIsWallSupported);
							if(isAlowed){
								jboolean canBeSet	= jEnv->CallBooleanMethod(jInstance, mIsSetAllowed);
								if(canBeSet){
									jstring jPath	= jEnv->NewStringUTF(filepath);
									jobject jBmp	= jEnv->CallStaticObjectMethod(clsBmpFact, mDecodeFile, jPath);
									if(jBmp != NULL){
										jint rr = jEnv->CallIntMethod(jInstance, mSetBitmap, jBmp, NULL, JNI_TRUE, (jint)(((wallsMask & ENWallpaperBit_System) != 0 ? 0x00000001 /*FLAG_SYSTEM*/: 0) | ((wallsMask & ENWallpaperBit_Lock) != 0 ? 0x00000002 /*FLAG_LOCK*/: 0)));
										r = TRUE;
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jPath)
									NBJNI_DELETE_REF_LOCAL(jEnv, jBmp)
								}
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jInstance)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jAppContext)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsWallMngr)
		}
	}
	return r;
}
