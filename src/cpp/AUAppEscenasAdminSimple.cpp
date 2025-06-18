//
//  AUAppEscenasAdminSimple.cpp
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppEscenasAdminSimple.h"

AUAppEscenasAdminSimple::AUAppEscenasAdminSimple(const SI32 iEscena, const ENGestorTexturaModo modoCargaTexturas, const char* prefijoPaquetes, const STAppScenasPkgLoadDef* paquetesCargar, const SI32 conteoPaquetesCargar)
: AUAppEscenasAdmin(iEscena, prefijoPaquetes, paquetesCargar, conteoPaquetesCargar) {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdminSimple::AUAppEscenasAdminSimple")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppEscenasAdminSimple")
	_escenaActiva = NULL;
	_escenaCargar = NULL;
	_modoCargaTexturas = modoCargaTexturas; NBASSERT(_modoCargaTexturas != ENGestorTexturaModo_cargaPendiente)
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUAppEscenasAdminSimple::~AUAppEscenasAdminSimple(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdminSimple::~AUAppEscenasAdminSimple")
	if(_escenaActiva != NULL) _escenaActiva->liberar(NB_RETENEDOR_THIS); _escenaActiva = NULL;
	if(_escenaCargar != NULL) _escenaCargar->liberar(NB_RETENEDOR_THIS); _escenaCargar = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

bool AUAppEscenasAdminSimple::permitidoGirarPantalla(){
	if(_escenaActiva != NULL) return _escenaActiva->escenaPermitidoGirarPantalla();
	return false;
}

//

bool AUAppEscenasAdminSimple::tickProcesoCarga(float segsTranscurridos){
	if(_escenaCargar != NULL){
		
	}
	return false;
}

//TECLAS
bool AUAppEscenasAdminSimple::teclaPresionada(SI32 codigoTecla){
	if(_escenaActiva != NULL) return _escenaActiva->teclaPresionada(codigoTecla);
	return false;
}

bool AUAppEscenasAdminSimple::teclaLevantada(SI32 codigoTecla){
	if(_escenaActiva != NULL) return _escenaActiva->teclaLevantada(codigoTecla);
	return false;
}

bool AUAppEscenasAdminSimple::teclaEspecialPresionada(SI32 codigoTecla){
	if(_escenaActiva != NULL) return _escenaActiva->teclaEspecialPresionada(codigoTecla);
	return false;
}

//

void AUAppEscenasAdminSimple::escenaCargar(AUAppEscena* objEscena){
	if(_escenaCargar != NULL) _escenaCargar->liberar(NB_RETENEDOR_THIS);
	_escenaCargar = objEscena;
	if(_escenaCargar != NULL) _escenaCargar->retener(NB_RETENEDOR_THIS);
}

void AUAppEscenasAdminSimple::escenasCargar(){
	if(_escenaActiva != NULL) _escenaActiva->liberar(NB_RETENEDOR_THIS); _escenaActiva = NULL;
	_escenaActiva = _escenaCargar;
	_escenaCargar = NULL; //Hereda la retencion
}

void AUAppEscenasAdminSimple::escenasLiberar(){
	if(_escenaActiva != NULL) _escenaActiva->liberar(NB_RETENEDOR_THIS); _escenaActiva = NULL;
}

void AUAppEscenasAdminSimple::escenasLiberarRecursosSinUso(){
	//Nada
}

//

bool AUAppEscenasAdminSimple::escenasEnProcesoDeCarga(){
	return false; //(_escenaCargar != NULL);
}

bool AUAppEscenasAdminSimple::escenasTieneAccionesPendiente(){
	return (_escenaCargar != NULL);
}

void AUAppEscenasAdminSimple::escenasEjecutaAccionesPendientes(){
	if(_escenaCargar != NULL){
		//Liberar escena actual
		this->escenasQuitarDePrimerPlano();
		this->escenasLiberar();
		//Cargar escena y precargar sus recursos
		NBGestorTexturas::modoCargaTexturasPush(ENGestorTexturaModo_cargaPendiente);
		this->escenasCargar();
		NBGestorTexturas::modoCargaTexturasPop();
		//Liberar los recursos que quedaron sin uso
		this->escenasLiberarRecursosSinUso();
		//Cargar recursos pendientes
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		while(NBGestorSonidos::conteoBufferesPendientesDeCargar()!=0){ NBGestorSonidos::cargarBufferesPendientesDeCargar(9999); }
#		endif
		NBGestorTexturas::modoCargaTexturasPush((_modoCargaTexturas == ENGestorTexturaModo_cargaPendiente ? ENGestorTexturaModo_cargaInmediata : _modoCargaTexturas));
		while(NBGestorTexturas::texPendienteOrganizarConteo()!=0){ NBGestorTexturas::texPendienteOrganizarProcesar(9999); }
		NBGestorTexturas::modoCargaTexturasPop();
		//Finalizar proceso
		NBGestorTexturas::generarMipMapsDeTexturas();
		//Colocar nueva escena en primer plano
		this->escenasColocarEnPrimerPlano();
		#ifdef CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA
		NBGestorMemoria::liberarZonasSinUso();
		#endif
	}
}

//

void AUAppEscenasAdminSimple::escenasQuitarDePrimerPlano(){
	if(_escenaActiva != NULL) _escenaActiva->escenaQuitarDePrimerPlano();
}

void AUAppEscenasAdminSimple::escenasColocarEnPrimerPlano(){
	if(_escenaActiva != NULL) _escenaActiva->escenaColocarEnPrimerPlano();
}

//

bool AUAppEscenasAdminSimple::escenasAnimandoSalida(){
	if(_escenaActiva != NULL) _escenaActiva->escenaAnimarSalida();
	return false;
}

void AUAppEscenasAdminSimple::escenasAnimarSalida(){
	if(_escenaActiva != NULL) _escenaActiva->escenaAnimarSalida();
}

void AUAppEscenasAdminSimple::escenasAnimarEntrada(){
	if(_escenaActiva != NULL) _escenaActiva->escenaAnimarEntrada();
}

//

AUOBJMETODOS_CLASESID_MULTICLASE(AUAppEscenasAdminSimple, AUAppEscenasAdmin)
AUOBJMETODOS_CLASESNOMBRES_MULTICLASE(AUAppEscenasAdminSimple, "AUAppEscenasAdminSimple", AUAppEscenasAdmin)
AUOBJMETODOS_CLONAR_NULL(AUAppEscenasAdminSimple)






