//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppTransicionConCortina_h
#define AUAppTransicionConCortina_h

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicion.h"
#include "AUAppEscenasAdmin.h"

//Etapas de la transicion de escena con color de fondo solido
typedef enum ENEtapaCargaCortina_ {
	ENEtapaCargaCortina_Ninguna = -1,
	ENEtapaCargaCortina_AparecerIni = 0,
	ENEtapaCargaCortina_Aparecer,
	ENEtapaCargaCortina_LiberarViejo,
	ENEtapaCargaCortina_CrearNuevoLiberarRecursosSinUso,
	ENEtapaCargaCortina_Desaparecer,
	ENEtapaCargaCortina_DesaparecerFin
} ENEtapaCargaCortina;

typedef struct STCargaCortina_ {
	ENEtapaCargaCortina		etapaCarga;
	bool					animarEntrada;
	bool					permitidoGirarPantallaAntesDeIniciarTransicion;
	float					alphaSegundosCambiando;
	float					alphaSegundosCambiar;
	float					alphaOrigen;
	float					alphaDestino;
	AUEscenaContenedor*		cortinaCapa;
	AUEscenaFigura*			cortina;
} STCargaCortina;

class AUAppTransicionConCortina: public AUAppTransicion, public IEscuchadorCambioPuertoVision {
	public:
		AUAppTransicionConCortina(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const ENGestorTexturaModo modoCargaTexturas, const bool animarEntrada, const NBColor8 colorVertLeftBottom, const NBColor8 colorVertRightBottom, const NBColor8 colorVertRightTop, const NBColor8 colorVertLeftTop);
		virtual ~AUAppTransicionConCortina();
		//
		virtual void		puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		virtual bool		ejecutandoTransicion();
		virtual bool		tickTransicion(float segsTranscurridos);
	protected:
		SI32				_iScene;
		AUAppEscenasAdmin*	_escuchador;
		ENGestorTexturaModo	_modoCargaTexturas;
		STCargaCortina		_datosCargaCortina;
};

#endif
