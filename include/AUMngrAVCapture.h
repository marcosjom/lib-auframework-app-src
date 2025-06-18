//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrAVCapture_h
#define AUMngrAVCapture_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

//Callbacks
typedef enum ENAVCaptureState_ {
	ENAVCaptureState_Stopped = 0,
	ENAVCaptureState_Stopping,
	ENAVCaptureState_Starting,
	ENAVCaptureState_Running,
	//
	ENAVCaptureState_Count
} ENAVCaptureState;

typedef enum ENAVCaptureOutBit_ {
	ENAVCaptureOutBit_None = 0,
	ENAVCaptureOutBit_QRCode = 1
} ENAVCaptureOutBit;

typedef enum ENAVCaptureAuthStatus_ {
	ENAVCaptureAuthStatus_NotDetermined = 0,
	ENAVCaptureAuthStatus_Authorized,
	ENAVCaptureAuthStatus_Requesting,
	ENAVCaptureAuthStatus_Denied,
	ENAVCaptureAuthStatus_Restricted,
	//
	ENAVCaptureAuthStatus_Count
} ENAVCaptureAuthStatus;

typedef enum ENAVCaptureSize_ {
	ENAVCaptureSize_Low192 = 0,	//192x144
	ENAVCaptureSize_Low352,		//352 x 288
	ENAVCaptureSize_Med640,		//640 x 480
	ENAVCaptureSize_Med1280,	//1280 x 720
	ENAVCaptureSize_High1920,	//1920, 1080
	ENAVCaptureSize_High3840,	//3840 x 2160
	ENAVCaptureSize_High4032,	//4032 x 3024
} ENAVCaptureSize;

typedef enum ENAVCaptureFocusMode_ {
    ENAVCaptureFocusMode_Locked = 0,
    ENAVCaptureFocusMode_AutoFocusOnce,
    ENAVCaptureFocusMode_AutoFocusContinous,
    //
    ENAVCaptureFocusMode_Count
} ENAVCaptureFocusMode;

typedef enum ENAVCaptureFocusRange_ {
    ENAVCaptureFocusRange_Any = 0,
    ENAVCaptureFocusRange_Near,
    ENAVCaptureFocusRange_Far,
    //
    ENAVCaptureFocusRange_Count
} ENAVCaptureFocusRange;

typedef struct STMngrAVCaptureCalls_ STMngrAVCaptureCalls;

typedef bool (*PTRFuncAVCCreate)(AUAppI* app, STMngrAVCaptureCalls* obj);
typedef bool (*PTRFuncAVCDestroy)(void* obj);
//
typedef ENAVCaptureAuthStatus (*PTRFuncAVCAuthStatus)(void* obj, const BOOL requestIfNecesary);
typedef bool (*PTRFuncAVCStart)(void* obj, const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask); //ENAVCaptureOutBit
typedef bool (*PTRFuncAVCIsRuning)(void* obj);
typedef void (*PTRFuncAVCStop)(void* obj);
//
typedef bool (*PTRFuncAVCPhoto)(void* obj);
typedef void* (*PTRFuncAVCPhotoSampleRetain)(void* obj, UI64* frameIdFilterAndDst);
typedef void (*PTRFuncAVCPhotoSampleRelease)(void* obj, void* ptr);
typedef const void* (*PTRFuncAVCPhotoSamplePixData)(void* obj, void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended);
//
typedef void* (*PTRFuncAVCVideoSampleRetain)(void* obj, UI64* frameIdFilterAndDst);
typedef void (*PTRFuncAVCVideoSampleRelease)(void* obj, void* ptr);
typedef const void* (*PTRFuncAVCVideoSamplePixData)(void* obj, void* ptr, STNBBitmapProps* dstProps);
//
typedef BOOL (*PTRFuncAVCMetaQRCode)(void* obj, STNBString* dst, const UI64 secsLastValid);
//focus
typedef BOOL (*PTRFuncAVCIsFocusModeSupported)(void* obj, const ENAVCaptureFocusMode focusMode);
typedef ENAVCaptureFocusMode (*PTRFuncAVCGetFocusMode)(void* obj);
typedef BOOL (*PTRFuncAVCSetFocusMode)(void* obj, const ENAVCaptureFocusMode focusMode);
typedef BOOL (*PTRFuncAVCIsAdjustingFocus)(void* obj);
typedef float (*PTRFuncAVCGetLensPositionRel)(void* obj);
typedef BOOL (*PTRFuncAVCLockLensToPositionRel)(void* obj, const float relPos);


