//
//  AUAppGlueApplePdfKit.h
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#ifndef AUAppGlueApplePdfKit_h
#define AUAppGlueApplePdfKit_h

#include "AUMngrPdfKit.h"
#include "nb/2d/NBBitmap.h"

class AUAppGlueApplePdfKit {
	public:
		//Calls
		static bool		create(AUAppI* app, STMngrPdfKitCalls* obj);
		static bool		destroy(void* param);
		//Specs
		static BOOL		isSupported();	//Determines if the current OS version supports this rendering engine
		static BOOL		canMultithread(void* param);
		//Document
		static void*	docOpenFromPath(void* param, const char* path);
		static void*	docOpenFromData(void* param, const void* data, const UI32 dataSz);
		static void*	docOpenFromRenderJob(void* param, const STNBPdfRenderDoc* job);
		static void		docRetain(void* param, void* docRef);
		static void		docRelease(void* param, void* docRef);
		static UI32		docGetPagesCount(void* param, void* docRef);
		static void*	docGetPageAtIdx(void* param, void* docRef, const UI32 idx);
		static BOOL		docWriteToFilepath(void* param, void* docRef, const char* filepath);
		//Page
		static void		pageRetain(void* param, void* pageRef);
		static void		pageRelease(void* param, void* pageRef);
		static STNBRect	pageGetBox(void* param, void* pageRef);
		static BOOL		pageGetLabel(void* param, void* pageRef, STNBString* dst);
		static BOOL		pageGetText(void* param, void* pageRef, STNBString* dst);
		static BOOL		pageRenderArea(void* param, void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst);
		//Render job
		static void*					renderStartToMemory(void* param, const STNBPdfRenderDoc* job);
		static ENMngrPdfKitRenderResult	renderContinue(void* param, void* jobRef, const STNBPdfRenderDoc* job);
		static void						renderCancel(void* param, void* jobRef);
		static void*					renderEndAndOpenDoc(void* param, void* jobRef);
};

#endif /* AUAppGlueApplePdfKit_h */
