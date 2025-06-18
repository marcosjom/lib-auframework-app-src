//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSAVCapture.h"
#include "nb/core/NBMemory.h"
#include "nb/core/NBThread.h"
//
#import <AVFoundation/AVFoundation.h>
//

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

#ifdef NB_CONFIG_INCLUDE_ASSERTS
#	define NB_IOS_AVC_ITM_MUTEX_ACTIVATE(ITM_PTR)	NBASSERT(!ITM_PTR->dbgIsLocked) NBHILO_MUTEX_ACTIVAR(&ITM_PTR->mutex) ITM_PTR->dbgIsLocked = TRUE;
#	define NB_IOS_AVC_ITM_MUTEX_DEACTIVATE(ITM_PTR)	NBASSERT(ITM_PTR->dbgIsLocked) ITM_PTR->dbgIsLocked = FALSE; NBHILO_MUTEX_DESACTIVAR(&ITM_PTR->mutex)
#else
#	define NB_IOS_AVC_ITM_MUTEX_ACTIVATE(ITM_PTR)	NBHILO_MUTEX_ACTIVAR(&ITM_PTR->mutex)
#	define NB_IOS_AVC_ITM_MUTEX_DEACTIVATE(ITM_PTR)	NBHILO_MUTEX_DESACTIVAR(&ITM_PTR->mutex)
#endif

typedef struct AUAVCaptureSample_ {
	UI64				sequential;		//Sample idx; order received
	SI32				retainCount;	//
	SI32				rotDeg;			//the encoded image is rotated from intended image
	CVPixelBufferRef	pixBuffer;		//CFRetain, CFRelease
	BOOL				pixDataLocked;
	NBHILO_MUTEX_CLASE	mutex;
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	BOOL				dbgIsLocked;
#	endif
} AUAVCaptureSample;

void AUAVCaptureSample_init(AUAVCaptureSample* obj);
void AUAVCaptureSample_release(AUAVCaptureSample* obj);
void AUAVCaptureSample_setPixelBuffer(AUAVCaptureSample* obj, CVPixelBufferRef pixs);
BOOL AUAVCaptureSample_pushConsumer(AUAVCaptureSample* obj);
const void* AUAVCaptureSample_getLockedPixData(AUAVCaptureSample* obj, STNBBitmapProps* dstProps);
BOOL AUAVCaptureSample_popConsumer(AUAVCaptureSample* obj);

//

typedef struct AUAVCaptureVideoBuffer_ {
	SI32				readIdx;		//Currently read sample
	SI32				writeIdx;		//Currently write sample
	UI64				samplesReaded;	//Total count of samples consumed
	UI64				samplesWritten;	//Total count of samples written
	AUAVCaptureSample	samples[2];
	NBHILO_MUTEX_CLASE	mutex;
} AUAVCaptureVideoBuffer;

@interface AUAppVideoDataOutputDelegate : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate> {
@private
	AUAVCaptureVideoBuffer* _buffer; //dst
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	UI32					_dbgAccumWrite;
	UI32					_dbgAccumMissd;
	CICLOS_CPU_TIPO			_dbgCicleStarted;
#	endif
}
+ (AUAppVideoDataOutputDelegate*)sharedInstance:(AUAVCaptureVideoBuffer*)buffer;
@end


typedef struct AUAppGlueIOSAVCaptureData_ {
	AUAppI*							app;
	BOOL							requestingAuth;
	//Session
	struct {
		AVCaptureSession*			osObj;
        AVCaptureDevice*            captureDevice;
		ENAVCaptureState			state;
        ENAVCaptureFocusRange       focusRng;
		ENAVCaptureSize				streamSz;
		ENAVCaptureSize				photoSz;
		UI32						extraOutputsMask; //ENAVCaptureOutBit
	} session;
	//Video output
	struct {
		AUAVCaptureVideoBuffer			buffer;
		AVCaptureVideoDataOutput*		osOutput;
		AUAppVideoDataOutputDelegate*	osDelegate;
		dispatch_queue_t				osQueue;
	} video;
	//Photo output
	struct {
		NBHILO_MUTEX_CLASE			mutex;
		STNBArray					captured; //AUAVCaptureSample*
		UI64						samplesWritten;
		AVCapturePhotoOutput*		osOutput;
	} photo;
	//Metadata
	struct {
		NBHILO_MUTEX_CLASE			mutex;
		UI64						capturedTime;
		STNBString					captured;
		UI64						samplesWritten;
		AVCaptureMetadataOutput*	osOutput;
		dispatch_queue_t			osQueue;
	} metadata;
} AUAppGlueIOSAVCaptureData;

@interface AUAppMetaDataOutputDelegate : NSObject<AVCaptureMetadataOutputObjectsDelegate> {
@private
	AUAppGlueIOSAVCaptureData* _data; //dst
}
+ (AUAppMetaDataOutputDelegate*)sharedInstance:(AUAppGlueIOSAVCaptureData*)data;
@end

void AUAVCaptureSample_init(AUAVCaptureSample* s){
	s->sequential		= 0;
	s->retainCount		= 0;
	s->rotDeg			= 0;
	s->pixBuffer		= nil;
	s->pixDataLocked = FALSE;
	NBHILO_MUTEX_INICIALIZAR(&s->mutex);
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	s->dbgIsLocked		= FALSE;
#	endif
}

void AUAVCaptureSample_release(AUAVCaptureSample* s){
	NBHILO_MUTEX_ACTIVAR(&s->mutex);
	NBASSERT(!s->dbgIsLocked)
	NBASSERT(s->retainCount == 0)
	@autoreleasepool {
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		s->dbgIsLocked	= TRUE;
#		endif
		s->sequential	= 0;
		if(s->pixBuffer != nil){
			if(s->pixDataLocked){
				CVPixelBufferUnlockBaseAddress(s->pixBuffer, 0);
				s->pixDataLocked = FALSE;
			}
			CFRelease(s->pixBuffer);
			s->pixBuffer = nil;
		}
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		s->dbgIsLocked	= FALSE;
#		endif
		NBHILO_MUTEX_DESACTIVAR(&s->mutex);
		NBHILO_MUTEX_FINALIZAR(&s->mutex);
	}
}

void AUAVCaptureSample_setPixelBuffer(AUAVCaptureSample* itm, CVPixelBufferRef pixs){
	NBASSERT(itm != NULL)
	NB_IOS_AVC_ITM_MUTEX_ACTIVATE(itm)
	NBASSERT(itm->dbgIsLocked)
	NBASSERT(itm->retainCount == 0)
	@autoreleasepool {
		//Release previous
		if(itm->pixBuffer != nil){
			//PRINTF_INFO("Sample #%d buffer over-writtes previous non-consumed-sample.\n", (itm->sequential + 1));
			if(itm->pixDataLocked){
				CVPixelBufferUnlockBaseAddress(itm->pixBuffer, 0);
				itm->pixDataLocked = FALSE;
			}
			CFRelease(itm->pixBuffer);
			itm->pixBuffer = nil;
		} else {
			//PRINTF_INFO("Sample #%d buffer retained.\n", (itm->sequential + 1));
		}
		NBASSERT(itm->pixBuffer == nil)
		itm->pixBuffer = pixs;
		if(itm->pixBuffer != nil){
			NBASSERT(!itm->pixDataLocked)
			CFRetain(itm->pixBuffer);
		}
		NB_IOS_AVC_ITM_MUTEX_DEACTIVATE(itm)
	}
}

BOOL AUAVCaptureSample_pushConsumer(AUAVCaptureSample* itm){
	BOOL r = FALSE;
	@autoreleasepool {
		//Lock data
		if(!itm->pixDataLocked){
			NBASSERT(itm->pixBuffer != nil)
			if(kCVReturnSuccess != CVPixelBufferLockBaseAddress(itm->pixBuffer, 0)){
				//PRINTF_ERROR("VideoSampleBuffer-data could not be locked %d.\n", buff->readIdx);
			} else {
				//PRINTF_INFO("VideoSampleBuffer-data locked %d.\n", buff->readIdx);
				itm->pixDataLocked = TRUE;
			}
		}
		//Return (if locked)
		if(itm->pixDataLocked){
			//Lock read-itm
			if(itm->retainCount == 0){
				NB_IOS_AVC_ITM_MUTEX_ACTIVATE(itm)
				//PRINTF_INFO("VideoSampleBuffer-mutex locked %d.\n", buff->readIdx);
			}
			NBASSERT(itm->retainCount >= 0)
			itm->retainCount++;
			r = TRUE;
		}
	}
	return r;
}

