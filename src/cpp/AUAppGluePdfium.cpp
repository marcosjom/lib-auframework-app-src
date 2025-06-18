//
//  AUAppGluePdfium.mm
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGluePdfium.h"
//
#include "NBMngrPdfKit.h"
//
#include "pdfium/fpdfview.h"			//
#include "pdfium/fpdf_doc.h"			//For 'FPDF_GetPageLabel'.
#include "pdfium/fpdf_transformpage.h"	//For 'FPDFPage_GetMediaBox'.
#include "pdfium/fpdf_edit.h"			//For document creation.
#include "pdfium/fpdf_ppo.h"			//For copying pages form one document to other.
#include "pdfium/fpdf_save.h"			//For saving document.
#include "pdfium/fpdf_text.h"			//For pageGetText
//Note: PDFium is not thread-safe,
//only one page should be rendering at time.

typedef enum ENAppGluePdfiumDataType_ {
	ENAppGluePdfiumDataType_Unknown = 0,
	ENAppGluePdfiumDataType_Doc,
	ENAppGluePdfiumDataType_Page,
	ENAppGluePdfiumDataType_Render
} ENAppGluePdfiumDataType;

typedef struct AUAppGluePdfiumData_ {
	AUAppI*			app;
	STNBThreadMutex	pdfiumMutex;
} AUAppGluePdfiumData;

struct AUAppGluePdfiumDoc_;

typedef struct AUAppGluePdfiumPage_ {
	ENAppGluePdfiumDataType 	type;
	AUAppGluePdfiumData*		glue;
	STNBThreadMutex				mutex;
	struct AUAppGluePdfiumDoc_* doc;
	//Cache
	struct {
		SI32					idx;		//last known index (can change when file is updated)
		STNBString				lbl;		//last known label (can change when file is updated)
		STNBRect				mediaBox;	//last known mediabox (is the biggest and only-one-required of all boxes)
	} cache;
	//
	FPDF_PAGE					page;
	//
	SI32						retainCount;
} AUAppGluePdfiumPage;

typedef struct AUAppGluePdfiumDoc_ {
	ENAppGluePdfiumDataType 	type;
	AUAppGluePdfiumData*		glue;
	STNBThreadMutex				mutex;
	STNBArray					pages;	//AUAppGluePdfiumPage*
	STNBString					filename; //if was loaded from file
	//Cache
	struct {
		SI32					pagesCount;		//last known count (can change when file is updated)
	} cache;
	//
	FPDF_DOCUMENT				doc;
	//
	SI32						retainCount;
} AUAppGluePdfiumDoc;

typedef struct AUAppGluePdfiumRender_ {
	ENAppGluePdfiumDataType		type;
	AUAppGluePdfiumData*		glue;
	STNBThreadMutex				mutex;
	//
	FPDF_DOCUMENT				doc;
	FPDF_FONT					fontContent;	//font for content
	UI32						iNextPage;		//current progress
	//
	SI32						retainCount;
} AUAppGluePdfiumRender;

//Calls

bool AUAppGluePdfium::create(AUAppI* app, STMngrPdfKitCalls* obj){
	//Library
	FPDF_InitLibrary();
	//Data
	AUAppGluePdfiumData* data			= NBMemory_allocType(AUAppGluePdfiumData);
	NBMemory_setZeroSt(*data, AUAppGluePdfiumData);
	NBMemory_setZeroSt(*obj, STMngrPdfKitCalls);
	data->app							= (AUAppI*)app;
	NBThreadMutex_init(&data->pdfiumMutex);
	//
	obj->funcCreate						= create;
	obj->funcCreateParam				= data;
	obj->funcDestroy					= destroy;
	obj->funcDestroyParam				= data;
	//Specs
	obj->funcCanMultithread				= canMultithread;
	obj->funcCanMultithreadParam		= data;
	//Document
	obj->funcDocOpenPath				= docOpenFromPath;
	obj->funcDocOpenPathParam			= data;
	obj->funcDocOpenData				= docOpenFromData;
	obj->funcDocOpenDataParam			= data;
	obj->funcDocOpenRenderJob			= docOpenFromRenderJob;
	obj->funcDocOpenRenderJobParam		= data;
	obj->funcDocRetain					= docRetain;
	obj->funcDocRetainParam				= data;
	obj->funcDocRelease					= docRelease;
	obj->funcDocReleaseParam			= data;
	obj->funcDocGetPagesCount			= docGetPagesCount;
	obj->funcDocGetPagesCountParam		= data;
	obj->funcDocGetPageAtIdx			= docGetPageAtIdx;
	obj->funcDocGetPageAtIdxParam		= data;
	obj->funcDocWriteToFilepath			= docWriteToFilepath;
	obj->funcDocWriteToFilepathParam	= data;
	//Page
	obj->funcPageRetain					= pageRetain;
	obj->funcPageRetainParam			= data;
	obj->funcPageRelease				= pageRelease;
	obj->funcPageReleaseParam			= data;
	obj->funcPageGetBox					= pageGetBox;
	obj->funcPageGetBoxParam			= data;
	obj->funcPageGetLabel				= pageGetLabel;
	obj->funcPageGetLabelParam			= data;
	obj->funcPageGetText				= pageGetText;
	obj->funcPageGetTextParam			= data;
	obj->funcPageRenderArea				= pageRenderArea;
	obj->funcPageRenderAreaParam		= data;
	//Render
	obj->funcRenderStartToMemory		= renderStartToMemory;
	obj->funcRenderStartToMemoryParam	= data;
	obj->funcRenderContinue				= renderContinue;
	obj->funcRenderContinueParam		= data;
	obj->funcRenderCancel				= renderCancel;
	obj->funcRenderCancelParam			= data;
	obj->funcRenderEndAndOpenDoc		= renderEndAndOpenDoc;
	obj->funcRenderEndAndOpenDocParam	= data;
	//
	return true;
}

bool AUAppGluePdfium::destroy(void* param){
	bool r = false;
	AUAppGluePdfiumData* data = (AUAppGluePdfiumData*)param;
	if(data != NULL){
		NBThreadMutex_release(&data->pdfiumMutex);
		NBMemory_free(data);
		r = true;
		//Library
		FPDF_DestroyLibrary();
	}
	return r;
}

//Specs

BOOL AUAppGluePdfium::canMultithread(void* param){
	return FALSE; //PDFium is not thread-safe, only one PDFium-action per process
}

//Document

