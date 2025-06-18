//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicionConFondo.h"

AUAppTransicionConFondo::AUAppTransicionConFondo(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const ENGestorTexturaModo modoCargaTexturas, const char* fondoRutaActual, AUEscenaContenedor* fondoContenedorActual, AUEscenaSpritePorcion* fondoSaliente, const char* fondoRutaCargar, const char* costuraTexturaPatron) : AUAppTransicion(), _fondoRutaActual(this, fondoRutaActual), _fondoRutaCargar(this, fondoRutaCargar) {
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppTransicionConFondo")
	_iScene							= iEscena;
	_escuchador							= escuchador;
	_modoCargaTexturas					= modoCargaTexturas; NBASSERT(_modoCargaTexturas != ENGestorTexturaModo_cargaPendiente)
	_datosCargaFondo.etapaCarga			= (ENEtapaCargaFondo)((SI32)ENEtapaCargaFondo_Ninguna + 1);
	_animadorObjsEscena					= new(this) AUAnimadorObjetoEscena();
	//
	_datosCargaFondo.etapaCarga			= ENEtapaCargaFondo_EsperarSalidaViejo;
	_datosCargaFondo.permitidoGirarPantallaAntesDeIniciarTransicion = false;
	_datosCargaFondo.fondoSalienteCapa	= fondoContenedorActual; if(fondoContenedorActual != NULL) fondoContenedorActual->retener(NB_RETENEDOR_THIS);
	_datosCargaFondo.fondoSaliente		= fondoSaliente; if(fondoSaliente != NULL) fondoSaliente->retener(NB_RETENEDOR_THIS);
	_datosCargaFondo.fondoEntranteCapa	= NULL;
	_datosCargaFondo.fondoEntrante		= NULL;
	_datosCargaFondo.costuraTextura		= NULL;
	_datosCargaFondo.costuraSprite		= NULL;
	//
	NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene, this);
}

AUAppTransicionConFondo::~AUAppTransicionConFondo(){
	NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene, this);
	this->privFinalizarDatos();
}

void AUAppTransicionConFondo::privFinalizarDatos(){
	if(_datosCargaFondo.fondoSalienteCapa != NULL){
		NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaFondo.fondoSalienteCapa);
		_datosCargaFondo.fondoSalienteCapa->liberar(NB_RETENEDOR_THIS);
		_datosCargaFondo.fondoSalienteCapa = NULL;
	}
	if(_datosCargaFondo.fondoSaliente != NULL) _datosCargaFondo.fondoSaliente->liberar(NB_RETENEDOR_THIS); _datosCargaFondo.fondoSaliente = NULL;
	if(_datosCargaFondo.fondoEntranteCapa != NULL) _datosCargaFondo.fondoEntranteCapa->liberar(NB_RETENEDOR_THIS); _datosCargaFondo.fondoEntranteCapa = NULL;
	if(_datosCargaFondo.fondoEntrante != NULL) _datosCargaFondo.fondoEntrante->liberar(NB_RETENEDOR_THIS); _datosCargaFondo.fondoEntrante = NULL;
	if(_datosCargaFondo.costuraTextura != NULL) _datosCargaFondo.costuraTextura->liberar(NB_RETENEDOR_THIS); _datosCargaFondo.costuraTextura = NULL;
	if(_datosCargaFondo.costuraSprite != NULL){
		AUEscenaContenedor* contenedor = (AUEscenaContenedor*)_datosCargaFondo.costuraSprite->contenedor();
		if(contenedor != NULL) contenedor->quitarObjetoEscena(_datosCargaFondo.costuraSprite);
		_datosCargaFondo.costuraSprite->liberar(NB_RETENEDOR_THIS); _datosCargaFondo.costuraSprite = NULL;
	}
	if(_animadorObjsEscena != NULL){
		_animadorObjsEscena->purgarAnimaciones();
		_animadorObjsEscena->liberar(NB_RETENEDOR_THIS);
		_animadorObjsEscena = NULL;
	}
}

void AUAppTransicionConFondo::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	if(_animadorObjsEscena != NULL) _animadorObjsEscena->purgarAnimaciones();
	if(_datosCargaFondo.fondoEntrante != NULL) this->privEscalarFondo(_datosCargaFondo.fondoEntrante);
}

