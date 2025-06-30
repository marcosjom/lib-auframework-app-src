//
//  AUEscenaResumenDebug.h
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 04/05/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef AUAPPESCENASRESUMENDEBUG_H_INCLUIDO
#define AUAPPESCENASRESUMENDEBUG_H_INCLUIDO

#define AUESCENARESUMENDEBUG_SEGUNDOS_ACUMULAR_PARA_PRESENTAR_GRAFICO_MEMORIA	3

#include "AUAppNucleoEncabezado.h"

enum ENResumenDebugIcono {
	ENResumenDebugIcono_CPU = 0,			//Uso de procesador
	ENResumenDebugIcono_APS,				//Actualizaciones de logica por segundo
	ENResumenDebugIcono_FPS,				//Renderizados por segundos
	ENResumenDebugIcono_Separador0,
	#ifdef CONFIG_NB_RECOPILAR_ESTADISTICAS_DE_GESTION_MEMORIA
	ENResumenDebugIcono_PunterosMemoria,	//Conteo de punteros
	ENResumenDebugIcono_MovimientoMemoria,	//Movimiento de memoria
	ENResumenDebugIcono_RamSis,				//Uso de memoria del sistema
	ENResumenDebugIcono_RamVid,				//Uso de memoria de video (OpenGL)
	ENResumenDebugIcono_Separador1,
	#endif
	//ENResumenDebugIcono_cargaAnimacion,	//Carga de trabajo de animar
	ENResumenDebugIcono_Separador2,
	//
	ENResumenDebugIcono_Conteo
};

struct STCicloRango {
	CICLOS_CPU_TIPO cicloIni;
	CICLOS_CPU_TIPO cicloFin;
	bool operator==(const STCicloRango &otro) const {
		return (cicloIni==otro.cicloIni && cicloFin==otro.cicloFin);
	}
	bool operator!=(const STCicloRango &otro) const {
		return !(cicloIni==otro.cicloIni && cicloFin==otro.cicloFin);
	}
};

struct STZonaInfoVisual {
	void*			primerByteZona;
	ENMemoriaTipo	tipoMemoriaZona;
	UI32			bytesTotalIndice;
	UI32			bytesTotalDatos;
	float			porcentajeUsoIndice;
	float			porcentajeUsoDatos;
	float			porcentajesUso[10];
	//COMPARACIONES CON REGISTROS
	bool operator==(const STZonaInfoVisual &otroRegistro) const {
		return (primerByteZona==otroRegistro.primerByteZona);
	}
	bool operator!=(const STZonaInfoVisual &otroRegistro) const {
		return (primerByteZona!=otroRegistro.primerByteZona);
	}
	bool operator<(const STZonaInfoVisual &otroRegistro) const {
		return (primerByteZona<otroRegistro.primerByteZona);
	}
	bool operator<=(const STZonaInfoVisual &otroRegistro) const {
		return (primerByteZona<=otroRegistro.primerByteZona);
	}
	bool operator>(const STZonaInfoVisual &otroRegistro) const {
		return (primerByteZona>otroRegistro.primerByteZona);
	}
	bool operator>=(const STZonaInfoVisual &otroRegistro) const {
		return (primerByteZona>=otroRegistro.primerByteZona);
	}
};

struct STCicloInfoVisual {
	SI32	indiceBarra;
	float	xPos;
	float	ancho;
	bool operator==(const STCicloInfoVisual &otro) const {
		return (indiceBarra==otro.indiceBarra && xPos==otro.xPos && ancho==otro.ancho);
	}
	bool operator!=(const STCicloInfoVisual &otro) const {
		return !(indiceBarra==otro.indiceBarra && xPos==otro.xPos && ancho==otro.ancho);
	}
};

struct STCicloAcumulados {
	UI32 conteoCiclosAnim;
	UI32 ciclosAnimacion;
	UI32 ciclosActMatrices;
	UI32 ciclosActVertsGL;
	UI32 ciclosFisicaLiquidos;
	UI32 ciclosFisicaSolidos;
	UI32 ciclosFisicaUniones;
	UI32 ciclosAnimando;
	//
	UI32 conteoCiclosRender;
	UI32 ciclosEnvCmdsGL;
	//
	UI32 conteoCiclosvolcado;
	UI32 ciclosVolcandoEscena;
};

