//
//  AUAppNucleoPrecompilado.cpp
//  AUAppNucleo
//
//  Created by Nicaragua Binary on 13/03/14.
//  Copyright (c) 2014 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppTransicionConCaptura.h"

AUAppTransicionConCaptura::AUAppTransicionConCaptura(const SI32 iEscena, AUAppEscenasAdmin* escuchador, const ENGestorTexturaModo modoCargaTexturas) : AUAppTransicion(){
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppTransicionConCaptura")
	_iScene						= iEscena;
	_escuchador						= escuchador;
	_modoCargaTexturas				= modoCargaTexturas; NBASSERT(_modoCargaTexturas != ENGestorTexturaModo_cargaPendiente)
	_datosCargaCaptura.etapaCarga	= ENEtapaCargaCaptura_ninguna;
	_animadorObjsEscena				= new(this) AUAnimadorObjetoEscena();
	/*STGestorEscenaEscena pEscna	= NBGestorEscena::propiedadesEscena(iEscena);
	 //PRINTF_INFO("CREANDO BUFFER DE TRANSICION A TAM PANTALLA: (%f, %f)\n", pEscna.tamanoEscenaGL.ancho, pEscna.tamanoEscenaGL.alto);
	 _datosCargaCaptura.dimensionesCapturas.ancho	= pEscna.tamanoEscenaGL.ancho / 4.0f;
	 _datosCargaCaptura.dimensionesCapturas.alto	= pEscna.tamanoEscenaGL.alto / 4.0f;
	 _datosCargaCaptura.iCapturaEscenaSaliente		= NBGestorEscena::crearEscena(_datosCargaCaptura.dimensionesCapturas.ancho, _datosCargaCaptura.dimensionesCapturas.alto, pEscna.escalaParaHD, pEscna.escalaParaMascarasIlum, COLOR_RGBA8, ENGestorEscenaDestinoGl_Textura);
	 _datosCargaCaptura.iCapturaEscenaEntrante		= NBGestorEscena::crearEscena(_datosCargaCaptura.dimensionesCapturas.ancho, _datosCargaCaptura.dimensionesCapturas.alto, pEscna.escalaParaHD, pEscna.escalaParaMascarasIlum, COLOR_RGBA8, ENGestorEscenaDestinoGl_Textura);
	 NBGestorEscena::deshabilitarEscena(_datosCargaCaptura.iCapturaEscenaSaliente);
	 NBGestorEscena::deshabilitarEscena(_datosCargaCaptura.iCapturaEscenaEntrante);
	 NBGestorEscena::establecerFondoModo(_datosCargaCaptura.iCapturaEscenaSaliente, ENGestorEscenaFondoModo_Opaco);
	 NBGestorEscena::establecerFondoModo(_datosCargaCaptura.iCapturaEscenaEntrante, ENGestorEscenaFondoModo_Opaco);
	 NBGestorEscena::establecerColorFondo(_datosCargaCaptura.iCapturaEscenaSaliente, 0.0f, 0.0f, 0.0f, 1.0f);
	 NBGestorEscena::establecerColorFondo(_datosCargaCaptura.iCapturaEscenaEntrante, 0.0f, 0.0f, 0.0f, 1.0f);
	 _datosCargaCaptura.contenedorCapturaSaliente	= new(this) AUEscenaContenedor();
	 _datosCargaCaptura.contenedorCapturaEntrante	= new(this) AUEscenaContenedor();
	 _datosCargaCaptura.spriteCapturaSaliente		= new(this) AUEscenaSpriteMascara();
	 _datosCargaCaptura.spriteCapturaEntrante		= new(this) AUEscenaSpriteMascara();
	 _datosCargaCaptura.spriteCapturaSaliente->establecerTexturaFondo(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaSaliente).texturaEscena);
	 _datosCargaCaptura.spriteCapturaEntrante->establecerTexturaFondo(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaEntrante).texturaEscena);
	 _datosCargaCaptura.spriteCapturaSaliente->redimensionar(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaSaliente).texturaEscena);
	 _datosCargaCaptura.spriteCapturaEntrante->redimensionar(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaEntrante).texturaEscena);
	 _datosCargaCaptura.contenedorCapturaSaliente->agregarObjetoEscena(_datosCargaCaptura.spriteCapturaSaliente);
	 _datosCargaCaptura.contenedorCapturaEntrante->agregarObjetoEscena(_datosCargaCaptura.spriteCapturaEntrante);
	 _datosCargaCaptura.iAmbitoCargaTexturas		= 0;
	 _datosCargaCaptura.marcoCleanCorp				= NULL;
	 _datosCargaCaptura.marcoAlaNica				= NULL;
	 _datosCargaCaptura.etapaCarga					= (ENGameplayEstadoCargaCaptura)((SI32)ENGameplayEstadoCargaCaptura_ninguna + 1);*/
	//Carga de recursos
	_cargaConteoRecursos		= 0;
	_contenedorCarga			= new(this) AUEscenaContenedor();
	_iconoCargaAnimandoIndice	= -1;
	_iconoCargaIncrementando	= false;
	_spriteIconoEtapaCarga[0]	= new(this) AUEscenaSprite(NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoMotor01Eliminando.png"));	_contenedorCarga->agregarObjetoEscena(_spriteIconoEtapaCarga[0]); _spriteIconoEtapaCarga[0]->liberar(NB_RETENEDOR_THIS);
	_spriteIconoEtapaCarga[1]	= new(this) AUEscenaSprite(NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoMotor02Creando.png"));		_contenedorCarga->agregarObjetoEscena(_spriteIconoEtapaCarga[1]); _spriteIconoEtapaCarga[1]->liberar(NB_RETENEDOR_THIS);
	_spriteIconoEtapaCarga[2]	= new(this) AUEscenaSprite(NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoMotor03Liberando.png"));	_contenedorCarga->agregarObjetoEscena(_spriteIconoEtapaCarga[2]); _spriteIconoEtapaCarga[2]->liberar(NB_RETENEDOR_THIS);
	_spriteIconoEtapaCarga[3]	= new(this) AUEscenaSprite(NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoMotor04Cargando.png"));	_contenedorCarga->agregarObjetoEscena(_spriteIconoEtapaCarga[3]); _spriteIconoEtapaCarga[3]->liberar(NB_RETENEDOR_THIS);
	_barraProgresoMarco			= new(this) AUEscenaSpriteElastico(NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoMotorBarraMarco32.png")); _contenedorCarga->agregarObjetoEscena(_barraProgresoMarco); _barraProgresoMarco->liberar(NB_RETENEDOR_THIS);
	_barraProgresoRelleno		= new(this) AUEscenaSpriteElastico(NBGestorTexturas::texturaDesdeArchivo("Interfaces/Motor/icoMotorBarraRelleno32.png")); _contenedorCarga->agregarObjetoEscena(_barraProgresoRelleno); _barraProgresoRelleno->liberar(NB_RETENEDOR_THIS);
	_progresoRelativo			= 0.0f;
	//
	privInlineIconosMotorInicializarEstado(_datosCargaCaptura.etapaCarga);
	NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene, this);
}

AUAppTransicionConCaptura::~AUAppTransicionConCaptura(){
	NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene, this);
	//
	if(_animadorObjsEscena != NULL){
		_animadorObjsEscena->purgarAnimaciones();
		_animadorObjsEscena->liberar(NB_RETENEDOR_THIS);
		_animadorObjsEscena = NULL;
	}
	privLiberarRecursosTrasicionEscenas();
	/*if(_datosCargaCaptura.contenedorCapturaSaliente != NULL) _datosCargaCaptura.contenedorCapturaSaliente->liberar(NB_RETENEDOR_THIS); _datosCargaCaptura.contenedorCapturaSaliente = NULL;
	 if(_datosCargaCaptura.contenedorCapturaEntrante != NULL) _datosCargaCaptura.contenedorCapturaEntrante->liberar(NB_RETENEDOR_THIS); _datosCargaCaptura.contenedorCapturaEntrante = NULL;
	 if(_datosCargaCaptura.spriteCapturaSaliente != NULL) _datosCargaCaptura.spriteCapturaSaliente->liberar(NB_RETENEDOR_THIS); _datosCargaCaptura.spriteCapturaSaliente = NULL;
	 if(_datosCargaCaptura.spriteCapturaEntrante != NULL) _datosCargaCaptura.spriteCapturaEntrante->liberar(NB_RETENEDOR_THIS); _datosCargaCaptura.spriteCapturaEntrante = NULL;
	 if(_datosCargaCaptura.iCapturaEscenaSaliente!=-1){
	 NBGestorEscena::liberarEscena(_datosCargaCaptura.iCapturaEscenaSaliente);
	 _datosCargaCaptura.iCapturaEscenaSaliente = -1;
	 }
	 if(_datosCargaCaptura.iCapturaEscenaEntrante!=-1){
	 NBGestorEscena::liberarEscena(_datosCargaCaptura.iCapturaEscenaEntrante);
	 _datosCargaCaptura.iCapturaEscenaEntrante = -1;
	 }*/
}

void AUAppTransicionConCaptura::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	if(_animadorObjsEscena != NULL) _animadorObjsEscena->purgarAnimaciones();
	this->privOrganizarElementos();
}

void AUAppTransicionConCaptura::privOrganizarElementos(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privOrganizarElementos")
	//_baseRotacion		= baseRotacion;		//PENDIENTE: rehabilitar esto
	//Volver a crear escenas
	STGestorEscenaEscena pEscna = NBGestorEscena::propiedadesEscena(_iScene);
	PRINTF_INFO("CREANDO BUFFER DE TRANSICION A TAM PANTALLA: (%f, %f)\n", pEscna.tamanoEscenaGL.ancho, pEscna.tamanoEscenaGL.alto);
	//NBTamano nuevasDimensionesCapturas;
	//nuevasDimensionesCapturas.ancho	= pEscna.tamanoEscenaGL.ancho / 4.0f;
	//nuevasDimensionesCapturas.alto	= pEscna.tamanoEscenaGL.alto / 4.0f;
	/*if(_datosCargaCaptura.dimensionesCapturas!=nuevasDimensionesCapturas){
	 _datosCargaCaptura.dimensionesCapturas = nuevasDimensionesCapturas;
	 _datosCargaCaptura.spriteCapturaSaliente->establecerTexturaFondo(NULL); //liberar la textura anterior
	 _datosCargaCaptura.spriteCapturaEntrante->establecerTexturaFondo(NULL); //liberar la textura anterior
	 NBGestorEscena::redimensionarEscena(_datosCargaCaptura.iCapturaEscenaSaliente, _datosCargaCaptura.dimensionesCapturas.ancho, _datosCargaCaptura.dimensionesCapturas.alto, pEscna.escalaParaHD, pEscna.escalaParaMascarasIlum);
	 NBGestorEscena::redimensionarEscena(_datosCargaCaptura.iCapturaEscenaEntrante, _datosCargaCaptura.dimensionesCapturas.ancho, _datosCargaCaptura.dimensionesCapturas.alto, pEscna.escalaParaHD, pEscna.escalaParaMascarasIlum);
	 NBGestorEscena::normalizaCajaProyeccionEscena(_datosCargaCaptura.iCapturaEscenaSaliente);
	 NBGestorEscena::normalizaCajaProyeccionEscena(_datosCargaCaptura.iCapturaEscenaEntrante);
	 _datosCargaCaptura.spriteCapturaSaliente->redimensionar(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaSaliente).texturaEscena);
	 _datosCargaCaptura.spriteCapturaEntrante->redimensionar(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaEntrante).texturaEscena);
	 _datosCargaCaptura.spriteCapturaSaliente->establecerTexturaFondo(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaSaliente).texturaEscena);
	 _datosCargaCaptura.spriteCapturaEntrante->establecerTexturaFondo(NBGestorEscena::propiedadesEscena(_datosCargaCaptura.iCapturaEscenaEntrante).texturaEscena);
	 PRINTF_INFO("Escenas redimensionadas saliente(%d) entrante(%d) dimensiones(%f, %f)\n", _datosCargaCaptura.iCapturaEscenaSaliente, _datosCargaCaptura.iCapturaEscenaEntrante, _datosCargaCaptura.dimensionesCapturas.ancho, _datosCargaCaptura.dimensionesCapturas.alto);
	 }*/
	float anchoParaContenidoCarga = 400;
	//Iconos de la carga
	float posXIzq = (anchoParaContenidoCarga/-2.0f);
	float posYSup = (320/-2.0f)-8.0f;
	SI32 iIconoCarga;
	for(iIconoCarga=0; iIconoCarga<4; iIconoCarga++){
		NBTamano tamSpriteBoton = _spriteIconoEtapaCarga[iIconoCarga]->textura()->tamanoHD();
		_spriteIconoEtapaCarga[iIconoCarga]->establecerTraslacion(posXIzq+(tamSpriteBoton.ancho/2.0f), posYSup-(tamSpriteBoton.alto/2.0f));
		posXIzq += tamSpriteBoton.ancho + 8.0f;
	}
	//Bara de progreso de carga
	float anchoSobrante = (anchoParaContenidoCarga/2.0f)-posXIzq;
	const NBTamano tamTexMarcoProg	= _barraProgresoMarco->textura()->tamanoHD();
	//NBCajaAABB cajaMinimaMarcoProgeso = _barraProgresoMarco->cajaMinima();
	NBCajaAABB cajaMarcoProgeso;
	cajaMarcoProgeso.xMin = 0.0f;
	cajaMarcoProgeso.xMax = anchoSobrante;
	cajaMarcoProgeso.yMax = tamTexMarcoProg.alto;
	cajaMarcoProgeso.yMin = 0.0f;
	_barraProgresoMarco->redimensionar(cajaMarcoProgeso.xMin, cajaMarcoProgeso.yMax, cajaMarcoProgeso.xMax - cajaMarcoProgeso.xMin, cajaMarcoProgeso.yMin - cajaMarcoProgeso.yMax);
	//_barraProgresoMarco->establecerCajaMarcoMalla(cajaMarcoProgeso);
	_barraProgresoMarco->establecerTraslacion(posXIzq, posYSup+cajaMarcoProgeso.yMin);
	_barraProgresoRelleno->establecerTraslacion(_barraProgresoMarco->traslacion());
	privInlineIconosMotorMarcarProgresoCarga(_progresoRelativo);
	//
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUAppTransicionConCaptura::ejecutandoTransicion(){
	return (_datosCargaCaptura.etapaCarga != ENEtapaCargaCaptura_ninguna);
}

SI32 AUAppTransicionConCaptura::privCargarRecursoPendiente(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privCargarRecursoPendiente")
	NBGestorTexturas::modoCargaTexturasPush((_modoCargaTexturas == ENGestorTexturaModo_cargaPendiente ? ENGestorTexturaModo_cargaInmediata : _modoCargaTexturas));
	if(NBGestorTexturas::texPendienteOrganizarConteo()!=0)		NBGestorTexturas::texPendienteOrganizarProcesar(1);
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	else if(NBGestorSonidos::conteoBufferesPendientesDeCargar()!=0)		NBGestorSonidos::cargarBufferesPendientesDeCargar(1);
#	endif
	NBGestorTexturas::modoCargaTexturasPop();
	//Actualizar la presentacion
	SI32 cantidadRestante = 0;
	cantidadRestante += NBGestorTexturas::texPendienteOrganizarConteo();
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	cantidadRestante += NBGestorSonidos::conteoBufferesPendientesDeCargar();
#	endif
	if(_cargaConteoRecursos!=0){
		privInlineIconosMotorMarcarProgresoCarga(((float)_cargaConteoRecursos-(float)cantidadRestante)/(float)_cargaConteoRecursos);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return  cantidadRestante;
}

UI8 AUAppTransicionConCaptura::privCargarRecursosTransicionEscenas(SI32 indiceTransicion){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privCargarRecursosTransicionEscenas")
	NBASSERT(_datosCargaCaptura.marcoCleanCorp == NULL && _datosCargaCaptura.marcoAlaNica == NULL);
	UI8 ambitoCargaTexturas		= NBGestorTexturas::reservarAmbitoTexturas(512, 512); NBASSERT(ambitoCargaTexturas!=0);
	NBGestorTexturas::activarAmbitoTexturas(ambitoCargaTexturas);
	if(indiceTransicion==0){
		AUTextura* texMascara = NBGestorTexturas::texturaDesdeArchivo("Interfaces/AdministradorEscenas/cuadroCleanCorpMascara.png");
		_datosCargaCaptura.spriteCapturaSaliente->establecerTexturaMascara(texMascara);
		_datosCargaCaptura.spriteCapturaEntrante->establecerTexturaMascara(texMascara);
		_datosCargaCaptura.marcoCleanCorp			= NBGestorCuerpos::cuerpoDesdeArchivo(this->tipoMemoriaResidencia(), "AdministradorEscenas/marcoCleanCorp.cuerpo"); //es autoliberado
		NBASSERT(_datosCargaCaptura.marcoCleanCorp != NULL); //Falla en carga de archivo
		if(_datosCargaCaptura.marcoCleanCorp != NULL) _datosCargaCaptura.marcoCleanCorp->retener(NB_RETENEDOR_THIS);
	} else {
		AUTextura* texMascara = NBGestorTexturas::texturaDesdeArchivo("Interfaces/AdministradorEscenas/cuadroDodoTownMascara.png");
		_datosCargaCaptura.spriteCapturaSaliente->establecerTexturaMascara(texMascara);
		_datosCargaCaptura.spriteCapturaEntrante->establecerTexturaMascara(texMascara);
		_datosCargaCaptura.marcoAlaNica			= NBGestorCuerpos::cuerpoDesdeArchivo(this->tipoMemoriaResidencia(), "AdministradorEscenas/marcoDodoTown.cuerpo"); //es autoliberado
		//NBASSERT(_datosCargaCaptura.marcoAlaNica != NULL); //Falla en carga de archivo
		if(_datosCargaCaptura.marcoAlaNica != NULL) _datosCargaCaptura.marcoAlaNica->retener(NB_RETENEDOR_THIS);
	}
	NBGestorTexturas::generarMipMapsDeTexturas(ambitoCargaTexturas);
	NBGestorTexturas::activarAmbitoTexturas(0);
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return ambitoCargaTexturas;
}

void AUAppTransicionConCaptura::privLiberarRecursosTrasicionEscenas(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privLiberarRecursosTrasicionEscenas")
	/*if(_datosCargaCaptura.marcoCleanCorp != NULL){
	 _datosCargaCaptura.marcoCleanCorp->liberar(NB_RETENEDOR_THIS);
	 _datosCargaCaptura.marcoCleanCorp = NULL;
	 }
	 if(_datosCargaCaptura.marcoAlaNica != NULL){
	 _datosCargaCaptura.marcoAlaNica->liberar(NB_RETENEDOR_THIS);
	 _datosCargaCaptura.marcoAlaNica = NULL;
	 }*/
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUAppTransicionConCaptura::privInlineIconosMotorInicializarEstado(SI32 indiceEtapaCarga){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privInlineIconosMotorInicializarEstado")
	SI32 indiceEquivalenteIconoCarga = 0;
	if(indiceEtapaCarga<=ENEtapaCargaCaptura_animarSalidaEscenaSaliente_Alpha){
		indiceEquivalenteIconoCarga = 0;
	} else if(indiceEtapaCarga<=ENEtapaCargaCaptura_previoCrearNuevo){
		indiceEquivalenteIconoCarga = 1;
	} else if(indiceEtapaCarga<=ENEtapaCargaCaptura_previoLiberarRecursosSinUso){
		indiceEquivalenteIconoCarga = 2;
	} else {
		indiceEquivalenteIconoCarga = 3;
	}
	SI32 iIconoCarga;
	for(iIconoCarga=0; iIconoCarga<4; iIconoCarga++){
		_spriteIconoEtapaCarga[iIconoCarga]->establecerMultiplicadorColor(1.0f, 1.0f, 1.0f, indiceEquivalenteIconoCarga>=iIconoCarga?1.0f:0.5f);
	}
	_iconoCargaAnimandoIndice	= indiceEquivalenteIconoCarga;
	_iconoCargaIncrementando	= false;
	privInlineIconosMotorMarcarProgresoCarga(0.0f);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUAppTransicionConCaptura::privInlineIconosMotorMarcarProgresoCarga(float progesoCeroHaciaUno){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privInlineIconosMotorMarcarProgresoCarga")
	if(_iconoCargaAnimandoIndice>=0 && _iconoCargaAnimandoIndice<4){
		const NBRectangulo limitesMarcoProg = _barraProgresoMarco->limites();
		//NBCajaAABB cajaMarco = _barraProgresoMarco->cajaMarcoMalla();
		//NBTamano tamCajaMarco; NBCAJAAABB_TAMANO(tamCajaMarco, cajaMarco);
		const NBTamano tamCajaMinimaRelleno	= _barraProgresoRelleno->textura()->tamanoHD();
		//NBCajaAABB cajaMinimaRelleno = _barraProgresoRelleno->cajaMinima();
		//NBTamano tamCajaMinimaRelleno; NBCAJAAABB_TAMANO(tamCajaMinimaRelleno, cajaMinimaRelleno);
		//
		NBCajaAABB cajaRellenoProgreso;
		cajaRellenoProgreso.yMin = limitesMarcoProg.y;
		cajaRellenoProgreso.yMax = limitesMarcoProg.y + limitesMarcoProg.alto;
		cajaRellenoProgreso.xMin = limitesMarcoProg.x;
		cajaRellenoProgreso.xMax = limitesMarcoProg.x + tamCajaMinimaRelleno.ancho + ((limitesMarcoProg.ancho - tamCajaMinimaRelleno.ancho) * progesoCeroHaciaUno);
		_barraProgresoRelleno->redimensionar(cajaRellenoProgreso.xMin, cajaRellenoProgreso.yMax, cajaRellenoProgreso.xMax - cajaRellenoProgreso.xMin, cajaRellenoProgreso.yMin - cajaRellenoProgreso.yMax);
		//_barraProgresoRelleno->establecerCajaMarcoMalla(cajaRellenoProgreso);
	}
	_progresoRelativo = progesoCeroHaciaUno;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUAppTransicionConCaptura::privInlineIconosMotorTickAnimacion(float segsTranscurridos){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppTransicionConCaptura::privInlineIconosMotorTickAnimacion")
	if(_iconoCargaAnimandoIndice>=0 && _iconoCargaAnimandoIndice<4){
		float alpha = (float)_spriteIconoEtapaCarga[_iconoCargaAnimandoIndice]->_propiedades.color8.a/255.0f;
		if(_iconoCargaIncrementando){
			alpha += (2.0f * segsTranscurridos);
			if(alpha>1.0f){
				alpha = 1.0f; _iconoCargaIncrementando = !_iconoCargaIncrementando;
			}
		} else {
			alpha -= (2.0f * segsTranscurridos);
			if(alpha<0.5f){
				alpha = 0.5f; _iconoCargaIncrementando = !_iconoCargaIncrementando;
			}
		}
		_spriteIconoEtapaCarga[_iconoCargaAnimandoIndice]->establecerMultiplicadorAlpha8(255.0f*alpha);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

bool AUAppTransicionConCaptura::tickTransicion(float segsTranscurridos){
	float duracionAnimaciones = 0.33f;
	if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_ninguna){
		//nada
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_capturarEscenaSaliente){
		//Cargar elementos necesarios para transicion
		_datosCargaCaptura.indiceMarcoEnUso			= (rand()%2);
		_datosCargaCaptura.iAmbitoCargaTexturas		= privCargarRecursosTransicionEscenas(_datosCargaCaptura.indiceMarcoEnUso);
		//
		_datosCargaCaptura.permitidoGirarPantallaAntesDeIniciarTransicion = NBGestorEscena::pantallaPermitidoRotar();
		NBGestorEscena::pantallaEstablecerPermitidoRotar(false);
		NBGestorAnimadores::establecerAnimadorActivo(false);
		//
		NBCajaAABB cajaEscena	= NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
		//NBTamano tamCajaEscena;	NBCAJAAABB_TAMANO(tamCajaEscena, cajaEscena);
		NBPunto centroCajaEscena; NBCAJAAABB_CENTRO(centroCajaEscena, cajaEscena);
		//
		//Capturar la escena saliente
		STGestorEscenaEscena datosEscena			= NBGestorEscena::propiedadesEscena(_iScene);
		SI32 iGrp;
		for(iGrp=0; iGrp<ENGestorEscenaGrupo_Conteo; iGrp++){
			if(iGrp==ENGestorEscenaGrupo_Escena || iGrp==ENGestorEscenaGrupo_GUI){
				AUArregloNativoP<STGestorEscenaCapa>* capas = datosEscena.gruposCapas[iGrp].capas;
				UI32 iCapa; for(iCapa=0; iCapa<capas->conteo; iCapa++){
					NBGestorEscena::enlaceCapaAgregar(_datosCargaCaptura.iCapturaEscenaSaliente, _iScene, capas->elemento[iCapa].idCapa);
				}
			}
		}
		//NBGestorEscena::enlazarEscenas(_datosCargaCaptura.iCapturaEscenaSaliente, _iScene);
		NBGestorEscena::copiarCajasDeGrupos(_iScene, _datosCargaCaptura.iCapturaEscenaSaliente);
		NBGestorEscena::establecerColorFondo(_datosCargaCaptura.iCapturaEscenaSaliente, NBGestorEscena::colorFondo(_iScene));
		NBGestorEscena::habilitarEscena(_datosCargaCaptura.iCapturaEscenaSaliente);
		_datosCargaCaptura.secuencialRenderEspera = NBGestorEscena::secuencialRenderizadasModelosGL();
		//
		_datosCargaCaptura.spriteCapturaSaliente->establecerMultiplicadorColor8(255, 255, 255, 0);
		_datosCargaCaptura.spriteCapturaSaliente->establecerTraslacion(centroCajaEscena.x, centroCajaEscena.y);
		_datosCargaCaptura.spriteCapturaSaliente->establecerEscalacion(4.0f, 4.0f);
		_datosCargaCaptura.spriteCapturaSaliente->establecerRotacion(0.0f);
		if(_datosCargaCaptura.indiceMarcoEnUso==0){
			if(_datosCargaCaptura.marcoCleanCorp != NULL) _datosCargaCaptura.marcoCleanCorp->establecerTraslacion(centroCajaEscena.x, centroCajaEscena.y);
		} else {
			if(_datosCargaCaptura.marcoAlaNica != NULL) _datosCargaCaptura.marcoAlaNica->establecerTraslacion(centroCajaEscena.x, centroCajaEscena.y);
		}
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_esperarCapturaEscenaSaliente){
		//Garantizar que se haya renderizado por lo menos una vez
		if(_datosCargaCaptura.secuencialRenderEspera!=NBGestorEscena::secuencialRenderizadasModelosGL()){
			NBGestorEscena::deshabilitarEscena(_datosCargaCaptura.iCapturaEscenaSaliente);
			NBGestorEscena::enlaceCapaVaciar(_datosCargaCaptura.iCapturaEscenaSaliente);
			NBGestorEscena::vaciarGrupos(_datosCargaCaptura.iCapturaEscenaSaliente);
			NBColor8 coloLuzAmbienteCapa; NBCOLOR_ESTABLECER(coloLuzAmbienteCapa, 255, 255, 255, 255)
			NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.contenedorCapturaSaliente, coloLuzAmbienteCapa);
			_animadorObjsEscena->animarColorMult(_datosCargaCaptura.spriteCapturaSaliente, 255, 255, 255, 255, duracionAnimaciones/2.0f, ENAnimPropVelocidad_Acelerada);
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_animarSalidaEscenaSaliente_Alpha){
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()!=0){
			_animadorObjsEscena->tickAnimacion(segsTranscurridos); //El animador principal esta detenido
		} else {
			//Quitar escena saliente de primer plano
			_escuchador->escenasQuitarDePrimerPlano();
			NBGestorEscena::establecerColorFondo(_iScene, 0.0f, 0.0f, 0.0f, 1.0f);
			privInlineIconosMotorInicializarEstado((SI32)_datosCargaCaptura.etapaCarga);
			//
			NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCaptura.contenedorCapturaSaliente); //Para que quede encima del marco
			NBColor8 coloLuzAmbienteCapa; NBCOLOR_ESTABLECER(coloLuzAmbienteCapa, 255, 255, 255, 255)
			NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.indiceMarcoEnUso==0?_datosCargaCaptura.marcoCleanCorp:_datosCargaCaptura.marcoAlaNica, coloLuzAmbienteCapa);
			NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.contenedorCapturaSaliente, coloLuzAmbienteCapa);
			NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _contenedorCarga, coloLuzAmbienteCapa);
			//
			_animadorObjsEscena->animarEscalacion(_datosCargaCaptura.spriteCapturaSaliente, 1.f, 1.0f, duracionAnimaciones, ENAnimPropVelocidad_Acelerada);
			if(_datosCargaCaptura.indiceMarcoEnUso==0){
				_animadorObjsEscena->animarPosicion(_datosCargaCaptura.spriteCapturaSaliente, 92.0f, -20.0f, duracionAnimaciones, ENAnimPropVelocidad_Acelerada);
				_animadorObjsEscena->animarRotacion(_datosCargaCaptura.spriteCapturaSaliente, -8.0f, duracionAnimaciones, ENAnimPropVelocidad_Acelerada);
			} else {
				_animadorObjsEscena->animarPosicion(_datosCargaCaptura.spriteCapturaSaliente, 0.0f, -15.0f, duracionAnimaciones, ENAnimPropVelocidad_Acelerada);
				_animadorObjsEscena->animarRotacion(_datosCargaCaptura.spriteCapturaSaliente, (rand()%20)-10, duracionAnimaciones, ENAnimPropVelocidad_Acelerada);
			}
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_animarSalidaEscenaSaliente){
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()!=0){
			_animadorObjsEscena->tickAnimacion(segsTranscurridos); //El animador principal esta detenido
		} else {
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
			privInlineIconosMotorInicializarEstado((SI32)_datosCargaCaptura.etapaCarga);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_previoCrearNuevo){
		//Tick solo para que se actualice la escena
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_crearNuevo){
		_escuchador->escenasLiberar();
		NBGestorEscena::establecerColorFondo(_iScene, 0.0f, 0.0f, 0.0f, 1.0f);
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		NBGestorSonidos::establecerModoDeCarga(ENGestorSonidoModo_cargaPendiente);
#		endif
		NBGestorTexturas::modoCargaTexturasPush(ENGestorTexturaModo_cargaPendiente);
		NBGestorAnimadores::establecerModoAgregarNuevos(ENGestorAnimadoresModo_agregarInhabilitados);
		_escuchador->escenasCargar();
		_escuchador->escenasQuitarDePrimerPlano();
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		NBGestorSonidos::establecerModoDeCarga(ENGestorSonidoModo_cargaInmediata);
#		endif
		NBGestorTexturas::modoCargaTexturasPop();
		NBGestorAnimadores::establecerModoAgregarNuevos(ENGestorAnimadoresModo_agregarHabilitados);
		_cargaConteoRecursos	= 0;
		_cargaConteoRecursos	+= NBGestorTexturas::texPendienteOrganizarConteo();
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		_cargaConteoRecursos	+= NBGestorSonidos::conteoBufferesPendientesDeCargar();
#		endif
		//
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		privInlineIconosMotorInicializarEstado((SI32)_datosCargaCaptura.etapaCarga);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_previoLiberarRecursosSinUso){
		//Tick solo para que se actualice la escena
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_liberarRecursosSinUso){
		_escuchador->escenasLiberarRecursosSinUso();
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		privInlineIconosMotorInicializarEstado((SI32)_datosCargaCaptura.etapaCarga);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_cargarRecursosPendientes){
		SI32 cantidadRestante = privCargarRecursoPendiente();
		if(cantidadRestante==0){
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
			privInlineIconosMotorInicializarEstado((SI32)_datosCargaCaptura.etapaCarga);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_generarMipmaps){
		NBGestorTexturas::generarMipMapsDeTexturas();
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_capturarEscenaNueva){
		NBGestorEscena::normalizaCajaProyeccionEscena(_iScene);
		NBGestorEscena::normalizaMatricesCapasEscena(_iScene);
		_escuchador->escenasColocarEnPrimerPlano();
		NBGestorEscena::quitarObjetoCapa(_iScene, _contenedorCarga);
		privInlineIconosMotorInicializarEstado((SI32)_datosCargaCaptura.etapaCarga);
		NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCaptura.indiceMarcoEnUso==0?_datosCargaCaptura.marcoCleanCorp:_datosCargaCaptura.marcoAlaNica); //quitar para que no aparezca en la captura de la nueva
		NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCaptura.contenedorCapturaSaliente); //quitar para que no aparezca en la captura de la nueva
		NBGestorEscena::moverCapasHaciaEscena(_iScene, _datosCargaCaptura.iCapturaEscenaEntrante);
		NBGestorEscena::copiarCajasDeGrupos(_iScene, _datosCargaCaptura.iCapturaEscenaEntrante);
		NBColor colorFondoEscena = NBGestorEscena::colorFondo(_iScene); PRINTF_INFO("COLOR FONDO ESCENA (%f, %f, %f, %f)\n", colorFondoEscena.r, colorFondoEscena.g, colorFondoEscena.b, colorFondoEscena.a);
		NBGestorEscena::establecerColorFondo(_datosCargaCaptura.iCapturaEscenaEntrante, colorFondoEscena);
		NBGestorEscena::establecerColorFondo(_iScene, 0.0f, 0.0f, 0.0f, 1.0f);
		NBColor8 coloLuzAmbienteCapa; NBCOLOR_ESTABLECER(coloLuzAmbienteCapa, 255, 255, 255, 255)
		NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.indiceMarcoEnUso==0?_datosCargaCaptura.marcoCleanCorp:_datosCargaCaptura.marcoAlaNica, coloLuzAmbienteCapa); //quitar para que no aparezca en la captura de la nueva
		NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.contenedorCapturaSaliente, coloLuzAmbienteCapa); //quitar para que no aparezca en la captura de la nueva
		NBGestorEscena::habilitarEscena(_datosCargaCaptura.iCapturaEscenaEntrante);
		_datosCargaCaptura.secuencialRenderEspera = NBGestorEscena::secuencialRenderizadasModelosGL();
		//
		_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_esperarCapturaEscenaNueva){
		//Garantizar que se haya renderizado por lo menos una vez
		if(_datosCargaCaptura.secuencialRenderEspera!=NBGestorEscena::secuencialRenderizadasModelosGL()){
			//NBCajaAABB cajaEscena	= NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
			//NBTamano tamCajaEscena;	NBCAJAAABB_TAMANO(tamCajaEscena, cajaEscena);
			//NBPunto centroCajaEscena; NBCAJAAABB_CENTRO(centroCajaEscena, cajaEscena);
			NBColor8 colorCapturaSaliente = _datosCargaCaptura.spriteCapturaSaliente->_propiedades.color8;
			NBColor colorFondoEscena = NBGestorEscena::colorFondo(_datosCargaCaptura.iCapturaEscenaEntrante); PRINTF_INFO("COLOR FONDO ESCENA (%f, %f, %f, %f)\n", colorFondoEscena.r, colorFondoEscena.g, colorFondoEscena.b, colorFondoEscena.a);
			_datosCargaCaptura.spriteCapturaEntrante->establecerMultiplicadorColor8(colorCapturaSaliente.r, colorCapturaSaliente.g, colorCapturaSaliente.b, 0.0f);
			_datosCargaCaptura.spriteCapturaEntrante->establecerTraslacion(_datosCargaCaptura.spriteCapturaSaliente->traslacion());
			_datosCargaCaptura.spriteCapturaEntrante->establecerEscalacion(_datosCargaCaptura.spriteCapturaSaliente->escalacion());
			_datosCargaCaptura.spriteCapturaEntrante->establecerRotacion(_datosCargaCaptura.spriteCapturaSaliente->rotacion());
			NBColor8 coloLuzAmbienteCapa; NBCOLOR_ESTABLECER(coloLuzAmbienteCapa, 255, 255, 255, 255)
			//NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.indiceMarcoEnUso==0?_datosCargaCaptura.marcoCleanCorp:_datosCargaCaptura.marcoAlaNica);
			//NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.spriteCapturaSaliente);
			NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_Cortina, _datosCargaCaptura.contenedorCapturaEntrante, coloLuzAmbienteCapa);
			//
			//privQuitarEscenasDePrimerPlano(); NBGestorEscena::establecerColorFondo(_iScene, 0.0f, 0.0f, 0.0f, 1.0f);
			//
			_animadorObjsEscena->animarColorMult(_datosCargaCaptura.spriteCapturaEntrante, colorCapturaSaliente.r, colorCapturaSaliente.g, colorCapturaSaliente.b, 255, duracionAnimaciones);
			//
			NBGestorEscena::deshabilitarEscena(_datosCargaCaptura.iCapturaEscenaEntrante);
			//NBGestorEscena::vaciarGrupos(_datosCargaCaptura.iCapturaEscenaEntrante);
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_animarEntradaEscenaNueva_Alpha){
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()!=0){
			_animadorObjsEscena->tickAnimacion(segsTranscurridos); //El animador principal esta detenido
		} else {
			NBCajaAABB cajaEscena	= NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_Cortina);
			//NBTamano tamCajaEscena;	NBCAJAAABB_TAMANO(tamCajaEscena, cajaEscena);
			NBPunto centroCajaEscena; NBCAJAAABB_CENTRO(centroCajaEscena, cajaEscena);
			NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCaptura.contenedorCapturaSaliente);
			_animadorObjsEscena->animarColorMult(_datosCargaCaptura.spriteCapturaEntrante, 255, 255, 255, 255, duracionAnimaciones, ENAnimPropVelocidad_Desacelerada);
			_animadorObjsEscena->animarPosicion(_datosCargaCaptura.spriteCapturaEntrante, centroCajaEscena.x, centroCajaEscena.y, duracionAnimaciones, ENAnimPropVelocidad_Desacelerada);
			_animadorObjsEscena->animarEscalacion(_datosCargaCaptura.spriteCapturaEntrante, 4.0f, 4.0f, duracionAnimaciones, ENAnimPropVelocidad_Desacelerada);
			_animadorObjsEscena->animarRotacion(_datosCargaCaptura.spriteCapturaEntrante, 0.0f, duracionAnimaciones, ENAnimPropVelocidad_Desacelerada);
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_animarEntradaEscenaNueva){
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()!=0){
			_animadorObjsEscena->tickAnimacion(segsTranscurridos); //El animador principal esta detenido
		} else {
			_animadorObjsEscena->animarColorMult(_datosCargaCaptura.spriteCapturaEntrante, 255, 255, 255, 0, duracionAnimaciones/2.0f, ENAnimPropVelocidad_Desacelerada);
			//
			NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCaptura.indiceMarcoEnUso==0?_datosCargaCaptura.marcoCleanCorp:_datosCargaCaptura.marcoAlaNica);
			NBGestorEscena::moverCapasHaciaEscena(_datosCargaCaptura.iCapturaEscenaEntrante, _iScene);
			NBGestorEscena::copiarCajasDeGrupos(_datosCargaCaptura.iCapturaEscenaEntrante, _iScene);
			NBGestorEscena::establecerColorFondo(_iScene, NBGestorEscena::colorFondo(_datosCargaCaptura.iCapturaEscenaEntrante));
			_escuchador->escenasColocarEnPrimerPlano();
			_datosCargaCaptura.etapaCarga = (ENEtapaCargaCaptura)(_datosCargaCaptura.etapaCarga + 1);
		}
	} else if(_datosCargaCaptura.etapaCarga==ENEtapaCargaCaptura_animarSalidaEscenaNueva_Alpha){
		if(_animadorObjsEscena->conteoAnimacionesEjecutando()!=0){
			_animadorObjsEscena->tickAnimacion(segsTranscurridos); //El animador principal esta detenido
		} else {
			NBGestorAnimadores::habilitarAnimadoresTodos();
			NBGestorAnimadores::establecerAnimadorActivo(true);
			NBGestorEscena::pantallaEstablecerPermitidoRotar(_datosCargaCaptura.permitidoGirarPantallaAntesDeIniciarTransicion);
			NBGestorEscena::quitarObjetoCapa(_iScene, _datosCargaCaptura.contenedorCapturaEntrante);
			_datosCargaCaptura.spriteCapturaSaliente->establecerTexturaMascara(NULL);
			_datosCargaCaptura.spriteCapturaEntrante->establecerTexturaMascara(NULL);
			privLiberarRecursosTrasicionEscenas();
			_escuchador->escenasLiberarRecursosSinUso();
			NBGestorTexturas::liberarAmbitoTexturas(_datosCargaCaptura.iAmbitoCargaTexturas);
			_datosCargaCaptura.iAmbitoCargaTexturas = 0;
			_datosCargaCaptura.etapaCarga = ENEtapaCargaCaptura_ninguna;
			#ifdef CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
			NBGestorMemoria::liberarZonasSinUso();
			#endif
		}
	}
	return false; //PENDIENTE: optimizar para que se carguen avrios recursos dentro de un tickTransicion
}