void AUAppTransicionConFondo::privEscalarFondo(AUEscenaSpritePorcion* spriteFondo){
	if(spriteFondo != NULL){
		NBCajaAABB cajaFondo	= NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Fondo);
		//Probar el calculo de fondo
		//cajaFondo.xMin += 8.0f; cajaFondo.yMin += 8.0f; cajaFondo.xMax -= 8.0f; cajaFondo.yMax -= 8.0f;
		//
		AUTextura* texFondo		= spriteFondo->textura();
		NBTamano tamCajaFondo;	NBCAJAAABB_TAMANO(tamCajaFondo, cajaFondo)
		NBTamano tamImgFondo	= texFondo->tamanoHD();
		float escalaAncho		= tamCajaFondo.ancho / tamImgFondo.ancho;
		float escalaAlto		= tamCajaFondo.alto / tamImgFondo.alto;
		float escalaUsar		= (escalaAncho > escalaAlto ? escalaAncho : escalaAlto);
		//_fondoImagen->establecerVisible(false);
		spriteFondo->redimensionar(cajaFondo.xMin, cajaFondo.yMax, tamCajaFondo.ancho, -tamCajaFondo.alto); //Invertido en ejeY
		float anchoRelExcede	= ((tamImgFondo.ancho * escalaUsar) - tamCajaFondo.ancho) / (tamImgFondo.ancho * escalaUsar); if(anchoRelExcede < 0.0f && anchoRelExcede > -0.0001f) anchoRelExcede = 0.0f; /*para tratar problemas de precision de coma flotante*/
		float altoRelExcede		= ((tamImgFondo.alto * escalaUsar) - tamCajaFondo.alto) / (tamImgFondo.ancho * escalaUsar); if(altoRelExcede < 0.0f && altoRelExcede > -0.0001f) altoRelExcede = 0.0f; /*para tratar problemas de precision de coma flotante*/
		#ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
			if(!(altoRelExcede>=0.0f) || !(anchoRelExcede>=0.0f)) PRINTF_ERROR("altoRelExcede = %f, anchoRelExcede = %f.\n", altoRelExcede, anchoRelExcede);
			NBASSERT(altoRelExcede>=0.0f)
			NBASSERT(anchoRelExcede>=0.0f)
		#endif
		spriteFondo->establecerPorcionTextura(anchoRelExcede * 0.5f, 1.0f - (anchoRelExcede * 0.5f), altoRelExcede * 0.5f, 1.0f - (altoRelExcede * 0.5f));
	}
}

//

bool AUAppTransicionConFondo::ejecutandoTransicion(){
	return (_datosCargaFondo.etapaCarga != ENEtapaCargaFondo_Ninguna);
}

