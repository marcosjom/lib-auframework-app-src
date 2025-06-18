//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSBiometrics_H
#define AUAppGlueIOSBiometrics_H

#include "AUMngrBiometrics.h"

class AUAppGlueIOSBiometrics {
	public:
		//Calls
		static bool create(AUAppI* app, STMngrBiometricsCalls* obj);
		static bool destroy(void* data);
		//
		static void	getTypeName(void* data, const ENBiometricsType type, STNBString* dstName);
		static BOOL	canAuthenticate(void* data, const ENBiometricsType type, STNBString* dstError);
		static BOOL	showsOwnGui(void* data);
		static BOOL	startAuthentication(void* data, const char* reasonTitle, const char* cancelTitle);
		static void	cancelAuthentication(void* data);
		static ENBiometricsAuthStatus authStatus(void* data, const UI64 secsLastValid);
};

#endif
