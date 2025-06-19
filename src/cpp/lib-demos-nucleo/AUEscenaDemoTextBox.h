//
//  IScenesListener.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AUEscenaDemoTextBox_h
#define AUEscenaDemoTextBox_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppEscena.h"

class AUEscenaDemoTextBox : public AUAppEscena, public IEscuchadorCambioPuertoVision, public IEscuchadorTouchEscenaObjeto, public NBAnimador {
	public:
		AUEscenaDemoTextBox(const SI32 iEscena);
		virtual ~AUEscenaDemoTextBox();
		//
		void puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		void tickAnimacion(float segsTranscurridos);
		//
		void touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto);
		void touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto);
		void touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto);
		//
		bool escenaEnPrimerPlano();
		void escenaColocarEnPrimerPlano();
		void escenaQuitarDePrimerPlano();
        //
        void escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate);
		//
		bool escenaEstaAnimandoSalida();
		void escenaAnimarSalida();
		void escenaAnimarEntrada();
		bool escenaPermitidoGirarPantalla();
		//TECLAS
		bool teclaPresionada(SI32 codigoTecla);
		bool teclaLevantada(SI32 codigoTecla);
		bool teclaEspecialPresionada(SI32 codigoTecla);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		bool				_enPrimerPlano;
		SI32				_iScene;
		AUEscenaContenedor*	_capaRaiz;
		AUEscenaSprite*		_fondo;
		//
		AUEscenaContenedor*	_textboxsLayer;
		AUArregloMutable	_textboxs;
		//
		void				privOrganizarContenido();
		void				privScrollV(const float delta);
	
};

#endif