bool AUAppTransicionConFondo::tickTransicion(float segsTranscurridos){
	if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_Ninguna){
		//
	} else if(_datosCargaFondo.etapaCarga == ENEtapaCargaFondo_IniciarSalidaViejo){
		//Iniciar animacion de salida
		_escuchador->escenasAnimarSalida();
		_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
	} else if(_datosCargaFondo.etapaCarga == ENEtapaCargaFondo_EsperarSalidaViejo){
		//ESPERAR A QUE SE EJECUTEN TODAS LAS ANIMACIONES DE SALIDA
		if(!_escuchador->escenasAnimandoSalida()){
			_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
		}
	} else if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_RenderConEscenaFuera){
		//ASEGURAR QUE SE RENDERICE CON LAS ANIMACIONES DE SALIDA TERMINADAS
		_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
	} else if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_CargarNuevoFondo){
		//CARGAR NUEVO FONDO y PREPARAR ESTRUCTURA
		if(_fondoRutaActual.esIgual(_fondoRutaCargar.str())){
			_datosCargaFondo.fondoEntranteCapa	= _datosCargaFondo.fondoSalienteCapa;
			_datosCargaFondo.fondoEntrante		= _datosCargaFondo.fondoSaliente;
			_datosCargaFondo.fondoSalienteCapa	= NULL;
			_datosCargaFondo.fondoSaliente		= NULL;
		} else {
			AUTextura* texFondoNuevo			= NULL;
			if(_fondoRutaCargar.tamano() != 0){
				texFondoNuevo					= NBGestorTexturas::texturaDesdeArchivo(_fondoRutaCargar.str()); NBASSERT(texFondoNuevo != NULL)
			}
			if(texFondoNuevo == NULL){
				if(_datosCargaFondo.fondoSalienteCapa != NULL){
					_animadorObjsEscena->animarVisible(_datosCargaFondo.fondoSalienteCapa, false, 0.5f, ENAnimPropVelocidad_Desacelerada3);
				}
			} else {
				_datosCargaFondo.fondoEntranteCapa	= new(this) AUEscenaContenedor();
				_datosCargaFondo.fondoEntrante		= new(this) AUEscenaSpritePorcion(texFondoNuevo); this->privEscalarFondo(_datosCargaFondo.fondoEntrante);
				_datosCargaFondo.fondoEntranteCapa->agregarObjetoEscena(_datosCargaFondo.fondoEntrante);
				if(_datosCargaFondo.fondoSalienteCapa == NULL){
					_datosCargaFondo.fondoEntranteCapa->establecerVisible(false);
					_animadorObjsEscena->animarVisible(_datosCargaFondo.fondoEntranteCapa, true, 0.5f, ENAnimPropVelocidad_Desacelerada3);
				} else {
					_datosCargaFondo.costuraTextura	= NBGestorTexturas::texturaPatronDesdeArchivoPNG("alanica/escena/fondos/objCosturaP.png"); _datosCargaFondo.costuraTextura->retener(NB_RETENEDOR_THIS);
					const NBPunto posCapaFndAct		= _datosCargaFondo.fondoSalienteCapa->traslacion();
					const NBCajaAABB cajaCapaFndAct	= _datosCargaFondo.fondoSalienteCapa->cajaAABBLocalCalculada();
					const NBPunto posFndNvo			= _datosCargaFondo.fondoEntrante->traslacion();
					const NBCajaAABB cajaFndNvo		= _datosCargaFondo.fondoEntrante->cajaAABBLocal();
					const NBTamano tamTextura		= _datosCargaFondo.costuraTextura->tamanoHD();
					_datosCargaFondo.costuraSprite	= new(this) AUEscenaFigura(ENEscenaFiguraTipo_PoligonoCerrado);
					_datosCargaFondo.costuraSprite->establecerTexturaPatron(_datosCargaFondo.costuraTextura, false);
					_datosCargaFondo.costuraSprite->establecerCoordenadasComoCaja(tamTextura.ancho * -0.5f, 0.0f, tamTextura.ancho, (cajaCapaFndAct.yMax - cajaCapaFndAct.yMin));
					_datosCargaFondo.fondoEntranteCapa->agregarObjetoEscena(_datosCargaFondo.costuraSprite);
					const NBPunto posFondoNvo(posCapaFndAct.x + (cajaCapaFndAct.xMax - cajaCapaFndAct.xMin), posCapaFndAct.y);
					const NBPunto posSalidaFondoAct(posCapaFndAct.x - (cajaCapaFndAct.xMax - cajaCapaFndAct.xMin), posCapaFndAct.y);
					_datosCargaFondo.costuraSprite->establecerVisible(false);
					_datosCargaFondo.costuraSprite->establecerTraslacion(posFndNvo.x + cajaFndNvo.xMin + (tamTextura.ancho * 0.5f), posFndNvo.y);
					_datosCargaFondo.fondoEntranteCapa->establecerTraslacion(posFondoNvo);
					_animadorObjsEscena->animarVisible(_datosCargaFondo.costuraSprite, true, 0.2f, ENAnimPropVelocidad_Lineal);
					_animadorObjsEscena->animarPosicion(_datosCargaFondo.fondoSalienteCapa, posSalidaFondoAct, 0.6f, ENAnimPropVelocidad_Desacelerada3, 0.3f);
					_animadorObjsEscena->animarPosicion(_datosCargaFondo.fondoEntranteCapa, posCapaFndAct, 0.6f, ENAnimPropVelocidad_Desacelerada3, 0.3f);
					NBGestorEscena::establecerEscenaLimpiaColorBuffer(_iScene, false);
				}
				const NBColor8 colorBlanco(255, 255, 255, 255);
				NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Fondo, _datosCargaFondo.fondoEntranteCapa, colorBlanco);
			}
		}
		_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
	} else if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_CambiarFondos){
		//ESPERAR A QUE LA ANIMACION TERMINE
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()==0){
			if(_datosCargaFondo.costuraSprite != NULL) _animadorObjsEscena->animarVisible(_datosCargaFondo.costuraSprite, false, 0.2f, ENAnimPropVelocidad_Lineal);
			_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
		}
	} else if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_QuitarExtras){
		//ESPEARAR A QUE DESAPAREZCAN LAS COSTURAS
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()==0){
			_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
		}
	} else if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_RenderFondoFinal){
		//RENDERIZAR LA ESCENA CON LOS FONDOS EN SUS POSICIONES FINALES
		_datosCargaFondo.etapaCarga = (ENEtapaCargaFondo)(_datosCargaFondo.etapaCarga + 1);
	} else if(_datosCargaFondo.etapaCarga==ENEtapaCargaFondo_CrearNuevoLiberarRecursosSinUso){
		//LIBERAR RECURSOS DE CARGA
		_escuchador->escenasHeredarFondo(_fondoRutaCargar.str(), _datosCargaFondo.fondoEntranteCapa, _datosCargaFondo.fondoEntrante, ENAppEscenasFondoModo_ajustar);
		NBGestorEscena::establecerEscenaLimpiaColorBuffer(_iScene, (_datosCargaFondo.fondoEntranteCapa == NULL)); //PENDIENTE, pasar esta instruccion al Administrador de escenas
		this->privFinalizarDatos();
		//Liberar escena actual
		_escuchador->escenasQuitarDePrimerPlano();
		_escuchador->escenasLiberar();
		//Cargar escena y precargar sus recursos
		NBGestorTexturas::modoCargaTexturasPush(ENGestorTexturaModo_cargaPendiente);
		_escuchador->escenasCargar();
		NBGestorTexturas::modoCargaTexturasPop();
		//Liberar los recursos que quedaron sin uso
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
		_escuchador->escenasColocarEnPrimerPlano();
		//FINALIZAR CARGA
		_datosCargaFondo.etapaCarga = ENEtapaCargaFondo_Ninguna;
		#ifdef CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
		NBGestorMemoria::liberarZonasSinUso();
		#endif
	}
	return false;
}

