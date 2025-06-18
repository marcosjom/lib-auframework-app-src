//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrAVCapture.h"

#define AUMngrAVCapture_MUTEX_ACTIVATE		NBHILO_MUTEX_ACTIVAR(&_mutex) _mutexLocksCount++;
#define AUMngrAVCapture_MUTEX_DEACTIVATE	_mutexLocksCount--; NBHILO_MUTEX_DESACTIVAR(&_mutex)

STMngrAVCaptureCalls AUMngrAVCapture::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
};

AUMngrAVCapture::AUMngrAVCapture() : AUObjeto()
	, _addedToAnimators(FALSE)
	, _videoSamplesTextureSeq(0)
	, _videoSamplesTexture(NULL)
	, _mutexLocksCount(0)
{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::AUMngrAVCapture")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrAVCapture")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		SI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0)
		}
	}
#	endif
#	ifdef NB_CONFIG_INCLUDE_ASSERTS
	_dbgAccumRead		= 0;
	_dbgCicleStarted	= 0;
#	endif
	//Last sample
	{
		NBMemory_setZero(_lastSample);
		_lastSample.keepCopy = FALSE;
		NBBitmap_init(&_lastSample.bmp);
	}
	NBHILO_MUTEX_INICIALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrAVCapture::~AUMngrAVCapture(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::~AUMngrAVCapture")
	NBGestorAnimadores::quitarAnimador(this);
	AUMngrAVCapture_MUTEX_ACTIVATE
	//Last sample
	{
		_lastSample.keepCopy = FALSE;
		NBBitmap_release(&_lastSample.bmp);
	}
	if(_videoSamplesTexture != NULL){
		_videoSamplesTexture->liberar(NB_RETENEDOR_THIS);
		_videoSamplesTexture = NULL;
	}
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	NBHILO_MUTEX_FINALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrAVCapture::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCaptureAuthStatus != NULL);
}

