//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrNotifs.h"
#include "AUAppGlueAndroidAVCapture.h"
//
#include "nb/core/NBThread.h"
#include "nb/core/NBThreadMutex.h"
#include "nb/core/NBThreadCond.h"

#ifdef __ANDROID__
//is android
#endif

//QRCode reader
#include "../../../../sys-nbframework/lib-nbframework-src/src/ext/quirc/quirc.h"

//

#define AVCAPTURE_PERM_ID			"android.permission.CAMERA"
#define AVCAPTURE_PREF_NAME			"NB_AVCAPTURE"
#define AVCAPTURE_PREF_DATA_NAME	"android.permission.CAMERA.requested"

//

struct AUAppGlueAndroidAVCaptureData_;

typedef struct AUAVCapturePlane_ {
	const BYTE*	buffer;
	SI32	pxStride;
	SI32	rwStride;
} AUAVCapturePlane;

typedef enum ENAVCaptureSampleState_ {
	ENAVCaptureSampleState_Empty = 0,	//No data
	ENAVCaptureSampleState_Filling,		//Locked for filling
	ENAVCaptureSampleState_Filled		//Filled
} ENAVCaptureSampleState;

typedef enum ENAVCaptureSampleColor_ {
	ENAVCaptureSampleColor_RGB_565		= 0x00000004,	//RGB_565 (API 8)
	ENAVCaptureSampleColor_YUV_420_888	= 0x00000023,	//YUV_420_888 (API 19)
	ENAVCaptureSampleColor_FLEX_RGB_888	= 0x00000029,	//FLEX_RGB_888 (API 23)
} ENAVCaptureSampleColor;

typedef struct AUAVCaptureSample_ {
	//State
	ENAVCaptureSampleState	state;			//Sample state
	SI32					consumersCount;	//Ammout of consumers that locked this sample
	//Data
	UI64					sequential;		//Sample idx; order received
	SI32					rotDeg;			//the encoded image is rotated from intended image
	STNBBitmap				pixBuffer;
} AUAVCaptureSample;

//Buffer
typedef struct AUAVCaptureVideoBuffer_ {
	//Stats
	struct {
		UI64		seq;		//Current sequence
		UI64		readed;		//Total count of samples consumed
		UI64		written;	//Total count of samples written
		UI64		ignored;	//Total count of sampled ignored (buffers were full)
		UI64		errors;		//Total count of sampled could not be written
	} stats;
	//Samples
	struct {
		STNBArray	arr;	//AUAVCaptureSample*
	} samples;
	//Mutex
	STNBThreadMutex	mutex;
} AUAVCaptureVideoBuffer;

//

void AUAVCaptureSample_init(AUAVCaptureSample* obj);
void AUAVCaptureSample_release(AUAVCaptureSample* obj);
void AUAVCaptureSample_setPixelBuffer(AUAVCaptureSample* obj, const UI64 sampleSeq, const SI32 format, const STNBSizeI size, const AUAVCapturePlane* planes, const SI32 planesSz, AUAVCaptureVideoBuffer* buffer, struct AUAppGlueAndroidAVCaptureData_ *qrParserParent);
BOOL AUAVCaptureSample_setBitmapData(STNBBitmap* bmp, const SI32 format, const STNBSizeI size, const AUAVCapturePlane* planes, const SI32 planesSz);
//Consume
BOOL AUAVCaptureSample_consumerPush(AUAVCaptureSample* obj, const UI64 ifSeqIsGreatherThan);
const void* AUAVCaptureSample_consumerGetPixData(AUAVCaptureSample* obj, STNBBitmapProps* dstProps);
void AUAVCaptureSample_consumerPop(AUAVCaptureSample* obj);
//Size
STNBSizeI AUAVCaptureSample_sizeValues(const ENAVCaptureSize size);

//

class AUAppGlueAndroidAVCaptureListener;

typedef struct AUAppGlueAndroidAVCaptureData_ {
	AUAppI* app;
	//
	AUAppGlueAndroidAVCaptureListener* listener;
	BOOL	requestingAuth;
	//Camera
	struct {
		jobject		osObj;		//CameraDevice
	} input;
	//Session
	struct {
		jobject				osObj;		//CameraCaptureSession
		ENAVCaptureState	state;
		ENAVCaptureSize		streamSz;
		ENAVCaptureSize		photoSz;
		UI32				extraOutputsMask; //ENAVCaptureOutBit
	} session;
	//Video output
	struct {
		AUAVCaptureVideoBuffer	buffer;
		jobject					osOutput;	//ImageReader
	} video;
	//Photo output
	struct {
		STNBThreadMutex			mutex;
		STNBArray				captured; //STNBBitmap*
		UI64					seq;
		UI64					samplesWritten;
		jobject					osOutput;
	} photo;
	//Metadata
	struct {
		//Thread
		struct {
			BOOL			isRunning;
			BOOL			exitSignal;
			AUAVCaptureVideoBuffer*	sampleBuffCur;	//Current sample buffer analyzing
			AUAVCaptureSample*		sampleCur;			//Current sample analyzing
		} thread;
		//Data
		UI64				capturedTime;
		STNBString			captured;
		UI64				seq;
		UI64				samplesWritten;
		//
		STNBSizeI			parserSz;
		struct quirc*		parser;
		//
		STNBThreadMutex		mutex;
		STNBThreadCond		cond;
	} metadata;
	//
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	UI32			_dbgAccumWrite;
	UI32			_dbgAccumMissd;
	UI32			_dbgAccumError;
	UI64			_dbgCicleStarted;
	STNBThreadMutex	_dbgMutex;
#	endif
} AUAppGlueAndroidAVCaptureData;

class AUAppGlueAndroidAVCaptureListener: public AUAppReqPermResultListener {
	public:
		AUAppGlueAndroidAVCaptureListener(AUAppGlueAndroidAVCaptureData* data){
			_data = data;
		}
		virtual ~AUAppGlueAndroidAVCaptureListener(){
			_data = NULL;
		}
		//AUAppReqPermResultListener
		void appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* data /*jintArray*/);
	private:
		AUAppGlueAndroidAVCaptureData* _data;
};

void AUAVCaptureSample_init(AUAVCaptureSample* s){
	//State
	s->state			= ENAVCaptureSampleState_Empty;
	s->consumersCount	= 0;
	//Data
	s->sequential		= 0;
	s->rotDeg			= 0;
	NBBitmap_init(&s->pixBuffer);
}

void AUAVCaptureSample_release(AUAVCaptureSample* s){
	//State
	NBASSERT(s->state != ENAVCaptureSampleState_Filling) //Not filling
	NBASSERT(s->consumersCount <= 0) //Not in use
	//Data
	s->sequential	= 0;
	s->rotDeg		= 0;
	NBBitmap_release(&s->pixBuffer);
}

