//
//  AUAppGlueApplePdfKit.mm
//  lib-auframework-app
//
//  Created by Marcos Ortega on 20/3/19.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueApplePdfKit.h"
//
#include "NBMngrPdfKit.h"
#include <PDFKit/PDFKit.h>
#if TARGET_OS_IPHONE
//
#else
//	PDFKit is included in Quartz module
#	import <Quartz/Quartz.h>
#endif

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

typedef enum ENAppGlueApplePdfKitDataType_ {
	ENAppGlueApplePdfKitDataType_Unknown = 0,
	ENAppGlueApplePdfKitDataType_Doc,
	ENAppGlueApplePdfKitDataType_Page,
	ENAppGlueApplePdfKitDataType_Render
} ENAppGlueApplePdfKitDataType;

typedef struct AUAppGlueApplePdfKitData_ {
	AUAppI*		app;
} AUAppGlueApplePdfKitData;

struct AUAppGlueApplePdfKitDoc_;

typedef struct AUAppGlueApplePdfKitPage_ {
	ENAppGlueApplePdfKitDataType type;
	AUAppGlueApplePdfKitData*	glue;
	STNBThreadMutex				mutex;
	struct AUAppGlueApplePdfKitDoc_* doc;
	struct {
		SI32					idx;		//las known index (can change when file is updated)
		STNBRect				mediaBox;	//las known mediabox (is the biggest and only-one-required of all boxes)
	} cache;
	//iOS11+
	PDFPage*					page;
	//
	SI32						retainCount;
} AUAppGlueApplePdfKitPage;

typedef struct AUAppGlueApplePdfKitDoc_ {
	ENAppGlueApplePdfKitDataType type;
	AUAppGlueApplePdfKitData*	glue;
	STNBThreadMutex				mutex;
	STNBArray					pages;	//AUAppGlueApplePdfKitPage*
	STNBString					filename; //if was loaded from file
	//iOS11+
	PDFDocument*				doc;
	//
	SI32						retainCount;
} AUAppGlueApplePdfKitDoc;

typedef struct AUAppGlueApplePdfKitRender_ {
	ENAppGlueApplePdfKitDataType type;
	AUAppGlueApplePdfKitData*	glue;
	STNBThreadMutex				mutex;
	//
	NSMutableData*				pdfData;	//if destination is memory
#	if !(TARGET_OS_IPHONE)
	CGDataConsumerRef			consumer;
#	endif
	CGContextRef				context;
	UI32						iNextPage;	//current progress
	//
	SI32						retainCount;
} AUAppGlueApplePdfKitRender;

//Calls

bool AUAppGlueApplePdfKit::create(AUAppI* app, STMngrPdfKitCalls* obj){
	AUAppGlueApplePdfKitData* data		= NBMemory_allocType(AUAppGlueApplePdfKitData);
	NBMemory_setZeroSt(*data, AUAppGlueApplePdfKitData);
	NBMemory_setZeroSt(*obj, STMngrPdfKitCalls);
	data->app							= (AUAppI*)app;
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

bool AUAppGlueApplePdfKit::destroy(void* param){
	bool r = false;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL){
		NBMemory_free(obj);
		r = true;
	}
	return r;
}

//Specs

BOOL AUAppGlueApplePdfKit::isSupported(){	//Determines if the current OS version supports this rendering engine
	BOOL r = FALSE;
	{
		Class clsPage = NSClassFromString(@"PDFPage");
		if(clsPage){
			if([clsPage instancesRespondToSelector:@selector(drawWithBox:toContext:)]){
				r = TRUE;
			}
		}
	}
	return r;
}

BOOL AUAppGlueApplePdfKit::canMultithread(void* param){
	return TRUE;
}

//Document

void* AUAppGlueApplePdfKit::docOpenFromPath(void* param, const char* path){
	void* r = NULL;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL && path != NULL){
		//iOS11+
		@autoreleasepool {
			NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:path]];
			PDFDocument* doc = [[PDFDocument alloc] initWithURL:url];
			if(doc != nil){
				AUAppGlueApplePdfKitDoc* dd = NBMemory_allocType(AUAppGlueApplePdfKitDoc);
				NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitDoc);
				dd->type		= ENAppGlueApplePdfKitDataType_Doc;
				dd->glue		= obj;
				dd->doc			= doc;
				dd->retainCount	= 1;
				NBString_init(&dd->filename); //if was loaded from file
				{
					const SI32 pathLen	= NBString_strLenBytes(path);
					const SI32 iSlash1	= NBString_strLastIndexOf(path, "/", pathLen);
					const SI32 iSlash2	= NBString_strLastIndexOf(path, "\\", pathLen);
					const SI32 iSlash	= (iSlash1 > iSlash2 ? iSlash1 : iSlash2);
					NBString_concat(&dd->filename, &path[iSlash]);
				}
				NBArray_init(&dd->pages, sizeof(AUAppGlueApplePdfKitPage*), NULL);
				NBThreadMutex_init(&dd->mutex);
				//PRINTF_INFO("PdfDoc opened from filename('%s') path('%s').\n", dd->filename.str, path);
				/*{
					const SI32 rc = (SI32)[dd->doc retainCount];
					PRINTF_INFO("PdfDoc retain count at start: %d.\n", rc);
				}*/
				r = dd;
			}
		}
	}
	return r;
}

void* AUAppGlueApplePdfKit::docOpenFromData(void* param, const void* data, const UI32 dataSz){
	void* r = NULL;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL){
		//iOS11+
		@autoreleasepool {
			NSData* dd = [NSData dataWithBytes:data length:dataSz];
			PDFDocument* doc = [[PDFDocument alloc] initWithData:dd];
			if(doc != nil){
				AUAppGlueApplePdfKitDoc* dd = NBMemory_allocType(AUAppGlueApplePdfKitDoc);
				NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitDoc);
				dd->type		= ENAppGlueApplePdfKitDataType_Doc;
				dd->glue		= obj;
				dd->doc			= doc;
				dd->retainCount	= 1;
				NBString_init(&dd->filename); //if was loaded from file
				NBArray_init(&dd->pages, sizeof(AUAppGlueApplePdfKitPage*), NULL);
				NBThreadMutex_init(&dd->mutex);
				PRINTF_INFO("PdfDoc opened from data(%d bytes).\n", dataSz);
				/*{
					const SI32 rc = (SI32)[dd->doc retainCount];
					PRINTF_INFO("PdfDoc retain count at start: %d.\n", rc);
				}*/
				r = dd;
			}
		}
	}
	return r;
}

