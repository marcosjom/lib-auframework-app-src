//
//  AUAppGlueAndroidStore.mm
//
//

#include "AUAppNucleoPrecompilado.h"
#include "NBMngrStore.h"
#import "AUAppGlueAndroidStore.h"
//Android and JNI headers
#include <jni.h>

//----------------------------
// AUAppGlueAndroidStore
//----------------------------

#define AU_GLUE_ANDROID_STORE_PURCHASE_FLOW_EXPIRES_AFTER_REGAINING_FOCUS_SECS	5.0f

class AUAppGlueAndroidStoreAppListener;

typedef struct STAppGlueAndroidStoreSkuDet_ {
	char*	sku;
	char*	title;
	char*	desc;
	char*	price;
	jobject	skuDetails;
} STAppGlueAndroidStoreSkuDet;

typedef struct STAppGlueAndroidStorePurchase_ {
	//UniqueId
	struct {
		char*	sku;
		char*	token;
	} uid;
	//Data
	struct {
		BOOL	isSet;		//Values are explicit
		char*	orderId;
		SI32	state;
		BOOL	isAcknowledged;
		BOOL	isAutoRenewing;
	} data;
	//Consume
	struct {
		float	secsAcumWait;
		BOOL	isConsumed;
	} consume;
} STAppGlueAndroidStorePurchase;

void AppGlueAndroidStorePurchase_release(STAppGlueAndroidStorePurchase* obj){
	//UniqueId
	{
		if(obj->uid.sku != NULL) NBMemory_free(obj->uid.sku); obj->uid.sku = NULL;
		if(obj->uid.token != NULL) NBMemory_free(obj->uid.token); obj->uid.token = NULL;
	}
	//Data
	{
		if(obj->data.orderId != NULL) NBMemory_free(obj->data.orderId); obj->data.orderId = NULL;
	}
}
	
typedef struct AUAppGlueAndroidStoreData_ {
	AUAppI* app;
	//Actions
	struct {
		//Refresh action
		struct {
			BOOL				isSet;
			STAppStoreProdId*	skus;
			UI32				skusSz;
			//Requests
			STNBThreadMutex		reqsMutex;
			SI32				reqsRemain;		//Waiting for these requests
			SI32				reqsSuccess;	//Sucess results
			SI32				reqsError;		//Error results
		} refresh;
		//Purchase action
		struct {
			BOOL				isSet;
			BOOL				isLaunchFlow;
			BOOL				isLaunchFlowGainedFocus;	//The first time the focus is lost for the purchaseFlow
			BOOL				isLaunchFlowReturnedFocus;	//The first time the focus is gained after the purchaseFlow
			float				isLaunchFlowReturnedFocusSecsAccum; //Teh ammount of seconds with the focus gained after the purchaseFlow
			char*				sku;
			ENStoreProdType		type;
			char*				grpId;
		} purchase;
		//Restore action
		struct {
			BOOL				isSet;
			//Requests
			STNBThreadMutex		reqsMutex;
			SI32				reqsRemain;		//Waiting for these requests
			SI32				reqsSuccess;	//Sucess results
			SI32				reqsError;		//Error results
		} restore;
		//Post-purchase (acknowledgent or consumption)
		struct {
			//Uses the 'purchases.mutex' lock
			BOOL				isSet;
			char*				sku;
			char*				token;
		} consumption;
	} actions;
	//Purchases (active only)
	struct {
		STNBThreadMutex mutex;
		STNBArray	arr;	//STAppGlueAndroidStorePurchase
		STNBString	tokensJsonStr;
		//Pend actions
		UI32		pendSynced;
		UI32		pendAdded;
	} purchases;
	//New libray
	struct {
		jobject		jClt;
		jobject		jLstnr;
		BOOL		isConnecting;
		BOOL		isConnected;
		STNBArray	skusDetails;	//STAppGlueAndroidStoreSkuDet
	} billingClient;
	//
	AUAppGlueAndroidStoreAppListener* appStateLister;
} AUAppGlueAndroidStoreData;

//Helper methods
void AUAppGlueAndroidStore_skuArrRelease(STAppStoreProdId* strs, const UI32 strsSz);
STAppStoreProdId* AUAppGlueAndroidStore_skuArrClone(UI32* dstStrsSz, const STAppStoreProdId* strs, const UI32 strsSz);

//Actions
void AUAppGlueAndroidStore_tick(AUAppGlueAndroidStoreData* data, const float secs);
void AUAppGlueAndroidStore_doResfreshAction(AUAppGlueAndroidStoreData* data);
void AUAppGlueAndroidStore_doPurchaseAction(AUAppGlueAndroidStoreData* data);
void AUAppGlueAndroidStore_doRestoreAction(AUAppGlueAndroidStoreData* data);
//
void AUAppGlueAndroidStore_buildReceiptOfToken(AUAppGlueAndroidStoreData* data, const BOOL onlyIfNecesary);
void AUAppGlueAndroidStore_doSyncWithPurchasesCache(AUAppGlueAndroidStoreData* data);

class AUAppGlueAndroidStoreAppListener: public AUAppStateListener, public AUAppSrvcConnListener, public AUAppActResultListener, public NBAnimador {
	public:
		AUAppGlueAndroidStoreAppListener(AUAppGlueAndroidStoreData* data){
			_data = data;
			_addedAsAnim = FALSE;
		}
		virtual ~AUAppGlueAndroidStoreAppListener(){
			_data = NULL;
			if(_addedAsAnim){
				NBGestorAnimadores::quitarAnimador(this);
				_addedAsAnim = FALSE;
			}
		}
		//AUAppStateListener
		void appStateOnCreate(AUAppI* app){
			//
		}
		void appStateOnDestroy(AUAppI* app){
			//
		}
		void appStateOnStart(AUAppI* app){
			//Bind service only-here, if the service will also be consumed when the app is at BACKGROUND.
			//AUAppGlueAndroidStore::bindService(_data);
		}
		void appStateOnStop(AUAppI* app){
			//Unbind service only-here, if the service will also be consumed when the app is at BACKGROUND.
			//AUAppGlueAndroidStore::unbindService(_data);
		}
		void appStateOnResume(AUAppI* app){
			//Bind service only-here, if the service will only be consumed when the app is at FOREGROUND.
			if(_data->actions.purchase.isLaunchFlow){
				if(_data->actions.purchase.isLaunchFlowGainedFocus){
					if(!_data->actions.purchase.isLaunchFlowReturnedFocus){
						_data->actions.purchase.isLaunchFlowReturnedFocus = TRUE;
						PRINTF_INFO("AUAppGlueAndroidStoreAppListener, focus returned from purchaseFlow.\n");
					}
				}
			}
			//
			if(!_addedAsAnim){
				NBGestorAnimadores::agregarAnimador(NULL, this);
				_addedAsAnim = TRUE;
			}
			//
			AUAppGlueAndroidStore_doSyncWithPurchasesCache(_data);
		}
		void appStateOnPause(AUAppI* app){
			//Unbind service only-here, if the service will only be consumed when the app is at FOREGROUND.
			if(_data->actions.purchase.isLaunchFlow){
				if(!_data->actions.purchase.isLaunchFlowGainedFocus){
					_data->actions.purchase.isLaunchFlowGainedFocus = TRUE;
					PRINTF_INFO("AUAppGlueAndroidStoreAppListener, focus given to purchaseFlow.\n");
				}
			}
			//
			if(_addedAsAnim){
				NBGestorAnimadores::quitarAnimador(this);
				_addedAsAnim = FALSE;
			}
		}
		//NBAnimador
		void tickAnimacion(float segsTranscurridos){
			AUAppGlueAndroidStore_tick(_data, segsTranscurridos);
		}
		//AUAppSrvcConnListener
		void appSrcvOnConnected(AUAppI* app, void* compName /*jobject::ComponentName*/, void* binder /*jobject::IBinder*/){
			//AUAppGlueAndroidStore::analyzeServiceConnected(_data, compName, binder);
		}
		void appSrcvOnDisconnected(AUAppI* app, void* compName /*jobject::ComponentName*/){
			//AUAppGlueAndroidStore::analyzeServiceDisconnected(_data, compName);
		}
		//AUAppActResultListener
		void appActResultReceived(AUAppI* app, SI32 request, SI32 response, void* data /*jobject::Intent*/){
			//AUAppGlueAndroidStore::analyzeActivityResult(_data, request, response, data);
		}
	private:
		AUAppGlueAndroidStoreData*	_data;
		BOOL						_addedAsAnim;
};

//Callbacks

bool AUAppGlueAndroidStore::create(AUAppI* app, STMngrStoreCalls* obj){
	AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidStoreData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAndroidStoreData);
	NBMemory_setZeroSt(*obj, STMngrStoreCalls);
	data->app		= app;
	//
	NBMemory_setZero(data->actions);
	NBThreadMutex_init(&data->actions.refresh.reqsMutex);
	NBThreadMutex_init(&data->actions.restore.reqsMutex);
	//Purchases
	{
		NBMemory_setZero(data->purchases);
		NBThreadMutex_init(&data->purchases.mutex);
		NBArray_init(&data->purchases.arr, sizeof(STAppGlueAndroidStorePurchase), NULL);
		NBString_init(&data->purchases.tokensJsonStr);
	}
	//BillingClient
	{
		NBMemory_setZero(data->billingClient);
		NBArray_init(&data->billingClient.skusDetails, sizeof(STAppGlueAndroidStoreSkuDet), NULL);
	}
	//NBMemory_setZero(data->IInAppBillingService);
	//
	data->appStateLister = new AUAppGlueAndroidStoreAppListener(data);
	data->app->addAppStateListener(data->appStateLister);
	data->app->addAppSrvcConnListener(data->appStateLister);
	data->app->addAppActivityResultListener(data->appStateLister);
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
	//New billing listenr
	{
		AUAppGlueAndroidJNI* jniGlue = app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		if(jEnv != NULL){
			//Create listener
			{
				jclass clsLstnr				= jEnv->FindClass("com/auframework/AppNative$AUBillingClientStateListener");
				if(clsLstnr == NULL){
					if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
					PRINTF_ERROR("AUAppGlueAndroidStore, class com/auframework/AppNative$AUBillingClientStateListener not found.\n");
				} else {
					jmethodID mInitLstr		= jEnv->GetMethodID(clsLstnr, "<init>", "(J)V"); NBASSERT(mInitLstr != NULL)
					jobject jLstnr 			= jEnv->NewObject(clsLstnr, mInitLstr, (jlong)data); NBASSERT(jLstnr != NULL)
					if(jLstnr != NULL){
						//Set global ref
						{
							if(data->billingClient.jLstnr != NULL){
								jEnv->DeleteGlobalRef(data->billingClient.jLstnr);
								data->billingClient.jLstnr = NULL;
							}
							data->billingClient.jLstnr	= jEnv->NewGlobalRef(jLstnr);
						}
						PRINTF_INFO("AUAppGlueAndroidStore, billingClient.jLstnr created.\n");
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jLstnr)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsLstnr)
			}
			//Build client
			if(data->billingClient.jLstnr != NULL){
				jclass clsClt = jEnv->FindClass("com/android/billingclient/api/BillingClient");
				if(clsClt == NULL){
					if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
					PRINTF_ERROR("AUAppGlueAndroidStore, class com/android/billingclient/api/BillingClient not found.\n");
				} else {
					jclass clsBldr = jEnv->FindClass("com/android/billingclient/api/BillingClient$Builder"); NBASSERT(clsBldr != NULL)
					if(clsBldr != NULL){
						jmethodID mNewBldr		= jEnv->GetStaticMethodID(clsClt, "newBuilder", "(Landroid/content/Context;)Lcom/android/billingclient/api/BillingClient$Builder;"); NBASSERT(mNewBldr != NULL)
						jmethodID mSetLstn		= jEnv->GetMethodID(clsBldr, "setListener", "(Lcom/android/billingclient/api/PurchasesUpdatedListener;)Lcom/android/billingclient/api/BillingClient$Builder;"); NBASSERT(mSetLstn != NULL)
						jmethodID mEnbPendPurch	= jEnv->GetMethodID(clsBldr, "enablePendingPurchases", "()Lcom/android/billingclient/api/BillingClient$Builder;"); NBASSERT(mEnbPendPurch != NULL)
						jmethodID mBuild		= jEnv->GetMethodID(clsBldr, "build", "()Lcom/android/billingclient/api/BillingClient;"); NBASSERT(mBuild != NULL)
						{
							jobject jBldr = jEnv->CallStaticObjectMethod(clsClt, mNewBldr, jContext);
							if(jBldr != NULL){
								//Set listener
								jEnv->CallObjectMethod(jBldr, mSetLstn, data->billingClient.jLstnr);
								jEnv->CallObjectMethod(jBldr, mEnbPendPurch);
								{
									jobject jClt = jEnv->CallObjectMethod(jBldr, mBuild);
									if(jClt != NULL){
										PRINTF_INFO("AUAppGlueAndroidStore, jClt created.\n");
										//Set global ref
										{
											if(data->billingClient.jClt != NULL){
												jEnv->DeleteGlobalRef(data->billingClient.jClt);
												data->billingClient.jClt = NULL;
											}
											data->billingClient.jClt	= jEnv->NewGlobalRef(jClt);
										}
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jClt)
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jBldr)
						}
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, clsBldr)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsClt)
			}
		}
	}
	//
	return true;
}

bool AUAppGlueAndroidStore::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		//
		if(data->appStateLister != NULL){
			data->app->removeAppStateListener(data->appStateLister);
			data->app->removeAppSrvcConnListener(data->appStateLister);
			data->app->removeAppActivityResultListener(data->appStateLister);
			delete data->appStateLister;
			data->appStateLister = NULL;
		}
		//Purchases
		{
			NBThreadMutex_lock(&data->purchases.mutex);
			{
				{
					SI32 i; for(i = 0; i < data->purchases.arr.use; i++){
						STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
						AppGlueAndroidStorePurchase_release(pur);
					}
					NBArray_empty(&data->purchases.arr);
					NBArray_release(&data->purchases.arr);
				}
				NBString_release(&data->purchases.tokensJsonStr);
			}
			NBThreadMutex_unlock(&data->purchases.mutex);
			NBThreadMutex_release(&data->purchases.mutex);
		}
		//Billing client
		{
			{
				SI32 i; for(i = 0; i < data->billingClient.skusDetails.use; i++){
					STAppGlueAndroidStoreSkuDet* skuDet = NBArray_itmPtrAtIndex(&data->billingClient.skusDetails, STAppGlueAndroidStoreSkuDet, i);
					if(skuDet->sku != NULL) NBMemory_free(skuDet->sku); skuDet->sku = NULL;
					if(skuDet->title != NULL) NBMemory_free(skuDet->title); skuDet->title = NULL;
					if(skuDet->desc != NULL) NBMemory_free(skuDet->desc); skuDet->desc = NULL;
					if(skuDet->price != NULL) NBMemory_free(skuDet->price); skuDet->price = NULL;
					if(skuDet->skuDetails != NULL) jEnv->DeleteGlobalRef(skuDet->skuDetails); skuDet->skuDetails = NULL;
				}
				NBArray_empty(&data->billingClient.skusDetails);
				NBArray_release(&data->billingClient.skusDetails);
			}
			//Release 'new billingClient' gloabl refs
			if(jEnv != NULL){
				if(data->billingClient.jLstnr != NULL) jEnv->DeleteGlobalRef(data->billingClient.jLstnr); data->billingClient.jLstnr = NULL;
				if(data->billingClient.jClt != NULL) jEnv->DeleteGlobalRef(data->billingClient.jClt); data->billingClient.jClt = NULL;
				//Actions
				{
					//Refresh
					{
						AUAppGlueAndroidStore_skuArrRelease(data->actions.refresh.skus, data->actions.refresh.skusSz);
						data->actions.refresh.skus		= NULL;
						data->actions.refresh.skusSz	= 0;
						NBThreadMutex_release(&data->actions.refresh.reqsMutex);
					}
					//Purchase
					{
						if(data->actions.purchase.sku != NULL) NBMemory_free(data->actions.purchase.sku); data->actions.purchase.sku = NULL;
					}
					//Restore
					{
						NBThreadMutex_release(&data->actions.restore.reqsMutex);
					}
				}
			}
			//
		}
		//
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

