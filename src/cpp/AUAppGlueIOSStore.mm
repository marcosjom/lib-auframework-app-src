//
//  AUAppGlueIOSStore.mm
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrStore.h"
#import "AUAppGlueIOSStore.h"
#import <StoreKit/StoreKit.h>

//---------
// Delegate
//---------

@interface AUAppStoreDelegate : NSObject<SKRequestDelegate, SKProductsRequestDelegate, SKPaymentTransactionObserver> {
	@private
	NSArray*			_prodsData;
	SKProductsRequest*	_reqInventory;
	NSMutableSet*		_reqInventorySet;
	SKReceiptRefreshRequest* _reqReceipt;
	bool				_reqRestore;
	NSString*			_actionProdId;
}
+ (AUAppStoreDelegate*)sharedInstance;
- (bool) startRefresh:(const STAppStoreProdId*) prodIds count:(const SI32) count;
- (bool) startPurchase:(const char*) prodId;
- (bool) startRestore;
@end

@implementation AUAppStoreDelegate

+ (AUAppStoreDelegate*)sharedInstance {
	static AUAppStoreDelegate* inst = nil;
	if(inst == nil){
		inst = [[AUAppStoreDelegate alloc] init];
	}
	return inst;
}

- (id)init {
	self = [super init];
	if (self) {
		_prodsData		= nil;
		_reqInventory	= nil;
		_reqInventorySet = nil;
		_reqReceipt		= nil;
		_reqRestore		= false;
		_actionProdId	= nil;
		//Add observer
		[[SKPaymentQueue defaultQueue] addTransactionObserver:self];
	}
	return self;
}

- (void)dealloc {
	[super dealloc];
	//
	if(_prodsData != nil) [_prodsData release]; _prodsData = nil;
	if(_reqInventory != nil) [_reqInventory release]; _reqInventory = nil;
	if(_reqInventorySet != nil) [_reqInventorySet release]; _reqInventorySet = nil;
	if(_reqReceipt != nil) [_reqReceipt release]; _reqReceipt = nil;
	if(_actionProdId != nil) [_actionProdId release]; _actionProdId = nil;
}

//

- (bool) startRefresh:(const STAppStoreProdId*) prodIds count:(const SI32) count {
	bool r = false;
	if (![SKPaymentQueue canMakePayments]) {
		PRINTF_ERROR("AUAppStoreDelegate, this device cannot make purchases (startRefresh).\n");
	} else if(_reqInventory != nil){
		PRINTF_ERROR("AUAppStoreDelegate, already executing an inventory request.\n");
	} else if(prodIds != NULL && count > 0){
		_reqInventorySet = [[NSMutableSet alloc] initWithCapacity: count];
		SI32 i;
		for(i = 0; i < count; i++){
			if(!NBString_strIsEmpty(prodIds[i].prodId)){
				[_reqInventorySet addObject:[NSString stringWithUTF8String:prodIds[i].prodId]];
			}
		}
		_reqInventory = [[SKProductsRequest alloc] initWithProductIdentifiers: _reqInventorySet];
		[_reqInventory setDelegate:self];
		[_reqInventory start];
		PRINTF_INFO("AUAppStoreDelegate, products data request started.\n");
		r = true;
	}
	return r;
}

- (bool) startPurchase:(const char*) prodId {
	bool r = false;
	if (![SKPaymentQueue canMakePayments]) {
		PRINTF_ERROR("AUAppStoreDelegate, this device cannot make purchases (startPurchase).\n");
	} else if(_prodsData == nil){
		PRINTF_ERROR("AUAppStoreDelegate, no productData (startPurchase).\n");
	} else if(prodId != NULL){
		if(prodId[0] != '\0'){
			NSString* strPID = [NSString stringWithUTF8String:prodId];
			for (SKProduct* prod in _prodsData) {
				if([prod.productIdentifier isEqualToString:strPID]){
					SKPayment* payment = [SKPayment paymentWithProduct:prod];
					if(_actionProdId != nil){
						[_actionProdId release];
						_actionProdId = nil;
					}
					_actionProdId = [[NSString stringWithString:prod.productIdentifier] retain];
					[[SKPaymentQueue defaultQueue] addPayment:payment];
					PRINTF_INFO("AUAppStoreDelegate, purchase of '%s' started.\n", prodId);
					r = true;
					break;
				}
			}
		}
	}
	return r;
}

-(bool) startReceiptRefresh {
	bool r = false;
	if (![SKPaymentQueue canMakePayments]) {
		PRINTF_ERROR("AUAppStoreDelegate, this device cannot make purchases (startReceiptRefresh).\n");
	} else if(_reqReceipt != nil){
		PRINTF_ERROR("AUAppStoreDelegate, already executing a receipt request.\n");
	} else {
		_reqReceipt = [[SKReceiptRefreshRequest alloc] init];
		[_reqReceipt setDelegate:self];
		[_reqReceipt start];
		PRINTF_INFO("AUAppStoreDelegate, receipt request started.\n");
		r = true;
	}
	return r;
}

