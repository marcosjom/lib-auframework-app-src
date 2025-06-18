//
//  AUAppEscena.cpp
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppEscena.h"

AUAppEscena::AUAppEscena() : AUObjeto() {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscena::AUAppEscena")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppEscena")
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUAppEscena::~AUAppEscena(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscena::~AUAppEscena")
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUAppEscena)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUAppEscena, "AUAppEscena")
AUOBJMETODOS_CLONAR_NULL(AUAppEscena)