BOOL AUAVCaptureSample_popConsumer(AUAVCaptureSample* itm){
	BOOL r = FALSE;
	NBASSERT(itm->dbgIsLocked)
	NBASSERT(itm->retainCount > 0)
	@autoreleasepool {
		itm->retainCount--;
		if(itm->retainCount == 0){
			//Release buffer
			if(itm->pixBuffer != nil){
				//Unlock data
				if(itm->pixDataLocked){
					CVPixelBufferUnlockBaseAddress(itm->pixBuffer, 0);
					itm->pixDataLocked = FALSE;
					//PRINTF_INFO("VideoSampleBuffer-data unlocked %d.\n", buff->readIdx);
				}
				CFRelease(itm->pixBuffer);
				itm->pixBuffer = nil;
				//PRINTF_INFO("VideoSampleBuffer released %d.\n", buff->readIdx);
			}
			r = TRUE;
			//Unlock read-itm
			NB_IOS_AVC_ITM_MUTEX_DEACTIVATE(itm)
		}
	}
	return r;
}

const void* AUAVCaptureSample_getLockedPixData(AUAVCaptureSample* itm, STNBBitmapProps* dstProps){
	const void* r = NULL;
	@autoreleasepool {
		STNBBitmapProps props;
		NBMemory_setZeroSt(props, STNBBitmapProps);
		if(itm != NULL){
			NBASSERT(itm->pixDataLocked)
			if(itm->pixDataLocked){
				NBASSERT(itm->pixBuffer != NULL)
				CVPixelBufferRef pixs		= itm->pixBuffer;
				const SI32 width 			= (SI32)CVPixelBufferGetWidth(pixs);
				const SI32 height 			= (SI32)CVPixelBufferGetHeight(pixs);
				const SI32 bytesPerRow 		= (SI32)CVPixelBufferGetBytesPerRow(pixs);
				const OSType pixFmt 		= CVPixelBufferGetPixelFormatType(pixs);
				switch (pixFmt) {
					case kCVPixelFormatType_32BGRA:
						props.color				= ENNBBitmapColor_BGRA8;
						props.size.width		= width;
						props.size.height		= height;
						props.bitsPerPx			= 32;
						props.bytesPerLine		= bytesPerRow;
						break;
					case kCVPixelFormatType_32ARGB:
						props.color				= ENNBBitmapColor_ARGB8;
						props.size.width		= width;
						props.size.height		= height;
						props.bitsPerPx			= 32;
						props.bytesPerLine		= bytesPerRow;
						break;
					case kCVPixelFormatType_32RGBA:
						props.color				= ENNBBitmapColor_RGBA8;
						props.size.width		= width;
						props.size.height		= height;
						props.bitsPerPx			= 32;
						props.bytesPerLine		= bytesPerRow;
						break;
					default:
						break;
				}
				const BYTE* pixData	= (const BYTE*)CVPixelBufferGetBaseAddress(pixs);
				const SI32 dataSz 	= (SI32)CVPixelBufferGetDataSize(pixs);
				NBASSERT(pixData != NULL)
				NBASSERT(dataSz >= (SI32)(CVPixelBufferGetBytesPerRow(pixs) * CVPixelBufferGetHeight(pixs)))
				if(dstProps != NULL){
					*dstProps = props;
				}
				r = pixData;
			}
		}
	}
	return r;
}

//

@implementation AUAppMetaDataOutputDelegate
+ (AUAppMetaDataOutputDelegate*)sharedInstance:(AUAppGlueIOSAVCaptureData*)data {
	static AUAppMetaDataOutputDelegate* inst = nil;
	if(inst == nil){
		inst = [[AUAppMetaDataOutputDelegate alloc] initWithData: data];
	}
	return inst;
}
- (id)initWithData:(AUAppGlueIOSAVCaptureData*)data {
	PRINTF_INFO("AUAppMetaDataOutputDelegate, init\n");
	self = [super init];
	if (self) {
		_data = data;
	}
	return self;
}
- (void)dealloc {
	PRINTF_INFO("AUAppMetaDataOutputDelegate, dealloc\n");
	_data = NULL;
	[super dealloc];
}
- (void)captureOutput:(AVCaptureOutput *)output didOutputMetadataObjects:(NSArray<__kindof AVMetadataObject *> *)metadataObjects fromConnection:(AVCaptureConnection *)connection {
	@autoreleasepool {
		if (metadataObjects != nil && [metadataObjects count] > 0) {
			AVMetadataMachineReadableCodeObject *metadataObj = [metadataObjects objectAtIndex:0];
			if ([[metadataObj type] isEqualToString:AVMetadataObjectTypeQRCode]) {
				{
					NBHILO_MUTEX_ACTIVAR(&_data->metadata.mutex);
					_data->metadata.capturedTime = NBDatetime_getCurUTCTimestamp();
					NBString_set(&_data->metadata.captured, [[metadataObj stringValue] UTF8String]);
					NBHILO_MUTEX_DESACTIVAR(&_data->metadata.mutex);
				}
				//PRINTF_INFO("AUAppMetaDataOutputDelegate, capured: '%s'\n", [[metadataObj stringValue] UTF8String]);
			}
		}
	}
}
@end

//

@implementation AUAppVideoDataOutputDelegate
+ (AUAppVideoDataOutputDelegate*)sharedInstance:(AUAVCaptureVideoBuffer*)buffer {
	static AUAppVideoDataOutputDelegate* inst = nil;
	if(inst == nil){
		inst = [[AUAppVideoDataOutputDelegate alloc] initWithBuffer: buffer];
	}
	return inst;
}
- (id)initWithBuffer:(AUAVCaptureVideoBuffer*)buffer {
	PRINTF_INFO("AUAppVideoDataOutputDelegate, init\n");
	self = [super init];
	if (self) {
		_buffer				= buffer;
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		_dbgAccumWrite		= 0;
		_dbgAccumMissd		= 0;
		_dbgCicleStarted	= 0;
#		endif
	}
	return self;
}
- (void)dealloc {
	PRINTF_INFO("AUAppVideoDataOutputDelegate, dealloc\n");
	_buffer = NULL;
	[super dealloc];
}
- (void)captureOutput:(AVCaptureOutput *)output didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
	//PRINTF_INFO("AUAppVideoDataOutputDelegate, captureOutput:didOutputSampleBuffer:fromConnection:\n");
	@autoreleasepool {
		if(_buffer != NULL){
			AUAVCaptureVideoBuffer* buff = _buffer;
			//Determine and lock the destination
			AUAVCaptureSample* itm = NULL;
			{
				NBHILO_MUTEX_ACTIVAR(&buff->mutex)
				{
					itm = &buff->samples[buff->writeIdx];
					CVImageBufferRef img	= CMSampleBufferGetImageBuffer(sampleBuffer);
					CVPixelBufferRef pixs	= (CVPixelBufferRef)img;
					AUAVCaptureSample_setPixelBuffer(itm, pixs);
					//Move writeIdx to next
					if(buff->writeIdx == buff->readIdx){
						buff->writeIdx = (buff->writeIdx == 0 ? 1 : 0);
					}
					buff->samplesWritten++;
					itm->sequential = buff->samplesWritten;
				}
#				ifdef NB_CONFIG_INCLUDE_ASSERTS
				{
					CICLOS_CPU_TIPO curCicle, oneSec;
					CICLOS_CPU_POR_SEGUNDO(oneSec);
					CICLOS_CPU_HILO(curCicle);
					_dbgAccumWrite++;
					if(_dbgCicleStarted == 0) _dbgCicleStarted = curCicle;
					while((_dbgCicleStarted + oneSec) <= curCicle){
						//PRINTF_INFO("AUAppVideoDataOutputDelegate, %d writen, %d missed; frames-per-sec.\n", _dbgAccumWrite, _dbgAccumMissd);
						_dbgCicleStarted += oneSec;
						_dbgAccumMissd = 0;
						_dbgAccumWrite = 0;
					}
				}
#				endif
				NBHILO_MUTEX_DESACTIVAR(&buff->mutex)
			}
		}
	}
}