bool AUMngrAVCapture::setGlue(AUAppI* app, PTRFuncAVCCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::setGlue")
	bool r = false;
	//Finish
	if(_calls.funcDestroy != NULL){
		(*_calls.funcDestroy)(_calls.funcDestroyParam);
		_calls.funcDestroy = NULL;
	}
	//Init
	if(initCall != NULL){
		if(!(*initCall)(app, &_calls)){
			NBASSERT(false)
		} else {
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//


ENAVCaptureAuthStatus AUMngrAVCapture::captureAuthStatus(const BOOL requestIfNecesary){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::captureAuthStatus")
	AUMngrAVCapture_MUTEX_ACTIVATE
	ENAVCaptureAuthStatus r = ENAVCaptureAuthStatus_Denied;
	if(_calls.funcCaptureAuthStatus != NULL){
		r = (*_calls.funcCaptureAuthStatus)(_calls.funcCaptureAuthStatusParam, requestIfNecesary);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrAVCapture::captureStart(const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask){ //ENAVCaptureOutBit
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::captureStart")
	AUMngrAVCapture_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcCaptureStart != NULL){
		if((*_calls.funcCaptureStart)(_calls.funcCaptureStartParam, focusRng, streamSz, photoSz, extraOutputsMask)){
			if(!_addedToAnimators){
				NBGestorAnimadores::agregarAnimador(this, this);
				_addedToAnimators = TRUE;
			}
			r = true;
		}
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool AUMngrAVCapture::captureIsRuning(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::captureIsRuning")
	AUMngrAVCapture_MUTEX_ACTIVATE
	bool r = false;
	if(_calls.funcCaptureIsRuning != NULL){
		if((*_calls.funcCaptureIsRuning)(_calls.funcCaptureIsRuningParam)){
			r = true;
		}
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrAVCapture::captureStop(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::captureStop")
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcCaptureStop != NULL){
		(*_calls.funcCaptureStop)(_calls.funcCaptureStopParam);
		if(_addedToAnimators){
			NBGestorAnimadores::quitarAnimador(this);
			_addedToAnimators = FALSE;
		}
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrAVCapture::photoTrigger(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::photoTrigger")
	bool r = false;
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcPhotoTrigger != NULL){
		if((*_calls.funcPhotoTrigger)(_calls.funcPhotoTriggerParam)){
			r = true;
		}
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* AUMngrAVCapture::photoSampleRetain(UI64* frameIdFilterAndDst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::photoSampleRetain")
	void* r = NULL;
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcPhotoSampleRetain != NULL){
		r = (*_calls.funcPhotoSampleRetain)(_calls.funcPhotoSampleRetainParam, frameIdFilterAndDst);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrAVCapture::photoSampleRelease(void* ptr){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::photoSampleRelease")
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcPhotoSampleRelease != NULL){
		(*_calls.funcPhotoSampleRelease)(_calls.funcPhotoSampleReleaseParam, ptr);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const void* AUMngrAVCapture::photoSamplePixData(void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::photoSamplePixData")
	const void* r = NULL;
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcPhotoSamplePixData != NULL){
		r = (*_calls.funcPhotoSamplePixData)(_calls.funcPhotoSamplePixDataParam, ptr, dstProps, dstDegRotFromIntended);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUTextura* AUMngrAVCapture::videoSamplesTexture() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSamplesTexture")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _videoSamplesTexture;
}

UI64 AUMngrAVCapture::videoSamplesTextureSeq() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSamplesTextureSeq")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _videoSamplesTextureSeq;
}

void* AUMngrAVCapture::videoSampleRetain(UI64* frameIdFilterAndDst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSampleRetain")
	void* r = NULL;
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcVideoSampleRetain != NULL){
		r = (*_calls.funcVideoSampleRetain)(_calls.funcVideoSampleRetainParam, frameIdFilterAndDst);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrAVCapture::videoSampleRelease(void* ptr){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSampleRelease")
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcVideoSampleRelease != NULL){
		(*_calls.funcVideoSampleRelease)(_calls.funcVideoSampleReleaseParam, ptr);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const void* AUMngrAVCapture::videoSamplePixData(void* ptr, STNBBitmapProps* dstProps){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSamplePixData")
	const void* r = NULL;
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcVideoSamplePixData != NULL){
		r = (*_calls.funcVideoSamplePixData)(_calls.funcVideoSamplePixDataParam, ptr, dstProps);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Keep last sample

BOOL AUMngrAVCapture::videoSampleLastIsKept(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSampleLastIsKept")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _lastSample.keepCopy;
}

void AUMngrAVCapture::videoSampleLastSetKeep(const BOOL keepLast){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSampleLastSetKeep")
	_lastSample.keepCopy = keepLast;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

STNBBitmapProps AUMngrAVCapture::videoSampleLastProps(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSampleLastProps")
	STNBBitmapProps r = NBBitmap_getProps(&_lastSample.bmp);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrAVCapture::videoSampleLastGetRect(STNBBitmap* dst, const STNBPointI pos, const STNBSizeI size){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::videoSampleLastGetRect")
	BOOL r = FALSE;
	{
		const STNBBitmapProps props = NBBitmap_getProps(&_lastSample.bmp);
		if(props.size.width > 0 && props.size.height > 0 && props.color > ENNBBitmapColor_undef && props.color < ENNBBitmapColor_Count){
			if(dst == NULL){
				r = TRUE;
			} else if(NBBitmap_pasteBitmapRect(dst, NBST_P(STNBPointI, 0, 0 ), &_lastSample.bmp, NBST_P(STNBRectI, (SI32)pos.x, (SI32)pos.y, (SI32)size.width, (SI32)size.height ), NBST_P(STNBColor8, 255, 255, 255, 255 ))){
				r = TRUE;
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

BOOL AUMngrAVCapture::metaQRCode(STNBString* dst, const UI64 secsLastValid){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::metaQRCode")
	BOOL r = FALSE;
	AUMngrAVCapture_MUTEX_ACTIVATE
	if(_calls.funcMetaQRCode != NULL){
		r = (*_calls.funcMetaQRCode)(_calls.funcMetaQRCodeParam, dst, secsLastValid);
	}
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//focus

BOOL AUMngrAVCapture::isFocusModeSupported(const ENAVCaptureFocusMode focusMode){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::isFocusModeSupported")
    BOOL r = FALSE;
    AUMngrAVCapture_MUTEX_ACTIVATE
    if(_calls.funcIsFocusModeSupported != NULL){
        r = (*_calls.funcIsFocusModeSupported)(_calls.funcIsFocusModeSupportedParam, focusMode);
    }
    AUMngrAVCapture_MUTEX_DEACTIVATE
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

ENAVCaptureFocusMode AUMngrAVCapture::getFocusMode(){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::getFocusMode")
    ENAVCaptureFocusMode r = ENAVCaptureFocusMode_Count;
    AUMngrAVCapture_MUTEX_ACTIVATE
    if(_calls.funcGetFocusMode != NULL){
        r = (*_calls.funcGetFocusMode)(_calls.funcGetFocusModeParam);
    }
    AUMngrAVCapture_MUTEX_DEACTIVATE
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

BOOL AUMngrAVCapture::setFocusMode(const ENAVCaptureFocusMode focusMode){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::setFocusMode")
    BOOL r = FALSE;
    AUMngrAVCapture_MUTEX_ACTIVATE
    if(_calls.funcSetFocusMode != NULL){
        r = (*_calls.funcSetFocusMode)(_calls.funcSetFocusModeParam, focusMode);
    }
    AUMngrAVCapture_MUTEX_DEACTIVATE
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

BOOL AUMngrAVCapture::isAdjustingFocus(){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::isAdjustingFocus")
    BOOL r = FALSE;
    AUMngrAVCapture_MUTEX_ACTIVATE
    if(_calls.funcIsAdjustingFocus != NULL){
        r = (*_calls.funcIsAdjustingFocus)(_calls.funcIsAdjustingFocusParam);
    }
    AUMngrAVCapture_MUTEX_DEACTIVATE
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

float AUMngrAVCapture::getLensPositionRel(){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::getLensPositionRel")
    float r = 0.0f;
    AUMngrAVCapture_MUTEX_ACTIVATE
    if(_calls.funcGetLensPositionRel != NULL){
        r = (*_calls.funcGetLensPositionRel)(_calls.funcGetLensPositionRelParam);
    }
    AUMngrAVCapture_MUTEX_DEACTIVATE
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

BOOL AUMngrAVCapture::lockLensToPositionRel(const float relPos){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::lockLensToPositionRel")
    BOOL r = FALSE;
    AUMngrAVCapture_MUTEX_ACTIVATE
    if(_calls.funcLockLensToPositionRel != NULL){
        r = (*_calls.funcLockLensToPositionRel)(_calls.funcLockLensToPositionRelParam, relPos);
    }
    AUMngrAVCapture_MUTEX_DEACTIVATE
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

//

void AUMngrAVCapture::tickAnimacion(float secs){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::tickAnimacion")
	//Update texture
	{
		AUMngrAVCapture_MUTEX_ACTIVATE
		if(_calls.funcVideoSampleRetain != NULL && _calls.funcVideoSamplePixData != NULL && _calls.funcVideoSampleRelease != NULL){
			void* sampleV = (*_calls.funcVideoSampleRetain)(_calls.funcVideoSampleRetainParam, &this->_videoSamplesTextureSeq);
			if(sampleV != NULL){
				STNBBitmapProps vProps; //video
				NBMemory_setZero(vProps);
				const void* vData = (*_calls.funcVideoSamplePixData)(_calls.funcVideoSamplePixDataParam, sampleV, &vProps); NBASSERT(vData != NULL)
				if(vData != NULL){
					//Process sample
					MapaDeBitsDesciptor tProps; //texture
					NBMemory_setZero(tProps);
					tProps.ancho			= vProps.size.width;
					tProps.alto				= vProps.size.height;
					tProps.bitsPorPixel		= vProps.bitsPerPx;
					tProps.bytesPorLinea	= vProps.bytesPerLine;
					switch (vProps.color) {
						case ENNBBitmapColor_ALPHA8:		tProps.color = COLOR_ALPHA8; break;
						case ENNBBitmapColor_GRIS8:			tProps.color = COLOR_GRIS8; break;
						case ENNBBitmapColor_GRISALPHA8:	tProps.color = COLOR_GRISALPHA8; break;
						case ENNBBitmapColor_RGB4:			tProps.color = COLOR_RGB4; break;
						case ENNBBitmapColor_RGB8:			tProps.color = COLOR_RGB8; break;
						case ENNBBitmapColor_RGBA4:			tProps.color = COLOR_RGBA4; break;
						case ENNBBitmapColor_RGBA8:			tProps.color = COLOR_RGBA8; break;
						case ENNBBitmapColor_ARGB4:			tProps.color = COLOR_ARGB4; break;
						case ENNBBitmapColor_ARGB8:			tProps.color = COLOR_ARGB8; break;
						case ENNBBitmapColor_BGRA8:			tProps.color = COLOR_BGRA8; break;
						case ENNBBitmapColor_SWF_PIX15:		tProps.color = COLOR_SWF_PIX15; break;
						case ENNBBitmapColor_SWF_PIX24:		tProps.color = COLOR_SWF_PIX24; break;
						default: break;
					} //ToDo: COLOR_RGBA8 to COLOR_BGRA8
					if(tProps.color == 0){
						PRINTF_ERROR("AUMngrAVCapture, could not determine the equivalent color for texture.\n");
						NBASSERT(FALSE)
					} else{
						NBTamano texSz; texSz.ancho = texSz.alto = 0;
						if(_videoSamplesTexture != NULL) texSz = _videoSamplesTexture->tamano();
						if(texSz.ancho != vProps.size.width || texSz.alto != vProps.size.height){
							//Init texture
							AUTextura* tex = NBGestorTexturas::texturaDesdeDatos(&tProps, vData, ENTexturaTipoAlmacenamientoGL_AtlasUnico, ENTExturaTipoUsoGL_Lectura, ENTexturaModoPintado_Imagen_Precisa, ENTexturaMipMap_Inhabilitado, ENTexturaOrdenV_HaciaArriba, 1, 1.0f);
							if(tex == NULL){
								PRINTF_ERROR("AUMngrAVCapture, could not create texture.\n");
								NBASSERT(FALSE)
							} else {
								PRINTF_INFO("AUMngrAVCapture, texture changed.\n");
								tex->retener(NB_RETENEDOR_THIS);
								if(_videoSamplesTexture != NULL) _videoSamplesTexture->liberar(NB_RETENEDOR_THIS);
								_videoSamplesTexture = tex;
							}
						} else if(_videoSamplesTexture != NULL){
							//Update texture
							NBGestorTexturas::texturaEstablecerContenidoData(_videoSamplesTexture, &tProps, vData);
						}
						//Keep last sample
						if(_lastSample.keepCopy){
							//Sync bitmap props
							{
								const STNBBitmapProps props = NBBitmap_getProps(&_lastSample.bmp);
								if(vProps.size.width != props.size.width || vProps.size.height != props.size.height || vProps.color != props.color){
									NBBitmap_create(&_lastSample.bmp, vProps.size.width, vProps.size.height, vProps.color);
								}
							}
							//Sync bitmap data
							{
								const STNBBitmapProps props = NBBitmap_getProps(&_lastSample.bmp);
								if(!(vProps.size.width != props.size.width || vProps.size.height != props.size.height || vProps.color != props.color)){
									NBBitmap_pasteBitmapData(&_lastSample.bmp, NBST_P(STNBPointI,0 , 0), vProps, (const BYTE*)vData, NBST_P(STNBColor8, 255, 255, 255, 255 ));
								}
							}
						}
						//Print bytes
						/*{
							const BYTE* data = (const BYTE*)vData;
							const SI32 bytesPerRow = vProps.bytesPerLine;
							UI64 sums[4] = { 0, 0, 0, 0 }, sumCount = 0, lnStartOfSums = 0;
							SI32 i; const SI32 count = (bytesPerRow * vProps.size.height);
							PRINTF_INFO("VideoSamplePtr: %llu.\n", (UI64)data);
							for(i = 0; i < count; i += 4){
								sums[0] += data[i];
								sums[1] += data[i + 1];
								sums[2] += data[i + 2];
								sums[3] += data[i + 3];
								sumCount++;
								//PRINTF_INFO("#%d (%d, %d, %d, %d).\n", (i + 1), data[i], data[i + 1], data[i + 2], data[i + 3]);
								if(i != 0 && (i % (bytesPerRow * 64)) == 0){
									PRINTF_INFO("Rslt-video-sample lines#(%d - %d): avg(%d, %d, %d, %d).\n", (SI32)(lnStartOfSums + 1), (SI32)(i / bytesPerRow), (SI32)(sums[0] / sumCount), (SI32)(sums[1] / sumCount), (SI32)(sums[2] / sumCount), (SI32)(sums[3] / sumCount));
									sums[0] = sums[1] = sums[2] = sums[3] = sumCount = 0;
									lnStartOfSums = (i / bytesPerRow);
								}
							}
							if(sumCount != 0){
								PRINTF_INFO("Rslt-video-sample lines#(%d - %d): avg(%d, %d, %d, %d).\n", (SI32)(lnStartOfSums + 1), (SI32)(i / bytesPerRow), (SI32)(sums[0] / sumCount), (SI32)(sums[1] / sumCount), (SI32)(sums[2] / sumCount), (SI32)(sums[3] / sumCount));
								sums[0] = sums[1] = sums[2] = sums[3] = sumCount = 0;
								lnStartOfSums = (i / bytesPerRow);
							}
						}*/
					}
#					ifdef NB_CONFIG_INCLUDE_ASSERTS
					{
						CICLOS_CPU_TIPO curCicle, oneSec;
						CICLOS_CPU_POR_SEGUNDO(oneSec);
						CICLOS_CPU_HILO(curCicle);
						_dbgAccumRead++;
						if(_dbgCicleStarted == 0) _dbgCicleStarted = curCicle;
						while((_dbgCicleStarted + oneSec) <= curCicle){
							//PRINTF_INFO("AUMngrAVCapture, %d frames/sec read.\n", _dbgAccumRead);
							_dbgCicleStarted += oneSec;
							_dbgAccumRead = 0;
						}
					}
#					endif
				}
				//Release sample
				(*_calls.funcVideoSampleRelease)(_calls.funcVideoSampleReleaseParam, sampleV);
			}
		}
		AUMngrAVCapture_MUTEX_DEACTIVATE
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

void AUMngrAVCapture::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::lock")
	AUMngrAVCapture_MUTEX_ACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrAVCapture::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrAVCapture::unlock")
	NBASSERT(_mutexLocksCount > 0)
	AUMngrAVCapture_MUTEX_DEACTIVATE
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrAVCapture)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrAVCapture, "AUMngrAVCapture")
AUOBJMETODOS_CLONAR_NULL(AUMngrAVCapture)
