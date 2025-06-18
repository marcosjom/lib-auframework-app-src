//
//  NBMngrPdfKit.h
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#ifndef NBMngrPdfKit_h
#define NBMngrPdfKit_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrPdfKit.h"

class NBMngrPdfKit {
	public:
		static void		init();
		static void		finish();
		static bool		isInited();
		//
		static bool		isGlued();
		static bool		setGlue(AUAppI* app, PTRfuncPdfKitCreate initCall);
		//Specs
		static BOOL		canMultithread();
		//Document
		static void*	docOpenFromPath(const char* path);
		static void*	docOpenFromData(const void* data, const UI32 dataSz);
		static void*	docOpenFromRenderJob(const STNBPdfRenderDoc* job);
		static void*	docGetDataGlued(void* docRef);
		static void		docRetain(void* docRef);
		static void		docRelease(void* docRef);
		static UI32		docGetPagesCount(void* docRef);
		static void*	docGetPageAtIdx(void* docRef, const UI32 idx);
		static BOOL		docWriteToFilepath(void* docRef, const char* filePath);
		//Page
		static void*	pageGetDataGluedInner(void* pageRef);
		static BOOL		pageGetDataBitmap(void* pageRef, STNBBitmapProps* dstProps, void** dstData);
		static void		pageRetain(void* pageRef);
		static void		pageRelease(void* pageRef);
		static STNBRect	pageGetBox(void* pageRef);
		static BOOL		pageGetLabel(void* pageRef, STNBString* dst);
		static BOOL		pageGetText(void* pageRef, STNBString* dst);
		static BOOL		pageRenderArea(void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst);
		//Render job
		static void*					renderStartToMemory(const STNBPdfRenderDoc* job);
		static ENMngrPdfKitRenderResult	renderContinue(void* jobRef, const STNBPdfRenderDoc* job);
		static void						renderCancel(void* jobRef);
		static void*					renderEndAndOpenDoc(void* jobRef);
	private:
		static AUMngrPdfKit* _instance;
};

#endif /* NBMngrPdfKit_h */