- (void)captureOutput:(AVCaptureOutput *)output didDropSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
	//PRINTF_INFO("AUAppVideoDataOutputDelegate, captureOutput:didDropSampleBuffer:fromConnection:\n");
	@autoreleasepool {
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		if(_buffer != NULL){
			AUAVCaptureVideoBuffer* buff = _buffer;
			NBHILO_MUTEX_ACTIVAR(&buff->mutex)
			{
				CICLOS_CPU_TIPO curCicle, oneSec;
				CICLOS_CPU_POR_SEGUNDO(oneSec);
				CICLOS_CPU_HILO(curCicle);
				_dbgAccumMissd++;
				if(_dbgCicleStarted == 0) _dbgCicleStarted = curCicle;
				while((_dbgCicleStarted + oneSec) <= curCicle){
					//PRINTF_INFO("AUAppVideoDataOutputDelegate, %d writen, %d missed; frames-per-sec.\n", _dbgAccumWrite, _dbgAccumMissd);
					_dbgCicleStarted += oneSec;
					_dbgAccumMissd = 0;
					_dbgAccumWrite = 0;
				}
			}
			NBHILO_MUTEX_DESACTIVAR(&buff->mutex)
		}
#		endif
	}
}
@end


//

@interface AUAppPhotoCaptureDelegate : NSObject<AVCapturePhotoCaptureDelegate> {
@private
	AUAppGlueIOSAVCaptureData*	data;
}
+ (AUAppPhotoCaptureDelegate*)sharedInstance:(AUAppGlueIOSAVCaptureData*)data;
@end

@implementation AUAppPhotoCaptureDelegate
+ (AUAppPhotoCaptureDelegate*)sharedInstance:(AUAppGlueIOSAVCaptureData*)data {
	static AUAppPhotoCaptureDelegate* inst = nil;
	if(inst == nil){
		inst = [[AUAppPhotoCaptureDelegate alloc] initWithData: data];
	}
	return inst;
}
- (id)initWithData:(AUAppGlueIOSAVCaptureData*)data {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, init\n");
	self = [super init];
	if (self) {
		self->data = data;
	}
	return self;
}
- (void)dealloc {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, dealloc\n");
	[super dealloc];
	//
}
//Monitoring Capture Progress
- (void)captureOutput:(AVCapturePhotoOutput *)output willBeginCaptureForResolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:willBeginCaptureForResolvedSettings:\n");
}
- (void)captureOutput:(AVCapturePhotoOutput *)output willCapturePhotoForResolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:willCapturePhotoForResolvedSettings:\n");
}
- (void)captureOutput:(AVCapturePhotoOutput *)output didCapturePhotoForResolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didCapturePhotoForResolvedSettings:\n");
}
- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishCaptureForResolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings error:(NSError *)error {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didFinishCaptureForResolvedSettings:error:\n");
}
//Receiving Capture Results
- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishProcessingPhoto:(AVCapturePhoto *)photo error:(NSError *)error  API_AVAILABLE(ios(11.0)){
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didFinishProcessingPhoto:error:\n");
	@autoreleasepool {
		if(error) {
			NSLog(@"AUAppPhotoCaptureDelegate, ERROR captureOutput: %@", error.localizedDescription);
		}
		//
		/*if([photo isRawPhoto]){
		 PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured RAW photo.\n");
		 } else {
		 PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured not-RAW photo.\n");
		 }*/
		//
		/*{
		 CVPixelBufferRef pxBuff = [photo previewPixelBuffer];
		 if(pxBuff == nil){
		 PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured has no previewBuffer.\n");
		 } else {
		 const SI32 width	= (SI32)CVPixelBufferGetWidth(pxBuff);
		 const SI32 height	= (SI32)CVPixelBufferGetHeight(pxBuff);
		 const SI32 bPerRow	= (SI32)CVPixelBufferGetBytesPerRow(pxBuff);
		 const SI32 bSize	= (SI32)CVPixelBufferGetDataSize(pxBuff);
		 const SI32 planes	= (SI32)CVPixelBufferGetPlaneCount(pxBuff);
		 const OSType pxType	= CVPixelBufferGetPixelFormatType(pxBuff);
		 const char* pxTypeC	= (const char*)&pxType;
		 const BYTE* data	= (const BYTE*)CVPixelBufferGetBaseAddress(pxBuff);
		 PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured previewBuffer sz(%d, %d) pxFmt('%c%c%c%c') bytesPerRow(%d) rows(%d); planes(%d).\n", width, height, pxTypeC[0], pxTypeC[1], pxTypeC[2], pxTypeC[3], bPerRow, (bSize / bPerRow), planes);
		 }
		 }*/
		{
			CVPixelBufferRef pxBuff = [photo pixelBuffer];
			if(pxBuff == nil){
				PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured has no pixelBuffer.\n");
			} else {
				//Add to queue
				{
					AUAVCaptureSample* s = NBMemory_allocType(AUAVCaptureSample);
					AUAVCaptureSample_init(s);
					AUAVCaptureSample_setPixelBuffer(s, pxBuff);
					//Detect rotation
					{
						NSDictionary<NSString *,id>* meta = [photo metadata];
						if(meta != nil){
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
											s->rotDeg = 180;
											break;
										case kCGImagePropertyOrientationDownMirrored:
											//The encoded image data is vertically flipped from the image's intended display orientation.
											break;
										case kCGImagePropertyOrientationLeftMirrored:
											//The encoded image data is horizontally flipped and rotated 90° counter-clockwise from the image's intended display orientation.
											break;
										case kCGImagePropertyOrientationRight:
											//The encoded image data is rotated 90° clockwise from the image's intended display orientation.
											s->rotDeg = 90;
											break;
										case kCGImagePropertyOrientationRightMirrored:
											//The encoded image data is horizontally flipped and rotated 90° clockwise from the image's intended display orientation.
											break;
										case kCGImagePropertyOrientationLeft:
											//The encoded image data is rotated 90° clockwise from the image's intended display orientation.
											s->rotDeg = 270;
											break;
									}
								}
							}
						}
					}
					NBHILO_MUTEX_ACTIVAR(&data->photo.mutex);
					{
						data->photo.samplesWritten++;
						s->sequential = data->photo.samplesWritten;
						NBArray_addValue(&data->photo.captured, s);
						PRINTF_INFO("AUAppPhotoCaptureDelegate, photo #%llu added to buffer (%d in queue).\n", data->photo.samplesWritten, data->photo.captured.use);
					}
					NBHILO_MUTEX_DESACTIVAR(&data->photo.mutex);
				}
				/*void AUAVCaptureSample_init(AUAVCaptureSample* obj);
				 void AUAVCaptureSample_release(AUAVCaptureSample* obj);
				 BOOL AUAVCaptureSample_pushConsumer(AUAVCaptureSample* obj);
				 const void* AUAVCaptureSample_getLockedPixData(AUAVCaptureSample* obj, STNBBitmapProps* dstProps);
				 BOOL AUAVCaptureSample_popConsumer(AUAVCaptureSample* obj);*/
				
				/*const SI32 width	= (SI32)CVPixelBufferGetWidth(pxBuff);
				 const SI32 height	= (SI32)CVPixelBufferGetHeight(pxBuff);
				 const SI32 bPerRow	= (SI32)CVPixelBufferGetBytesPerRow(pxBuff);
				 const SI32 bSize	= (SI32)CVPixelBufferGetDataSize(pxBuff);
				 const SI32 planes	= (SI32)CVPixelBufferGetPlaneCount(pxBuff);
				 const OSType pxType	= CVPixelBufferGetPixelFormatType(pxBuff);
				 const char* pxTypeC	= (const char*)&pxType;
				 const BYTE* data	= (const BYTE*)CVPixelBufferGetBaseAddress(pxBuff);
				 PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured pixelBuffer sz(%d, %d) pxFmt('%c%c%c%c') bytesPerRow(%d) rows(%d) planes(%d).\n", width, height, pxTypeC[0], pxTypeC[1], pxTypeC[2], pxTypeC[3], bPerRow, (bSize / bPerRow), planes);*/
			}
		}
	}
}
- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishRecordingLivePhotoMovieForEventualFileAtURL:(NSURL *)outputFileURL resolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didFinishRecordingLivePhotoMovieForEventualFileAtURL:resolvedSettings:\n");
}
- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishProcessingLivePhotoToMovieFileAtURL:(NSURL *)outputFileURL duration:(CMTime)duration photoDisplayTime:(CMTime)photoDisplayTime resolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings error:(NSError *)error {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didFinishProcessingLivePhotoToMovieFileAtURL:duration:photoDisplayTime:resolvedSettings:error:\n");
}
//Receiving Capture Results (Deprecated)
- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishProcessingPhotoSampleBuffer:(CMSampleBufferRef)photoSampleBuffer previewPhotoSampleBuffer:(CMSampleBufferRef)previewPhotoSampleBuffer resolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings bracketSettings:(AVCaptureBracketedStillImageSettings *)bracketSettings error:(NSError *)error {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didFinishProcessingPhotoSampleBuffer:previewPhotoSampleBuffer:resolvedSettings:bracketSettings:error:\n");
	@autoreleasepool {
		if(error) {
			NSLog(@"AUAppPhotoCaptureDelegate, ERROR captureOutput: %@", error.localizedDescription);
		}
		if(!photoSampleBuffer) {
			PRINTF_ERROR("AUAppPhotoCaptureDelegate, Capture returned with no photoSampleBuffer.\n");
		} else {
#			if defined(NB_CONFIG_INCLUDE_ASSERTS) && !defined(CONFIG_NB_DESHABILITAR_IMPRESIONES_PRINTF)
			NSData *data	= [AVCapturePhotoOutput JPEGPhotoDataRepresentationForJPEGSampleBuffer:photoSampleBuffer previewPhotoSampleBuffer:previewPhotoSampleBuffer];
			UIImage *image	= [UIImage imageWithData:data];
			NSData *pngData	= UIImagePNGRepresentation(image);
			PRINTF_INFO("AUAppPhotoCaptureDelegate, Captured image has %d bytes as JPEG.\n", (UI32)[pngData length]);
#			endif
		}
	}
}
- (void)captureOutput:(AVCapturePhotoOutput *)output didFinishProcessingRawPhotoSampleBuffer:(CMSampleBufferRef)rawSampleBuffer previewPhotoSampleBuffer:(CMSampleBufferRef)previewPhotoSampleBuffer resolvedSettings:(AVCaptureResolvedPhotoSettings *)resolvedSettings bracketSettings:(AVCaptureBracketedStillImageSettings *)bracketSettings error:(NSError *)error {
	PRINTF_INFO("AUAppPhotoCaptureDelegate, captureOutput:didFinishProcessingRawPhotoSampleBuffer:previewPhotoSampleBuffer:resolvedSettings:bracketSettings:error:\n");
}
@end