//

void AUAppGlueAndroidStore_tick(AUAppGlueAndroidStoreData* data, const float secs){
	AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
	JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
	//Analyze PurchaseFlow timeout (when the user moves the focus form the app in the middle of a PurchaeFlow)
	if(data->actions.purchase.isLaunchFlow){
		if(data->actions.purchase.isLaunchFlowGainedFocus){
			data->actions.purchase.isLaunchFlowReturnedFocusSecsAccum += secs;
			if(data->actions.purchase.isLaunchFlowReturnedFocusSecsAccum >= AU_GLUE_ANDROID_STORE_PURCHASE_FLOW_EXPIRES_AFTER_REGAINING_FOCUS_SECS){
				PRINTF_ERROR("AUAppGlueAndroidStore, purchase flow expired %f second after regaining focus.\n", data->actions.purchase.isLaunchFlowReturnedFocusSecsAccum);
				NBMngrStore::endStoreAction(ENStoreResult_Error);
				data->actions.purchase.isSet			= FALSE;
				data->actions.purchase.isLaunchFlow	= FALSE;
			}
		}
	}
	//Trigger Acknowledge and Consume actions
	if(data->billingClient.isConnected){
		NBThreadMutex_lock(&data->actions.refresh.reqsMutex);
		{
			if(!data->actions.refresh.isSet){ //Avoid processing while the 'skus' array is renewed
				NBThreadMutex_lock(&data->purchases.mutex);
				{
					UI32 countStarted = 0;
					BOOL actionStarted = FALSE;
					SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
						STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
						//PRINTF_INFO("AUAppGlueAndroidStore, pur #%d/%d sku('%s') token(%d bytes) data.isSet(%d) skusSz(%d).\n", (i + 1), data->purchases.arr.use, pur->uid.sku, NBString_strLenBytes(pur->uid.token), pur->data.isSet, data->actions.refresh.skusSz);
						if(!NBString_strIsEmpty(pur->uid.sku) && !NBString_strIsEmpty(pur->uid.token) && pur->data.isSet){
							//Search definition
							SI32 i; for(i = 0; i < data->actions.refresh.skusSz; i++){
								const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
								if(NBString_strIsEqual(sku->prodId, pur->uid.sku)){
									BOOL isErr = FALSE, isCurrent = FALSE;
									//
									if(data->actions.consumption.isSet){
										if(NBString_strIsEqual(pur->uid.sku, data->actions.consumption.sku) && NBString_strIsEqual(pur->uid.token, data->actions.consumption.token)){
											//PRINTF_INFO("AUAppGlueAndroidStore, pur is current.\n");
											isCurrent = TRUE;
										}
									}
									//Increase timer (to others)
									if(!isCurrent){
										pur->consume.secsAcumWait += secs;
									}
									//Analyze timer state
									//PRINTF_INFO("AUAppGlueAndroidStore, secs(%f  of %f).\n", pur->consume.secsAcumWait, sku->actionsSecsWait);
									if(pur->consume.secsAcumWait >= sku->actionsSecsWait){
										pur->consume.secsAcumWait = 0.0f;
										//PRINTF_INFO("AUAppGlueAndroidStore, pur #%d/%d sku('%s') token(%d bytes) data.isSet(%d) data.isAcknowledged(%d) actionMsk(%d) isConsumed(%d).\n", (i + 1), data->purchases.arr.use, pur->uid.sku, NBString_strLenBytes(pur->uid.token), pur->data.isSet, pur->data.isAcknowledged, sku->actions, pur->consume.isConsumed);
										//Try to Acknowledge
										if(!data->actions.consumption.isSet && !isErr && !actionStarted){
											if(!pur->data.isAcknowledged && (sku->actions & ENStorePurchaseActionBit_Acknowledge) != 0){
												//Do Acknowledge
												jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
												jclass clsParms		= jEnv->FindClass("com/android/billingclient/api/AcknowledgePurchaseParams"); NBASSERT(clsParms != NULL)
												jclass clsParmsBldr	= jEnv->FindClass("com/android/billingclient/api/AcknowledgePurchaseParams$Builder"); NBASSERT(clsParmsBldr != NULL)
												if(clsClt != NULL && clsParms != NULL && clsParmsBldr != NULL){
													jmethodID mNewBldr	= jEnv->GetStaticMethodID(clsParms, "newBuilder", "()Lcom/android/billingclient/api/AcknowledgePurchaseParams$Builder;"); NBASSERT(mNewBldr != NULL)
													jmethodID mSetToken	= jEnv->GetMethodID(clsParmsBldr, "setPurchaseToken", "(Ljava/lang/String;)Lcom/android/billingclient/api/AcknowledgePurchaseParams$Builder;"); NBASSERT(mSetToken != NULL)
													jmethodID mBuild	= jEnv->GetMethodID(clsParmsBldr, "build", "()Lcom/android/billingclient/api/AcknowledgePurchaseParams;"); NBASSERT(mBuild != NULL)
													jmethodID mQueryA	= jEnv->GetMethodID(clsClt, "acknowledgePurchase", "(Lcom/android/billingclient/api/AcknowledgePurchaseParams;Lcom/android/billingclient/api/AcknowledgePurchaseResponseListener;)V"); NBASSERT(mQueryA != NULL)
													if(mNewBldr != NULL && mSetToken != NULL && mBuild != NULL && mQueryA != NULL){
														jobject jBldr = jEnv->CallStaticObjectMethod(clsParms, mNewBldr); NBASSERT(jBldr != NULL)
														if(jBldr != NULL){
															jstring jToken = jEnv->NewStringUTF(pur->uid.token);
															jEnv->CallObjectMethod(jBldr, mSetToken, jToken);
															{
																jobject params = jEnv->CallObjectMethod(jBldr, mBuild);
																if(params != NULL){
																	{
																		if(data->actions.consumption.sku != NULL) NBMemory_free(data->actions.consumption.sku); data->actions.consumption.sku = NULL;
																		if(data->actions.consumption.token != NULL) NBMemory_free(data->actions.consumption.token); data->actions.consumption.token = NULL;
																		if(!NBString_strIsEmpty(pur->uid.sku)) data->actions.consumption.sku = NBString_strNewBuffer(pur->uid.sku);
																		if(!NBString_strIsEmpty(pur->uid.token)) data->actions.consumption.token = NBString_strNewBuffer(pur->uid.token);
																		data->actions.consumption.isSet = TRUE;
																		PRINTF_INFO("AUAppGlueAndroidStore, starting acknowledge for sku('%s').\n", pur->uid.sku);
																	}
																	NBThreadMutex_unlock(&data->purchases.mutex);
																	{ //Call (unlocked)
																		//ToDo: enable
																		jEnv->CallVoidMethod(data->billingClient.jClt, mQueryA, params, data->billingClient.jLstnr);
																		//ToDo: remove
																		//data->actions.consumption.isSet = FALSE;
																		actionStarted = TRUE;
																		countStarted++;
																	}
																	NBThreadMutex_lock(&data->purchases.mutex);
																}
															}
															NBJNI_DELETE_REF_LOCAL(jEnv, jToken);
														}
														NBJNI_DELETE_REF_LOCAL(jEnv, jBldr);
													}
												}
												NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
												NBJNI_DELETE_REF_LOCAL(jEnv, clsParms);
												NBJNI_DELETE_REF_LOCAL(jEnv, clsParmsBldr);
											}
										}
										//Try to consume
										if(!data->actions.consumption.isSet && !isErr && !actionStarted){
											if(!pur->consume.isConsumed && ((sku->actions & ENStorePurchaseActionBit_Consume) != 0 || (pur->data.isAcknowledged && (sku->actions & ENStorePurchaseActionBit_ConsumeIfAcknowledged) != 0))){
												//Do Consumption
												jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
												jclass clsParms		= jEnv->FindClass("com/android/billingclient/api/ConsumeParams"); NBASSERT(clsParms != NULL)
												jclass clsParmsBldr	= jEnv->FindClass("com/android/billingclient/api/ConsumeParams$Builder"); NBASSERT(clsParmsBldr != NULL)
												if(clsClt != NULL && clsParms != NULL && clsParmsBldr != NULL){
													jmethodID mNewBldr	= jEnv->GetStaticMethodID(clsParms, "newBuilder", "()Lcom/android/billingclient/api/ConsumeParams$Builder;"); NBASSERT(mNewBldr != NULL)
													jmethodID mSetToken	= jEnv->GetMethodID(clsParmsBldr, "setPurchaseToken", "(Ljava/lang/String;)Lcom/android/billingclient/api/ConsumeParams$Builder;"); NBASSERT(mSetToken != NULL)
													jmethodID mBuild	= jEnv->GetMethodID(clsParmsBldr, "build", "()Lcom/android/billingclient/api/ConsumeParams;"); NBASSERT(mBuild != NULL)
													jmethodID mQueryA	= jEnv->GetMethodID(clsClt, "consumeAsync", "(Lcom/android/billingclient/api/ConsumeParams;Lcom/android/billingclient/api/ConsumeResponseListener;)V"); NBASSERT(mQueryA != NULL)
													if(mNewBldr != NULL && mSetToken != NULL && mBuild != NULL && mQueryA != NULL){
														jobject jBldr = jEnv->CallStaticObjectMethod(clsParms, mNewBldr); NBASSERT(jBldr != NULL)
														if(jBldr != NULL){
															jstring jToken = jEnv->NewStringUTF(pur->uid.token);
															jEnv->CallObjectMethod(jBldr, mSetToken, jToken);
															{
																jobject params = jEnv->CallObjectMethod(jBldr, mBuild);
																if(params != NULL){
																	{
																		if(data->actions.consumption.sku != NULL) NBMemory_free(data->actions.consumption.sku); data->actions.consumption.sku = NULL;
																		if(data->actions.consumption.token != NULL) NBMemory_free(data->actions.consumption.token); data->actions.consumption.token = NULL;
																		if(!NBString_strIsEmpty(pur->uid.sku)) data->actions.consumption.sku = NBString_strNewBuffer(pur->uid.sku);
																		if(!NBString_strIsEmpty(pur->uid.token)) data->actions.consumption.token = NBString_strNewBuffer(pur->uid.token);
																		data->actions.consumption.isSet = TRUE;
																		PRINTF_INFO("AUAppGlueAndroidStore, starting consumption for sku('%s').\n", pur->uid.sku);
																	}
																	NBThreadMutex_unlock(&data->purchases.mutex);
																	{ //Call (unlocked)
																		//ToDo: enable
																		jEnv->CallVoidMethod(data->billingClient.jClt, mQueryA, params, data->billingClient.jLstnr);
																		//ToDo: remove
																		//data->actions.consumption.isSet = FALSE;
																		actionStarted = TRUE;
																		countStarted++;
																	}
																	NBThreadMutex_lock(&data->purchases.mutex);
																}
															}
															NBJNI_DELETE_REF_LOCAL(jEnv, jToken);
														}
														NBJNI_DELETE_REF_LOCAL(jEnv, jBldr);
													}
												}
												NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
												NBJNI_DELETE_REF_LOCAL(jEnv, clsParms);
												NBJNI_DELETE_REF_LOCAL(jEnv, clsParmsBldr);
											}
										}
									}
									//Next SKU-def
									break;
								}
							}
						}
					}
					if(countStarted > 0){
						PRINTF_INFO("AUAppGlueAndroidStore, tick %d events triggered from %d purchases.\n", countStarted, data->purchases.arr.use);
					}
				}
				NBThreadMutex_unlock(&data->purchases.mutex);
			}
		}
		NBThreadMutex_unlock(&data->actions.refresh.reqsMutex);
	}
}

//

BOOL AUAppGlueAndroidStore::concatLocalReceiptPayload(void* pData, STNBString* dst){
	BOOL r = FALSE;
	AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
	if(data != NULL && dst != NULL){
		NBThreadMutex_lock(&data->purchases.mutex);
		{
			NBString_setBytes(dst, data->purchases.tokensJsonStr.str, data->purchases.tokensJsonStr.length);
			r = TRUE;
		}
		NBThreadMutex_unlock(&data->purchases.mutex);
	}
	return r;
}

//New Billing client

