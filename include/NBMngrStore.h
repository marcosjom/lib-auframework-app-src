//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrStore_h
#define NBMngrStore_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrStore.h"

class NBMngrStore {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool		isGlued();
		static bool		setGlue(AUAppI* app, PTRfuncStoreCreate initCall);
		//
		static ENStoreResult storeState();
		static ENStoreResult curActionState();
		static void	addProdId(const char* prodId, const ENStoreProdType type, const char* grpId, const UI32 actions /*ENStorePurchaseActionBit*/, const float actionsSecsWait, const float actionsSecsRetry);
		static bool	startStoreSync();
		static bool startRestoringReceipt();
		static bool	startRestoringPurchases();
		static bool	startPurchase(const char* prodId);
		static bool	getProdProps(const char* prodId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDesc, AUCadenaMutable8* dstPrice, bool* dstOwned);
		static bool	isOwned(const char* prodId);
		//
		static BOOL	concatLocalReceiptPayload(STNBString* dst);
		//
		static bool	loadData();
		static bool	setProdData(const char* prodId, const bool owned);
		static bool	setProdData(const char* prodId, const char* name, const char* desc, const char* price);
		static bool	setProdData(const char* prodId, const char* name, const char* desc, const char* price, const bool owned);
		static bool	endStoreSync(const ENStoreResult result);
		static bool	endStoreAction(const ENStoreResult result);
		//
		static void	lock();
		static void	unlock();
		static bool	lockedLoadFromJSON(const char* json);
		static bool	lockedSaveToJSON(AUCadenaMutable8* dst);
	private:
		static AUMngrStore* _instance;
};

#endif
