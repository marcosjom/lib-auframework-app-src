//
//  NBMngrPdfKit.cpp
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrPdfKit.h"

AUMngrPdfKit* NBMngrPdfKit::_instance	= NULL;

void NBMngrPdfKit::init(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::init")
	_instance	= new(ENMemoriaTipo_Temporal) AUMngrPdfKit();
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrPdfKit::finish(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::finish")
	if(_instance != NULL) _instance->liberar(NB_RETENEDOR_NULL); _instance = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool NBMngrPdfKit::isInited(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::isInited")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_instance != NULL);
}

//

bool NBMngrPdfKit::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::isGlued")
	bool r = AUMngrPdfKit::isGlued();
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

bool NBMngrPdfKit::setGlue(AUAppI* app, PTRfuncPdfKitCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::setGlue")
	bool r = AUMngrPdfKit::setGlue(app, initCall);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Specs
BOOL NBMngrPdfKit::canMultithread(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::canMultithread")
	BOOL r = FALSE;
	if(_instance != NULL){
		r = _instance->canMultithread();
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Document

void* NBMngrPdfKit::docOpenFromPath(const char* path){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docOpenFromPath")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->docOpenFromPath(path);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* NBMngrPdfKit::docOpenFromData(const void* data, const UI32 dataSz){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docOpenFromData")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->docOpenFromData(data, dataSz);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* NBMngrPdfKit::docOpenFromRenderJob(const STNBPdfRenderDoc* job){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docOpenFromRenderJob")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->docOpenFromRenderJob(job);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* NBMngrPdfKit::docGetDataGlued(void* pageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docGetDataGlued")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->docGetDataGlued(pageRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrPdfKit::docRetain(void* docRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docRetain")
	if(_instance != NULL){
		_instance->docRetain(docRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrPdfKit::docRelease(void* docRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docRelease")
	if(_instance != NULL){
		_instance->docRelease(docRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

UI32 NBMngrPdfKit::docGetPagesCount(void* docRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docGetPagesCount")
	UI32 r = 0;
	if(_instance != NULL){
		r = _instance->docGetPagesCount(docRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* NBMngrPdfKit::docGetPageAtIdx(void* docRef, const UI32 idx){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docGetPageAtIdx")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->docGetPageAtIdx(docRef, idx);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrPdfKit::docWriteToFilepath(void* docRef, const char* filePath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::docWriteToFilepath")
	BOOL r = FALSE;
	if(_instance != NULL){
		r = _instance->docWriteToFilepath(docRef, filePath);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Page

void* NBMngrPdfKit::pageGetDataGluedInner(void* pageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageGetDataGlued")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->pageGetDataGluedInner(pageRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrPdfKit::pageGetDataBitmap(void* pageRef, STNBBitmapProps* dstProps, void** dstData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageGetDataBitmap")
	BOOL r = FALSE;
	if(_instance != NULL){
		r = _instance->pageGetDataBitmap(pageRef, dstProps, dstData);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrPdfKit::pageRetain(void* pageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageRetain")
	if(_instance != NULL){
		_instance->pageRetain(pageRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void NBMngrPdfKit::pageRelease(void* pageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageRelease")
	if(_instance != NULL){
		_instance->pageRelease(pageRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

STNBRect NBMngrPdfKit::pageGetBox(void* pageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageGetBox")
	STNBRect r;
	NBMemory_setZeroSt(r, STNBRect);
	if(_instance != NULL){
		r = _instance->pageGetBox(pageRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrPdfKit::pageGetLabel(void* pageRef, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageGetLabel")
	BOOL r = FALSE;
	if(_instance != NULL){
		r = _instance->pageGetLabel(pageRef, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrPdfKit::pageGetText(void* pageRef, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageGetText")
	BOOL r = FALSE;
	if(_instance != NULL){
		r = _instance->pageGetText(pageRef, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL NBMngrPdfKit::pageRenderArea(void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::pageRenderArea")
	BOOL r = FALSE;
	if(_instance != NULL){
		r = _instance->pageRenderArea(pageRef, area, dstSz, dst);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Render job

void* NBMngrPdfKit::renderStartToMemory(const STNBPdfRenderDoc* job){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::renderStartToMemory")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->renderStartToMemory(job);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

ENMngrPdfKitRenderResult NBMngrPdfKit::renderContinue(void* jobRef, const STNBPdfRenderDoc* job){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::renderContinue")
	ENMngrPdfKitRenderResult r = ENMngrPdfKitRenderResult_Error;
	if(_instance != NULL){
		r = _instance->renderContinue(jobRef, job);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void NBMngrPdfKit::renderCancel(void* jobRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::renderCancel")
	if(_instance != NULL){
		_instance->renderCancel(jobRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void* NBMngrPdfKit::renderEndAndOpenDoc(void* jobRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("NBMngrPdfKit::renderEndAndOpenDoc")
	void* r = NULL;
	if(_instance != NULL){
		r = _instance->renderEndAndOpenDoc(jobRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}
