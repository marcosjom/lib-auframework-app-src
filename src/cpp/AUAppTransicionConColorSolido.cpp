//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicionConColorSolido.h"

AUAppTransicionConColorSolido::AUAppTransicionConColorSolido(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const float secsDur, const STNBColor8 colorUsar, const ENGestorTexturaModo modoCargaTexturas) : AUAppTransicion(){
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppTransicionConColorSolido")
	_iScene						= iEscena;
	_escuchador					= escuchador;
	_modoCargaTexturas			= modoCargaTexturas; NBASSERT(_modoCargaTexturas != ENGestorTexturaModo_cargaPendiente)
	_datosCargaNegro.layer		= NULL;
	_datosCargaNegro.glass		= NULL;
	_datosCargaNegro.etapaCarga	= (ENEtapaCargaSolido)((SI32)ENEtapaCargaSolido_Ninguna + 1);
	_datosCargaNegro.colorUsar	= colorUsar;
	_datosCargaNegro.secsCur	= 0.0f;
	_datosCargaNegro.secsDur	= (secsDur / 2.0f);
	NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene, this);
}

AUAppTransicionConColorSolido::~AUAppTransicionConColorSolido(){
	NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene, this);
	//Remove from scene
	{
		if(_datosCargaNegro.layer != NULL){
			NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaNegro.layer);
			_datosCargaNegro.layer->liberar(NB_RETENEDOR_THIS);
			_datosCargaNegro.layer = NULL;
		}
		if(_datosCargaNegro.glass != NULL){
			_datosCargaNegro.glass->liberar(NB_RETENEDOR_THIS);
			_datosCargaNegro.glass = NULL;
		}
	}
}

void AUAppTransicionConColorSolido::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	if(_datosCargaNegro.glass != NULL){
		const NBCajaAABB scnBox = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
		_datosCargaNegro.glass->redimensionar(scnBox.xMin - 1.0f, scnBox.yMin - 1.0f, (scnBox.xMax - scnBox.xMin) + 1.0f, (scnBox.yMax - scnBox.yMin) + 1.0f);
	}
}

//

bool AUAppTransicionConColorSolido::ejecutandoTransicion(){
	return (_datosCargaNegro.etapaCarga != ENEtapaCargaSolido_Ninguna);
}

bool AUAppTransicionConColorSolido::tickTransicion(float segsTranscurridos){
	if(_datosCargaNegro.etapaCarga==ENEtapaCargaSolido_Ninguna){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_Ninguna.\n");
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_IniciarSalidaViejo){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_IniciarSalidaViejo.\n");
		//Iniciar animacion de salida
		_escuchador->escenasAnimarSalida();
		_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_EsperarSalidaViejo){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_EsperarSalidaViejo.\n");
		//ESPERAR A QUE SE EJECUTEN TODAS LAS ANIMACIONES DE SALIDA
		if(!_escuchador->escenasAnimandoSalida()){
			_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
		}
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_OscurecerIni){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_OscurecerIni.\n");
		//Color de fondo
		_datosCargaNegro.secsCur = 0.0f;
		//Create layer
		{
			if(_datosCargaNegro.layer == NULL){
				NBColor8 lightColor; lightColor.r = lightColor.g = lightColor.b  = lightColor.a = 255;
				_datosCargaNegro.layer = new(this) AUEscenaContenedor();
				NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaNegro.layer, lightColor);
			}
			if(_datosCargaNegro.glass == NULL){
				_datosCargaNegro.glass = new(this) AUEscenaSprite();
				_datosCargaNegro.layer->agregarObjetoEscena(_datosCargaNegro.glass);
			}
			if(_datosCargaNegro.glass != NULL){
				const NBCajaAABB scnBox = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
				_datosCargaNegro.glass->redimensionar(scnBox.xMin - 1.0f, scnBox.yMin - 1.0f, (scnBox.xMax - scnBox.xMin) + 1.0f, (scnBox.yMax - scnBox.yMin) + 1.0f);
				_datosCargaNegro.glass->establecerMultiplicadorColor8(_datosCargaNegro.colorUsar.r, _datosCargaNegro.colorUsar.g, _datosCargaNegro.colorUsar.b, (UI8)((float)_datosCargaNegro.colorUsar.a * 0.0f));
			}
		}
		_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_Oscurecer){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_Oscurecer.\n");
		_datosCargaNegro.secsCur += segsTranscurridos;
		if(_datosCargaNegro.secsCur < _datosCargaNegro.secsDur){
			if(_datosCargaNegro.glass != NULL){
				const float rel = (_datosCargaNegro.secsCur / _datosCargaNegro.secsDur);
				_datosCargaNegro.glass->establecerMultiplicadorAlpha8((float)_datosCargaNegro.colorUsar.a * rel);
			}
		} else {
			_datosCargaNegro.glass->establecerMultiplicadorAlpha8((float)_datosCargaNegro.colorUsar.a * 1.0f);
			_datosCargaNegro.secsCur = 0.0f;
			_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
		}
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_Aclarar){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_Aclarar.\n");
		_datosCargaNegro.secsCur += segsTranscurridos;
		if(_datosCargaNegro.secsCur < _datosCargaNegro.secsDur){
			if(_datosCargaNegro.glass != NULL){
				const float rel = (_datosCargaNegro.secsCur / _datosCargaNegro.secsDur);
				_datosCargaNegro.glass->establecerMultiplicadorAlpha8((float)_datosCargaNegro.colorUsar.a * (1.0f - rel));
			}
		} else {
			_datosCargaNegro.glass->establecerMultiplicadorAlpha8((float)_datosCargaNegro.colorUsar.a * 0.0f);
			_datosCargaNegro.secsCur = 0.0f;
			_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
		}
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_LiberarViejo){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_LiberarViejo.\n");
		NBGestorAnimadores::establecerAnimadorActivo(false);
		//Liberar escena actual
		_escuchador->escenasQuitarDePrimerPlano();
		_escuchador->escenasLiberar();
		_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_CrearNuevoLiberarRecursosSinUso){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_CrearNuevoLiberarRecursosSinUso.\n");
		//Cargar escena y precargar sus recursos
		NBGestorTexturas::modoCargaTexturasPush(ENGestorTexturaModo_cargaInmediata);
		_escuchador->escenasCargar();
		NBGestorTexturas::modoCargaTexturasPop();
		//Color de fondo
		_escuchador->escenasColocarEnPrimerPlano();
		_escuchador->escenasAnimarEntrada();
		_datosCargaNegro.secsCur = 0.0f;
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
		_datosCargaNegro.etapaCarga = (ENEtapaCargaSolido)(_datosCargaNegro.etapaCarga + 1);
	} else if(_datosCargaNegro.etapaCarga == ENEtapaCargaSolido_AclararFin){
		//PRINTF_INFO("AUAppTransicionConColorSolido, ENEtapaCargaSolido_AclararFin.\n");
		_datosCargaNegro.etapaCarga = ENEtapaCargaSolido_Ninguna;
		#ifdef CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
		NBGestorMemoria::liberarZonasSinUso();
		#endif
	}
	return false;
}