BOOL AUAVCaptureSample_setBitmapData(STNBBitmap* bmp, const SI32 format, const STNBSizeI size, const AUAVCapturePlane* planes, const SI32 planesSz){
	BOOL r = FALSE;
	//Resize bitmap (if necesary)
	{
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp);
		if(bmpProps.size.width != size.width || bmpProps.size.height != size.height){
			NBBitmap_create(bmp, size.width, size.height, ENNBBitmapColor_RGB8); //RGB (in android is faster than RGBA or BGRA)
			PRINTF_INFO("AVCapture, bitmap resized from (%d, %d) to (%d, %d)...\n", bmpProps.size.width, bmpProps.size.height, size.width, size.height);
		}
	}
	//Copy pixels
	{
		const STNBBitmapProps bmpProps = NBBitmap_getProps(bmp); NBASSERT(bmpProps.size.width == size.width || bmpProps.size.height == size.height)
		BYTE* pixData = (BYTE*)NBBitmap_getData(bmp);
		switch (bmpProps.color) {
			case ENNBBitmapColor_RGB8:
				/*if(format == ENAVCaptureSampleColor_RGB_565){
					PRINTF_INFO("AVCapture, RGB_565 planesSz(%d).\n", planesSz);
				} else*/
				if(format == ENAVCaptureSampleColor_FLEX_RGB_888){
					//Copy up to 3 planes
					SI32 i, x, y; const SI32 planesSzz = (planesSz > 3 ? 3 : planesSz);
					for(i = 0; i < planesSzz; i++){
						const AUAVCapturePlane* p = &planes[i];
						for(y = 0 ; y < size.height; y++){
							const BYTE* srcLn = &p->buffer[y * p->rwStride];
							BYTE* dstPx = &pixData[(y * bmpProps.bytesPerLine) + i]; //"+ i" to align to component
							for(x = 0 ; x < size.width; x++){
								*dstPx = srcLn[x];
								dstPx += 3;
							}
						}
					}
					r = TRUE;
				} else if(format == ENAVCaptureSampleColor_YUV_420_888){
					//35=YUV_420_888 (API 19) or 41=FLEX_RGB_888 (API 23)
					//Optimization notes: the planes UV are packed together.
					const AUAVCapturePlane* pY = &planes[0];
					const AUAVCapturePlane* pU = &planes[1];
					const AUAVCapturePlane* pV = &planes[2];
					//PRINTF_INFO("AVCapture, pxStride y(%d)u(%d)v(%d).\n", pY->pxStride, pU->pxStride, pV->pxStride);
					/*{
						SI64 deltaYU = ((SI64)pU->buffer - (SI64)pY->buffer);
						SI64 deltaUV = ((SI64)pV->buffer - (SI64)pU->buffer);
						if(deltaYU < 0) deltaYU = -deltaYU;
						if(deltaUV < 0) deltaUV = -deltaUV;
						if(deltaYU < pY->rwStride && deltaUV < pV->rwStride){
							PRINTF_INFO("AVCapture, all YUV planes are packed together; deltaYU(%llu) deltaUV(%llu).\n", deltaYU, deltaUV);
						} else if(deltaYU < pY->rwStride){
							PRINTF_INFO("AVCapture, all YU planes are packed together, V separated; deltaYU(%llu) deltaUV(%llu).\n", deltaYU, deltaUV);
						} else if(deltaUV < pV->rwStride){
							PRINTF_INFO("AVCapture, all UV planes are packed together, Y separated; deltaYU(%llu) deltaUV(%llu).\n", deltaYU, deltaUV);
						} else {
							PRINTF_INFO("AVCapture, all YUV planes are packed separated; deltaYU(%llu) deltaUV(%llu).\n", deltaYU, deltaUV);
						}
					}*/
					//Original
					if(!(pY->rwStride == 1 && pU->rwStride == 2 && pV->rwStride == 2)){
						SI32 y, x, C, D, E, R, G, B;
						for(y = 0 ; y < size.height; y++){
							const BYTE* srcLnY = &pY->buffer[y * pY->rwStride];
							const BYTE* srcLnU = &pU->buffer[(y / 2) * pU->rwStride]; //UV downsampling by 2
							const BYTE* srcLnV = &pV->buffer[(y / 2) * pV->rwStride]; //UV downsampling by 2
							BYTE* dstLn = &pixData[y * bmpProps.bytesPerLine];
							BYTE* dstPx = dstLn;
							for(x = 0 ; x < size.width; x++){
								//RGB (in android is faster than RGBA or BGRA)
								C = (SI32)srcLnY[x * pY->pxStride] - 16;
								D = (SI32)srcLnU[(x / 2) * pU->pxStride] - 128; //UV downsampling by 2
								E = (SI32)srcLnV[(x / 2) * pV->pxStride] - 128; //UV downsampling by 2
								//
								R = (( 298 * C           + 409 * E + 128) >> 8);
								G = (( 298 * C - 100 * D - 208 * E + 128) >> 8);
								B = (( 298 * C + 516 * D           + 128) >> 8);
								//
								*(dstPx + 0) = (R < 0 ? 0 : R > 255 ? 255 : R);
								*(dstPx + 1) = (G < 0 ? 0 : G > 255 ? 255 : G);
								*(dstPx + 2) = (B < 0 ? 0 : B > 255 ? 255 : B);
								//
								dstPx	+= 3;
							}
						}
					} else {
						//Optimization (avoid x-cicle)
						/*SI32 y, x, C, D, E, R, G, B; BOOL isPair;
						for(y = 0 ; y < size.height; y++){
							const BYTE* srcLnY = &pY->buffer[y * pY->rwStride];
							const BYTE* srcLnU = &pU->buffer[(y / 2) * pU->rwStride]; //UV downsampling by 2
							const BYTE* srcLnV = &pV->buffer[(y / 2) * pV->rwStride]; //UV downsampling by 2
							BYTE* dstLn = &pixData[y * bmpProps.bytesPerLine];
							BYTE* dstPx = dstLn;
							const BYTE* dstPxAfterEnd = (dstPx + bmpProps.bytesPerLine);
							isPair = TRUE;
							while(dstPx < dstPxAfterEnd){
								//RGB (in android is faster than RGBA or BGRA)
								C = (SI32)(*(srcLnY++)) - 16;
								D = (SI32)(*(srcLnU)) - 128; //UV downsampling by 2
								E = (SI32)(*(srcLnV)) - 128; //UV downsampling by 2
								//
								R = (( (298 * C)             + (409 * E) + 128) >> 8);
								G = (( (298 * C) - (100 * D) - (208 * E) + 128) >> 8);
								B = (( (298 * C) + (516 * D)           + 128) >> 8);
								//
								dstPx[0] = (R < 0 ? 0 : R > 255 ? 255 : R);
								dstPx[1] = (G < 0 ? 0 : G > 255 ? 255 : G);
								dstPx[2] = (B < 0 ? 0 : B > 255 ? 255 : B);
								//
								dstPx += 3;
								if((isPair = !isPair)){
									srcLnU += 2;
									srcLnV += 2;
								}
							}
						}*/
						//Testing floating point method
						/*SI32 y, x, yy, uu, vv, rr, gg, bb;
						for(y = 0; y < size.height; y++){
							const BYTE* srcLnY = &pY->buffer[y * pY->rwStride];
							const BYTE* srcLnU = &pU->buffer[(y / 2) * pU->rwStride]; //UV downsampling by 2
							const BYTE* srcLnV = &pV->buffer[(y / 2) * pV->rwStride]; //UV downsampling by 2
							BYTE* dstLn = &pixData[y * bmpProps.bytesPerLine];
							BYTE* dstPx = dstLn;
							const BYTE* dstPxAfterEnd = (dstPx + bmpProps.bytesPerLine);
							BOOL isPairX = TRUE;
							while(dstPx < dstPxAfterEnd){
								//RGB (in android is faster than RGBA or BGRA)
								yy	= (SI32)(*(srcLnY++));
								uu	= (SI32)(*(srcLnU)) - 128;
								vv	= (SI32)(*(srcLnV)) - 128;
								//
								rr = yy + (1.370705 * vv);
								gg = yy - (0.698001 * vv) - (0.337633 * uu);
								bb = yy                   + (1.732446 * uu);
								//
								dstPx[0] = (rr < 0 ? 0 : rr > 255 ? 255 : rr);
								dstPx[1] = (gg < 0 ? 0 : gg > 255 ? 255 : gg);
								dstPx[2] = (bb < 0 ? 0 : bb > 255 ? 255 : bb);
								dstPx	+= 3;
								//
								if((isPairX = !isPairX)){
									srcLnU += 2;
									srcLnV += 2;
								}
							}
						}*/
						//Optimization (avoid x-cicle, and integer formula)
						SI32 y, x, yy, uu, vv, rr, gg, bb; BOOL isPair;
						for(y = 0; y < size.height; y++){
							const BYTE* srcLnY = &pY->buffer[y * pY->rwStride];
							const BYTE* srcLnU = &pU->buffer[(y / 2) * pU->rwStride]; //UV downsampling by 2
							const BYTE* srcLnV = &pV->buffer[(y / 2) * pV->rwStride]; //UV downsampling by 2
							BYTE* dstLn = &pixData[y * bmpProps.bytesPerLine];
							BYTE* dstPx = dstLn;
							const BYTE* dstPxAfterEnd = (dstPx + bmpProps.bytesPerLine);
							BOOL isPairX = TRUE;
							while(dstPx < dstPxAfterEnd){
								//RGB (in android is faster than RGBA or BGRA)
								yy	= (SI32)(*(srcLnY++) * 128);
								uu	= (SI32)(*(srcLnU)) - 128;
								vv	= (SI32)(*(srcLnV)) - 128;
								//
								rr = yy + (174 * vv);
								gg = yy - (89 * vv) - (43 * uu);
								bb = yy                   + (221 * uu);
								//
								dstPx[0] = (rr < 0 ? 0 : rr > 32640 ? 255 : rr / 128);
								dstPx[1] = (gg < 0 ? 0 : gg > 32640 ? 255 : gg / 128);
								dstPx[2] = (bb < 0 ? 0 : bb > 32640 ? 255 : bb / 128);
								dstPx	+= 3;
								//
								if((isPairX = !isPairX)){
									srcLnU += 2;
									srcLnV += 2;
								}
							}
						}
					}
					r = TRUE;
				}
				break;
			case ENNBBitmapColor_GRIS8:
				if(planesSz > 0){
					const AUAVCapturePlane* p = &planes[0];
					SI32 y, x; for(y = 0 ; y < size.height; y++){
						const BYTE* srcLn = &p->buffer[y * p->rwStride];
						const BYTE* srcPx = srcLn;
						BYTE* dstLn = &pixData[y * bmpProps.bytesPerLine];
						BYTE* dstPx = dstLn;
						for(x = 0 ; x < size.width; x++){
							//RGB (in android is faster than RGBA or BGRA)
							*dstPx 	= *srcPx;
							dstPx++;
							srcPx	+= p->pxStride;
						}
					}
					r = TRUE;
				}
				break;
			default:
				break;
		}
	}
	return r;
}

