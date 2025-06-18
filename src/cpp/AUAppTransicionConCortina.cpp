//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicionConCortina.h"

AUAppTransicionConCortina::AUAppTransicionConCortina(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const ENGestorTexturaModo modoCargaTexturas, const bool animarEntrada, const NBColor8 colorVertLeftBottom, const NBColor8 colorVertRightBottom, const NBColor8 colorVertRightTop, const NBColor8 colorVertLeftTop) : AUAppTransicion(){
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppTransicionConCortina")
	_iScene					= iEscena;
	_escuchador					= escuchador;
	_modoCargaTexturas			= modoCargaTexturas; NBASSERT(_modoCargaTexturas != ENGestorTexturaModo_cargaPendiente)
	const NBCajaAABB cajaCortina = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
	_datosCargaCortina.etapaCarga = (ENEtapaCargaCortina)((SI32)ENEtapaCargaCortina_Ninguna + 1);
	_datosCargaCortina.animarEntrada = animarEntrada;
	_datosCargaCortina.permitidoGirarPantallaAntesDeIniciarTransicion = false;
	_datosCargaCortina.alphaSegundosCambiando	= 0.0f;
	_datosCargaCortina.alphaSegundosCambiar	= 0.0f;
	_datosCargaCortina.alphaOrigen	= 0.0f;
	_datosCargaCortina.alphaDestino	= 0.0f;
	_datosCargaCortina.cortinaCapa = new(this) AUEscenaContenedor();
	_datosCargaCortina.cortina	= new(this) AUEscenaFigura(ENEscenaFiguraTipo_PoligonoCerrado);
	_datosCargaCortina.cortina->establecerModo(ENEscenaFiguraModo_Relleno);
	_datosCargaCortina.cortina->agregarCoordenadaLocal(cajaCortina.xMin, cajaCortina.yMin, colorVertLeftBottom.r, colorVertLeftBottom.g, colorVertLeftBottom.b, colorVertLeftBottom.a);
	_datosCargaCortina.cortina->agregarCoordenadaLocal(cajaCortina.xMax, cajaCortina.yMin, colorVertRightBottom.r, colorVertRightBottom.g, colorVertRightBottom.b, colorVertRightBottom.a);
	_datosCargaCortina.cortina->agregarCoordenadaLocal(cajaCortina.xMax, cajaCortina.yMax, colorVertRightTop.r, colorVertRightTop.g, colorVertRightTop.b, colorVertRightTop.a);
	_datosCargaCortina.cortina->agregarCoordenadaLocal(cajaCortina.xMin, cajaCortina.yMax, colorVertLeftTop.r, colorVertLeftTop.g, colorVertLeftTop.b, colorVertLeftTop.a);
	_datosCargaCortina.cortinaCapa->agregarObjetoEscena(_datosCargaCortina.cortina);
	NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene, this);
}

AUAppTransicionConCortina::~AUAppTransicionConCortina(){
	NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene, this);
	if(_datosCargaCortina.cortinaCapa != NULL){
		NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCortina.cortinaCapa);
		_datosCargaCortina.cortinaCapa->liberar(NB_RETENEDOR_THIS);
		_datosCargaCortina.cortinaCapa = NULL;
	}
	if(_datosCargaCortina.cortina != NULL) _datosCargaCortina.cortina->liberar(NB_RETENEDOR_THIS); _datosCargaCortina.cortina = NULL;
}

void AUAppTransicionConCortina::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	const NBCajaAABB cajaCortina = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
	_datosCargaCortina.cortina->moverVerticeHacia(0, cajaCortina.xMin, cajaCortina.yMin);
	_datosCargaCortina.cortina->moverVerticeHacia(1, cajaCortina.xMax, cajaCortina.yMin);
	_datosCargaCortina.cortina->moverVerticeHacia(2, cajaCortina.xMax, cajaCortina.yMax);
	_datosCargaCortina.cortina->moverVerticeHacia(3, cajaCortina.xMin, cajaCortina.yMax);
}

//

bool AUAppTransicionConCortina::ejecutandoTransicion(){
	return (_datosCargaCortina.etapaCarga != ENEtapaCargaCortina_Ninguna);
}

