//
//  AUAppEscenasAdminSimple.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef AUAppEscenasAdminSimple_h
#define AUAppEscenasAdminSimple_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppEscenasAdmin.h"

class AUAppEscenasAdminSimple: public AUAppEscenasAdmin {
	public:
		AUAppEscenasAdminSimple(const SI32 iEscena, const ENGestorTexturaModo modoCargaTexturas, const char* prefijoPaquetes, const STAppScenasPkgLoadDef* paquetesCargar, const SI32 conteoPaquetesCargar);
		virtual ~AUAppEscenasAdminSimple();
		//
		bool	permitidoGirarPantalla();
		//
		bool	tickProcesoCarga(float segsTranscurridos);
		//TECLAS
		bool	teclaPresionada(SI32 codigoTecla);
		bool	teclaLevantada(SI32 codigoTecla);
		bool	teclaEspecialPresionada(SI32 codigoTecla);
		//
		void	escenaCargar(AUAppEscena* objEscena);
		void	escenasCargar();
		void	escenasLiberar();
		void	escenasLiberarRecursosSinUso();
		//
		bool	escenasEnProcesoDeCarga();
		bool	escenasTieneAccionesPendiente();
		void	escenasEjecutaAccionesPendientes();
		//
		void	escenasQuitarDePrimerPlano();
		void	escenasColocarEnPrimerPlano();
		//
		bool	escenasAnimandoSalida();
		void	escenasAnimarSalida();
		void	escenasAnimarEntrada();
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	protected:
		AUAppEscena*		_escenaActiva;
		AUAppEscena*		_escenaCargar;
		ENGestorTexturaModo	_modoCargaTexturas;
};

#endif
