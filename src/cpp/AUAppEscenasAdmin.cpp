//
//  AUAppEscenasAdmin.cpp
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppEscenasAdmin.h"

AUAppEscenasAdmin::AUAppEscenasAdmin(const SI32 iEscena, const char* prefijoPaquetes, const STAppScenasPkgLoadDef* paquetesCargar, const SI32 conteoPaquetesCargar)
: AUObjeto(), _fondoRuta(this) {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdmin::AUAppEscenasAdmin")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUAppEscenasAdmin")
	_iScene		= iEscena;
	_fondoModo		= ENAppEscenasFondoModo_ajustar;
	_fondoCapa		= NULL;
	_fondoSprite	= NULL;
	//Cargar paquetes
	if(conteoPaquetesCargar <= 0){
		PRINTF_INFO("Ningún paquete a cargar.\n");
	} else {
		SI32 conteoPaquetesCargados = 0;
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal)AUCadenaMutable8();
		SI32 i;
		for(i = 0; i < conteoPaquetesCargar; i++){
			const STAppScenasPkgLoadDef* pkgDef = &paquetesCargar[i];
			if(pkgDef->data != NULL && pkgDef->dataSz > 0){
				//Load from persistent data
				strTmp->establecer(pkgDef->filePath);
				if(!NBGestorArchivos::cargarPaqueteFromPersistentData(pkgDef->filePath, pkgDef->data, pkgDef->dataSz)){
					PRINTF_ERROR("ERROR, pre-cargando paquete desde datos persistentes: '%s'\n", pkgDef->filePath);
				} else {
					conteoPaquetesCargados++;
				}
			} else {
				//Load form file
				strTmp->establecer(prefijoPaquetes); strTmp->agregar(pkgDef->filePath);
				if(!NBGestorArchivos::cargarPaquete(NBGestorArchivos::rutaHaciaRecursoEnPaquete(strTmp->str()))){
					PRINTF_ERROR("ERROR, pre-cargando paquete: '%s'\n", strTmp->str());
				} else {
					conteoPaquetesCargados++;
				}
			}
		}
		strTmp->liberar(NB_RETENEDOR_THIS);
		PRINTF_INFO("%d de %d paquetes cargados.\n", conteoPaquetesCargados, conteoPaquetesCargar);
	}
	//
	NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene, this);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUAppEscenasAdmin::~AUAppEscenasAdmin(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdmin::~AUAppEscenasAdmin")
	NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene, this);
	if(_fondoCapa != NULL){
		NBGestorEscena::quitarObjetoCapa(_iScene, _fondoCapa);
		_fondoCapa->liberar(NB_RETENEDOR_THIS);
		_fondoCapa = NULL;
	}
	if(_fondoSprite != NULL) _fondoSprite->liberar(NB_RETENEDOR_THIS); _fondoSprite = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUAppEscenasAdmin::escenasHeredarFondo(const char* rutaTextura, AUEscenaContenedor* contenedorFondo, AUEscenaSpritePorcion* spriteFondo, const ENAppEscenasFondoModo modoFondo){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdmin::escenasHeredarFondo")
	if(contenedorFondo != NULL) contenedorFondo->retener(NB_RETENEDOR_THIS);
	if(spriteFondo != NULL) spriteFondo->retener(NB_RETENEDOR_THIS);
	if(_fondoCapa != NULL) _fondoCapa->liberar(NB_RETENEDOR_THIS);
	if(_fondoSprite != NULL) _fondoSprite->liberar(NB_RETENEDOR_THIS);
	_fondoModo		= modoFondo;
	_fondoCapa		= contenedorFondo;
	_fondoSprite	= spriteFondo;
	_fondoRuta.establecer(rutaTextura);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUAppEscenasAdmin::privLiberarRecursosSinUso(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdmin::privLiberarRecursosSinUso")
	NBGestorEscena::liberarRecursosCacheRenderEscenas();
	UI32 conteoLiberados;
	do {
		conteoLiberados = 0;
		conteoLiberados += NBGestorCuerpos::liberarPlantillasSinReferencias();
		conteoLiberados += NBGestorAnimaciones::liberarAnimacionesSinReferencias();
		conteoLiberados += NBGestorTexturas::liberarTexturasSinReferencias();
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		conteoLiberados += NBGestorSonidos::liberarBufferesSinReferencias();
#		endif
		//conteoLiberados +=  //PEDIENTE: aplicar autoliberaciones
	} while(conteoLiberados != 0);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

// EVENTOS

void AUAppEscenasAdmin::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppEscenasAdmin::puertoDeVisionModificado")
	if(_fondoSprite != NULL) this->privEscalarFondo(_fondoSprite);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

void AUAppEscenasAdmin::privEscalarFondo(AUEscenaSpritePorcion* spriteFondo){
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
		if(_fondoModo == ENAppEscenasFondoModo_ajustar){
			float anchoRelExcede	= ((tamImgFondo.ancho * escalaUsar) - tamCajaFondo.ancho) / (tamImgFondo.ancho * escalaUsar); NBASSERT(anchoRelExcede > -0.01f) if(anchoRelExcede < 0.0f) anchoRelExcede = 0.0f; //Precision de coma flotante
			float altoRelExcede		= ((tamImgFondo.alto * escalaUsar) - tamCajaFondo.alto) / (tamImgFondo.ancho * escalaUsar); NBASSERT(altoRelExcede > -0.01f) if(altoRelExcede < 0.0f) altoRelExcede = 0.0f; //Precision de coma flotante
			spriteFondo->establecerPorcionTextura(anchoRelExcede * 0.5f, 1.0f - (anchoRelExcede * 0.5f), altoRelExcede * 0.5f, 1.0f - (altoRelExcede * 0.5f));
		} else {
			//ENAppEscenasFondoModo_estirar
			spriteFondo->establecerPorcionTextura(0.0f, 1.0f, 0.0f, 1.0f);
		}
	}
}
//

AUOBJMETODOS_CLASESID_UNICLASE(AUAppEscenasAdmin)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUAppEscenasAdmin, "AUAppEscenasAdmin")
AUOBJMETODOS_CLONAR_NULL(AUAppEscenasAdmin)






