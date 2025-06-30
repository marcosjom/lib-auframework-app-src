//
//  AUEscenaResumenDebug.cpp
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 04/05/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppEscenasResumenDebug.h"

AUEscenaResumenDebug::AUEscenaResumenDebug(const SI32 iEscena) : AUEscenaContenedor(), _sprites(this), _textos(this), _strTmp(this), _strResumenConsola(this), _pendCiclosAnimacion(this), _pendCiclosRender(this), _pendCiclosVolcado(this), _visualZonasMemoria(this), _visualCiclos(this) {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::AUEscenaResumenDebug")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUEscenaResumenDebug");
	_iScene							= iEscena;
	_contenedor							= new(this) AUEscenaContenedor();	NB_DEFINE_NOMBRE_PUNTERO(_contenedor, "AUEscenaResumenDebug::_contenedor")
	NBCAJAAABB_INICIALIZAR(_ultimaCajaOrganizada);
	//
	_acumCiclosCPU.conteoCiclosAnim		= 0;
	_acumCiclosCPU.ciclosAnimacion		= 0;
	_acumCiclosCPU.ciclosActMatrices	= 0;
	_acumCiclosCPU.ciclosActVertsGL		= 0;
	_acumCiclosCPU.ciclosFisicaLiquidos	= 0;
	_acumCiclosCPU.ciclosFisicaSolidos	= 0;
	_acumCiclosCPU.ciclosFisicaUniones	= 0;
	_acumCiclosCPU.ciclosAnimando		= 0;
	//
	_acumCiclosCPU.conteoCiclosRender	= 0;
	_acumCiclosCPU.ciclosEnvCmdsGL		= 0;
	//
	_acumCiclosCPU.conteoCiclosvolcado	= 0;
	_acumCiclosCPU.ciclosVolcandoEscena	= 0;
	//GRAFICO DE ZONAS DE MEMORIA
	#ifdef CONFIG_NB_RECOPILAR_ESTADISTICAS_DE_GESTION_MEMORIA
	_ciclosAcumSinActualizarMemoria		= AUESCENARESUMENDEBUG_SEGUNDOS_ACUMULAR_PARA_PRESENTAR_GRAFICO_MEMORIA;
	_conteoBloquesAsignadosTotal		= 0;
	_bytesTotalZonas					= 0;
	SI32 iTipoMem; 
	for(iTipoMem=0; iTipoMem<ENMemoriaTipo_Conteo; iTipoMem++){
		NBGestorMemoria::formatearRegistroEstadoMemoria(&_datosEstadoMem[iTipoMem]);
		_datosEstadoMem[iTipoMem].tipoMemoria = (ENMemoriaTipo)iTipoMem;
	}
	#endif
	//GRAFICO DE CICLOS DE CPU
	_escalaBarraCiclos					= 4.0f;
	_anchoBarraCiclos					= 960.0f;
	_xBarraCiclos						= 0.0f;
	_derechaBarraCiclos					= 0.0f;
	_velGraficoCiclos					= 0.0f;
	_cicloProcesoUltimoTick				= 0;
	//
	AUFuenteTextura* fuente = NBGestorEscena::fuenteTexturaEnPuntos(_iScene, "Arial", 12.0f, false, false);
	//Primero agregar los iconos (juntos para evitar que se cambie de textura constantemente)
	SI32 iIco;
	for(iIco=0; iIco<ENResumenDebugIcono_Conteo; iIco++){
		AUTextura* textura		= privTexturaDeIcono((ENResumenDebugIcono)iIco); NBASSERT(textura != NULL)
		AUEscenaSprite* icono	= new(this) AUEscenaSprite(textura); NB_DEFINE_NOMBRE_PUNTERO(icono, "AUEscenaResumenDebug::icono")
		_sprites.agregarElemento(icono);
		_contenedor->agregarObjetoEscena(icono);
		icono->liberar(NB_RETENEDOR_THIS);
	}
	this->agregarObjetoEscena(_contenedor);
	//Luego agregar todos los textos (juntos para evitar que se cambie de textura constantemente)
	for(iIco=0; iIco<ENResumenDebugIcono_Conteo; iIco++){
		AUEscenaTexto* texto	= new(this) AUEscenaTexto(fuente);  NB_DEFINE_NOMBRE_PUNTERO(texto, "AUEscenaResumenDebug::texto")
		texto->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_Center);
		texto->establecerMultiplicadorColor8(255, 0, 0, 255);
		_textos.agregarElemento(texto);
		_contenedor->agregarObjetoEscena(texto);
		texto->liberar(NB_RETENEDOR_THIS);
	}
	this->agregarObjetoEscena(_contenedor);
	NBCAJAAABB_INICIALIZAR(_ultimaCajaOrganizada);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUEscenaResumenDebug::~AUEscenaResumenDebug(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::~AUEscenaResumenDebug")
	_contenedor->liberar(NB_RETENEDOR_THIS);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

void AUEscenaResumenDebug::privAgregarBarraCiclosProcesador(SI32 indiceBarra, CICLOS_CPU_TIPO cicloIni, CICLOS_CPU_TIPO cicloFin, CICLOS_CPU_TIPO primerCicloDeTick, CICLOS_CPU_TIPO ciclosEnTick){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::privAgregarBarraCiclosProcesador")
	STCicloInfoVisual datosVisual;
	datosVisual.indiceBarra	= indiceBarra;
	datosVisual.xPos		= _derechaBarraCiclos+((float)_anchoBarraCiclos*((float)(cicloIni-primerCicloDeTick)/(float)ciclosEnTick)*_escalaBarraCiclos);
	datosVisual.ancho		= (float)_anchoBarraCiclos*((float)(cicloFin-cicloIni)/(float)ciclosEnTick)*_escalaBarraCiclos;
	_visualCiclos.agregarElemento(datosVisual);
	//PRINTF_INFO("SPRITE DE CICLO(%d - %d) CICLOS(%d) ... xPOS(%f) ancho(%f) anchoBarra(%f)\n", (SI32)cicloIni, (SI32)cicloFin, (SI32)(cicloFin-cicloIni), xPos, anchoSprite, _anchoBarraCiclos);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::acumularCiclosAnimacion(const NBEstadoCicloAnimacion &ciclosPorEstado){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::acumularCiclosAnimacion")
	_acumCiclosCPU.conteoCiclosAnim++;
	_acumCiclosCPU.ciclosAnimacion		+= (ciclosPorEstado.cicloProcesadorFin - ciclosPorEstado.cicloProcesadorInicio);
	_acumCiclosCPU.ciclosActMatrices	+= (ciclosPorEstado.cicloFinMatricesActualizadas - ciclosPorEstado.cicloIniMatricesActualizadas);
	_acumCiclosCPU.ciclosActVertsGL		+= (ciclosPorEstado.cicloFinVerticesGLActualizados - ciclosPorEstado.cicloIniVerticesGLActualizados);
#	ifndef CONFIG_NB_UNSUPPORT_BOX2D
	_acumCiclosCPU.ciclosFisicaLiquidos	+= (ciclosPorEstado.cicloFinFisicaLiquidos - ciclosPorEstado.cicloIniFisicaLiquidos);
	_acumCiclosCPU.ciclosFisicaSolidos	+= (ciclosPorEstado.cicloFinFisicaSolidos - ciclosPorEstado.cicloIniFisicaSolidos);
	_acumCiclosCPU.ciclosFisicaUniones	+= (ciclosPorEstado.cicloFinFisicaUniones - ciclosPorEstado.cicloIniFisicaUniones);
#	endif
	_acumCiclosCPU.ciclosAnimando		+= (ciclosPorEstado.cicloFinAnimacionesEjecutadas - ciclosPorEstado.cicloIniAnimacionesEjecutadas);
	//
	STCicloRango rango;
	rango.cicloIni = ciclosPorEstado.cicloProcesadorInicio;
	rango.cicloFin = ciclosPorEstado.cicloProcesadorFin;
	_pendCiclosAnimacion.agregarElemento(rango);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::acumularCiclosRender(const NBEstadoCicloRender &ciclosPorEstado){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::acumularCiclosRender")
	_acumCiclosCPU.conteoCiclosRender++;
	_acumCiclosCPU.ciclosEnvCmdsGL		+= (ciclosPorEstado.cicloFinComandosGLEnviados - ciclosPorEstado.cicloIniComandosGLEnviados);
	//
	STCicloRango rango;
	rango.cicloIni = ciclosPorEstado.cicloIniComandosGLEnviados;
	rango.cicloFin = ciclosPorEstado.cicloFinComandosGLEnviados;
	_pendCiclosRender.agregarElemento(rango);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::acumularCiclosVolcado(const NBEstadoCicloVolcado &ciclosPorEstado){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::acumularCiclosVolcado")
	_acumCiclosCPU.conteoCiclosvolcado++;
	_acumCiclosCPU.ciclosVolcandoEscena	+= (ciclosPorEstado.cicloFinEscenaVolcada - ciclosPorEstado.cicloIniEscenaVolcada);
	//
	STCicloRango rango;
	rango.cicloIni = ciclosPorEstado.cicloIniEscenaVolcada;
	rango.cicloFin = ciclosPorEstado.cicloFinEscenaVolcada;
	_pendCiclosVolcado.agregarElemento(rango);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::tickSegundoRecopilarInformacion(CICLOS_CPU_TIPO cicloActualDeProceso, bool actualizarDatosTexto, bool actualizarGraficos, bool imprimirEnConsola){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::tickSegundoRecopilarInformacion")
	if(this->idEscena!=-1){
		CICLOS_CPU_TIPO ciclosCpuPorSeg; CICLOS_CPU_POR_SEGUNDO(ciclosCpuPorSeg);
		//Reorganizar (si es necesario)
		const NBCajaAABB cajaResumenDebug = NBGestorEscena::cajaProyeccionGrupo(this->idEscena, ENGestorEscenaGrupo_DebugFrontal);
		_strResumenConsola.vaciar();
		if(_ultimaCajaOrganizada!=cajaResumenDebug){
			privOrganizarIconos(cajaResumenDebug);
			_ultimaCajaOrganizada = cajaResumenDebug;
		}	
		//APS
		STAnimadoresEstad estadAnimadores = NBGestorAnimadores::estadisticasAnimadores();
		_strTmp.vaciar(); _strTmp.agregarConFormato("%d", estadAnimadores.ticksAcumulados);
		if(estadAnimadores.conteoAnimadoresRecorridos!=0){
			_strTmp.agregarConFormato(" (%d%% exec) ", (SI32)(100*estadAnimadores.conteoAnimadoresEjecutados/estadAnimadores.conteoAnimadoresRecorridos));
		}
		if(_acumCiclosCPU.ciclosAnimacion != 0){
			_strTmp.agregar(" [");
			if(_acumCiclosCPU.ciclosActMatrices!=0) _strTmp.agregarConFormato(" %.1f%% matrices /", (float)(100.0f*(float)_acumCiclosCPU.ciclosActMatrices/(float)_acumCiclosCPU.ciclosAnimacion));
			if(_acumCiclosCPU.ciclosActVertsGL!=0) _strTmp.agregarConFormato(" %.1f%% modelos /", (float)(100.0f*(float)_acumCiclosCPU.ciclosActVertsGL/(float)_acumCiclosCPU.ciclosAnimacion));
			if(_acumCiclosCPU.ciclosAnimando!=0) _strTmp.agregarConFormato(" %.1f%% anim", (float)(100.0f*(float)_acumCiclosCPU.ciclosAnimando/(float)_acumCiclosCPU.ciclosAnimacion));
			if(_acumCiclosCPU.ciclosFisicaLiquidos!=0) _strTmp.agregarConFormato(" %.1f%% liquidos /", (float)(100.0f*(float)_acumCiclosCPU.ciclosFisicaLiquidos/(float)_acumCiclosCPU.ciclosAnimacion));
			if(_acumCiclosCPU.ciclosFisicaSolidos!=0) _strTmp.agregarConFormato(" %.1f%% fisica /", (float)(100.0f*(float)_acumCiclosCPU.ciclosFisicaSolidos/(float)_acumCiclosCPU.ciclosAnimacion));
			if(_acumCiclosCPU.ciclosFisicaUniones!=0) _strTmp.agregarConFormato(" %.1f%% uniones fisica /", (float)(100.0f*(float)_acumCiclosCPU.ciclosFisicaUniones/(float)_acumCiclosCPU.ciclosAnimacion));
			_strTmp.agregar(']');
		}
		((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_APS])->establecerTexto(_strTmp.str());
		_strResumenConsola.agregarConFormato("APS: %s\n", _strTmp.str());
		NBGestorAnimadores::resetearTicksAcumulados();
		//FPS
		_strTmp.vaciar(); _strTmp.agregarConFormato("%d / %d", _acumCiclosCPU.conteoCiclosvolcado, _acumCiclosCPU.conteoCiclosRender); ((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_FPS])->establecerTexto(_strTmp.str());
		_strResumenConsola.agregarConFormato("FPS: %s\n", _strTmp.str());
		NBGestorEscena::resetearTicksAcumulados();
		//CPU
		_strTmp.vaciar(); 
		_strTmp.agregarConFormato("%.1f%% anim", (float)(100.0f*(float)_acumCiclosCPU.ciclosAnimacion/(float)ciclosCpuPorSeg));
		_strTmp.agregarConFormato(" / %.1f%% cmdsGL", (float)(100.0f*(float)_acumCiclosCPU.ciclosEnvCmdsGL/(float)ciclosCpuPorSeg));
		_strTmp.agregarConFormato(" / %.1f%% swap", (float)(100.0f*(float)_acumCiclosCPU.ciclosVolcandoEscena/(float)ciclosCpuPorSeg));
		((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_CPU])->establecerTexto(_strTmp.str());
		_strResumenConsola.agregarConFormato("CPU: %s\n", _strTmp.str());
		//
		SI32 iInfo;
		for(iInfo=ENResumenDebugIcono_FPS+1; iInfo<ENResumenDebugIcono_Conteo; iInfo++){
			((AUEscenaObjeto*)_sprites.elemento[iInfo])->establecerVisible(actualizarDatosTexto);
			((AUEscenaObjeto*)_textos.elemento[iInfo])->establecerVisible(actualizarDatosTexto);
		}
		//
		if(actualizarDatosTexto){
			_ciclosAcumSinActualizarMemoria++;
			//Estadisticas de memoria
			#ifdef CONFIG_NB_RECOPILAR_ESTADISTICAS_DE_GESTION_MEMORIA
			//Estado de la memoria por tipo
			_conteoBloquesAsignadosTotal = 0;
			UI32 bytesTotalZonas = 0;
			SI32 iTipoMem;
			for(iTipoMem=0; iTipoMem<ENMemoriaTipo_Conteo; iTipoMem++){
				_datosEstadoMem[iTipoMem] = NBGestorMemoria::estadoMemoriaPorTipo((ENMemoriaTipo)iTipoMem);
				_conteoBloquesAsignadosTotal += _datosEstadoMem[iTipoMem].conteoBloquesAsignados;
				bytesTotalZonas += _datosEstadoMem[iTipoMem].bytesTotalZonas;
			}
			//Punteros
			_strTmp.vaciar(); _strTmp.agregarConFormato("%d  (", _conteoBloquesAsignadosTotal);
			for(iTipoMem=0; iTipoMem<ENMemoriaTipo_Conteo; iTipoMem++){
				if(iTipoMem!=0) _strTmp.agregar(", ");
				_strTmp.agregarConFormato("%s: %d", NBGestorMemoria::nombreCortoTipoMemoria((ENMemoriaTipo)iTipoMem), _datosEstadoMem[iTipoMem].conteoBloquesAsignados);
			}
			_strTmp.agregar(")");
			((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_PunterosMemoria])->establecerTexto(_strTmp.str());
			_strResumenConsola.agregarConFormato("Punteros: %s\n", _strTmp.str());
			//Movimiento de memoria
			_strTmp.vaciar(); _strTmp.agregarConFormato("+%d -%d (+", (SI32)NBGestorMemoria::debugConteoPunterosCreadosAcum(), (SI32)NBGestorMemoria::debugConteoPunterosEliminadosAcum());
			privAgregarCadenaEnFormatoBytes(NBGestorMemoria::debugBytesReservadosAcum(), &_strTmp);
			_strTmp.agregar(" / -");
			privAgregarCadenaEnFormatoBytes(NBGestorMemoria::debugBytesLiberadosAcum(), &_strTmp);
			_strTmp.agregar(")");
			((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_MovimientoMemoria])->establecerTexto(_strTmp.str());
			NBGestorMemoria::debugResetearContadoresAcum();
			_strResumenConsola.agregarConFormato("Punteros mov: %s\n", _strTmp.str());
			//Memoria del sistema
			_strTmp.vaciar(); privAgregarCadenaEnFormatoBytes(bytesTotalZonas, &_strTmp); _strTmp.agregar(" (");
			for(iTipoMem=0; iTipoMem<ENMemoriaTipo_Conteo; iTipoMem++){
				if(iTipoMem!=0) _strTmp.agregar(", ");
				_strTmp.agregarConFormato("%s: ", NBGestorMemoria::nombreCortoTipoMemoria((ENMemoriaTipo)iTipoMem));
				privAgregarCadenaEnFormatoBytes(_datosEstadoMem[iTipoMem].bytesTotalZonas, &_strTmp);
				if(_datosEstadoMem[iTipoMem].bytesTotalZonas!=0) _strTmp.agregarConFormato(" al %.1f%%", 100.0f*(float)(_datosEstadoMem[iTipoMem].bytesTotalZonas - _datosEstadoMem[iTipoMem].bytesLibresZonas) / (float)_datosEstadoMem[iTipoMem].bytesTotalZonas);
			}
			_strTmp.agregar(") maxSimult(");
			privAgregarCadenaEnFormatoBytes(NBGestorMemoria::debugBytesReservadosMaxSimultaneos(), &_strTmp);
			_strTmp.agregar(")");
			((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_RamSis])->establecerTexto(_strTmp.str());
			_strResumenConsola.agregarConFormato("MemoriaSis: %s\n", _strTmp.str());
			//Memoria de video
			_strTmp.vaciar(); privAgregarCadenaEnFormatoBytes(NBGestorTexturas::debugBytesTotalTexturas()+NBGestorEscena::debugBytesDeRenderBuffers(), &_strTmp); ((AUEscenaTexto*)_textos.elemento[ENResumenDebugIcono_RamVid])->establecerTexto(_strTmp.str());
			_strResumenConsola.agregarConFormato("MemoriaVid: %s\n", _strTmp.str());
			//Audio
			UI32 bytesAudio; SI32 conteoFuentesALEnUso, conteoBufferes, conteoBufferesStream, conteoStreams;
			bytesAudio	= NBGestorSonidos::debugBytesTotalBufferes(&conteoFuentesALEnUso, &conteoBufferes, &conteoBufferesStream, &conteoStreams);
			#endif
			if(imprimirEnConsola) PRINTF_INFO("RESUMEN DEBUG ----------\n%s", _strResumenConsola.str());
		}
		if(actualizarGraficos){
			//
			//GRAFICO DE ZONAS DE MEMORIA
			//
			if(_ciclosAcumSinActualizarMemoria>=AUESCENARESUMENDEBUG_SEGUNDOS_ACUMULAR_PARA_PRESENTAR_GRAFICO_MEMORIA){
				_visualZonasMemoria.vaciar();
				#ifdef CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
				const STZonaMemoria* zonasMemoria = NBGestorMemoria::zonasMemoria();
				SI32 iZona;
				for(iZona=0; iZona<NB_GESTOR_MEMORIA_CANTIDAD_REGISTROS_ZONA_MEMORIA; iZona++){
					if(zonasMemoria[iZona].registroOcupado){
						STZonaInfoVisual datosZona;
						datosZona.primerByteZona		= NBZonaMemoriaConIndiceLibres::primerByteZona(zonasMemoria[iZona].datos);
						datosZona.tipoMemoriaZona		= (ENMemoriaTipo)zonasMemoria[iZona].tipoMemoria;
						datosZona.bytesTotalIndice		= NBZonaMemoriaConIndiceLibres::bytesIndiceTotal(zonasMemoria[iZona].datos);
						datosZona.bytesTotalDatos		= NBZonaMemoriaConIndiceLibres::bytesDatosTotal(zonasMemoria[iZona].datos);
						datosZona.porcentajeUsoIndice	= 1.0f-((float)NBZonaMemoriaConIndiceLibres::bytesIndiceLibres(zonasMemoria[iZona].datos)/(float)datosZona.bytesTotalIndice);
						datosZona.porcentajeUsoDatos	= 1.0f-((float)NBZonaMemoriaConIndiceLibres::bytesDatosLibres(zonasMemoria[iZona].datos)/(float)datosZona.bytesTotalDatos);
						unsigned long primerByteZona	= (unsigned long)datosZona.primerByteZona;
						unsigned long tamanoZona		= (unsigned long)datosZona.bytesTotalDatos;
						datosZona.porcentajesUso[0]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.00f), primerByteZona+(tamanoZona*0.10f));
						datosZona.porcentajesUso[1]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.10f), primerByteZona+(tamanoZona*0.20f));
						datosZona.porcentajesUso[2]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.20f), primerByteZona+(tamanoZona*0.30f));
						datosZona.porcentajesUso[3]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.30f), primerByteZona+(tamanoZona*0.40f));
						datosZona.porcentajesUso[4]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.40f), primerByteZona+(tamanoZona*0.50f));
						datosZona.porcentajesUso[5]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.50f), primerByteZona+(tamanoZona*0.60f));
						datosZona.porcentajesUso[6]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.60f), primerByteZona+(tamanoZona*0.70f));
						datosZona.porcentajesUso[7]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.70f), primerByteZona+(tamanoZona*0.90f));
						datosZona.porcentajesUso[8]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.80f), primerByteZona+(tamanoZona*0.90f));
						datosZona.porcentajesUso[9]		= NBZonaMemoriaConIndiceLibres::porcentajeUsoRango(zonasMemoria[iZona].datos, primerByteZona+(tamanoZona*0.90f), primerByteZona+(tamanoZona*1.00f));
						_visualZonasMemoria.agregarElemento(datosZona);
					}
				}
				#endif
			}
			//
			//GRAFICO DE CICLOS DE PROCESADOR
			//
			NBCajaAABB cajaEscena = NBGestorEscena::cajaProyeccionGrupo(this->idEscena, (ENGestorEscenaGrupo)this->idGrupo);
			NBTamano tamEscena; NBCAJAAABB_TAMANO(tamEscena, cajaEscena);
			_anchoBarraCiclos = tamEscena.ancho;
			SI32 iVisual;
			for(iVisual=_visualCiclos.conteo-1; iVisual>=0; iVisual--){
				STCicloInfoVisual datosVisual = _visualCiclos.elemento[iVisual];
				if(datosVisual.xPos+datosVisual.ancho+_xBarraCiclos<cajaEscena.xMin){
					_visualCiclos.quitarElementoEnIndice(iVisual);
				}
			}
			//
			CICLOS_CPU_TIPO ciclosEnTick = cicloActualDeProceso - _cicloProcesoUltimoTick;
			UI32 iRango;
			for(iRango=0; iRango<_pendCiclosAnimacion.conteo; iRango++){
				STCicloRango rango = _pendCiclosAnimacion.elemento[iRango];
				privAgregarBarraCiclosProcesador(0, rango.cicloIni, rango.cicloFin, _cicloProcesoUltimoTick, ciclosEnTick);
			}
			for(iRango=0; iRango<_pendCiclosRender.conteo; iRango++){
				STCicloRango rango = _pendCiclosRender.elemento[iRango];
				privAgregarBarraCiclosProcesador(1, rango.cicloIni, rango.cicloFin, _cicloProcesoUltimoTick, ciclosEnTick);
			}
			for(iRango=0; iRango<_pendCiclosVolcado.conteo; iRango++){
				STCicloRango rango = _pendCiclosVolcado.elemento[iRango];
				privAgregarBarraCiclosProcesador(2, rango.cicloIni, rango.cicloFin, _cicloProcesoUltimoTick, ciclosEnTick);
			}
			//
			_velGraficoCiclos		= (cajaResumenDebug.xMin-_derechaBarraCiclos)-_xBarraCiclos;
			_derechaBarraCiclos		+= _anchoBarraCiclos * _escalaBarraCiclos;
		}
		if(_ciclosAcumSinActualizarMemoria>=AUESCENARESUMENDEBUG_SEGUNDOS_ACUMULAR_PARA_PRESENTAR_GRAFICO_MEMORIA){
			_ciclosAcumSinActualizarMemoria = 0;
		}
	}
	//
	_acumCiclosCPU.conteoCiclosAnim		= 0;
	_acumCiclosCPU.ciclosAnimacion		= 0;
	_acumCiclosCPU.ciclosActMatrices	= 0;
	_acumCiclosCPU.ciclosActVertsGL		= 0;
	_acumCiclosCPU.ciclosFisicaLiquidos	= 0;
	_acumCiclosCPU.ciclosFisicaSolidos	= 0;
	_acumCiclosCPU.ciclosFisicaUniones	= 0;
	_acumCiclosCPU.ciclosAnimando		= 0;
	//
	_acumCiclosCPU.conteoCiclosRender	= 0;
	_acumCiclosCPU.ciclosEnvCmdsGL		= 0;
	//
	_acumCiclosCPU.conteoCiclosvolcado	= 0;
	_acumCiclosCPU.ciclosVolcandoEscena	= 0;
	//
	_pendCiclosRender.vaciar();
	_pendCiclosAnimacion.vaciar();
	_pendCiclosVolcado.vaciar();
	_cicloProcesoUltimoTick = cicloActualDeProceso;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