void* AUAppGluePdfium::docOpenFromPath(void* param, const char* path){
	void* r = NULL;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL && path != NULL){
		//Testing if multiple files can be loaded in parallel
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			FPDF_DOCUMENT doc = FPDF_LoadDocument(path, NULL);
			if(doc != NULL){
				AUAppGluePdfiumDoc* dd = NBMemory_allocType(AUAppGluePdfiumDoc);
				NBMemory_setZeroSt(*dd, AUAppGluePdfiumDoc);
				dd->type		= ENAppGluePdfiumDataType_Doc;
				dd->glue		= obj;
				dd->doc			= doc;
				dd->retainCount	= 1;
				NBString_init(&dd->filename); //if was loaded from file
				//Filename
				{
					const SI32 pathLen	= NBString_strLenBytes(path);
					const SI32 iSlash1	= NBString_strLastIndexOf(path, "/", pathLen);
					const SI32 iSlash2	= NBString_strLastIndexOf(path, "\\", pathLen);
					const SI32 iSlash	= (iSlash1 > iSlash2 ? iSlash1 : iSlash2);
					NBString_concat(&dd->filename, &path[iSlash]);
				}
				NBArray_init(&dd->pages, sizeof(AUAppGluePdfiumPage*), NULL);
				NBThreadMutex_init(&dd->mutex);
				//Cache
				{
					dd->cache.pagesCount = (SI32)FPDF_GetPageCount(dd->doc);
				}
				//PRINTF_INFO("Opened doc(%llu)-pdfDoc(%llu) ('%s').\n", (UI64)dd, (UI64)dd->doc, dd->filename.str);
				r = dd;
			}
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

void* AUAppGluePdfium::docOpenFromData(void* param, const void* data, const UI32 dataSz){
	void* r = NULL;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL){
		//Testing if multiple files can be loaded in parallel
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			FPDF_DOCUMENT doc = FPDF_LoadMemDocument(data, dataSz, NULL);
			if(doc != NULL){
				AUAppGluePdfiumDoc* dd = NBMemory_allocType(AUAppGluePdfiumDoc);
				NBMemory_setZeroSt(*dd, AUAppGluePdfiumDoc);
				dd->type		= ENAppGluePdfiumDataType_Doc;
				dd->glue		= obj;
				dd->doc			= doc;
				dd->retainCount	= 1;
				NBString_init(&dd->filename); //if was loaded from file
				NBArray_init(&dd->pages, sizeof(AUAppGluePdfiumPage*), NULL);
				NBThreadMutex_init(&dd->mutex);
				//Cache
				{
					dd->cache.pagesCount = (SI32)FPDF_GetPageCount(dd->doc);
				}
				//PRINTF_INFO("Opened doc(%llu)-pdfDoc(%llu) ('%s').\n", (UI64)dd, (UI64)dd->doc, dd->filename.str);
				r = dd;
			}
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

BOOL AUAppGluePdfium_renderPage(FPDF_DOCUMENT doc, FPDF_FONT* fontContentRef, const STNBPdfRenderDoc* job, const STNBPdfRenderPage* page, const SI32 iPage);

void* AUAppGluePdfium::docOpenFromRenderJob(void* param, const STNBPdfRenderDoc* job){
	void* r = NULL;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL && job != NULL){
		NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
			if(doc != NULL){
				BOOL error = FALSE;
				FPDF_FONT fontContent = NULL;
				//Render pages
				if(!error){
					SI32 i; for(i = 0; i < job->pages.use && !error; i++){
						const STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&job->pages, STNBPdfRenderPage*, i);
						if(!AUAppGluePdfium_renderPage(doc, &fontContent, job, page, i)){
							error = TRUE;
						}
					}
				}
				//
				if(fontContent != NULL){
					FPDFFont_Close(fontContent);
					fontContent = NULL;
				}
				//
				if(error){
					FPDF_CloseDocument(doc);
					doc = NULL;
				} else {
					AUAppGluePdfiumDoc* dd = NBMemory_allocType(AUAppGluePdfiumDoc);
					NBMemory_setZeroSt(*dd, AUAppGluePdfiumDoc);
					dd->type		= ENAppGluePdfiumDataType_Doc;
					dd->glue		= obj;
					dd->doc			= doc;
					dd->retainCount	= 1;
					NBString_init(&dd->filename); //if was loaded from file
					NBArray_init(&dd->pages, sizeof(AUAppGluePdfiumPage*), NULL);
					NBThreadMutex_init(&dd->mutex);
					//Cache
					{
						dd->cache.pagesCount = (SI32)FPDF_GetPageCount(dd->doc);
					}
					//PRINTF_INFO("Opened doc(%llu)-pdfDoc(%llu) ('%s').\n", (UI64)dd, (UI64)dd->doc, dd->filename.str);
					r = dd;
				}
			}
		}
		NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

void AUAppGluePdfium::docRetain(void* param, void* docRef){
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumDoc* dd = (AUAppGluePdfiumDoc*)docRef;
	if(obj != NULL && dd != NULL){
		NBASSERT(dd->type == ENAppGluePdfiumDataType_Doc)
		//AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)dd->glue;
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			NBThreadMutex_lock(&dd->mutex);
			NBASSERT(dd->retainCount > 0)
			dd->retainCount++;
			NBThreadMutex_unlock(&dd->mutex);
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
}

void AUAppGluePdfium_docReleaseAndFreeLocked_(AUAppGluePdfiumDoc* dd){
	if(dd != NULL){
		//PRINTF_INFO("Freeing doc(%llu)-pdfDoc(%llu) ('%s').\n", (UI64)dd, (UI64)dd->doc, dd->filename.str);
		{
			//Release pages
			{
				NBASSERT(dd->pages.use == 0) //Should be released after no pages remain
				NBArray_release(&dd->pages);
			}
			if(dd->doc != NULL){
				FPDF_CloseDocument(dd->doc);
				dd->doc = NULL;
			}
			dd->glue = NULL;
			dd->type = ENAppGluePdfiumDataType_Unknown;
			NBString_release(&dd->filename); //if was loaded from file
			NBThreadMutex_unlock(&dd->mutex);
			NBThreadMutex_release(&dd->mutex);
			NBMemory_setZeroSt(*dd, AUAppGluePdfiumDoc);
			NBMemory_free(dd);
		}
	}
}

void AUAppGluePdfium::docRelease(void* param, void* docRef){
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumDoc* dd = (AUAppGluePdfiumDoc*)docRef;
	if(obj != NULL && dd != NULL){
		NBASSERT(dd->type == ENAppGluePdfiumDataType_Doc)
		AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)dd->glue;
		//Testing if multiple files can be loaded in parallel
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			NBThreadMutex_lock(&dd->mutex);
			NBASSERT(dd->retainCount > 0)
			dd->retainCount--;
			if(dd->retainCount > 0){
				NBThreadMutex_unlock(&dd->mutex);
			} else {
				AUAppGluePdfium_docReleaseAndFreeLocked_(dd);
			}
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
}

UI32 AUAppGluePdfium::docGetPagesCount(void* param, void* docRef){
	UI32 r = 0;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumDoc* dd = (AUAppGluePdfiumDoc*)docRef;
	if(obj != NULL && dd != NULL){
		NBASSERT(dd->type == ENAppGluePdfiumDataType_Doc)
		//AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)dd->glue; //Do not lock
		//NBThreadMutex_lock(&obj->pdfiumMutex); //Do not lock
		{
			//NBThreadMutex_lock(&dd->mutex); //Do not lock
			r = (UI32)dd->cache.pagesCount;
			//NBThreadMutex_unlock(&dd->mutex); //Do not lock
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //Do not lock
	}
	return r;
}

void* AUAppGluePdfium::docGetPageAtIdx(void* param, void* docRef, const UI32 idx){
	void* r = NULL;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumDoc* dd = (AUAppGluePdfiumDoc*)docRef;
	if(obj != NULL && dd != NULL){
		NBASSERT(dd->type == ENAppGluePdfiumDataType_Doc)
		AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)dd->glue;
		//Testing if multiple files can be loaded in parallel
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			NBThreadMutex_lock(&dd->mutex);
			if(dd->doc != NULL){
				//Search in current array
				if(r == NULL){
					UI32 i; for(i = 0 ; i < dd->pages.use && r == NULL; i++){
						AUAppGluePdfiumPage* pp = NBArray_itmValueAtIndex(&dd->pages, AUAppGluePdfiumPage*, i);
						NBASSERT(pp->type == ENAppGluePdfiumDataType_Page || pp->type == ENAppGluePdfiumDataType_Unknown);
						//when releasing the object is marked as unknown
						if(pp->type == ENAppGluePdfiumDataType_Page){
							NBThreadMutex_lock(&pp->mutex);
							//Update index (if necesary)
							/*if(pp->cache.idx < 0){
							 const NSUInteger idx = [dd->doc indexForPage:pp->page];
							 if(idx != NSNotFound){
							 pp->cache.idx = (SI32)idx;
							 }
							 }*/
							//
							if(pp->cache.idx == idx){
								pp->retainCount++; //increase retain
								//PRINTF_INFO("PdfPage(%llu) #%d found in already loaded ('%s').\n", (UI64)pp, (idx + 1), dd->filename.str);
								r = pp;
							}
							NBThreadMutex_unlock(&pp->mutex);
						}
					}
				}
				//Create new page
				if(r == NULL){
					const UI32 pagesCount = (UI32)FPDF_GetPageCount(dd->doc);
					if(idx >= 0 && idx < pagesCount){
						FPDF_PAGE page = FPDF_LoadPage(dd->doc, idx);
						if(page != NULL){
							AUAppGluePdfiumPage* pp = NBMemory_allocType(AUAppGluePdfiumPage);
							NBMemory_setZeroSt(*pp, AUAppGluePdfiumPage);
							pp->type		= ENAppGluePdfiumDataType_Page;
							pp->glue		= obj;
							pp->doc			= dd;
							pp->page		= page;
							//PRINTF_INFO("Loaded page(%llu)-pdfpage(%llu): w(%f)-h(%f).\n", (UI64)pp, (UI64)pp->page, FPDF_GetPageWidth(pp->page), FPDF_GetPageHeight(pp->page));
							//Cache
							{
								//Index
								pp->cache.idx	= idx;
								//Box
								{
									float left = 0.0f, bottom = 0.0f, right = 0.0f, top = 0.0f;
									if(FPDFPage_GetMediaBox(page, &left, &bottom, &right, &top)){
										pp->cache.mediaBox.x		= left;
										pp->cache.mediaBox.y		= bottom; NBASSERT(bottom <= top)
										pp->cache.mediaBox.width	= (right - left);
										pp->cache.mediaBox.height	= (top - bottom);
									}
								}
								//Label
								{
									NBString_init(&pp->cache.lbl);
									{
										unsigned short bufUtf16[128];
										unsigned long len = FPDF_GetPageLabel(pp->doc->doc, pp->cache.idx, bufUtf16, sizeof(bufUtf16));
										if(len > 0){
											NBString_concatUtf16(&pp->cache.lbl, bufUtf16);
										} else {
											NBString_concatSI32(&pp->cache.lbl, pp->cache.idx);
										}
									}
								}
							}
							pp->retainCount	= 1;
							NBThreadMutex_init(&pp->mutex);
							//UPdate doc
							{
								//Add to array
								dd->retainCount++;	//page retains document
								NBArray_addValue(&dd->pages, pp);
							}
							//PRINTF_INFO("PdfPage(%llu) #%d newly loaded ('%s').\n", (UI64)pp, (idx + 1), dd->filename.str);
							r = pp;
						}
					}
				}
			}
			NBThreadMutex_unlock(&dd->mutex);
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

//Write to PDF

typedef struct STNBPdfiumWriteBlock_ {
	FPDF_FILEWRITE	base;	//note: must be the first element in the struct (shared pointer)
	STNBFileRef		file;
} STNBPdfiumWriteBlock;

int AUAppGluePdfium_WriteBlock(struct FPDF_FILEWRITE_* pThis, const void* pData, unsigned long size){
	int r = 0;
	if(pThis != NULL){
		STNBPdfiumWriteBlock* bb = (STNBPdfiumWriteBlock*)pThis;
		NBASSERT(bb->base.version == 1)
		NBASSERT(bb->base.WriteBlock == AUAppGluePdfium_WriteBlock)
		if(bb->file != NULL){
			if(NBFile_write(bb->file, pData, size, 1)){
				r = size;
			}
		}
	}
	return r;
}

BOOL AUAppGluePdfium::docWriteToFilepath(void* param, void* docRef, const char* filepath){
	BOOL r = FALSE;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumDoc* dd = (AUAppGluePdfiumDoc*)docRef;
	if(obj != NULL && dd != NULL){
        STNBFileRef file = NBFile_alloc(NULL);
		if(!NBFile_open(&file, filepath, ENNBFileMode_Write)){
			PRINTF_ERROR("AUAppGluePdfium, could not open file for write: '%s'.\n", filepath);
		} else {
			NBFile_lock(file);
			{
				AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)dd->glue;
				//Testing if multiple files can be loaded in parallel
				//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
				{
					NBThreadMutex_lock(&dd->mutex);
					{
						STNBPdfiumWriteBlock bb;
						NBMemory_setZeroSt(bb, STNBPdfiumWriteBlock);
						bb.base.version		= 1;
						bb.base.WriteBlock	= AUAppGluePdfium_WriteBlock;
						bb.file				= &file;
						if(FPDF_SaveAsCopy(dd->doc, &bb.base, 0)){
							r = TRUE;
						}
					}
					NBThreadMutex_unlock(&dd->mutex);
				}
				//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
			}
			NBFile_unlock(file);
			NBFile_close(file);
		}
		NBFile_release(&file);
	}
	return r;
}

//Page

void AUAppGluePdfium_pageRetainLocked_(AUAppGluePdfiumPage* pp) {
	NBASSERT(pp->retainCount > 0)
	pp->retainCount++;
	//PRINTF_INFO("PdfPage(%llu) retained: idx('%d') retainCount(%d).\n", (UI64)pp, pp->cache.idx, pp->retainCount);
}

void AUAppGluePdfium::pageRetain(void* param, void* pageRef){
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)pageRef;
	if(obj != NULL && pp != NULL){
		NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
		//AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)pp->glue;
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			NBThreadMutex_lock(&pp->mutex);
			{
				AUAppGluePdfium_pageRetainLocked_(pp);
			}
			NBThreadMutex_unlock(&pp->mutex);
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
}

void AUAppGluePdfium_pageReleaseLockedAndUnlock_(AUAppGluePdfiumPage* pp) {
	NBASSERT(pp->retainCount > 0)
	pp->retainCount--;
	//PRINTF_INFO("PdfPage(%llu) released: idx('%d') retainCount(%d).\n", (UI64)pp, pp->cache.idx, pp->retainCount);
	if (pp->retainCount > 0) {
		NBThreadMutex_unlock(&pp->mutex);
	} else {
		AUAppGluePdfiumDoc* dd = pp->doc; pp->doc = NULL;
		//Release page
		{
			pp->type = ENAppGluePdfiumDataType_Unknown;
			NBThreadMutex_unlock(&pp->mutex);
		}
		//Release doc
		{
			NBASSERT(dd != NULL)
			if (dd != NULL) {
				NBASSERT(dd->type == ENAppGluePdfiumDataType_Doc)
				NBThreadMutex_lock(&dd->mutex);
				NBASSERT(dd->retainCount > 0)
				//Remove from array
				{
					BOOL found = FALSE;
					SI32 i; for (i = 0; i < dd->pages.use; i++) {
						AUAppGluePdfiumPage* pp2 = NBArray_itmValueAtIndex(&dd->pages, AUAppGluePdfiumPage*, i);
						NBASSERT(pp2->type == ENAppGluePdfiumDataType_Page || pp2->type == ENAppGluePdfiumDataType_Unknown);
						if (pp2 == pp) {
							//Release page
							{
								//PRINTF_INFO("Freeing page(%llu)-pdfpage(%llu): w(%f)-h(%f).\n", (UI64)pp, (UI64)pp->page, FPDF_GetPageWidth(pp->page), FPDF_GetPageHeight(pp->page));
								//Cache
								{
									NBString_release(&pp->cache.lbl);
								}
								if (pp->page != NULL) {
									FPDF_ClosePage(pp->page);
									pp->page = NULL;
								}
								pp->glue = NULL;
								NBThreadMutex_release(&pp->mutex);
								NBMemory_setZeroSt(*pp, AUAppGluePdfiumPage);
								NBMemory_free(pp);
							}
							//Remove
							NBArray_removeItemAtIndex(&dd->pages, i);
							found = TRUE;
							break;
						}
					} NBASSERT(found)
				}
				dd->retainCount--;
				if (dd->retainCount > 0) {
					NBThreadMutex_unlock(&dd->mutex);
				} else {
					//Unlocked by this call
					AUAppGluePdfium_docReleaseAndFreeLocked_(dd);
				}
			}
		}
	}
}

void AUAppGluePdfium::pageRelease(void* param, void* pageRef){
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)pageRef;
	if(obj != NULL && pp != NULL){
		NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
		AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)pp->glue;
		//Testing if multiple files can be loaded in parallel
		//NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			NBThreadMutex_lock(&pp->mutex);
			AUAppGluePdfium_pageReleaseLockedAndUnlock_(pp); //It will unlock
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
}

STNBRect AUAppGluePdfium::pageGetBox(void* param, void* pageRef){
	STNBRect r;
	NBMemory_setZeroSt(r, STNBRect);
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)pageRef;
	if(obj != NULL && pp != NULL){
		NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
		//AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)pp->glue; //Do not lock
		//NBThreadMutex_lock(&obj->pdfiumMutex); //Do not lock
		{
			//NBThreadMutex_lock(&pp->mutex); //Do not lock
			r = pp->cache.mediaBox;
			//NBThreadMutex_unlock(&pp->mutex); //Do not lock
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //Do not lock
	}
	return r;
}

BOOL AUAppGluePdfium::pageGetLabel(void* param, void* pageRef, STNBString* dst){
	BOOL r = FALSE;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)pageRef;
	if(obj != NULL && pp != NULL){
		NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
		//AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)pp->glue;
		//NBThreadMutex_lock(&obj->pdfiumMutex); //Do not lock
		if(dst != NULL){
			//NBThreadMutex_lock(&pp->mutex); //Do not lock
			NBString_set(dst, pp->cache.lbl.str);
			//NBThreadMutex_unlock(&pp->mutex); //Do not lock
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //Do not lock
	}
	return r;
}

BOOL AUAppGluePdfium::pageGetText(void* param, void* pageRef, STNBString* dst){
	BOOL r = FALSE;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)pageRef;
	if(obj != NULL && pp != NULL){
		NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
		//AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)pp->glue;
		//NBThreadMutex_lock(&obj->pdfiumMutex); //Do not lock
		if(dst != NULL){
			//NBThreadMutex_lock(&pp->mutex); //Do not lock
			FPDF_TEXTPAGE pg = FPDFText_LoadPage(pp->page);
			if(pg != NULL){
				const int charsCount = FPDFText_CountChars(pg);
				if(charsCount <= 0){
					r = TRUE;
				} else {
					unsigned short* bufUtf16 = NBMemory_allocTypes(unsigned short, charsCount + 2); //+1 for the '\0' added by the library, +1 for another safety '\0' added by this code
					const int charsGot = FPDFText_GetText(pg, 0, charsCount, bufUtf16);
					if(charsGot > 0){
						bufUtf16[charsGot] = '\0';
						NBString_concatUtf16(dst, bufUtf16);
						r = TRUE;
					}
					NBMemory_free(bufUtf16);
					bufUtf16 = NULL;
				}
				FPDFText_ClosePage(pg);
			}
			//NBThreadMutex_unlock(&pp->mutex); //Do not lock
		}
		//NBThreadMutex_unlock(&obj->pdfiumMutex); //Do not lock
	}
	return r;
}

BOOL AUAppGluePdfium::pageRenderArea(void* param, void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst){
	BOOL r = FALSE;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)pageRef;
	if(obj != NULL && pp != NULL && dst != NULL && area.width > 0 && area.height > 0 && dstSz.width > 0 && dstSz.height > 0){
		NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
		AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)pp->glue;
		NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			//Prepare buffer
			{
				const BYTE* data = NBBitmap_getData(dst);
				const STNBBitmapProps props = NBBitmap_getProps(dst);
				if(data == NULL || props.size.width != dstSz.width || props.size.height != dstSz.height || props.color != ENNBBitmapColor_RGBA8){
					NBBitmap_createAndSet(dst, dstSz.width, dstSz.height, ENNBBitmapColor_RGBA8, 255);
				}
			}
			//Render
			{
				BYTE* data = NBBitmap_getData(dst);
				const STNBBitmapProps props = NBBitmap_getProps(dst);
				int fformat = FPDFBitmap_Unknown;
				switch(props.color) {
					case ENNBBitmapColor_GRIS8:
					case ENNBBitmapColor_ALPHA8:
						fformat = FPDFBitmap_Gray; // Gray scale bitmap, one byte per pixel.
						break;
					case ENNBBitmapColor_RGB8:
						fformat = FPDFBitmap_BGR; // 3 bytes per pixel, byte order: blue, green, red.
						break;
					case ENNBBitmapColor_RGBA8:
					case ENNBBitmapColor_BGRA8:
					case ENNBBitmapColor_ARGB8:
						fformat = FPDFBitmap_BGRA; // 4 bytes per pixel, byte order: blue, green, red, alpha.
						break;
					default:
						break;
				}
				if(fformat != FPDFBitmap_Unknown && data != NULL && props.size.width == dstSz.width && props.size.height == dstSz.height && props.color == ENNBBitmapColor_RGBA8){
					FPDF_BITMAP fbmp = FPDFBitmap_CreateEx(props.size.width, props.size.height, fformat, data, props.bytesPerLine);
					//FPDF_BITMAP fbmp = FPDFBitmap_Create(props.size.width, props.size.height, 1);
					NBASSERT(fbmp != NULL)
					if(fbmp != NULL){
						//PRINTF_INFO("Rendering page(%llu): idx('%d') w(%d)h(%d)fmt(%d)data(%llu)bytesPerLine(%d).\n", (UI64)pp, pp->cache.idx, props.size.width, props.size.height, fformat, (UI64)data, props.bytesPerLine);
						// Matrix for transformation, in the form [a b c d e f], equivalent to:
						// | a  b  0 |
						// | c  d  0 |
						// | e  f  1 |
						FS_RECTF clip;
						FS_MATRIX transform;
						NBMemory_setZero(clip);
						NBMemory_setZero(transform);
						//Apply clip
						{
							clip.left	= 0;
							clip.top	= 0;
							clip.right	= (float) props.size.width - 1;
							clip.bottom	= (float) props.size.height - 1;
						}
						//Apply transforms
						{
							// scale up by 110%
							/*transform.a = ((float)props.size.width / (float)area.width);
							 transform.b = -area.x;
							 transform.c = 0;
							 transform.d = ((float)props.size.height / (float)area.height);
							 transform.e = 0;
							 transform.f = 0;*/
							//AU, F0 [2.259804] [0.000000] [0.000000]
							//AU, F1 [0.000000] [2.260101] [-766.000061]
							//AU, F2 [0.000000] [0.000000] [1.000000]
							//
							//#define NBMATRIZ_MULTIPLICAR_PUNTO(P_DEST, MATRIZ, P_X, P_Y)
							//P_DEST.x = (NBMATRIZ_ELEM00(MATRIZ) * (P_X)) + (NBMATRIZ_ELEM01(MATRIZ) * (P_Y)) + NBMATRIZ_ELEM02(MATRIZ);  \
							//P_DEST.y = (NBMATRIZ_ELEM10(MATRIZ) * (P_X)) + (NBMATRIZ_ELEM11(MATRIZ) * (P_Y)) + NBMATRIZ_ELEM12(MATRIZ);  \
							//
							//x′ = a × x + c × y + e
							//y′ = b × x + d × y + f
							//
							NBMatriz mx;
							NBMATRIZ_ESTABLECER_IDENTIDAD(mx);
							NBMATRIZ_ESCALAR(mx, (float)props.size.width / (float)area.width, (float)props.size.height / (float)area.height);
							NBMATRIZ_TRASLADAR(mx, -area.x, -area.y);
							//NBMATRIZ_IMPRIMIR_PRINTF(mx);
							transform.a = NBMATRIZ_ELEM00(mx);
							transform.b = NBMATRIZ_ELEM01(mx);
							transform.c = NBMATRIZ_ELEM10(mx);
							transform.d = NBMATRIZ_ELEM11(mx);
							transform.e = NBMATRIZ_ELEM02(mx);
							transform.f = NBMATRIZ_ELEM12(mx);
							//CGContextScaleCTM(dstCtxt, (float)props.size.width / (float)area.width, (float)props.size.height / (float)area.height);
							//CGContextTranslateCTM(dstCtxt, -area.x, area.y + area.height - pp->cache.mediaBox.height); //pp->cache.mediaBox.height -
							//PRINTF_INFO("Rendering area.y(%f) mediaBox.height(%f).\n", area.y, pp->cache.mediaBox.height);
						}
						//Retain
						{
							NBThreadMutex_lock(&pp->mutex);
							AUAppGluePdfium_pageRetainLocked_(pp);
							NBThreadMutex_unlock(&pp->mutex);
						}
						//Render (unlocked)
						{
							//FPDF_RenderPageBitmap(fbmp, pp->page, area.x, area.y, area.width, area.height, 0, 0);
							FPDF_RenderPageBitmapWithMatrix(fbmp, pp->page, &transform, &clip, 0);
							//Transfor color RGB to BGR if necesary
							switch(props.color) {
								case ENNBBitmapColor_GRIS8:
								case ENNBBitmapColor_ALPHA8:
									//Nothing to do
									break;
								case ENNBBitmapColor_RGB8:
									//FPDFBitmap_BGR, flip R-B bytes
									{
										SI32 iRow; BYTE tmp;
										for(iRow = 0; iRow < props.size.height; iRow++){
											BYTE* ln = &data[props.bytesPerLine * iRow];
											const BYTE* lnEndAfter = &data[props.bytesPerLine * (iRow + 1)];
											while(ln < lnEndAfter){
												tmp		= ln[0];
												ln[0]	= ln[2];
												ln[2]	= tmp;
												ln		+= 3;
											}
										}
									}
									break;
								case ENNBBitmapColor_RGBA8:
									//FPDFBitmap_BGRA, flip R-B bytes
									{
										SI32 iRow; BYTE tmp;
										for(iRow = 0; iRow < props.size.height; iRow++){
											BYTE* ln = &data[props.bytesPerLine * iRow];
											const BYTE* lnEndAfter = &data[props.bytesPerLine * (iRow + 1)];
											while(ln < lnEndAfter){
												tmp		= ln[0];
												ln[0]	= ln[2];
												ln[2]	= tmp;
												ln		+= 4;
											}
										}
									}
									break;
								case ENNBBitmapColor_BGRA8:
									//FPDFBitmap_BGRA, nothing to do
									break;
								case ENNBBitmapColor_ARGB8:
									//FPDFBitmap_BGRA, flip Alpha
									{
										SI32 iRow; BYTE tmp;
										for(iRow = 0; iRow < props.size.height; iRow++){
											BYTE* ln = &data[props.bytesPerLine * iRow];
											const BYTE* lnEndAfter = &data[props.bytesPerLine * (iRow + 1)];
											while(ln < lnEndAfter){
												tmp		= ln[0]; //A
												ln[0]	= ln[1]; //R
												ln[1]	= ln[2]; //G
												ln[2]	= ln[3]; //B
												ln[3]	= tmp;   //A
												ln		+= 4;
											}
										}
									}
									break;
								default:
									break;
							}
						}
						//Release
						{
							NBThreadMutex_lock(&pp->mutex);
							AUAppGluePdfium_pageReleaseLockedAndUnlock_(pp); //It will unlock
						}
						FPDFBitmap_Destroy(fbmp);
						fbmp = NULL;
						r = TRUE;
					}
				}
			}
		}
		NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

