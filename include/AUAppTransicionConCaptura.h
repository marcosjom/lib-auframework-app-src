//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppTransicionConCaptura_h
#define AUAppTransicionConCaptura_h

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicion.h"
#include "AUAppEscenasAdmin.h"

//Etapas de la transicion de escena con captura
enum ENEtapaCargaCaptura {
	ENEtapaCargaCaptura_ninguna						= -1,
	ENEtapaCargaCaptura_capturarEscenaSaliente		= 0,
	ENEtapaCargaCaptura_esperarCapturaEscenaSaliente,
	ENEtapaCargaCaptura_animarSalidaEscenaSaliente_Alpha,
	ENEtapaCargaCaptura_animarSalidaEscenaSaliente,
	ENEtapaCargaCaptura_previoCrearNuevo,					//Para que se actualice la escena antes de cargar
	ENEtapaCargaCaptura_crearNuevo,
	ENEtapaCargaCaptura_previoLiberarRecursosSinUso,
	ENEtapaCargaCaptura_liberarRecursosSinUso,
	ENEtapaCargaCaptura_cargarRecursosPendientes,
	ENEtapaCargaCaptura_generarMipmaps,
	ENEtapaCargaCaptura_capturarEscenaNueva,
	ENEtapaCargaCaptura_esperarCapturaEscenaNueva,
	ENEtapaCargaCaptura_animarEntradaEscenaNueva_Alpha,
	ENEtapaCargaCaptura_animarEntradaEscenaNueva,
	ENEtapaCargaCaptura_animarSalidaEscenaNueva_Alpha
};

//Propiedades de la carga de escena con captura
struct STCargaCaptura {
	ENEtapaCargaCaptura		etapaCarga;
	bool					permitidoGirarPantallaAntesDeIniciarTransicion;
	SI32					indiceMarcoEnUso;
	AUEscenaCuerpo*			marcoCleanCorp;
	AUEscenaCuerpo*			marcoAlaNica;
	UI8						iAmbitoCargaTexturas;
	SI32					iCapturaEscenaSaliente;
	SI32					iCapturaEscenaEntrante;
	NBTamano				dimensionesCapturas;
	AUEscenaContenedor*		contenedorCapturaSaliente;
	AUEscenaContenedor*		contenedorCapturaEntrante;
	AUEscenaSpriteMascara*	spriteCapturaSaliente;
	AUEscenaSpriteMascara*	spriteCapturaEntrante;
	UI32					secuencialRenderEspera;
};

class AUAppTransicionConCaptura: public AUAppTransicion, public IEscuchadorCambioPuertoVision {
	public:
		AUAppTransicionConCaptura(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const ENGestorTexturaModo modoCargaTexturas);
		virtual ~AUAppTransicionConCaptura();
		//
		virtual void		puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		virtual bool		ejecutandoTransicion();
		virtual bool		tickTransicion(float segsTranscurridos);
	protected:
		SI32				_iScene;
		AUAppEscenasAdmin*	_escuchador;
		ENGestorTexturaModo	_modoCargaTexturas;
		STCargaCaptura		_datosCargaCaptura;
		AUAnimadorObjetoEscena*	_animadorObjsEscena;
		//ICONOS DE CARGA
		AUEscenaContenedor*	_contenedorCarga;
		AUEscenaSprite*		_spriteIconoEtapaCarga[4];
		AUEscenaSpriteElastico*	_barraProgresoMarco;
		AUEscenaSpriteElastico*	_barraProgresoRelleno;
		float				_progresoRelativo;
		SI32				_iconoCargaAnimandoIndice;	//animacion de icono decarga
		bool				_iconoCargaIncrementando;	//animacion de icono decarga
		UI32				_cargaConteoRecursos;
		//
		void				privOrganizarElementos();
		UI8					privCargarRecursosTransicionEscenas(SI32 indiceTransicion);
		void				privLiberarRecursosTrasicionEscenas();
		SI32				privCargarRecursoPendiente();
		inline void			privInlineIconosMotorInicializarEstado(SI32 indiceEtapaCarga);
		inline void			privInlineIconosMotorMarcarProgresoCarga(float progesoCeroHaciaUno);
		inline void			privInlineIconosMotorTickAnimacion(float segsTranscurridos);
};

#endif

