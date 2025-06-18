//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppTransicionConFondo_h
#define AUAppTransicionConFondo_h

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicion.h"
#include "AUAppEscena.h"
#include "AUAppEscenasAdmin.h"

//Etapas de la tranaiscion de escena cambiando fondo
enum ENEtapaCargaFondo {
	ENEtapaCargaFondo_Ninguna = -1,
	ENEtapaCargaFondo_IniciarSalidaViejo = 0,
	ENEtapaCargaFondo_EsperarSalidaViejo,
	ENEtapaCargaFondo_RenderConEscenaFuera,
	ENEtapaCargaFondo_CargarNuevoFondo,
	ENEtapaCargaFondo_CambiarFondos,
	ENEtapaCargaFondo_QuitarExtras,
	ENEtapaCargaFondo_RenderFondoFinal,
	ENEtapaCargaFondo_CrearNuevoLiberarRecursosSinUso
};

struct STCargaFondo {
	ENEtapaCargaFondo		etapaCarga;
	bool					permitidoGirarPantallaAntesDeIniciarTransicion;
	AUEscenaContenedor*		fondoSalienteCapa;
	AUEscenaContenedor*		fondoEntranteCapa;
	AUEscenaSpritePorcion*	fondoSaliente;
	AUEscenaSpritePorcion*	fondoEntrante;
	AUTextura*				costuraTextura;
	AUEscenaFigura*			costuraSprite;
};

class AUAppTransicionConFondo: public AUAppTransicion, public IEscuchadorCambioPuertoVision {
	public:
		AUAppTransicionConFondo(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const ENGestorTexturaModo modoCargaTexturas, const char* fondoRutaActual, AUEscenaContenedor* fondoContenedorActual, AUEscenaSpritePorcion* fondoSaliente, const char* fondoRutaCargar, const char* costuraTexturaPatron);
		virtual ~AUAppTransicionConFondo();
		//
		virtual void		puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		virtual bool		ejecutandoTransicion();
		virtual bool		tickTransicion(float segsTranscurridos);
	protected:
		SI32				_iScene;
		AUAppEscenasAdmin*	_escuchador;
		ENGestorTexturaModo	_modoCargaTexturas;
		STCargaFondo		_datosCargaFondo;
		AUAnimadorObjetoEscena*	_animadorObjsEscena;
		AUCadenaMutable8	_fondoRutaActual;
		AUCadenaMutable8	_fondoRutaCargar;
		//
		void				privFinalizarDatos();
		void				privEscalarFondo(AUEscenaSpritePorcion* spriteFondo);
};

#endif
