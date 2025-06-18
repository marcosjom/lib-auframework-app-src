//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSAVCapture_H
#define AUAppGlueIOSAVCapture_H

#include "AUAppI.h"
#include "AUMngrAVCapture.h"

class AUAppGlueIOSAVCapture {
	public:
		//Calls
		static bool			create(AUAppI* app, STMngrAVCaptureCalls* obj);
		static bool			destroy(void* data);
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
        //focus
        static BOOL         isFocusModeSupported(void* data, const ENAVCaptureFocusMode focusMode);
        static ENAVCaptureFocusMode getFocusMode(void* data);
        static BOOL         setFocusMode(void* data, const ENAVCaptureFocusMode focusMode);
        static BOOL         isAdjustingFocus(void* data);
        static float        getLensPositionRel(void* data);
        static BOOL         lockLensToPositionRel(void* data, const float relPos);
};

#endif