//Render job

void* AUAppGluePdfium::renderStartToMemory(void* param, const STNBPdfRenderDoc* job){
	void* r = NULL;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL && job != NULL){
		NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
			if(doc != NULL){
				AUAppGluePdfiumRender* dd = NBMemory_allocType(AUAppGluePdfiumRender);
				NBMemory_setZeroSt(*dd, AUAppGluePdfiumRender);
				dd->type		= ENAppGluePdfiumDataType_Render;
				dd->glue		= obj;
				//
				dd->doc			= doc;
				dd->iNextPage	= 0;
				//
				NBThreadMutex_init(&dd->mutex);
				dd->retainCount	= 1;
				r = dd;
			}
		}
		NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

ENMngrPdfKitRenderResult AUAppGluePdfium::renderContinue(void* param, void* jobRef, const STNBPdfRenderDoc* job){
	ENMngrPdfKitRenderResult r = ENMngrPdfKitRenderResult_Error;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL && jobRef != NULL && job != NULL){
		NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			AUAppGluePdfiumRender* rndr = (AUAppGluePdfiumRender*)jobRef;
			NBASSERT(rndr->type == ENAppGluePdfiumDataType_Render)
			if(rndr->type == ENAppGluePdfiumDataType_Render){
				if(rndr->iNextPage >= job->pages.use){
					r = ENMngrPdfKitRenderResult_Ended;
				} else {
					const STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&job->pages, STNBPdfRenderPage*, rndr->iNextPage);
					if(!AUAppGluePdfium_renderPage(rndr->doc, &rndr->fontContent, job, page, rndr->iNextPage)){
						r = ENMngrPdfKitRenderResult_Error;
					} else {
						//Next page
						rndr->iNextPage++;
						if(rndr->iNextPage >= job->pages.use){
							r = ENMngrPdfKitRenderResult_Ended;
						} else {
							r = ENMngrPdfKitRenderResult_Partial;
						}
					}
				}
			}
		}
		NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}

