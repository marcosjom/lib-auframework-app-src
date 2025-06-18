//
//  AUAppGlueAndroidStore.h
//
//

#ifndef AUAppGlueAndroidStore_H
#define AUAppGlueAndroidStore_H

#include "AUMngrStore.h"
#include "AUAppGlueAndroidJNI.h"

class AUAppGlueAndroidStore {
	public:
		//Old Billing client
		/*static bool	analyzeServiceConnected(void* data, void* jCompName / *jobject* /, void* jBinder / *jobject* /);
		static bool	analyzeServiceDisconnected(void* data, void* jCompName / *jobject* /);
		static bool	analyzeActivityResult(void* data, const SI32 reqCode, const SI32 resultCode, void* jIntent / *jobject* /);
		static bool	bindService(void* data);
		static void	unbindService(void* data);*/
		//New Billing client
		static void onBillingSetupFinished(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* dataPtr);
		static void onBillingServiceDisconnected(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* dataPtr);
		static void onSkuDetailsResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* skuDetailsList /*jobject*/, void* dataPtr);
		static void onPurchasesUpdated(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* purchases /*jobject*/, void* dataPtr);
		static void onAcknowledgePurchaseResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* dataPtr);
		static void onConsumeResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* purchaseToken /*jstring*/, void* dataPtr);
		static void onPurchaseHistoryResponse(void* pEnv /*JNIEnv*/, void* pObj /*jobject*/, void* billingResult /*jobject*/, void* purchaseHistoryRecordList /*jobject*/, void* dataPtr);
		//Callbacks
		static bool create(AUAppI* app, STMngrStoreCalls* obj);
		static bool destroy(void* data);
		static BOOL concatLocalReceiptPayload(void* data, STNBString* dst);
		static bool loadData(void* data);
		static bool saveData(void* data);
		static bool startRefresh(void* data, const STAppStoreProdId* prodIds, const SI32 prodIdsCount);
		static bool startPurchase(void* data, const char* prodId);
		static bool startReceiptRefresh(void* data);
		static bool startRestore(void* data);
};

#endif