//
typedef struct STMngrAVCaptureCalls_ {
	PTRFuncAVCCreate			funcCreate;
	void*						funcCreateParam;
	PTRFuncAVCDestroy			funcDestroy;
	void*						funcDestroyParam;
	//
	PTRFuncAVCAuthStatus		funcCaptureAuthStatus;
	void*						funcCaptureAuthStatusParam;
	PTRFuncAVCStart				funcCaptureStart;
	void*						funcCaptureStartParam;
	PTRFuncAVCIsRuning			funcCaptureIsRuning;
	void*						funcCaptureIsRuningParam;
	PTRFuncAVCStop				funcCaptureStop;
	void*						funcCaptureStopParam;
	//
	PTRFuncAVCPhoto				funcPhotoTrigger;
	void*						funcPhotoTriggerParam;
	PTRFuncAVCPhotoSampleRetain	funcPhotoSampleRetain;
	void*						funcPhotoSampleRetainParam;
	PTRFuncAVCPhotoSampleRelease funcPhotoSampleRelease;
	void*						funcPhotoSampleReleaseParam;
	PTRFuncAVCPhotoSamplePixData funcPhotoSamplePixData;
	void*						funcPhotoSamplePixDataParam;
	//
	PTRFuncAVCVideoSampleRetain	funcVideoSampleRetain;
	void*						funcVideoSampleRetainParam;
	PTRFuncAVCVideoSampleRelease funcVideoSampleRelease;
	void*						funcVideoSampleReleaseParam;
	PTRFuncAVCVideoSamplePixData funcVideoSamplePixData;
	void*						funcVideoSamplePixDataParam;
	//
	PTRFuncAVCMetaQRCode		funcMetaQRCode;
	void*						funcMetaQRCodeParam;
    //
    PTRFuncAVCIsFocusModeSupported funcIsFocusModeSupported;
    void*                       funcIsFocusModeSupportedParam;
    PTRFuncAVCGetFocusMode      funcGetFocusMode;
    void*                       funcGetFocusModeParam;
    PTRFuncAVCSetFocusMode      funcSetFocusMode;
    void*                       funcSetFocusModeParam;
    PTRFuncAVCIsAdjustingFocus  funcIsAdjustingFocus;
    void*                       funcIsAdjustingFocusParam;
    PTRFuncAVCGetLensPositionRel funcGetLensPositionRel;
    void*                       funcGetLensPositionRelParam;
    PTRFuncAVCLockLensToPositionRel funcLockLensToPositionRel;
    void*                       funcLockLensToPositionRelParam;
} STMngrAVCaptureCalls;

//

class AUMngrAVCapture : public AUObjeto, public NBAnimador {
	public:
		AUMngrAVCapture();
		virtual ~AUMngrAVCapture();
		//
		static bool			isGlued();
		static bool			setGlue(AUAppI* app, PTRFuncAVCCreate initCall);
		//
		ENAVCaptureAuthStatus captureAuthStatus(const BOOL requestIfNecesary);
		bool				captureStart(const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask); //ENAVCaptureOutBit);
		bool				captureIsRuning();
		void				captureStop();
		//
		bool				photoTrigger();
		void*				photoSampleRetain(UI64* frameIdFilterAndDst);
		void				photoSampleRelease(void* ptr);
		const void*			photoSamplePixData(void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended);
		//
		AUTextura*			videoSamplesTexture() const;
		UI64				videoSamplesTextureSeq() const;
		void*				videoSampleRetain(UI64* frameIdFilterAndDst);
		void				videoSampleRelease(void* ptr);
		const void*			videoSamplePixData(void* ptr, STNBBitmapProps* dstProps);
		//Keep last sample
		BOOL				videoSampleLastIsKept();
		void				videoSampleLastSetKeep(const BOOL keepLast);
		STNBBitmapProps		videoSampleLastProps();
		BOOL				videoSampleLastGetRect(STNBBitmap* dst, const STNBPointI pos, const STNBSizeI size);
		//
		BOOL				metaQRCode(STNBString* dst, const UI64 secsLastValid);
        //focus
        BOOL                isFocusModeSupported(const ENAVCaptureFocusMode focusMode);
        ENAVCaptureFocusMode getFocusMode();
        BOOL                setFocusMode(const ENAVCaptureFocusMode focusMode);
        BOOL                isAdjustingFocus();
        float               getLensPositionRel();
        BOOL                lockLensToPositionRel(const float relPos);
		//
		void				tickAnimacion(float secs);
		//
		void				lock();
		void				unlock();
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrAVCaptureCalls _calls;
		BOOL					_addedToAnimators;
		UI64					_videoSamplesTextureSeq;
		AUTextura*				_videoSamplesTexture;
		//Last sample
		struct {
			BOOL				keepCopy;
			STNBBitmap			bmp;
		} _lastSample;
		//
		NBHILO_MUTEX_CLASE		_mutex;
		SI32					_mutexLocksCount;			//Depth of calling ->lock() and unlock().
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		UI32					_dbgAccumRead;
		CICLOS_CPU_TIPO			_dbgCicleStarted;
#		endif
};

#endif
