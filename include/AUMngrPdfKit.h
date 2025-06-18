//
//  AUMngrPdfKit.h
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#ifndef AUMngrPdfKit_h
#define AUMngrPdfKit_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"
#include "nb/pdf/NBPdfRenderDoc.h"

typedef struct STMngrPdfKitCalls_ STMngrPdfKitCalls;

typedef enum ENMngrPdfKitRenderResult_ {
	ENMngrPdfKitRenderResult_Error = 0,	//Error
	ENMngrPdfKitRenderResult_Partial,	//Can continue
	ENMngrPdfKitRenderResult_Ended,		//Job ended
	ENMngrPdfKitRenderResult_Count
} ENMngrPdfKitRenderResult;

//Callbacks
typedef bool (*PTRfuncPdfKitCreate)(AUAppI* app, STMngrPdfKitCalls* obj);
typedef bool (*PTRfuncPdfKitDestroy)(void* obj);
//Specs
typedef BOOL (*PTRFuncPdfKitCanMultithread)(void* obj);
//
typedef void* (*PTRFuncPdfKitDocOpenPath)(void* obj, const char* path);
typedef void* (*PTRFuncPdfKitDocOpenData)(void* obj, const void* data, const UI32 dataSz);
typedef void* (*PTRFuncPdfKitDocOpenRenderJob)(void* obj, const STNBPdfRenderDoc* job);
typedef void (*PTRFuncPdfKitDocRetain)(void* obj, void* docRef);
typedef void (*PTRFuncPdfKitDocRelease)(void* obj, void* docRef);
typedef UI32 (*PTRFuncPdfKitDocGetPagesCount)(void* obj, void* docRef);
typedef void* (*PTRFuncPdfKitDocGetPageAtIdx)(void* obj, void* docRef, const UI32 idx);
typedef BOOL (*PTRFuncPdfKitDocAddPageAtIdx)(void* obj, void* docRef, void* pageRef, const UI32 idx);
typedef BOOL (*PTRFuncPdfKitDocWriteToFilepath)(void* obj, void* docRef, const char* filepath);
//Page
typedef void (*PTRFuncPdfKitPageRetain)(void* obj, void* pageRef);
typedef void (*PTRFuncPdfKitPageRelease)(void* obj, void* pageRef);
typedef STNBRect (*PTRFuncPdfKitPageGetBox)(void* obj, void* pageRef);
typedef BOOL (*PTRFuncPdfKitPageGetLabel)(void* obj, void* pageRef, STNBString* dst);
typedef BOOL (*PTRFuncPdfKitPageGetText)(void* obj, void* pageRef, STNBString* dst);
typedef BOOL (*PTRFuncPdfKitPageRenderArea)(void* obj, void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst);
//Render
typedef void* (*PTRFuncPdfKitRenderStartToMemory)(void* obj, const STNBPdfRenderDoc* job);
typedef ENMngrPdfKitRenderResult (*PTRFuncPdfKitRenderContinue)(void* obj, void* jobRef, const STNBPdfRenderDoc* job);
typedef void  (*PTRFuncPdfKitRenderCancel)(void* obj, void* jobRef);
typedef void* (*PTRFuncPdfKitRenderEndAndOpenDoc)(void* obj, void* jobRef);