// A function pointer for getting a block of data from a specific position.
// Position is specified by byte offset from the beginning of the file.
// The pointer to the buffer is never NULL and the size is never 0.
// The position and size will never go out of range of the file length.
// It may be possible for FPDFSDK to call this function multiple times for
// the same position.
// Return value: should be non-zero if successful, zero for error.

typedef struct STNBPdfiumDataPtr_ {
	char*	str;
	UI32	length;
} STNBPdfiumDataPtr;

int AUAppGluePdfium_NBPdfiumDataPtr(void* param, unsigned long position, unsigned char* pBuf, unsigned long size){
	int r = 0;
	if(param != NULL && pBuf != NULL && size > 0){
		STNBPdfiumDataPtr* src = (STNBPdfiumDataPtr*)param;
		if(position < src->length){
			unsigned long avail		= (src->length - position);
			unsigned long copySz	= (avail < size ? avail : size);
			unsigned char* copySrc	= (unsigned char*)&src->str[position];
			NBMemory_copy(pBuf, copySrc, copySz);
			r = (int)copySz;
		}
	}
	PRINTF_INFO("%d bytes copied, pos(%lu, +%lu).\n", r, position, size);
	return r;
}

BOOL AUAppGluePdfium_renderPageImg(FPDF_DOCUMENT doc, FPDF_PAGE fpage, const STNBSize fpageSz, const ENNBPdfRenderPageItmImgType type, const STNBRect rect, const void* data, const UI32 dataSz, const STNBPoint traslation, const float rotDeg){
	BOOL r = TRUE;
	{
		switch (type) {
			case ENNBPdfRenderPageItmImgType_Png:
				//PNG bitmap
				{
                    STNBFileRef stream = NBFile_alloc(NULL);
					if(!NBFile_openAsDataRng(&stream, (void*)data, dataSz)){
						r = FALSE;
					} else {
						NBFile_lock(stream);
						{
							STNBBitmap bmp;
							NBBitmap_init(&bmp);
							if(!NBPng_loadFromFile(&stream, TRUE, &bmp, NULL)){
								r = FALSE;
							} else {
								//Render
								BYTE* dataFliped = NULL;
								BYTE* data = NBBitmap_getData(&bmp);
								const STNBBitmapProps props = NBBitmap_getProps(&bmp);
								int fformat = FPDFBitmap_Unknown;
								switch(props.color) {
									case ENNBBitmapColor_GRIS8:
									case ENNBBitmapColor_ALPHA8:
										fformat = FPDFBitmap_Gray; // Gray scale bitmap, one byte per pixel.
										break;
									case ENNBBitmapColor_RGB8:
										fformat = FPDFBitmap_BGR; // 3 bytes per pixel, byte order: blue, green, red.
										//FPDFBitmap_BGR, flip R-B bytes
										{
											NBASSERT(dataFliped == NULL)
											dataFliped = (BYTE*)NBMemory_alloc(props.size.height * props.bytesPerLine);
											NBMemory_copy(dataFliped, data, props.size.height * props.bytesPerLine);
											{
												SI32 iRow; BYTE tmp;
												for(iRow = 0; iRow < props.size.height; iRow++){
													BYTE* ln = &dataFliped[props.bytesPerLine * iRow];
													const BYTE* lnEndAfter = &dataFliped[props.bytesPerLine * (iRow + 1)];
													while(ln < lnEndAfter){
														tmp		= ln[0];
														ln[0]	= ln[2];
														ln[2]	= tmp;
														ln		+= 3;
													}
												}
											}
											data = dataFliped;
										}
										break;
									case ENNBBitmapColor_RGBA8:
										fformat = FPDFBitmap_BGRA; // 4 bytes per pixel, byte order: blue, green, red, alpha.
										//FPDFBitmap_BGRA, flip R-B bytes
										{
											NBASSERT(dataFliped == NULL)
											dataFliped = (BYTE*)NBMemory_alloc(props.size.height * props.bytesPerLine);
											NBMemory_copy(dataFliped, data, props.size.height * props.bytesPerLine);
											{
												SI32 iRow; BYTE tmp;
												for(iRow = 0; iRow < props.size.height; iRow++){
													BYTE* ln = &dataFliped[props.bytesPerLine * iRow];
													const BYTE* lnEndAfter = &dataFliped[props.bytesPerLine * (iRow + 1)];
													while(ln < lnEndAfter){
														tmp		= ln[0];
														ln[0]	= ln[2];
														ln[2]	= tmp;
														ln		+= 4;
													}
												}
											}
											data = dataFliped;
										}
										break;
									case ENNBBitmapColor_BGRA8:
										fformat = FPDFBitmap_BGRA; // 4 bytes per pixel, byte order: blue, green, red, alpha.
										break;
									case ENNBBitmapColor_ARGB8:
										fformat = FPDFBitmap_BGRA; // 4 bytes per pixel, byte order: blue, green, red, alpha.
										{
											NBASSERT(dataFliped == NULL)
											dataFliped = (BYTE*)NBMemory_alloc(props.size.height * props.bytesPerLine);
											NBMemory_copy(dataFliped, data, props.size.height * props.bytesPerLine);
											{
												SI32 iRow; BYTE tmp;
												for(iRow = 0; iRow < props.size.height; iRow++){
													BYTE* ln = &dataFliped[props.bytesPerLine * iRow];
													const BYTE* lnEndAfter = &dataFliped[props.bytesPerLine * (iRow + 1)];
													while(ln < lnEndAfter){
														tmp		= ln[0]; //A
														ln[0]	= ln[1]; //R
														ln[1]	= ln[2]; //G
														ln[2]	= ln[3]; //B
														ln[3]	= tmp;   //A
														ln		+= 4;
													}
												}
											}
											data = dataFliped;
										}
										break;
									default:
										break;
								}
								if(fformat == FPDFBitmap_Unknown || data == NULL){
									r = FALSE;
								} else {
									FPDF_BITMAP fbmp = FPDFBitmap_CreateEx(props.size.width, props.size.height, fformat, data, props.bytesPerLine);
									//FPDF_BITMAP fbmp = FPDFBitmap_Create(props.size.width, props.size.height, 1);
									NBASSERT(fbmp != NULL)
									if(fbmp != NULL){
										FPDF_PAGEOBJECT tempImg = FPDFPageObj_NewImageObj(doc);
										if(!FPDFImageObj_SetBitmap(&fpage, 1, tempImg, fbmp)){
											r = FALSE;
										} else {
											NBMatriz mx;
											NBMATRIZ_ESTABLECER_IDENTIDAD(mx);
											NBMATRIZ_TRASLADAR(mx, traslation.x + rect.x, fpageSz.height - traslation.y - rect.y);
											NBMATRIZ_ROTAR_GRADOS(mx, rotDeg);
											NBMATRIZ_ESCALAR(mx, rect.width, -rect.height);
											//NBMATRIZ_IMPRIMIR_PRINTF(mx);
											FPDFImageObj_SetMatrix(tempImg, NBMATRIZ_ELEM00(mx), NBMATRIZ_ELEM01(mx), NBMATRIZ_ELEM10(mx), NBMATRIZ_ELEM11(mx), NBMATRIZ_ELEM02(mx), NBMATRIZ_ELEM12(mx));
											FPDFPage_InsertObject(fpage, tempImg);
										}
										FPDFBitmap_Destroy(fbmp);
										fbmp = NULL;
									}
								}
								//Release flipped data
								if(dataFliped != NULL){
									NBMemory_free(dataFliped);
									dataFliped = NULL;
								}
							}
							NBBitmap_release(&bmp);
						}
						NBFile_unlock(stream);
					}
					NBFile_release(&stream);
				}
				break;
			case ENNBPdfRenderPageItmImgType_Jpeg:
				//JPG bitmap
				{
					FPDF_PAGEOBJECT tempImg = FPDFPageObj_NewImageObj(doc);
					STNBPdfiumDataPtr dataPtr;
					FPDF_FILEACCESS fileAccess;
					NBMemory_setZero(fileAccess);
					dataPtr.str				= (char*)data;
					dataPtr.length			= dataSz;
					fileAccess.m_FileLen	= dataSz;
					fileAccess.m_Param		= &dataPtr;
					fileAccess.m_GetBlock	= AUAppGluePdfium_NBPdfiumDataPtr;
					if(!FPDFImageObj_LoadJpegFileInline(&fpage, 1, tempImg, &fileAccess)){
						FPDFPageObj_Destroy(tempImg);
						tempImg = NULL;
						r = FALSE;
					} else {
						NBMatriz mx;
						NBMATRIZ_ESTABLECER_IDENTIDAD(mx);
						NBMATRIZ_TRASLADAR(mx, traslation.x + rect.x, fpageSz.height - traslation.y - rect.y);
						NBMATRIZ_ROTAR_GRADOS(mx, rotDeg);
						NBMATRIZ_ESCALAR(mx, rect.width, -rect.height);
						//NBMATRIZ_IMPRIMIR_PRINTF(mx);
						FPDFImageObj_SetMatrix(tempImg, NBMATRIZ_ELEM00(mx), NBMATRIZ_ELEM01(mx), NBMATRIZ_ELEM10(mx), NBMATRIZ_ELEM11(mx), NBMATRIZ_ELEM02(mx), NBMATRIZ_ELEM12(mx));
						FPDFPage_InsertObject(fpage, tempImg);
					}
				}
				break;
			default:
				NBASSERT(FALSE)
				break;
		}
	}
	return r;
}
	
