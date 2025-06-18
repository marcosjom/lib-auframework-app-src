//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrAVCapture_h
#define NBMngrAVCapture_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrAVCapture.h"

class NBMngrAVCapture {
	public:
		static void			init();
		static void			finish();
		static bool			isInited();
		//
		static bool			isGlued();
		static bool			setGlue(AUAppI* app, PTRFuncAVCCreate initCall);
		//
		static ENAVCaptureAuthStatus captureAuthStatus(const BOOL requestIfNecesary);
		static bool			captureStart(const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask); //ENAVCaptureOutBit
		static bool			captureIsRuning();
		static void			captureStop();
		//
		static bool			photoTrigger();
		static void*		photoSampleRetain(UI64* frameIdFilterAndDst);
		static void			photoSampleRelease(void* ptr);
		static const void*	photoSamplePixData(void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended);
		//
		static AUTextura*	videoSamplesTexture();
		static UI64			videoSamplesTextureSeq();
		static void*		videoSampleRetain(UI64* frameIdFilterAndDst);
		static void			videoSampleRelease(void* ptr);
		static const void*	videoSamplePixData(void* ptr, STNBBitmapProps* dstProps);
		//Keep last sample
		static BOOL			videoSampleLastIsKept();
		static void			videoSampleLastSetKeep(const BOOL keepLast);
		static STNBBitmapProps videoSampleLastProps();
		static BOOL			videoSampleLastGetRect(STNBBitmap* dst, const STNBPointI pos, const STNBSizeI size);
		//
		static BOOL			metaQRCode(STNBString* dst, const UI64 secsLastValid);
        //focus
        static BOOL         isFocusModeSupported(const ENAVCaptureFocusMode focusMode);
        static ENAVCaptureFocusMode getFocusMode();
        static BOOL         setFocusMode(const ENAVCaptureFocusMode focusMode);
        static BOOL         isAdjustingFocus();
        static float        getLensPositionRel();
        static BOOL         lockLensToPositionRel(const float relPos);
		//
		static void		lock();
		static void		unlock();
	private:
		static AUMngrAVCapture* _instance;
};

#endif