struct STResumenDebugRender {
	STBloqueGL	bloqueIndicesGL;
};

class AUEscenaResumenDebug : public AUEscenaContenedor, public NBAnimador {
	public:
		AUEscenaResumenDebug(const SI32 iEscena);
		virtual ~AUEscenaResumenDebug();
		//
		void						acumularCiclosAnimacion(const NBEstadoCicloAnimacion &ciclosPorEstado);
		void						acumularCiclosRender(const NBEstadoCicloRender &ciclosPorEstado);
		void						acumularCiclosVolcado(const NBEstadoCicloVolcado &ciclosPorEstado);
		void						tickSegundoRecopilarInformacion(CICLOS_CPU_TIPO cicloActualDeProceso, bool actualizarDatosTexto, bool actualizarGraficos, bool imprimirEnConsola);
		//
		virtual PTRfunCmdsGL		actualizarModeloGL(const NBPropRenderizado &propsRenderizado, const NBPropIluminacion &propsIluminacion, const NBPropHeredadasRender &propsHeredadas);
		static void					enviarComandosGL(UI8* punteroDatosModelo);
		//
		void						tickAnimacion(float segsTranscurridos);
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	protected:
		SI32						_iScene;
		NBCajaAABB					_ultimaCajaOrganizada;
		AUEscenaContenedor*			_contenedor;
		AUArregloMutable 			_sprites;
		AUArregloMutable 			_textos;
		AUCadenaMutable8			_strTmp;
		AUCadenaMutable8			_strResumenConsola;
		//
		STCicloAcumulados			_acumCiclosCPU;
		#ifdef CONFIG_NB_RECOPILAR_ESTADISTICAS_DE_GESTION_MEMORIA
		UI32						_conteoBloquesAsignadosTotal;
		UI32						_bytesTotalZonas;
		STEstadoTipoMemoria			_datosEstadoMem[ENMemoriaTipo_Conteo];
		#endif
		//GRAFICO DE CILCOS DE PROCESADOR
		SI32						_ciclosAcumSinActualizarMemoria;
		float						_escalaBarraCiclos;
		float						_anchoBarraCiclos;
		float						_xBarraCiclos;
		float						_derechaBarraCiclos;
		float						_velGraficoCiclos;
		CICLOS_CPU_TIPO				_cicloProcesoUltimoTick;
		AUArregloNativoMutableP<STCicloRango> _pendCiclosAnimacion;
		AUArregloNativoMutableP<STCicloRango> _pendCiclosRender;
		AUArregloNativoMutableP<STCicloRango> _pendCiclosVolcado;
		AUArregloNativoOrdenadoMutableP<STZonaInfoVisual> _visualZonasMemoria;
		AUArregloNativoMutableP<STCicloInfoVisual> _visualCiclos;	//Para evitar sobrecargar al motor con sprites y otros objetos que reportarian mas carga que la real
		//
		virtual void				agregadoEnEscena();
		virtual void				quitandoDeEscena();
		AUTextura*					privTexturaDeIcono(ENResumenDebugIcono icono);
		void						privOrganizarIconos(NBCajaAABB caja);
		void						privCadenaAgregarEnFormatoMillones(UI32 cantidadTotal, AUCadenaMutable8* cadenaDestino);
		void						privAgregarCadenaEnFormatoBytes(UI32 bytesTotal, AUCadenaMutable8* cadenaDestino);
		void						privCadenaEnFormatoCantidadMasBytes(SI32 cantidadTotal, UI32 bytesTotal, AUCadenaMutable8* cadenaDestino);
		void						privAgregarBarraCiclosProcesador(SI32 indiceBarra, CICLOS_CPU_TIPO cicloIni, CICLOS_CPU_TIPO cicloFin, CICLOS_CPU_TIPO primerCicloDeTick, CICLOS_CPU_TIPO ciclosEnTick);
};

#endif