BOOL AUAppGluePdfium_renderPage(FPDF_DOCUMENT doc, FPDF_FONT* fontContentRef, const STNBPdfRenderDoc* job, const STNBPdfRenderPage* page, const SI32 iPage){
	BOOL r = FALSE;
	//Render
	{
		STNBSize fpageSz = page->size;
		STNBAABox mediaBox, cropBox, bleedBox, trimBox, artBox;
		//
		mediaBox.xMin	= 0;
		mediaBox.yMin	= 0;
		mediaBox.xMax	= page->size.width;
		mediaBox.yMax	= page->size.height;
		cropBox			= bleedBox = trimBox = artBox = mediaBox;
		//
		//Load sizes from template page
		if(page->sizeTemplate.pageRef != NULL){
			AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
			if(pp != NULL){
				NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
				NBThreadMutex_lock(&pp->mutex);
				if(pp->page != NULL){
					fpageSz.width	= FPDF_GetPageWidth(pp->page);
					fpageSz.height	= FPDF_GetPageHeight(pp->page);
					FPDFPage_GetArtBox(pp->page, &artBox.xMin, &artBox.yMin, &artBox.xMax, &artBox.yMax);
					FPDFPage_GetTrimBox(pp->page, &trimBox.xMin, &trimBox.yMin, &trimBox.xMax, &trimBox.yMax);
					FPDFPage_GetBleedBox(pp->page, &bleedBox.xMin, &bleedBox.yMin, &bleedBox.xMax, &bleedBox.yMax);
					FPDFPage_GetCropBox(pp->page, &cropBox.xMin, &cropBox.yMin, &cropBox.xMax, &cropBox.yMax);
					FPDFPage_GetMediaBox(pp->page, &mediaBox.xMin, &mediaBox.yMin, &mediaBox.xMax, &mediaBox.yMax);
				}
				NBThreadMutex_unlock(&pp->mutex);
			} else {
				void* bmpData = NULL;
				STNBBitmapProps bmpProps;
				NBMemory_setZeroSt(bmpProps, STNBBitmapProps);
				if(NBMngrPdfKit::pageGetDataBitmap(page->sizeTemplate.pageRef, &bmpProps, &bmpData)){
					fpageSz.width	= (bmpProps.size.width <= bmpProps.size.height ? 612 : 792);
					fpageSz.height	= (bmpProps.size.width <= bmpProps.size.height ? 792 : 612);
					//
					mediaBox.xMin	= 0;
					mediaBox.yMin	= 0;
					mediaBox.xMax	= fpageSz.width;
					mediaBox.yMax	= fpageSz.height;
					cropBox			= bleedBox = trimBox = artBox = mediaBox;
				} else {
					NBASSERT(FALSE) //ERROR, undetermined type of content
				}
			}
		}
		//Build page
		//NBASSERT(size.width > 0 && size.height > 0)
		if(fpageSz.width > 0 && fpageSz.height > 0){
			BOOL pageCreated = FALSE;
			FPDF_PAGE fpage = NULL;
			r = TRUE;
			//Itms
			{
				SI32 i; for(i = 0; i < page->itms.use && r; i++){
					const STNBPdfRenderPageItm* itm = NBArray_itmPtrAtIndex(&page->itms, STNBPdfRenderPageItm, i);
					//Draw
					{
						switch(itm->type){
							case ENNBPdfRenderPageItmType_Text:
								{
									//Create page if necesary
									if(fpage == NULL && !pageCreated){
										fpage = FPDFPage_New(doc, iPage, fpageSz.width, fpageSz.height);
										FPDFPage_SetArtBox(fpage, artBox.xMin, artBox.yMin, artBox.xMax, artBox.yMax);
										FPDFPage_SetTrimBox(fpage, trimBox.xMin, trimBox.yMin, trimBox.xMax, trimBox.yMax);
										FPDFPage_SetBleedBox(fpage, bleedBox.xMin, bleedBox.yMin, bleedBox.xMax, bleedBox.yMax);
										FPDFPage_SetCropBox(fpage, cropBox.xMin, cropBox.yMin, cropBox.xMax, cropBox.yMax);
										FPDFPage_SetMediaBox(fpage, mediaBox.xMin, mediaBox.yMin, mediaBox.xMax, mediaBox.yMax);
										pageCreated = TRUE;
									}
									//Load font if necesary
									if(*fontContentRef == NULL){
										*fontContentRef = FPDFText_LoadStandardFont(doc, "Helvetica"); //Helvetica-BoldItalic
									}
									//Add text
									if(*fontContentRef == NULL){
										//Font could not be loaded
										r = FALSE;
									} else {
										const char* str = itm->text.text.str;
										STNBString strUtf16;
										NBString_init(&strUtf16);
										//Build UTF16 string
										{
											float yPosExtra = 0.0f;
											UI8 surrExpd = 0; UI32 unicode = 0; UI16 utf16 = 0;
											while(TRUE){
												if(*str == '\0'){
													surrExpd = 0;
												} else {
													surrExpd = NBEncoding_utf8BytesExpected(*str);
													if(surrExpd <= 0){
														r = FALSE; break;
													}
													unicode = NBEncoding_unicodeFromUtf8s(str, surrExpd, 0);
													utf16	= unicode;
													if(*str != '\r' && *str != '\n'){
														NBString_concatBytes(&strUtf16, (const char*)&utf16, sizeof(utf16));
													}
												}
												//Flush line
												if(*str == '\0' || *str == '\n'){
													if(strUtf16.length > 0){
														//Add zero-char
														{
															utf16 = '\0';
															NBString_concatBytes(&strUtf16, (const char*)&utf16, sizeof(utf16));
														}
														//Add text-line obj
														{
															FPDF_PAGEOBJECT txtObj = FPDFPageObj_CreateTextObj(doc, *fontContentRef, itm->text.fontSize);
															if(!FPDFPageObj_SetFillColor(txtObj, itm->text.fontColor.r, itm->text.fontColor.g, itm->text.fontColor.b, itm->text.fontColor.a)){
																r = FALSE;
															} else {
																NBMatriz mx;
																NBMATRIZ_ESTABLECER_IDENTIDAD(mx);
																NBMATRIZ_TRASLADAR(mx, itm->matrix.traslation.x, fpageSz.height - itm->matrix.traslation.y - yPosExtra);
																NBMATRIZ_ROTAR_GRADOS(mx, itm->matrix.rotDeg);
																//NBMATRIZ_IMPRIMIR_PRINTF(mx);
																FPDFPageObj_Transform(txtObj, NBMATRIZ_ELEM00(mx), NBMATRIZ_ELEM01(mx), NBMATRIZ_ELEM10(mx), NBMATRIZ_ELEM11(mx), NBMATRIZ_ELEM02(mx), NBMATRIZ_ELEM12(mx));
																if(!FPDFText_SetText(txtObj, (FPDF_WIDESTRING)strUtf16.str)){
																	r = FALSE;
																} else {
																	FPDFPage_InsertObject(fpage, txtObj);
																}
															}
														}
													}
													//Next line
													yPosExtra += (itm->text.fontSize * 1.10f); //1.10f is just random selected
													NBString_empty(&strUtf16);
													if(*str == '\0') break;
												}
												//Next char
												str += surrExpd;
											}
										}
										NBString_release(&strUtf16);
									}
								}
								/*{
									UIColor* color		= [UIColor colorWithRed:(itm->text.fontColor.r / 255.0f) green:(itm->text.fontColor.g / 255.0f) blue:(itm->text.fontColor.b / 255.0f) alpha:(itm->text.fontColor.a / 255.0f)];
									UIFont* font		= [UIFont systemFontOfSize:itm->text.fontSize];
									NSString* str		= [NSString stringWithUTF8String:itm->text.text.str];
									NSDictionary* atts	= [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, color, NSForegroundColorAttributeName, nil];
									NSAttributedString* strAtt = [[NSAttributedString alloc] initWithString:str attributes:atts];
									CGPoint pos			= CGPointMake(0.0f, 0.0f);
									[strAtt drawAtPoint:pos];
									[strAtt release];
									strAtt = nil;
								}*/
								break;
							case ENNBPdfRenderPageItmType_Image:
								{
									//Create page if necesary
									if(fpage == NULL && !pageCreated){
										fpage = FPDFPage_New(doc, iPage, fpageSz.width, fpageSz.height);
										FPDFPage_SetArtBox(fpage, artBox.xMin, artBox.yMin, artBox.xMax, artBox.yMax);
										FPDFPage_SetTrimBox(fpage, trimBox.xMin, trimBox.yMin, trimBox.xMax, trimBox.yMax);
										FPDFPage_SetBleedBox(fpage, bleedBox.xMin, bleedBox.yMin, bleedBox.xMax, bleedBox.yMax);
										FPDFPage_SetCropBox(fpage, cropBox.xMin, cropBox.yMin, cropBox.xMax, cropBox.yMax);
										FPDFPage_SetMediaBox(fpage, mediaBox.xMin, mediaBox.yMin, mediaBox.xMax, mediaBox.yMax);
										pageCreated = TRUE;
									}
									//Add image
									if(fpage == NULL){
										r = FALSE;
									} else if(!AUAppGluePdfium_renderPageImg(doc, fpage, fpageSz, itm->image.type, itm->image.rect, itm->image.data.str, itm->image.data.length, itm->matrix.traslation, itm->matrix.rotDeg)){
										r = FALSE;
									}
								}
								break;
							case ENNBPdfRenderPageItmType_PdfPage:
								if(itm->pdfPage.pageRef != NULL){
									AUAppGluePdfiumPage* pp = (AUAppGluePdfiumPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
									if(pp != NULL){
										NBASSERT(pp->type == ENAppGluePdfiumDataType_Page)
										NBThreadMutex_lock(&pp->mutex);
										if(pp->page != NULL && pp->doc != NULL){
											if(pp->doc->doc != NULL){
												STNBString pgRng;
												NBString_init(&pgRng);
												{
													NBString_concatSI32(&pgRng, (pp->cache.idx + 1));
													if(!FPDF_ImportPages(doc, pp->doc->doc, pgRng.str, iPage)){
														r = FALSE;
													} else {
														fpage = FPDF_LoadPage(doc, iPage);
														pageCreated = TRUE;
													}
												}
												NBString_release(&pgRng);
											}
										}
										NBThreadMutex_unlock(&pp->mutex);
									} else {
										void* bmpData = NULL;
										STNBBitmapProps bmpProps;
										NBMemory_setZeroSt(bmpProps, STNBBitmapProps);
										if(NBMngrPdfKit::pageGetDataBitmap(page->sizeTemplate.pageRef, &bmpProps, &bmpData)){
											STNBString imgData;
											NBString_initWithSz(&imgData, 100 * 1024, 100 * 1024, 0.25f);
											{
                                                STNBFileRef stream = NBFile_alloc(NULL);
												if(!NBFile_openAsString(&stream, &imgData)){
													NBASSERT(FALSE) //ERROR
												} else {
													ENNBPdfRenderPageItmImgType imgType = ENNBPdfRenderPageItmImgType_Count;
													if(bmpProps.color == ENNBBitmapColor_RGB8 || bmpProps.color == ENNBBitmapColor_GRIS8){
														NBFile_lock(stream);
														{
															//Save as JPEG
															STNBJpegWrite jWrite;
															NBJpegWrite_init(&jWrite);
															if(!NBJpegWrite_feedStart(&jWrite, bmpData, (bmpProps.bytesPerLine * bmpProps.size.height), bmpProps, stream, 85, 10)){
																r = FALSE; NBASSERT(FALSE)
															} else {
																SI32 ciclesCount = 0;
																ENJpegWriteResult rr = ENJpegWriteResult_error;
																while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																	ciclesCount++;
																}
																if(rr != ENJpegWriteResult_end){
																	r = FALSE; NBASSERT(FALSE) //ERROR
																} else {
																	imgType = ENNBPdfRenderPageItmImgType_Jpeg;
																	NBJpegWrite_feedEnd(&jWrite);
																}
															}
															NBJpegWrite_release(&jWrite);
														}
														NBFile_unlock(stream);
													} else {
														//Save as PNG
														if(!NBPng_saveDataToFile(&bmpProps, bmpData, &stream, ENPngCompressLvl_9)){
															r = FALSE; NBASSERT(FALSE)
														} else {
															imgType = ENNBPdfRenderPageItmImgType_Png;
														}
													}
													if(imgType == ENNBPdfRenderPageItmImgType_Jpeg || imgType == ENNBPdfRenderPageItmImgType_Png){
														//Create page if necesary
														if(fpage == NULL && !pageCreated){
															fpageSz.width	= (bmpProps.size.width <= bmpProps.size.height ? 612 : 792);
															fpageSz.height	= (bmpProps.size.width <= bmpProps.size.height ? 792 : 612);
															//
															mediaBox.xMin	= 0;
															mediaBox.yMin	= 0;
															mediaBox.xMax	= fpageSz.width;
															mediaBox.yMax	= fpageSz.height;
															cropBox			= bleedBox = trimBox = artBox = mediaBox;
															//
															fpage = FPDFPage_New(doc, iPage, fpageSz.width, fpageSz.height);
															FPDFPage_SetArtBox(fpage, artBox.xMin, artBox.yMin, artBox.xMax, artBox.yMax);
															FPDFPage_SetTrimBox(fpage, trimBox.xMin, trimBox.yMin, trimBox.xMax, trimBox.yMax);
															FPDFPage_SetBleedBox(fpage, bleedBox.xMin, bleedBox.yMin, bleedBox.xMax, bleedBox.yMax);
															FPDFPage_SetCropBox(fpage, cropBox.xMin, cropBox.yMin, cropBox.xMax, cropBox.yMax);
															FPDFPage_SetMediaBox(fpage, mediaBox.xMin, mediaBox.yMin, mediaBox.xMax, mediaBox.yMax);
															pageCreated = TRUE;
														}
														//Add image
														if(fpage == NULL){
															r = FALSE;
														} else {
															//Render
															const float scaleW = ((fpageSz.width * 0.75f) / bmpProps.size.width);
															const float scaleH = ((fpageSz.height * 0.75f) / bmpProps.size.height);
															float imgScale = (scaleW < scaleH ? scaleW : scaleH);
															if(imgScale > 1.0f) imgScale = 1.0f;
															{
																const STNBRect rect = NBST_P(STNBRect, 0.0f + ((fpageSz.width - (bmpProps.size.width * imgScale)) * 0.5f), 0.0f + ((fpageSz.height - (bmpProps.size.height * imgScale)) * 0.5f) + (bmpProps.size.height * imgScale), bmpProps.size.width * imgScale, -(bmpProps.size.height * imgScale));
																if(!AUAppGluePdfium_renderPageImg(doc, fpage, fpageSz, imgType, rect, imgData.str, imgData.length, NBST_P(STNBPoint, 0.0f, 0.0f), 0.0f)){
																	r = FALSE;
																}
															}
														}
													}
												}
												NBFile_release(&stream);
											}
											NBString_release(&imgData);
										} else {
											NBASSERT(FALSE) //ERROR, undetermined type of content
										}
									}
								}
								break;
							default:
								NBASSERT(FALSE)
								break;
						}
					}
					NBASSERT(r); //Testing
				}
			}
			//Create empty page (if necesary)
			if(fpage == NULL && !pageCreated){
				fpage = FPDFPage_New(doc, iPage, fpageSz.width, fpageSz.height);
				FPDFPage_SetArtBox(fpage, artBox.xMin, artBox.yMin, artBox.xMax, artBox.yMax);
				FPDFPage_SetTrimBox(fpage, trimBox.xMin, trimBox.yMin, trimBox.xMax, trimBox.yMax);
				FPDFPage_SetBleedBox(fpage, bleedBox.xMin, bleedBox.yMin, bleedBox.xMax, bleedBox.yMax);
				FPDFPage_SetCropBox(fpage, cropBox.xMin, cropBox.yMin, cropBox.xMax, cropBox.yMax);
				FPDFPage_SetMediaBox(fpage, mediaBox.xMin, mediaBox.yMin, mediaBox.xMax, mediaBox.yMax);
				pageCreated = TRUE;
			}
			//Create content
			if(fpage != NULL){
				FPDFPage_GenerateContent(fpage);
				FPDF_ClosePage(fpage);
				fpage = NULL;
			}
		}
		NBASSERT(r); //Testing
	}
	NBASSERT(r); //Testing
	return r;
}