void AUAVCaptureSample_setPixelBuffer(AUAVCaptureSample* itm, const UI64 sampleSeq, const SI32 format, const STNBSizeI size, const AUAVCapturePlane* planes, const SI32 planesSz, AUAVCaptureVideoBuffer* buffer, struct AUAppGlueAndroidAVCaptureData_ *qrParserParent){
	NBASSERT(itm != NULL)
	if(itm != NULL){
		BOOL isFilled = FALSE;
		NBASSERT(itm->state == ENAVCaptureSampleState_Filling) //Must be filling
		NBASSERT(itm->consumersCount == 0) //Must not beign retained
		//Procesa data
		if(planesSz != 3){ //Expecting 35=YUV_420_888 (API 19) or 41=FLEX_RGB_888 (API 23)
			PRINTF_ERROR("AVCapture, expeected 3 planes YUV.\n");
		} else {
			STNBBitmap* bmp = &itm->pixBuffer;
			//Copy and analyze data
			if(!AUAVCaptureSample_setBitmapData(bmp, format, size, planes, planesSz)){
				PRINTF_ERROR("AVCapture, could not copy data from YUV-video-preview to bitmap.\n");
			} else {
				isFilled = TRUE;
			}
		}
		itm->sequential	= sampleSeq;
		itm->state		= (isFilled ? ENAVCaptureSampleState_Filled : ENAVCaptureSampleState_Empty);
		//Activate QRParser
		if(isFilled && planesSz > 0 && buffer != NULL && qrParserParent != NULL){
			NBThreadMutex_lock(&qrParserParent->metadata.mutex);
			if(qrParserParent->metadata.thread.isRunning && qrParserParent->metadata.parser != NULL){
				if(qrParserParent->metadata.thread.sampleBuffCur == NULL && qrParserParent->metadata.thread.sampleCur == NULL){
					//Set
					NBThreadMutex_lock(&buffer->mutex);
					{
						//Resize parser (if necesary)
						if(qrParserParent->metadata.parserSz.width != size.width || qrParserParent->metadata.parserSz.height != size.height){
							quirc_resize(qrParserParent->metadata.parser, size.width, size.height);
							qrParserParent->metadata.parserSz.width = size.width;
							qrParserParent->metadata.parserSz.height = size.height;
							PRINTF_INFO("AVCapture, qrParser resized from (%d, %d) to (%d, %d)...\n", qrParserParent->metadata.parserSz.width, qrParserParent->metadata.parserSz.height, size.width, size.height);
						}
						//Feed parser bitmap
						{
							uint8_t* qrBuff; int w, h;
							qrBuff = quirc_begin(qrParserParent->metadata.parser, &w, &h);
							NBASSERT(w == size.width && h == size.height)
							{
								const AUAVCapturePlane* p = &planes[0];
								if(p->pxStride == 1 && p->rwStride == size.width){
									//Quick copy of content
									NBMemory_copy(qrBuff, p->buffer, (size.width * size.height));
								} else {
									//Copy byte per byte
									SI32 y, x; for(y = 0 ; y < size.height; y++){
										const BYTE* srcLn = &p->buffer[y * p->rwStride];
										const BYTE* srcPx = srcLn;
										for(x = 0 ; x < size.width; x++){
											//RGB (in android is faster than RGBA or BGRA)
											*qrBuff = *srcPx;
											qrBuff++;
											srcPx += p->pxStride;
										}
									}
								}
							}
						}
						//Save to process in secondary thread
						qrParserParent->metadata.thread.sampleBuffCur	= buffer;
						qrParserParent->metadata.thread.sampleCur		= itm;
						NBASSERT(itm->consumersCount >= 0)
						itm->consumersCount++;
					}
					NBThreadMutex_unlock(&buffer->mutex);
					//PRINTF_INFO("AVCapture, processing QRCode.\n");
					NBThreadCond_broadcast(&qrParserParent->metadata.cond);
				} else {
					//PRINTF_INFO("AVCapture, waiting for QRCode parser.\n");
				}
			}
			NBThreadMutex_unlock(&qrParserParent->metadata.mutex);
		}
	}
}

BOOL AUAVCaptureSample_consumerPush(AUAVCaptureSample* itm, const UI64 ifSeqIsGreatherThan){
	BOOL r = FALSE;
	if(itm->state == ENAVCaptureSampleState_Filled && ifSeqIsGreatherThan < itm->sequential){
		NBASSERT(itm->consumersCount >= 0)
		itm->consumersCount++;
		r = TRUE;
	}
	return r;
}

void AUAVCaptureSample_consumerPop(AUAVCaptureSample* itm){
	NBASSERT(itm->state == ENAVCaptureSampleState_Filled)
	NBASSERT(itm->consumersCount > 0)
	itm->consumersCount--;
}

const void* AUAVCaptureSample_consumerGetPixData(AUAVCaptureSample* itm, STNBBitmapProps* dstProps){
	const void* r = NULL;
	STNBBitmapProps props;
	NBMemory_setZeroSt(props, STNBBitmapProps);
	if(itm != NULL){
		NBASSERT(itm->state == ENAVCaptureSampleState_Filled) //Must be filled
		NBASSERT(itm->consumersCount > 0) //Must be retained
		const STNBBitmapProps props = NBBitmap_getProps(&itm->pixBuffer);
		const BYTE* pixData = NBBitmap_getData(&itm->pixBuffer);
		if(dstProps != NULL){
			*dstProps = props;
		}
		r = pixData;
	}
	return r;
}

STNBSizeI AUAVCaptureSample_sizeValues(const ENAVCaptureSize size){
	STNBSizeI r;
	switch(size) {
		case ENAVCaptureSize_Low192:	r.width = 192; r.height = 144; break;
		case ENAVCaptureSize_Low352:	r.width = 352; r.height = 288; break;
		case ENAVCaptureSize_Med640:	r.width = 640; r.height = 480; break;
		case ENAVCaptureSize_High1920:	r.width = 1920; r.height = 1080; break;
		case ENAVCaptureSize_High3840:	r.width = 3840; r.height = 2160; break;
		case ENAVCaptureSize_High4032:	r.width = 4032; r.height = 3024; break;
		default: r.width = 1280; r.height = 720; break; //ENAVCaptureSize_Med1280
	}
	return r;
}

//Calls

SI64 AUAppGlueAndroidAVCapture_runQRCodeThread_(STNBThread* t, void* param);

bool AUAppGlueAndroidAVCapture::create(AUAppI* app, STMngrAVCaptureCalls* obj){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)NBMemory_alloc(sizeof(AUAppGlueAndroidAVCaptureData));
	NBMemory_setZeroSt(*data, AUAppGlueAndroidAVCaptureData);
	NBMemory_setZeroSt(*obj, STMngrAVCaptureCalls);
	data->app					= (AUAppI*)app;
	data->requestingAuth		= FALSE;
	data->listener				= new AUAppGlueAndroidAVCaptureListener(data);
	data->app->addReqPermResultListener(data->listener);
	//Input camera
	{
		data->input.osObj		= NULL;
	}
	//capture session
	{
		data->session.osObj		= NULL;
		data->session.state		= ENAVCaptureState_Stopped;
		data->session.extraOutputsMask = 0; //ENAVCaptureOutBit
	}
	//Video preview
	{
		//Buffer
		{
			AUAVCaptureVideoBuffer* buff = &data->video.buffer;
			NBMemory_setZeroSt(*buff, AUAVCaptureVideoBuffer);
			NBArray_init(&buff->samples.arr, sizeof(AUAVCaptureSample*), NULL);
			{
				SI32 i; const SI32 count = 2;
				for(i = 0 ; i < count; i++){
					AUAVCaptureSample* s = NBMemory_allocType(AUAVCaptureSample);
					AUAVCaptureSample_init(s);
					NBArray_addValue(&buff->samples.arr, s);
				}
			}
			NBThreadMutex_init(&buff->mutex);
		}
		data->video.osOutput	= NULL;
	}
	//Photo output
	{
		NBThreadMutex_init(&data->photo.mutex);
		NBArray_init(&data->photo.captured, sizeof(AUAVCaptureSample*), NULL);
		data->photo.seq				= 0;
		data->photo.samplesWritten	= 0;
		data->photo.osOutput		= NULL;
	}
	//Metadata
	{
		data->metadata.capturedTime		= 0;
		NBString_init(&data->metadata.captured);
		data->metadata.samplesWritten	= 0;
		data->metadata.parser			= quirc_new();
		data->metadata.parserSz			= NBST_P(STNBSizeI, 0, 0 );
		//Thread
		{
			data->metadata.thread.isRunning		= FALSE;
			data->metadata.thread.exitSignal	= FALSE;
			data->metadata.thread.sampleBuffCur	= NULL;	//Current sample buffer analyzing
			data->metadata.thread.sampleCur		= NULL;	//Current sample analyzing
		}
		//
		NBThreadMutex_init(&data->metadata.mutex);
		NBThreadCond_init(&data->metadata.cond);
		//Start thread
		{
			STNBThread* thread = NBMemory_allocType(STNBThread);
			NBThread_init(thread);
			NBThread_setIsJoinable(thread, FALSE);
			data->metadata.thread.isRunning = TRUE;
			if(!NBThread_start(thread, AUAppGlueAndroidAVCapture_runQRCodeThread_, data, NULL)){
				data->metadata.thread.isRunning	= FALSE;
				NBThread_release(thread);
				NBMemory_free(thread);
				thread = NULL;
			}
		}
	}
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	{
		data->_dbgAccumWrite	= 0;
		data->_dbgAccumMissd	= 0;
		data->_dbgAccumError	= 0;
		data->_dbgCicleStarted	= 0;
		NBThreadMutex_init(&data->_dbgMutex);
	}
#	endif
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//
	obj->funcCaptureAuthStatus			= captureAuthStatus;
	obj->funcCaptureAuthStatusParam		= data;
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
	return true;
}

