
#ifndef COM_AUFRAMEWORK_APPNATIVE_H
#define COM_AUFRAMEWORK_APPNATIVE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_setView(JNIEnv *pEnv, jobject pObj, jobject pView);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_onCreate(JNIEnv *pEnv, jobject pObj, jobject savedInstanceState /*Bundle*/);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_onDestroy(JNIEnv *pEnv, jobject pObj);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_onStart(JNIEnv *pEnv, jobject pObj);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_onStop(JNIEnv *pEnv, jobject pObj);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_onResume(JNIEnv *pEnv, jobject pObj);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_onPause(JNIEnv *pEnv, jobject pObj);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_onNewIntent(JNIEnv *pEnv, jobject pObj, jobject pIntent);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_onActivityResult(JNIEnv *pEnv, jobject pObj, jint reqCode, jint resp, jobject pIntent);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_onRequestPermissionsResult(JNIEnv *pEnv, jobject pObj, jint reqCode, jobjectArray perms, jintArray grantResults);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_openFilePath(JNIEnv *pEnv, jobject pObj, jobject filepath, jobject params);
	//---------------
	//- ServiceConnectionMonitor
	//---------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceConnected(JNIEnv* pEnv, jobject pObj, jobject compName, jobject service);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceDisconnected(JNIEnv* pEnv, jobject pObj, jobject compName);
	//---------------
	//- AlarmReceiver
	//---------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AlarmReceiver_onReceive(JNIEnv *pEnv, jobject pObj, jobject pContext, jobject pIntent);
	//---------------
	//- AURunable
	//---------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AURunnable_runNative(JNIEnv *pEnv, jobject pObj, jlong func, jlong funcParam);
	//---------------
	//- AUSurfaceView
	//---------------
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onSurfaceCreated(JNIEnv *pEnv, jobject pObj, jobject pGl, jobject pConfig);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onSurfaceChanged(JNIEnv *pEnv, jobject pObj, jobject pGl, jint pWidth, jint pHeight);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onDrawFrame(JNIEnv *pEnv, jobject pObj, jobject gl);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_doFrameNative(JNIEnv *pEnv, jobject pObj, jlong frameTimeNanos);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onTouchEvent(JNIEnv* pEnv, jobject pObj, jobject pEvent);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onCheckIsTextEditor(JNIEnv* pEnv, jobject pObj);
	JNIEXPORT jobject /*InputConnection*/ JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onCreateInputConnectionWithClass(JNIEnv * pEnv, jobject pObj, jobject outAttrs /*EditorInfo*/, jobject objClass /*Class*/);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onKeyDown(JNIEnv *pEnv, jobject pObj, jint pKeyCode, jobject pKeyEvent);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onKeyUp(JNIEnv *pEnv, jobject pObj, jint pKeyCode, jobject pKeyEvent);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onWindowFocusChanged(JNIEnv *pEnv, jobject pObj, jboolean hasFocus);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onFocusChange(JNIEnv *pEnv, jobject pObj, jobject pView, jboolean hasFocus);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_setKeyboardHeight(JNIEnv *pEnv, jobject pObj, jfloat height, jboolean overlapsContent);
	//---------------
	//- CameraDevice.StateCallback (API 21+)
	//---------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onClosed(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onDisconnected(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onError(JNIEnv *pEnv, jobject pObj, jobject jCamera, jint error, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onOpened(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr);
	//------------------------
	//- CameraCaptureSession.StateCallback (API 21+)
	//------------------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onActive(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onCaptureQueueEmpty(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onClosed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onConfigureFailed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onConfigured(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onReady(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onSurfacePrepared(JNIEnv *pEnv, jobject pObj, jobject jSession, jobject jSurface, jlong dataPtr);
	//------------------------
	//- ImageReader.OnImageAvailableListener (API 19+)
	//------------------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024ImageReaderListener__1onImageAvailable(JNIEnv *pEnv, jobject pObj, jobject jReader, jlong dataPtr);
	//------------------------
	//- FingerprintManager.AuthenticationCallback (API 23-28)
	//------------------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationError(JNIEnv *pEnv, jobject pObj, jint errorCode, jobject errString, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationFailed(JNIEnv *pEnv, jobject pObj, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationHelp(JNIEnv *pEnv, jobject pObj, jint helpCode, jobject errString, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationSucceeded(JNIEnv *pEnv, jobject pObj, jobject result, jlong dataPtr);
	//------------------------
	// Billing library listener
	//------------------------
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingSetupFinished(JNIEnv *pEnv, jobject pObj, jobject billingResult, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingServiceDisconnected(JNIEnv *pEnv, jobject pObj, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onSkuDetailsResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject skuDetailsList, jlong dataPtr);
    JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onQueryPurchasesResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchases, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchasesUpdated(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchases, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onAcknowledgePurchaseResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onConsumeResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchaseToken, jlong dataPtr);
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchaseHistoryResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchaseHistoryRecordList, jlong dataPtr);
	//---------------
	//- AUInputConnection
	//---------------
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_beginBatchEdit(JNIEnv* pEnv, jobject pObj);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_endBatchEdit(JNIEnv* pEnv, jobject pObj);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_clearMetaKeyStates(JNIEnv* pEnv, jobject pObj, jint states);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitCompletion(JNIEnv* pEnv, jobject pObj, jobject /*CompletionInfo*/ text);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitCorrection(JNIEnv* pEnv, jobject pObj, jobject /*CorrectionInfo*/ correctionInfo);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitText(JNIEnv* pEnv, jobject pObj, jobject /*CharSequence*/ text, jint newCursorPosition);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingText(JNIEnv* pEnv, jobject pObj, jint beforeLength, jint afterLength);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_finishComposingText(JNIEnv* pEnv, jobject pObj);
	JNIEXPORT jint JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getCursorCapsMode(JNIEnv* pEnv, jobject pObj, jint reqModes);
	JNIEXPORT jobject /*ExtractedText*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getExtractedText(JNIEnv* pEnv, jobject pObj, jobject /*ExtractedTextRequest*/ request, jint flags);
	JNIEXPORT jobject /*CharSequence*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getSelectedText(JNIEnv* pEnv, jobject pObj, jint flags);
	JNIEXPORT /*CharSequence*/ jobject JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getTextAfterCursor(JNIEnv* pEnv, jobject pObj, jint length, jint flags);
	JNIEXPORT jobject /*CharSequence*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getTextBeforeCursor(JNIEnv* pEnv, jobject pObj, jint length, jint flags);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_performContextMenuAction(JNIEnv* pEnv, jobject pObj, jint id);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_performEditorAction(JNIEnv* pEnv, jobject pObj, jint actionCode);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_performPrivateCommand(JNIEnv* pEnv, jobject pObj, jobject /*String*/ action, jobject /*Bundle*/ data);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_reportFullscreenMode(JNIEnv* pEnv, jobject pObj, jboolean enabled);
	// (API 25+)
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_requestCursorUpdates(JNIEnv* pEnv, jobject pObj, jint cursorUpdateMode);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_sendKeyEvent(JNIEnv* pEnv, jobject pObj, jobject /*KeyEvent*/ event);
	// (API 9+)
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_setComposingRegion(JNIEnv* pEnv, jobject pObj, jint start, jint end);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_setComposingText(JNIEnv* pEnv, jobject pObj, jobject /*CharSequence*/ text, jint newCursorPosition);
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_setSelection(JNIEnv* pEnv, jobject pObj, jint start, jint end);
	// (API 24+)
	JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUInputConnection_closeConnection(JNIEnv* pEnv, jobject pObj);
	// (API 25+)
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitContent(JNIEnv* pEnv, jobject pObj, jobject /*InputContentInfo*/ inputContentInfo, jint flags, jobject /*Bundle*/ opts);
	// (API 24+)
	JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingTextInCodePoints(JNIEnv* pEnv, jobject pObj, jint beforeLength, jint afterLength);
	// (API 24+)
	JNIEXPORT jobject /*Handler*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getHandler(JNIEnv* pEnv, jobject pObj);

#ifdef __cplusplus
}
#endif

#endif

