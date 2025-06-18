//
//  AUMngrPdfKit.cpp
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUMngrPdfKit.h"
#include "nb/2d/NBPng.h"
#include "nb/2d/NBJpeg.h"

//Types

typedef enum ENMngrPdfKitDocType_ {
	ENMngrPdfKitDocType_Image = 0,		//Document is an image managed locally
	ENMngrPdfKitDocType_Glued,			//Document is redirected to the glued methods
	ENMngrPdfKitDocType_Count
} ENMngrPdfKitDocType;

typedef enum ENMngrPdfKitPageType_ {
	ENMngrPdfKitPageType_Image = 0,		//Page is an image managed locally
	ENMngrPdfKitPageType_Glued,			//Page is redirected to the glued methods
	ENMngrPdfKitPageType_Count
} ENMngrPdfKitPageType;

//Data pointers

typedef struct STMngrPdfKitDocRef_ {
	ENMngrPdfKitDocType		type;			//Type, defines the data-pointer's content
	void*					data;			//Data pointer
	STNBThreadMutex			mutex;			//Mutex for retain counts
	UI32					retainCount;	//Doc retainCount
} STMngrPdfKitDocRef;

//void AUMngrPdfKit_docRetain_(STMngrPdfKitDocRef* ref);
//void AUMngrPdfKit_docRelease_(STMngrPdfKitDocRef* ref);

typedef struct STMngrPdfKitPageRef_ {
	ENMngrPdfKitPageType	type;			//Type, defines the data-pointer's content
	void*					data;			//Data pointer
	STNBThreadMutex			mutex;			//Mutex for retain counts
	UI32					retainCount;	//Page retainCount
} STMngrPdfKitPageRef;

//void AUMngrPdfKit_pageRetain_(STMngrPdfKitPageRef* ref);
//void AUMngrPdfKit_pageRelease_(STMngrPdfKitPageRef* ref);

//Image, managed locally

typedef struct STMngrPdfKitImageDoc_ {
	STNBBitmap				image;			//Image loaded to memory
	STNBThreadMutex			mutex;			//Mutex for retain counts
	UI32					retainCount;	//Image retainCount
} STMngrPdfKitImageDoc;

//

STMngrPdfKitCalls AUMngrPdfKit::_calls = {
	NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	//
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
	, NULL, NULL
};