bool AUAppGlueAndroidAVCapture::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		//Stop
		while(data->session.state != ENAVCaptureState_Stopped){
			AUAppGlueAndroidAVCapture::captureStop(pData);
		}
		//Metadata
		{
			NBThreadMutex_lock(&data->metadata.mutex);
			{
				//Stop
				while(data->metadata.thread.isRunning){
					data->metadata.thread.exitSignal = TRUE;
					NBThreadCond_broadcast(&data->metadata.cond);
				}
				data->metadata.thread.sampleBuffCur	= NULL;	//Current sample buffer analyzing
				data->metadata.thread.sampleCur		= NULL;	//Current sample analyzing
				if(data->metadata.parser != NULL){
					data->metadata.parserSz = NBST_P(STNBSizeI, 0, 0 );
					quirc_destroy(data->metadata.parser);
					data->metadata.parser = NULL;
				}
				NBString_release(&data->metadata.captured);
				NBThreadCond_release(&data->metadata.cond);
			}
			NBThreadMutex_unlock(&data->metadata.mutex);
			NBThreadMutex_release(&data->metadata.mutex);
		}
		//Photo output
		{
			if(data->photo.osOutput != NULL){
				jEnv->DeleteGlobalRef(data->photo.osOutput);
				data->photo.osOutput = NULL;
			}
			{
				NBThreadMutex_lock(&data->photo.mutex);
				{
					SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
						AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
						AUAVCaptureSample_release(s);
						NBMemory_free(s);
					}
					NBArray_empty(&data->photo.captured);
					NBArray_release(&data->photo.captured);
				}
				NBThreadMutex_unlock(&data->photo.mutex);
				NBThreadMutex_release(&data->photo.mutex);
			}
		}
		//Video output
		{
			if(data->video.osOutput != NULL){
				jEnv->DeleteGlobalRef(data->video.osOutput);
				data->video.osOutput = NULL;
			}
			//
			{
				AUAVCaptureVideoBuffer* buff = &data->video.buffer;
				NBThreadMutex_lock(&buff->mutex);
				{
					SI32 i; for(i = 0 ; i < buff->samples.arr.use; i++){
						AUAVCaptureSample* s = NBArray_itmValueAtIndex(&buff->samples.arr, AUAVCaptureSample*, i);
						AUAVCaptureSample_release(s);
						NBMemory_free(s);
					}
					NBArray_empty(&buff->samples.arr);
					NBArray_release(&buff->samples.arr);
				}
				NBThreadMutex_unlock(&buff->mutex);
				NBThreadMutex_release(&buff->mutex);
			}
		}
		//Session
		{
			if(data->session.osObj != NULL){
				jEnv->DeleteGlobalRef(data->session.osObj);
				data->session.osObj = NULL;
			}
			data->session.state		= ENAVCaptureState_Stopped;
		}
		//Camera
		{
			if(data->input.osObj != NULL){
				jEnv->DeleteGlobalRef(data->input.osObj);
				data->input.osObj = NULL;
			}
		}
		//Debug
#		ifdef NB_CONFIG_INCLUDE_ASSERTS
		{
			NBThreadMutex_release(&data->_dbgMutex);
		}
#		endif
		if(data->listener != NULL){
			data->app->removeReqPermResultListener(data->listener);
			delete data->listener;
			data->listener = NULL;
		}
		data->app = NULL;
		NBMemory_free(pData);
		r = true;
	}
	return r;
}

//

SI64 AUAppGlueAndroidAVCapture_runQRCodeThread_(STNBThread* t, void* param){
	SI64 r = 0;
	if(param != NULL){
		AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)param;
		NBThreadMutex_lock(&data->metadata.mutex);
		{
			NBASSERT(data->metadata.thread.isRunning)
			while(!data->metadata.thread.exitSignal){
				if(data->metadata.thread.sampleBuffCur == NULL || data->metadata.thread.sampleCur == NULL){
					//Wait for data
					NBThreadCond_wait(&data->metadata.cond, &data->metadata.mutex);
				} else {
					//Process data (unlocked)
					NBThreadMutex_unlock(&data->metadata.mutex);
					{
						//Analyze
						{
							quirc_end(data->metadata.parser);
						}
						//Extract codes
						{
							int i, num_codes = quirc_count(data->metadata.parser);
							//if(num_codes > 0){
							//	PRINTF_INFO("AVCapture, quirc_count: %d\n", num_codes);
							//}
							for (i = 0; i < num_codes; i++) {
								struct quirc_code codeQR;
								struct quirc_data dataQR;
								quirc_decode_error_t err;
								quirc_extract(data->metadata.parser, i, &codeQR);
								//Decoding stage
								err = quirc_decode(&codeQR, &dataQR);
								if(err){
									//PRINTF_INFO("AVCapture, DECODE FAILED: %s\n", quirc_strerror(err));
								} else {
									data->metadata.capturedTime = NBDatetime_getCurUTCTimestamp();
									data->metadata.seq++;
									data->metadata.samplesWritten++;
									NBString_set(&data->metadata.captured, (const char*)dataQR.payload);
									//PRINTF_INFO("AVCapture, Data: %s\n", data.payload);
								}
							}
						}
						//Release sample
						{
							NBThreadMutex_lock(&data->metadata.thread.sampleBuffCur->mutex);
							{
								NBASSERT(data->metadata.thread.sampleCur->state == ENAVCaptureSampleState_Filled)
								NBASSERT(data->metadata.thread.sampleCur->consumersCount > 0)
								data->metadata.thread.sampleCur->consumersCount--;
							}
							NBThreadMutex_unlock(&data->metadata.thread.sampleBuffCur->mutex);
						}
						
					}
					NBThreadMutex_lock(&data->metadata.mutex);
					//Release metadata
					{
						data->metadata.thread.sampleBuffCur	= NULL;
						data->metadata.thread.sampleCur		= NULL;
						NBThreadCond_broadcast(&data->metadata.cond);
					}
				}
			}
			//End-of-run
			data->metadata.thread.isRunning = FALSE;
		}
		NBThreadMutex_unlock(&data->metadata.mutex);
	}
	//Release thread
	if(t != NULL){
		NBThread_release(t);
		NBMemory_free(t);
		t = NULL;
	}
	//
	return r;
}

//

