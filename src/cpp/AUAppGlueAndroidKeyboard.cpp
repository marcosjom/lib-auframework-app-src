//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueAndroidKeyboard.h"
//Android and JNI headers
#include <jni.h>

//-------------------------
// AUAppGlueAndroidKeyboard
//-------------------------

#ifdef __ANDROID__
	//is android
#endif

typedef struct AUAppGlueAndroidKeybData_ {
	AUAppI*	app;
	SI32 	apiVersion;
} AUAppGlueAndroidKeybData;

//

bool AUAppGlueAndroidKeyboard::create(void* app, STMngrKeyboardCalls* obj){
	AUAppGlueAndroidKeybData* data = (AUAppGlueAndroidKeybData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidKeybData), ENMemoriaTipo_General);
	data->app = (AUAppI*)app;
	data->apiVersion = 0;
	//
	{
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv				= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			data->apiVersion		= jniGlue->getAPICurrent(jEnv);
		}
	}
	//
	obj->funcCreate					= create;
	obj->funcCreateParam			= data;
	obj->funcDestroy				= destroy;
	obj->funcDestroyParam			= data;
	obj->funcRequestFocus			= requestFocus;
	obj->funcRequestFocusParam		= data;
	obj->funcSetVisible				= setVisible;
	obj->funcSetVisibleParam		= data;
	obj->funcRestartKeyboard		= restartKeyboard;
	obj->funcRestartKeyboardParam	= data;
	obj->funcUpdateKeyboardCursor	= updateKeyboardCursor;
	obj->funcUpdateKeyboardCursorParam = data;
	obj->funcRestartInputPre		= NULL;
	obj->funcRestartInputPreParam	= NULL;
	obj->funcRestartInputPost		= NULL;
	obj->funcRestartInputPostParam	= NULL;
	//
	return true;
}

bool AUAppGlueAndroidKeyboard::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidKeybData* data = (AUAppGlueAndroidKeybData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//this.requestFocus();

bool AUAppGlueAndroidKeyboard::requestFocus(void* pEnv /*JNIEnv*/, void* pView /*jobject*/){
	bool r = false;
	PRINTF_INFO("AUAppGlueAndroidKeyboard, requestFocus.\n");
	if(pEnv != NULL && pView != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jView		= (jobject) pView;
		jclass clsView	= jEnv->FindClass("android/view/View"); NBASSERT(clsView != NULL)
		if(clsView != NULL){
			jmethodID mReqFocus	= jEnv->GetMethodID(clsView, "requestFocus", "()Z"); NBASSERT(mReqFocus != NULL)
			if(mReqFocus != NULL){
				r = (jEnv->CallBooleanMethod(jView, mReqFocus) == JNI_TRUE);
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsView)
		}
	}
	return r;
}

/*InputMethodManager imm = (InputMethodManager) _activity.getSystemService(Context.INPUT_METHOD_SERVICE);
if(!imm.showSoftInput(this, 0)){
	//
}
if(!imm.hideSoftInputFromWindow(this.getWindowToken(), 0)){
	//
}*/

bool AUAppGlueAndroidKeyboard::setKeyboardVisible(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, void* pView /*jobject*/, const bool visible){
	bool r = false;
	//PRINTF_INFO("AUAppGlueAndroidKeyboard, setKeyboardVisible(%s).\n", (visible ? "YES" : "NO"));
	if(pEnv != NULL && pContext != NULL && pView != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jobject jView		= (jobject) pView;
		//
		jobject imm	= (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "INPUT_METHOD_SERVICE");
		if(imm != NULL){
			jclass clsIMM	= jEnv->FindClass("android/view/inputmethod/InputMethodManager"); NBASSERT(clsIMM != NULL)
			jclass clsView	= jEnv->FindClass("android/view/View"); NBASSERT(clsView != NULL)
			if(clsIMM != NULL && clsView != NULL){
				jmethodID mShow		= jEnv->GetMethodID(clsIMM, "showSoftInput", "(Landroid/view/View;I)Z"); NBASSERT(mShow != NULL)
				jmethodID mHide		= jEnv->GetMethodID(clsIMM, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z"); NBASSERT(mHide != NULL)
				jmethodID mGetWinTkn = jEnv->GetMethodID(clsView, "getWindowToken", "()Landroid/os/IBinder;"); NBASSERT(mGetWinTkn != NULL)
				if(mShow != NULL && mHide != NULL && mGetWinTkn != NULL){
					if(visible){
						r = (jEnv->CallBooleanMethod(imm, mShow, jView, 0) == JNI_TRUE);
						PRINTF_WARNING("AUAppGlueAndroidKeyboard, showSoftInput.\n");
					} else {
						jobject winToken = jEnv->CallObjectMethod(jView, mGetWinTkn); NBASSERT(winToken != NULL)
						if(winToken != NULL){
							r = (jEnv->CallBooleanMethod(imm, mHide, winToken, 0) == JNI_TRUE);
							NBJNI_DELETE_REF_LOCAL(jEnv, winToken)
							//PRINTF_WARNING("AUAppGlueAndroidKeyboard, hideSoftInputFromWindow.\n");
						}
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsView)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsIMM)
			}
		}
	}
	return r;
}

//Calls

void AUAppGlueAndroidKeyboard_requestFocus_(void* pData){
	AUAppGlueAndroidKeybData* data = (AUAppGlueAndroidKeybData*)pData;
	AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
	JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
	if(jEnv != NULL){
		AUAppGlueAndroidKeyboard::requestFocus(jEnv, data->app->getView());
	}
}

bool AUAppGlueAndroidKeyboard::requestFocus(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidKeybData* glue = (AUAppGlueAndroidKeybData*)pData;
		AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
		if(jniGlue->addRunableForMainThread(glue->app->getAppNative(), AUAppGlueAndroidKeyboard_requestFocus_, pData)){
			r = true;
		}
		/*JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			r = AUAppGlueAndroidKeyboard::requestFocus(jEnv, glue->app->getView());
		}*/
	}
	return r;
}

typedef struct STSetVisibleData_ {
	AUAppGlueAndroidKeybData* data;
	BOOL visible;
} STSetVisibleData;

void AUAppGlueAndroidKeyboard_setVisible_(void* pData){
	STSetVisibleData* params = (STSetVisibleData*)pData;
	if(params != NULL){
		AUAppGlueAndroidJNI* jniGlue = params->data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				AUAppGlueAndroidKeyboard::setKeyboardVisible(jEnv, jContext, params->data->app->getView(), params->visible);
			}
		}
		//Free
		NBMemory_free(params);
		params = NULL;
	}
}
	