void* AUAppGlueApplePdfKit::docOpenFromRenderJob(void* param, const STNBPdfRenderDoc* job){
	void* r = NULL;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL){
#		if TARGET_OS_IPHONE
		@autoreleasepool {
			NSMutableData* pdfData	= [NSMutableData data];
			//Render
			//OSX creates a CGContextRef system with Y-axis normal, where (0, 0) is lower-left corner.
			//iOS helpers create a CGContextRef system with Y-axis inverted, where (0, 0) is upper-left corner.
			{
				CGRect bounds			= CGRectMake(0, 0, 612, 792);
				UIGraphicsBeginPDFContextToData(pdfData, bounds, nil );
				{
					CGContextRef context = UIGraphicsGetCurrentContext();
					SI32 i; for(i = 0; i < job->pages.use; i++){
						const STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&job->pages, STNBPdfRenderPage*, i);
						CGRect bounds			= CGRectMake(0, 0, page->size.width, page->size.height);
						NSDictionary* pgInfo	= nil;
						//Load sizes from template page
						if(page->sizeTemplate.pageRef != NULL){
							AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
							if(pp != NULL){
								NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
								NBThreadMutex_lock(&pp->mutex);
								if(pp->page != NULL){
									const CGRect bMedia	= [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
									const CGRect bCrop	= [pp->page boundsForBox:kPDFDisplayBoxCropBox];
									const CGRect bBleed	= [pp->page boundsForBox:kPDFDisplayBoxBleedBox];
									const CGRect bTrim	= [pp->page boundsForBox:kPDFDisplayBoxTrimBox];
									const CGRect bArt	= [pp->page boundsForBox:kPDFDisplayBoxArtBox];
									bounds			= bMedia;
									pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
													   [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
													   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
													   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
													   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
													   , nil];
									//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
									//adding the "kCGPDFContextMediaBox" key to the dictionary.
								}
								NBThreadMutex_unlock(&pp->mutex);
							} else {
								void* bmpData = NULL;
								STNBBitmapProps bmpProps;
								NBMemory_setZeroSt(bmpProps, STNBBitmapProps);
								if(NBMngrPdfKit::pageGetDataBitmap(page->sizeTemplate.pageRef, &bmpProps, &bmpData)){
									const CGRect bMedia	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bCrop	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bBleed	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bTrim	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bArt	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									bounds			= bMedia;
									pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
													   [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
													   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
													   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
													   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
													   , nil];
									//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
									//adding the "kCGPDFContextMediaBox" key to the dictionary.
								} else {
									NBASSERT(FALSE) //ERROR, undetermined type of content
								}
							}
						}
						//Build page
						{
							UIGraphicsBeginPDFPageWithInfo(bounds, pgInfo);
							//Itms
							{
								SI32 i; for(i = 0; i < page->itms.use; i++){
									const STNBPdfRenderPageItm* itm = NBArray_itmPtrAtIndex(&page->itms, STNBPdfRenderPageItm, i);
									const float rotRad = (itm->matrix.rotDeg * M_PI / 180.0);
									//Apply transform
									{
										CGContextTranslateCTM(context, itm->matrix.traslation.x, itm->matrix.traslation.y);
										CGContextRotateCTM(context, rotRad);
									}
									//Draw
									{
										switch(itm->type){
											case ENNBPdfRenderPageItmType_Text:
												{
													UIColor* color		= [UIColor colorWithRed:(itm->text.fontColor.r / 255.0f) green:(itm->text.fontColor.g / 255.0f) blue:(itm->text.fontColor.b / 255.0f) alpha:(itm->text.fontColor.a / 255.0f)];
													UIFont* font		= [UIFont systemFontOfSize:itm->text.fontSize];
													NSString* str		= [NSString stringWithUTF8String:itm->text.text.str];
													NSDictionary* atts	= [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, color, NSForegroundColorAttributeName, nil];
													NSAttributedString* strAtt = [[NSAttributedString alloc] initWithString:str attributes:atts];
													CGPoint pos			= CGPointMake(0.0f, 0.0f);
													[strAtt drawAtPoint:pos];
													[strAtt release];
													strAtt = nil;
												}
												break;
											case ENNBPdfRenderPageItmType_Image:
												{
													NSData* data = [NSData dataWithBytes:itm->image.data.str length:itm->image.data.length];
													CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
													if(imgDataProvider != nil){
														CGImageRef image = nil;
														switch (itm->image.type) {
															case ENNBPdfRenderPageItmImgType_Png:
																image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																break;
															case ENNBPdfRenderPageItmImgType_Jpeg:
																image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																break;
															default:
																NBASSERT(FALSE)
																break;
														}
														if(image != nil){
															//Render
															CGRect bounds = CGRectMake(itm->image.rect.x, itm->image.rect.y, itm->image.rect.width, itm->image.rect.height);
															CGContextDrawImage(context, bounds, image);
															CGImageRelease(image);
															image = nil;
														}
														CGDataProviderRelease(imgDataProvider);
														imgDataProvider = nil;
													}
												}
												break;
											case ENNBPdfRenderPageItmType_PdfPage:
												if(itm->pdfPage.pageRef != NULL){
													AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
													if(pp != NULL){
														NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
														NBThreadMutex_lock(&pp->mutex);
														if(pp->page != NULL){
															const CGRect bMedia = [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
															const float sizeH	= (bMedia.size.height - bMedia.origin.y);
															CGContextTranslateCTM(context, 0.0f, sizeH);
															CGContextScaleCTM(context, 1.0f, -1.0f);
															{
																NBThreadMutex_unlock(&pp->mutex);
																{
																	CGContextDrawPDFPage(context, [pp->page pageRef]);
																}
																NBThreadMutex_lock(&pp->mutex);
															}
															CGContextScaleCTM(context, 1.0f, -1.0f);
															CGContextTranslateCTM(context, 0.0f, -sizeH);
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
																if(!NBFile_openAsString(stream, &imgData)){
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
																		if(!NBPng_saveDataToFile(&bmpProps, bmpData, stream, ENPngCompressLvl_9)){
																			r = FALSE; NBASSERT(FALSE)
																		} else {
																			imgType = ENNBPdfRenderPageItmImgType_Png;
																		}
																	}
																	if(imgType == ENNBPdfRenderPageItmImgType_Jpeg || imgType == ENNBPdfRenderPageItmImgType_Png){
																		//Draw image into page
																		NSData* data = [NSData dataWithBytes:imgData.str length:imgData.length];
																		CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
																		if(imgDataProvider != nil){
																			CGImageRef image = nil;
																			switch (imgType) {
																				case ENNBPdfRenderPageItmImgType_Png:
																					image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																					break;
																				case ENNBPdfRenderPageItmImgType_Jpeg:
																					image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																					break;
																				default:
																					NBASSERT(FALSE)
																					break;
																			}
																			if(image != nil){
																				//Render
																				const float scaleW = ((bounds.size.width * 0.75f) / bmpProps.size.width);
																				const float scaleH = ((bounds.size.height * 0.75f) / bmpProps.size.height);
																				float imgScale = (scaleW < scaleH ? scaleW : scaleH);
																				if(imgScale > 1.0f){
																					imgScale = 1.0f;
																				}
																				{
																					CGRect imgBounds = CGRectMake(bounds.origin.x + ((bounds.size.width - (bmpProps.size.width * imgScale)) * 0.5f), bounds.origin.y + ((bounds.size.height - (bmpProps.size.height * imgScale)) * 0.5f) + (bmpProps.size.height * imgScale), bmpProps.size.width * imgScale, -(bmpProps.size.height * imgScale));
																					CGContextDrawImage(context, imgBounds, image);
																				}
																				CGImageRelease(image);
																				image = nil;
																			}
																			CGDataProviderRelease(imgDataProvider);
																			imgDataProvider = nil;
																		}
																	}
																	/*NBFile_lock(stream);
																	{
																		STNBJpegWrite jWrite;
																		NBJpegWrite_init(&jWrite);
																		if(!NBJpegWrite_feedStart(&jWrite, bmpData, (bmpProps.bytesPerLine * bmpProps.size.height), bmpProps, &stream, 85, 10)){
																			NBASSERT(FALSE) //ERROR
																		} else {
																			SI32 ciclesCount = 0;
																			ENJpegWriteResult rr = ENJpegWriteResult_error;
																			while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																				ciclesCount++;
																			}
																			if(rr != ENJpegWriteResult_end){
																				NBASSERT(FALSE) //ERROR
																			} else {
																				//
																			}
																			NBJpegWrite_feedEnd(&jWrite);
																		}
																		NBJpegWrite_release(&jWrite);
																	}
																	NBFile_unlock(stream);*/
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
									//Reset transform
									{
										CGContextRotateCTM(context, -rotRad);
										CGContextTranslateCTM(context, -itm->matrix.traslation.x, -itm->matrix.traslation.y);
									}
								}
							}
						}
					}
				}
				UIGraphicsEndPDFContext();
			}
			//Load
			{
				//iOS11+
				PDFDocument* doc = [[PDFDocument alloc] initWithData:pdfData];
				if(doc != nil){
					AUAppGlueApplePdfKitDoc* dd = NBMemory_allocType(AUAppGlueApplePdfKitDoc);
					NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitDoc);
					dd->type		= ENAppGlueApplePdfKitDataType_Doc;
					dd->glue		= obj;
					dd->doc			= doc;
					dd->retainCount	= 1;
					NBString_init(&dd->filename); //if was loaded from file
					NBArray_init(&dd->pages, sizeof(AUAppGlueApplePdfKitPage*), NULL);
					NBThreadMutex_init(&dd->mutex);
					PRINTF_INFO("PdfDoc opened from render job.\n");
					/*{
						const SI32 rc = (SI32)[dd->doc retainCount];
						PRINTF_INFO("PdfDoc retain count at start: %d.\n", rc);
					}*/
					r = dd;
				}
			}
		}
#		else
		@autoreleasepool {
			NSMutableData* pdfData	= [NSMutableData data];
			//Render
			//OSX creates a CGContextRef system with Y-axis normal, where (0, 0) is lower-left corner.
			//iOS helpers create a CGContextRef system with Y-axis inverted, where (0, 0) is upper-left corner.
			{
				CGRect bounds = CGRectMake(0, 0, 612, 792);
				CGDataConsumerRef consumer = CGDataConsumerCreateWithCFData((CFMutableDataRef)pdfData);
				{
					CGContextRef context = CGPDFContextCreate(consumer, &bounds, nil);
					//NSGraphicsContext* context2 = [NSGraphicsContext graphicsContextWithCGContext:context flipped:FALSE];
					//[NSGraphicsContext saveGraphicsState];
					//[NSGraphicsContext setCurrentContext:context2];
					SI32 i; for(i = 0; i < job->pages.use; i++){
						const STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&job->pages, STNBPdfRenderPage*, i);
						CGRect bounds			= CGRectMake(0, 0, page->size.width, page->size.height);
						NSDictionary* pgInfo	= nil;
						//Load sizes from template page
						if(page->sizeTemplate.pageRef != NULL){
							AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
							if(pp != NULL){
								NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
								NBThreadMutex_lock(&pp->mutex);
								if(pp->page != NULL){
									const CGRect bMedia	= [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
									const CGRect bCrop	= [pp->page boundsForBox:kPDFDisplayBoxCropBox];
									const CGRect bBleed	= [pp->page boundsForBox:kPDFDisplayBoxBleedBox];
									const CGRect bTrim	= [pp->page boundsForBox:kPDFDisplayBoxTrimBox];
									const CGRect bArt	= [pp->page boundsForBox:kPDFDisplayBoxArtBox];
									bounds			= bMedia;
									pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
													   [NSData dataWithBytes:&bMedia length:sizeof(bMedia)], kCGPDFContextMediaBox
													   , [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
													   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
													   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
													   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
													   , nil];
									//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
									//adding the "kCGPDFContextMediaBox" key to the dictionary.
								}
								NBThreadMutex_unlock(&pp->mutex);
							} else {
								void* bmpData = NULL;
								STNBBitmapProps bmpProps;
								NBMemory_setZeroSt(bmpProps, STNBBitmapProps);
								if(NBMngrPdfKit::pageGetDataBitmap(page->sizeTemplate.pageRef, &bmpProps, &bmpData)){
									const CGRect bMedia	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bCrop	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bBleed	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bTrim	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									const CGRect bArt	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
									bounds			= bMedia;
									pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
													   [NSData dataWithBytes:&bMedia length:sizeof(bMedia)], kCGPDFContextMediaBox
													   , [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
													   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
													   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
													   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
													   , nil];
									//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
									//adding the "kCGPDFContextMediaBox" key to the dictionary.
								} else {
									NBASSERT(FALSE) //ERROR, undetermined type of content
								}
							}
						}
						//Build page
						{
							CGPDFContextBeginPage(context, (CFDictionaryRef)pgInfo);
							//Invert Y-axis
							{
								CGContextTranslateCTM(context, 0.0f, bounds.origin.y + bounds.size.height);
								CGContextScaleCTM(context, 1.0f, -1.0f);
							}
							//Itms
							{
								SI32 i; for(i = 0; i < page->itms.use; i++){
									const STNBPdfRenderPageItm* itm = NBArray_itmPtrAtIndex(&page->itms, STNBPdfRenderPageItm, i);
									const float rotRad = (itm->matrix.rotDeg * M_PI / 180.0);
									//Apply transform
									{
										CGContextTranslateCTM(context, itm->matrix.traslation.x, itm->matrix.traslation.y);
										CGContextRotateCTM(context, rotRad);
									}
									//Draw
									{
										switch(itm->type){
											case ENNBPdfRenderPageItmType_Text:
												{
													NSColor* color		= [NSColor colorWithRed:(itm->text.fontColor.r / 255.0f) green:(itm->text.fontColor.g / 255.0f) blue:(itm->text.fontColor.b / 255.0f) alpha:(itm->text.fontColor.a / 255.0f)];
													NSFont* font		= [NSFont systemFontOfSize:itm->text.fontSize];
													NSString* str		= [NSString stringWithUTF8String:itm->text.text.str];
													NSDictionary* atts	= [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, color, NSForegroundColorAttributeName, nil];
													NSAttributedString* strAtt = [[NSAttributedString alloc] initWithString:str attributes:atts];
													//CoreText method
													{
														CGContextSetRGBStrokeColor(context, (itm->text.fontColor.r / 255.0f), (itm->text.fontColor.g / 255.0f), (itm->text.fontColor.b / 255.0f), (itm->text.fontColor.a / 255.0f));
														CGContextSetRGBFillColor(context, (itm->text.fontColor.r / 255.0f), (itm->text.fontColor.g / 255.0f), (itm->text.fontColor.b / 255.0f), (itm->text.fontColor.a / 255.0f));
														CGContextScaleCTM(context, 1.0f, -1.0f);
														{
															CTFramesetterRef framesetter = CTFramesetterCreateWithAttributedString((CFAttributedStringRef)strAtt);
															//
															CGMutablePathRef leftColumnPath = CGPathCreateMutable();
															CGPathAddRect(leftColumnPath, NULL, CGRectMake(0, -1024, 1024, 1024));
															//
															CTFrameRef leftFrame = CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), leftColumnPath, NULL);
															//
															CTFrameDraw(leftFrame, context);
															//
															CFRelease(leftFrame);
															CFRelease(framesetter);
															CGPathRelease(leftColumnPath);
														}
														CGContextScaleCTM(context, 1.0f, -1.0f);
														CGContextSetRGBStrokeColor(context, 1.0f, 1.0f, 1.0f, 1.0f);
														CGContextSetRGBFillColor(context, 1.0f, 1.0f, 1.0f, 1.0f);
													}
													[strAtt release];
													strAtt = nil;
												}
												break;
											case ENNBPdfRenderPageItmType_Image:
												{
													NSData* data = [NSData dataWithBytes:itm->image.data.str length:itm->image.data.length];
													CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
													if(imgDataProvider != nil){
														CGImageRef image = nil;
														switch (itm->image.type) {
															case ENNBPdfRenderPageItmImgType_Png:
																image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																break;
															case ENNBPdfRenderPageItmImgType_Jpeg:
																image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																break;
															default:
																NBASSERT(FALSE)
																break;
														}
														if(image != nil){
															//Render
															CGRect bounds = CGRectMake(itm->image.rect.x, itm->image.rect.y, itm->image.rect.width, itm->image.rect.height);
															CGContextDrawImage(context, bounds, image);
															CGImageRelease(image);
															image = nil;
														}
														CGDataProviderRelease(imgDataProvider);
														imgDataProvider = nil;
													}
												}
												break;
											case ENNBPdfRenderPageItmType_PdfPage:
												if(itm->pdfPage.pageRef != NULL){
													AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
													if(pp != NULL){
														NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
														NBThreadMutex_lock(&pp->mutex);
														if(pp->page != NULL){
															const CGRect bMedia = [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
															const float sizeH	= (bMedia.size.height - bMedia.origin.y);
															CGContextTranslateCTM(context, 0.0f, sizeH);
															CGContextScaleCTM(context, 1.0f, -1.0f);
															{
																NBThreadMutex_unlock(&pp->mutex);
																{
																	CGContextDrawPDFPage(context, [pp->page pageRef]);
																}
																NBThreadMutex_lock(&pp->mutex);
															}
															CGContextScaleCTM(context, 1.0f, -1.0f);
															CGContextTranslateCTM(context, 0.0f, -sizeH);
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
																if(!NBFile_openAsString(stream, &imgData)){
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
																		if(!NBPng_saveDataToFile(&bmpProps, bmpData, stream, ENPngCompressLvl_9)){
																			r = FALSE; NBASSERT(FALSE)
																		} else {
																			imgType = ENNBPdfRenderPageItmImgType_Png;
																		}
																	}
																	if(imgType == ENNBPdfRenderPageItmImgType_Jpeg || imgType == ENNBPdfRenderPageItmImgType_Png){
																		//Draw image into page
																		NSData* data = [NSData dataWithBytes:imgData.str length:imgData.length];
																		CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
																		if(imgDataProvider != nil){
																			CGImageRef image = nil;
																			switch (imgType) {
																				case ENNBPdfRenderPageItmImgType_Png:
																					image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																					break;
																				case ENNBPdfRenderPageItmImgType_Jpeg:
																					image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																					break;
																				default:
																					NBASSERT(FALSE)
																					break;
																			}
																			if(image != nil){
																				//Render
																				const float scaleW = ((bounds.size.width * 0.75f) / bmpProps.size.width);
																				const float scaleH = ((bounds.size.height * 0.75f) / bmpProps.size.height);
																				float imgScale = (scaleW < scaleH ? scaleW : scaleH);
																				if(imgScale > 1.0f){
																					imgScale = 1.0f;
																				}
																				{
																					CGRect imgBounds = CGRectMake(bounds.origin.x + ((bounds.size.width - (bmpProps.size.width * imgScale)) * 0.5f), bounds.origin.y + ((bounds.size.height - (bmpProps.size.height * imgScale)) * 0.5f) + (bmpProps.size.height * imgScale), bmpProps.size.width * imgScale, -(bmpProps.size.height * imgScale));
																					CGContextDrawImage(context, imgBounds, image);
																				}
																				CGImageRelease(image);
																				image = nil;
																			}
																			CGDataProviderRelease(imgDataProvider);
																			imgDataProvider = nil;
																		}
																	}
																	/*NBFile_lock(stream);
																	{
																		STNBJpegWrite jWrite;
																		NBJpegWrite_init(&jWrite);
																		if(!NBJpegWrite_feedStart(&jWrite, bmpData, (bmpProps.bytesPerLine * bmpProps.size.height), bmpProps, &stream, 85, 10)){
																			NBASSERT(FALSE) //ERROR
																		} else {
																			SI32 ciclesCount = 0;
																			ENJpegWriteResult rr = ENJpegWriteResult_error;
																			while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																				ciclesCount++;
																			}
																			if(rr != ENJpegWriteResult_end){
																				NBASSERT(FALSE) //ERROR
																			} else {
																				//
																			}
																			NBJpegWrite_feedEnd(&jWrite);
																		}
																		NBJpegWrite_release(&jWrite);
																	}
																	NBFile_unlock(stream);*/
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
									//Reset transform
									{
										CGContextRotateCTM(context, -rotRad);
										CGContextTranslateCTM(context, -itm->matrix.traslation.x, -itm->matrix.traslation.y);
									}
								}
							}
							CGPDFContextEndPage(context);
						}
					}
					//[NSGraphicsContext restoreGraphicsState];
					CGContextRelease(context);
				}
			}
			//Load
			{
				//iOS11+
				PDFDocument* doc = [[PDFDocument alloc] initWithData:pdfData];
				if(doc == nil){
					PRINTF_ERROR("PDFDocument initWithData failed.\n");
				} else {
					AUAppGlueApplePdfKitDoc* dd = NBMemory_allocType(AUAppGlueApplePdfKitDoc);
					NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitDoc);
					dd->type		= ENAppGlueApplePdfKitDataType_Doc;
					dd->glue		= obj;
					dd->doc			= doc;
					dd->retainCount	= 1;
					NBString_init(&dd->filename); //if was loaded from file
					NBArray_init(&dd->pages, sizeof(AUAppGlueApplePdfKitPage*), NULL);
					NBThreadMutex_init(&dd->mutex);
					PRINTF_INFO("PdfDoc opened from render job.\n");
					/*{
						const SI32 rc = (SI32)[dd->doc retainCount];
						PRINTF_INFO("PdfDoc retain count at start: %d.\n", rc);
					}*/
					r = dd;
				}
			}
		}
#		endif
	}
	return r;
}

void AUAppGlueApplePdfKit::docRetain(void* param, void* docRef){
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitDoc* dd = (AUAppGlueApplePdfKitDoc*)docRef;
	if(obj != NULL && dd != NULL){
		@autoreleasepool {
			NBASSERT(dd->type == ENAppGlueApplePdfKitDataType_Doc)
			NBThreadMutex_lock(&dd->mutex);
			NBASSERT(dd->retainCount > 0)
			dd->retainCount++;
			NBThreadMutex_unlock(&dd->mutex);
		}
	}
}

void AUAppGlueApplePdfKit_docReleaseAndFreeLocked_(AUAppGlueApplePdfKitDoc* dd){
	if(dd != NULL){
		PRINTF_INFO("PdfDoc freeing ('%s').\n", dd->filename.str);
		@autoreleasepool {
			//Release pages
			{
				NBASSERT(dd->pages.use == 0) //Should be released after no pages remain
				NBArray_release(&dd->pages);
			}
			if(dd->doc != NULL){
				/*{
					const SI32 rc = (SI32)[dd->doc retainCount];
					PRINTF_INFO("PdfDoc retain count at end: %d ('%s').\n", rc, dd->filename.str);
				}*/
				[dd->doc release];
				dd->doc = NULL;
			}
			dd->glue = NULL;
			dd->type = ENAppGlueApplePdfKitDataType_Unknown;
			NBString_release(&dd->filename); //if was loaded from file
			NBThreadMutex_unlock(&dd->mutex);
			NBThreadMutex_release(&dd->mutex);
			NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitDoc);
			NBMemory_free(dd);
		}
	}
}

void AUAppGlueApplePdfKit::docRelease(void* param, void* docRef){
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitDoc* dd = (AUAppGlueApplePdfKitDoc*)docRef;
	if(obj != NULL && dd != NULL){
		@autoreleasepool {
			NBASSERT(dd->type == ENAppGlueApplePdfKitDataType_Doc)
			NBThreadMutex_lock(&dd->mutex);
			NBASSERT(dd->retainCount > 0)
			dd->retainCount--;
			if(dd->retainCount > 0){
				NBThreadMutex_unlock(&dd->mutex);
			} else {
				//Unlocked by this call
				AUAppGlueApplePdfKit_docReleaseAndFreeLocked_(dd);
			}
		}
	}
}

UI32 AUAppGlueApplePdfKit::docGetPagesCount(void* param, void* docRef){
	UI32 r = 0;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitDoc* dd = (AUAppGlueApplePdfKitDoc*)docRef;
	if(obj != NULL && dd != NULL){
		@autoreleasepool {
			NBASSERT(dd->type == ENAppGlueApplePdfKitDataType_Doc)
			NBThreadMutex_lock(&dd->mutex);
			if(dd->doc != NULL){
				r = (UI32)[dd->doc pageCount];
			}
			NBThreadMutex_unlock(&dd->mutex);
		}
	}
	return r;
}

void* AUAppGlueApplePdfKit::docGetPageAtIdx(void* param, void* docRef, const UI32 idx){
	void* r = NULL;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitDoc* dd = (AUAppGlueApplePdfKitDoc*)docRef;
	if(obj != NULL && dd != NULL){
		@autoreleasepool {
			NBASSERT(dd->type == ENAppGlueApplePdfKitDataType_Doc)
			NBThreadMutex_lock(&dd->mutex);
			if(dd->doc != NULL){
				//Search in current array
				if(r == NULL){
					UI32 i; for(i = 0 ; i < dd->pages.use && r == NULL; i++){
						AUAppGlueApplePdfKitPage* pp = NBArray_itmValueAtIndex(&dd->pages, AUAppGlueApplePdfKitPage*, i);
						NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page || pp->type == ENAppGlueApplePdfKitDataType_Unknown)
						//when releasing the object is marked as unknown
						if(pp->type == ENAppGlueApplePdfKitDataType_Page){
							NBThreadMutex_lock(&pp->mutex);
							//Update index (if necesary)
							if(pp->cache.idx < 0){
								const NSUInteger idx = [dd->doc indexForPage:pp->page];
								if(idx != NSNotFound){
									pp->cache.idx = (SI32)idx;
								}
							}
							//
							if(pp->cache.idx == idx){
								pp->retainCount++; //increase retain
								//PRINTF_INFO("PdfPage #%d found in already loaded ('%s').\n", (idx + 1), dd->filename.str);
								r = pp;
							}
							NBThreadMutex_unlock(&pp->mutex);
						}
					}
				}
				//Create new page
				if(r == NULL){
					const UI32 pagesCount = (UI32)[dd->doc pageCount];
					if(idx >= 0 && idx < pagesCount){
						PDFPage* page = [dd->doc pageAtIndex:idx];
						if(page != nil){
							AUAppGlueApplePdfKitPage* pp = NBMemory_allocType(AUAppGlueApplePdfKitPage);
							NBMemory_setZeroSt(*pp, AUAppGlueApplePdfKitPage);
							pp->type		= ENAppGlueApplePdfKitDataType_Page;
							pp->glue		= obj;
							pp->doc			= dd;
							pp->page		= page;
							{
								pp->cache.idx	= idx;
#								if TARGET_OS_IPHONE
								{
									const CGRect rect = [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
									pp->cache.mediaBox.x		= rect.origin.x;
									pp->cache.mediaBox.y		= rect.origin.y;
									pp->cache.mediaBox.width	= rect.size.width;
									pp->cache.mediaBox.height	= rect.size.height;
								}
#								else
								{
									const NSRect rect = [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
									pp->cache.mediaBox.x		= rect.origin.x;
									pp->cache.mediaBox.y		= rect.origin.y;
									pp->cache.mediaBox.width	= rect.size.width;
									pp->cache.mediaBox.height	= rect.size.height;
								}
#								endif
							}
							[pp->page retain];
							pp->retainCount	= 1;
							NBThreadMutex_init(&pp->mutex);
							//UPdate doc
							{
								//Add to array
								NBASSERT(dd->retainCount > 0)
								dd->retainCount++;	//page retains document
								NBArray_addValue(&dd->pages, pp);
							}
							//PRINTF_INFO("PdfPage #%d newly loaded ('%s').\n", (idx + 1), dd->filename.str);
							/*{
								const SI32 rc = (SI32)[pp->page retainCount];
								PRINTF_INFO("PdfPage retain count at start: %d.\n", rc);
							}*/
							r = pp;
						}
					}
				}
			}
			NBThreadMutex_unlock(&dd->mutex);
		}
	}
	return r;
}

BOOL AUAppGlueApplePdfKit::docWriteToFilepath(void* param, void* docRef, const char* filepath){
	BOOL r = FALSE;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitDoc* dd = (AUAppGlueApplePdfKitDoc*)docRef;
	if(obj != NULL && dd != NULL){
		@autoreleasepool {
			NBASSERT(dd->type == ENAppGlueApplePdfKitDataType_Doc)
			NBThreadMutex_lock(&dd->mutex);
			if(dd->doc != NULL){
				r = [dd->doc writeToFile:[NSString stringWithUTF8String:filepath]];
			}
			NBThreadMutex_unlock(&dd->mutex);
		}
	}
	return r;
}

//Page

void AUAppGlueApplePdfKit_pageRetainLocked_(AUAppGlueApplePdfKitPage* pp){
	NBASSERT(pp->retainCount > 0)
	pp->retainCount++;
}
	
void AUAppGlueApplePdfKit::pageRetain(void* param, void* pageRef){
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)pageRef;
	if(obj != NULL && pp != NULL){
		NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
		NBThreadMutex_lock(&pp->mutex);
		{
			AUAppGlueApplePdfKit_pageRetainLocked_(pp);
		}
		NBThreadMutex_unlock(&pp->mutex);
	}
}

void AUAppGlueApplePdfKit_pageReleaseLockedAndUnlock_(AUAppGlueApplePdfKitPage* pp){
	NBASSERT(pp->retainCount > 0)
	pp->retainCount--;
	if(pp->retainCount > 0){
		NBThreadMutex_unlock(&pp->mutex);
	} else {
		AUAppGlueApplePdfKitDoc* dd = pp->doc; pp->doc = NULL;
		//Release page
		{
			pp->type	= ENAppGlueApplePdfKitDataType_Unknown;
			NBThreadMutex_unlock(&pp->mutex);
		}
		//Release doc
		{
			NBASSERT(dd != NULL)
			if(dd != NULL){
				NBASSERT(dd->type == ENAppGlueApplePdfKitDataType_Doc)
				NBThreadMutex_lock(&dd->mutex);
				NBASSERT(dd->retainCount > 0)
				//Remove page from doc
				{
					BOOL found = FALSE;
					SI32 i; for(i = 0; i < dd->pages.use; i++){
						AUAppGlueApplePdfKitPage* pp2 = NBArray_itmValueAtIndex(&dd->pages, AUAppGlueApplePdfKitPage*, i);
						NBASSERT(pp2->type == ENAppGlueApplePdfKitDataType_Page || pp2->type == ENAppGlueApplePdfKitDataType_Unknown);
						if(pp2 == pp){
							//Release
							{
								if(pp->page != NULL){
									/*{
									 const SI32 rc = (SI32)[pp->page retainCount];
									 PRINTF_INFO("PdfPage retain count at end: %d.\n", rc);
									 }*/
									[pp->page release];
									pp->page = NULL;
								}
								pp->glue	= NULL;
								NBThreadMutex_release(&pp->mutex);
								NBMemory_setZeroSt(*pp, AUAppGlueApplePdfKitPage);
								NBMemory_free(pp);
							}
							//Remove
							NBArray_removeItemAtIndex(&dd->pages, i);
							found = TRUE;
							break;
						}
					} NBASSERT(found)
				}
				//Release doc
				dd->retainCount--;
				if(dd->retainCount > 0){
					NBThreadMutex_unlock(&dd->mutex);
				} else {
					//Unlocked by this call
					AUAppGlueApplePdfKit_docReleaseAndFreeLocked_(dd);
				}
			}
		}
	}
}
	
void AUAppGlueApplePdfKit::pageRelease(void* param, void* pageRef){
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)pageRef;
	if(obj != NULL && pp != NULL){
		@autoreleasepool {
			NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
			NBThreadMutex_lock(&pp->mutex);
			AUAppGlueApplePdfKit_pageReleaseLockedAndUnlock_(pp); //It will unlock
		}
	}
}

STNBRect AUAppGlueApplePdfKit::pageGetBox(void* param, void* pageRef){
	STNBRect r;
	NBMemory_setZeroSt(r, STNBRect);
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)pageRef;
	if(obj != NULL && pp != NULL){
		//@autoreleasepool {
			NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
			NBThreadMutex_lock(&pp->mutex);
			r = pp->cache.mediaBox;
			NBThreadMutex_unlock(&pp->mutex);
		//}
	}
	return r;
}

BOOL AUAppGlueApplePdfKit::pageGetLabel(void* param, void* pageRef, STNBString* dst){
	BOOL r = FALSE;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)pageRef;
	if(obj != NULL && pp != NULL){
		@autoreleasepool {
			NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
			NBThreadMutex_lock(&pp->mutex);
			{
				NSString* lbl = [pp->page label];
				if(lbl != nil){
					if(dst != NULL){
						NBString_set(dst, [lbl UTF8String]);
					}
					r = TRUE;
				}
			}
			NBThreadMutex_unlock(&pp->mutex);
		}
	}
	return r;
}

BOOL AUAppGlueApplePdfKit::pageGetText(void* param, void* pageRef, STNBString* dst){
	BOOL r = FALSE;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)pageRef;
	if(obj != NULL && pp != NULL){
		@autoreleasepool {
			NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
			NBThreadMutex_lock(&pp->mutex);
			{
				NSString* lbl = [pp->page string];
				if(lbl != nil){
					if(dst != NULL){
						NBString_set(dst, [lbl UTF8String]);
					}
					r = TRUE;
				}
			}
			NBThreadMutex_unlock(&pp->mutex);
		}
	}
	return r;
}

BOOL AUAppGlueApplePdfKit::pageRenderArea(void* param, void* pageRef, const STNBRect area, const STNBSizeI dstSz, STNBBitmap* dst){
	BOOL r = FALSE;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)pageRef;
	if(obj != NULL && pp != NULL && dst != NULL && area.width > 0 && area.height > 0 && dstSz.width > 0 && dstSz.height > 0){
		@autoreleasepool {
			NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
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
				if(data != NULL && props.size.width == dstSz.width && props.size.height == dstSz.height && props.color == ENNBBitmapColor_RGBA8){
					CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
					CGContextRef dstCtxt = CGBitmapContextCreate(data,
																 props.size.width,
																 props.size.height,
																 8, // bitsPerComponent
																 props.bytesPerLine, // bytesPerRow
																 colorSpace,
																 kCGImageAlphaNoneSkipLast);
					if(dstCtxt != NULL){
						//Apply transforms
						{
							CGContextScaleCTM(dstCtxt, (float)props.size.width / (float)area.width, (float)props.size.height / (float)area.height);
							CGContextTranslateCTM(dstCtxt, -area.x, area.y + area.height - pp->cache.mediaBox.height); //pp->cache.mediaBox.height -
							//PRINTF_INFO("Rendering area.y(%f) mediaBox.height(%f).\n", area.y, pp->cache.mediaBox.height);
						}
						//Render
						{
							//Retain
							{
								NBThreadMutex_lock(&pp->mutex);
								AUAppGlueApplePdfKit_pageRetainLocked_(pp);
								NBThreadMutex_unlock(&pp->mutex);
							}
							//Render (unlocked)
							{
								[pp->page drawWithBox:kPDFDisplayBoxMediaBox toContext:dstCtxt];
							}
							//Release
							{
								NBThreadMutex_lock(&pp->mutex);
								AUAppGlueApplePdfKit_pageReleaseLockedAndUnlock_(pp); //It will unlock
							}
						}
						//Release
						CFRelease(dstCtxt);
						r = TRUE;
					}
					CFRelease(colorSpace);
				}
			}
		}
	}
	return r;
}