PTRfunCmdsGL AUEscenaResumenDebug::actualizarModeloGL(const NBPropRenderizado &propsRenderizado, const NBPropIluminacion &propsIluminacion, const NBPropHeredadasRender &propsHeredadas){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::actualizarModeloGL")
	NBASSERT(idEscena!=-1) //Solo si esta en escena
	SI32 modelosTotal	= (_visualCiclos.conteo + (_visualZonasMemoria.conteo * 12) + (_visualCiclos.conteo==0?0:1)); //+1 el fondo
	if(modelosTotal!=0){
		//NBColor colorMultiplicado; NBCOLOR_MULTIPLICAR_CON_UI8(colorMultiplicado, propsHeredadas.colorPadre, _propiedades.color8);
		STBloqueGL bloqueVerticesGL;
		bloqueVerticesGL.cantidadElementos	= modelosTotal * 4;
		bloqueVerticesGL.primerElemento		= (*propsRenderizado.funcReservarVertices)(ENVerticeGlTipo_MonoTextura, bloqueVerticesGL.cantidadElementos);
		NBCajaAABB cajaEscena			= NBGestorEscena::cajaProyeccionGrupo(this->idEscena, (ENGestorEscenaGrupo)this->idGrupo);
		//NBTamano tamEscena;			NBCAJAAABB_TAMANO(tamEscena, cajaEscena);
		float yPos						= cajaEscena.yMin + 48.0f;
		NBVerticeGL* verticesGL			= &((*propsRenderizado.verticesGL1)[bloqueVerticesGL.primerElemento]);
		//Zonas de memoria
		UI32 iZona; float xPos			= cajaEscena.xMin + 92.0f, yPosAba  = cajaEscena.yMin + 48.0f + 2.0f;
		for(iZona=0; iZona<_visualZonasMemoria.conteo; iZona++){
			STZonaInfoVisual datosZona	= _visualZonasMemoria.elemento[iZona];
			float pixelesPorMB			= 64.0f;
			float altoFondoIndice		= ((float)datosZona.bytesTotalIndice/(1024.0f*1024.0f)) * pixelesPorMB;
			float altoUsoIndice			= (((float)datosZona.bytesTotalIndice*datosZona.porcentajeUsoIndice)/(1024.0f*1024.0f)) * pixelesPorMB;
			float altoBloque10P			= ((float)datosZona.bytesTotalDatos/10.0f/(1024.0f*1024.0f)) * pixelesPorMB;
			//
			NBColor8 colorFondoIndice, colorUsoIndice;
			if(datosZona.tipoMemoriaZona==ENMemoriaTipo_Nucleo){
				NBCOLOR_ESTABLECER(colorFondoIndice, 155, 255, 155, 255)
				NBCOLOR_ESTABLECER(colorUsoIndice, 0, 55, 0, 255)
			} else if(datosZona.tipoMemoriaZona==ENMemoriaTipo_General){
				NBCOLOR_ESTABLECER(colorFondoIndice, 155, 155, 255, 255)
				NBCOLOR_ESTABLECER(colorUsoIndice, 0, 0, 55, 255)
			} else {
				NBCOLOR_ESTABLECER(colorFondoIndice, 255, 155, 155, 255)
				NBCOLOR_ESTABLECER(colorUsoIndice, 55, 0, 0, 255)
			}
			//Fondo de uso vertices
			NBPUNTO_ESTABLECER(verticesGL[0], xPos,			yPosAba+altoFondoIndice);	NBCOLOR_ESTABLECER(verticesGL[0], colorFondoIndice.r, colorFondoIndice.g, colorFondoIndice.b, colorFondoIndice.a); verticesGL[0].tex.x = 0.0f; verticesGL[0].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[1], xPos+12.0f,	yPosAba+altoFondoIndice);	NBCOLOR_ESTABLECER(verticesGL[1], colorFondoIndice.r, colorFondoIndice.g, colorFondoIndice.b, colorFondoIndice.a); verticesGL[1].tex.x = 0.0f; verticesGL[1].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[2], xPos,			yPosAba);					NBCOLOR_ESTABLECER(verticesGL[2], colorFondoIndice.r, colorFondoIndice.g, colorFondoIndice.b, colorFondoIndice.a); verticesGL[2].tex.x = 0.0f; verticesGL[2].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[3], xPos+12.0f,	yPosAba);					NBCOLOR_ESTABLECER(verticesGL[3], colorFondoIndice.r, colorFondoIndice.g, colorFondoIndice.b, colorFondoIndice.a); verticesGL[3].tex.x = 0.0f; verticesGL[3].tex.y = 0.0f;
			verticesGL		+= 4;
			//
			//Fondo de uso vertices
			NBPUNTO_ESTABLECER(verticesGL[0], xPos,			yPosAba+altoUsoIndice);		NBCOLOR_ESTABLECER(verticesGL[0], colorUsoIndice.r, colorUsoIndice.g, colorUsoIndice.b, colorUsoIndice.a); verticesGL[0].tex.x = 0.0f; verticesGL[0].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[1], xPos+12.0f,	yPosAba+altoUsoIndice);		NBCOLOR_ESTABLECER(verticesGL[1], colorUsoIndice.r, colorUsoIndice.g, colorUsoIndice.b, colorUsoIndice.a); verticesGL[1].tex.x = 0.0f; verticesGL[1].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[2], xPos,			yPosAba);					NBCOLOR_ESTABLECER(verticesGL[2], colorUsoIndice.r, colorUsoIndice.g, colorUsoIndice.b, colorUsoIndice.a); verticesGL[2].tex.x = 0.0f; verticesGL[2].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[3], xPos+12.0f,	yPosAba);					NBCOLOR_ESTABLECER(verticesGL[3], colorUsoIndice.r, colorUsoIndice.g, colorUsoIndice.b, colorUsoIndice.a); verticesGL[3].tex.x = 0.0f; verticesGL[3].tex.y = 0.0f;
			verticesGL		+= 4;
			//
			xPos += 12.0f + 4.0f;
			SI32 iPorc; float yPosPorcBarra = yPosAba;
			for(iPorc=0; iPorc<10; iPorc++){
				NBPUNTO_ESTABLECER(verticesGL[0], xPos,			yPosPorcBarra+altoBloque10P);	verticesGL[0].tex.x = 0.0f; verticesGL[0].tex.y = 0.0f;
				NBPUNTO_ESTABLECER(verticesGL[1], xPos+20.0f,	yPosPorcBarra+altoBloque10P);	verticesGL[1].tex.x = 0.0f; verticesGL[1].tex.y = 0.0f;
				NBPUNTO_ESTABLECER(verticesGL[2], xPos,			yPosPorcBarra);					verticesGL[2].tex.x = 0.0f; verticesGL[2].tex.y = 0.0f;
				NBPUNTO_ESTABLECER(verticesGL[3], xPos+20.0f,	yPosPorcBarra);					verticesGL[3].tex.x = 0.0f; verticesGL[3].tex.y = 0.0f;
				NBColor8 color;
				if(datosZona.tipoMemoriaZona==ENMemoriaTipo_Nucleo){
					NBCOLOR_ESTABLECER(color, 255*(1.0f-datosZona.porcentajesUso[iPorc]), 255, 255*(1.0f-datosZona.porcentajesUso[iPorc]), 255)
				} else if(datosZona.tipoMemoriaZona==ENMemoriaTipo_General){
					NBCOLOR_ESTABLECER(color, 255*(1.0f-datosZona.porcentajesUso[iPorc]), 255*(1.0f-datosZona.porcentajesUso[iPorc]), 255, 255)
				} else {
					NBCOLOR_ESTABLECER(color, 255, 255*(1.0f-datosZona.porcentajesUso[iPorc]), 255*(1.0f-datosZona.porcentajesUso[iPorc]), 255)
				}
				NBCOLOR_ESTABLECER(verticesGL[0], color.r, color.g, color.b, color.a);
				NBCOLOR_ESTABLECER(verticesGL[1], color.r, color.g, color.b, color.a);
				NBCOLOR_ESTABLECER(verticesGL[2], color.r, color.g, color.b, color.a);
				NBCOLOR_ESTABLECER(verticesGL[3], color.r, color.g, color.b, color.a);
				verticesGL		+= 4;
				yPosPorcBarra	+= altoBloque10P;
			}
			xPos += 20.0f + 12.0f;
		}
		//GRAFICO CPU/PROCESADOR
		if(_visualCiclos.conteo>0){
			//Fondo
			NBPUNTO_ESTABLECER(verticesGL[0], cajaEscena.xMin, yPos+0.0f);	NBCOLOR_ESTABLECER(verticesGL[0], 255, 255, 255, 255); verticesGL[0].tex.x = 0.0f; verticesGL[0].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[1], cajaEscena.xMax, yPos+0.0f);	NBCOLOR_ESTABLECER(verticesGL[1], 255, 255, 255, 255); verticesGL[1].tex.x = 0.0f; verticesGL[1].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[2], cajaEscena.xMin, yPos-48.0f);	NBCOLOR_ESTABLECER(verticesGL[2], 255, 255, 255, 255); verticesGL[2].tex.x = 0.0f; verticesGL[2].tex.y = 0.0f;
			NBPUNTO_ESTABLECER(verticesGL[3], cajaEscena.xMax, yPos-48.0f);	NBCOLOR_ESTABLECER(verticesGL[3], 255, 255, 255, 255); verticesGL[3].tex.x = 0.0f; verticesGL[3].tex.y = 0.0f;
			verticesGL		+= 4;
			//Segmentos
			UI32 iVisual;
			for(iVisual=0; iVisual<_visualCiclos.conteo; iVisual++){
				STCicloInfoVisual datosVisual = _visualCiclos.elemento[iVisual];
				NBColor color; NBCOLOR_ESTABLECER(color, (datosVisual.indiceBarra==0?255:0.0f), (datosVisual.indiceBarra==2?255:0), (datosVisual.indiceBarra==1?255:0), 255);
				float yBarra = (datosVisual.indiceBarra==0?0.0f:datosVisual.indiceBarra==1?-16.0f:-32.0f);
				float xIzq = _xBarraCiclos+datosVisual.xPos;
				float xDer = _xBarraCiclos+datosVisual.xPos+datosVisual.ancho;
				NBPUNTO_ESTABLECER(verticesGL[0], xIzq, yPos+yBarra+0.0f);	NBCOLOR_ESTABLECER(verticesGL[0], color.r, color.g, color.b, color.a); verticesGL[0].tex.x = 0.0f; verticesGL[0].tex.y = 0.0f;
				NBPUNTO_ESTABLECER(verticesGL[1], xDer, yPos+yBarra+0.0f);	NBCOLOR_ESTABLECER(verticesGL[1], color.r, color.g, color.b, color.a); verticesGL[1].tex.x = 0.0f; verticesGL[1].tex.y = 0.0f;
				NBPUNTO_ESTABLECER(verticesGL[2], xIzq, yPos+yBarra-16.0f);	NBCOLOR_ESTABLECER(verticesGL[2], color.r, color.g, color.b, color.a); verticesGL[2].tex.x = 0.0f; verticesGL[2].tex.y = 0.0f;
				NBPUNTO_ESTABLECER(verticesGL[3], xDer, yPos+yBarra-16.0f);	NBCOLOR_ESTABLECER(verticesGL[3], color.r, color.g, color.b, color.a); verticesGL[3].tex.x = 0.0f; verticesGL[3].tex.y = 0.0f;
				verticesGL		+= 4;
			}
		}
		NBASSERT(verticesGL==&((*propsRenderizado.verticesGL1)[bloqueVerticesGL.primerElemento+bloqueVerticesGL.cantidadElementos]))
		//
		NBASSERT(modelosTotal > 0)
		const SI32 iPosDatosRender			= propsRenderizado.bytesDatosModelos->conteo; propsRenderizado.bytesDatosModelos->reservarRegistrosAlFinal(sizeof(STResumenDebugRender));
		STResumenDebugRender* datosRender	= (STResumenDebugRender*)&(propsRenderizado.bytesDatosModelos->elemento[iPosDatosRender]);
		datosRender->bloqueIndicesGL		= NB_GESTOR_GL_DAME_ELEMS_PARA_TRIANGSTRIPS_4(ENVerticeGlTipo_MonoTextura, bloqueVerticesGL, modelosTotal);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return (modelosTotal!=0 ? &AUEscenaResumenDebug::enviarComandosGL : NULL);
}

