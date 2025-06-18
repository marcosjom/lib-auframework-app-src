//
//  AUAppEscenasAdmin.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppEscenasAdmin_h
#define AUAppEscenasAdmin_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppEscena.h"

typedef enum ENAppEscenasFondoModo_ {
	ENAppEscenasFondoModo_estirar = 0,	//estira y deforma la imagen
	ENAppEscenasFondoModo_ajustar		//estira sin deformar la imagen, cortando el excedente
} ENAppEscenasFondoModo;

typedef struct STAppScenasPkgLoadDef_ {
	const char* filePath;	//file path to load
	const BYTE*	data;		//persistent data (optional)
	const UI64	dataSz;		//persistent dataSz (optional)
} STAppScenasPkgLoadDef;

class AUAppEscenasAdmin: public AUObjeto, public IEscuchadorCambioPuertoVision {
	public:
		AUAppEscenasAdmin(const SI32 iEscena, const char* prefijoPaquetes, const STAppScenasPkgLoadDef* paquetesCargar, const SI32 conteoPaquetesCargar);
		virtual ~AUAppEscenasAdmin();
		//
		virtual bool	permitidoGirarPantalla() = 0;
		virtual void	puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after);
		//
		virtual bool	tickProcesoCarga(float segsTranscurridos) = 0;
		//TECLAS
		virtual bool	teclaPresionada(SI32 codigoTecla) = 0;
		virtual bool	teclaLevantada(SI32 codigoTecla) = 0;
		virtual bool	teclaEspecialPresionada(SI32 codigoTecla) = 0;
		//
		virtual void	escenasCargar() = 0;
		virtual void	escenasLiberar() = 0;
		virtual void	escenasLiberarRecursosSinUso() = 0;
		//
		virtual bool	escenasEnProcesoDeCarga() = 0;
		virtual bool	escenasTieneAccionesPendiente() = 0;
		virtual void	escenasEjecutaAccionesPendientes() = 0;
		//
		virtual void	escenasQuitarDePrimerPlano() = 0;
		virtual void	escenasColocarEnPrimerPlano() = 0;
		//
		virtual bool	escenasAnimandoSalida() = 0;
		virtual void	escenasAnimarSalida() = 0;
		virtual void	escenasAnimarEntrada() = 0;
		//
		void			escenasHeredarFondo(const char* rutaTextura, AUEscenaContenedor* contenedorFondo, AUEscenaSpritePorcion* spriteFondo, const ENAppEscenasFondoModo tipoFondo);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	protected:
		SI32					_iScene;
		//
		ENAppEscenasFondoModo	_fondoModo;
		AUCadenaMutable8		_fondoRuta;
		AUEscenaContenedor*		_fondoCapa;
		AUEscenaSpritePorcion*	_fondoSprite;
	private:
		//
		void					privLiberarRecursosSinUso();
		void					privEscalarFondo(AUEscenaSpritePorcion* spriteFondo);
};

#endif