ENAVCaptureAuthStatus AUAppGlueAndroidAVCapture::captureAuthStatus(void* pData, const BOOL requestIfNecesary){
	ENAVCaptureAuthStatus r = ENAVCaptureAuthStatus_Denied;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	if(data->requestingAuth){
		r = ENAVCaptureAuthStatus_Requesting;
	} else {
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		//Query permission
		if(AUAppGlueAndroidJNI::isPermissionGranted(jEnv, jContext, AVCAPTURE_PERM_ID)){
			r = ENAVCaptureAuthStatus_Authorized;
		} else if(AUAppGlueAndroidJNI::getAPICurrent(jEnv) < 23){
			//API 23 and before permissions were granted at install time (and cannot be requested)
			//PRINTF_INFO("AVCapture, no permission '%s' and cannot request it (API 22-or-less)...\n", AVCAPTURE_PERM_ID);
			r = ENAVCaptureAuthStatus_Denied;
		} else {
			//API 23+
			{
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(!AUAppGlueAndroidJNI::loadDataFromSharedPrefs(jEnv, jContext, AVCAPTURE_PREF_NAME, AVCAPTURE_PREF_DATA_NAME, str)){
					r = ENAVCaptureAuthStatus_NotDetermined;
				} else {
					if(str->tamano() > 0){
						r = ENAVCaptureAuthStatus_Denied;
					} else {
						r = ENAVCaptureAuthStatus_NotDetermined;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
			//Start request
			if(r == ENAVCaptureAuthStatus_NotDetermined && requestIfNecesary){
				PRINTF_INFO("AVCapture, starting permission request...\n");
				data->requestingAuth = TRUE;
				const char* permId = AVCAPTURE_PERM_ID;
				if(!AUAppGlueAndroidJNI::requestPermissions(jEnv, jContext, &permId, 1)){
					PRINTF_ERROR("AVCapture, ... could not start permission request.\n");
					data->requestingAuth = FALSE;
					r = ENAVCaptureAuthStatus_Denied;
				} else {
					PRINTF_INFO("AVCapture, ... started permission request.\n");
				}
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidAVCapture::captureStart(void* pData, const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask){ //ENAVCaptureOutBit
	bool r = false;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
	NBASSERT(ENAVCaptureAuthStatus_Authorized == AUAppGlueAndroidAVCapture::captureAuthStatus(pData, FALSE));
	if(data != NULL){
		if(data->session.state == ENAVCaptureState_Stopped){
			PRINTF_INFO("AUAppGlueAndroidAVCapture::captureStart (start).\n");
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jEnv != NULL){
				//API 21+
				jobject jCamSvr	= (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "CAMERA_SERVICE");
				if(jCamSvr != NULL){
					jclass clsHndlr	= jEnv->FindClass("android/os/Handler"); NBASSERT(clsHndlr != NULL)
					jclass clsLstnr	= jEnv->FindClass("com/auframework/AppNative$CameraStateListener"); NBASSERT(clsLstnr != NULL)
					jclass clsMngr	= jEnv->FindClass("android/hardware/camera2/CameraManager"); NBASSERT(clsMngr != NULL)
					if(clsHndlr != NULL && clsLstnr != NULL && clsMngr != NULL){
						jmethodID mInitHndlr = jEnv->GetMethodID(clsHndlr, "<init>", "()V"); NBASSERT(mInitHndlr != NULL)
						jmethodID mInitLstr	= jEnv->GetMethodID(clsLstnr, "<init>", "(J)V"); NBASSERT(mInitLstr != NULL)
						jmethodID mGetIds	= jEnv->GetMethodID(clsMngr, "getCameraIdList", "()[Ljava/lang/String;"); NBASSERT(mGetIds != NULL)
						jmethodID mOpenCam	= jEnv->GetMethodID(clsMngr, "openCamera", "(Ljava/lang/String;Landroid/hardware/camera2/CameraDevice$StateCallback;Landroid/os/Handler;)V"); NBASSERT(mOpenCam != NULL)
						if(mInitHndlr != NULL && mInitLstr != NULL && mGetIds != NULL && mOpenCam != NULL){
							STNBString camId;
							NBString_init(&camId);
							//Read cameras IDs
							jobjectArray jCamIds = (jobjectArray)jEnv->CallObjectMethod(jCamSvr, mGetIds); NBASSERT(jCamIds != NULL)
							if(jCamIds != NULL){
								SI32 i; const SI32 count = (SI32)jEnv->GetArrayLength(jCamIds);
								for(i = 0; i < count; i++){
									jobject jCamId = jEnv->GetObjectArrayElement(jCamIds, i);
									const char* utf8 = jEnv->GetStringUTFChars((jstring)jCamId, 0);
									if(i == 0){
										NBString_set(&camId, utf8);
									}
									PRINTF_INFO("AVCapture, CamId #%d / %d: '%s'.\n", (i + 1), count, utf8);
									jEnv->ReleaseStringUTFChars((jstring)jCamId, utf8);
								}
								if(i == 0) {
									PRINTF_INFO("AVCapture, zero cameras-ids.\n");
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jCamIds)
							}
							//Start
							if(camId.length > 0){
								jclass clsHndlr		= jEnv->FindClass("android/os/Handler"); NBASSERT(clsHndlr != NULL)
								jobject jCamsStatesLstnr = jEnv->NewObject(clsLstnr, mInitLstr, (jlong)data); NBASSERT(jCamsStatesLstnr != NULL)
								jobject jHandler	= (jobject)jniGlue->jHandler(); /*jEnv->NewObject(clsHndlr, mInitHndlr);*/ NBASSERT(jHandler != NULL)
								if(jCamsStatesLstnr != NULL && jHandler != NULL){
									jstring jCamId = jEnv->NewStringUTF(camId.str);
									{
										PRINTF_INFO("AVCapture, openCamera('%s').\n", camId.str);
										data->session.state		= ENAVCaptureState_Starting;
										data->session.streamSz	= streamSz;
										data->session.photoSz	= photoSz;
										data->session.extraOutputsMask = extraOutputsMask; //ENAVCaptureOutBit
										jEnv->CallVoidMethod(jCamSvr, mOpenCam, jCamId, jCamsStatesLstnr, jHandler);
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jCamId)
									r = true;
								}
							}
							NBString_release(&camId);
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsLstnr)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsMngr)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsHndlr)
					}
				}
			}
			PRINTF_INFO("AUAppGlueAndroidAVCapture::captureStart (end).\n");
		}
	}
	return r;
}

bool AUAppGlueAndroidAVCapture::captureIsRuning(void* pData){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	return (data->session.state == ENAVCaptureState_Running); //Must be "running" state only
}

void AUAppGlueAndroidAVCapture::captureStop(void* pData){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	PRINTF_INFO("AUAppGlueAndroidAVCapture::captureStop (start).\n");
	//Close camera
	if(data->session.state == ENAVCaptureState_Running){
		data->session.state = ENAVCaptureState_Stopping;
		NBASSERT(data->input.osObj != NULL)
		if(data->input.osObj != NULL){ //Camera
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			//Call close
			{
				jclass clsCam = jEnv->FindClass("android/hardware/camera2/CameraDevice"); NBASSERT(clsCam != NULL)
				if(clsCam != NULL){
					jmethodID mClose = jEnv->GetMethodID(clsCam, "close", "()V"); NBASSERT(mClose != NULL)
					if(mClose != NULL){
						jEnv->CallVoidMethod(data->input.osObj, mClose);
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsCam)
				}
			}
			//Release camera (session and others will be closed too)
			{
				if(data->input.osObj != NULL){
					jEnv->DeleteGlobalRef(data->input.osObj);
					data->input.osObj = NULL;
				}
			}
		}
	}
	PRINTF_INFO("AUAppGlueAndroidAVCapture::captureStop (end).\n");
}

//

bool AUAppGlueAndroidAVCapture::photoTrigger(void* pData){
	bool r = false;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	if(data->input.osObj != NULL && data->session.osObj != NULL){
		NBASSERT(data->photo.osOutput != NULL)
		if(data->photo.osOutput != NULL){
			/*CaptureRequest.Builder builder = camera.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
			 builder.addTarget(imageReader.getSurface());
			 obj = builder.build();
			 session.setRepeatingRequest(obj, null, null);
			 */
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			//Photo output
			jclass clsCam		= jEnv->FindClass("android/hardware/camera2/CameraDevice"); NBASSERT(clsCam != NULL)
			jclass clsSess		= jEnv->FindClass("android/hardware/camera2/CameraCaptureSession"); NBASSERT(clsSess != NULL)
			jclass clsBldr		= jEnv->FindClass("android/hardware/camera2/CaptureRequest$Builder"); NBASSERT(clsBldr != NULL)
			jclass clsImgRdr	= jEnv->FindClass("android/media/ImageReader"); NBASSERT(clsImgRdr != NULL)
			if(clsCam != NULL && clsSess != NULL && clsBldr != NULL && clsImgRdr != NULL){
				jmethodID mCreate	= jEnv->GetMethodID(clsCam, "createCaptureRequest", "(I)Landroid/hardware/camera2/CaptureRequest$Builder;"); NBASSERT(mCreate != NULL)
				jmethodID mGetSurf	= jEnv->GetMethodID(clsImgRdr, "getSurface", "()Landroid/view/Surface;"); NBASSERT(mGetSurf != NULL)
				jmethodID mAddTgt	= jEnv->GetMethodID(clsBldr, "addTarget", "(Landroid/view/Surface;)V"); NBASSERT(mAddTgt != NULL)
				jmethodID mBuild	= jEnv->GetMethodID(clsBldr, "build", "()Landroid/hardware/camera2/CaptureRequest;"); NBASSERT(mBuild != NULL)
				jmethodID mCapture	= jEnv->GetMethodID(clsSess, "capture", "(Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CameraCaptureSession$CaptureCallback;Landroid/os/Handler;)I"); NBASSERT(mCapture != NULL)
				if(mCreate != NULL && mGetSurf != NULL && mAddTgt != NULL && mBuild != NULL && mCapture != NULL){
					jobject jCamera = data->input.osObj; NBASSERT(jCamera != NULL)
					//Start Photo Capture
					{
						jobject jImgRdr = data->photo.osOutput; NBASSERT(jImgRdr != NULL)
						jobject jSurface = jEnv->CallObjectMethod(jImgRdr, mGetSurf); NBASSERT(jSurface != NULL)
						if(jSurface != NULL){
							//TEMPLATE_PREVIEW: Constant Value: 1 (0x00000001)
							//TEMPLATE_STILL_CAPTURE: Constant Value: 2 (0x00000002)
							jobject jBldr = jEnv->CallObjectMethod(jCamera, mCreate, 2 /*TEMPLATE_STILL_CAPTURE*/); NBASSERT(jBldr != NULL)
							if(jBldr != NULL){
								jEnv->CallVoidMethod(jBldr, mAddTgt, jSurface);
								jobject jCapReq = jEnv->CallObjectMethod(jBldr, mBuild); NBASSERT(jCapReq != NULL)
								if(jCapReq != NULL){
									jint uid = jEnv->CallIntMethod(data->session.osObj, mCapture, jCapReq, NULL, NULL);
									PRINTF_INFO("AVCapture, Photo Capture Started.\n");
									r = true;
								}
							}
						}
					}
				}
				//
				NBJNI_DELETE_REF_LOCAL(jEnv, clsImgRdr)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsBldr)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsSess)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsCam)
			}
		}
	}
	return r;
}

void* AUAppGlueAndroidAVCapture::photoSampleRetain(void* pData, UI64* frameIdFilterAndDst){
	void* r = NULL;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	UI64 filter = 0; if(frameIdFilterAndDst != NULL) filter = *frameIdFilterAndDst;
	{
		NBThreadMutex_lock(&data->photo.mutex);
		if(data->photo.captured.use > 0){
			SI32 i; for(i = ((SI32)data->photo.captured.use - 1); i >= 0; i--){
				AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
				if(AUAVCaptureSample_consumerPush(s, filter)){
					filter = s->sequential;
					r = s;
					break;
				}
			}
		}
		NBThreadMutex_unlock(&data->photo.mutex);
	}
	if(frameIdFilterAndDst != NULL) *frameIdFilterAndDst = filter;
	return r;
}

void AUAppGlueAndroidAVCapture::photoSampleRelease(void* pData, void* ptr){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	NBThreadMutex_lock(&data->photo.mutex);
	NBASSERT(data->photo.captured.use > 0)
	if(data->photo.captured.use > 0){
		BOOL found = FALSE;
		SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
			AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
			if(s == ptr){
				//Pop and release
				{
					AUAVCaptureSample_consumerPop(s);
					if(s->consumersCount <= 0){
						AUAVCaptureSample_release(s);
						NBMemory_free(s);
						s = NULL;
						NBArray_removeItemAtIndex(&data->photo.captured, i);
					}
				}
				found = TRUE;
				break;
			}
		}
		NBASSERT(found)
	}
	NBThreadMutex_unlock(&data->photo.mutex);
}

const void* AUAppGlueAndroidAVCapture::photoSamplePixData(void* pData, void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended){
	const void* r = NULL;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	NBThreadMutex_lock(&data->photo.mutex);
	NBASSERT(data->photo.captured.use > 0)
	if(data->photo.captured.use > 0){
		BOOL found = FALSE;
		SI32 i; for(i = 0 ; i < data->photo.captured.use; i++){
			AUAVCaptureSample* s = NBArray_itmValueAtIndex(&data->photo.captured, AUAVCaptureSample*, i);
			if(s == ptr){
				if(dstDegRotFromIntended != NULL) *dstDegRotFromIntended = s->rotDeg;
				r = AUAVCaptureSample_consumerGetPixData((AUAVCaptureSample*)ptr, dstProps);
				found = TRUE;
				break;
			}
		}
		NBASSERT(found)
	}
	NBThreadMutex_unlock(&data->photo.mutex);
	return r;
}