bool AUAppGlueAndroidKeyboard::setVisible(void* pData, const bool visible){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidKeybData* glue = (AUAppGlueAndroidKeybData*)pData;
		AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
		STSetVisibleData* params = NBMemory_allocType(STSetVisibleData);
		NBMemory_setZeroSt(*params, STSetVisibleData);
		params->data	= glue;
		params->visible = visible;
		if(!jniGlue->addRunableForMainThread(glue->app->getAppNative(), AUAppGlueAndroidKeyboard_setVisible_, params)){
			NBMemory_free(params);
			params = NULL;
		} else {
			r = true;
		}
		/*JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				r = AUAppGlueAndroidKeyboard::setKeyboardVisible(jEnv, jContext, glue->app->getView(), visible);
			}
		}*/
	}
	return r;
}

//InputMethodManager imm = (InputMethodManager)_activity.getSystemService(Context.INPUT_METHOD_SERVICE);
//imm.updateSelection(_appView, rngSel[0], rngSel[0] + rngSel[1], rngMrc[0], rngMrc[0] + rngMrc[1]);

void AUAppGlueAndroidKeyboard_restartKeyboard_(void* pData){
	if(pData != NULL){
		AUAppGlueAndroidKeybData* glue = (AUAppGlueAndroidKeybData*)pData;
		AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL && glue->app->getView() != NULL){
				jobject imm = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "INPUT_METHOD_SERVICE");
				if(imm != NULL){
					jclass clsIMM	= jEnv->FindClass("android/view/inputmethod/InputMethodManager"); NBASSERT(clsIMM != NULL)
					if(clsIMM != NULL){
						jmethodID mRestart = jEnv->GetMethodID(clsIMM, "restartInput", "(Landroid/view/View;)V"); NBASSERT(mRestart != NULL)
						if(mRestart != NULL){
							jEnv->CallVoidMethod(imm, mRestart, (jobject)glue->app->getView());
						}
						//
						/*jmethodID mIsAct = jEnv->GetMethodID(clsIMM, "isActive", "(Landroid/view/View;)Z"); NBASSERT(mIsAct != NULL)
						if(mIsAct != NULL){
							//
							jboolean isWatchingCursor = JNI_TRUE;
							//if(data->apiVersion < 21){
								//before "InputConnection.html#requestCursorUpdates(int)"
								jmethodID mWatchCur = jEnv->GetMethodID(clsIMM, "isWatchingCursor", "(Landroid/view/View;)Z"); NBASSERT(mWatchCur != NULL)
								if(mWatchCur == NULL){
									if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception 'android/support/v4/app/NotificationCompat$Builder' not found
								} else {
									isWatchingCursor = jEnv->CallBooleanMethod(imm, mWatchCur, (jobject)glue->app->getView());
								}
							//}
							//
							jboolean isActive = jEnv->CallBooleanMethod(imm, mIsAct, (jobject)glue->app->getView());
							if(isWatchingCursor != JNI_TRUE){
								PRINTF_WARNING("AUAppGlueAndroidKeyboard, could not update sel/marked ranges (is not watching cursor).\n");
							} else if(isActive != JNI_TRUE){
								PRINTF_WARNING("AUAppGlueAndroidKeyboard, could not update sel/marked ranges (view is not active).\n");
							} else {
								jmethodID mUpAnc = jEnv->GetMethodID(clsIMM, "updateCursorAnchorInfo", "(Landroid/view/View;Landroid/view/inputmethod/CursorAnchorInfo;)V");
								if(mUpAnc == NULL){
									if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception 'android/support/v4/app/NotificationCompat$Builder' not found
									//API 20-
									jmethodID mUpSel = jEnv->GetMethodID(clsIMM, "updateSelection", "(Landroid/view/View;IIII)V"); NBASSERT(mUpSel != NULL)
									if(mUpSel != NULL){
										PRINTF_INFO("AUAppGlueAndroidKeyboard, updateSelection sel(%d, +%d) mrk(%d, +%d, '%s').\n", selRng.inicio, selRng.tamano, mrkRng.inicio, mrkRng.tamano, mrkRngTxt);
										jEnv->CallVoidMethod(imm, mUpSel, (jobject)glue->app->getView(), (jint)selRng.inicio, (jint)(selRng.inicio + selRng.tamano), (jint)mrkRng.inicio, (jint)(mrkRng.inicio + mrkRng.tamano));
										r = true;
									}
								} else {
									//API 21+
									jclass clsBldr	= jEnv->FindClass("android/view/inputmethod/CursorAnchorInfo$Builder"); NBASSERT(clsBldr != NULL)
									if(clsBldr != NULL){
										jmethodID mBldrInit	= jEnv->GetMethodID(clsBldr, "<init>", "()V"); NBASSERT(mBldrInit != NULL)
										jmethodID mSetSel = jEnv->GetMethodID(clsBldr, "setSelectionRange", "(II)Landroid/view/inputmethod/CursorAnchorInfo$Builder;"); NBASSERT(mSetSel != NULL)
										jmethodID mSetCmp = jEnv->GetMethodID(clsBldr, "setComposingText", "(ILjava/lang/CharSequence;)Landroid/view/inputmethod/CursorAnchorInfo$Builder;"); NBASSERT(mSetCmp != NULL)
										jmethodID mBuild = jEnv->GetMethodID(clsBldr, "build", "()Landroid/view/inputmethod/CursorAnchorInfo;"); NBASSERT(mBuild != NULL)
										if(mBldrInit != NULL && mBuild != NULL && mSetSel != NULL && mSetCmp != NULL){
											jobject builder	= jEnv->NewObject(clsBldr, mBldrInit); NBASSERT(builder != NULL)
											if(builder != NULL){
												jstring jStrMrkTxt = jEnv->NewStringUTF(mrkRngTxt);
												//Selection range
												jEnv->CallObjectMethod(builder, mSetSel, (jint)selRng.inicio, (jint)(selRng.inicio + selRng.tamano));
												//Composing range
												if(mrkRng.tamano > 0) jEnv->CallObjectMethod(builder, mSetCmp, (jint)mrkRng.inicio, jStrMrkTxt);
												//Build
												jobject curAnchr = jEnv->CallObjectMethod(builder, mBuild);
												//
												PRINTF_INFO("AUAppGlueAndroidKeyboard, updateCursorAnchorInfo sel(%d, +%d) mrk(%d, +%d, '%s').\n", selRng.inicio, selRng.tamano, mrkRng.inicio, mrkRng.tamano, mrkRngTxt);
												jEnv->CallVoidMethod(imm, mUpAnc, (jobject)glue->app->getView(), curAnchr);
												//
												NBJNI_DELETE_REF_LOCAL(jEnv, jStrMrkTxt)
												NBJNI_DELETE_REF_LOCAL(jEnv, builder)
												r = true;
											}
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, clsBldr)
									}
								}
							}
						}*/
						NBJNI_DELETE_REF_LOCAL(jEnv, clsIMM)
					}
				}
			}
		}
	}
}

