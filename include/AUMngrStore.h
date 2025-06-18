//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrStore_h
#define AUMngrStore_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"
#include "nb/core/NBMngrStoreDefs.h"

//Callbacks

typedef struct STMngrStoreCalls_ STMngrStoreCalls;

typedef bool (*PTRfuncStoreCreate)(AUAppI* app, STMngrStoreCalls* obj);
typedef bool (*PTRfuncStoreDestroy)(void* obj);
typedef bool (*PTRfuncStoreLoadData)(void* obj);
typedef bool (*PTRfuncStoreSaveData)(void* obj);
typedef bool (*PTRfuncStoreStartRefresh)(void* obj, const STAppStoreProdId* prodIds, const SI32 prodIdsCount);
typedef bool (*PTRfuncStoreStartPurchase)(void* obj, const char* prodId);
typedef bool (*PTRfuncStoreStartReceiptRefresh)(void* obj);
typedef bool (*PTRfuncStoreStartRestore)(void* obj);
typedef BOOL (*PTRfuncStoreConcatLocalReceiptPayload)(void* obj, STNBString* dst);

//

typedef struct STMngrStoreCalls_ {
	PTRfuncStoreCreate			funcCreate;
	void*						funcCreateParam;
	PTRfuncStoreDestroy			funcDestroy;
	void*						funcDestroyParam;
	//
	PTRfuncStoreConcatLocalReceiptPayload funcConcatLocalReceipt;
	void*						funcConcatLocalReceiptParam;
	PTRfuncStoreLoadData		funcLoadData;
	void*						funcLoadDataParam;
	PTRfuncStoreSaveData		funcSaveData;
	void*						funcSaveDataParam;
	//
	PTRfuncStoreStartRefresh	funcStartRefresh;
	void*						funcStartRefreshParam;
	PTRfuncStoreStartPurchase	funcStartPruchase;
	void*						funcStartPruchaseParam;
	PTRfuncStoreStartReceiptRefresh	funcStartReceiptRefresh;
	void*						funcStartReceiptRefreshParam;
	PTRfuncStoreStartRestore	funcStartRestore;
	void*						funcStartRestoreParam;
} STMngrStoreCalls;

//
	
typedef struct STAppStoreProd_ {
	AUCadena8*			prodId;
	ENStoreProdType		type;
	AUCadena8*			grpId;
	UI32				actions; //ENStorePurchaseActionBit
	float				actionsSecsWait;	//Wait before applying actions
	float				actionsSecsRetry;	//Wait before retrying applying actions after failure
	AUCadenaMutable8*	name;
	AUCadenaMutable8*	desc;
	AUCadenaMutable8*	price;
	bool				owned;
} STAppStoreProd;

class AUMngrStore : public AUObjeto {
	public:
		AUMngrStore();
		virtual ~AUMngrStore();
		//
		static bool		isGlued();
		static bool		setGlue(AUAppI* app, PTRfuncStoreCreate initCall);
		//
		ENStoreResult storeState() const;
		ENStoreResult curActionState() const;
		void	addProdId(const char* prodId, const ENStoreProdType type, const char* grpId, const UI32 actions /*ENStorePurchaseActionBit*/, const float actionsSecsWait, const float actionsSecsRetry);
		bool	startStoreSync();
		bool	startRestoringReceipt();
		bool	startRestoringPurchases();
		bool	startPurchase(const char* prodId);
		bool	getProdProps(const char* prodId, AUCadenaMutable8* dstName, AUCadenaMutable8* dstDesc, AUCadenaMutable8* dstPrice, bool* dstOwned);
		bool	isOwned(const char* prodId);
		//
		BOOL	concatLocalReceiptPayload(STNBString* dst);
		bool	loadData();
		bool	setProdData(const char* prodId, const bool owned);
		bool	setProdData(const char* prodId, const char* name, const char* desc, const char* price);
		bool	setProdData(const char* prodId, const char* name, const char* desc, const char* price, const bool owned);
		bool	endStoreSync(const ENStoreResult result);
		bool	endStoreAction(const ENStoreResult result);
		//
		void	lock();
		void	unlock();
		bool	lockedLoadFromJSON(const char* json);
		bool	lockedSaveToJSON(AUCadenaMutable8* dst) const;
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrStoreCalls _calls;
		//
		NBHILO_MUTEX_CLASE		_mutex;
		SI32					_mutexLocksCount;	//Depth of calling ->lock() and unlock().
		//
		ENStoreResult			_storeState;
		ENStoreResult			_actionState;
		AUCadena8*				_actionProdId;		//NULL if actions is 'restorePurchases', NOT NULL if action is 'purchasing'
		AUArregloNativoMutableP<STAppStoreProd> _storeProds;
};

#endif
