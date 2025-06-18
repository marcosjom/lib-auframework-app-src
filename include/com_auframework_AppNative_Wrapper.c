#include "com_auframework_AppNative.h"

/*
 This file's purpose is to be compiled at your top library;
 to avoid these methods (and their dependencies) get stripped
 out from the final binary object.
 
 Add once:
 
 #include "com_auframework_AppNative_Wrapper.c"
 
 at any source file (.c, .cpp, ...) of your library,
 or add this file to your to-be-compiled list (like LOCAL_SRC_FILES).
*/

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_setView(JNIEnv *pEnv, jobject pObj, jobject pView){
	return Java_com_auframework_AppNative_setView(pEnv, pObj, pView);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_onCreate(JNIEnv *pEnv, jobject pObj, jobject savedInstanceState /*Bundle*/){
	return Java_com_auframework_AppNative_onCreate(pEnv, pObj, savedInstanceState);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_onDestroy(JNIEnv *pEnv, jobject pObj){
	Java_com_auframework_AppNative_onDestroy(pEnv, pObj);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_onStart(JNIEnv *pEnv, jobject pObj){
	Java_com_auframework_AppNative_onStart(pEnv, pObj);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_onStop(JNIEnv *pEnv, jobject pObj){
	Java_com_auframework_AppNative_onStop(pEnv, pObj);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_onResume(JNIEnv *pEnv, jobject pObj){
	Java_com_auframework_AppNative_onResume(pEnv, pObj);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_onPause(JNIEnv *pEnv, jobject pObj){
	Java_com_auframework_AppNative_onPause(pEnv, pObj);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_onNewIntent(JNIEnv *pEnv, jobject pObj, jobject pIntent){
	return Java_com_auframework_AppNative_onNewIntent(pEnv, pObj, pIntent);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_onActivityResult(JNIEnv *pEnv, jobject pObj, jint reqCode, jint resp, jobject pIntent){
	return Java_com_auframework_AppNative_onActivityResult(pEnv, pObj, reqCode, resp, pIntent);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_onRequestPermissionsResult(JNIEnv *pEnv, jobject pObj, jint reqCode, jobjectArray perms, jintArray grantResults){
	Java_com_auframework_AppNative_onRequestPermissionsResult(pEnv, pObj, reqCode, perms, grantResults);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_openFilePath(JNIEnv *pEnv, jobject pObj, jobject filepath, jobject params){
	return Java_com_auframework_AppNative_openFilePath(pEnv, pObj, filepath, params);
}

//---------------
//- ServiceConnectionMonitor
//---------------

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceConnected(JNIEnv *pEnv, jobject pObj, jobject compName, jobject service){
	Java_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceConnected(pEnv, pObj, compName, service);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceDisconnected(JNIEnv *pEnv, jobject pObj, jobject compName){
	Java_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceDisconnected(pEnv, pObj, compName);
}

//---------------
//- AlarmReceiver
//---------------

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024AlarmReceiver_onReceive(JNIEnv *pEnv, jobject pObj, jobject pContext, jobject pIntent){
	Java_com_auframework_AppNative_00024AlarmReceiver_onReceive(pEnv, pObj, pContext, pIntent);
}

//---------------
//- AURunable
//---------------

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AURunnable_runNative(JNIEnv *pEnv, jobject pObj, jlong func, jlong funcParam){
	Java_com_auframework_AppNative_00024AURunnable_runNative(pEnv, pObj, func, funcParam);
}
	
//---------------
//- AUSurfaceView
//---------------

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onSurfaceCreated(JNIEnv *pEnv, jobject pObj, jobject pGl, jobject pConfig){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onSurfaceCreated(pEnv, pObj, pGl, pConfig);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onSurfaceChanged(JNIEnv *pEnv, jobject pObj, jobject pGl, jint pWidth, jint pHeight){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onSurfaceChanged(pEnv, pObj, pGl, pWidth, pHeight);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onDrawFrame(JNIEnv *pEnv, jobject pObj, jobject pGl){
	Java_com_auframework_AppNative_00024AUSurfaceView_onDrawFrame(pEnv, pObj, pGl);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUSurfaceView_doFrameNative(JNIEnv *pEnv, jobject pObj, jlong frameTimeNanos){
	Java_com_auframework_AppNative_00024AUSurfaceView_doFrameNative(pEnv, pObj, frameTimeNanos);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onTouchEvent(JNIEnv *pEnv, jobject pObj, jobject pEvent){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onTouchEvent(pEnv, pObj, pEvent);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onCheckIsTextEditor(JNIEnv *pEnv, jobject pObj){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onCheckIsTextEditor(pEnv, pObj);
}

JNIEXPORT jobject /*InputConnection*/ JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onCreateInputConnectionWithClass(JNIEnv *pEnv, jobject pObj, jobject outAttrs /*EditorInfo*/, jobject objClass /*Class*/){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onCreateInputConnectionWithClass(pEnv, pObj, outAttrs, objClass);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onKeyDown(JNIEnv *pEnv, jobject pObj, jint pKeyCode, jobject pKeyEvent){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onKeyDown(pEnv, pObj, pKeyCode, pKeyEvent);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onKeyUp(JNIEnv *pEnv, jobject pObj, jint pKeyCode, jobject pKeyEvent){
	return Java_com_auframework_AppNative_00024AUSurfaceView_onKeyUp(pEnv, pObj, pKeyCode, pKeyEvent);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onWindowFocusChanged(JNIEnv *pEnv, jobject pObj, jboolean hasFocus){
	Java_com_auframework_AppNative_00024AUSurfaceView_onWindowFocusChanged(pEnv, pObj, hasFocus);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_onFocusChange(JNIEnv *pEnv, jobject pObj, jobject pView, jboolean hasFocus){
	Java_com_auframework_AppNative_00024AUSurfaceView_onFocusChange(pEnv, pObj, pView, hasFocus);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024AUSurfaceView_setKeyboardHeight(JNIEnv *pEnv, jobject pObj, jfloat height, jboolean overlapsContent){
	Java_com_auframework_AppNative_00024AUSurfaceView_setKeyboardHeight(pEnv, pObj, height, overlapsContent);
}

//---------------
//- CameraDevice.StateCallback
//---------------
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CameraStateListener__1onClosed(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	Java_com_auframework_AppNative_00024CameraStateListener__1onClosed(pEnv, pObj, jCamera, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CameraStateListener__1onDisconnected(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	Java_com_auframework_AppNative_00024CameraStateListener__1onDisconnected(pEnv, pObj, jCamera, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CameraStateListener__1onError(JNIEnv *pEnv, jobject pObj, jobject jCamera, jint error, jlong dataPtr){
	Java_com_auframework_AppNative_00024CameraStateListener__1onError(pEnv, pObj, jCamera, error, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CameraStateListener__1onOpened(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	Java_com_auframework_AppNative_00024CameraStateListener__1onOpened(pEnv, pObj, jCamera, dataPtr);
}
//------------------------
//- CameraCaptureSession.StateCallback (API 21)
//------------------------
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onActive(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onActive(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onCaptureQueueEmpty(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onCaptureQueueEmpty(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onClosed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onClosed(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onConfigureFailed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onConfigureFailed(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onConfigured(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onConfigured(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onReady(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onReady(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024CaptureStateListener__1onSurfacePrepared(JNIEnv *pEnv, jobject pObj, jobject jSession, jobject jSurface, jlong dataPtr){
	Java_com_auframework_AppNative_00024CaptureStateListener__1onSurfacePrepared(pEnv, pObj, jSession, jSurface, dataPtr);
}
//------------------------
//- ImageReader.OnImageAvailableListener (API 19+)
//------------------------
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024ImageReaderListener__1onImageAvailable(JNIEnv *pEnv, jobject pObj, jobject jReader, jlong dataPtr){
	Java_com_auframework_AppNative_00024ImageReaderListener__1onImageAvailable(pEnv, pObj, jReader, dataPtr);
}
//------------------------
//- FingerprintManager.AuthenticationCallback (API 23-28)
//------------------------

JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationError(JNIEnv *pEnv, jobject pObj, jint errorCode, jobject errString, jlong dataPtr){
	Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationError(pEnv, pObj, errorCode, errString, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationFailed(JNIEnv *pEnv, jobject pObj, jlong dataPtr){
	Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationFailed(pEnv, pObj, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationHelp(JNIEnv *pEnv, jobject pObj, jint helpCode, jobject errString, jlong dataPtr){
	Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationHelp(pEnv, pObj, helpCode, errString, dataPtr);
}
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationSucceeded(JNIEnv *pEnv, jobject pObj, jobject result, jlong dataPtr){
	Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationSucceeded(pEnv, pObj, result, dataPtr);
}

//------------------------
// Billing library listener
//------------------------

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingSetupFinished(JNIEnv *pEnv, jobject pObj, jobject billingResult, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingSetupFinished(pEnv, pObj, billingResult, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingServiceDisconnected(JNIEnv *pEnv, jobject pObj, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingServiceDisconnected(pEnv, pObj, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onSkuDetailsResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject skuDetailsList, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onSkuDetailsResponse(pEnv, pObj, billingResult, skuDetailsList, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onQueryPurchasesResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchases, jlong dataPtr){
    Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onQueryPurchasesResponse(pEnv, pObj, billingResult, purchases, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchasesUpdated(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchases, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchasesUpdated(pEnv, pObj, billingResult, purchases, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onAcknowledgePurchaseResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onAcknowledgePurchaseResponse(pEnv, pObj, billingResult, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onConsumeResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchaseToken, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onConsumeResponse(pEnv, pObj, billingResult, purchaseToken, dataPtr);
}

JNIEXPORT void JNICALL Wrapper_Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchaseHistoryResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchaseHistoryRecordList, jlong dataPtr){
	Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchaseHistoryResponse(pEnv, pObj, billingResult, purchaseHistoryRecordList, dataPtr);
}
//---------------
//- AUInputConnection
//---------------

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_beginBatchEdit(JNIEnv *pEnv, jobject pObj){
	return Java_com_auframework_AppNative_00024AUInputConnection_beginBatchEdit(pEnv, pObj);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_endBatchEdit(JNIEnv *pEnv, jobject pObj){
	return Java_com_auframework_AppNative_00024AUInputConnection_endBatchEdit(pEnv, pObj);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_clearMetaKeyStates(JNIEnv *pEnv, jobject pObj, jint states){
	return Java_com_auframework_AppNative_00024AUInputConnection_clearMetaKeyStates(pEnv, pObj, states);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_commitCompletion(JNIEnv *pEnv, jobject pObj, jobject /*CompletionInfo*/ text){
	return Java_com_auframework_AppNative_00024AUInputConnection_commitCompletion(pEnv, pObj, text);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_commitCorrection(JNIEnv *pEnv, jobject pObj, jobject /*CorrectionInfo*/ correctionInfo){
	return Java_com_auframework_AppNative_00024AUInputConnection_commitCorrection(pEnv, pObj, correctionInfo);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_commitText(JNIEnv *pEnv, jobject pObj, jobject /*CharSequence*/ text, jint newCursorPosition){
	return Java_com_auframework_AppNative_00024AUInputConnection_commitText(pEnv, pObj, text, newCursorPosition);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingText(JNIEnv *pEnv, jobject pObj, jint beforeLength, jint afterLength){
	return Java_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingText(pEnv, pObj, beforeLength, afterLength);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_finishComposingText(JNIEnv *pEnv, jobject pObj){
	return Java_com_auframework_AppNative_00024AUInputConnection_finishComposingText(pEnv, pObj);
}

JNIEXPORT jint JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_getCursorCapsMode(JNIEnv *pEnv, jobject pObj, jint reqModes){
	return Java_com_auframework_AppNative_00024AUInputConnection_getCursorCapsMode(pEnv, pObj, reqModes);
}

JNIEXPORT jobject /*ExtractedText*/ JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_getExtractedText(JNIEnv *pEnv, jobject pObj, jobject /*ExtractedTextRequest*/ request, jint flags){
	return Java_com_auframework_AppNative_00024AUInputConnection_getExtractedText(pEnv, pObj, request, flags);
}

JNIEXPORT jobject /*CharSequence*/ JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_getSelectedText(JNIEnv *pEnv, jobject pObj, jint flags){
	return Java_com_auframework_AppNative_00024AUInputConnection_getSelectedText(pEnv, pObj, flags);
}

JNIEXPORT /*CharSequence*/ jobject JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_getTextAfterCursor(JNIEnv *pEnv, jobject pObj, jint length, jint flags){
	return Java_com_auframework_AppNative_00024AUInputConnection_getTextAfterCursor(pEnv, pObj, length, flags);
}

JNIEXPORT jobject /*CharSequence*/ JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_getTextBeforeCursor(JNIEnv *pEnv, jobject pObj, jint length, jint flags){
	return Java_com_auframework_AppNative_00024AUInputConnection_getTextBeforeCursor(pEnv, pObj, length, flags);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_performContextMenuAction(JNIEnv *pEnv, jobject pObj, jint pId){
	return Java_com_auframework_AppNative_00024AUInputConnection_performContextMenuAction(pEnv, pObj, pId);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_performEditorAction(JNIEnv *pEnv, jobject pObj, jint actionCode){
	return Java_com_auframework_AppNative_00024AUInputConnection_performEditorAction(pEnv, pObj, actionCode);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_performPrivateCommand(JNIEnv *pEnv, jobject pObj, jobject /*String*/ action, jobject /*Bundle*/ data){
	return Java_com_auframework_AppNative_00024AUInputConnection_performPrivateCommand(pEnv, pObj, action, data);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_reportFullscreenMode(JNIEnv *pEnv, jobject pObj, jboolean enabled){
	return Java_com_auframework_AppNative_00024AUInputConnection_reportFullscreenMode(pEnv, pObj, enabled);
}

// (API 25+)
JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_requestCursorUpdates(JNIEnv *pEnv, jobject pObj, jint cursorUpdateMode){
	return Java_com_auframework_AppNative_00024AUInputConnection_requestCursorUpdates(pEnv, pObj, cursorUpdateMode);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_sendKeyEvent(JNIEnv *pEnv, jobject pObj, jobject /*KeyEvent*/ event){
	return Java_com_auframework_AppNative_00024AUInputConnection_sendKeyEvent(pEnv, pObj, event);
}

// (API 9+)
JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_setComposingRegion(JNIEnv *pEnv, jobject pObj, jint start, jint end){
	return Java_com_auframework_AppNative_00024AUInputConnection_setComposingRegion(pEnv, pObj, start, end);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_setComposingText(JNIEnv *pEnv, jobject pObj, jobject /*CharSequence*/ text, jint newCursorPosition){
	return Java_com_auframework_AppNative_00024AUInputConnection_setComposingText(pEnv, pObj, text, newCursorPosition);
}

JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_setSelection(JNIEnv *pEnv, jobject pObj, jint start, jint end){
	return Java_com_auframework_AppNative_00024AUInputConnection_setSelection(pEnv, pObj, start, end);
}

// (API 24+)
JNIEXPORT void JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_closeConnection(JNIEnv *pEnv, jobject pObj){
	Java_com_auframework_AppNative_00024AUInputConnection_closeConnection(pEnv, pObj);
}

// (API 25+)
JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_commitContent(JNIEnv *pEnv, jobject pObj, jobject /*InputContentInfo*/ inputContentInfo, jint flags, jobject /*Bundle*/ opts){
	return Java_com_auframework_AppNative_00024AUInputConnection_commitContent(pEnv, pObj, inputContentInfo, flags, opts);
}
// (API 24+)
JNIEXPORT jboolean JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingTextInCodePoints(JNIEnv *pEnv, jobject pObj, jint beforeLength, jint afterLength){
	return Java_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingTextInCodePoints(pEnv, pObj, beforeLength, afterLength);
}

// (API 24+)
JNIEXPORT jobject /*Handler*/ JNICALL Wrapper_com_auframework_AppNative_00024AUInputConnection_getHandler(JNIEnv *pEnv, jobject pObj){
	return Java_com_auframework_AppNative_00024AUInputConnection_getHandler(pEnv, pObj);
}
