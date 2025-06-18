//
//  IScenesListener.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppEscena_h
#define AUAppEscena_h

#include "AUAppNucleoEncabezado.h"
#include "NBMngrOSTools.h"

#define AU_TECLA_REGRESAR	0xFFFF01
#define AU_TECLA_MENU		0xFFFF02

class AUAppEscena : public AUObjeto {
	public:
		AUAppEscena();
		virtual ~AUAppEscena();
		//
		virtual bool escenaEnPrimerPlano() = 0;
		virtual void escenaColocarEnPrimerPlano() = 0;
		virtual void escenaQuitarDePrimerPlano() = 0;
		//Orientations
		virtual void escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate) = 0;
		//
		virtual bool escenaEstaAnimandoSalida() = 0;
		virtual void escenaAnimarSalida() = 0;
		virtual void escenaAnimarEntrada() = 0;
		virtual bool escenaPermitidoGirarPantalla() = 0;
		//TECLAS
		virtual bool teclaPresionada(SI32 codigoTecla) = 0;
		virtual bool teclaLevantada(SI32 codigoTecla) = 0;
		virtual bool teclaEspecialPresionada(SI32 codigoTecla) = 0;
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
};

#endif
