//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppTransicionConColorSolido_h
#define AUAppTransicionConColorSolido_h

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicion.h"
#include "AUAppEscenasAdmin.h"

//Etapas de la transicion de escena con color de fondo solido
typedef enum ENEtapaCargaSolido_ {
	ENEtapaCargaSolido_Ninguna = -1,
	ENEtapaCargaSolido_IniciarSalidaViejo = 0,
	ENEtapaCargaSolido_EsperarSalidaViejo,
	ENEtapaCargaSolido_OscurecerIni,
	ENEtapaCargaSolido_Oscurecer,
	ENEtapaCargaSolido_LiberarViejo,
	ENEtapaCargaSolido_CrearNuevoLiberarRecursosSinUso,
	ENEtapaCargaSolido_Aclarar,
	ENEtapaCargaSolido_AclararFin
} ENEtapaCargaSolido;

typedef struct STCargaNegro_ {
	ENEtapaCargaSolido		etapaCarga;
	float					secsCur;
	float					secsDur;
	STNBColor8				colorUsar;
	bool					permitidoGirarPantallaAntesDeIniciarTransicion;
	AUEscenaContenedor*		layer;
	AUEscenaSprite*			glass;
} STCargaNegro;

class AUAppTransicionConColorSolido: public AUAppTransicion, public IEscuchadorCambioPuertoVision {
	public:
		AUAppTransicionConColorSolido(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const float secsDur, const STNBColor8 colorUsar, const ENGestorTexturaModo modoCargaTexturas);
		virtual ~AUAppTransicionConColorSolido();
		//
		virtual void		puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		virtual bool		ejecutandoTransicion();
		virtual bool		tickTransicion(float segsTranscurridos);
	protected:
		SI32				_iScene;
		AUAppEscenasAdmin*	_escuchador;
		ENGestorTexturaModo	_modoCargaTexturas;
		STCargaNegro		_datosCargaNegro;
};

#endif