//Render job

void* AUAppGlueApplePdfKit::renderStartToMemory(void* param, const STNBPdfRenderDoc* job){
	void* r = NULL;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL && job != NULL){
		@autoreleasepool {
#			if TARGET_OS_IPHONE
			{
				NSMutableData* pdfData	= [NSMutableData data];
				//Render
				//OSX creates a CGContextRef system with Y-axis normal, where (0, 0) is lower-left corner.
				//iOS helpers create a CGContextRef system with Y-axis inverted, where (0, 0) is upper-left corner.
				{
					CGRect bounds			= CGRectMake(0, 0, 612, 792);
					UIGraphicsBeginPDFContextToData(pdfData, bounds, nil);
					CGContextRef context = UIGraphicsGetCurrentContext();
					//
					{
						AUAppGlueApplePdfKitRender* dd = NBMemory_allocType(AUAppGlueApplePdfKitRender);
						NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitRender);
						dd->type		= ENAppGlueApplePdfKitDataType_Render;
						dd->glue		= obj;
						//
						dd->pdfData		= pdfData; [pdfData retain];
						dd->context		= context; CGContextRetain(context);
						dd->iNextPage	= 0;
						//
						NBThreadMutex_init(&dd->mutex);
						dd->retainCount	= 1;
						r = dd;
					}
					UIGraphicsPopContext();
				}
			}
#			else
			{
				NSMutableData* pdfData	= [NSMutableData data];
				//Render
				//OSX creates a CGContextRef system with Y-axis normal, where (0, 0) is lower-left corner.
				//iOS helpers create a CGContextRef system with Y-axis inverted, where (0, 0) is upper-left corner.
				{
					CGRect bounds = CGRectMake(0, 0, 612, 792);
					CGDataConsumerRef consumer = CGDataConsumerCreateWithCFData((CFMutableDataRef)pdfData);
					CGContextRef context = CGPDFContextCreate(consumer, &bounds, nil);
					{
						AUAppGlueApplePdfKitRender* dd = NBMemory_allocType(AUAppGlueApplePdfKitRender);
						NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitRender);
						dd->type		= ENAppGlueApplePdfKitDataType_Render;
						dd->glue		= obj;
						//
						dd->pdfData		= pdfData;	[pdfData retain];
						dd->consumer	= consumer; CGDataConsumerRetain(consumer);
						dd->context		= context; CGContextRetain(context);
						dd->iNextPage	= 0;
						//
						NBThreadMutex_init(&dd->mutex);
						dd->retainCount	= 1;
						r = dd;
					}
				}
			}
