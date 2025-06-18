//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUMngrBiometrics_h
#define AUMngrBiometrics_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppI.h"

typedef enum ENBiometricsType_ {
	ENBiometricsType_Unknown = 0,
	ENBiometricsType_Face,
	ENBiometricsType_Finger,
	//
	ENBiometricsType_Count
} ENBiometricsType;
	
typedef enum ENBiometricsAuthStatus_ {
	ENBiometricsAuthStatus_NotDetermined = 0,
	ENBiometricsAuthStatus_Authenticated,
	ENBiometricsAuthStatus_Requesting,
	ENBiometricsAuthStatus_Failed,
	//
	ENBiometricsAuthStatus_Count
} ENBiometricsAuthStatus;


typedef struct STMngrBiometricsCalls_ STMngrBiometricsCalls;

//Callbacks
typedef bool (*PTRfuncBiometricsCreate)(AUAppI* app, STMngrBiometricsCalls* obj);
typedef bool (*PTRfuncBiometricsDestroy)(void* obj);
//
typedef void (*PTRfuncBiometricsGetTypeName)(void* obj, const ENBiometricsType type, STNBString* dstName);
typedef BOOL (*PTRfuncBiometricsCanAuthenticate)(void* obj, const ENBiometricsType type, STNBString* dstError);
typedef BOOL (*PTRfuncBiometricsShowsOwnGUI)(void* obj);
typedef BOOL (*PTRfuncBiometricsStartAuthentication)(void* obj, const char* reasonTitle, const char* cancelTitle);
typedef void (*PTRfuncBiometricsCancelAuthentication)(void* obj);
typedef ENBiometricsAuthStatus (*PTRFuncBiometricsAuthStatus)(void* obj, const UI64 secsLastValid);
//

typedef struct STMngrBiometricsCalls_ {
	PTRfuncBiometricsCreate					funcCreate;
	void*									funcCreateParam;
	PTRfuncBiometricsDestroy				funcDestroy;
	void*									funcDestroyParam;
	//
	PTRfuncBiometricsGetTypeName			funcGetTypeName;
	void*									funcGetTypeNameParam;
	PTRfuncBiometricsCanAuthenticate		funcCanAuthenticate;
	void*									funcCanAuthenticateParam;
	PTRfuncBiometricsShowsOwnGUI			funcShowsOwnGui;
	void*									funcShowsOwnGuiParam;
	PTRfuncBiometricsStartAuthentication	funcStartAuthentication;
	void*									funcStartAuthenticationParam;
	PTRfuncBiometricsCancelAuthentication	funcCancelAuthentication;
	void*									funcCancelAuthenticationParam;
	PTRFuncBiometricsAuthStatus				funcAuthStatus;
	void*									funcAuthStatusParam;
} STMngrBiometricsCalls;

//

class AUMngrBiometrics : public AUObjeto {
	public:
		AUMngrBiometrics();
		virtual ~AUMngrBiometrics();
		//
		static bool	isGlued();
		static bool	setGlue(AUAppI* app, PTRfuncBiometricsCreate initCall);
		//
		void	getTypeName(const ENBiometricsType type, STNBString* dstName);
		BOOL	canAuthenticate(const ENBiometricsType type, STNBString* dstError);
		BOOL	showsOwnGui();
		BOOL	startAuthentication(const char* reasonTitle, const char* cancelTitle);
		void	cancelAuthentication();
		ENBiometricsAuthStatus authStatus(const UI64 secsLastValid);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		//
		static STMngrBiometricsCalls _calls;
};

#endif