//

void* AUAppGlueAndroidAVCapture::videoSampleRetain(void* pData, UI64* frameIdFilterAndDst){
	void* r = NULL;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	UI64 frameIdFilter = 0; if(frameIdFilterAndDst != NULL) frameIdFilter = *frameIdFilterAndDst;
	//Determine and lock the source
	{
		AUAVCaptureVideoBuffer* buff = &data->video.buffer;
		NBThreadMutex_lock(&buff->mutex);
		{
			SI32 idxSel = -1;
			SI32 i; for(i = 0; i < buff->samples.arr.use; i++){
				AUAVCaptureSample* itm = NBArray_itmValueAtIndex(&buff->samples.arr, AUAVCaptureSample*, i);
				if(AUAVCaptureSample_consumerPush(itm, frameIdFilter)){
					frameIdFilter = itm->sequential;
					//PRINTF_INFO("AVCapture, videoSampleRetain returned sample-seq(%llu).\n", itm->sequential);
					//Release previous
					if(r != NULL){
						AUAVCaptureSample_consumerPop((AUAVCaptureSample*)r);
						r = NULL;
					}
					//Set as new
					{
						idxSel = i;
						r = itm;
					}
				}
			}
			//if(r == NULL){
			//	PRINTF_INFO("AVCapture, video, no sample.\n");
			//} else {
			//	PRINTF_INFO("AVCapture, video, sample #%d/%d retained %d times.\n", (idxSel + 1), buff->samples.arr.use, ((AUAVCaptureSample*)r)->consumersCount);
			//}
		}
		NBThreadMutex_unlock(&buff->mutex);
	}
	if(frameIdFilterAndDst != NULL) *frameIdFilterAndDst = frameIdFilter;
	return r;
}

void AUAppGlueAndroidAVCapture::videoSampleRelease(void* pData, void* ptr){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	//Determine, release and unlock the source (it match)
	{
		AUAVCaptureVideoBuffer* buff = &data->video.buffer;
		NBThreadMutex_lock(&buff->mutex);
		{
			AUAVCaptureSample* itm = (AUAVCaptureSample*)ptr;
			AUAVCaptureSample_consumerPop(itm);
		}
		NBThreadMutex_unlock(&buff->mutex);
	}
}

const void* AUAppGlueAndroidAVCapture::videoSamplePixData(void* data, void* ptr, STNBBitmapProps* dstProps){
	return AUAVCaptureSample_consumerGetPixData((AUAVCaptureSample*)ptr, dstProps);
}

//

BOOL AUAppGlueAndroidAVCapture::metaQRCode(void* pData, STNBString* dst, const UI64 secsLastValid){ //Captured in the current second
	BOOL r = 0;
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)pData;
	{
		NBThreadMutex_lock(&data->metadata.mutex);
		if(data->metadata.captured.length > 0){
			if((data->metadata.capturedTime + secsLastValid) >= NBDatetime_getCurUTCTimestamp()){
				NBString_set(dst, data->metadata.captured.str);
				r = TRUE;
			}
		}
		NBThreadMutex_unlock(&data->metadata.mutex);
	}
	return r;
}

