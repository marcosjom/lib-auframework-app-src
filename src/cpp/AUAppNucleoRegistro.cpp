//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppNucleo.h"
#include "AUAppNucleoRegistro.h"
#ifdef __ANDROID__
#include "AUAppGlueAndroidFiles.h" //for AUArchivoFisicoAPK class
#endif
//ToDo:remove
/*#include "AUMngrOSTools.h"
#include "AUMngrOSSecure.h"
#include "AUMngrNotifs.h"
#include "AUMngrStore.h"
#include "AUMngrOSTelephony.h"
#include "AUMngrBiometrics.h"
#include "AUMngrGameKit.h"
#include "AUMngrFbLogin.h"
#include "AUMngrGoogleLogin.h"*/

UI16 AUAppEscena::idTipoClase;
UI16 AUAppEscenasAdmin::idTipoClase;
UI16 AUAppEscenasAdminSimple::idTipoClase;
UI16 AUAppTransicion::idTipoClase;
UI16 AUEscenaResumenDebug::idTipoClase;
#ifdef __ANDROID__
UI16 AUArchivoFisicoAPK::idTipoClase;
#endif
UI16 AUMngrOSTools::idTipoClase;
UI16 AUMngrOSSecure::idTipoClase;
UI16 AUMngrNotifs::idTipoClase;
UI16 AUMngrAVCapture::idTipoClase;
UI16 AUMngrStore::idTipoClase;
UI16 AUMngrOSTelephony::idTipoClase;
UI16 AUMngrBiometrics::idTipoClase;
UI16 AUMngrContacts::idTipoClase;
UI16 AUMngrGameKit::idTipoClase;
UI16 AUMngrPdfKit::idTipoClase;
UI16 AUMngrFbLogin::idTipoClase;
UI16 AUMngrGoogleLogin::idTipoClase;

#ifndef NB_METODO_INICIALIZADOR_CUERPO
#error "Falta inclusion. Aun no se ha definido la macro NB_METODO_INICIALIZADOR_CUERPO."
#endif

NB_METODO_INICIALIZADOR_CUERPO(AUAppNucleoRegistrar){
	//printf("\n\n+++++++++++++ AUAppNucleoRegistrar +++++++++++++++\n\n");
	NBGestorAUObjetos::registrarClase("AUAppEscena", &AUAppEscena::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUAppEscenasAdmin", &AUAppEscenasAdmin::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUAppEscenasAdminSimple", &AUAppEscenasAdminSimple::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUAppTransicion", &AUAppTransicion::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUEscenaResumenDebug", &AUEscenaResumenDebug::idTipoClase);
#	ifdef __ANDROID__
	NBGestorAUObjetos::registrarClase("AUArchivoFisicoAPK", &AUArchivoFisicoAPK::idTipoClase);
#	endif
	NBGestorAUObjetos::registrarClase("AUMngrOSTools", &AUMngrOSTools::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrOSSecure", &AUMngrOSSecure::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrNotifs", &AUMngrNotifs::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrAVCapture", &AUMngrAVCapture::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrStore", &AUMngrStore::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrOSTelephony", &AUMngrOSTelephony::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrBiometrics", &AUMngrBiometrics::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrContacts", &AUMngrContacts::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrGameKit", &AUMngrGameKit::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrPdfKit", &AUMngrPdfKit::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrFbLogin", &AUMngrFbLogin::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUMngrGoogleLogin", &AUMngrGoogleLogin::idTipoClase);
};

  
