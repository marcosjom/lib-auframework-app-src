//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrOSTelephony_h
#define NBMngrOSTelephony_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrOSTelephony.h"

class NBMngrOSTelephony {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool	isGlued();
		static bool setGlue(AUAppI* app, PTRfuncOSTelephonyCreate initCall);
		//
		static ENTelephonyAuthStatus authStatus(const BOOL requestIfNecesary);
		//
		static SI32	getPhoneCount();	//sims supported by device
		static BOOL	getIMEI(const SI32 slot, STNBString* dst);
		static bool canMakeCalls();
		static bool	getCarrierCountryISO(STNBString* dst);
	private:
		static AUMngrOSTelephony* _instance;
};

#endif