//---------------
//- CameraDevice.StateCallback (API 21+)
//---------------
void AUAppGlueAndroidAVCapture::CameraStateListener_onClosed(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)dataPtr;
	NBASSERT(data->session.state != ENAVCaptureState_Stopped)
	data->session.state = ENAVCaptureState_Stopped;
}
void AUAppGlueAndroidAVCapture::CameraStateListener_onDisconnected(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	//
}
void AUAppGlueAndroidAVCapture::CameraStateListener_onError(JNIEnv *pEnv, jobject pObj, jobject jCamera, jint error, jlong dataPtr){
	//
}
void AUAppGlueAndroidAVCapture::CameraStateListener_onOpened(JNIEnv *jEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	//Create capture requests
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)dataPtr;
	AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
	{
		NBASSERT(data->session.state == ENAVCaptureState_Starting)
		//
		//camera.createCaptureSession(Arrays.asList(imageReader.getSurface()), sessionStateCallback, null);
		//
		//Start session
		jclass clsHndlr		= jEnv->FindClass("android/os/Handler"); NBASSERT(clsHndlr != NULL)
		jclass clsLstnr		= jEnv->FindClass("com/auframework/AppNative$CaptureStateListener"); NBASSERT(clsLstnr != NULL)
		jclass clsLstnr2	= jEnv->FindClass("com/auframework/AppNative$ImageReaderListener"); NBASSERT(clsLstnr2 != NULL)
		jclass clsList		= jEnv->FindClass("java/util/ArrayList"); NBASSERT(clsList != NULL)
		jclass clsCam		= jEnv->FindClass("android/hardware/camera2/CameraDevice"); NBASSERT(clsCam != NULL)
		jclass clsImgRdr	= jEnv->FindClass("android/media/ImageReader"); NBASSERT(clsImgRdr != NULL)
		if(clsHndlr != NULL && clsLstnr != NULL && clsLstnr2 != NULL && clsList != NULL && clsCam != NULL && clsImgRdr != NULL){
			jmethodID mInitHndlr = jEnv->GetMethodID(clsHndlr, "<init>", "()V"); NBASSERT(mInitHndlr != NULL)
			jmethodID mInitLstr	= jEnv->GetMethodID(clsLstnr, "<init>", "(J)V"); NBASSERT(mInitLstr != NULL)
			jmethodID mInitLstr2 = jEnv->GetMethodID(clsLstnr2, "<init>", "(J)V"); NBASSERT(mInitLstr2 != NULL)
			jmethodID mInit		= jEnv->GetMethodID(clsList, "<init>", "(I)V"); NBASSERT(mInit != NULL)
			jmethodID mAdd		= jEnv->GetMethodID(clsList, "add", "(Ljava/lang/Object;)Z"); NBASSERT(mAdd != NULL)
			jmethodID mNewInst	= jEnv->GetStaticMethodID(clsImgRdr, "newInstance", "(IIII)Landroid/media/ImageReader;"); NBASSERT(mNewInst != NULL)
			jmethodID mGetSurf	= jEnv->GetMethodID(clsImgRdr, "getSurface", "()Landroid/view/Surface;"); NBASSERT(mGetSurf != NULL)
			jmethodID mSetLstr	= jEnv->GetMethodID(clsImgRdr, "setOnImageAvailableListener", "(Landroid/media/ImageReader$OnImageAvailableListener;Landroid/os/Handler;)V"); NBASSERT(mSetLstr != NULL)
			jmethodID mCreate	= jEnv->GetMethodID(clsCam, "createCaptureSession", "(Ljava/util/List;Landroid/hardware/camera2/CameraCaptureSession$StateCallback;Landroid/os/Handler;)V"); NBASSERT(mCreate != NULL)
			if(mInitHndlr != NULL && mInitLstr != NULL && mInitLstr2 != NULL && mInit != NULL && mAdd != NULL && mNewInst != NULL && mGetSurf != NULL && mSetLstr != NULL && mCreate != NULL){
				//Must be YUV_420_888
				const ENAVCaptureSampleColor color = ENAVCaptureSampleColor_YUV_420_888;
				//Set camera (input)
				{
					if(data->input.osObj != NULL){
						jEnv->DeleteGlobalRef(data->input.osObj);
						data->input.osObj = NULL;
					}
					data->input.osObj		= jEnv->NewGlobalRef(jCamera);
				}
				//Add outputs
				jobject jList = jEnv->NewObject(clsList, mInit, 2); NBASSERT(jList != NULL)
				if(jList != NULL){
					//Video output
					{
						const STNBSizeI sampleSz = AUAVCaptureSample_sizeValues(data->session.streamSz);
						jobject jImgRdr = jEnv->CallStaticObjectMethod(clsImgRdr, mNewInst, sampleSz.width, sampleSz.height, (jint)color, 2 /*maxImages*/); NBASSERT(jImgRdr != NULL)
						if(jImgRdr != NULL){
							jobject jSurface = jEnv->CallObjectMethod(jImgRdr, mGetSurf); NBASSERT(jSurface != NULL)
							if(jSurface != NULL){
								//Set listeners
								jobject jCapImgLstnr	= jEnv->NewObject(clsLstnr2, mInitLstr2, (jlong)data); NBASSERT(jCapImgLstnr != NULL)
								jobject jHandler2		= (jobject)jniGlue->jHandler(); /*jEnv->NewObject(clsHndlr, mInitHndlr);*/ NBASSERT(jHandler2 != NULL)
								if(jCapImgLstnr != NULL && jHandler2 != NULL){
									jEnv->CallVoidMethod(jImgRdr, mSetLstr, jCapImgLstnr, jHandler2);
									//Set video preview buffer (output)
									{
										if(data->video.osOutput != NULL){
											jEnv->DeleteGlobalRef(data->video.osOutput);
											data->video.osOutput = NULL;
										}
										
										data->video.osOutput 	= jEnv->NewGlobalRef(jImgRdr);
									}
									//Add to outputs list
									jEnv->CallBooleanMethod(jList, mAdd, jSurface);
									PRINTF_INFO("AVCapture, Added preview output.\n");
								}
							}
						}
					}
					//Photo output
					{
						const STNBSizeI sampleSz = AUAVCaptureSample_sizeValues(data->session.photoSz);
						jobject jImgRdr = jEnv->CallStaticObjectMethod(clsImgRdr, mNewInst, sampleSz.width, sampleSz.height, (jint)color, 1 /*maxImages*/); NBASSERT(jImgRdr != NULL)
						if(jImgRdr != NULL){
							jobject jSurface = jEnv->CallObjectMethod(jImgRdr, mGetSurf); NBASSERT(jSurface != NULL)
							if(jSurface != NULL){
								//Set listeners
								jobject jCapImgLstnr	= jEnv->NewObject(clsLstnr2, mInitLstr2, (jlong)data); NBASSERT(jCapImgLstnr != NULL)
								jobject jHandler2		= (jobject)jniGlue->jHandler(); /*jEnv->NewObject(clsHndlr, mInitHndlr);*/ NBASSERT(jHandler2 != NULL)
								if(jCapImgLstnr != NULL && jHandler2 != NULL){
									jEnv->CallVoidMethod(jImgRdr, mSetLstr, jCapImgLstnr, jHandler2);
									//Set video preview buffer (output)
									{
										if(data->photo.osOutput != NULL){
											jEnv->DeleteGlobalRef(data->photo.osOutput);
											data->photo.osOutput = NULL;
										}
										data->photo.osOutput 	= jEnv->NewGlobalRef(jImgRdr);
									}
									//Add to outputs list
									jEnv->CallBooleanMethod(jList, mAdd, jSurface);
									PRINTF_INFO("AVCapture, Added photo output.\n");
								}
							}
						}
					}
				}
				//Start session
				{
					jobject jCapSessLstnr	= jEnv->NewObject(clsLstnr, mInitLstr, (jlong)data); NBASSERT(jCapSessLstnr != NULL)
					jobject jHandler		= (jobject)jniGlue->jHandler(); /*jEnv->NewObject(clsHndlr, mInitHndlr);*/ NBASSERT(jHandler != NULL)
					if(jCapSessLstnr != NULL && jHandler != NULL){
						jEnv->CallVoidMethod(jCamera, mCreate, jList, jCapSessLstnr, jHandler);
						PRINTF_INFO("AVCapture, Session started.\n");
					}
				}
				//
				NBJNI_DELETE_REF_LOCAL(jEnv, jList)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsList)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsImgRdr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsCam)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsLstnr2)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsLstnr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsHndlr)
		}
	}
}
//------------------------
//- CameraCaptureSession.StateCallback (API 21+)
//------------------------
void AUAppGlueAndroidAVCapture::CaptureStateListener_onActive(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	//
}
void AUAppGlueAndroidAVCapture::CaptureStateListener_onCaptureQueueEmpty(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	//
}
void AUAppGlueAndroidAVCapture::CaptureStateListener_onClosed(JNIEnv *jEnv, jobject pObj, jobject jSession, jlong dataPtr){
	//
}
void AUAppGlueAndroidAVCapture::CaptureStateListener_onConfigureFailed(JNIEnv *jEnv, jobject pObj, jobject jSession, jlong dataPtr){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)dataPtr;
	NBASSERT(data->session.state != ENAVCaptureState_Stopped)
	data->session.state = ENAVCaptureState_Stopped;
}
void AUAppGlueAndroidAVCapture::CaptureStateListener_onConfigured(JNIEnv *jEnv, jobject pObj, jobject jSession, jlong dataPtr){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)dataPtr;
	data->session.osObj		= jEnv->NewGlobalRef(jSession);
	data->session.state		= ENAVCaptureState_Running;
	//Create CaptureRequest
	{
		/*CaptureRequest.Builder builder = camera.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
		 builder.addTarget(imageReader.getSurface());
		 obj = builder.build();
		 session.setRepeatingRequest(obj, null, null);
		 */
		//Video preview
		jclass clsCam		= jEnv->FindClass("android/hardware/camera2/CameraDevice"); NBASSERT(clsCam != NULL)
		jclass clsSess		= jEnv->FindClass("android/hardware/camera2/CameraCaptureSession"); NBASSERT(clsSess != NULL)
		jclass clsBldr		= jEnv->FindClass("android/hardware/camera2/CaptureRequest$Builder"); NBASSERT(clsBldr != NULL)
		jclass clsImgRdr	= jEnv->FindClass("android/media/ImageReader"); NBASSERT(clsImgRdr != NULL)
		if(clsCam != NULL && clsSess != NULL && clsBldr != NULL && clsImgRdr != NULL){
			jmethodID mCreate	= jEnv->GetMethodID(clsCam, "createCaptureRequest", "(I)Landroid/hardware/camera2/CaptureRequest$Builder;"); NBASSERT(mCreate != NULL)
			jmethodID mGetSurf	= jEnv->GetMethodID(clsImgRdr, "getSurface", "()Landroid/view/Surface;"); NBASSERT(mGetSurf != NULL)
			jmethodID mAddTgt	= jEnv->GetMethodID(clsBldr, "addTarget", "(Landroid/view/Surface;)V"); NBASSERT(mAddTgt != NULL)
			jmethodID mBuild	= jEnv->GetMethodID(clsBldr, "build", "()Landroid/hardware/camera2/CaptureRequest;"); NBASSERT(mBuild != NULL)
			jmethodID mSetReq	= jEnv->GetMethodID(clsSess, "setRepeatingRequest", "(Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CameraCaptureSession$CaptureCallback;Landroid/os/Handler;)I"); NBASSERT(mSetReq != NULL)
			if(mCreate != NULL && mGetSurf != NULL && mAddTgt != NULL && mBuild != NULL && mSetReq != NULL){
				jobject jCamera = data->input.osObj; NBASSERT(jCamera != NULL)
				//Start Video Preview
				{
					jobject jImgRdr = data->video.osOutput; NBASSERT(jImgRdr != NULL)
					jobject jSurface = jEnv->CallObjectMethod(jImgRdr, mGetSurf); NBASSERT(jSurface != NULL)
					if(jSurface != NULL){
						//TEMPLATE_PREVIEW: Constant Value: 1 (0x00000001)
						//TEMPLATE_STILL_CAPTURE: Constant Value: 2 (0x00000002)
						jobject jBldr = jEnv->CallObjectMethod(jCamera, mCreate, 1 /*TEMPLATE_PREVIEW*/); NBASSERT(jBldr != NULL)
						if(jBldr != NULL){
							jEnv->CallVoidMethod(jBldr, mAddTgt, jSurface);
							jobject jCapReq = jEnv->CallObjectMethod(jBldr, mBuild); NBASSERT(jCapReq != NULL)
							if(jCapReq != NULL){
								jint uid = jEnv->CallIntMethod(jSession, mSetReq, jCapReq, NULL, NULL);
								PRINTF_INFO("AVCapture, Preview Capture Started.\n");
							}
						}
					}
				}
			}
			//
			NBJNI_DELETE_REF_LOCAL(jEnv, clsImgRdr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsBldr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsSess)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsCam)
		}
	}
}
void AUAppGlueAndroidAVCapture::CaptureStateListener_onReady(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	//
}
void AUAppGlueAndroidAVCapture::CaptureStateListener_onSurfacePrepared(JNIEnv *pEnv, jobject pObj, jobject jSession, jobject jSurface, jlong dataPtr){
	//
}
//------------------------
//- ImageReader.OnImageAvailableListener (API 19+)
//------------------------