bool AUAppGlueAndroidKeyboard::restartKeyboard(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidKeybData* glue = (AUAppGlueAndroidKeybData*)pData;
		AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
		if(jniGlue->addRunableForMainThread(glue->app->getAppNative(), AUAppGlueAndroidKeyboard_restartKeyboard_, pData)){
			r = TRUE;
		}
	}
	return r;
}

typedef struct STSetUpdateData_ {
	AUAppGlueAndroidKeybData* data;
	SI32	rngSelStart;
	SI32	rngSelSz;
	SI32	rngMrkStart;
	SI32	rngMrkSz;
	char*	rngMrkText;
} STSetUpdateData;

void AUAppGlueAndroidKeyboard_updateKeyboardCursor_(void* pParam){
	STSetUpdateData* param = (STSetUpdateData*)pParam;
	if(param != NULL){
		 AUAppGlueAndroidKeybData* glue = (AUAppGlueAndroidKeybData*)param->data;
		 AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
		 JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		 if(jEnv != NULL){
			 jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			 if(jContext != NULL && glue->app->getView() != NULL){
				 jobject imm = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "INPUT_METHOD_SERVICE");
				 if(imm != NULL){
					 jclass clsIMM	= jEnv->FindClass("android/view/inputmethod/InputMethodManager"); NBASSERT(clsIMM != NULL)
					 if(clsIMM != NULL){
						 /*jmethodID mIsAct = jEnv->GetMethodID(clsIMM, "isActive", "(Landroid/view/View;)Z"); NBASSERT(mIsAct != NULL)
						 if(mIsAct != NULL){*/
							 //
							 //ToDo: enable code when 'requestCursorUpdates' usage is clear.
							 /*jboolean isWatchingCursor = JNI_TRUE;
							 if(glue->apiVersion >= 21){
								 jmethodID mReqCurUp = jEnv->GetMethodID(clsIMM, "requestCursorUpdates", "(I)Z"); NBASSERT(mReqCurUp != NULL)
								 if(mReqCurUp != NULL){
									 //CURSOR_UPDATE_IMMEDIATE = 1;
									 //CURSOR_UPDATE_MONITOR = 2
									 isWatchingCursor = jEnv->CallBooleanMethod(imm, mReqCurUp, 0);
								 }
							 } else {
								 //before "InputConnection.html#requestCursorUpdates(int)"
								 jmethodID mWatchCur = jEnv->GetMethodID(clsIMM, "isWatchingCursor", "(Landroid/view/View;)Z"); NBASSERT(mWatchCur != NULL)
								 if(mWatchCur == NULL){
									 if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception 'android/support/v4/app/NotificationCompat$Builder' not found
								 } else {
									 isWatchingCursor = jEnv->CallBooleanMethod(imm, mWatchCur, (jobject)glue->app->getView());
								 }
							 }*/
							 //
							 /*jboolean isActive = jEnv->CallBooleanMethod(imm, mIsAct, (jobject)glue->app->getView());
							 if(isWatchingCursor != JNI_TRUE){
								 PRINTF_WARNING("AUAppGlueAndroidKeyboard, could not update sel/marked ranges (is not watching cursor).\n");
							 } else if(isActive != JNI_TRUE){
								 PRINTF_WARNING("AUAppGlueAndroidKeyboard, could not update sel/marked ranges (view is not active).\n");
							 } else*/
							 if(glue->apiVersion >= 21){
								 //API 21+
								 jmethodID mUpAnc = jEnv->GetMethodID(clsIMM, "updateCursorAnchorInfo", "(Landroid/view/View;Landroid/view/inputmethod/CursorAnchorInfo;)V"); NBASSERT(mUpAnc != NULL)
								 jclass clsBldr	= jEnv->FindClass("android/view/inputmethod/CursorAnchorInfo$Builder"); NBASSERT(clsBldr != NULL)
								 if(mUpAnc != NULL && clsBldr != NULL){
									 jmethodID mBldrInit	= jEnv->GetMethodID(clsBldr, "<init>", "()V"); NBASSERT(mBldrInit != NULL)
									 jmethodID mSetSel = jEnv->GetMethodID(clsBldr, "setSelectionRange", "(II)Landroid/view/inputmethod/CursorAnchorInfo$Builder;"); NBASSERT(mSetSel != NULL)
									 jmethodID mSetCmp = jEnv->GetMethodID(clsBldr, "setComposingText", "(ILjava/lang/CharSequence;)Landroid/view/inputmethod/CursorAnchorInfo$Builder;"); NBASSERT(mSetCmp != NULL)
									 jmethodID mBuild = jEnv->GetMethodID(clsBldr, "build", "()Landroid/view/inputmethod/CursorAnchorInfo;"); NBASSERT(mBuild != NULL)
									 if(mBldrInit != NULL && mBuild != NULL && mSetSel != NULL && mSetCmp != NULL){
										 jobject builder	= jEnv->NewObject(clsBldr, mBldrInit); NBASSERT(builder != NULL)
										 if(builder != NULL){
											 jstring jStrMrkTxt = jEnv->NewStringUTF(param->rngMrkText);
											 //Selection range
											 jEnv->CallObjectMethod(builder, mSetSel, (jint)param->rngSelStart, (jint)(param->rngSelStart + param->rngSelSz));
											 //Composing range
											 if(param->rngMrkSz > 0) jEnv->CallObjectMethod(builder, mSetCmp, (jint)param->rngMrkStart, jStrMrkTxt);
											 //Build
											 jobject curAnchr = jEnv->CallObjectMethod(builder, mBuild);
											 //
											 PRINTF_INFO("AUAppGlueAndroidKeyboard, updateCursorAnchorInfo sel(%d, +%d) mrk(%d, +%d, '%s').\n", param->rngSelStart, param->rngSelSz, param->rngMrkStart, param->rngMrkSz, param->rngMrkText);
											 jEnv->CallVoidMethod(imm, mUpAnc, (jobject)glue->app->getView(), curAnchr);
											 //
											 NBJNI_DELETE_REF_LOCAL(jEnv, jStrMrkTxt)
											 NBJNI_DELETE_REF_LOCAL(jEnv, builder)
											 //r = true;
										 }
									 }
								 }
								 NBJNI_DELETE_REF_LOCAL(jEnv, clsBldr)
							 } else {
								 //API 20-
								 jmethodID mUpSel = jEnv->GetMethodID(clsIMM, "updateSelection", "(Landroid/view/View;IIII)V"); NBASSERT(mUpSel != NULL)
								 if(mUpSel != NULL){
									 PRINTF_INFO("AUAppGlueAndroidKeyboard, updateSelection sel(%d, +%d) mrk(%d, +%d, '%s').\n", param->rngSelStart, param->rngSelSz, param->rngMrkStart, param->rngMrkSz, param->rngMrkText);
									 jEnv->CallVoidMethod(imm, mUpSel, (jobject)glue->app->getView(), (jint)param->rngSelStart, (jint)(param->rngSelStart + param->rngSelSz), (jint)param->rngMrkStart, (jint)(param->rngMrkStart + param->rngMrkSz));
									 //r = true;
								 }
							 }
						 //}
						 NBJNI_DELETE_REF_LOCAL(jEnv, clsIMM)
					 }
				 }
			 }
		 }
		//Release
		if(param->rngMrkText != NULL) NBMemory_free(param->rngMrkText); param->rngMrkText = NULL;
		NBMemory_free(param);
		param = NULL;
	}
}

