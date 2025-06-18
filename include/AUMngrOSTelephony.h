//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrOSTelephony_h
#define AUMngrOSTelephony_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

typedef enum ENTelephonyAuthStatus_ {
	ENTelephonyAuthStatus_NotDetermined = 0,
	ENTelephonyAuthStatus_Authorized,
	ENTelephonyAuthStatus_Requesting,
	ENTelephonyAuthStatus_Denied,
	ENTelephonyAuthStatus_Restricted,
	//
	ENTelephonyAuthStatus_Count
} ENTelephonyAuthStatus;

typedef struct STMngrOSTelephonyCalls_ STMngrOSTelephonyCalls;

//Callbacks
typedef bool (*PTRfuncOSTelephonyCreate)(AUAppI* app, STMngrOSTelephonyCalls* obj);
typedef bool (*PTRfuncOSTelephonyDestroy)(void* obj);
//
typedef ENTelephonyAuthStatus (*PTRFuncOSTelephonyAuthStatus)(void* obj, const BOOL requestIfNecesary);
//
typedef SI32 (*PTRfuncOSTelephonyGetPhoneCount)(void* obj);
typedef BOOL (*PTRfuncOSTelephonyGetIMEI)(void* obj, const SI32 slot, STNBString* dst);
typedef bool (*PTRfuncOSTelephonyCanMakeCalls)(void* obj);
typedef bool (*PTRfuncOSTelephonyGetCarrierCountryISO)(void* obj, STNBString* dst);
//

typedef struct STMngrOSTelephonyCalls_ {
	PTRfuncOSTelephonyCreate				funcCreate;
	void*									funcCreateParam;
	PTRfuncOSTelephonyDestroy				funcDestroy;
	void*									funcDestroyParam;
	//
	PTRFuncOSTelephonyAuthStatus			funcAuthStatus;
	void*									funcAuthStatusParam;
	//
	PTRfuncOSTelephonyGetPhoneCount			funcGetPhoneCount;
	void*									funcGetPhoneCountParam;
	PTRfuncOSTelephonyGetIMEI				funcGetIMEI;
	void*									funcGetIMEIParam;
	PTRfuncOSTelephonyCanMakeCalls			funcCanMakeCalls;
	void*									funcCanMakeCallsParam;
	PTRfuncOSTelephonyGetCarrierCountryISO	funcGetCarrierCountryISO;
	void*									funcGetCarrierCountryISOParam;
} STMngrOSTelephonyCalls;

//

class AUMngrOSTelephony : public AUObjeto {
	public:
		AUMngrOSTelephony();
		virtual ~AUMngrOSTelephony();
		//
		static bool	isGlued();
		static bool	setGlue(AUAppI* app, PTRfuncOSTelephonyCreate initCall);
		//
		ENTelephonyAuthStatus authStatus(const BOOL requestIfNecesary);
		//
		SI32	getPhoneCount();	//sims supported by device
		BOOL	getIMEI(const SI32 slot, STNBString* dst);
		bool	canMakeCalls();
		bool	getCarrierCountryISO(STNBString* dst);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrOSTelephonyCalls _calls;
};

#endif