void AUAppGluePdfium::renderCancel(void* param, void* jobRef){
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL && jobRef != NULL){
		NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			AUAppGluePdfiumRender* rndr = (AUAppGluePdfiumRender*)jobRef;
			NBASSERT(rndr->type == ENAppGluePdfiumDataType_Render)
			if(rndr->type == ENAppGluePdfiumDataType_Render){
				NBASSERT(rndr->retainCount == 1)
				if(rndr->glue != NULL){
					rndr->glue = NULL;
				}
				if(rndr->fontContent != NULL){
					FPDFFont_Close(rndr->fontContent);
					rndr->fontContent = NULL;
				}
				if(rndr->doc != NULL){
					FPDF_CloseDocument(rndr->doc);
					rndr->doc = NULL;
				}
				NBThreadMutex_release(&rndr->mutex);
				NBMemory_free(rndr);
				rndr = NULL;
			}
		}
		NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
}

void* AUAppGluePdfium::renderEndAndOpenDoc(void* param, void* jobRef){
	void* r = NULL;
	AUAppGluePdfiumData* obj = (AUAppGluePdfiumData*)param;
	if(obj != NULL && jobRef != NULL){
		NBThreadMutex_lock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
		{
			AUAppGluePdfiumRender* rndr = (AUAppGluePdfiumRender*)jobRef;
			NBASSERT(rndr->type == ENAppGluePdfiumDataType_Render)
			if(rndr->type == ENAppGluePdfiumDataType_Render){
				NBASSERT(rndr->retainCount == 1)
				if(rndr->doc != NULL){
					{
						AUAppGluePdfiumDoc* dd = NBMemory_allocType(AUAppGluePdfiumDoc);
						NBMemory_setZeroSt(*dd, AUAppGluePdfiumDoc);
						dd->type		= ENAppGluePdfiumDataType_Doc;
						dd->glue		= obj;
						dd->doc			= rndr->doc;
						dd->retainCount	= 1;
						NBString_init(&dd->filename); //if was loaded from file
						NBArray_init(&dd->pages, sizeof(AUAppGluePdfiumPage*), NULL);
						NBThreadMutex_init(&dd->mutex);
						//PRINTF_INFO("Opened doc(%llu)-pdfDoc(%llu) ('%s').\n", (UI64)dd, (UI64)dd->doc, dd->filename.str);
						r = dd;
					}
					rndr->doc = NULL;
				}
				//Release
				{
					if(rndr->glue != NULL){
						rndr->glue = NULL;
					}
					if(rndr->fontContent != NULL){
						FPDFFont_Close(rndr->fontContent);
						rndr->fontContent = NULL;
					}
					if(rndr->doc != NULL){
						FPDF_CloseDocument(rndr->doc);
						rndr->doc = NULL;
					}
					NBThreadMutex_release(&rndr->mutex);
					NBMemory_free(rndr);
					rndr = NULL;
				}
			}
		}
		NBThreadMutex_unlock(&obj->pdfiumMutex); //PDFium is not thread-safe, only one PDFium-action per process
	}
	return r;
}