AUMngrPdfKit::AUMngrPdfKit() : AUObjeto(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::AUMngrPdfKit")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUMngrPdfKit")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	{
		//If fails, '_calls' initial values are not properly set to NULL.
		UI32 i; for(i = 0; i < sizeof(_calls); i++){
			NBASSERT(((BYTE*)&_calls)[i] == 0);
		}
	}
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUMngrPdfKit::~AUMngrPdfKit(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::~AUMngrPdfKit")
	//
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUMngrPdfKit::isGlued(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::isGlued")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (_calls.funcCreate != NULL);
}

bool AUMngrPdfKit::setGlue(AUAppI* app, PTRfuncPdfKitCreate initCall){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::setGlue")
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

//Specs

BOOL AUMngrPdfKit::canMultithread(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::canMultithread")
	BOOL r = FALSE;
	if(_calls.funcCanMultithread != NULL){
		r = (*_calls.funcCanMultithread)(_calls.funcCanMultithreadParam);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Document

void* AUMngrPdfKit::docOpenFromPath(const char* path){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docOpenFromPath")
	void* r = NULL;
	{
		void* rrData = NULL;
		ENMngrPdfKitDocType rrType = ENMngrPdfKitDocType_Count;
		BYTE hdr[64]; SI32 hdrRead = 0;
		NBMemory_setZero(hdr);
		//Load header: 64 bytes
		{
            STNBFileRef file = NBFile_alloc(NULL);
			if(NBFile_open(file, path, ENNBFileMode_Read)){
				NBFile_lock(file);
				hdrRead = NBFile_read(file, hdr, sizeof(hdr));
				NBFile_unlock(file);
			}
			NBFile_release(&file);
		}
		//Load
		if(hdrRead > 0){
			BOOL typeDetected = FALSE;
			//Try PNG header
			if(!typeDetected){
				//if(NBFilepath_isExtension(path, "png")) //Remove
				if(NBPng_dataStartsAsPng(hdr, hdrRead)){
					STNBBitmap bmp;
					NBBitmap_init(&bmp);
					if(!NBPng_loadFromPath(path, TRUE, &bmp, NULL)){
						PRINTF_ERROR("AUMngrPdfKit, could not parse PNG filepath '%s'.\n", path);
					} else {
						STMngrPdfKitImageDoc* ref = NBMemory_allocType(STMngrPdfKitImageDoc);
						NBMemory_setZeroSt(*ref, STMngrPdfKitImageDoc);
						NBBitmap_init(&ref->image);
						NBBitmap_swapData(&bmp, &ref->image);
						ref->retainCount = 1;
						NBThreadMutex_init(&ref->mutex);
						//
						rrType	= ENMngrPdfKitDocType_Image;
						rrData	= ref;
					}
					NBBitmap_release(&bmp);
					typeDetected = TRUE;
				}
			}
			//Try JPEG header
			if(!typeDetected){
				//if(NBFilepath_isExtension(path, "jpg") || NBFilepath_isExtension(path, "jpeg"))
				if(NBJpeg_dataStartsAsJpeg(hdr, hdrRead)){
					STNBBitmap bmp;
					NBBitmap_init(&bmp);
					if(!NBJpeg_loadFromPath(path, TRUE, &bmp)){
						PRINTF_ERROR("AUMngrPdfKit, could not parse JPG filepath '%s'.\n", path);
					} else {
						STMngrPdfKitImageDoc* ref = NBMemory_allocType(STMngrPdfKitImageDoc);
						NBMemory_setZeroSt(*ref, STMngrPdfKitImageDoc);
						NBBitmap_init(&ref->image);
						NBBitmap_swapData(&bmp, &ref->image);
						ref->retainCount = 1;
						NBThreadMutex_init(&ref->mutex);
						//
						rrType	= ENMngrPdfKitDocType_Image;
						rrData	= ref;
					}
					NBBitmap_release(&bmp);
					typeDetected = TRUE;
				}
			}
			//Try glued method
			if(!typeDetected){
				if(_calls.funcDocOpenPath != NULL){
					rrData = (*_calls.funcDocOpenPath)(_calls.funcDocOpenPathParam, path);
					rrType = ENMngrPdfKitDocType_Glued;
				}
			}
		}
		//Result
		if(rrData != NULL && rrType >= 0 && rrType < ENMngrPdfKitDocType_Count){
			STMngrPdfKitDocRef* rr = NBMemory_allocType(STMngrPdfKitDocRef);
			NBMemory_setZeroSt(*rr, STMngrPdfKitDocRef);
			rr->type		= rrType;
			rr->data		= rrData;
			rr->retainCount	= 1;
			NBThreadMutex_init(&rr->mutex);
			r = rr;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* AUMngrPdfKit::docOpenFromData(const void* data, const UI32 dataSz){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docOpenFromData")
	void* r = NULL;
	{
		void* rrData = NULL;
		ENMngrPdfKitDocType rrType = ENMngrPdfKitDocType_Count;
		//Load
		{
			BOOL typeDetected = FALSE;
			//Try PNG header
			if(!typeDetected){
				if(NBPng_dataStartsAsPng(data, dataSz)){
                    STNBFileRef file = NBFile_alloc(NULL);
					if(NBFile_openAsDataRng(file, (void*)data, dataSz)){
						NBFile_lock(file);
						{
							//PNG
							STNBBitmap bmp;
							NBBitmap_init(&bmp);
							if(!NBPng_loadFromFile(file, TRUE, &bmp, NULL)){
								PRINTF_ERROR("AUMngrPdfKit, could not parse PNG filedata '%d bytes'.\n", dataSz);
							} else {
								STMngrPdfKitImageDoc* ref = NBMemory_allocType(STMngrPdfKitImageDoc);
								NBMemory_setZeroSt(*ref, STMngrPdfKitImageDoc);
								NBBitmap_init(&ref->image);
								NBBitmap_swapData(&bmp, &ref->image);
								ref->retainCount = 1;
								NBThreadMutex_init(&ref->mutex);
								//
								rrType	= ENMngrPdfKitDocType_Image;
								rrData	= ref;
							}
							NBBitmap_release(&bmp);
						}
						NBFile_unlock(file);
					}
					NBFile_release(&file);
					typeDetected = TRUE;
				}
			}
			//Try JPEG header
			if(!typeDetected){
				if(NBJpeg_dataStartsAsJpeg(data, dataSz)){
                    STNBFileRef file =NBFile_alloc(NULL);
					if(NBFile_openAsDataRng(file, (void*)data, dataSz)){
						NBFile_lock(file);
						{
							STNBBitmap bmp;
							NBBitmap_init(&bmp);
							if(!NBJpeg_loadFromFile(file, TRUE, &bmp)){
								PRINTF_ERROR("AUMngrPdfKit, could not parse JPG filedata '%d bytes'.\n", dataSz);
							} else {
								STMngrPdfKitImageDoc* ref = NBMemory_allocType(STMngrPdfKitImageDoc);
								NBMemory_setZeroSt(*ref, STMngrPdfKitImageDoc);
								NBBitmap_init(&ref->image);
								NBBitmap_swapData(&bmp, &ref->image);
								ref->retainCount = 1;
								NBThreadMutex_init(&ref->mutex);
								//
								rrType	= ENMngrPdfKitDocType_Image;
								rrData	= ref;
							}
							NBBitmap_release(&bmp);
						}
						NBFile_unlock(file);
					}
					NBFile_release(&file);
					typeDetected = TRUE;
				}
			}
			//Try glued method
			if(!typeDetected){
				if(_calls.funcDocOpenData != NULL){
					rrData = (*_calls.funcDocOpenData)(_calls.funcDocOpenDataParam, data, dataSz);
					rrType = ENMngrPdfKitDocType_Glued;
				}
			}
		}
		//Result
		if(rrData != NULL && rrType >= 0 && rrType < ENMngrPdfKitDocType_Count){
			STMngrPdfKitDocRef* rr = NBMemory_allocType(STMngrPdfKitDocRef);
			NBMemory_setZeroSt(*rr, STMngrPdfKitDocRef);
			rr->type		= rrType;
			rr->data		= rrData;
			rr->retainCount	= 1;
			NBThreadMutex_init(&rr->mutex);
			r = rr;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* AUMngrPdfKit::docOpenFromRenderJob(const STNBPdfRenderDoc* job){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docOpenFromRenderJob")
	void* r = NULL;
	{
		void* rrData = NULL;
		ENMngrPdfKitDocType rrType = ENMngrPdfKitDocType_Count;
		//Load
		{
			if(_calls.funcDocOpenRenderJob != NULL){
				rrData = (*_calls.funcDocOpenRenderJob)(_calls.funcDocOpenRenderJobParam, job);
				rrType = ENMngrPdfKitDocType_Glued;
			}
		}
		//Result
		if(rrData != NULL && rrType >= 0 && rrType < ENMngrPdfKitDocType_Count){
			STMngrPdfKitDocRef* rr = NBMemory_allocType(STMngrPdfKitDocRef);
			NBMemory_setZeroSt(*rr, STMngrPdfKitDocRef);
			rr->type		= rrType;
			rr->data		= rrData;
			rr->retainCount	= 1;
			NBThreadMutex_init(&rr->mutex);
			r = rr;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* AUMngrPdfKit::docGetDataGlued(void* pDocRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docGetDataGlued")
	void* r = NULL;
	if(pDocRef != NULL){
		STMngrPdfKitDocRef* ref = (STMngrPdfKitDocRef*)pDocRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitDocType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitDocType_Image:
				//Nothing
				break;
			case ENMngrPdfKitDocType_Glued:
				r = ref->data;
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitDocType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

/*void AUMngrPdfKit_docRetain_(STMngrPdfKitDocRef* ref){
	//
}

void AUMngrPdfKit_docRelease_(STMngrPdfKitDocRef* ref){
	//
}

void AUMngrPdfKit_pageRetain_(STMngrPdfKitPageRef* ref){
	//
}

void AUMngrPdfKit_pageRelease_(STMngrPdfKitPageRef* ref){
	//
}*/

void AUMngrPdfKit::docRetain(void* pDocRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docRetain")
	if(pDocRef != NULL){
		STMngrPdfKitDocRef* ref = (STMngrPdfKitDocRef*)pDocRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitDocType_Count)
		{
			NBThreadMutex_lock(&ref->mutex);
			NBASSERT(ref->retainCount > 0)
			ref->retainCount++;
			NBThreadMutex_unlock(&ref->mutex);
		}
		//Use
		switch (ref->type) {
			case ENMngrPdfKitDocType_Image:
				//Nothing
				break;
			case ENMngrPdfKitDocType_Glued:
				if(_calls.funcDocRetain != NULL){
					(*_calls.funcDocRetain)(_calls.funcDocRetainParam, ref->data);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitDocType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrPdfKit::docRelease(void* pDocRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docRelease")
	if(pDocRef != NULL){
		STMngrPdfKitDocRef* ref = (STMngrPdfKitDocRef*)pDocRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitDocType_Count)
		{
			NBThreadMutex_lock(&ref->mutex);
			NBASSERT(ref->retainCount > 0)
			ref->retainCount--;
			NBThreadMutex_unlock(&ref->mutex);
		}
		switch (ref->type) {
			case ENMngrPdfKitDocType_Image:
				if(ref->retainCount == 0){
					NBASSERT(ref->data != NULL)
					if(ref->data != NULL){
						STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
						NBThreadMutex_lock(&img->mutex);
						NBASSERT(img->retainCount > 0)
						img->retainCount--;
						if(img->retainCount > 0){
							NBThreadMutex_unlock(&img->mutex);
						} else if(img->retainCount == 0){
							NBThreadMutex_unlock(&img->mutex);
							NBThreadMutex_release(&img->mutex);
							NBBitmap_release(&img->image);
							NBMemory_setZeroSt(*img, STMngrPdfKitImageDoc);
							NBMemory_free(img);
						}
						ref->data = NULL;
					}
				}
				break;
			case ENMngrPdfKitDocType_Glued:
				if(_calls.funcDocRelease != NULL){
					(*_calls.funcDocRelease)(_calls.funcDocReleaseParam, ref->data);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitDocType.\n")
				break;
		}
		NBASSERT(ref->retainCount >= 0) //If fails, mutex while retain/release of doc/page failed
		if(ref->retainCount == 0){
			NBThreadMutex_release(&ref->mutex);
			NBMemory_setZeroSt(*ref, STMngrPdfKitDocRef);
			NBMemory_free(ref);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

UI32 AUMngrPdfKit::docGetPagesCount(void* pDocRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docGetPagesCount")
	UI32 r = 0;
	if(pDocRef != NULL){
		STMngrPdfKitDocRef* ref = (STMngrPdfKitDocRef*)pDocRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitDocType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitDocType_Image:
				NBASSERT(ref->data != NULL)
				if(ref->data != NULL){
					STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
					void* bmpData = NBBitmap_getData(&img->image);
					if(bmpData != NULL){
						r = 1; //Images are only one page
					}
				}
				break;
			case ENMngrPdfKitDocType_Glued:
				if(_calls.funcDocGetPagesCount != NULL){
					r = (*_calls.funcDocGetPagesCount)(_calls.funcDocGetPagesCountParam, ref->data);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitDocType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void* AUMngrPdfKit::docGetPageAtIdx(void* pDocRef, const UI32 idx){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docGetPageAtIdx")
	void* r = NULL;
	void* rrData = NULL;
	ENMngrPdfKitPageType rrType = ENMngrPdfKitPageType_Count;
	{
		//Load
		if(pDocRef != NULL){
			STMngrPdfKitDocRef* ref = (STMngrPdfKitDocRef*)pDocRef;
			NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitDocType_Count)
			NBASSERT(ref->retainCount > 0)
			switch (ref->type) {
				case ENMngrPdfKitDocType_Image:
					NBASSERT(ref->data != NULL)
					if(ref->data != NULL){
						STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
						NBThreadMutex_lock(&img->mutex);
						NBASSERT(img->retainCount > 0)
						{
							void* bmpData = NBBitmap_getData(&img->image);
							if(bmpData != NULL){
								if(idx == 0){
									img->retainCount++;
									rrData	= img;
									rrType	= ENMngrPdfKitPageType_Image;
								}
							}
						}
						NBThreadMutex_unlock(&img->mutex);
					}
					break;
				case ENMngrPdfKitDocType_Glued:
					if(_calls.funcDocGetPageAtIdx != NULL){
						rrData	= (*_calls.funcDocGetPageAtIdx)(_calls.funcDocGetPageAtIdxParam, ref->data, idx);
						rrType	= ENMngrPdfKitPageType_Glued;
					}
					break;
				default:
					PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitDocType.\n")
					break;
			}
			NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
		}
		//Result
		if(rrData != NULL && rrType >= 0 && rrType < ENMngrPdfKitPageType_Count){
			STMngrPdfKitPageRef* rr = NBMemory_allocType(STMngrPdfKitPageRef);
			NBMemory_setZeroSt(*rr, STMngrPdfKitPageRef);
			rr->type		= rrType;
			rr->data		= rrData;
			rr->retainCount	= 1;
			NBThreadMutex_init(&rr->mutex);
			r = rr;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrPdfKit::docWriteToFilepath(void* pDocRef, const char* filePath){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::docWriteToFilepath")
	BOOL r = FALSE;
	if(pDocRef != NULL){
		STMngrPdfKitDocRef* ref = (STMngrPdfKitDocRef*)pDocRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitDocType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitDocType_Image:
				NBASSERT(ref->data != NULL)
				if(ref->data != NULL){
					STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
					void* bmpData = NBBitmap_getData(&img->image);
					if(bmpData != NULL){
						if(NBFilepath_isExtension(filePath, "png")){
							if(NBPng_saveToPath(&img->image, filePath, ENPngCompressLvl_5)){
								r = TRUE;
							}
						} else if(NBFilepath_isExtension(filePath, "jpg") || NBFilepath_isExtension(filePath, "jpeg")){
							if(NBJpeg_saveToPath(&img->image, filePath, 85, 10)){
								r = TRUE;
							}
						} else {
							PRINTF_ERROR("AUMngrPdfKit, unexpected file extension: '%s'.\n", filePath);
						}
					}
				}
				break;
			case ENMngrPdfKitDocType_Glued:
				if(_calls.funcDocWriteToFilepath != NULL){
					r = (*_calls.funcDocWriteToFilepath)(_calls.funcDocWriteToFilepathParam, ref->data, filePath);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitDocType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Page

void* AUMngrPdfKit::pageGetDataGluedInner(void* pPageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pagetGetData")
	void* r = NULL;
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				//Nothing
				break;
			case ENMngrPdfKitPageType_Glued:
				r = ref->data;
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrPdfKit::pageGetDataBitmap(void* pPageRef, STNBBitmapProps* dstProps, void** dstData){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pagetGetData")
	BOOL r = FALSE;
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				NBASSERT(ref->data != NULL)
				if(ref->data != NULL){
					STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
					NBThreadMutex_lock(&img->mutex);
					NBASSERT(img->retainCount > 0)
					{
						void* bmpData = NBBitmap_getData(&img->image);
						if(bmpData != NULL){
							if(dstProps != NULL) *dstProps = NBBitmap_getProps(&img->image);
							if(dstData != NULL) *dstData = bmpData;
							r = TRUE;
						}
					}
					NBThreadMutex_unlock(&img->mutex);
				}
				break;
			case ENMngrPdfKitPageType_Glued:
				//Nothing
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrPdfKit::pageRetain(void* pPageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pageRetain")
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		{
			NBThreadMutex_lock(&ref->mutex);
			NBASSERT(ref->retainCount > 0)
			ref->retainCount++;
			NBThreadMutex_unlock(&ref->mutex);
		}
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				//Nothing
				break;
			case ENMngrPdfKitPageType_Glued:
				if(_calls.funcPageRetain != NULL){
					(*_calls.funcPageRetain)(_calls.funcPageRetainParam, ref->data);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUMngrPdfKit::pageRelease(void* pPageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pageRelease")
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		{
			NBThreadMutex_lock(&ref->mutex);
			NBASSERT(ref->retainCount > 0)
			ref->retainCount--;
			NBThreadMutex_unlock(&ref->mutex);
		}
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				if(ref->retainCount == 0){
					NBASSERT(ref->data != NULL)
					if(ref->data != NULL){
						STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
						NBThreadMutex_lock(&img->mutex);
						NBASSERT(img->retainCount > 0)
						img->retainCount--;
						if(img->retainCount > 0){
							NBThreadMutex_unlock(&img->mutex);
						} else if(img->retainCount == 0){
							NBThreadMutex_unlock(&img->mutex);
							NBThreadMutex_release(&img->mutex);
							NBBitmap_release(&img->image);
							NBMemory_setZeroSt(*img, STMngrPdfKitImageDoc);
							NBMemory_free(img);
						}
						ref->data = NULL;
					}
				}
				break;
			case ENMngrPdfKitPageType_Glued:
				if(_calls.funcPageRelease != NULL){
					(*_calls.funcPageRelease)(_calls.funcPageReleaseParam, ref->data);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount >= 0) //If fails, mutex while retain/release of doc/page failed
		if(ref->retainCount == 0){
			NBThreadMutex_release(&ref->mutex);
			NBMemory_setZeroSt(*ref, STMngrPdfKitPageRef);
			NBMemory_free(ref);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

STNBRect AUMngrPdfKit::pageGetBox(void* pPageRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pageGetBox")
	//AU_MNGR_PDFKIT_CALL_START //Not necesary, multithreada allowed
	STNBRect r;
	NBMemory_setZeroSt(r, STNBRect);
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				NBASSERT(ref->data != NULL)
				if(ref->data != NULL){
					STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
					const STNBBitmapProps props = NBBitmap_getProps(&img->image);
					r.x			= r.y = 0;
					r.width		= props.size.width;
					r.height	= props.size.height;
				}
				break;
			case ENMngrPdfKitPageType_Glued:
				if(_calls.funcPageGetBox != NULL){
					r = (*_calls.funcPageGetBox)(_calls.funcPageGetBoxParam, ref->data);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	//AU_MNGR_PDFKIT_CALL_END //Not necesary, multithreada allowed
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrPdfKit::pageGetLabel(void* pPageRef, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pageGetLabel")
	//AU_MNGR_PDFKIT_CALL_START //Not necesary, multithreada allowed
	BOOL r = FALSE;
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				if(dst != NULL) NBString_set(dst, "1");
				r = TRUE;
				break;
			case ENMngrPdfKitPageType_Glued:
				if(_calls.funcPageGetLabel != NULL){
					r = (*_calls.funcPageGetLabel)(_calls.funcPageGetLabelParam, ref->data, dst);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	//AU_MNGR_PDFKIT_CALL_END //Not necesary, multithreada allowed
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrPdfKit::pageGetText(void* pPageRef, STNBString* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pageGetText")
	//AU_MNGR_PDFKIT_CALL_START //Not necesary, multithreada allowed
	BOOL r = FALSE;
	if(pPageRef != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitPageType_Glued:
				if(_calls.funcPageGetText != NULL){
					r = (*_calls.funcPageGetText)(_calls.funcPageGetTextParam, ref->data, dst);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	//AU_MNGR_PDFKIT_CALL_END //Not necesary, multithreada allowed
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

BOOL AUMngrPdfKit::pageRenderArea(void* pPageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::pageRenderArea")
	//AU_MNGR_PDFKIT_CALL_START //Not necesary, multithreada allowed
	BOOL r = FALSE;
	if(pPageRef != NULL && dst != NULL){
		STMngrPdfKitPageRef* ref = (STMngrPdfKitPageRef*)pPageRef;
		NBASSERT(ref->type >= 0 && ref->type < ENMngrPdfKitPageType_Count)
		NBASSERT(ref->retainCount > 0)
		switch (ref->type) {
			case ENMngrPdfKitPageType_Image:
				NBASSERT(ref->data != NULL)
				if(ref->data != NULL){
					STMngrPdfKitImageDoc* img = (STMngrPdfKitImageDoc*)ref->data;
					//Prepare buffer
					{
						const BYTE* data = NBBitmap_getData(dst);
						const STNBBitmapProps props = NBBitmap_getProps(dst);
						const STNBBitmapProps srcProps = NBBitmap_getProps(&img->image);
						if(data == NULL || props.size.width != dstSz.width || props.size.height != dstSz.height || props.color != srcProps.color){
							NBBitmap_createAndSet(dst, dstSz.width, dstSz.height, srcProps.color, 255);
						}
					}
					//Render
					{
						//Faster method (for non rotated images)
						STNBColor8 color8; color8.r = color8.g = color8.b = color8.a = 255;
						STNBRectI dstRect; dstRect.x = 0; dstRect.y = 0; dstRect.width = dstSz.width; dstRect.height = dstSz.height;
						STNBRectI srcRect; srcRect.x = area.x; srcRect.y = area.y; srcRect.width = area.width; srcRect.height = area.height;
						if(NBBitmap_pasteBitmapScaledRect(dst, dstRect, &img->image, srcRect, color8)){
							r = TRUE;
						}
						//Universal method (slower, includes rotation)
						/*
						const float rotRad = 0.0f;
						STNBPoint posCenter; posCenter.x = dstSz.width / 2.0f; posCenter.y = dstSz.height / 2.0f;
						STNBSize scaleRel; scaleRel.width = dstSz.width / area.width; scaleRel.height = dstSz.height / area.height;
						STNBColor8 color8; color8.r = color8.g = color8.b = color8.a = 255;
						if(NBBitmap_drawBitmapRect(dst, &img->image, area, posCenter, rotRad, scaleRel, color8)){
							r = TRUE;
						}
						*/
					}
				}
				break;
			case ENMngrPdfKitPageType_Glued:
				if(_calls.funcPageRenderArea != NULL){
					r = (*_calls.funcPageRenderArea)(_calls.funcPageRenderAreaParam, ref->data, area, dstSz, dst);
				}
				break;
			default:
				PRINTF_ERROR("AUMngrPdfKit, unexpected ENMngrPdfKitPageType.\n")
				break;
		}
		NBASSERT(ref->retainCount > 0) //If fails, mutex while retain/release of doc/page failed
	}
	//AU_MNGR_PDFKIT_CALL_END //Not necesary, multithreada allowed
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//Render job

void* AUMngrPdfKit::renderStartToMemory(const STNBPdfRenderDoc* job){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::renderStartToMemory")
	void* r = NULL;
	if(_calls.funcRenderStartToMemory != NULL){
		r = (*_calls.funcRenderStartToMemory)(_calls.funcRenderStartToMemoryParam, job);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

ENMngrPdfKitRenderResult AUMngrPdfKit::renderContinue(void* jobRef, const STNBPdfRenderDoc* job){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::renderContinue")
	ENMngrPdfKitRenderResult r = ENMngrPdfKitRenderResult_Error;
	if(_calls.funcRenderContinue != NULL){
		r = (*_calls.funcRenderContinue)(_calls.funcRenderContinueParam, jobRef, job);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

void AUMngrPdfKit::renderCancel(void* jobRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::renderCancel")
	if(_calls.funcRenderCancel != NULL){
		(*_calls.funcRenderCancel)(_calls.funcRenderCancelParam, jobRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void* AUMngrPdfKit::renderEndAndOpenDoc(void* jobRef){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUMngrPdfKit::renderEndAndOpenDoc")
	void* r = NULL;
	if(_calls.funcRenderEndAndOpenDoc != NULL){
		r = (*_calls.funcRenderEndAndOpenDoc)(_calls.funcRenderEndAndOpenDocParam, jobRef);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUMngrPdfKit)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUMngrPdfKit, "AUMngrPdfKit")
AUOBJMETODOS_CLONAR_NULL(AUMngrPdfKit)