bool AUAppTransicionConCortina::tickTransicion(float segsTranscurridos){
	float segundosParaCambioColor = 0.33f;
	NBColor colorSolidoDestino; NBCOLOR_ESTABLECER(colorSolidoDestino, 0.0f, 0.0f, 0.0f, 1.0f)
	if(_datosCargaCortina.etapaCarga == ENEtapaCargaCortina_Ninguna){
		//nada
	} else if(_datosCargaCortina.etapaCarga == ENEtapaCargaCortina_AparecerIni){
		//Color de fondo
		_datosCargaCortina.alphaSegundosCambiando	= 0.0f;
		_datosCargaCortina.alphaSegundosCambiar		= segundosParaCambioColor;
		_datosCargaCortina.alphaOrigen				= 0.0f;
		_datosCargaCortina.alphaDestino				= 1.0f;
		NBColor8 colorLuz; NBCOLOR_ESTABLECER(colorLuz, 255, 255, 255, 255)
		NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCortina.cortinaCapa, colorLuz);
		if(_datosCargaCortina.animarEntrada){
			_datosCargaCortina.cortina->establecerMultiplicadorAlpha8(255.0f * _datosCargaCortina.alphaOrigen);
			_datosCargaCortina.etapaCarga = (ENEtapaCargaCortina)(_datosCargaCortina.etapaCarga + 1);
		} else {
			_datosCargaCortina.cortina->establecerMultiplicadorAlpha8(255.0f * _datosCargaCortina.alphaDestino);
			_datosCargaCortina.etapaCarga = (ENEtapaCargaCortina)(_datosCargaCortina.etapaCarga + 2);
		}
	} else if(_datosCargaCortina.etapaCarga == ENEtapaCargaCortina_Aparecer || _datosCargaCortina.etapaCarga == ENEtapaCargaCortina_Desaparecer){
		_datosCargaCortina.alphaSegundosCambiando += segsTranscurridos;
		if(_datosCargaCortina.alphaSegundosCambiando < _datosCargaCortina.alphaSegundosCambiar){
			const float avance	= _datosCargaCortina.alphaSegundosCambiando / _datosCargaCortina.alphaSegundosCambiar;
			const float alpha	= _datosCargaCortina.alphaOrigen + ((_datosCargaCortina.alphaDestino - _datosCargaCortina.alphaOrigen) * avance);
			_datosCargaCortina.cortina->establecerMultiplicadorAlpha8(255.0f * alpha);
		} else {
			_datosCargaCortina.cortina->establecerMultiplicadorAlpha8(255.0f * _datosCargaCortina.alphaDestino);
			_datosCargaCortina.etapaCarga = (ENEtapaCargaCortina)(_datosCargaCortina.etapaCarga + 1);
		}
	} else if(_datosCargaCortina.etapaCarga == ENEtapaCargaCortina_LiberarViejo){
		NBGestorAnimadores::establecerAnimadorActivo(false);
		//Liberar escena actual
		_escuchador->escenasQuitarDePrimerPlano();
		_escuchador->escenasLiberar();
		_datosCargaCortina.etapaCarga = (ENEtapaCargaCortina)(_datosCargaCortina.etapaCarga + 1);
	} else if(_datosCargaCortina.etapaCarga == ENEtapaCargaCortina_CrearNuevoLiberarRecursosSinUso){
		//Cargar escena y precargar sus recursos
		NBGestorTexturas::modoCargaTexturasPush(ENGestorTexturaModo_cargaPendiente);
		_escuchador->escenasCargar();
		NBGestorTexturas::modoCargaTexturasPop();
		//Color de fondo
		_escuchador->escenasColocarEnPrimerPlano();
		_datosCargaCortina.alphaSegundosCambiando	= 0.0f;
		_datosCargaCortina.alphaSegundosCambiar		= segundosParaCambioColor;
		_datosCargaCortina.alphaOrigen				= _datosCargaCortina.alphaDestino;
		_datosCargaCortina.alphaDestino				= 0.0f;
		//
		NBGestorAnimadores::establecerAnimadorActivo(true);
		_escuchador->escenasLiberarRecursosSinUso();
		//Cargar recursos pendientes
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		while(NBGestorSonidos::conteoBufferesPendientesDeCargar()!=0){ NBGestorSonidos::cargarBufferesPendientesDeCargar(9999); }
#		endif
		NBGestorTexturas::modoCargaTexturasPush((_modoCargaTexturas == ENGestorTexturaModo_cargaPendiente ? ENGestorTexturaModo_cargaInmediata : _modoCargaTexturas));
		while(NBGestorTexturas::texPendienteOrganizarConteo()!=0){ NBGestorTexturas::texPendienteOrganizarProcesar(9999); }
		NBGestorTexturas::modoCargaTexturasPop();
		//Finalizar proceso
		NBGestorTexturas::generarMipMapsDeTexturas();
		_datosCargaCortina.etapaCarga = (ENEtapaCargaCortina)(_datosCargaCortina.etapaCarga + 1);
	} else if(_datosCargaCortina.etapaCarga == ENEtapaCargaCortina_DesaparecerFin){
		NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCortina.cortinaCapa);
		_datosCargaCortina.etapaCarga = ENEtapaCargaCortina_Ninguna;
		#ifdef CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
		NBGestorMemoria::liberarZonasSinUso();
		#endif
	}
	return false;
}