bool AUAppGlueAndroidKeyboard::updateKeyboardCursor(void* pData, const SI32 rngSelStart, const SI32 rngSelSz, const SI32 rngMrkStart, const SI32 rngMrkSz, const char* rngMrkText){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidKeybData* glue = (AUAppGlueAndroidKeybData*)pData;
		AUAppGlueAndroidJNI* jniGlue = glue->app->getGlueJNI();
		{
			STSetUpdateData* param = NBMemory_allocType(STSetUpdateData);
			NBMemory_setZeroSt(*param, STSetUpdateData);
			param->data			= glue;
			param->rngSelStart	= rngSelStart;
			param->rngSelSz		= rngSelSz;
			param->rngMrkStart	= rngMrkStart;
			param->rngMrkSz		= rngMrkSz;
			param->rngMrkText	= NULL; if(rngMrkText != NULL) param->rngMrkText = NBString_strNewBuffer(rngMrkText);
			if(!jniGlue->addRunableForMainThread(glue->app->getAppNative(), AUAppGlueAndroidKeyboard_updateKeyboardCursor_, param)){
				if(param->rngMrkText != NULL) NBMemory_free(param->rngMrkText); param->rngMrkText = NULL;
				NBMemory_free(param);
				param = NULL;
			} else {
				r = TRUE;
			}
		}
	}
	return r;
}



