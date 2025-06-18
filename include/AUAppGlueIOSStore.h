//
//  AUAppGlueIOSStore.h
//
//

#ifndef AUAppGlueIOSStore_H
#define AUAppGlueIOSStore_H

#include "AUMngrStore.h"

class AUAppGlueIOSStore {
	public:
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
