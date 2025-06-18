//
//  IScenesListener.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AUEscenaDemoTextRender_h
#define AUEscenaDemoTextRender_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppEscena.h"

typedef enum ENEscenaDemoTextRenderMode_ {
	ENEscenaDemoTextRenderMode_textureFont = 0,		//Common pre-rendered texture font
	ENEscenaDemoTextRenderMode_rasterFont,			//Real time rasterizing font
	//ENEscenaDemoTextRenderMode_spritesText,		//Individual char-sprites text
	ENEscenaDemoTextRenderMode_multiFormatText		//multiple fonts in the same text
} ENEscenaDemoTextRenderMode;

class AUEscenaDemoTextRender : public AUAppEscena, public IEscuchadorCambioPuertoVision, public IEscuchadorTouchEscenaObjeto, public NBAnimador {
	public:
		AUEscenaDemoTextRender(const SI32 iEscena);
		virtual ~AUEscenaDemoTextRender();
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
		SI32						_iScene;
		bool						_enPrimerPlano;
		bool						_rotating;
		bool						_scaling;
		bool						_scaleGrowing;
		UI32						_iCurrentText;	//text content
		ENEscenaDemoTextRenderMode	_mode;
		AUEscenaContenedor*			_capaRaiz;
		AUEscenaSprite*				_fondo;
		AUEscenaContenedor*			_capaTextos;
		AUArregloMutable*			_textos;
		AUEscenaSprite*				_btnRotate;
		AUEscenaSprite*				_btnScale;
		AUEscenaSprite*				_btnChngTxt;
		//
		void						privOrganizarContenido();
		static const char*			privGetText(const UI32 iText, const UI32 fontSz, AUCadenaMutable8* dst);
		static void					privSetMultiformatText(const UI32 iText, const UI32 fontSz, AUCadenaMutable8* strTmp, AUEscenaTextoEditable* dst);
	
};

#endif
