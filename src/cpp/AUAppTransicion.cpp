//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicion.h"


AUAppTransicion::AUAppTransicion() : AUObjeto() {
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppTransicion")
}

AUAppTransicion::~AUAppTransicion(){
	//
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUAppTransicion)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUAppTransicion, "AUAppTransicion")
AUOBJMETODOS_CLONAR_NULL(AUAppTransicion)