- (bool) startRestore {
	bool r = false;
	if (![SKPaymentQueue canMakePayments]) {
		PRINTF_ERROR("AUAppStoreDelegate, this device cannot make purchases (startRestore).\n");
	} else if(_reqRestore){
		PRINTF_ERROR("AUAppStoreDelegate, already executing a restore request.\n");
	} else {
		_reqRestore = true;
		[[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
		PRINTF_INFO("AUAppStoreDelegate, restore started.\n");
		r = true;
	}
	return r;
}

//-----------
// SKRequestDelegate
//-----------

- (void)request:(SKRequest *)request didFailWithError:(NSError *)error {
	PRINTF_INFO("AUAppStoreDelegate, request:didFailWithError.\n");
	if(_reqInventory != nil && request == _reqInventory){
		if(error == nil){
			PRINTF_ERROR("AUAppStoreDelegate, _reqInventory failed: [no error data].\n");
		} else {
			PRINTF_ERROR("AUAppStoreDelegate, _reqInventory failed: '%s'.\n", [error.description UTF8String]);
		}
		if(_reqInventory != nil){
			[_reqInventory setDelegate:nil];
			[_reqInventory release];
			_reqInventory = nil;
		}
		if(_reqInventorySet != nil){
			[_reqInventorySet release];
			_reqInventorySet = nil;
		}
		NBMngrStore::endStoreSync(ENStoreResult_Error);
	} else if(_reqReceipt != nil && request == _reqReceipt){
		if(error == nil){
			PRINTF_ERROR("AUAppStoreDelegate, _reqReceipt failed: [no error data].\n");
		} else {
			PRINTF_ERROR("AUAppStoreDelegate, _reqReceipt failed: '%s'.\n", [error.description UTF8String]);
		}
		if(_reqReceipt != nil){
			[_reqReceipt setDelegate:nil];
			[_reqReceipt release];
			_reqReceipt = nil;
		}
		NBMngrStore::endStoreAction(ENStoreResult_Error);
	}
}

- (void)requestDidFinish:(SKRequest *)request {
	PRINTF_INFO("AUAppStoreDelegate, requestDidFinish.\n");
	if(_reqInventory != nil && request == _reqInventory){
		PRINTF_INFO("AUAppStoreDelegate, _reqInventory success.\n");
		if(_reqInventory != nil){
			[_reqInventory setDelegate:nil];
			[_reqInventory release];
			_reqInventory = nil;
		}
		if(_reqInventorySet != nil){
			[_reqInventorySet release];
			_reqInventorySet = nil;
		}
	} else if(_reqReceipt != nil && request == _reqReceipt){
		PRINTF_INFO("AUAppStoreDelegate, _reqReceipt success.\n");
		if(_reqReceipt != nil){
			[_reqReceipt setDelegate:nil];
			[_reqReceipt release];
			_reqReceipt = nil;
		}
		NBMngrStore::endStoreAction(ENStoreResult_Success);
	}
}

//-----------
// SKProductsRequestDelegate
//-----------

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response {
	if(_reqInventory != nil && request == _reqInventory){
		//PRINTF_INFO("AUAppStoreDelegate, _reqInventory success.\n");
		//invalidProductIdentifiers
		//for(NSString* pid in response.invalidProductIdentifiers) {
			//PRINTF_WARNING("AUAppStoreDelegate, not valid prodId: '%s'.\n", [pid UTF8String]);
		//}
		//products
		for(SKProduct* p in response.products) {
			NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
			[numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
			[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
			[numberFormatter setLocale:p.priceLocale];
			//PRINTF_INFO("AUAppStoreDelegate, valid prodId: '%s'.\n", [p.productIdentifier UTF8String]);
			NBMngrStore::setProdData([p.productIdentifier UTF8String], [p.localizedTitle UTF8String], [p.localizedDescription UTF8String], [[numberFormatter stringFromNumber:p.price] UTF8String]);
			[numberFormatter release];
			numberFormatter = nil;
		}
		//Set array
		if(_prodsData != nil){
			[_prodsData release];
			_prodsData = nil;
		}
		_prodsData = [NSArray arrayWithArray: response.products];
		[_prodsData retain];
		//
		if(_reqInventory != nil){
			[_reqInventory setDelegate:nil];
			[_reqInventory release];
			_reqInventory = nil;
		}
		if(_reqInventorySet != nil){
			[_reqInventorySet release];
			_reqInventorySet = nil;
		}
		NBMngrStore::endStoreSync(ENStoreResult_Success);
	}
}

//

- (void)paymentQueue:(SKPaymentQueue *)queue removedTransactions:(NSArray *)transactions {
	PRINTF_INFO("AUAppStoreDelegate, paymentQueue, %d transactions removed.\n", (int)[transactions count]);
}

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray *)transactions {
	PRINTF_INFO("AUAppStoreDelegate, paymentQueue, %d transactions updated.\n", (int)[transactions count]);
	for(SKPaymentTransaction* t in transactions) {
		switch (t.transactionState) {
			case SKPaymentTransactionStatePurchasing:
				PRINTF_INFO("AUAppStoreDelegate, product '%s' state: purchasing.\n", [t.payment.productIdentifier UTF8String]);
				break;
			case SKPaymentTransactionStatePurchased:
				PRINTF_INFO("AUAppStoreDelegate, product '%s' state: purchased.\n", [t.payment.productIdentifier UTF8String]);
				NBMngrStore::setProdData([t.payment.productIdentifier UTF8String], true);
				if(_actionProdId != nil){
					if([_actionProdId isEqualToString:t.payment.productIdentifier]){
						NBMngrStore::endStoreAction(ENStoreResult_Success);
					}
				}
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				break;
			case SKPaymentTransactionStateRestored:
				PRINTF_INFO("AUAppStoreDelegate, product '%s' state: restored.\n", [t.payment.productIdentifier UTF8String]);
				NBMngrStore::setProdData([t.payment.productIdentifier UTF8String], true);
				if(_actionProdId != nil){
					if([_actionProdId isEqualToString:t.payment.productIdentifier]){
						NBMngrStore::endStoreAction(ENStoreResult_Success);
					}
				}
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				break;
			case SKPaymentTransactionStateFailed:
				PRINTF_ERROR("AUAppStoreDelegate, product '%s' state: failed.\n", [t.payment.productIdentifier UTF8String]);
				if(_actionProdId != nil){
					if([_actionProdId isEqualToString:t.payment.productIdentifier]){
						NBMngrStore::endStoreAction(ENStoreResult_Error);
					}
				}
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				break;
			default:
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				break;
		}
	}
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error {
	if(error == nil){
		PRINTF_ERROR("AUAppStoreDelegate, paymentQueue:restoreCompletedTransactionsFailedWithError: [no error data].\n");
	} else {
		PRINTF_ERROR("AUAppStoreDelegate, paymentQueue:restoreCompletedTransactionsFailedWithError: '%s'.\n", [error.description UTF8String]);
	}
	NBMngrStore::endStoreAction(ENStoreResult_Error);
	_reqRestore = FALSE;
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue *)queue {
	bool somethingRestored = false;
	bool somethingError = false;
	PRINTF_INFO("AUAppStoreDelegate, paymentQueueRestoreCompletedTransactionsFinished with %lu transactions!\n", (unsigned long)queue.transactions.count);
	for(SKPaymentTransaction* t in queue.transactions) {
		switch (t.transactionState) {
			case SKPaymentTransactionStatePurchasing:
				PRINTF_INFO("AUAppStoreDelegate, product '%s' state: purchasing.\n", [t.payment.productIdentifier UTF8String]);
				break;
			case SKPaymentTransactionStatePurchased:
				PRINTF_INFO("AUAppStoreDelegate, product '%s' state: purchased.\n", [t.payment.productIdentifier UTF8String]);
				NBMngrStore::setProdData([t.payment.productIdentifier UTF8String], true);
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				somethingRestored = true;
				break;
			case SKPaymentTransactionStateRestored:
				PRINTF_INFO("AUAppStoreDelegate, product '%s' state: restored.\n", [t.payment.productIdentifier UTF8String]);
				NBMngrStore::setProdData([t.payment.productIdentifier UTF8String], true);
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				somethingRestored = true;
				break;
			case SKPaymentTransactionStateFailed:
				PRINTF_ERROR("AUAppStoreDelegate, product '%s' state: failed.\n", [t.payment.productIdentifier UTF8String]);
				[[SKPaymentQueue defaultQueue] finishTransaction:t];
				somethingError = true;
				break;
			default:
				break;
		}
	}
	if(somethingRestored){
		NBMngrStore::endStoreAction(ENStoreResult_Success);
	} else if(somethingError){
		NBMngrStore::endStoreAction(ENStoreResult_Error);
	} else {
		NBMngrStore::endStoreAction(ENStoreResult_NoChanges);
	}
	_reqRestore = FALSE;
}

@end



//----------------------------
// AUAppGlueIOSStore
//----------------------------

typedef struct AUAppGlueIOSStoreData_ {
	AUAppI* app;
} AUAppGlueIOSStoreData;

//

bool AUAppGlueIOSStore::create(AUAppI* app, STMngrStoreCalls* obj){
	AUAppGlueIOSStoreData* data = (AUAppGlueIOSStoreData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSStoreData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueIOSStoreData);
	NBMemory_setZeroSt(*obj, STMngrStoreCalls);
	data->app = app;
	//
	obj->funcCreate					= create;
	obj->funcCreateParam			= data;
	obj->funcDestroy				= destroy;
	obj->funcDestroyParam			= data;
	//
	obj->funcConcatLocalReceipt		= concatLocalReceiptPayload;
	obj->funcConcatLocalReceiptParam = data;
	obj->funcLoadData				= loadData;
	obj->funcLoadDataParam			= data;
	obj->funcSaveData				= saveData;
	obj->funcSaveDataParam			= data;
	//
	obj->funcStartRefresh			= startRefresh;
	obj->funcStartRefreshParam		= data;
	obj->funcStartPruchase			= startPurchase;
	obj->funcStartPruchaseParam		= data;
	obj->funcStartRestore			= startRestore;
	obj->funcStartRestoreParam		= data;
	obj->funcStartReceiptRefresh	= startReceiptRefresh;
	obj->funcStartReceiptRefreshParam = data;
	//Create delegate instance
	[AUAppStoreDelegate sharedInstance];
	//
	return true;
}

bool AUAppGlueIOSStore::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSStoreData* data = (AUAppGlueIOSStoreData*)pData;
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//Callbacks

BOOL AUAppGlueIOSStore::concatLocalReceiptPayload(void* pData, STNBString* dst){
	BOOL r = FALSE;
	if(pData != NULL){
		@autoreleasepool {
			NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
			if(receiptURL != nil){
				NSData *receipt = [NSData dataWithContentsOfURL:receiptURL];
				if(receipt != nil){
					if(dst != NULL){
						NBString_concatBytes(dst, (const char*)[receipt bytes], (UI32)[receipt length]);
					}
					r = TRUE;
				}
			}
		}
	}
	return r;
}

bool AUAppGlueIOSStore::loadData(void* pData){
	bool r = false;
	/*AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnCache("_storeData.bin"), ENArchivoModo_SoloLectura);
	if(file != NULL){
		AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		AUArchivoCrypt* fileCrypt = new(ENMemoriaTipo_Temporal) AUArchivoCrypt(file, "AUAppGlueIOSStore");
		fileCrypt->lock();
		char buff[4096];
		do {
			const SI32 read = fileCrypt->leer(buff, sizeof(char), 4096, fileCrypt);
			if(read <= 0) break;
			str->agregar(buff, read);
		} while(1);
		//
		if(NBMngrStore::lockedLoadFromJSON(str->str())){
			r = true;
		}
		//
		fileCrypt->unlock();
		fileCrypt->cerrar();
		fileCrypt->liberar(NB_RETENEDOR_THIS);
		file->cerrar();
		str->liberar(NB_RETENEDOR_THIS);
	}*/
	return r;
}

bool AUAppGlueIOSStore::saveData(void* pData){
	bool r = false;
	AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	if(NBMngrStore::lockedSaveToJSON(str)){
		AUArchivo* file = NBGestorArchivos::flujoDeArchivo(ENMemoriaTipo_Temporal, NBGestorArchivos::rutaHaciaRecursoEnCache("_storeData.bin"), ENArchivoModo_SoloEscritura);
		if(file != NULL){
			AUArchivoCrypt* fileCrypt = new(ENMemoriaTipo_Temporal) AUArchivoCrypt(file, "AUAppGlueIOSStore");
			fileCrypt->lock();
			fileCrypt->escribir(str->str(), sizeof(char), str->tamano(), fileCrypt);
			fileCrypt->unlock();
			fileCrypt->cerrar();
			fileCrypt->liberar(NB_RETENEDOR_NULL);
			file->cerrar();
			r = true;
		}
	}
	str->liberar(NB_RETENEDOR_NULL);
	return r;
}

bool AUAppGlueIOSStore::startRefresh(void* pData, const STAppStoreProdId* prodIds, const SI32 prodIdsCount){
	const bool r = [[AUAppStoreDelegate sharedInstance] startRefresh: prodIds count: prodIdsCount];
	return r;
}

bool AUAppGlueIOSStore::startPurchase(void* pData, const char* prodId){
	const bool r = [[AUAppStoreDelegate sharedInstance] startPurchase: prodId];
	return r;
}

bool AUAppGlueIOSStore::startReceiptRefresh(void* data){
	const bool r = [[AUAppStoreDelegate sharedInstance] startReceiptRefresh];
	return r;
}

bool AUAppGlueIOSStore::startRestore(void* pData){
	const bool r = [[AUAppStoreDelegate sharedInstance] startRestore];
	return r;
}