void AUAppGlueAndroidAVCapture::ImageReaderListener_onImageAvailable(JNIEnv *jEnv, jobject pObj, jobject jReader, jlong dataPtr){
	AUAppGlueAndroidAVCaptureData* data = (AUAppGlueAndroidAVCaptureData*)dataPtr;
	//Filter, do not process new images while 'stopping'
	if(data->session.state == ENAVCaptureState_Starting || data->session.state == ENAVCaptureState_Running){
		jclass clsImgRdr	= jEnv->FindClass("android/media/ImageReader"); NBASSERT(clsImgRdr != NULL)
		jclass clsImg		= jEnv->FindClass("android/media/Image"); NBASSERT(clsImg != NULL)
		jclass clsPlane		= jEnv->FindClass("android/media/Image$Plane"); NBASSERT(clsPlane != NULL)
		jclass clsBuff		= jEnv->FindClass("java/nio/ByteBuffer"); NBASSERT(clsBuff != NULL)
		if(clsImgRdr != NULL && clsImg != NULL && clsPlane != NULL && clsBuff != NULL){
			jmethodID mAcqLast		= jEnv->GetMethodID(clsImgRdr, "acquireLatestImage", "()Landroid/media/Image;"); NBASSERT(mAcqLast != NULL)
			jmethodID mGetFormat	= jEnv->GetMethodID(clsImg, "getFormat", "()I"); NBASSERT(mGetFormat != NULL)
			jmethodID mGetWidth		= jEnv->GetMethodID(clsImg, "getWidth", "()I"); NBASSERT(mGetWidth != NULL)
			jmethodID mGetHeight	= jEnv->GetMethodID(clsImg, "getHeight", "()I"); NBASSERT(mGetHeight != NULL)
			jmethodID mGetPlns		= jEnv->GetMethodID(clsImg, "getPlanes", "()[Landroid/media/Image$Plane;"); NBASSERT(mGetPlns != NULL)
			jmethodID mClose		= jEnv->GetMethodID(clsImg, "close", "()V"); NBASSERT(mClose != NULL)
			jmethodID mGetBuff		= jEnv->GetMethodID(clsPlane, "getBuffer", "()Ljava/nio/ByteBuffer;"); NBASSERT(mGetBuff != NULL)
			jmethodID mGetPxStrd	= jEnv->GetMethodID(clsPlane, "getPixelStride", "()I"); NBASSERT(mGetPxStrd != NULL)
			jmethodID mGetRwStrd	= jEnv->GetMethodID(clsPlane, "getRowStride", "()I"); NBASSERT(mGetRwStrd != NULL)
			if(mAcqLast != NULL && mGetFormat != NULL && mGetWidth != NULL && mGetHeight != NULL && mGetPlns != NULL && mClose != NULL && mGetBuff != NULL && mGetPxStrd != NULL && mGetRwStrd != NULL){
				jobject jImg = jEnv->CallObjectMethod(jReader, mAcqLast);
				if(jImg != NULL){
					const jint format	= jEnv->CallIntMethod(jImg, mGetFormat);
					const jint width	= jEnv->CallIntMethod(jImg, mGetWidth);
					const jint height	= jEnv->CallIntMethod(jImg, mGetHeight);
					jobjectArray jPlanes = (jobjectArray)jEnv->CallObjectMethod(jImg, mGetPlns);
					if(jPlanes != NULL){
						const SI32 planesCount = (SI32)jEnv->GetArrayLength(jPlanes);
						//PRINTF_INFO("AVCapture, Image format(%d) width(%d) height(%d) planes(%d).\n", format, width, height, count);
						if(planesCount > 0){
							AUAVCapturePlane* planes = NBMemory_allocTypes(AUAVCapturePlane, planesCount);
							//Load planes refs
							{
								SI32 i; for(i = 0; i < planesCount; i++){
									jobject jPlane = jEnv->GetObjectArrayElement(jPlanes, i);
									AUAVCapturePlane* plane = &planes[i];
									if(jPlane == NULL){
										plane->buffer	= NULL;
										plane->pxStride	= 0;
										plane->rwStride	= 0;
									} else {
										jobject jBuff		= jEnv->CallObjectMethod(jPlane, mGetBuff); NBASSERT(jBuff != NULL)
										const jint pxStride = jEnv->CallIntMethod(jPlane, mGetPxStrd);
										const jint rwStride = jEnv->CallIntMethod(jPlane, mGetRwStrd);
										//
										plane->buffer	= (const BYTE*)jEnv->GetDirectBufferAddress(jBuff);
										plane->pxStride	= pxStride;
										plane->rwStride	= rwStride;
										//PRINTF_INFO("AVCapture, Image plane #%d / %d has buffer(%s) pxStride(%d) rwStride(%d).\n", (i + 1), planesCount, (plane->buffer != NULL ? "NOT_NULL" : "NULL"), plane->pxStride, plane->rwStride);
									}
								}
							}
							if(jEnv->IsSameObject(jReader, data->photo.osOutput)){
								//Photo captures
								UI64 sampleSeq = 0;
								AUAVCaptureSample* dstSmpl = NBMemory_allocType(AUAVCaptureSample);
								AUAVCaptureSample_init(dstSmpl);
								//Define sequential
								{
									NBThreadMutex_lock(&data->photo.mutex);
									{
										data->photo.seq++;
										sampleSeq = data->photo.seq;
									}
									NBThreadMutex_unlock(&data->photo.mutex);
								}
								//Fill
								{
									dstSmpl->state = ENAVCaptureSampleState_Filling;
									AUAVCaptureSample_setPixelBuffer(dstSmpl, sampleSeq, format, NBST_P(STNBSizeI, width, height), planes, planesCount, NULL, NULL);
								}
								if(dstSmpl->state != ENAVCaptureSampleState_Filled){
									//Release
									AUAVCaptureSample_release(dstSmpl);
								} else {
									//Add to queue
									NBThreadMutex_lock(&data->photo.mutex);
									{
										data->photo.samplesWritten++;
										NBArray_addValue(&data->photo.captured, dstSmpl);
										PRINTF_INFO("AVCapture, photo #%llu added to buffer (%d in queue).\n", sampleSeq, data->photo.captured.use);
									}
									NBThreadMutex_unlock(&data->photo.mutex);
								}
							} else if(jEnv->IsSameObject(jReader, data->video.osOutput)){
								//Video sample captured
								//PRINTF_INFO("AVCapture, onImageAvailable from previewCapture.\n");
								AUAVCaptureVideoBuffer* buff = &data->video.buffer;
								AUAVCaptureSample* dstSmpl = NULL;
								UI64 sampleSeq = 0;
								//Determine and lock the destination
								{
									NBThreadMutex_lock(&buff->mutex);
									{
										SI32 i; for(i = 0; i < buff->samples.arr.use; i++){
											AUAVCaptureSample* itm = NBArray_itmValueAtIndex(&buff->samples.arr, AUAVCaptureSample*, i);
											if(itm->state != ENAVCaptureSampleState_Filling && itm->consumersCount <= 0){
												itm->state	= ENAVCaptureSampleState_Filling;
												dstSmpl		= itm;
												break;
											}
										}
										//Detemine seq
										if(dstSmpl != NULL){
											buff->stats.seq++;
											sampleSeq = buff->stats.seq;
										}
									}
									NBThreadMutex_unlock(&buff->mutex);
								}
								//Ignore or fill
								if(dstSmpl == NULL){
									NBThreadMutex_lock(&buff->mutex);
									{
										buff->stats.ignored++;
									}
									NBThreadMutex_unlock(&buff->mutex);
									//Debug
#									ifdef NB_CONFIG_INCLUDE_ASSERTS
									{
										NBThreadMutex_lock(&data->_dbgMutex);
										data->_dbgAccumMissd++;
										NBThreadMutex_unlock(&data->_dbgMutex);
									}
#									endif
								} else {
									AUAVCaptureSample_setPixelBuffer(dstSmpl, sampleSeq, format, NBST_P(STNBSizeI, width, height), planes, planesCount, buff, (data->session.extraOutputsMask & ENAVCaptureOutBit_QRCode) != 0 ? data : NULL);
									if(dstSmpl->state != ENAVCaptureSampleState_Filled){
										NBThreadMutex_lock(&buff->mutex);
										{
											buff->stats.errors++;
										}
										NBThreadMutex_unlock(&buff->mutex);
#										ifdef NB_CONFIG_INCLUDE_ASSERTS
										{
											NBThreadMutex_lock(&data->_dbgMutex);
											data->_dbgAccumError++;
											NBThreadMutex_unlock(&data->_dbgMutex);
										}
#										endif
									} else {
										NBThreadMutex_lock(&buff->mutex);
										{
											buff->stats.written++;
										}
										NBThreadMutex_unlock(&buff->mutex);
#										ifdef NB_CONFIG_INCLUDE_ASSERTS
										{
											NBThreadMutex_lock(&data->_dbgMutex);
											data->_dbgAccumWrite++;
											NBThreadMutex_unlock(&data->_dbgMutex);
										}
#										endif
									}
								}
#								ifdef NB_CONFIG_INCLUDE_ASSERTS
								{
									NBThreadMutex_lock(&data->_dbgMutex);
									{
										CICLOS_CPU_TIPO curCicle, oneSec;
										CICLOS_CPU_POR_SEGUNDO(oneSec);
										CICLOS_CPU_HILO(curCicle);
										if(data->_dbgCicleStarted == 0) data->_dbgCicleStarted = curCicle;
										while((data->_dbgCicleStarted + oneSec) <= curCicle){
											PRINTF_INFO("AVCapture, %d writen, %d missed, %d error; frames-per-sec.\n", data->_dbgAccumWrite, data->_dbgAccumMissd, data->_dbgAccumError);
											data->_dbgCicleStarted += oneSec;
											data->_dbgAccumMissd = 0;
											data->_dbgAccumWrite = 0;
											data->_dbgAccumError = 0;
										}
									}
									NBThreadMutex_unlock(&data->_dbgMutex);
								}
#								endif
							} else {
								PRINTF_ERROR("onImageAvailable ignored (unknown source).\n");
							}
							//Release planes refs
							NBMemory_free(planes);
						}
					}
					//Close image (let it be used by the buffer)
					jEnv->CallVoidMethod(jImg, mClose);
				}
			}
			//Release
			NBJNI_DELETE_REF_LOCAL(jEnv, clsBuff)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPlane)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsImg)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsImgRdr)
		}
	}
}

// Listener methods

void AUAppGlueAndroidAVCaptureListener::appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* grantsResults /*jintArray*/){
	//PRINTF_INFO("AUAppGlueAndroidAVCaptureListener::appReqPermResult.\n");
	AUAppGlueAndroidJNI* jniGlue = _data->app->getGlueJNI();
	JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
	jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
	{
		jobjectArray jPerms	= (jobjectArray)perms;
		jintArray jGrants	= (jintArray)grantsResults;
		jint* iGrants		= jEnv->GetIntArrayElements(jGrants, 0);
		SI32 i; const SI32 count = (SI32)jEnv->GetArrayLength(jPerms); NBASSERT(count == (SI32)jEnv->GetArrayLength(jGrants))
		for(i = 0; i < count; i++){
			jobject jPerm	= jEnv->GetObjectArrayElement(jPerms, i);
			jint granted	= iGrants[i];
			const char* utf8 = jEnv->GetStringUTFChars((jstring)jPerm, 0);
			if(NBString_strIsEqual(utf8, AVCAPTURE_PERM_ID)){
				//Save "already asked" value
				AUAppGlueAndroidJNI::saveDataToSharedPrefs(jEnv, jContext, AVCAPTURE_PREF_NAME, AVCAPTURE_PREF_DATA_NAME, "YES");
				//
				_data->requestingAuth = FALSE;
			}
			//PRINTF_INFO("AVCapture, Perm #%d / %d: '%s' (%s).\n", (i + 1), count, utf8, granted ? "GRANTED" : "DENIED");
			jEnv->ReleaseStringUTFChars((jstring)jPerm, utf8);
		}
	}
}