//Calls

bool AUAppGlueIOSAVCapture::create(AUAppI* app, STMngrAVCaptureCalls* obj){
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)NBMemory_alloc(sizeof(AUAppGlueIOSAVCaptureData));
	NBMemory_setZeroSt(*data, AUAppGlueIOSAVCaptureData);
	NBMemory_setZeroSt(*obj, STMngrAVCaptureCalls);
	data->app					= (AUAppI*)app;
	data->requestingAuth		= FALSE;
	//
	@autoreleasepool {
		//Session
		{
			data->session.osObj		= nil;
			data->session.state		= ENAVCaptureState_Stopped;
			data->session.extraOutputsMask = 0;
		}
		//Video output
		{
			AUAVCaptureVideoBuffer* buff = &data->video.buffer;
			buff->readIdx			= 0;
			buff->writeIdx			= 0;
			buff->samplesReaded		= 0;
			buff->samplesWritten	= 0;
			{
				SI32 i; const SI32 count = (sizeof(buff->samples) / sizeof(buff->samples[0]));
				for(i = 0 ; i < count; i++){
					AUAVCaptureSample* s = &buff->samples[i];
					AUAVCaptureSample_init(s);
				}
			}
			NBHILO_MUTEX_INICIALIZAR(&buff->mutex);
			data->video.osOutput	= nil;
			data->video.osDelegate	= nil;
			data->video.osQueue		= nil;
		}
		//Photo output
		{
			NBHILO_MUTEX_INICIALIZAR(&data->photo.mutex)
			NBArray_init(&data->photo.captured, sizeof(AUAVCaptureSample*), NULL);
			data->photo.samplesWritten = 0;
			data->photo.osOutput	= nil;
		}
		//Metadata
		{
			NBHILO_MUTEX_INICIALIZAR(&data->metadata.mutex)
			data->metadata.capturedTime	= 0;
			NBString_init(&data->metadata.captured);
			data->metadata.samplesWritten = 0;
			data->metadata.osOutput		= nil;
			data->metadata.osQueue		= nil;
		}
	}
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcCaptureAuthStatus			= captureAuthStatus;
	obj->funcCaptureAuthStatusParam 	= data;
	obj->funcCaptureStart				= captureStart;
	obj->funcCaptureStartParam			= data;
	obj->funcCaptureIsRuning			= captureIsRuning;
	obj->funcCaptureIsRuningParam		= data;
	obj->funcCaptureStop				= captureStop;
	obj->funcCaptureStopParam			= data;
	//
	obj->funcPhotoTrigger				= photoTrigger;
	obj->funcPhotoTriggerParam			= data;
	obj->funcPhotoSampleRetain			= photoSampleRetain;
	obj->funcPhotoSampleRetainParam		= data;
	obj->funcPhotoSampleRelease			= photoSampleRelease;
	obj->funcPhotoSampleReleaseParam	= data;
	obj->funcPhotoSamplePixData			= photoSamplePixData;
	obj->funcPhotoSamplePixDataParam	= data;
	//
	obj->funcVideoSampleRetain			= videoSampleRetain;
	obj->funcVideoSampleRetainParam		= data;
	obj->funcVideoSampleRelease			= videoSampleRelease;
	obj->funcVideoSampleReleaseParam	= data;
	obj->funcVideoSamplePixData			= videoSamplePixData;
	obj->funcVideoSamplePixDataParam	= data;
	//
	obj->funcMetaQRCode					= metaQRCode;
	obj->funcMetaQRCodeParam			= data;
    //
    //focus
    obj->funcIsFocusModeSupported       = isFocusModeSupported;
    obj->funcIsFocusModeSupportedParam  = data;
    obj->funcGetFocusMode               = getFocusMode;
    obj->funcGetFocusModeParam          = data;
    obj->funcSetFocusMode               = setFocusMode;
    obj->funcSetFocusModeParam          = data;
    obj->funcIsAdjustingFocus           = isAdjustingFocus;
    obj->funcIsAdjustingFocusParam      = data;
    obj->funcGetLensPositionRel         = getLensPositionRel;
    obj->funcGetLensPositionRelParam    = data;
    obj->funcLockLensToPositionRel      = lockLensToPositionRel;
    obj->funcLockLensToPositionRelParam = data;
	//
	return true;
}