void AUEscenaResumenDebug::enviarComandosGL(UI8* punteroDatosModelo){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::enviarComandosGL")
	STResumenDebugRender* datosRender = (STResumenDebugRender*)punteroDatosModelo;
	NBASSERT(datosRender->bloqueIndicesGL.cantidadElementos>0)
	NBGestorGL::activarVerticesGL(ENVerticeGlTipo_MonoTextura); NBGESTORGL_DBG_NOMBRAR_ACTIVADOR_VERTICES_GL("AUEscenaResumenDebug")
	NBGestorGL::bindTexture(0, NBGestorTexturas::texturaBlanca->idTexturaGL); //required on some systems where texture-0 will not be drawn
	NB_GESTOR_GL_RENDER_TRIANGSTRIPS_4(datosRender->bloqueIndicesGL.primerElemento, datosRender->bloqueIndicesGL.cantidadElementos); NBGESTORGL_DBG_NOMBRAR_CONVOCADOR_ACUMULAR_TRIANG_STRIPS_INDEP("AUEscenaResumenDebug")
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::agregadoEnEscena(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::agregadoEnEscena")
	this->establecerGrupoAnimacion(ENGestorAnimadoresGrupo_Nucleo); //Animador importante: seguir ejecutando al pausar el GamePlay
	NBGestorAnimadores::agregarAnimador(this, this);
	//AUEscenaContenedor::agregadoEnEscena(); //COMENTARIADO, por ahora AUEscenaContenedor::agregadoEnEscena no hace nada.
	AU_GESTOR_PILA_LLAMADAS_POP_3
}
					   
void AUEscenaResumenDebug::quitandoDeEscena(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::quitandoDeEscena")
	//AUEscenaContenedor::quitandoDeEscena(); //COMENTARIADO, por ahora AUEscenaContenedor::quitandoDeEscena no hace nada.
	NBGestorAnimadores::quitarAnimador(this);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}
					   
void AUEscenaResumenDebug::tickAnimacion(float segsTranscurridos){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::tickAnimacion")
	_xBarraCiclos += (_velGraficoCiclos * segsTranscurridos);
	AU_GESTOR_PILA_LLAMADAS_POP_3	
}

void AUEscenaResumenDebug::privCadenaAgregarEnFormatoMillones(UI32 cantidadTotal, AUCadenaMutable8* cadenaDestino){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::privCadenaAgregarEnFormatoMillones")
	if(cantidadTotal>=1000000){
		cadenaDestino->agregarConFormato("%f M", (float)cantidadTotal/1000000.0f);
	} else if(cantidadTotal>=1000){
		cadenaDestino->agregarConFormato("%d K", (SI32)(cantidadTotal/1024));
	} else {
		cadenaDestino->agregarConFormato("%d", (SI32)(cantidadTotal));
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::privAgregarCadenaEnFormatoBytes(UI32 bytesTotal, AUCadenaMutable8* cadenaDestino){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::privAgregarCadenaEnFormatoBytes")
	if(bytesTotal==0){
		cadenaDestino->agregarConFormato("-");
	} else {
		if(bytesTotal>=1048576){ //(1024x1024)
			cadenaDestino->agregarConFormato("%.1f MB", (float)bytesTotal/1048576.0f);
		} else if(bytesTotal>=1024){
			cadenaDestino->agregarConFormato("%.1f KB", (float)bytesTotal/1024.0f);
		} else {
			cadenaDestino->agregarConFormato("%.1f bytes", (float)bytesTotal);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUEscenaResumenDebug::privCadenaEnFormatoCantidadMasBytes(SI32 cantidadTotal, UI32 bytesTotal, AUCadenaMutable8* cadenaDestino){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::privCadenaEnFormatoBytes")
	cadenaDestino->vaciar();
	if(bytesTotal==0){
		cadenaDestino->establecer("-");
	} else {
		if(bytesTotal>=1048576){ //(1024x1024)
			cadenaDestino->agregarConFormato("%d (%.1f MB)", cantidadTotal, (float)bytesTotal/(1024.0f*1024.0f));
		} else if(bytesTotal>=1024){
			cadenaDestino->agregarConFormato("%d (%.1f KB)", cantidadTotal, (float)bytesTotal/1024.0f);
		} else {
			cadenaDestino->agregarConFormato("%d (%.1f bytes)", cantidadTotal, (float)bytesTotal);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}


void AUEscenaResumenDebug::privOrganizarIconos(NBCajaAABB cajaInfo){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::privOrganizarIconos")
	float posYSup		= cajaInfo.yMax - 4.0f;
	float posXIzq		= cajaInfo.xMin + 4.0f;
	SI32 iIco;
	for(iIco=0; iIco<ENResumenDebugIcono_Conteo; iIco++){
		AUEscenaSprite* icono	= (AUEscenaSprite*)_sprites.elemento[iIco];
		AUEscenaTexto* texto	= (AUEscenaTexto*)_textos.elemento[iIco];
		NBTamano tamIcono		= icono->textura()->tamanoHD();
		icono->establecerTraslacion(posXIzq+(tamIcono.ancho/2.0f), posYSup-(tamIcono.alto/2.0f));
		texto->establecerTraslacion(posXIzq+tamIcono.ancho+4.0f, posYSup-(tamIcono.alto/2.0f));
		posYSup -= (tamIcono.alto + 2.0f);
	}
	_contenedor->establecerTraslacion(0.0f, 0.0f /*((cajaInfo.yMax-cajaInfo.yMin)-(cajaInfo.yMax-posYSup))/-2.0f*/);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUTextura* AUEscenaResumenDebug::privTexturaDeIcono(ENResumenDebugIcono icono) {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaResumenDebug::privTexturaDeIcono")
	AUTextura* textura = NULL;
	switch (icono) {
		case ENResumenDebugIcono_CPU:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugCPU.png");
			break;
		case ENResumenDebugIcono_APS:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugAPS.png");
			break;
		case ENResumenDebugIcono_FPS:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugFPS.png");
			break;
		case ENResumenDebugIcono_Separador0:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugSeparador.png");
			break;
		case ENResumenDebugIcono_Separador2:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugSeparador.png");
			break;
		#ifdef CONFIG_NB_RECOPILAR_ESTADISTICAS_DE_GESTION_MEMORIA
		case ENResumenDebugIcono_PunterosMemoria:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugMem.png");
			break;
		case ENResumenDebugIcono_MovimientoMemoria:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugMem.png");
			break;
		case ENResumenDebugIcono_RamSis:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugRamSis.png");
			break;
		case ENResumenDebugIcono_RamVid:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugRamVid.png");
			break;
		case ENResumenDebugIcono_Separador1:
			textura = NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoDebugSeparador.png");
			break;
		#endif
		default:
			break;
	}
	NBASSERT(textura != NULL)
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return textura;
}

//
AUOBJMETODOS_CLASESID_MULTICLASE(AUEscenaResumenDebug, AUEscenaContenedor)
AUOBJMETODOS_CLASESNOMBRES_MULTICLASE(AUEscenaResumenDebug, "AUEscenaResumenDebug", AUEscenaContenedor)
AUOBJMETODOS_CLONAR_NULL(AUEscenaResumenDebug)





