//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrAVCapture.h"

AUMngrAVCapture* NBMngrAVCapture::_instance	= NULL;

void NBMngrAVCapture::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrAVCapture();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrAVCapture::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrAVCapture::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrAVCapture::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::isGlued")
	const bool r = AUMngrAVCapture::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}


bool NBMngrAVCapture::setGlue(AUAppI* app, PTRFuncAVCCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::setGlue")
	const bool r = AUMngrAVCapture::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

ENAVCaptureAuthStatus NBMngrAVCapture::captureAuthStatus(const BOOL requestIfNecesary){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::captureAuthStatus")
	const ENAVCaptureAuthStatus r = _instance->captureAuthStatus(requestIfNecesary);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrAVCapture::captureStart(const ENAVCaptureFocusRange focusRng, const ENAVCaptureSize streamSz, const ENAVCaptureSize photoSz, const UI32 extraOutputsMask){ //ENAVCaptureOutBit)
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::captureStart")
	bool r = _instance->captureStart(focusRng, streamSz, photoSz, extraOutputsMask);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrAVCapture::captureIsRuning(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::captureIsRuning")
	bool r = _instance->captureIsRuning();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrAVCapture::captureStop(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::captureStop")
	_instance->captureStop();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrAVCapture::photoTrigger(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::photoTrigger")
	const bool r = _instance->photoTrigger();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* NBMngrAVCapture::photoSampleRetain(UI64* frameIdFilterAndDst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::photoSampleRetain")
	void* r = _instance->photoSampleRetain(frameIdFilterAndDst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrAVCapture::photoSampleRelease(void* ptr){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::photoSampleRelease")
	_instance->photoSampleRelease(ptr);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const void* NBMngrAVCapture::photoSamplePixData(void* ptr, STNBBitmapProps* dstProps, SI32* dstDegRotFromIntended){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::photoSamplePixData")
	const void* r = _instance->photoSamplePixData(ptr, dstProps, dstDegRotFromIntended);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUTextura* NBMngrAVCapture::videoSamplesTexture() {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSamplesTexture")
	AUTextura* r = _instance->videoSamplesTexture();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

UI64 NBMngrAVCapture::videoSamplesTextureSeq(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSamplesTextureSeq")
	UI64 r = _instance->videoSamplesTextureSeq();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* NBMngrAVCapture::videoSampleRetain(UI64* frameIdFilterAndDst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSampleRetain")
	void* r = _instance->videoSampleRetain(frameIdFilterAndDst);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrAVCapture::videoSampleRelease(void* ptr){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSampleRelease")
	_instance->videoSampleRelease(ptr);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

const void* NBMngrAVCapture::videoSamplePixData(void* ptr, STNBBitmapProps* dstProps){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSamplePixData")
	const void* r = _instance->videoSamplePixData(ptr, dstProps);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Keep last sample

BOOL NBMngrAVCapture::videoSampleLastIsKept(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSampleLastIsKept")
	const BOOL r = _instance->videoSampleLastIsKept();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrAVCapture::videoSampleLastSetKeep(const BOOL keepLast){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSampleLastSetKeep")
	_instance->videoSampleLastSetKeep(keepLast);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

STNBBitmapProps NBMngrAVCapture::videoSampleLastProps(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSampleLastProps")
	STNBBitmapProps r = _instance->videoSampleLastProps();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrAVCapture::videoSampleLastGetRect(STNBBitmap* dst, const STNBPointI pos, const STNBSizeI size){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::videoSampleLastGetRect")
	BOOL r = _instance->videoSampleLastGetRect(dst, pos, size);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

BOOL NBMngrAVCapture::metaQRCode(STNBString* dst, const UI64 secsLastValid){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::metaQRCode")
	BOOL r = _instance->metaQRCode(dst, secsLastValid);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//focus

BOOL NBMngrAVCapture::isFocusModeSupported(const ENAVCaptureFocusMode focusMode){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::isFocusModeSupported")
    BOOL r = _instance->isFocusModeSupported(focusMode);
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

ENAVCaptureFocusMode NBMngrAVCapture::getFocusMode(){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::getFocusMode")
    const ENAVCaptureFocusMode r = _instance->getFocusMode();
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

BOOL NBMngrAVCapture::setFocusMode(const ENAVCaptureFocusMode focusMode){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::setFocusMode")
    BOOL r = _instance->setFocusMode(focusMode);
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

BOOL NBMngrAVCapture::isAdjustingFocus(){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::isAdjustingFocus")
    BOOL r = _instance->isAdjustingFocus();
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

float NBMngrAVCapture::getLensPositionRel(){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::getLensPositionRel")
    const float r = _instance->getLensPositionRel();
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

BOOL NBMngrAVCapture::lockLensToPositionRel(const float relPos){
    AU_GESTOR_PILA_LLAMADAS_PUSH_3("lockLensToPositionRel::lockLensToPositionRel")
    BOOL r = _instance->lockLensToPositionRel(relPos);
    AU_GESTOR_PILA_LLAMADAS_POP_3
    return r;
}

//

void NBMngrAVCapture::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::lock")
	_instance->lock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrAVCapture::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrAVCapture::unlock")
	_instance->unlock();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

