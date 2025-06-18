//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppTransicion_h
#define AUAppTransicion_h

#include "AUAppNucleoPrecompilado.h"
#include "AUAppNucleoEncabezado.h"

class AUAppTransicion : public AUObjeto {
	public:
		AUAppTransicion();
		virtual ~AUAppTransicion();
		//
		virtual bool	ejecutandoTransicion() = 0;
		virtual bool	tickTransicion(float segsTranscurridos) = 0;
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
};

#endif
