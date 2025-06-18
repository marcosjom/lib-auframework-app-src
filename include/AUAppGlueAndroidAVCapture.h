//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAndroidAVCapture_H
#define AUAppGlueAndroidAVCapture_H

#include "AUMngrAVCapture.h"
#include "AUAppGlueAndroidJNI.h"
//Android and JNI headers
#include <jni.h>

class AUAppGlueAndroidAVCapture {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrAVCaptureCalls* obj);
		static bool destroy(void* data);
		//
		static ENAVCaptureAuthStatus captureAuthStatus(void* data, const BOOL requestIfNecesary);
		static bool			captureStart(void* data, const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask); //ENAVCaptureOutBit
		static bool			captureIsRuning(void* data);
		static void			captureStop(void* data);
		//
		static bool			photoTrigger(void* data);
		static void*		photoSampleRetain(void* data, UI64* frameIdFilterAndDst);
		static void			photoSampleRelease(void* data, void* ptr);
		static const void*	photoSamplePixData(void* data, void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended);
		//
		static void*		videoSampleRetain(void* data, UI64* frameIdFilterAndDst);
		static void			videoSampleRelease(void* data, void* ptr);
		static const void*	videoSamplePixData(void* data, void* ptr, STNBBitmapProps* dstProps);
		//
		static BOOL			metaQRCode(void* data, STNBString* dst, const UI64 secsLastValid);
		//
		//---------------
		//- CameraDevice.StateCallback (API 21+)
		//---------------
	 	static void CameraStateListener_onClosed(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr);
	 	static void CameraStateListener_onDisconnected(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr);
	 	static void CameraStateListener_onError(JNIEnv *pEnv, jobject pObj, jobject jCamera, jint error, jlong dataPtr);
	 	static void CameraStateListener_onOpened(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr);
		//------------------------
		//- CameraCaptureSession.StateCallback (API 21+)
		//------------------------
	 	static void CaptureStateListener_onActive(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	 	static void CaptureStateListener_onCaptureQueueEmpty(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	 	static void CaptureStateListener_onClosed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	 	static void CaptureStateListener_onConfigureFailed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	 	static void CaptureStateListener_onConfigured(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	 	static void CaptureStateListener_onReady(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr);
	 	static void CaptureStateListener_onSurfacePrepared(JNIEnv *pEnv, jobject pObj, jobject jSession, jobject jSurface, jlong dataPtr);
		//------------------------
		//- ImageReader.OnImageAvailableListener (API 19+)
		//------------------------
	 	static void ImageReaderListener_onImageAvailable(JNIEnv *pEnv, jobject pObj, jobject jReader, jlong dataPtr);
};

#endif