void AUAppGlueAndroidStore::onBillingSetupFinished(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* dataPtr){
	if(dataPtr != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		jint respCode = -1;
		//Determine resultCode
		if(billingResult != NULL){
			jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
			if(clsResult != NULL){
				jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
				if(mGetRespCode != NULL){
					respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
		}
		//Change state
		{
			PRINTF_INFO("AUAppGlueAndroidStore, onBillingSetupFinished respCode(%d).\n", respCode);
			if(respCode == 0 /*BillingClient.BillingResponse.OK*/){
				data->billingClient.isConnecting	= FALSE;
				data->billingClient.isConnected		= TRUE;
			} else {
				data->billingClient.isConnecting	= FALSE;
				data->billingClient.isConnected		= FALSE;
			}
		}
		//Execute actions
		{
			if(data->actions.refresh.isSet){
				AUAppGlueAndroidStore_doResfreshAction(data);
			}
			if(data->actions.purchase.isSet){
				AUAppGlueAndroidStore_doPurchaseAction(data);
			}
			if(data->actions.restore.isSet){
				AUAppGlueAndroidStore_doRestoreAction(data);
			}
		}
	}
}

void AUAppGlueAndroidStore::onBillingServiceDisconnected(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* dataPtr){
	if(dataPtr != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		//Change state
		{
			PRINTF_INFO("AUAppGlueAndroidStore, onBillingServiceDisconnected.\n");
			data->billingClient.isConnecting	= FALSE;
			data->billingClient.isConnected		= FALSE;
		}
		//Execute actions
		//ToDo: set error to current requests?
		/*
		if(data->actions.refresh.isSet){
			AUAppGlueAndroidStore_doResfreshAction(data);
		}
		if(data->actions.purchase.isSet){
			AUAppGlueAndroidStore_doPurchaseAction(data);
		}
		if(data->actions.restore.isSet){
			AUAppGlueAndroidStore_doRestoreAction(data);
		}
		*/
	}
}

//v4
void AUAppGlueAndroidStore::onQueryPurchasesResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* pPurchases /*jobject*/, void* dataPtr){
    if(dataPtr != NULL){
        BOOL purchFound = FALSE;
        JNIEnv* jEnv = (JNIEnv*)pEnv;
        AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
        jint respCode = -1;
        //Determine resultCode
        if(billingResult != NULL){
            jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
            if(clsResult != NULL){
                jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
                if(mGetRespCode != NULL){
                    respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
                }
            }
            NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
        }
        if(respCode == 0 && pPurchases != NULL){ //BillingClient.BillingResponse.OK
            PRINTF_INFO("AUAppGlueAndroidStore, onQueryPurchasesResponse returned.\n");
            jobject purchases    = (jobject)pPurchases;
            jclass clsList        = jEnv->FindClass("java/util/List"); NBASSERT(clsList != NULL)
            jclass clsClt        = jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
            jclass clsPurchase    = jEnv->FindClass("com/android/billingclient/api/Purchase"); NBASSERT(clsPurchase != NULL)
            jclass clsArrList    = jEnv->FindClass("java/util/ArrayList"); NBASSERT(clsArrList != NULL)
            if(clsList != NULL && clsClt != NULL && clsPurchase != NULL && clsArrList != NULL){
                jmethodID mSize        = jEnv->GetMethodID(clsList, "size", "()I"); NBASSERT(mSize != NULL)
                jmethodID mGet         = jEnv->GetMethodID(clsList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mGet != NULL)
                jmethodID mGetSkus     = jEnv->GetMethodID(clsPurchase, "getSkus", "()Ljava/util/ArrayList;"); NBASSERT(mGetSkus != NULL)
                jmethodID mGetOrdId    = jEnv->GetMethodID(clsPurchase, "getOrderId", "()Ljava/lang/String;"); NBASSERT(mGetOrdId != NULL)
                jmethodID mGetToken    = jEnv->GetMethodID(clsPurchase, "getPurchaseToken", "()Ljava/lang/String;"); NBASSERT(mGetToken != NULL)
                jmethodID mGetState    = jEnv->GetMethodID(clsPurchase, "getPurchaseState", "()I"); NBASSERT(mGetState != NULL)
                jmethodID mIsAcknow    = jEnv->GetMethodID(clsPurchase, "isAcknowledged", "()Z"); NBASSERT(mIsAcknow != NULL)
                jmethodID mIsRenewg    = jEnv->GetMethodID(clsPurchase, "isAutoRenewing", "()Z"); NBASSERT(mIsRenewg != NULL)
                jmethodID mArrGet      = jEnv->GetMethodID(clsArrList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mArrGet != NULL)
                jmethodID mArrSize     = jEnv->GetMethodID(clsArrList, "size", "()I"); NBASSERT(mArrSize != NULL)
                //
                if(mSize != NULL && mGet != NULL && mGetSkus != NULL && mGetOrdId != NULL && mGetToken != NULL && mGetState != NULL && mIsAcknow != NULL && mIsRenewg != NULL && mArrGet != NULL && mArrSize != NULL){
                    jint size = jEnv->CallIntMethod(purchases, mSize);
                    PRINTF_INFO("AUAppGlueAndroidStore, %d purchases returned.\n", size);
                    jint i; for(i = 0; i < size; i++){
                        jobject purch = jEnv->CallObjectMethod(purchases, mGet, i);
                        if(purch == NULL){
                            PRINTF_INFO("AUAppGlueAndroidStore, #%d purchase empty.\n", (i + 1));
                        } else {
                            BOOL isCurrentPurchase = FALSE;
                            const char* strOrderId = NULL;
                            const char* strToken = NULL;
                            jstring jOrderId    = (jstring)jEnv->CallObjectMethod(purch, mGetOrdId);
                            jstring jToken        = (jstring)jEnv->CallObjectMethod(purch, mGetToken);
                            const jint pState        = jEnv->CallIntMethod(purch, mGetState);
                            const jboolean pIsAcknw    = jEnv->CallBooleanMethod(purch, mIsAcknow);
                            const jboolean pIsRenew    = jEnv->CallBooleanMethod(purch, mIsRenewg);
                            if(jOrderId != NULL)    strOrderId = jEnv->GetStringUTFChars(jOrderId, 0);
                            if(jToken != NULL)        strToken = jEnv->GetStringUTFChars(jToken, 0);
                            //Process
                            {
                                PRINTF_INFO("AUAppGlueAndroidStore, onQueryPurchasesResponse #%d/%d: sku(one-or-more) orderId('%s') token('%s') state(%d, %s) isAcknowledged(%s) isAutoRenewing(%s).\n", (i + 1), size, strOrderId, strToken, pState, (pState == 0 ? "UNSPECIFIED" : pState == 1 ? "PURCHASED" : pState == 2 ? "PENDING" : "UNKNOWN_CODE"), (mIsAcknow ? "yes" : "no"), (pIsRenew ? "yes" : "no"));
                                if(pState == 1){ //PURCHASED
                                    //Update or add token
                                    NBThreadMutex_lock(&data->purchases.mutex);
                                    {
                                        jobject jSkus = jEnv->CallObjectMethod(purch, mGetSkus);
                                        if(jSkus != NULL){
                                            jint skusSz = jEnv->CallIntMethod(jSkus, mArrSize);
                                            if(skusSz > 0){
                                                jint i; for(i = 0; i < skusSz; i++){
                                                    jstring jSku = (jstring)jEnv->CallObjectMethod(jSkus, mArrGet, i);
                                                    if(jSku != NULL){
                                                        const char* strSku = jEnv->GetStringUTFChars(jSku, 0);
                                                        if(!NBString_strIsEmpty(strSku)){
                                                            BOOL fnd = FALSE;
                                                            //Seach in current values
                                                            {
                                                                SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
                                                                    STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
                                                                    if(NBString_strIsEqual(pur->uid.sku, strSku) && NBString_strIsEqual(pur->uid.token, strToken)){
                                                                        //Update data values
                                                                        {
                                                                            pur->data.isSet            = TRUE;
                                                                            {
                                                                                if(pur->data.orderId != NULL) NBMemory_free(pur->data.orderId); pur->data.orderId = NULL;
                                                                                if(!NBString_strIsEmpty(strOrderId)) pur->data.orderId = NBString_strNewBuffer(strOrderId);
                                                                            }
                                                                            pur->data.state             = pState;
                                                                            pur->data.isAcknowledged    = pIsAcknw;
                                                                            pur->data.isAutoRenewing    = pIsRenew;
                                                                        }
                                                                        //
                                                                        data->purchases.pendSynced++;
                                                                        PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache token/sku('%s') already found.\n", strSku);
                                                                        fnd = TRUE;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                            //Add new value
                                                            if(!fnd){
                                                                STAppGlueAndroidStorePurchase purN;
                                                                NBMemory_setZeroSt(purN, STAppGlueAndroidStorePurchase);
                                                                //UniqueId
                                                                {
                                                                    purN.uid.sku          = NBString_strNewBuffer(strSku);
                                                                    purN.uid.token        = NBString_strNewBuffer(strToken);
                                                                }
                                                                //Data
                                                                {
                                                                    purN.data.isSet            = TRUE;
                                                                    if(!NBString_strIsEmpty(strOrderId)){
                                                                        purN.data.orderId      = NBString_strNewBuffer(strOrderId);
                                                                    }
                                                                    purN.data.state             = pState;
                                                                    purN.data.isAcknowledged    = pIsAcknw;
                                                                    purN.data.isAutoRenewing    = pIsRenew;
                                                                }
                                                                NBArray_addValue(&data->purchases.arr, purN);
                                                                //
                                                                data->purchases.pendAdded++;
                                                                PRINTF_INFO("AUAppGlueAndroidStore, onQueryPurchasesResponse token/sku('%s') added.\n", strSku);
                                                            }
                                                        }
                                                        if(strSku != NULL){
                                                            jEnv->ReleaseStringUTFChars(jSku, strSku);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    NBThreadMutex_unlock(&data->purchases.mutex);
                                }
                            }
                            if(jOrderId != NULL && strOrderId != NULL) jEnv->ReleaseStringUTFChars(jOrderId, strOrderId);
                            if(jToken != NULL && strToken != NULL) jEnv->ReleaseStringUTFChars(jToken, strToken);
                        }
                    }
                    //Build new tokens list
                    {
                        AUAppGlueAndroidStore_buildReceiptOfToken(data, TRUE);
                    }
                }
            }
            NBJNI_DELETE_REF_LOCAL(jEnv, clsPurchase)
            NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
            NBJNI_DELETE_REF_LOCAL(jEnv, clsList)
            NBJNI_DELETE_REF_LOCAL(jEnv, clsArrList)
        } else if(respCode == 1){ //USER_CANCELED
            PRINTF_INFO("AUAppGlueAndroidStore, onQueryPurchasesResponse, flow canceled by user.\n");
        } else {
            PRINTF_ERROR("AUAppGlueAndroidStore, onQueryPurchasesResponse, flow returned code(%d).\n", respCode);
        }
    }
}

//-------------
//- Warning! All purchases reported here must either be consumed or acknowledged.
//- Failure to either consume (via consumeAsync(ConsumeParams, ConsumeResponseListener))
//- or acknowledge (via acknowledgePurchase(AcknowledgePurchaseParams, AcknowledgePurchaseResponseListener))
//- a purchase will result in that purchase being refunded.
//-------------
void AUAppGlueAndroidStore::onPurchasesUpdated(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* pPurchases /*jobject*/, void* dataPtr){
	if(dataPtr != NULL){
		BOOL purchFound = FALSE;
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		jint respCode = -1;
		//Determine resultCode
		if(billingResult != NULL){
			jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
			if(clsResult != NULL){
				jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
				if(mGetRespCode != NULL){
					respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
		}
		if(respCode == 0 && pPurchases != NULL){ //BillingClient.BillingResponse.OK
			PRINTF_INFO("AUAppGlueAndroidStore, onPurchasesUpdated returned.\n");
			jobject purchases	= (jobject)pPurchases;
			jclass clsList		= jEnv->FindClass("java/util/List"); NBASSERT(clsList != NULL)
			jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
			jclass clsPurchase	= jEnv->FindClass("com/android/billingclient/api/Purchase"); NBASSERT(clsPurchase != NULL)
            jclass clsArrList   = jEnv->FindClass("java/util/ArrayList"); NBASSERT(clsArrList != NULL)
			if(clsList != NULL && clsClt != NULL && clsPurchase != NULL && clsArrList != NULL){
				jmethodID mSize		= jEnv->GetMethodID(clsList, "size", "()I"); NBASSERT(mSize != NULL)
				jmethodID mGet		= jEnv->GetMethodID(clsList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mGet != NULL)
				jmethodID mGetSkus	= jEnv->GetMethodID(clsPurchase, "getSkus", "()Ljava/util/ArrayList;"); NBASSERT(mGetSkus != NULL)
				jmethodID mGetOrdId	= jEnv->GetMethodID(clsPurchase, "getOrderId", "()Ljava/lang/String;"); NBASSERT(mGetOrdId != NULL)
				jmethodID mGetToken	= jEnv->GetMethodID(clsPurchase, "getPurchaseToken", "()Ljava/lang/String;"); NBASSERT(mGetToken != NULL)
				jmethodID mGetState	= jEnv->GetMethodID(clsPurchase, "getPurchaseState", "()I"); NBASSERT(mGetState != NULL)
				jmethodID mIsAcknow	= jEnv->GetMethodID(clsPurchase, "isAcknowledged", "()Z"); NBASSERT(mIsAcknow != NULL)
				jmethodID mIsRenewg	= jEnv->GetMethodID(clsPurchase, "isAutoRenewing", "()Z"); NBASSERT(mIsRenewg != NULL)
                jmethodID mArrGet   = jEnv->GetMethodID(clsArrList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mArrGet != NULL)
                jmethodID mArrSize  = jEnv->GetMethodID(clsArrList, "size", "()I"); NBASSERT(mArrSize != NULL)
				//
				if(mSize != NULL && mGet != NULL && mGetSkus != NULL && mGetOrdId != NULL && mGetToken != NULL && mGetState != NULL && mIsAcknow != NULL && mIsRenewg != NULL && mArrGet != NULL && mArrSize != NULL){
					jint size = jEnv->CallIntMethod(purchases, mSize);
					PRINTF_INFO("AUAppGlueAndroidStore, %d purchases returned.\n", size);
					jint i; for(i = 0; i < size; i++){
						jobject purch = jEnv->CallObjectMethod(purchases, mGet, i);
						if(purch == NULL){
							PRINTF_INFO("AUAppGlueAndroidStore, #%d purchase empty.\n", (i + 1));
						} else {
							BOOL isCurrentPurchase = FALSE;
							const char* strOrderId = NULL;
							const char* strToken = NULL;
							jstring jOrderId	= (jstring)jEnv->CallObjectMethod(purch, mGetOrdId);
							jstring jToken		= (jstring)jEnv->CallObjectMethod(purch, mGetToken);
							const jint pState		= jEnv->CallIntMethod(purch, mGetState);
							const jboolean pIsAcknw	= jEnv->CallBooleanMethod(purch, mIsAcknow);
							const jboolean pIsRenew	= jEnv->CallBooleanMethod(purch, mIsRenewg);
							if(jOrderId != NULL)	strOrderId = jEnv->GetStringUTFChars(jOrderId, 0);
							if(jToken != NULL)		strToken = jEnv->GetStringUTFChars(jToken, 0);
							//
							if(data->actions.purchase.isSet){
								if(!NBString_strIsEmpty(data->actions.purchase.sku)){
                                    jobject jSkus = jEnv->CallObjectMethod(purch, mGetSkus);
                                    if(jSkus != NULL){
                                        jint skusSz = jEnv->CallIntMethod(jSkus, mArrSize);
                                        if(skusSz > 0){
                                            jint i; for(i = 0; i < skusSz; i++){
                                                jstring jSku = (jstring)jEnv->CallObjectMethod(jSkus, mArrGet, i);
                                                if(jSku != NULL){
                                                    const char* strSku = jEnv->GetStringUTFChars(jSku, 0);
                                                    if(!NBString_strIsEmpty(strSku)){
                                                        if(NBString_strIsEqual(data->actions.purchase.sku, strSku)){
                                                            purchFound = TRUE;
                                                            isCurrentPurchase = TRUE;
                                                        }
                                                    }
                                                    if(strSku != NULL){
                                                        jEnv->ReleaseStringUTFChars(jSku, strSku);
                                                    }
                                                }
                                            }
                                        }
                                    }
								}
							}
							//Process
							{
								PRINTF_INFO("AUAppGlueAndroidStore, onPurchasesUpdated #%d/%d: sku(one-or-more) orderId('%s') token('%s') state(%d, %s) isAcknowledged(%s) isAutoRenewing(%s).\n", (i + 1), size, strOrderId, strToken, pState, (pState == 0 ? "UNSPECIFIED" : pState == 1 ? "PURCHASED" : pState == 2 ? "PENDING" : "UNKNOWN_CODE"), (mIsAcknow ? "yes" : "no"), (pIsRenew ? "yes" : "no"));
								if(pState == 1){ //PURCHASED
									//Update or add token
									{
										NBThreadMutex_lock(&data->purchases.mutex);
										{
                                            jobject jSkus       = jEnv->CallObjectMethod(purch, mGetSkus);
                                            if(jSkus != NULL){
                                                jint skusSz = jEnv->CallIntMethod(jSkus, mArrSize);
                                                if(skusSz > 0){
                                                    jint i; for(i = 0; i < skusSz; i++){
                                                        jstring jSku = (jstring)jEnv->CallObjectMethod(jSkus, mArrGet, i);
                                                        if(jSku != NULL){
                                                            const char* strSku = jEnv->GetStringUTFChars(jSku, 0);
                                                            if(!NBString_strIsEmpty(strSku)){
                                                                BOOL fnd = FALSE;
                                                                //Seach in current values
                                                                {
                                                                    SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
                                                                        STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
                                                                        if(NBString_strIsEqual(pur->uid.sku, strSku) && NBString_strIsEqual(pur->uid.token, strToken)){
                                                                            //Update Data values
                                                                            {
                                                                                pur->data.isSet                = TRUE;
                                                                                {
                                                                                    if(pur->data.orderId != NULL) NBMemory_free(pur->data.orderId); pur->data.orderId = NULL;
                                                                                    if(!NBString_strIsEmpty(strOrderId)) pur->data.orderId = NBString_strNewBuffer(strOrderId);
                                                                                }
                                                                                pur->data.state                = pState;
                                                                                pur->data.isAcknowledged    = pIsAcknw;
                                                                                pur->data.isAutoRenewing    = pIsRenew;
                                                                            }
                                                                            //
                                                                            data->purchases.pendSynced++;
                                                                            PRINTF_INFO("AUAppGlueAndroidStore, onPurchasesUpdated token/sku('%s') updated.\n", strSku);
                                                                            fnd = TRUE;
                                                                            break;
                                                                        }
                                                                    }
                                                                }
                                                                //Add new value
                                                                if(!fnd){
                                                                    STAppGlueAndroidStorePurchase purN;
                                                                    NBMemory_setZeroSt(purN, STAppGlueAndroidStorePurchase);
                                                                    //UniqueId
                                                                    {
                                                                        purN.uid.sku        = NBString_strNewBuffer(strSku);
                                                                        purN.uid.token        = NBString_strNewBuffer(strToken);
                                                                    }
                                                                    //Data
                                                                    {
                                                                        purN.data.isSet            = TRUE;
                                                                        if(!NBString_strIsEmpty(strOrderId)){
                                                                            purN.data.orderId    = NBString_strNewBuffer(strOrderId);
                                                                        }
                                                                        purN.data.state                = pState;
                                                                        purN.data.isAcknowledged    = pIsAcknw;
                                                                        purN.data.isAutoRenewing    = pIsRenew;
                                                                    }
                                                                    NBArray_addValue(&data->purchases.arr, purN);
                                                                    //
                                                                    data->purchases.pendAdded++;
                                                                    PRINTF_INFO("AUAppGlueAndroidStore, onPurchasesUpdated token/sku('%s') added.\n", strSku);
                                                                }
                                                            }
                                                            if(strSku != NULL){
                                                                jEnv->ReleaseStringUTFChars(jSku, strSku);
                                                            }
                                                        }
                                                    }
                                                }
                                            }
										}
										NBThreadMutex_unlock(&data->purchases.mutex);
									}
								}
							}
							if(jOrderId != NULL && strOrderId != NULL) jEnv->ReleaseStringUTFChars(jOrderId, strOrderId);
							if(jToken != NULL && strToken != NULL) jEnv->ReleaseStringUTFChars(jToken, strToken);
						}
					}
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPurchase)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
			NBJNI_DELETE_REF_LOCAL(jEnv, clsList)
            NBJNI_DELETE_REF_LOCAL(jEnv, clsArrList)
		} else if(respCode == 1){ //USER_CANCELED
			PRINTF_INFO("AUAppGlueAndroidStore, onPurchasesUpdated, flow canceled by user.\n");
		} else {
			PRINTF_ERROR("AUAppGlueAndroidStore, onPurchasesUpdated, flow returned code(%d).\n", respCode);
		}
		//
		if(data->actions.purchase.isSet && data->actions.purchase.isLaunchFlow){
			if(!purchFound){
				PRINTF_ERROR("AUAppGlueAndroidStore, purchase flow ended at 'onPurchasesUpdated' (not found).\n");
				NBMngrStore::endStoreAction(ENStoreResult_Error);
				data->actions.purchase.isSet		= FALSE;
				data->actions.purchase.isLaunchFlow	= FALSE;
			} else {
				PRINTF_INFO("AUAppGlueAndroidStore, purchase flow ended at 'onPurchasesUpdated'.\n");
				NBMngrStore::endStoreAction(ENStoreResult_Success);
				data->actions.purchase.isSet		= FALSE;
				data->actions.purchase.isLaunchFlow	= FALSE;
			}
		}
		//Build receipt (if necesary)
		{
			AUAppGlueAndroidStore_buildReceiptOfToken(data, TRUE);
		}
	}
}

void AUAppGlueAndroidStore::onPurchaseHistoryResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* pPurchaseHistoryRecordList /*jobject*/, void* dataPtr){
	if(dataPtr != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		jint respCode = -1; SI32 reqsRemain = 0, reqsError = 0, reqsSuccess = 0;
		//Determine resultCode
		if(billingResult != NULL){
			jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
			if(clsResult != NULL){
				jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
				if(mGetRespCode != NULL){
					respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
		}
		if(respCode == 0 && pPurchaseHistoryRecordList != NULL){ //BillingClient.BillingResponse.OK
			PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse returned.\n");
			jobject purchases	= (jobject)pPurchaseHistoryRecordList;
			jclass clsList		= jEnv->FindClass("java/util/List"); NBASSERT(clsList != NULL)
			jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
			jclass clsPurchase	= jEnv->FindClass("com/android/billingclient/api/PurchaseHistoryRecord"); NBASSERT(clsPurchase != NULL)
            jclass clsArrList   = jEnv->FindClass("java/util/ArrayList"); NBASSERT(clsArrList != NULL)
			if(clsList != NULL && clsClt != NULL && clsPurchase != NULL && clsArrList != NULL){
				jmethodID mSize		= jEnv->GetMethodID(clsList, "size", "()I"); NBASSERT(mSize != NULL)
				jmethodID mGet		= jEnv->GetMethodID(clsList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mGet != NULL)
				jmethodID mGetSkus	= jEnv->GetMethodID(clsPurchase, "getSkus", "()Ljava/util/ArrayList;"); NBASSERT(mGetSkus != NULL)
				jmethodID mGetToken	= jEnv->GetMethodID(clsPurchase, "getPurchaseToken", "()Ljava/lang/String;"); NBASSERT(mGetToken != NULL)
                jmethodID mArrGet   = jEnv->GetMethodID(clsArrList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mArrGet != NULL)
                 jmethodID mArrSize = jEnv->GetMethodID(clsArrList, "size", "()I"); NBASSERT(mArrSize != NULL)
				//
				if(mSize != NULL && mGet != NULL && mGetSkus != NULL && mGetToken != NULL && mArrGet != NULL && mArrSize != NULL){
					jint size = jEnv->CallIntMethod(purchases, mSize);
					PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse %d purchases returned.\n", size);
					jint i; for(i = 0; i < size; i++){
						jobject purch = jEnv->CallObjectMethod(purchases, mGet, i);
						if(purch == NULL){
							PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse #%d purchase empty.\n", (i + 1));
						} else {
							const char* strToken = NULL;
                            jstring jToken		= (jstring)jEnv->CallObjectMethod(purch, mGetToken);
                            jobject jSkus       = jEnv->CallObjectMethod(purch, mGetSkus);
							if(jToken != NULL)	strToken = jEnv->GetStringUTFChars(jToken, 0);
                            if(jSkus != NULL){
                                jint skusSz = jEnv->CallIntMethod(jSkus, mArrSize);
                                if(skusSz > 0){
                                    jint i; for(i = 0; i < skusSz; i++){
                                        jstring jSku = (jstring)jEnv->CallObjectMethod(jSkus, mArrGet, i);
                                        if(jSku != NULL){
                                            const char* strSku = jEnv->GetStringUTFChars(jSku, 0);
                                            if(!NBString_strIsEmpty(strSku)){
                                                //Update or add token
                                                NBThreadMutex_lock(&data->purchases.mutex);
                                                {
                                                    BOOL fnd = FALSE;
                                                    //Seach in current values
                                                    {
                                                        SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
                                                            STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
                                                            if(NBString_strIsEqual(pur->uid.sku, strSku) && NBString_strIsEqual(pur->uid.token, strToken)){
                                                                PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse token already found.\n");
                                                                fnd = TRUE;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                    //Add new value
                                                    if(!fnd){
                                                        STAppGlueAndroidStorePurchase purN;
                                                        NBMemory_setZeroSt(purN, STAppGlueAndroidStorePurchase);
                                                        //UniqueId
                                                        {
                                                            purN.uid.sku    = NBString_strNewBuffer(strSku);
                                                            purN.uid.token  = NBString_strNewBuffer(strToken);
                                                        }
                                                        NBArray_addValue(&data->purchases.arr, purN);
                                                        //
                                                        data->purchases.pendAdded++;
                                                        PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse token added.\n");
                                                    }
                                                }
                                                PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse #%d/%d: sku('%s') token('%s').\n", (i + 1), size, strSku, strToken);
                                                NBThreadMutex_unlock(&data->purchases.mutex);
                                            }
                                            if(strSku != NULL){
                                                jEnv->ReleaseStringUTFChars(jSku, strSku);
                                            }
                                        }
                                    }
                                }
                            }
                            if(jToken != NULL && strToken != NULL) jEnv->ReleaseStringUTFChars(jToken, strToken);
						}
					}
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPurchase)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
			NBJNI_DELETE_REF_LOCAL(jEnv, clsList)
            NBJNI_DELETE_REF_LOCAL(jEnv, clsArrList)
		} else if(respCode == 1){ //USER_CANCELED
			PRINTF_INFO("AUAppGlueAndroidStore, onPurchaseHistoryResponse, flow canceled by user.\n");
		} else {
			PRINTF_ERROR("AUAppGlueAndroidStore, onPurchaseHistoryResponse, flow returned code(%d).\n", respCode);
		}
		//Consume request end
		{
			NBThreadMutex_lock(&data->actions.restore.reqsMutex);
			{
				PRINTF_INFO("AUAppGlueAndroidStore, onSkuDetailsResponse respCode(%d) (%d remains).\n", respCode, (data->actions.restore.reqsRemain - 1));
				//Apply
				{
					data->actions.restore.reqsRemain--;
					if(respCode == 0 /*BillingClient.BillingResponse.OK*/){
						data->actions.restore.reqsSuccess++;
					} else {
						data->actions.restore.reqsError++;
					}
				}
				//Copy
				{
					reqsRemain	= data->actions.restore.reqsRemain;
					reqsError	= data->actions.restore.reqsError;
					reqsSuccess	= data->actions.restore.reqsSuccess;
				}
			}
			NBThreadMutex_unlock(&data->actions.restore.reqsMutex);
		}
		//Process final
		if(data->actions.restore.isSet){
			if(reqsRemain == 0){
				PRINTF_INFO("AUAppGlueAndroidStore, purchase flow ended at 'onPurchaseHistoryResponse' (last request eneded).\n");
				NBMngrStore::endStoreAction(reqsSuccess > 0 ? ENStoreResult_Success : ENStoreResult_Error);
				data->actions.restore.isSet = FALSE;
			}
		}
		//Refresh receipt of token
		{
			AUAppGlueAndroidStore_buildReceiptOfToken(data, TRUE);
		}
	}
}

void AUAppGlueAndroidStore::onAcknowledgePurchaseResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* dataPtr){
	if(dataPtr != NULL){
		BOOL fnd = FALSE;
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		jint respCode = -1;
		//Determine resultCode
		if(billingResult != NULL){
			jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
			if(clsResult != NULL){
				jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
				if(mGetRespCode != NULL){
					respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
		}
		//Process
		{
			NBThreadMutex_lock(&data->purchases.mutex);
			if(data->actions.consumption.isSet){
				BOOL actionStarted = FALSE;
				SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
					STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
					if(NBString_strIsEqual(pur->uid.sku, data->actions.consumption.sku) && NBString_strIsEqual(pur->uid.token, data->actions.consumption.token)){
						if(respCode == 0){
							PRINTF_INFO("AUAppGlueAndroidStore, onAcknowledgePurchaseResponse success for sku('%s').\n", data->actions.consumption.sku);
							pur->data.isAcknowledged = TRUE;
						} else if(respCode == 8){ //ITEM_NOT_OWNED
							PRINTF_INFO("AUAppGlueAndroidStore, onAcknowledgePurchaseResponse ITEM_NOT_OWNED for sku('%s').\n", data->actions.consumption.sku);
							pur->data.isAcknowledged = TRUE;
						} else {
							PRINTF_INFO("AUAppGlueAndroidStore, onAcknowledgePurchaseResponse code(%d) for sku('%s').\n", respCode, data->actions.consumption.sku);
						}
						data->actions.consumption.isSet = FALSE;
						fnd = TRUE;
						break;
					}
				}
			}
			NBThreadMutex_unlock(&data->purchases.mutex);
		}
		//Eval
		if(!fnd){
			if(respCode == 0){
				PRINTF_INFO("AUAppGlueAndroidStore, onAcknowledgePurchaseResponse success (not processed).\n");
			} else {
				PRINTF_ERROR("AUAppGlueAndroidStore, onAcknowledgePurchaseResponse returned code(%d) (not processed).\n", respCode);
			}
		}
	}
}

void AUAppGlueAndroidStore::onConsumeResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* purchaseToken /*jstring*/, void* dataPtr){
	if(dataPtr != NULL){
		BOOL fnd = FALSE;
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		jint respCode = -1;
		//Determine resultCode
		if(billingResult != NULL){
			jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
			if(clsResult != NULL){
				jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
				if(mGetRespCode != NULL){
					respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
		}
		//Process
		{
			NBThreadMutex_lock(&data->purchases.mutex);
			if(data->actions.consumption.isSet){
				BOOL actionStarted = FALSE;
				SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
					STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
					if(NBString_strIsEqual(pur->uid.sku, data->actions.consumption.sku) && NBString_strIsEqual(pur->uid.token, data->actions.consumption.token)){
						if(respCode == 0){
							PRINTF_INFO("AUAppGlueAndroidStore, onConsumeResponse success for sku('%s').\n", data->actions.consumption.sku);
							pur->consume.isConsumed = TRUE;
						} else if(respCode == 8){ //ITEM_NOT_OWNED
							PRINTF_INFO("AUAppGlueAndroidStore, onConsumeResponse ITEM_NOT_OWNED for sku('%s').\n", data->actions.consumption.sku);
							pur->consume.isConsumed = TRUE;
						} else {
							PRINTF_INFO("AUAppGlueAndroidStore, onConsumeResponse code(%d) for sku('%s').\n", respCode, data->actions.consumption.sku);
						}
						data->actions.consumption.isSet = FALSE;
						fnd = TRUE;
						break;
					}
				}
			}
			NBThreadMutex_unlock(&data->purchases.mutex);
		}
		//Eval
		if(!fnd){
			if(respCode == 0){
				PRINTF_INFO("AUAppGlueAndroidStore, onConsumeResponse success (not processed).\n");
			} else {
				PRINTF_ERROR("AUAppGlueAndroidStore, onConsumeResponse returned code(%d) (not processed).\n", respCode);
			}
		}
	}
}

void AUAppGlueAndroidStore::onSkuDetailsResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* pSkuDetailsList /*jobject*/, void* dataPtr){
	if(dataPtr != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)dataPtr;
		jint respCode = -1; SI32 reqsRemain = 0, reqsError = 0, reqsSuccess = 0;
		//Determine resultCode
		if(billingResult != NULL){
			jclass clsResult = jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
			if(clsResult != NULL){
				jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
				if(mGetRespCode != NULL){
					respCode = jEnv->CallIntMethod((jobject)billingResult, mGetRespCode);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
		}
		//Process list
		if(respCode == 0){ //BillingClient.BillingResponse.OK
			if(pSkuDetailsList == NULL){
				PRINTF_ERROR("AUAppGlueAndroidStore, empty sku-list returned.\n");
			} else {
				jobject skuDetailsList = (jobject)pSkuDetailsList;
				jclass clsList		= jEnv->FindClass("java/util/List"); NBASSERT(clsList != NULL)
				jclass clsSkuDet	= jEnv->FindClass("com/android/billingclient/api/SkuDetails"); NBASSERT(clsSkuDet != NULL)
				if(clsList != NULL && clsSkuDet != NULL){
					jmethodID mSize		 = jEnv->GetMethodID(clsList, "size", "()I"); NBASSERT(mSize != NULL)
					jmethodID mGet		= jEnv->GetMethodID(clsList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mGet != NULL)
					jmethodID mGetSku	= jEnv->GetMethodID(clsSkuDet, "getSku", "()Ljava/lang/String;"); NBASSERT(mGetSku != NULL)
					jmethodID mGetTit	= jEnv->GetMethodID(clsSkuDet, "getTitle", "()Ljava/lang/String;"); NBASSERT(mGetTit != NULL)
					jmethodID mGetDesc	= jEnv->GetMethodID(clsSkuDet, "getDescription", "()Ljava/lang/String;"); NBASSERT(mGetDesc != NULL)
					jmethodID mGetPrice	= jEnv->GetMethodID(clsSkuDet, "getPrice", "()Ljava/lang/String;"); NBASSERT(mGetPrice != NULL)
					if(mSize != NULL && mGet != NULL && mGetSku != NULL && mGetTit != NULL && mGetDesc != NULL && mGetPrice != NULL){
						jint size = jEnv->CallIntMethod(skuDetailsList, mSize);
						PRINTF_INFO("AUAppGlueAndroidStore, %d products returned.\n", size);
						jint i; for(i = 0; i < size; i++){
							jobject skuDet = jEnv->CallObjectMethod(skuDetailsList, mGet, i);
							if(skuDet == NULL){
								PRINTF_INFO("AUAppGlueAndroidStore, #%d product empty.\n", (i + 1));
							} else {
								const char* strSku = NULL;
								const char* strTit = NULL;
								const char* strDesc = NULL;
								const char* strPrice = NULL;
								jstring jSku	= (jstring)jEnv->CallObjectMethod(skuDet, mGetSku);
								jstring jTit	= (jstring)jEnv->CallObjectMethod(skuDet, mGetTit);
								jstring jDesc	= (jstring)jEnv->CallObjectMethod(skuDet, mGetDesc);
								jstring jPrice	= (jstring)jEnv->CallObjectMethod(skuDet, mGetPrice);
								{
									if(jSku != NULL)	strSku = jEnv->GetStringUTFChars(jSku, 0);
									if(jTit != NULL)	strTit = jEnv->GetStringUTFChars(jTit, 0);
									if(jDesc != NULL)	strDesc = jEnv->GetStringUTFChars(jDesc, 0);
									if(jPrice != NULL)	strPrice = jEnv->GetStringUTFChars(jPrice, 0);
									PRINTF_INFO("AUAppGlueAndroidStore, #%d product sku('%s'), price('%s'), tit('%s') desc('%s').\n", (i + 1), strSku, strPrice, strTit, strDesc);
									//Add sku to array
									{
										SI32 i; for(i = 0; i < data->billingClient.skusDetails.use; i++){
											STAppGlueAndroidStoreSkuDet* skuDett = NBArray_itmPtrAtIndex(&data->billingClient.skusDetails, STAppGlueAndroidStoreSkuDet, i);
											if(NBString_strIsEqual(skuDett->sku, strSku)){
												//Update sku
												NBString_strFreeAndNewBuffer(&skuDett->title, strTit);
												NBString_strFreeAndNewBuffer(&skuDett->desc, strDesc);
												NBString_strFreeAndNewBuffer(&skuDett->price, strPrice);
												if(skuDett->skuDetails != NULL) jEnv->DeleteGlobalRef(skuDett->skuDetails); skuDett->skuDetails = NULL;
												skuDett->skuDetails = jEnv->NewGlobalRef(skuDet);
												break;
											}
										}
										if(i == data->billingClient.skusDetails.use){
											//Add new sku
											STAppGlueAndroidStoreSkuDet skuDett;
											NBMemory_setZeroSt(skuDett, STAppGlueAndroidStoreSkuDet);
											skuDett.sku			= NBString_strNewBuffer(strSku);
											skuDett.title		= NBString_strNewBuffer(strTit);
											skuDett.desc		= NBString_strNewBuffer(strDesc);
											skuDett.price		= NBString_strNewBuffer(strPrice);
											skuDett.skuDetails	= jEnv->NewGlobalRef(skuDet);
											NBArray_addValue(&data->billingClient.skusDetails, skuDett);
										}
									}
									//Notify sku to local store
									{
										NBMngrStore::setProdData(strSku, strTit, strDesc, strPrice);
									}
								}
								if(jSku != NULL && strSku != NULL) jEnv->ReleaseStringUTFChars(jSku, strSku);
								if(jTit != NULL && strTit != NULL) jEnv->ReleaseStringUTFChars(jTit, strTit);
								if(jDesc != NULL && strDesc != NULL) jEnv->ReleaseStringUTFChars(jDesc, strDesc);
								if(jPrice != NULL && strPrice != NULL) jEnv->ReleaseStringUTFChars(jPrice, strPrice);
							}
						}
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsList)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsSkuDet)
			}
		}
		//Consume request end
		{
			NBThreadMutex_lock(&data->actions.refresh.reqsMutex);
			{
				PRINTF_INFO("AUAppGlueAndroidStore, onSkuDetailsResponse respCode(%d) (%d remains).\n", respCode, (data->actions.refresh.reqsRemain - 1));
				//Apply
				{
					data->actions.refresh.reqsRemain--;
					if(respCode == 0 /*BillingClient.BillingResponse.OK*/){
						data->actions.refresh.reqsSuccess++;
					} else {
						data->actions.refresh.reqsError++;
					}
				}
				//Copy
				{
					reqsRemain	= data->actions.refresh.reqsRemain;
					reqsError	= data->actions.refresh.reqsError;
					reqsSuccess	= data->actions.refresh.reqsSuccess;
				}
			}
			NBThreadMutex_unlock(&data->actions.refresh.reqsMutex);
		}
		//Process final
		if(data->actions.refresh.isSet){
			if(reqsRemain == 0){
				PRINTF_INFO("AUAppGlueAndroidStore, doResfreshAction ended at 'onSkuDetailsResponse' (last request ended).\n");
				NBMngrStore::endStoreSync(reqsSuccess > 0 ? ENStoreResult_Success : ENStoreResult_Error);
				data->actions.refresh.isSet = FALSE;
				//Refresh purchases cache using current skus list
				{
					AUAppGlueAndroidStore_doSyncWithPurchasesCache(data);
				}
			}
		}
	}
}

//

bool AUAppGlueAndroidStore::loadData(void* pData){
	bool r = false;
	/*NBASSERT(pData != NULL)
	if(pData != NULL){
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(!AUAppGlueAndroidJNI::loadDataFromSharedPrefs(jEnv, jContext, "NB_STORE", "DATA", str)){
					PRINTF_ERROR("AUAppGlueAndroidStore, loadDataFromSharedPrefs 'NB_STORE/DATA' failed.\n");
				} else {
					if(!NBMngrStore::lockedLoadFromJSON(str->str())){
						PRINTF_ERROR("AUAppGlueAndroidStore, lockedLoadFromJSON failed.\n");
					} else {
						//PRINTF_INFO("AUAppGlueAndroidStore, loaded: '%s'.\n", str->str());
						r = true;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
		}
	}*/
	return r;
}

bool AUAppGlueAndroidStore::saveData(void* pData){
	bool r = false;
	NBASSERT(pData != NULL)
	if(pData != NULL){
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(!NBMngrStore::lockedSaveToJSON(str)){
					PRINTF_ERROR("AUAppGlueAndroidStore, lockedSaveToJSON failed.\n");
				} else {
					if(!AUAppGlueAndroidJNI::saveDataToSharedPrefs(jEnv, jContext, "NB_STORE", "DATA", str->str())){
						PRINTF_ERROR("AUAppGlueAndroidStore, saveDataToSharedPrefs 'NB_STORE/DATA' failed.\n");
					} else {
						//PRINTF_INFO("AUAppGlueAndroidStore, saved: '%s'.\n", str->str());
						r = true;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidStore::startRefresh(void* pData, const STAppStoreProdId* prodIds, const SI32 prodIdsCount){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		//Refresh with new billing
		if(data->billingClient.jLstnr == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jLstnr is NULL.\n");
		} else if(data->billingClient.jClt == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jClt is NULL.\n");
		} else {
			//Set action
			if(data->actions.refresh.isSet){
				PRINTF_ERROR("AUAppGlueAndroidStore, 'startRefresh', already executing refresh.\n");
			} else if(data->actions.purchase.isSet){
				PRINTF_ERROR("AUAppGlueAndroidStore, 'startRefresh', already executing purchase.\n");
			} else if(data->actions.restore.isSet){
				PRINTF_ERROR("AUAppGlueAndroidStore, 'startRefresh', already executing restore.\n");
			} else {
				//Set action data
				{
					NBThreadMutex_lock(&data->actions.refresh.reqsMutex);
					{
						//Release
						{
							AUAppGlueAndroidStore_skuArrRelease(data->actions.refresh.skus, data->actions.refresh.skusSz);
							data->actions.refresh.skus		= NULL;
							data->actions.refresh.skusSz	= 0;
						}
						//Clone new
						{
							data->actions.refresh.skus = AUAppGlueAndroidStore_skuArrClone(&data->actions.refresh.skusSz, prodIds, prodIdsCount);
							{
								
								{
									data->actions.refresh.reqsRemain	= 0;
									data->actions.refresh.reqsSuccess	= 0;
									data->actions.refresh.reqsError		= 0;
								}
								
							}
						}
						data->actions.refresh.isSet = TRUE;
					}
					NBThreadMutex_unlock(&data->actions.refresh.reqsMutex);
				}
				//Execute
				if(data->billingClient.isConnected){
					//Do now
					AUAppGlueAndroidStore_doResfreshAction(data);
					r = true;
				} else {
					//Connect first and do after
					NBASSERT(!data->billingClient.isConnected)
					if(!data->billingClient.isConnecting){
						NBASSERT(!data->billingClient.isConnecting)
						NBASSERT(!data->billingClient.isConnected)
						JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
						if(jEnv == NULL){
							PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
						} else {
							jclass clsClt = jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
							//Start connection
							{
								jmethodID mStartConn	= jEnv->GetMethodID(clsClt, "startConnection", "(Lcom/android/billingclient/api/BillingClientStateListener;)V"); NBASSERT(mStartConn != NULL)
								data->billingClient.isConnecting = TRUE;
								PRINTF_INFO("AUAppGlueAndroidStore, jClt starting connection.\n");
								jEnv->CallVoidMethod(data->billingClient.jClt, mStartConn, data->billingClient.jLstnr);
								r = true;
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, clsClt)
						}
					}
				}
				//Result
				if(!r){
					NBMngrStore::endStoreSync(ENStoreResult_Error);
				}
			}
		}
	}
	return r;
}


void AUAppGlueAndroidStore_doResfreshAction(AUAppGlueAndroidStoreData* data){
	NBASSERT(data->actions.refresh.isSet)
	SI32 reqsStarted = 0;
	if(!data->billingClient.isConnected){
		PRINTF_ERROR("AUAppGlueAndroidStore_doResfreshAction, client not connected.\n");
	} else {
		PRINTF_INFO("AUAppGlueAndroidStore_doResfreshAction, client connected.\n");
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
		} else {
			//Start request
			if(data->billingClient.jClt != NULL && data->billingClient.jLstnr != NULL){
				jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
				jclass clsParms		= jEnv->FindClass("com/android/billingclient/api/SkuDetailsParams"); NBASSERT(clsParms != NULL)
				jclass clsParmsBldr	= jEnv->FindClass("com/android/billingclient/api/SkuDetailsParams$Builder"); NBASSERT(clsParmsBldr != NULL)
				if(clsClt != NULL && clsParms != NULL && clsParmsBldr != NULL){
					jmethodID mNewBldr = jEnv->GetStaticMethodID(clsParms, "newBuilder", "()Lcom/android/billingclient/api/SkuDetailsParams$Builder;"); NBASSERT(mNewBldr != NULL)
					jmethodID mSetSkus = jEnv->GetMethodID(clsParmsBldr, "setSkusList", "(Ljava/util/List;)Lcom/android/billingclient/api/SkuDetailsParams$Builder;"); NBASSERT(mSetSkus != NULL)
					jmethodID mSetType	= jEnv->GetMethodID(clsParmsBldr, "setType", "(Ljava/lang/String;)Lcom/android/billingclient/api/SkuDetailsParams$Builder;"); NBASSERT(mSetType != NULL)
					jmethodID mBuild	= jEnv->GetMethodID(clsParmsBldr, "build", "()Lcom/android/billingclient/api/SkuDetailsParams;"); NBASSERT(mBuild != NULL)
					jmethodID mQueryA	= jEnv->GetMethodID(clsClt, "querySkuDetailsAsync", "(Lcom/android/billingclient/api/SkuDetailsParams;Lcom/android/billingclient/api/SkuDetailsResponseListener;)V"); NBASSERT(mQueryA != NULL)
					if(mNewBldr != NULL && mSetSkus != NULL && mSetType != NULL && mBuild != NULL && mQueryA != NULL){
						jobject jBldr = jEnv->CallStaticObjectMethod(clsParms, mNewBldr); NBASSERT(jBldr != NULL)
						if(jBldr != NULL){
							SI32 mode = 0; //'0' = inapp products, '1' = subscriptions
							NBThreadMutex_lock(&data->actions.refresh.reqsMutex);
							while(mode < 2){ //Locked
								//Build list
								{
									jclass clsArrList	= jEnv->FindClass("java/util/ArrayList"); NBASSERT(clsArrList != NULL)
									if(clsArrList != NULL){
										jmethodID mArrInit		= jEnv->GetMethodID(clsArrList, "<init>", "()V"); NBASSERT(mArrInit != NULL)
										jmethodID mArrAdd		= jEnv->GetMethodID(clsArrList, "add", "(Ljava/lang/Object;)Z"); NBASSERT(mArrAdd != NULL)
										jmethodID mArrGet		= jEnv->GetMethodID(clsArrList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mArrGet != NULL)
										jmethodID mArrSize		= jEnv->GetMethodID(clsArrList, "size", "()I"); NBASSERT(mArrSize != NULL)
										if(mArrInit != NULL && mArrAdd != NULL && mArrGet != NULL && mArrSize != NULL){
											jobject jList = jEnv->NewObject(clsArrList, mArrInit); NBASSERT(jList != NULL)
											if(jList != NULL){
												UI32 listSz = 0;
												{
													SI32 i; for(i = 0; i < data->actions.refresh.skusSz; i++){
														const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
														if(!NBString_strIsEmpty(sku->prodId)){
															if(sku->type == (mode == 0 ? ENStoreProdType_InApp : ENStoreProdType_Subscription)){
																jstring jStrID = jEnv->NewStringUTF(sku->prodId);
																jEnv->CallBooleanMethod(jList, mArrAdd, jStrID);
																PRINTF_INFO("AUAppGlueAndroidStore_doResfreshAction, sku added('%s') as '%s'.\n", sku->prodId, (mode == 0 ? "inapp" : "subs"));
																NBJNI_DELETE_REF_LOCAL(jEnv, jStrID)
																listSz++;
															}
														}
													}
												}
												//Send request
												if(jList != NULL && listSz > 0){
													jstring jStrType = jEnv->NewStringUTF((mode == 0 ? "inapp" : "subs")); //"inapp" | "subs"
													jEnv->CallObjectMethod(jBldr, mSetSkus, jList);
													jEnv->CallObjectMethod(jBldr, mSetType, jStrType);
													{
														jobject params = jEnv->CallObjectMethod(jBldr, mBuild);
														if(params != NULL){
															jEnv->CallVoidMethod(data->billingClient.jClt, mQueryA, params, data->billingClient.jLstnr);
															data->actions.refresh.reqsRemain++;
															reqsStarted++;
														}
														NBJNI_DELETE_REF_LOCAL(jEnv, params)
													}
													NBJNI_DELETE_REF_LOCAL(jEnv, jStrType)
												}
											}
											NBJNI_DELETE_REF_LOCAL(jEnv, jList);
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, clsArrList)
									}
								}
								//Next mode
								mode++;
							}
							NBThreadMutex_unlock(&data->actions.refresh.reqsMutex);
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jBldr);
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
				NBJNI_DELETE_REF_LOCAL(jEnv, clsParms);
				NBJNI_DELETE_REF_LOCAL(jEnv, clsParmsBldr);
			}
		}
	}
	//Result
	{
		PRINTF_INFO("AUAppGlueAndroidStore_doResfreshAction, reqsStarted(%d).\n", reqsStarted);
		if(data->actions.refresh.isSet){
			if(reqsStarted <= 0){
				PRINTF_INFO("AUAppGlueAndroidStore, doResfreshAction ended at 'doResfreshAction' (zero requests started).\n");
				NBMngrStore::endStoreSync(ENStoreResult_Error);
				data->actions.refresh.isSet = FALSE;
			}
		}
	}
}

//
//Buy product
//
/*
 // Billing response codes
 public static final int BILLING_RESPONSE_RESULT_OK = 0;
 public static final int BILLING_RESPONSE_RESULT_USER_CANCELED = 1;
 public static final int BILLING_RESPONSE_RESULT_SERVICE_UNAVAILABLE = 2;
 public static final int BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE = 3;
 public static final int BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE = 4;
 public static final int BILLING_RESPONSE_RESULT_DEVELOPER_ERROR = 5;
 public static final int BILLING_RESPONSE_RESULT_ERROR = 6;
 public static final int BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED = 7;
 public static final int BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED = 8;
Bundle buyIntentBundle = _service.getBuyIntent(3, _activity.getPackageName(), productSKU, "inapp", "");
if(buyIntentBundle != null){
	int response = buyIntentBundle.getInt("RESPONSE_CODE");
	if(response == 0){
		Log.i("BILLING", "Consulta getBuyIntent retornó: " + response);
		PendingIntent pendingIntent = buyIntentBundle.getParcelable("BUY_INTENT");
		if(pendingIntent != null){
			_curActionState = ENPurchaseResult_Pending;
			_activity.startIntentSenderForResult(pendingIntent.getIntentSender(), 1001, new Intent(), Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));
			r = true;
		}
	} else {
		Log.e("BILLING", "Consulta getBuyIntent retornó: " + response);
	}
}*/

bool AUAppGlueAndroidStore::startPurchase(void* pData, const char* prodId){
	bool r = false;
	if(pData != NULL && prodId != NULL){
		if(prodId[0] != '\0'){
			AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			if(data->billingClient.jLstnr == NULL){
				PRINTF_ERROR("AUAppGlueAndroidStore, jLstnr is NULL.\n");
			} else if(data->billingClient.jClt == NULL){
				PRINTF_ERROR("AUAppGlueAndroidStore, jClt is NULL.\n");
			} else {
				//Set action
				if(data->actions.refresh.isSet){
					PRINTF_ERROR("AUAppGlueAndroidStore, 'startPurchase', already executing refresh.\n");
				} else if(data->actions.purchase.isSet){
					PRINTF_ERROR("AUAppGlueAndroidStore, 'startPurchase', already executing purchase.\n");
				} else if(data->actions.restore.isSet){
					PRINTF_ERROR("AUAppGlueAndroidStore, 'startPurchase', already executing restore.\n");
				} else {
					//Set action data
					{
						//Release
						{
							if(data->actions.purchase.sku != NULL) NBMemory_free(data->actions.purchase.sku); data->actions.purchase.sku = NULL;
							if(data->actions.purchase.grpId != NULL) NBMemory_free(data->actions.purchase.grpId); data->actions.purchase.grpId = NULL;
						}
						//Clone new
						{
							data->actions.purchase.sku		= NBString_strNewBuffer(prodId);
							data->actions.purchase.type		= ENStoreProdType_InApp;
							data->actions.purchase.grpId	= NULL;
							//Search sku props
							{
								SI32 i; for(i = 0; i < data->actions.refresh.skusSz; i++){
									const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
									if(NBString_strIsEqual(prodId, sku->prodId)){
										data->actions.purchase.type = sku->type;
										data->actions.purchase.grpId = NBString_strNewBuffer(sku->grpId);
										break;
									}
								}
							}
							PRINTF_INFO("AUAppGlueAndroidStore, purchasing('%s') type('%s') grp('%s').\n", data->actions.purchase.sku, (data->actions.purchase.type == ENStoreProdType_InApp ? "inapp" : data->actions.purchase.type == ENStoreProdType_Subscription ? "subs" : "unexpected_value"), data->actions.purchase.grpId);
						}
						data->actions.purchase.isSet		= TRUE;
						data->actions.purchase.isLaunchFlow	= FALSE;
						data->actions.purchase.isLaunchFlowGainedFocus = FALSE;	//The first time the focus is lost for the purchaseFlow
						data->actions.purchase.isLaunchFlowReturnedFocus = FALSE;	//The first time the focus is gained after the purchaseFlow
						data->actions.purchase.isLaunchFlowReturnedFocusSecsAccum = 0.0f; //Teh ammount of seconds with the focus gained after the purchaseFlow
					}
					//Execute
					if(data->billingClient.isConnected){
						//Do now
						AUAppGlueAndroidStore_doPurchaseAction(data);
						r = TRUE;
					} else {
						//Connect first and do after
						NBASSERT(!data->billingClient.isConnected)
						if(!data->billingClient.isConnecting){
							NBASSERT(!data->billingClient.isConnecting)
							NBASSERT(!data->billingClient.isConnected)
							JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
							if(jEnv == NULL){
								PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
							} else {
								jclass clsClt = jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
								//Start connection
								{
									jmethodID mStartConn	= jEnv->GetMethodID(clsClt, "startConnection", "(Lcom/android/billingclient/api/BillingClientStateListener;)V"); NBASSERT(mStartConn != NULL)
									data->billingClient.isConnecting = TRUE;
									PRINTF_INFO("AUAppGlueAndroidStore, jClt starting connection.\n");
									jEnv->CallVoidMethod(data->billingClient.jClt, mStartConn, data->billingClient.jLstnr);
									r = true;
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, clsClt)
							}
						}
					}
				}
			}
		}
	}
	return r;
}

void AUAppGlueAndroidStore_lauchBillingFlow(void* pData){
	BOOL alreadyOwned = FALSE;
	AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData; NBASSERT(data != NULL)
	NBASSERT(data->actions.purchase.isSet)
	NBASSERT(!data->actions.purchase.isLaunchFlow)
	if(!data->billingClient.isConnected){
		PRINTF_ERROR("AUAppGlueAndroidStore_lauchBillingFlow, client not connected.\n");
	} else {
		PRINTF_INFO("AUAppGlueAndroidStore_lauchBillingFlow, client connected.\n");
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jActivity = (jobject)jniGlue->jActivity(); NBASSERT(jActivity != NULL)
		if(jEnv == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
		} else if(jActivity == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jActivity is NULL.\n");
		} else {
			//Start request
			if(data->billingClient.jClt != NULL && data->billingClient.jLstnr != NULL){
				jclass clsClt			= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
				jclass clsResult		= jEnv->FindClass("com/android/billingclient/api/BillingResult"); NBASSERT(clsResult != NULL)
				if(clsClt != NULL && clsResult != NULL){
					BOOL isSupported = TRUE;
					jmethodID mGetRespCode = jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetRespCode != NULL)
					//Detemrine if purchase is supported
					if(data->actions.purchase.type == ENStoreProdType_Subscription){
						jmethodID mIsSupp	= jEnv->GetMethodID(clsClt, "isFeatureSupported", "(Ljava/lang/String;)Lcom/android/billingclient/api/BillingResult;"); NBASSERT(mIsSupp != NULL)
						if(mIsSupp != NULL && mGetRespCode != NULL){
							jstring jStrType	= jEnv->NewStringUTF("subscriptions"); //SUBSCRIPTIONS
							jobject jResp		= jEnv->CallObjectMethod((jobject)data->billingClient.jClt, mIsSupp, jStrType);
							if(jResp != NULL){
								const jint respCode = jEnv->CallIntMethod(jResp, mGetRespCode);
								if(respCode != 0){ //BillingClient.BillingResponseCode.OK
									isSupported = FALSE;
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jResp)
							NBJNI_DELETE_REF_LOCAL(jEnv, jStrType)
						}
					}
					//
					if(!isSupported){
						PRINTF_ERROR("AUAppGlueAndroidStore_doPurchaseAction, productType not supported by client.\n");
					} else {
						/*
						BillingFlowParams flowParams = BillingFlowParams.newBuilder()
								.setSkuDetails(skuDetails)
								.build();
						int responseCode = billingClient.launchBillingFlow(flowParams);
						*/
                        jclass clsParms        = jEnv->FindClass("com/android/billingclient/api/BillingFlowParams"); NBASSERT(clsParms != NULL)
                        jclass clsParmsBldr    = jEnv->FindClass("com/android/billingclient/api/BillingFlowParams$Builder"); NBASSERT(clsParmsBldr != NULL)
                        jclass clsParmsSubUpd = jEnv->FindClass("com/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams"); NBASSERT(clsParmsSubUpd != NULL)
                        jclass clsParmsSubUpdBldr = jEnv->FindClass("com/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder"); NBASSERT(clsParmsSubUpdBldr != NULL)
                        if(clsParms != NULL && clsParmsBldr != NULL && clsParmsSubUpd != NULL && clsParmsSubUpdBldr != NULL){
                            jmethodID mNewBldr    = jEnv->GetStaticMethodID(clsParms, "newBuilder", "()Lcom/android/billingclient/api/BillingFlowParams$Builder;"); NBASSERT(mNewBldr != NULL)
                            jmethodID mSetSkuD    = jEnv->GetMethodID(clsParmsBldr, "setSkuDetails", "(Lcom/android/billingclient/api/SkuDetails;)Lcom/android/billingclient/api/BillingFlowParams$Builder;"); NBASSERT(mSetSkuD != NULL)
                            jmethodID mBuild    = jEnv->GetMethodID(clsParmsBldr, "build", "()Lcom/android/billingclient/api/BillingFlowParams;"); NBASSERT(mBuild != NULL)
                            jmethodID mLaunch    = jEnv->GetMethodID(clsClt, "launchBillingFlow", "(Landroid/app/Activity;Lcom/android/billingclient/api/BillingFlowParams;)Lcom/android/billingclient/api/BillingResult;"); NBASSERT(mLaunch != NULL)
                            //v4
                            jmethodID mSetSubUpdParams = jEnv->GetMethodID(clsParmsBldr, "setSubscriptionUpdateParams", "(Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams;)Lcom/android/billingclient/api/BillingFlowParams$Builder;"); NBASSERT(mSetSubUpdParams != NULL)
                            jmethodID mUpdNewBldr = jEnv->GetStaticMethodID(clsParmsSubUpd, "newBuilder", "()Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder;"); NBASSERT(mUpdNewBldr != NULL)
                            jmethodID mUpdSetSkuO = jEnv->GetMethodID(clsParmsSubUpdBldr, "setOldSkuPurchaseToken", "(Ljava/lang/String;)Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder;"); NBASSERT(mUpdSetSkuO != NULL)
                            jmethodID mUpdBuild   = jEnv->GetMethodID(clsParmsSubUpdBldr, "build", "()Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams;"); NBASSERT(mUpdBuild != NULL)
                            if(mNewBldr != NULL && mSetSkuD != NULL && mSetSubUpdParams != NULL && mBuild != NULL && mLaunch != NULL && mGetRespCode != NULL && mUpdNewBldr != NULL && mUpdSetSkuO != NULL && mUpdBuild != NULL){
                                jobject jBldr = jEnv->CallStaticObjectMethod(clsParms, mNewBldr); NBASSERT(jBldr != NULL)
                                if(jBldr != NULL){
                                    BOOL detailsAdded = FALSE;
                                    //Add details
                                    {
                                        SI32 i; for(i = 0; i < data->billingClient.skusDetails.use; i++){
                                            STAppGlueAndroidStoreSkuDet* skuDett = NBArray_itmPtrAtIndex(&data->billingClient.skusDetails, STAppGlueAndroidStoreSkuDet, i);
                                            if(NBString_strIsEqual(skuDett->sku, data->actions.purchase.sku)){
                                                if(skuDett->skuDetails != NULL){
                                                    jEnv->CallObjectMethod(jBldr, mSetSkuD, skuDett->skuDetails);
                                                    detailsAdded = TRUE;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    //Search for old sku (if subscription)
                                    if(data->actions.purchase.type == ENStoreProdType_Subscription && !NBString_strIsEmpty(data->actions.purchase.grpId)){
                                        NBThreadMutex_lock(&data->purchases.mutex);
                                        {
                                            BOOL fnd = FALSE;
                                            SI32 i; for(i = 0; i < data->actions.refresh.skusSz && !fnd; i++){
                                                const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
                                                if(NBString_strIsEqual(sku->grpId, data->actions.purchase.grpId)){ //Same group
                                                    if(!NBString_strIsEqual(sku->prodId, data->actions.purchase.sku)){ //Not the same SKU
                                                        //Search in active purchases
                                                        SI32 i; for(i = 0; i < data->purchases.arr.use; i++){
                                                            STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
                                                            if(NBString_strIsEqual(pur->uid.sku, sku->prodId)){
                                                                if(!NBString_strIsEmpty(pur->uid.token)){
                                                                    PRINTF_INFO("AUAppGlueAndroidStore_doPurchaseAction, purchasing('%s'), with oldSku('%s') oldToken('%s').\n", data->actions.purchase.sku, pur->uid.sku, pur->uid.token);
                                                                    jobject jUpdBldr = jEnv->CallStaticObjectMethod(clsParmsSubUpd, mUpdNewBldr); NBASSERT(jUpdBldr != NULL)
                                                                    if(jUpdBldr != NULL){
                                                                        jstring jOldSku = jEnv->NewStringUTF(pur->uid.sku);
                                                                        jstring jOldSkuToken = jEnv->NewStringUTF(pur->uid.token);
                                                                        {
                                                                            jobject jUpdParams = NULL;
                                                                            jEnv->CallObjectMethod(jUpdBldr, mUpdSetSkuO, jOldSkuToken);
                                                                            jUpdParams = jEnv->CallObjectMethod(jUpdBldr, mUpdBuild); NBASSERT(jUpdParams != NULL)
                                                                            if(jUpdParams != NULL){
                                                                                //set upd-param
                                                                                jEnv->CallObjectMethod(jBldr, mSetSubUpdParams, jUpdParams);
                                                                                //
                                                                                NBJNI_DELETE_REF_LOCAL(jEnv, jUpdParams);
                                                                                jUpdParams = NULL;
                                                                            }
                                                                        }
                                                                        NBJNI_DELETE_REF_LOCAL(jEnv, jOldSku);
                                                                        NBJNI_DELETE_REF_LOCAL(jEnv, jOldSkuToken);
                                                                        //
                                                                        NBJNI_DELETE_REF_LOCAL(jEnv, jUpdBldr);
                                                                    }
                                                                    fnd = TRUE;
                                                                    break;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        NBThreadMutex_unlock(&data->purchases.mutex);
                                    }
                                    //Process
                                    if(!detailsAdded){
                                        PRINTF_ERROR("AUAppGlueAndroidStore_doPurchaseAction, skuDetails not found (in previous store-sync response).\n");
                                    } else {
                                        jobject jParams = jEnv->CallObjectMethod(jBldr, mBuild);
                                        if(jParams != NULL){
                                            jobject jResp = jEnv->CallObjectMethod(data->billingClient.jClt, mLaunch, jActivity, jParams);
                                            if(jResp != NULL){
                                                data->actions.purchase.isLaunchFlow    = TRUE;
                                                data->actions.purchase.isLaunchFlowGainedFocus = FALSE;    //The first time the focus is lost for the purchaseFlow
                                                data->actions.purchase.isLaunchFlowReturnedFocus = FALSE;    //The first time the focus is gained after the purchaseFlow
                                                data->actions.purchase.isLaunchFlowReturnedFocusSecsAccum = 0.0f; //Teh ammount of seconds with the focus gained after the purchaseFlow
                                                const jint respCode = jEnv->CallIntMethod(jResp, mGetRespCode);
                                                if(respCode == 0){ //BillingClient.BillingResponseCode.OK
                                                    PRINTF_INFO("AUAppGlueAndroidStore_doPurchaseAction, purchase flow launched.\n");
                                                } else if(respCode == 7){ //BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED
                                                    PRINTF_INFO("AUAppGlueAndroidStore_doPurchaseAction, purchase already owned.\n");
                                                    alreadyOwned = TRUE;
                                                    data->actions.purchase.isLaunchFlow    = FALSE;
                                                } else {
                                                    PRINTF_ERROR("AUAppGlueAndroidStore_doPurchaseAction, purchase flow launch returned code(%d).\n", respCode);
                                                    data->actions.purchase.isLaunchFlow    = FALSE;
                                                }
                                            }
                                        }
                                        NBJNI_DELETE_REF_LOCAL(jEnv, jParams);
                                    }
                                }
                                NBJNI_DELETE_REF_LOCAL(jEnv, jBldr);
                            }
                        }
                        NBJNI_DELETE_REF_LOCAL(jEnv, clsParms);
                        NBJNI_DELETE_REF_LOCAL(jEnv, clsParmsBldr);
                        NBJNI_DELETE_REF_LOCAL(jEnv, clsParmsSubUpd);
                        NBJNI_DELETE_REF_LOCAL(jEnv, clsParmsSubUpdBldr);
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsResult)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
			}
		}
	}
	//
	if(data->actions.purchase.isSet){
		if(!data->actions.purchase.isLaunchFlow){
			if(alreadyOwned){
				PRINTF_INFO("AUAppGlueAndroidStore, purchase flow ended at 'lauchBillingFlow' (already owned).\n");
				NBMngrStore::endStoreAction(ENStoreResult_NoChanges);
				data->actions.purchase.isSet		= FALSE;
				data->actions.purchase.isLaunchFlow	= FALSE;
			} else {
				PRINTF_INFO("AUAppGlueAndroidStore, purchase flow ended at 'lauchBillingFlow' (purchase flow not launched).\n");
				NBMngrStore::endStoreAction(ENStoreResult_Error);
				data->actions.purchase.isSet		= FALSE;
				data->actions.purchase.isLaunchFlow	= FALSE;
			}
		}
	}
}
	
void AUAppGlueAndroidStore_doPurchaseAction(AUAppGlueAndroidStoreData* data){
	BOOL runnableQueued = FALSE;
	NBASSERT(data->actions.purchase.isSet)
	if(!data->billingClient.isConnected){
		PRINTF_ERROR("AUAppGlueAndroidStore_doPurchaseAction, client not connected.\n");
	} else {
		PRINTF_INFO("AUAppGlueAndroidStore_doPurchaseAction, client connected.\n");
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
		} else {
			if(!jniGlue->addRunableForMainThread(data->app->getAppNative(), AUAppGlueAndroidStore_lauchBillingFlow, data)){
				PRINTF_ERROR("AUAppGlueAndroidStore_doPurchaseAction, addRunableForMainThread(lauchBillingFlow) failed.\n");
			} else {
				PRINTF_INFO("AUAppGlueAndroidStore_doPurchaseAction, addRunableForMainThread(lauchBillingFlow) for runnable.\n");
				runnableQueued = TRUE;
			}
		}
	}
	//Result
	if(data->actions.purchase.isSet){
		if(!runnableQueued){
			PRINTF_INFO("AUAppGlueAndroidStore, purchase flow ended at 'doPurchaseAction'.\n");
			NBMngrStore::endStoreAction(ENStoreResult_Error);
			data->actions.purchase.isSet		= FALSE;
			data->actions.purchase.isLaunchFlow	= FALSE;
		}
	}
}

//
//Restore purchases
//
/*public void run() {
	_curActionState = ENPurchaseResult_Pending;
	Log.i("BILLING", "Iniciando restauracion de compras.");
	try {
		Bundle ownedItems = _service.getPurchases(3, _activity.getPackageName(), "inapp", null);
		int response = ownedItems.getInt("RESPONSE_CODE");
		if (response != 0) {
			_curActionState = ENPurchaseResult_Error;
			Log.e("BILLING", "Restauracion 'getPurchases()' retorna '" + response + "'.");
		} else {
			_curActionState = ENPurchaseResult_Success;
			ArrayList<String> ownedSkus = ownedItems.getStringArrayList("INAPP_PURCHASE_ITEM_LIST");
			if(ownedSkus == null){
				Log.i("BILLING", "Restauracion retorno ningun SKU.");
			} else {
				Log.i("BILLING", "Restauracion retorno "+ownedSkus.size()+" SKUs.");
				//ArrayList<String>  purchaseDataList = ownedItems.getStringArrayList("INAPP_PURCHASE_DATA_LIST");
				//ArrayList<String>  signatureList = ownedItems.getStringArrayList("INAPP_DATA_SIGNATURE_LIST");
				//String continuationToken = ownedItems.getString("INAPP_CONTINUATION_TOKEN");
				for (int i = 0; i < ownedSkus.size(); ++i) {
					//String purchaseData = purchaseDataList.get(i);
					//String signature = signatureList.get(i);
					String sku = ownedSkus.get(i);
					Log.i("BILLING", "Restauracion de producto '"+sku+"'.");
					purchasedAddSKU(sku);
				}
				purchasedSave();
			}
		}
	} catch(Exception exc){
		Log.i("BILLING", "Excepcion en restauracion de compras: " + exc.getMessage());
		_curActionState = ENPurchaseResult_Error;
	}
	Log.i("BILLING", "Fin restauracion de compras.");
}*/

bool AUAppGlueAndroidStore::startRestore(void* pData){
	bool r = false;
	PRINTF_INFO("AUAppGlueAndroidStore, called startRestore.\n");
	if(pData != NULL){
		AUAppGlueAndroidStoreData* data = (AUAppGlueAndroidStoreData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		if(data->billingClient.jLstnr == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jLstnr is NULL.\n");
		} else if(data->billingClient.jClt == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jClt is NULL.\n");
		} else {
			//Set action
			if(data->actions.refresh.isSet){
				PRINTF_ERROR("AUAppGlueAndroidStore, 'startRestore', already executing refresh.\n");
			} else if(data->actions.purchase.isSet){
				PRINTF_ERROR("AUAppGlueAndroidStore, 'startRestore', already executing purchase.\n");
			} else if(data->actions.restore.isSet){
				PRINTF_ERROR("AUAppGlueAndroidStore, 'startRestore', already executing restore.\n");
			} else {
				//Set action data
				{
					data->actions.restore.isSet		= TRUE;
				}
				//Execute
				if(data->billingClient.isConnected){
					//Do now
					PRINTF_INFO("AUAppGlueAndroidStore, calling AUAppGlueAndroidStore_doRestoreAction.\n");
					AUAppGlueAndroidStore_doRestoreAction(data);
					r = TRUE;
				} else {
					//Connect first and do after
					NBASSERT(!data->billingClient.isConnected)
					if(!data->billingClient.isConnecting){
						NBASSERT(!data->billingClient.isConnecting)
						NBASSERT(!data->billingClient.isConnected)
						JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
						if(jEnv == NULL){
							PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
						} else {
							jclass clsClt = jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
							//Start connection
							{
								jmethodID mStartConn	= jEnv->GetMethodID(clsClt, "startConnection", "(Lcom/android/billingclient/api/BillingClientStateListener;)V"); NBASSERT(mStartConn != NULL)
								data->billingClient.isConnecting = TRUE;
								PRINTF_INFO("AUAppGlueAndroidStore, jClt starting connection.\n");
								jEnv->CallVoidMethod(data->billingClient.jClt, mStartConn, data->billingClient.jLstnr);
								r = true;
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, clsClt)
						}
					}
				}
			}
		}
	}
	return r;
}

void AUAppGlueAndroidStore_doRestoreAction(AUAppGlueAndroidStoreData* data){
	NBASSERT(data->actions.restore.isSet)
	SI32 reqsStarted = 0;
	if(!data->billingClient.isConnected){
		PRINTF_ERROR("AUAppGlueAndroidStore_doRestoreAction, client not connected.\n");
	} else {
		PRINTF_INFO("AUAppGlueAndroidStore_doRestoreAction, client connected.\n");
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv == NULL){
			PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
		} else {
			//Start request
			if(data->billingClient.jClt != NULL && data->billingClient.jLstnr != NULL){
				jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
				if(clsClt != NULL){
					jmethodID mQueryA	= jEnv->GetMethodID(clsClt, "queryPurchaseHistoryAsync", "(Ljava/lang/String;Lcom/android/billingclient/api/PurchaseHistoryResponseListener;)V"); NBASSERT(mQueryA != NULL)
					if(mQueryA != NULL){
						BOOL isReqInAppStarted = FALSE, isReqSubsStarted = FALSE;
						NBThreadMutex_lock(&data->actions.restore.reqsMutex);
						{
							SI32 i; for(i = 0; i < data->actions.refresh.skusSz; i++){
								const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
								if(!NBString_strIsEmpty(sku->prodId)){
									BOOL addQuery = FALSE; const char* skyType = "";
									if(sku->type == ENStoreProdType_InApp){
										if(!isReqInAppStarted){
											addQuery	= TRUE;
											skyType		= "inapp"; //INAPP
											isReqInAppStarted = TRUE;
										}
									} else if(sku->type == ENStoreProdType_Subscription){
										if(!isReqSubsStarted){
											addQuery	= TRUE;
											skyType		= "subs"; //SUBS
											isReqSubsStarted = TRUE;
										}
									}
									//Add query
									if(addQuery && !NBString_strIsEmpty(skyType)){
										jstring jSkuType = jEnv->NewStringUTF(skyType);
										PRINTF_INFO("AUAppGlueAndroidStore_doRestoreAction, skuType requests starting('%s').\n", skyType);
										{
											jEnv->CallVoidMethod(data->billingClient.jClt, mQueryA, jSkuType, data->billingClient.jLstnr);
											data->actions.restore.reqsRemain++;
											reqsStarted++;
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, jSkuType)
									}
								}
							}
						}
						NBThreadMutex_unlock(&data->actions.restore.reqsMutex);
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
			}
		}
	}
	//Result
	{
		PRINTF_INFO("AUAppGlueAndroidStore_doRestoreAction, reqsStarted(%d).\n", reqsStarted);
		if(data->actions.restore.isSet){
			if(reqsStarted <= 0){
				PRINTF_INFO("AUAppGlueAndroidStore, doRestoreAction ended at 'doRestoreAction' (zero requests started).\n");
				NBMngrStore::endStoreAction(ENStoreResult_Error);
				data->actions.restore.isSet = FALSE;
			}
		}
	}
}

bool AUAppGlueAndroidStore::startReceiptRefresh(void* pData){
	return  AUAppGlueAndroidStore::startRestore(pData);
}


//

void AUAppGlueAndroidStore_buildReceiptOfToken(AUAppGlueAndroidStoreData* data, const BOOL onlyIfNecesary){
	NBThreadMutex_lock(&data->purchases.mutex);
	{
		const BOOL isNecesary = (data->purchases.pendSynced > 0 || data->purchases.pendAdded > 0);
		if(isNecesary || !onlyIfNecesary){
			SI32 addedCount = 0;
			STNBString tokensJson;
			NBString_init(&tokensJson);
			NBString_concat(&tokensJson, "{");
			NBString_concat(&tokensJson, "\"tokens\":[");
			{
				SI32 i; for(i = 0; i < data->purchases.arr.use; i++){
					STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
					if(!NBString_strIsEmpty(pur->uid.sku) && !NBString_strIsEmpty(pur->uid.token)){
						if(addedCount > 0) NBString_concat(&tokensJson, ",");
						NBString_concat(&tokensJson, "{");
						NBString_concat(&tokensJson, "\"sku\":\""); NBJson_concatScaped(&tokensJson, pur->uid.sku); NBString_concat(&tokensJson, "\"");
						NBString_concat(&tokensJson, ",\"token\":\""); NBJson_concatScaped(&tokensJson, pur->uid.token); NBString_concat(&tokensJson, "\"");
						NBString_concat(&tokensJson, "}");
						addedCount++;
					}
				}
			}
			NBString_concat(&tokensJson, "]");
			NBString_concat(&tokensJson, "}");
			if(addedCount <= 0){
				NBString_empty(&data->purchases.tokensJsonStr);
			} else {
				NBString_set(&data->purchases.tokensJsonStr, tokensJson.str);
			}
			PRINTF_INFO("AUAppGlueAndroidStore_doSyncWithPurchasesCache tokens (%d): %s.\n", addedCount, data->purchases.tokensJsonStr.str);
			NBString_release(&tokensJson);
			//
			data->purchases.pendSynced	= 0;
			data->purchases.pendAdded	= 0;
		}
	}
	NBThreadMutex_unlock(&data->purchases.mutex);
}

void AUAppGlueAndroidStore_doSyncWithPurchasesCache(AUAppGlueAndroidStoreData* data){
	AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
	JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
	if(jEnv == NULL){
		PRINTF_ERROR("AUAppGlueAndroidStore, jEnv is NULL.\n");
	} else {
		//Start request
		if(data->billingClient.jClt != NULL && data->billingClient.jLstnr != NULL){
            //
            //v4
            //
            jclass clsClt = jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
            if(clsClt != NULL){
                jmethodID mQuery = jEnv->GetMethodID(clsClt, "queryPurchasesAsync", "(Ljava/lang/String;Lcom/android/billingclient/api/PurchasesResponseListener;)V"); NBASSERT(mQuery != NULL)
                if(mQuery != NULL){
                    BOOL isReqInAppStarted = FALSE, isReqSubsStarted = FALSE;
                    SI32 i; for(i = 0; i < data->actions.refresh.skusSz; i++){
                        const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
                        if(!NBString_strIsEmpty(sku->prodId)){
                            BOOL addQuery = FALSE; const char* skyType = "";
                            if(sku->type == ENStoreProdType_InApp){
                                if(!isReqInAppStarted){
                                    addQuery    = TRUE;
                                    skyType     = "inapp"; //INAPP
                                    isReqInAppStarted = TRUE;
                                }
                            } else if(sku->type == ENStoreProdType_Subscription){
                                if(!isReqSubsStarted){
                                    addQuery    = TRUE;
                                    skyType     = "subs"; //SUBS
                                    isReqSubsStarted = TRUE;
                                }
                            }
                            //Add query
                            if(addQuery && !NBString_strIsEmpty(skyType)){
                                jstring jSkuType = jEnv->NewStringUTF(skyType);
                                PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache skuType requests starting('%s').\n", skyType);
                                {
                                    jEnv->CallVoidMethod(data->billingClient.jClt, mQuery, jSkuType, data->billingClient.jLstnr);
                                }
                                NBJNI_DELETE_REF_LOCAL(jEnv, jSkuType)
                            }
                        }
                    }
                    //Print results
                    PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache.\n");
                }
            }
            NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
            //
            //v3
            //
			/*jclass clsClt		= jEnv->FindClass("com/android/billingclient/api/BillingClient"); NBASSERT(clsClt != NULL)
			jclass clsResult	= jEnv->FindClass("com/android/billingclient/api/Purchase$PurchasesResult"); NBASSERT(clsResult != NULL)
			jclass clsList		= jEnv->FindClass("java/util/List"); NBASSERT(clsList != NULL)
			jclass clsPurchase	= jEnv->FindClass("com/android/billingclient/api/Purchase"); NBASSERT(clsPurchase != NULL)
			if(clsClt != NULL && clsResult != NULL && clsList != NULL && clsPurchase != NULL){
				jmethodID mQuery	= jEnv->GetMethodID(clsClt, "queryPurchases", "(Ljava/lang/String;)Lcom/android/billingclient/api/Purchase$PurchasesResult;"); NBASSERT(mQuery != NULL)
				jmethodID mGetCode	= jEnv->GetMethodID(clsResult, "getResponseCode", "()I"); NBASSERT(mGetCode != NULL)
				jmethodID mGetList	= jEnv->GetMethodID(clsResult, "getPurchasesList", "()Ljava/util/List;"); NBASSERT(mGetList != NULL)
				jmethodID mSize		= jEnv->GetMethodID(clsList, "size", "()I"); NBASSERT(mSize != NULL)
				jmethodID mGet		= jEnv->GetMethodID(clsList, "get", "(I)Ljava/lang/Object;"); NBASSERT(mGet != NULL)
				//
				jmethodID mGetSku	= jEnv->GetMethodID(clsPurchase, "getSku", "()Ljava/lang/String;"); NBASSERT(mGetSku != NULL)
				jmethodID mGetOrdId	= jEnv->GetMethodID(clsPurchase, "getOrderId", "()Ljava/lang/String;"); NBASSERT(mGetOrdId != NULL)
				jmethodID mGetToken	= jEnv->GetMethodID(clsPurchase, "getPurchaseToken", "()Ljava/lang/String;"); NBASSERT(mGetToken != NULL)
				jmethodID mGetState	= jEnv->GetMethodID(clsPurchase, "getPurchaseState", "()I"); NBASSERT(mGetState != NULL)
				jmethodID mIsAcknow	= jEnv->GetMethodID(clsPurchase, "isAcknowledged", "()Z"); NBASSERT(mIsAcknow != NULL)
				jmethodID mIsRenewg	= jEnv->GetMethodID(clsPurchase, "isAutoRenewing", "()Z"); NBASSERT(mIsRenewg != NULL)
				//
				if(mQuery != NULL && mGetCode != NULL && mGetList != NULL && mSize != NULL && mGet != NULL && mGetSku != NULL && mGetOrdId != NULL && mGetToken != NULL && mGetState != NULL && mIsAcknow != NULL && mIsRenewg != NULL){
					BOOL isReqInAppStarted = FALSE, isReqSubsStarted = FALSE;
					SI32 i; for(i = 0; i < data->actions.refresh.skusSz; i++){
						const STAppStoreProdId* sku = &data->actions.refresh.skus[i];
						if(!NBString_strIsEmpty(sku->prodId)){
							BOOL addQuery = FALSE; const char* skyType = "";
							if(sku->type == ENStoreProdType_InApp){
								if(!isReqInAppStarted){
									addQuery	= TRUE;
									skyType		= "inapp"; //INAPP
									isReqInAppStarted = TRUE;
								}
							} else if(sku->type == ENStoreProdType_Subscription){
								if(!isReqSubsStarted){
									addQuery	= TRUE;
									skyType		= "subs"; //SUBS
									isReqSubsStarted = TRUE;
								}
							}
							//Add query
							if(addQuery && !NBString_strIsEmpty(skyType)){
								jstring jSkuType = jEnv->NewStringUTF(skyType);
								PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache skuType requests starting('%s').\n", skyType);
								{
									jobject jResult = jEnv->CallObjectMethod(data->billingClient.jClt, mQuery, jSkuType);
									if(jResult != NULL){
										jint rCode = jEnv->CallIntMethod(jResult, mGetCode);
										jobject jList = jEnv->CallObjectMethod(jResult, mGetList);
										if(jList != NULL){
											jint size = jEnv->CallIntMethod(jList, mSize);
											PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache %d purchases returned.\n", size);
											jint i; for(i = 0; i < size; i++){
												jobject purch = jEnv->CallObjectMethod(jList, mGet, i);
												if(purch == NULL){
													PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache #%d purchase empty.\n", (i + 1));
												} else {
													const char* strSku		= "";
													const char* strOrderId	= "";
													const char* strToken	= "";
													jstring jSku			= (jstring)jEnv->CallObjectMethod(purch, mGetSku);
													jstring jOrderId		= (jstring)jEnv->CallObjectMethod(purch, mGetOrdId);
													jstring jToken			= (jstring)jEnv->CallObjectMethod(purch, mGetToken);
													const jint pState		= jEnv->CallIntMethod(purch, mGetState);
													const jboolean pIsAcknw	= jEnv->CallBooleanMethod(purch, mIsAcknow);
													const jboolean pIsRenew	= jEnv->CallBooleanMethod(purch, mIsRenewg);
													if(jSku != NULL)		strSku = jEnv->GetStringUTFChars(jSku, 0);
													if(jOrderId != NULL)	strOrderId = jEnv->GetStringUTFChars(jOrderId, 0);
													if(jToken != NULL)		strToken = jEnv->GetStringUTFChars(jToken, 0);
													//Process
													{
														PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache #%d/%d: sku('%s') orderId('%s') token('%s') state(%d, %s) isAcknowledged(%s) isAutoRenewing(%s).\n", (i + 1), size, strSku, strOrderId, strToken, pState, (pState == 0 ? "UNSPECIFIED" : pState == 1 ? "PURCHASED" : pState == 2 ? "PENDING" : "UNKNOWN_CODE"), (mIsAcknow ? "yes" : "no"), (pIsRenew ? "yes" : "no"));
														if(pState == 1){ //PURCHASED
															//Update or add token
															NBThreadMutex_lock(&data->purchases.mutex);
															{
																BOOL fnd = FALSE;
																//Seach in current values
																{
																	SI32 i = 0; for(i = 0; i < data->purchases.arr.use; i++){
																		STAppGlueAndroidStorePurchase* pur = NBArray_itmPtrAtIndex(&data->purchases.arr, STAppGlueAndroidStorePurchase, i);
																		if(NBString_strIsEqual(pur->uid.sku, strSku) && NBString_strIsEqual(pur->uid.token, strToken)){
																			//Update data values
																			{
																				pur->data.isSet			= TRUE;
																				{
																					if(pur->data.orderId != NULL) NBMemory_free(pur->data.orderId); pur->data.orderId = NULL;
																					if(!NBString_strIsEmpty(strOrderId)) pur->data.orderId = NBString_strNewBuffer(strOrderId);
																				}
																				pur->data.state				= pState;
																				pur->data.isAcknowledged	= pIsAcknw;
																				pur->data.isAutoRenewing	= pIsRenew;
																			}
																			//
																			data->purchases.pendSynced++;
																			PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache token already found.\n");
																			fnd = TRUE;
																			break;
																		}
																	}
																}
																//Add new value
																if(!fnd){
																	STAppGlueAndroidStorePurchase purN;
																	NBMemory_setZeroSt(purN, STAppGlueAndroidStorePurchase);
																	//UniqueId
																	{
																		purN.uid.sku		= NBString_strNewBuffer(strSku);
																		purN.uid.token		= NBString_strNewBuffer(strToken);
																	}
																	//Data
																	{
																		purN.data.isSet			= TRUE;
																		if(!NBString_strIsEmpty(strOrderId)){
																			purN.data.orderId	= NBString_strNewBuffer(strOrderId);
																		}
																		purN.data.state				= pState;
																		purN.data.isAcknowledged	= pIsAcknw;
																		purN.data.isAutoRenewing	= pIsRenew;
																	}
																	NBArray_addValue(&data->purchases.arr, purN);
																	//
																	data->purchases.pendAdded++;
																	PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache token added.\n");
																}
															}
															NBThreadMutex_unlock(&data->purchases.mutex);
														}
													}
													if(jSku != NULL && strSku != NULL) jEnv->ReleaseStringUTFChars(jSku, strSku);
													if(jOrderId != NULL && strOrderId != NULL) jEnv->ReleaseStringUTFChars(jOrderId, strOrderId);
													if(jToken != NULL && strToken != NULL) jEnv->ReleaseStringUTFChars(jToken, strToken);
												}
											}
										}
									}
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jSkuType)
							}
						}
					}
					//Build new tokens list
					{
						AUAppGlueAndroidStore_buildReceiptOfToken(data, TRUE);
					}
					//Print results
					PRINTF_INFO("AUAppGlueAndroidStore, doSyncWithPurchasesCache.\n");
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPurchase);
			NBJNI_DELETE_REF_LOCAL(jEnv, clsList);
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResult);
			NBJNI_DELETE_REF_LOCAL(jEnv, clsClt);
            */
		}
	}
}

//



void AUAppGlueAndroidStore_skuArrRelease(STAppStoreProdId* strs, const UI32 strsSz){
	if(strs != NULL){
		SI32 i; for(i = 0; i < strsSz; i++){
			if(strs[i].prodId != NULL) NBMemory_free(strs[i].prodId); strs[i].prodId = NULL;
			if(strs[i].grpId != NULL) NBMemory_free(strs[i].grpId); strs[i].grpId = NULL;
		}
		NBMemory_free(strs);
		strs = NULL;
	}
}

STAppStoreProdId* AUAppGlueAndroidStore_skuArrClone(UI32* dstStrsSz, const STAppStoreProdId* strs, const UI32 strsSz){
	STAppStoreProdId* r = NULL;
	UI32 rSz = 0;
	//Clone new
	if(strs != NULL && strsSz > 0){
		r = NBMemory_allocTypes(STAppStoreProdId, strsSz);
		{
			SI32 i; for(i = 0; i < strsSz; i++){
				//Clone
				if(!NBString_strIsEmpty(strs[i].prodId)){
					//Reset
					NBMemory_setZero(r[rSz]);
					//Set
					r[rSz].prodId			= NBString_strNewBuffer(strs[i].prodId);
					r[rSz].grpId			= NBString_strNewBuffer(strs[i].grpId);
					r[rSz].type				= strs[i].type;
					r[rSz].actions			= strs[i].actions;
					r[rSz].actionsSecsWait	= strs[i].actionsSecsWait;
					r[rSz].actionsSecsRetry	= strs[i].actionsSecsRetry;
					rSz++;
				}
			}
		}
	}
	if(dstStrsSz != NULL) *dstStrsSz = rSz;
	return r;
}


