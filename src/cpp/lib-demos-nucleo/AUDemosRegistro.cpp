//
//  IScenesListener.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUEscenaDemoTextRender.h"
#include "AUEscenaDemoTextBox.h"
#include "AUDemosRegistro.h"

UI16 AUEscenaDemoTextRender::idTipoClase;
UI16 AUEscenaDemoTextBox::idTipoClase;

#ifndef NB_METODO_INICIALIZADOR_CUERPO
#error "Falta inclusion. Aun no se ha definido la macro NB_METODO_INICIALIZADOR_CUERPO."
#endif

NB_METODO_INICIALIZADOR_CUERPO(AUDemosRegistrar){
	NBGestorAUObjetos::registrarClase("AUEscenaDemoTextRender", &AUEscenaDemoTextRender::idTipoClase);
	NBGestorAUObjetos::registrarClase("AUEscenaDemoTextBox", &AUEscenaDemoTextBox::idTipoClase);
};