bool AUAppGlueIOSAVCapture::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
		@autoreleasepool {
			//Metadata
			{
				{
					NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
					NBString_release(&data->metadata.captured);
					NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
					NBHILO_MUTEX_FINALIZAR(&data->metadata.mutex)
				}
				if(data->metadata.osQueue != nil){
					dispatch_release(data->metadata.osQueue);
					data->metadata.osQueue = nil;
				}
				if(data->metadata.osOutput != nil){
					[data->metadata.osOutput release];
					data->metadata.osOutput = nil;
				}
			}
			//Photo output
			{
				if(data->photo.osOutput != nil){
					[data->photo.osOutput release];
					data->photo.osOutput = nil;
				}
				{
					NBHILO_MUTEX_ACTIVAR(&data->photo.mutex)
					{
						SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
							AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
							AUAVCaptureSample_release(s);
							NBMemory_free(s);
						}
						NBArray_empty(&data->photo.captured);
						NBArray_release(&data->photo.captured);
					}
					NBHILO_MUTEX_DESACTIVAR(&data->photo.mutex)
					NBHILO_MUTEX_FINALIZAR(&data->photo.mutex)
				}
			}
			//Video output
			{
				{
					if(data->video.osQueue != nil){
						dispatch_release(data->video.osQueue);
						data->video.osQueue = nil;
					}
					if(data->video.osDelegate != nil){
						[data->video.osDelegate release];
						data->video.osDelegate = nil;
					}
					if(data->video.osOutput != nil){
						[data->video.osOutput release];
						data->video.osOutput = nil;
					}
				}
				//
				{
					AUAVCaptureVideoBuffer* buff = &data->video.buffer;
					buff->readIdx		= 0;
					buff->writeIdx		= 0;
					buff->samplesReaded	= 0;
					buff->samplesWritten = 0;
					NBHILO_MUTEX_ACTIVAR(&buff->mutex);
					{
						SI32 i; const SI32 count = (sizeof(buff->samples) / sizeof(buff->samples[0]));
						for(i = 0 ; i < count; i++){
							AUAVCaptureSample* s = &buff->samples[i];
							AUAVCaptureSample_release(s);
						}
					}
					NBHILO_MUTEX_DESACTIVAR(&buff->mutex);
					NBHILO_MUTEX_FINALIZAR(&buff->mutex);
				}
			}
			//Session
			{
				//Wait until running or stopped (a thread is bussy)
				while(data->session.state != ENAVCaptureState_Running && data->session.state != ENAVCaptureState_Stopped){
					//
				}
                if(data->session.captureDevice != nil){
                    [data->session.captureDevice release];
                    data->session.captureDevice = nil;
                }
				if(data->session.osObj != nil){
					if(data->session.state != ENAVCaptureState_Stopped){
						[data->session.osObj stopRunning];
					}
					[data->session.osObj release];
					data->session.osObj = nil;
				}
				data->session.state = ENAVCaptureState_Stopped;
			}
		}
		data->app = NULL;
		NBMemory_free(pData);
		r = true;
	}
	return r;
}

//

ENAVCaptureAuthStatus AUAppGlueIOSAVCapture::captureAuthStatus(void* pData, const BOOL requestIfNecesary){
	ENAVCaptureAuthStatus r = ENAVCaptureAuthStatus_Denied;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	if(data->requestingAuth){
		r = ENAVCaptureAuthStatus_Requesting;
	} else {
		@autoreleasepool {
			AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
			switch(status) {
				case AVAuthorizationStatusRestricted: //The user cannot change this
					//PRINTF_INFO("VideoCapture, AVAuthorizationStatusRestricted.\n");
					r = ENAVCaptureAuthStatus_Restricted;
					break;
				case AVAuthorizationStatusDenied:
					//PRINTF_INFO("VideoCapture, AVAuthorizationStatusDenied.\n");
					r = ENAVCaptureAuthStatus_Denied;
					break;
				case AVAuthorizationStatusAuthorized:
					//PRINTF_INFO("VideoCapture, AVAuthorizationStatusAuthorized.\n");
					r = ENAVCaptureAuthStatus_Authorized;
					break;
				default: //AVAuthorizationStatusNotDetermined
					r = ENAVCaptureAuthStatus_NotDetermined;
					if(requestIfNecesary){
						r = ENAVCaptureAuthStatus_Requesting;
						data->requestingAuth = TRUE;
						[AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
							if(granted){
								PRINTF_INFO("VideoCapture, the user has granted authorization.\n");
							} else {
								PRINTF_ERROR("VideoCapture, the user has denied authorization.\n");
							}
							data->requestingAuth = FALSE;
						}];
					}
					break;
			}
		}
	}
	return r;
}

