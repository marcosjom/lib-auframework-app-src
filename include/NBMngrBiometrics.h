//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrBiometrics_h
#define NBMngrBiometrics_h

#include "AUAppNucleoEncabezado.h"
#include "AUMngrBiometrics.h"

class NBMngrBiometrics {
	public:
		static void init();
		static void finish();
		static bool isInited();
		//
		static bool	isGlued();
		static bool setGlue(AUAppI* app, PTRfuncBiometricsCreate initCall);
		//
		static void getTypeName(const ENBiometricsType type, STNBString* dstName);
		static BOOL	canAuthenticate(const ENBiometricsType type, STNBString* dstError);
		static BOOL	showsOwnGui();
		static BOOL	startAuthentication(const char* reasonTitle, const char* cancelTitle);
		static void	cancelAuthentication();
		static ENBiometricsAuthStatus authStatus(const UI64 secsLastValid);
	private:
		static AUMngrBiometrics* _instance;
};

#endif