#			endif
		}
	}
	return r;
}

ENMngrPdfKitRenderResult AUAppGlueApplePdfKit::renderContinue(void* param, void* jobRef, const STNBPdfRenderDoc* job){
	ENMngrPdfKitRenderResult r = ENMngrPdfKitRenderResult_Error;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL && jobRef != NULL && job != NULL){
		AUAppGlueApplePdfKitRender* rndr = (AUAppGlueApplePdfKitRender*)jobRef;
		NBASSERT(rndr->type == ENAppGlueApplePdfKitDataType_Render)
		if(rndr->type == ENAppGlueApplePdfKitDataType_Render){
			@autoreleasepool {
#				if TARGET_OS_IPHONE
				{
					//OSX creates a CGContextRef system with Y-axis normal, where (0, 0) is lower-left corner.
					//iOS helpers create a CGContextRef system with Y-axis inverted, where (0, 0) is upper-left corner.
					if(rndr->iNextPage >= job->pages.use){
						r = ENMngrPdfKitRenderResult_Ended;
					} else {
						UIGraphicsPushContext(rndr->context);
						//Render
						{
							const STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&job->pages, STNBPdfRenderPage*, rndr->iNextPage);
							CGRect bounds			= CGRectMake(0, 0, page->size.width, page->size.height);
							NSDictionary* pgInfo	= nil;
							//Load sizes from template page
							if(page->sizeTemplate.pageRef != NULL){
								AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
								if(pp != NULL){
									NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
									NBThreadMutex_lock(&pp->mutex);
									if(pp->page != NULL){
										const CGRect bMedia	= [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
										const CGRect bCrop	= [pp->page boundsForBox:kPDFDisplayBoxCropBox];
										const CGRect bBleed	= [pp->page boundsForBox:kPDFDisplayBoxBleedBox];
										const CGRect bTrim	= [pp->page boundsForBox:kPDFDisplayBoxTrimBox];
										const CGRect bArt	= [pp->page boundsForBox:kPDFDisplayBoxArtBox];
										bounds			= bMedia;
										pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
														   [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
														   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
														   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
														   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
														   , nil];
										//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
										//adding the "kCGPDFContextMediaBox" key to the dictionary.
									}
									NBThreadMutex_unlock(&pp->mutex);
								} else {
									void* bmpData = NULL;
									STNBBitmapProps bmpProps;
									NBMemory_setZeroSt(bmpProps, STNBBitmapProps);
									if(NBMngrPdfKit::pageGetDataBitmap(page->sizeTemplate.pageRef, &bmpProps, &bmpData)){
										const CGRect bMedia	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bCrop	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bBleed	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bTrim	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bArt	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										bounds			= bMedia;
										pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
														   [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
														   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
														   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
														   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
														   , nil];
										//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
										//adding the "kCGPDFContextMediaBox" key to the dictionary.
									} else {
										NBASSERT(FALSE) //ERROR, undetermined type of content
									}
								}
							}
							//Build page
							{
								UIGraphicsBeginPDFPageWithInfo(bounds, pgInfo);
								//Itms
								{
									SI32 i; for(i = 0; i < page->itms.use; i++){
										const STNBPdfRenderPageItm* itm = NBArray_itmPtrAtIndex(&page->itms, STNBPdfRenderPageItm, i);
										const float rotRad = (itm->matrix.rotDeg * M_PI / 180.0);
										//Apply transform
										{
											CGContextTranslateCTM(rndr->context, itm->matrix.traslation.x, itm->matrix.traslation.y);
											CGContextRotateCTM(rndr->context, rotRad);
										}
										//Draw
										{
											switch(itm->type){
												case ENNBPdfRenderPageItmType_Text:
													{
														UIColor* color		= [UIColor colorWithRed:(itm->text.fontColor.r / 255.0f) green:(itm->text.fontColor.g / 255.0f) blue:(itm->text.fontColor.b / 255.0f) alpha:(itm->text.fontColor.a / 255.0f)];
														UIFont* font		= [UIFont systemFontOfSize:itm->text.fontSize];
														NSString* str		= [NSString stringWithUTF8String:itm->text.text.str];
														NSDictionary* atts	= [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, color, NSForegroundColorAttributeName, nil];
														NSAttributedString* strAtt = [[NSAttributedString alloc] initWithString:str attributes:atts];
														CGPoint pos			= CGPointMake(0.0f, 0.0f);
														[strAtt drawAtPoint:pos];
														[strAtt release];
														strAtt = nil;
													}
													break;
												case ENNBPdfRenderPageItmType_Image:
													{
														NSData* data = [NSData dataWithBytes:itm->image.data.str length:itm->image.data.length];
														CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
														if(imgDataProvider != nil){
															CGImageRef image = nil;
															switch (itm->image.type) {
																case ENNBPdfRenderPageItmImgType_Png:
																	image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																	break;
																case ENNBPdfRenderPageItmImgType_Jpeg:
																	image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																	break;
																default:
																	NBASSERT(FALSE)
																	break;
															}
															if(image != nil){
																//Render
																CGRect bounds = CGRectMake(itm->image.rect.x, itm->image.rect.y, itm->image.rect.width, itm->image.rect.height);
																CGContextDrawImage(rndr->context, bounds, image);
																CGImageRelease(image);
																image = nil;
															}
															CGDataProviderRelease(imgDataProvider);
															imgDataProvider = nil;
														}
													}
													break;
												case ENNBPdfRenderPageItmType_PdfPage:
													if(itm->pdfPage.pageRef != NULL){
														AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
														if(pp != NULL){
															NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
															NBThreadMutex_lock(&pp->mutex);
															if(pp->page != NULL){
																const CGRect bMedia = [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
																const float sizeH	= (bMedia.size.height - bMedia.origin.y);
																CGContextTranslateCTM(rndr->context, 0.0f, sizeH);
																CGContextScaleCTM(rndr->context, 1.0f, -1.0f);
																{
																	NBThreadMutex_unlock(&pp->mutex);
																	{
																		CGContextDrawPDFPage(rndr->context, [pp->page pageRef]);
																	}
																	NBThreadMutex_lock(&pp->mutex);
																}
																CGContextScaleCTM(rndr->context, 1.0f, -1.0f);
																CGContextTranslateCTM(rndr->context, 0.0f, -sizeH);
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
																	if(!NBFile_openAsString(stream, &imgData)){
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
																					NBASSERT(FALSE)
																				} else {
																					SI32 ciclesCount = 0;
																					ENJpegWriteResult rr = ENJpegWriteResult_error;
																					while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																						ciclesCount++;
																					}
																					if(rr != ENJpegWriteResult_end){
																						NBASSERT(FALSE) //ERROR
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
																			if(!NBPng_saveDataToFile(&bmpProps, bmpData, stream, ENPngCompressLvl_9)){
																				NBASSERT(FALSE)
																			} else {
																				imgType = ENNBPdfRenderPageItmImgType_Png;
																			}
																		}
																		if(imgType == ENNBPdfRenderPageItmImgType_Jpeg || imgType == ENNBPdfRenderPageItmImgType_Png){
																			//Draw image into page
																			NSData* data = [NSData dataWithBytes:imgData.str length:imgData.length];
																			CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
																			if(imgDataProvider != nil){
																				CGImageRef image = nil;
																				switch (imgType) {
																					case ENNBPdfRenderPageItmImgType_Png:
																						image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																						break;
																					case ENNBPdfRenderPageItmImgType_Jpeg:
																						image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																						break;
																					default:
																						NBASSERT(FALSE)
																						break;
																				}
																				if(image != nil){
																					//Render
																					const float scaleW = ((bounds.size.width * 0.75f) / bmpProps.size.width);
																					const float scaleH = ((bounds.size.height * 0.75f) / bmpProps.size.height);
																					float imgScale = (scaleW < scaleH ? scaleW : scaleH);
																					if(imgScale > 1.0f){
																						imgScale = 1.0f;
																					}
																					{
																						CGRect imgBounds = CGRectMake(bounds.origin.x + ((bounds.size.width - (bmpProps.size.width * imgScale)) * 0.5f), bounds.origin.y + ((bounds.size.height - (bmpProps.size.height * imgScale)) * 0.5f) + (bmpProps.size.height * imgScale), bmpProps.size.width * imgScale, -(bmpProps.size.height * imgScale));
																						CGContextDrawImage(rndr->context, imgBounds, image);
																					}
																					CGImageRelease(image);
																					image = nil;
																				}
																				CGDataProviderRelease(imgDataProvider);
																				imgDataProvider = nil;
																			}
																		}
																		/*NBFile_lock(stream);
																		{
																			STNBJpegWrite jWrite;
																			NBJpegWrite_init(&jWrite);
																			if(!NBJpegWrite_feedStart(&jWrite, bmpData, (bmpProps.bytesPerLine * bmpProps.size.height), bmpProps, &stream, 85, 10)){
																				NBASSERT(FALSE) //ERROR
																			} else {
																				SI32 ciclesCount = 0;
																				ENJpegWriteResult rr = ENJpegWriteResult_error;
																				while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																					ciclesCount++;
																				}
																				if(rr != ENJpegWriteResult_end){
																					NBASSERT(FALSE) //ERROR
																				} else {
																					//
																				}
																				NBJpegWrite_feedEnd(&jWrite);
																			}
																			NBJpegWrite_release(&jWrite);
																		}
																		NBFile_unlock(stream);*/
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
										//Reset transform
										{
											CGContextRotateCTM(rndr->context, -rotRad);
											CGContextTranslateCTM(rndr->context, -itm->matrix.traslation.x, -itm->matrix.traslation.y);
										}
									}
								}
							}
						}
						UIGraphicsPopContext();
						//Next page
						rndr->iNextPage++;
						if(rndr->iNextPage >= job->pages.use){
							r = ENMngrPdfKitRenderResult_Ended;
						} else {
							r = ENMngrPdfKitRenderResult_Partial;
						}
					}
				}
#				else
				{
					//OSX creates a CGContextRef system with Y-axis normal, where (0, 0) is lower-left corner.
					//iOS helpers create a CGContextRef system with Y-axis inverted, where (0, 0) is upper-left corner.
					if(rndr->iNextPage >= job->pages.use){
						r = ENMngrPdfKitRenderResult_Ended;
					} else {
						//Render
						{
							const STNBPdfRenderPage* page = NBArray_itmValueAtIndex(&job->pages, STNBPdfRenderPage*, rndr->iNextPage);
							CGRect bounds			= CGRectMake(0, 0, page->size.width, page->size.height);
							NSDictionary* pgInfo	= nil;
							//Load sizes from template page
							if(page->sizeTemplate.pageRef != NULL){
								AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
								if(pp != NULL){
									NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
									NBThreadMutex_lock(&pp->mutex);
									if(pp->page != NULL){
										const CGRect bMedia	= [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
										const CGRect bCrop	= [pp->page boundsForBox:kPDFDisplayBoxCropBox];
										const CGRect bBleed	= [pp->page boundsForBox:kPDFDisplayBoxBleedBox];
										const CGRect bTrim	= [pp->page boundsForBox:kPDFDisplayBoxTrimBox];
										const CGRect bArt	= [pp->page boundsForBox:kPDFDisplayBoxArtBox];
										bounds			= bMedia;
										pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
														   [NSData dataWithBytes:&bMedia length:sizeof(bMedia)], kCGPDFContextMediaBox
														   , [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
														   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
														   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
														   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
														   , nil];
										//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
										//adding the "kCGPDFContextMediaBox" key to the dictionary.
									}
									NBThreadMutex_unlock(&pp->mutex);
								} else {
									void* bmpData = NULL;
									STNBBitmapProps bmpProps;
									NBMemory_setZeroSt(bmpProps, STNBBitmapProps);
									if(NBMngrPdfKit::pageGetDataBitmap(page->sizeTemplate.pageRef, &bmpProps, &bmpData)){
										const CGRect bMedia	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bCrop	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bBleed	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bTrim	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										const CGRect bArt	= CGRectMake(0, 0, (bmpProps.size.width <= bmpProps.size.height ? 612 : 792), (bmpProps.size.width <= bmpProps.size.height ? 792 : 612));;
										bounds			= bMedia;
										pgInfo			= [NSDictionary dictionaryWithObjectsAndKeys:
														   [NSData dataWithBytes:&bMedia length:sizeof(bMedia)], kCGPDFContextMediaBox
														   , [NSData dataWithBytes:&bCrop length:sizeof(bCrop)], kCGPDFContextCropBox
														   , [NSData dataWithBytes:&bBleed length:sizeof(bBleed)], kCGPDFContextBleedBox
														   , [NSData dataWithBytes:&bTrim length:sizeof(bTrim)], kCGPDFContextTrimBox
														   , [NSData dataWithBytes:&bArt length:sizeof(bArt)], kCGPDFContextArtBox
														   , nil];
										//Note: "UIGraphicsBeginPDFPageWithInfo" fails when
										//adding the "kCGPDFContextMediaBox" key to the dictionary.
									} else {
										NBASSERT(FALSE) //ERROR, undetermined type of content
									}
								}
							}
							//Build page
							{
								CGPDFContextBeginPage(rndr->context, (CFDictionaryRef)pgInfo);
								//Invert Y-axis
								{
									CGContextTranslateCTM(rndr->context, 0.0f, bounds.origin.y + bounds.size.height);
									CGContextScaleCTM(rndr->context, 1.0f, -1.0f);
								}
								//Itms
								{
									SI32 i; for(i = 0; i < page->itms.use; i++){
										const STNBPdfRenderPageItm* itm = NBArray_itmPtrAtIndex(&page->itms, STNBPdfRenderPageItm, i);
										const float rotRad = (itm->matrix.rotDeg * M_PI / 180.0);
										//Apply transform
										{
											CGContextTranslateCTM(rndr->context, itm->matrix.traslation.x, itm->matrix.traslation.y);
											CGContextRotateCTM(rndr->context, rotRad);
										}
										//Draw
										{
											switch(itm->type){
												case ENNBPdfRenderPageItmType_Text:
													{
														NSColor* color		= [NSColor colorWithRed:(itm->text.fontColor.r / 255.0f) green:(itm->text.fontColor.g / 255.0f) blue:(itm->text.fontColor.b / 255.0f) alpha:(itm->text.fontColor.a / 255.0f)];
														NSFont* font		= [NSFont systemFontOfSize:itm->text.fontSize];
														NSString* str		= [NSString stringWithUTF8String:itm->text.text.str];
														NSDictionary* atts	= [NSDictionary dictionaryWithObjectsAndKeys:font, NSFontAttributeName, color, NSForegroundColorAttributeName, nil];
														NSAttributedString* strAtt = [[NSAttributedString alloc] initWithString:str attributes:atts];
														CGPoint pos			= CGPointMake(0.0f, 0.0f);
														[strAtt drawAtPoint:pos];
														[strAtt release];
														strAtt = nil;
													}
													break;
												case ENNBPdfRenderPageItmType_Image:
													{
														NSData* data = [NSData dataWithBytes:itm->image.data.str length:itm->image.data.length];
														CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
														if(imgDataProvider != nil){
															CGImageRef image = nil;
															switch (itm->image.type) {
																case ENNBPdfRenderPageItmImgType_Png:
																	image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																	break;
																case ENNBPdfRenderPageItmImgType_Jpeg:
																	image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																	break;
																default:
																	NBASSERT(FALSE)
																	break;
															}
															if(image != nil){
																//Render
																CGRect bounds = CGRectMake(itm->image.rect.x, itm->image.rect.y, itm->image.rect.width, itm->image.rect.height);
																CGContextDrawImage(rndr->context, bounds, image);
																CGImageRelease(image);
																image = nil;
															}
															CGDataProviderRelease(imgDataProvider);
															imgDataProvider = nil;
														}
													}
													break;
												case ENNBPdfRenderPageItmType_PdfPage:
													if(itm->pdfPage.pageRef != NULL){
														AUAppGlueApplePdfKitPage* pp = (AUAppGlueApplePdfKitPage*)NBMngrPdfKit::pageGetDataGluedInner(page->sizeTemplate.pageRef);
														if(pp != NULL){
															NBASSERT(pp->type == ENAppGlueApplePdfKitDataType_Page)
															NBThreadMutex_lock(&pp->mutex);
															if(pp->page != NULL){
																const CGRect bMedia = [pp->page boundsForBox:kPDFDisplayBoxMediaBox];
																const float sizeH	= (bMedia.size.height - bMedia.origin.y);
																CGContextTranslateCTM(rndr->context, 0.0f, sizeH);
																CGContextScaleCTM(rndr->context, 1.0f, -1.0f);
																{
																	NBThreadMutex_unlock(&pp->mutex);
																	{
																		CGContextDrawPDFPage(rndr->context, [pp->page pageRef]);
																	}
																	NBThreadMutex_lock(&pp->mutex);
																}
																CGContextScaleCTM(rndr->context, 1.0f, -1.0f);
																CGContextTranslateCTM(rndr->context, 0.0f, -sizeH);
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
																	if(!NBFile_openAsString(stream, &imgData)){
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
																					NBASSERT(FALSE)
																				} else {
																					SI32 ciclesCount = 0;
																					ENJpegWriteResult rr = ENJpegWriteResult_error;
																					while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																						ciclesCount++;
																					}
																					if(rr != ENJpegWriteResult_end){
																						NBASSERT(FALSE) //ERROR
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
																			if(!NBPng_saveDataToFile(&bmpProps, bmpData, stream, ENPngCompressLvl_9)){
																				NBASSERT(FALSE)
																			} else {
																				imgType = ENNBPdfRenderPageItmImgType_Png;
																			}
																		}
																		if(imgType == ENNBPdfRenderPageItmImgType_Jpeg || imgType == ENNBPdfRenderPageItmImgType_Png){
																			//Draw image into page
																			NSData* data = [NSData dataWithBytes:imgData.str length:imgData.length];
																			CGDataProviderRef imgDataProvider = CGDataProviderCreateWithCFData((CFDataRef)data);
																			if(imgDataProvider != nil){
																				CGImageRef image = nil;
																				switch (imgType) {
																					case ENNBPdfRenderPageItmImgType_Png:
																						image = CGImageCreateWithPNGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																						break;
																					case ENNBPdfRenderPageItmImgType_Jpeg:
																						image = CGImageCreateWithJPEGDataProvider(imgDataProvider, NULL, true, kCGRenderingIntentDefault);
																						break;
																					default:
																						NBASSERT(FALSE)
																						break;
																				}
																				if(image != nil){
																					//Render
																					const float scaleW = ((bounds.size.width * 0.75f) / bmpProps.size.width);
																					const float scaleH = ((bounds.size.height * 0.75f) / bmpProps.size.height);
																					float imgScale = (scaleW < scaleH ? scaleW : scaleH);
																					if(imgScale > 1.0f){
																						imgScale = 1.0f;
																					}
																					{
																						CGRect imgBounds = CGRectMake(bounds.origin.x + ((bounds.size.width - (bmpProps.size.width * imgScale)) * 0.5f), bounds.origin.y + ((bounds.size.height - (bmpProps.size.height * imgScale)) * 0.5f) + (bmpProps.size.height * imgScale), bmpProps.size.width * imgScale, -(bmpProps.size.height * imgScale));
																						CGContextDrawImage(rndr->context, imgBounds, image);
																					}
																					CGImageRelease(image);
																					image = nil;
																				}
																				CGDataProviderRelease(imgDataProvider);
																				imgDataProvider = nil;
																			}
																		}
																		/*NBFile_lock(stream);
																		{
																			STNBJpegWrite jWrite;
																			NBJpegWrite_init(&jWrite);
																			if(!NBJpegWrite_feedStart(&jWrite, bmpData, (bmpProps.bytesPerLine * bmpProps.size.height), bmpProps, stream, 85, 10)){
																				NBASSERT(FALSE) //ERROR
																			} else {
																				SI32 ciclesCount = 0;
																				ENJpegWriteResult rr = ENJpegWriteResult_error;
																				while((rr = NBJpegWrite_feedWrite(&jWrite)) == ENJpegWriteResult_partial){
																					ciclesCount++;
																				}
																				if(rr != ENJpegWriteResult_end){
																					NBASSERT(FALSE) //ERROR
																				} else {
																					//
																				}
																				NBJpegWrite_feedEnd(&jWrite);
																			}
																			NBJpegWrite_release(&jWrite);
																		}
																		NBFile_unlock(stream);*/
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
										//Reset transform
										{
											CGContextRotateCTM(rndr->context, -rotRad);
											CGContextTranslateCTM(rndr->context, -itm->matrix.traslation.x, -itm->matrix.traslation.y);
										}
									}
								}
								CGPDFContextEndPage(rndr->context);
							}
						}
						//Next page
						rndr->iNextPage++;
						if(rndr->iNextPage >= job->pages.use){
							r = ENMngrPdfKitRenderResult_Ended;
						} else {
							r = ENMngrPdfKitRenderResult_Partial;
						}
					}
				}
#				endif
			}
		}
	}
	return r;
}

void AUAppGlueApplePdfKit::renderCancel(void* param, void* jobRef){
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL && jobRef != NULL){
		AUAppGlueApplePdfKitRender* rndr = (AUAppGlueApplePdfKitRender*)jobRef;
		NBASSERT(rndr->type == ENAppGlueApplePdfKitDataType_Render)
		if(rndr->type == ENAppGlueApplePdfKitDataType_Render){
			NBASSERT(rndr->retainCount == 1)
			@autoreleasepool {
#				if TARGET_OS_IPHONE
				{
					UIGraphicsPushContext(rndr->context);
					UIGraphicsEndPDFContext();
				}
#				else
				{
					if(rndr->consumer != NULL) CGDataConsumerRelease(rndr->consumer); rndr->consumer = NULL;
				}
#				endif
				if(rndr->context != NULL){
					CGContextRelease(rndr->context);
					rndr->context = NULL;
				}
				if(rndr->pdfData != nil){
					[rndr->pdfData release];
					rndr->pdfData = nil;
				}
				NBThreadMutex_release(&rndr->mutex);
				NBMemory_free(rndr);
				rndr = NULL;
			}
		}
	}
}

void* AUAppGlueApplePdfKit::renderEndAndOpenDoc(void* param, void* jobRef){
	void* r = NULL;
	AUAppGlueApplePdfKitData* obj = (AUAppGlueApplePdfKitData*)param;
	if(obj != NULL && jobRef != NULL){
		AUAppGlueApplePdfKitRender* rndr = (AUAppGlueApplePdfKitRender*)jobRef;
		NBASSERT(rndr->type == ENAppGlueApplePdfKitDataType_Render)
		if(rndr->type == ENAppGlueApplePdfKitDataType_Render){
			NBASSERT(rndr->retainCount == 1)
			@autoreleasepool {
#				if TARGET_OS_IPHONE
				{
					UIGraphicsPushContext(rndr->context);
					UIGraphicsEndPDFContext();
				}
#				endif
				if(rndr->context != NULL){
					CGContextRelease(rndr->context);
					rndr->context = NULL;
				}
				//Load
				if(rndr->pdfData != nil){
					//iOS11+
					PDFDocument* doc = [[PDFDocument alloc] initWithData:rndr->pdfData];
					if(doc != nil){
						AUAppGlueApplePdfKitDoc* dd = NBMemory_allocType(AUAppGlueApplePdfKitDoc);
						NBMemory_setZeroSt(*dd, AUAppGlueApplePdfKitDoc);
						dd->type		= ENAppGlueApplePdfKitDataType_Doc;
						dd->glue		= obj;
						dd->doc			= doc;
						dd->retainCount	= 1;
						NBString_init(&dd->filename); //if was loaded from file
						NBArray_init(&dd->pages, sizeof(AUAppGlueApplePdfKitPage*), NULL);
						NBThreadMutex_init(&dd->mutex);
						PRINTF_INFO("PdfDoc opened from render job.\n");
						/*{
						 const SI32 rc = (SI32)[dd->doc retainCount];
						 PRINTF_INFO("PdfDoc retain count at start: %d.\n", rc);
						 }*/
						r = dd;
					}
					[rndr->pdfData release];
					rndr->pdfData = nil;
				}
#				if !TARGET_OS_IPHONE
				{
					if(rndr->consumer != NULL) CGDataConsumerRelease(rndr->consumer); rndr->consumer = NULL;
				}
#				endif
				NBThreadMutex_release(&rndr->mutex);
				NBMemory_free(rndr);
				rndr = NULL;
				
			}
		}
	}
	return r;
}