SI64 AUAppGlueIOSAVCapture_applyStateAndRelease_(STNBThread* thread, void* param){
	SI64 r = 0;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)param;
	if(data != NULL){
		@autoreleasepool {
			NBASSERT(data->session.state == ENAVCaptureState_Starting || data->session.state == ENAVCaptureState_Stopping)
			if(data->session.state == ENAVCaptureState_Stopping){
				PRINTF_INFO("VideoCapture, Capture session stopping.\n");
				//Stop session
				if(data->session.osObj != nil){
					[data->session.osObj stopRunning];
					PRINTF_INFO("VideoCapture, Capture session stopped.\n");
					[data->session.osObj release];
					data->session.osObj = nil;
				}
				//Release metadata output
				{
					if(data->metadata.osQueue != nil){
						dispatch_release(data->metadata.osQueue);
						data->metadata.osQueue = nil;
					}
					if(data->metadata.osOutput != nil){
						[data->metadata.osOutput release];
						data->metadata.osOutput = nil;
					}
				}
				//Release photo output
				{
					if(data->photo.osOutput != nil){
						[data->photo.osOutput release];
						data->photo.osOutput = nil;
					}
				}
				//Release video stream
				{
					if(data->video.osQueue != nil){
						dispatch_release(data->video.osQueue);
						data->video.osQueue = nil;
					}
					if(data->video.osDelegate != nil){
						[data->video.osDelegate release];
						data->video.osDelegate = nil;
					}
					if(data->video.osOutput != nil){
						[data->video.osOutput release];
						data->video.osOutput = nil;
					}
				}
				data->session.state = ENAVCaptureState_Stopped;
			} else {
				//Start session
				PRINTF_INFO("VideoCapture, Capture session starting.\n");
				NBASSERT(data->session.state == ENAVCaptureState_Starting)
				//Create session
				if(data->session.osObj == nil){
					AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
					if(status == AVAuthorizationStatusAuthorized){
						AVCaptureSession* captureSession	= [[AVCaptureSession alloc] init];
						{
							//const ENAVCaptureSize highstSz = (data->session.streamSz > data->session.photoSz ? data->session.streamSz : data->session.photoSz); 
							switch (data->session.streamSz) {
								case ENAVCaptureSize_Low192:	//192 x 144 (1.33)
									[captureSession setSessionPreset:AVCaptureSessionPresetLow]; //Low (iPhoneSE: 192 x 144)
									break;
								case ENAVCaptureSize_Low352:	//352 x 288 (1.22)
									[captureSession setSessionPreset:AVCaptureSessionPreset352x288]; //LowMedium (iPhoneSE: 352 x 288)
									break;
								case ENAVCaptureSize_Med640:	//640 x 480 (1.33)
									[captureSession setSessionPreset:AVCaptureSessionPreset640x480]; //LowMedium (iPhoneSE: 640 x 480)
									//[captureSession setSessionPreset:AVCaptureSessionPresetMedium]; //Medium (iPhoneSE: 480, 360)
									break;
								case ENAVCaptureSize_Med1280:	//1280 x 720 (1.77)
									[captureSession setSessionPreset:AVCaptureSessionPreset1280x720]; //LowMedium (iPhoneSE: 1280 x 720)
									break;
								case ENAVCaptureSize_High3840:	//3840 x 2160 (1.77)
									[captureSession setSessionPreset:AVCaptureSessionPreset3840x2160]; //High (iPhoneSE: 3840 x 2160)
									break;
								case ENAVCaptureSize_High4032:	//4032 x 3024 (1.33)
									[captureSession setSessionPreset:AVCaptureSessionPresetPhoto]; //Photo quality (iPhoneSE: 4032 x 3024)
									//[captureSession setSessionPreset:AVCaptureSessionPresetHigh]; //High (iPhoneSE: 1920, 1080)
									break;
								default:
									//ENAVCaptureSize_High1920:	//1920, 1080 (1.77)
									[captureSession setSessionPreset:AVCaptureSessionPreset1920x1080]; //High (iPhoneSE: 1920, 1080)
									break;
							}
						}
						AVCaptureDevice* captureDevice		= [AVCaptureDevice defaultDeviceWithDeviceType:AVCaptureDeviceTypeBuiltInWideAngleCamera mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionBack];
						//AVCaptureDevice* captureDevice	= [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
						if(captureDevice == nil){
							PRINTF_ERROR("VideoCapture, No capture device returned.\n");
						} else {
							NSError *error = nil;
							if(![captureDevice lockForConfiguration:&error]){
								PRINTF_ERROR("VideoCapture, Capture device could not be locked for configuration: '%s'.\n", [[error description] UTF8String]);
							} else {
								[captureDevice setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
                                if(captureDevice.focusPointOfInterestSupported){
                                    CGPoint pintOfInterest;
                                    NBMemory_setZeroSt(pintOfInterest, CGPoint);
                                    pintOfInterest.x = pintOfInterest.y = 0.5f;
                                    [captureDevice setFocusPointOfInterest:pintOfInterest];
                                }
                                if(captureDevice.exposurePointOfInterestSupported){
                                    CGPoint pintOfInterest;
                                    NBMemory_setZeroSt(pintOfInterest, CGPoint);
                                    pintOfInterest.x = pintOfInterest.y = 0.5f;
                                    [captureDevice setExposurePointOfInterest:pintOfInterest];
                                }
                                if(captureDevice.autoFocusRangeRestrictionSupported){
                                    switch (data->session.focusRng) {
                                        case ENAVCaptureFocusRange_Near:
                                            [captureDevice setAutoFocusRangeRestriction:AVCaptureAutoFocusRangeRestrictionNear];
                                            break;
                                        case ENAVCaptureFocusRange_Far:
                                            [captureDevice setAutoFocusRangeRestriction:AVCaptureAutoFocusRangeRestrictionFar];
                                            break;
                                        default:
                                            [captureDevice setAutoFocusRangeRestriction:AVCaptureAutoFocusRangeRestrictionNone];
                                            break;
                                    }
                                }
								[captureDevice unlockForConfiguration];
								PRINTF_INFO("VideoCapture, Capture configured.\n");
								{
									NSError *error = nil;
									AVCaptureDeviceInput* videoInput = [AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:&error];
									if(videoInput == nil) {
										PRINTF_ERROR("VideoCapture, Could not obtain the device input: '%s'.\n", [[error description] UTF8String]);
									} else {
										if(![captureSession canAddInput:videoInput]){
											PRINTF_ERROR("VideoCapture, Could not add input.\n");
										} else {
											[captureSession addInput:videoInput];
											PRINTF_INFO("VideoCapture, Capture input added.\n");
											//PhotoOutput
											AVCapturePhotoOutput* photoOutput = [[AVCapturePhotoOutput alloc] init];
											[photoOutput setHighResolutionCaptureEnabled:TRUE];
											if(![captureSession canAddOutput:photoOutput]){
												PRINTF_ERROR("VideoCapture, Could not add output.\n");
											} else {
												[captureSession addOutput:photoOutput];
												PRINTF_INFO("VideoCapture, Capture output added.\n");
												{
													//VideoOutput
													AVCaptureVideoDataOutput* osOutput = [[AVCaptureVideoDataOutput alloc] init];
													{
														NSDictionary* outputSettings = @{ (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA) };
														osOutput.videoSettings = outputSettings;
													}
													AUAppVideoDataOutputDelegate* osDelegate = [[AUAppVideoDataOutputDelegate alloc] initWithBuffer:&data->video.buffer];
													dispatch_queue_t osQueue = dispatch_queue_create("capture_session_queue", NULL);
													[osOutput setSampleBufferDelegate:osDelegate queue:osQueue];
													if(![captureSession canAddOutput:osOutput]){
														PRINTF_ERROR("VideoCapture, Could not add outputData.\n");
													} else {
														[captureSession addOutput:osOutput];
														//Metadata output
														{
															//Release metadata
															{
																if(data->metadata.osQueue != nil){
																	dispatch_release(data->metadata.osQueue);
																	data->metadata.osQueue = nil;
																}
																if(data->metadata.osOutput != nil){
																	[data->metadata.osOutput release];
																	data->metadata.osOutput = nil;
																}
															}
															//Start metadata output
															if((data->session.extraOutputsMask & ENAVCaptureOutBit_QRCode) != 0){
																AVCaptureMetadataOutput* metadataOutput = [[AVCaptureMetadataOutput alloc] init];
																[captureSession addOutput:metadataOutput];
																//
																dispatch_queue_t dispatchQueue;
																dispatchQueue = dispatch_queue_create("metaQueue", NULL);
																[metadataOutput setMetadataObjectsDelegate:[AUAppMetaDataOutputDelegate sharedInstance:data] queue:dispatchQueue];
																[metadataOutput setMetadataObjectTypes:[NSArray arrayWithObject:AVMetadataObjectTypeQRCode]];
																//
																data->metadata.osOutput = metadataOutput;
																data->metadata.osQueue	= dispatchQueue;
															}
														}
														//Set
														{
															//Release video stream
															{
																if(data->video.osQueue != nil){
																	dispatch_release(data->video.osQueue);
																	data->video.osQueue = nil;
																}
																if(data->video.osDelegate != nil){
																	[data->video.osDelegate release];
																	data->video.osDelegate = nil;
																}
																if(data->video.osOutput != nil){
																	[data->video.osOutput release];
																	data->video.osOutput = nil;
																}
															}
															data->video.osOutput	= osOutput;
															data->video.osDelegate	= osDelegate;
															data->video.osQueue		= osQueue;
														}
														osDelegate = nil;
														osOutput = nil;
														osQueue = nil;
														PRINTF_INFO("VideoCapture, Capture outputData added.\n");
													}
													//Release
													{
														if(osDelegate != nil){
															[osDelegate release];
															osDelegate = nil;
														}
														if(osQueue != nil){
															dispatch_release(osQueue);
															osQueue = nil;
														}
														if(osOutput != nil){
															[osOutput release];
															osOutput = nil;
														}
													}
												}
												//Save photo output
												{
													//Release photo output
													if(data->photo.osOutput != nil){
														[data->photo.osOutput release];
														data->photo.osOutput = nil;
													}
													data->photo.osOutput = photoOutput;
													photoOutput = nil;
												}
												//Save session
												{
                                                    //Release capture device
                                                    if(data->session.captureDevice != nil){
                                                        [data->session.captureDevice release];
                                                        data->session.captureDevice = nil;
                                                    }
                                                    {
                                                        data->session.captureDevice = captureDevice;
                                                        if(data->session.captureDevice != nil){
                                                            [data->session.captureDevice retain];
                                                        }
                                                    }
													//Release session
													if(data->session.osObj != nil){
														[data->session.osObj release];
														data->session.osObj = nil;
													}
													data->session.osObj = captureSession;
													captureSession = nil;
													videoInput = nil;
													
												}
											}
											if(videoInput != nil){
												[captureSession removeInput:videoInput];
											}
											if(photoOutput != nil){
												[photoOutput release];
												photoOutput = nil;
											}
										}
									}
									if(videoInput != nil){
										[videoInput release];
										videoInput = nil;
									}
								}
							}
						}
						if(captureSession != nil){
							[captureSession release];
							captureSession = nil;
						}
					}
				}
				//Start session
				if(data->session.osObj == nil){
					PRINTF_ERROR("VideoCapture, Capture session could not be started.\n");
					data->session.state = ENAVCaptureState_Stopped;
				} else {
					[data->session.osObj startRunning];
					PRINTF_INFO("VideoCapture, Capture session started.\n");
					data->session.state = ENAVCaptureState_Running;
				}
			}
			NBASSERT(data->session.state == ENAVCaptureState_Running || data->session.state == ENAVCaptureState_Stopped)
		}
	}
	//Release thread
	if(thread != NULL){
		NBThread_release(thread);
		NBMemory_free(thread);
		thread = NULL;
	}
	return r;
}