typedef struct STMngrPdfKitCalls_ {
	PTRfuncPdfKitCreate				funcCreate;
	void*							funcCreateParam;
	PTRfuncPdfKitDestroy			funcDestroy;
	void*							funcDestroyParam;
	//Specs
	PTRFuncPdfKitCanMultithread		funcCanMultithread;
	void*							funcCanMultithreadParam;
	//Document
	PTRFuncPdfKitDocOpenPath		funcDocOpenPath;
	void*							funcDocOpenPathParam;
	PTRFuncPdfKitDocOpenData		funcDocOpenData;
	void*							funcDocOpenDataParam;
	PTRFuncPdfKitDocOpenRenderJob	funcDocOpenRenderJob;
	void*							funcDocOpenRenderJobParam;
	PTRFuncPdfKitDocRetain			funcDocRetain;
	void*							funcDocRetainParam;
	PTRFuncPdfKitDocRelease			funcDocRelease;
	void*							funcDocReleaseParam;
	PTRFuncPdfKitDocGetPagesCount	funcDocGetPagesCount;
	void*							funcDocGetPagesCountParam;
	PTRFuncPdfKitDocGetPageAtIdx	funcDocGetPageAtIdx;
	void*							funcDocGetPageAtIdxParam;
	PTRFuncPdfKitDocAddPageAtIdx	funcDocAddPageAtIdx;
	void*							funcDocAddPageAtIdxParam;
	PTRFuncPdfKitDocWriteToFilepath funcDocWriteToFilepath;
	void*							funcDocWriteToFilepathParam;
	//Page
	PTRFuncPdfKitPageRetain			funcPageRetain;
	void*							funcPageRetainParam;
	PTRFuncPdfKitPageRelease		funcPageRelease;
	void*							funcPageReleaseParam;
	PTRFuncPdfKitPageGetBox			funcPageGetBox;
	void*							funcPageGetBoxParam;
	PTRFuncPdfKitPageGetLabel		funcPageGetLabel;
	void*							funcPageGetLabelParam;
	PTRFuncPdfKitPageGetLabel		funcPageGetText;
	void*							funcPageGetTextParam;
	PTRFuncPdfKitPageRenderArea		funcPageRenderArea;
	void*							funcPageRenderAreaParam;
	//Render job
	PTRFuncPdfKitRenderStartToMemory funcRenderStartToMemory;
	void*							funcRenderStartToMemoryParam;
	PTRFuncPdfKitRenderContinue 	funcRenderContinue;
	void*							funcRenderContinueParam;
	PTRFuncPdfKitRenderCancel		funcRenderCancel;
	void*							funcRenderCancelParam;
	PTRFuncPdfKitRenderEndAndOpenDoc funcRenderEndAndOpenDoc;
	void*							funcRenderEndAndOpenDocParam;
} STMngrPdfKitCalls;

//

class AUMngrPdfKit : public AUObjeto {
	public:
		AUMngrPdfKit();
		virtual ~AUMngrPdfKit();
		//
		static bool	isGlued();
		static bool	setGlue(AUAppI* app, PTRfuncPdfKitCreate initCall);
		//Specs
		BOOL		canMultithread();
		//Document
		void*		docOpenFromPath(const char* path);
		void*		docOpenFromData(const void* data, const UI32 dataSz);
		void*		docOpenFromRenderJob(const STNBPdfRenderDoc* job);
		void*		docGetDataGlued(void* docRef);
		void		docRetain(void* docRef);
		void		docRelease(void* docRef);
		UI32		docGetPagesCount(void* docRef);
		void*		docGetPageAtIdx(void* docRef, const UI32 idx);
		BOOL		docWriteToFilepath(void* docRef, const char* filePath);
		//Page
		void*		pageGetDataGluedInner(void* pageRef);
		BOOL		pageGetDataBitmap(void* pageRef, STNBBitmapProps* dstProps, void** dstData);
		void		pageRetain(void* pageRef);
		void		pageRelease(void* pageRef);
		STNBRect	pageGetBox(void* pageRef);
		BOOL		pageGetLabel(void* pageRef, STNBString* dst);
		BOOL		pageGetText(void* pageRef, STNBString* dst);
		BOOL		pageRenderArea(void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst);
		//Render job
		void*						renderStartToMemory(const STNBPdfRenderDoc* job);
		ENMngrPdfKitRenderResult	renderContinue(void* jobRef, const STNBPdfRenderDoc* job);
		void						renderCancel(void* jobRef);
		void*						renderEndAndOpenDoc(void* jobRef);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		static STMngrPdfKitCalls _calls;
};

#endif /* AUMngrPdfKit_h */
