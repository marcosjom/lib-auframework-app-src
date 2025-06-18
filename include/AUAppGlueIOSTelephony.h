//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSTelephony_H
#define AUAppGlueIOSTelephony_H

#include "AUMngrOSTelephony.h"

class AUAppGlueIOSTelephony {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrOSTelephonyCalls* obj);
		static bool destroy(void* data);
		//
		static ENTelephonyAuthStatus authStatus(void* data, const BOOL requestIfNecesary);
		//
		static SI32 getPhoneCount(void* data);	//sims supported by device
		static BOOL getIMEI(void* data, const SI32 slot, STNBString* dst);
		static bool canMakeCalls(void* data);
		static bool	getCarrierCountryISO(void* data, STNBString* dst);
};

#endif