bool AUAppGlueIOSAVCapture::captureStart(void* pData, const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask){ //ENAVCaptureOutBit
	bool r = false;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	if(data->session.state == ENAVCaptureState_Stopped){
		@autoreleasepool {
			//Start in secondary thread
			STNBThread* t = NBMemory_allocType(STNBThread);
			NBThread_init(t);
			NBThread_setIsJoinable(t, FALSE);
			data->session.state		= ENAVCaptureState_Starting;
            data->session.focusRng  = focusRng;
			data->session.streamSz	= streamSz;
			data->session.photoSz	= photoSz;
			data->session.extraOutputsMask = extraOutputsMask;
			if(!NBThread_start(t, AUAppGlueIOSAVCapture_applyStateAndRelease_, data, NULL)){
				data->session.state = ENAVCaptureState_Stopped;
				NBThread_release(t);
				NBMemory_free(t);
				t = NULL;
			} else {
				r = TRUE;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSAVCapture::captureIsRuning(void* pData){
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	return (data->session.state == ENAVCaptureState_Running); //Must be "running" state only
}

void AUAppGlueIOSAVCapture::captureStop(void* pData){
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	//Stop session
	if(data->session.state == ENAVCaptureState_Running){
		@autoreleasepool {
			//Stop in secondary thread
			STNBThread* t = NBMemory_allocType(STNBThread);
			NBThread_init(t);
			NBThread_setIsJoinable(t, FALSE);
			data->session.state = ENAVCaptureState_Stopping;
			if(!NBThread_start(t, AUAppGlueIOSAVCapture_applyStateAndRelease_, data, NULL)){
				data->session.state = ENAVCaptureState_Running;
				NBThread_release(t);
				NBMemory_free(t);
				t = NULL;
			}
		}
	}
}

//

bool AUAppGlueIOSAVCapture::photoTrigger(void* pData){
	bool r = false;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	if(data->session.osObj != nil){
		@autoreleasepool {
			NBASSERT(data->photo.osOutput != nil)
			SI32 pxFormatType = 0;
			{
				NSArray<NSNumber*>* pixFormats = [data->photo.osOutput availablePhotoPixelFormatTypes];
				SI32 i; const SI32 count = (SI32)[pixFormats count];
				for(i = 0 ; i < count; i++){
					//kCVPixelFormatType
					const SI32 fmtId		= [[pixFormats objectAtIndex:i] intValue];
					const char* fmrChars	= (const char*)&fmtId;
					PRINTF_INFO("#%d) PhotoPixFmt: '%c%c%c%c'.\n", (i + 1), fmrChars[0], fmrChars[1], fmrChars[2], fmrChars[3]);
					if(fmtId != kCVPixelFormatType_32RGBA){
						if(fmtId == kCVPixelFormatType_32ARGB || fmtId == kCVPixelFormatType_32RGBA || fmtId == kCVPixelFormatType_32BGRA){
							pxFormatType = fmtId;
							PRINTF_INFO("#%d) Setted PhotoPixFmt to: '%c%c%c%c'.\n", (i + 1), fmrChars[0], fmrChars[1], fmrChars[2], fmrChars[3]);
						}
					}
				}
				if(i == 0){
					PRINTF_INFO("0 PhotoPixFmts.\n");
				}
			}
			{
				NSArray<NSNumber*>* pixFormats = [data->photo.osOutput availableRawPhotoPixelFormatTypes];
				SI32 i; const SI32 count = (SI32)[pixFormats count];
				for(i = 0 ; i < count; i++){
					//kCVPixelFormatType
					const UI32 fmtId		= [[pixFormats objectAtIndex:i] intValue];
					const char* fmrChars	= (const char*)&fmtId;
					PRINTF_INFO("#%d) RawPhotoPixFmt: '%c%c%c%c'.\n", (i + 1), fmrChars[0], fmrChars[1], fmrChars[2], fmrChars[3]);
				}
				if(i == 0){
					PRINTF_INFO("0 RawPhotoPixFmts.\n");
				}
			}
			{
				NSArray<AVVideoCodecType>* codecs = [data->photo.osOutput availablePhotoCodecTypes];
				SI32 i; const SI32 count = (SI32)[codecs count];
				for(i = 0 ; i < count; i++){
					AVVideoCodecType codec	= [codecs objectAtIndex: i];
					PRINTF_INFO("#%d) PhotoCodecTypes: '%s'.\n", (i + 1), [codec UTF8String]);
				}
				if(i == 0){
					PRINTF_INFO("0 PhotoCodecTypes.\n");
				}
			}
			//
			if(pxFormatType == 0){
				PRINTF_ERROR("No supported 'PhotoPixFmt' available.\n");
			} else {
				NSDictionary<NSString *,id>* settingParams = [NSDictionary<NSString *,id> dictionaryWithObjectsAndKeys:
															  [NSNumber numberWithInt:pxFormatType], kCVPixelBufferPixelFormatTypeKey,
															  nil];
				AVCapturePhotoSettings* settings = [AVCapturePhotoSettings photoSettingsWithFormat: settingParams];
				/*
				AVCapturePhotoSettings* settings = [[AVCapturePhotoSettings alloc] init];
				[settings setValue:[NSNumber numberWithInt:pxFormatType] forKey:kCVPixelBufferPixelFormatTypeKey];
				//[settings setFlashMode:AVCaptureFlashModeOff];
				//[settings setDepthDataFiltered:NO];
				//[settings setLivePhotoVideoCodecType:AVVideoCodecTypeJPEG];
				//[settings setDepthDataDeliveryEnabled:NO];
				*/
				[settings setHighResolutionPhotoEnabled:YES];
				{
					AUAppPhotoCaptureDelegate* delegate = [AUAppPhotoCaptureDelegate sharedInstance: data];
					[data->photo.osOutput capturePhotoWithSettings:settings delegate:delegate];
					r = true;
				}
				//[settings release];
				//settings = nil;
			}
		}
	}
	return r;
}

void* AUAppGlueIOSAVCapture::photoSampleRetain(void* pData, UI64* frameIdFilterAndDst){
	void* r = NULL;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	NBHILO_MUTEX_ACTIVAR(&data->photo.mutex)
	if(data->photo.captured.use > 0){
		@autoreleasepool {
			UI64 filter = 0; if(frameIdFilterAndDst != NULL) filter = *frameIdFilterAndDst;
			SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
				AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
				if(filter < s->sequential){
					if(AUAVCaptureSample_pushConsumer(s)){
						if(frameIdFilterAndDst != NULL) *frameIdFilterAndDst = s->sequential;
						r = s;
						break;
					}
				}
			}
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&data->photo.mutex)
	return r;
}

void AUAppGlueIOSAVCapture::photoSampleRelease(void* pData, void* ptr){
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	NBHILO_MUTEX_ACTIVAR(&data->photo.mutex)
	NBASSERT(data->photo.captured.use > 0)
	if(data->photo.captured.use > 0){
		@autoreleasepool {
			BOOL found = FALSE;
			SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
				AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
				if(s == ptr){
					if(AUAVCaptureSample_popConsumer(s)){
						AUAVCaptureSample_release(s);
						NBMemory_free(s);
						s = NULL;
						NBArray_removeItemAtIndex(&data->photo.captured, i);
					}
					found = TRUE;
					break;
				}
			}
			NBASSERT(found)
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&data->photo.mutex)
}

const void* AUAppGlueIOSAVCapture::photoSamplePixData(void* pData, void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended){
	const void* r = NULL;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	NBHILO_MUTEX_ACTIVAR(&data->photo.mutex)
	NBASSERT(data->photo.captured.use > 0)
	if(data->photo.captured.use > 0){
		@autoreleasepool {
			BOOL found = FALSE;
			SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
				AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
				if(s == ptr){
					if(dstDegRotFromIntended != NULL) *dstDegRotFromIntended = s->rotDeg;
					r = AUAVCaptureSample_getLockedPixData((AUAVCaptureSample*)ptr, dstProps);
					found = TRUE;
					break;
				}
			}
			NBASSERT(found)
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&data->photo.mutex)
	return r;
}

//

void* AUAppGlueIOSAVCapture::videoSampleRetain(void* pData, UI64* frameIdFilterAndDst){
	void* r = NULL;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	AUAVCaptureVideoBuffer* buff = &data->video.buffer;
	//Determine and lock the source
	@autoreleasepool {
		UI64 frameIdFilter = 0; if(frameIdFilterAndDst != NULL) frameIdFilter = *frameIdFilterAndDst;
		NBHILO_MUTEX_ACTIVAR(&buff->mutex)
		{
			AUAVCaptureSample* itm = &buff->samples[buff->readIdx];
			if(itm->pixBuffer != nil){
				if(frameIdFilter < itm->sequential){ //Only return newer frames
					if(AUAVCaptureSample_pushConsumer(itm)){
						if(frameIdFilterAndDst != NULL) *frameIdFilterAndDst = itm->sequential;
						r = itm;
					}
				}
			}
		}
		NBHILO_MUTEX_DESACTIVAR(&buff->mutex)
	}
	return r;
}

void AUAppGlueIOSAVCapture::videoSampleRelease(void* pData, void* ptr){
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	AUAVCaptureVideoBuffer* buff = &data->video.buffer;
	//Determine, release and unlock the source (it match)
	@autoreleasepool {
		NBHILO_MUTEX_ACTIVAR(&buff->mutex)
		{
			AUAVCaptureSample* itm = (AUAVCaptureSample*)ptr;
			NBASSERT(ptr == &buff->samples[buff->readIdx])
			if(AUAVCaptureSample_popConsumer(itm)){
				buff->readIdx = (buff->readIdx == 0 ? 1 : 0);
				//Move writeIdx to next (if filled)
				if(buff->writeIdx == buff->readIdx){
					AUAVCaptureSample* itm = &buff->samples[buff->writeIdx];
					NB_IOS_AVC_ITM_MUTEX_ACTIVATE(itm)
					if(itm->pixBuffer != nil){
						buff->writeIdx = (buff->writeIdx == 0 ? 1 : 0);
					}
					NB_IOS_AVC_ITM_MUTEX_DEACTIVATE(itm)
				}
			}
		}
		NBHILO_MUTEX_DESACTIVAR(&buff->mutex)
	}
}

const void* AUAppGlueIOSAVCapture::videoSamplePixData(void* data, void* ptr, STNBBitmapProps* dstProps){
	return AUAVCaptureSample_getLockedPixData((AUAVCaptureSample*)ptr, dstProps);
}

//

BOOL AUAppGlueIOSAVCapture::metaQRCode(void* pData, STNBString* dst, const UI64 secsLastValid){
	BOOL r = 0;
	AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
	@autoreleasepool {
		NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
		if(data->metadata.captured.length > 0){
			if((data->metadata.capturedTime + secsLastValid) >= NBDatetime_getCurUTCTimestamp()){ //Captured in the current second
				NBString_set(dst, data->metadata.captured.str);
				r = TRUE;
			}
		}
		NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
	}
	return r;
}

//focus

BOOL AUAppGlueIOSAVCapture::isFocusModeSupported(void* pData, const ENAVCaptureFocusMode focusMode){
    BOOL r = FALSE;
    AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
    @autoreleasepool {
        NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
        if(data->session.captureDevice != nil){
            switch (focusMode) {
                case ENAVCaptureFocusMode_Locked:
                    r = [data->session.captureDevice isFocusModeSupported:AVCaptureFocusModeLocked];
                    break;
                case ENAVCaptureFocusMode_AutoFocusOnce:
                    r = [data->session.captureDevice isFocusModeSupported:AVCaptureFocusModeAutoFocus];
                    break;
                case ENAVCaptureFocusMode_AutoFocusContinous:
                    r = [data->session.captureDevice isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus];
                    break;
                default:
                    r = FALSE; //unsupported focus
                    break;
            }
        }
        NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
    }
    return r;
}

ENAVCaptureFocusMode AUAppGlueIOSAVCapture::getFocusMode(void* pData){
    ENAVCaptureFocusMode r = ENAVCaptureFocusMode_Count;
    AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
    @autoreleasepool {
        NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
        if(data->session.captureDevice != nil){
            AVCaptureFocusMode focusMode = data->session.captureDevice.focusMode;
            switch (focusMode) {
                case AVCaptureFocusModeLocked:
                    r = ENAVCaptureFocusMode_Locked;
                    break;
                case AVCaptureFocusModeAutoFocus:
                    r = ENAVCaptureFocusMode_AutoFocusOnce;
                    break;
                case AVCaptureFocusModeContinuousAutoFocus:
                    r = ENAVCaptureFocusMode_AutoFocusContinous;
                    break;
                default:
                    r = ENAVCaptureFocusMode_Count;
                    break;
            }
        }
        NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
    }
    return r;
}

BOOL AUAppGlueIOSAVCapture::setFocusMode(void* pData, const ENAVCaptureFocusMode focusMode){
    BOOL r = FALSE;
    AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
    @autoreleasepool {
        NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
        if(data->session.captureDevice != nil){
            NSError *error = nil;
            switch (focusMode) {
                case ENAVCaptureFocusMode_Locked:
                    if(![data->session.captureDevice lockForConfiguration:&error]){
                        PRINTF_ERROR("VideoCapture, Capture device could not be locked for configuration: '%s'.\n", [[error description] UTF8String]);
                    } else {
                        [data->session.captureDevice setFocusMode:AVCaptureFocusModeLocked];
                        [data->session.captureDevice unlockForConfiguration];
                        r = TRUE;
                    }
                    break;
                case ENAVCaptureFocusMode_AutoFocusOnce:
                    if(![data->session.captureDevice lockForConfiguration:&error]){
                        PRINTF_ERROR("VideoCapture, Capture device could not be locked for configuration: '%s'.\n", [[error description] UTF8String]);
                    } else {
                        [data->session.captureDevice setFocusMode:AVCaptureFocusModeAutoFocus];
                        [data->session.captureDevice unlockForConfiguration];
                        r = TRUE;
                    }
                    break;
                case ENAVCaptureFocusMode_AutoFocusContinous:
                    if(![data->session.captureDevice lockForConfiguration:&error]){
                        PRINTF_ERROR("VideoCapture, Capture device could not be locked for configuration: '%s'.\n", [[error description] UTF8String]);
                    } else {
                        [data->session.captureDevice setFocusMode:AVCaptureFocusModeContinuousAutoFocus];
                        [data->session.captureDevice unlockForConfiguration];
                        r = TRUE;
                    }
                    break;
                default:
                    r = FALSE; //unsupported focus
                    break;
            }
        }
        NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
    }
    return r;
}

BOOL AUAppGlueIOSAVCapture::isAdjustingFocus(void* pData){
    BOOL r = FALSE;
    AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
    @autoreleasepool {
        NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
        if(data->session.captureDevice != nil){
            r = [data->session.captureDevice isAdjustingFocus];
        }
        NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
    }
    return r;
}

float AUAppGlueIOSAVCapture::getLensPositionRel(void* pData){
    float r = 0.0f;
    AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
    @autoreleasepool {
        NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
        if(data->session.captureDevice != nil){
            r = data->session.captureDevice.lensPosition;
        }
        NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
    }
    return r;
}

BOOL AUAppGlueIOSAVCapture::lockLensToPositionRel(void* pData, const float relPos){
    BOOL r = FALSE;
    AUAppGlueIOSAVCaptureData* data = (AUAppGlueIOSAVCaptureData*)pData;
    if(relPos >= 0.0f && relPos <= 1.0f){
        @autoreleasepool {
            NBHILO_MUTEX_ACTIVAR(&data->metadata.mutex)
            if(data->session.captureDevice != nil){
                NSError *error = nil;
                if(![data->session.captureDevice lockForConfiguration:&error]){
                    PRINTF_ERROR("VideoCapture, Capture device could not be locked for configuration: '%s'.\n", [[error description] UTF8String]);
                } else {
                    [data->session.captureDevice setFocusModeLockedWithLensPosition:relPos completionHandler:nil];
                    [data->session.captureDevice unlockForConfiguration];
                    r = TRUE;
                }
            }
            NBHILO_MUTEX_DESACTIVAR(&data->metadata.mutex)
        }
    }
    return r;
}
