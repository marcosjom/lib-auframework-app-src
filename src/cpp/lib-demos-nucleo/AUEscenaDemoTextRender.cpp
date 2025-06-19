//
//  AUEscenaDemoTextRender.cpp
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUEscenaDemoTextRender.h"

AUEscenaDemoTextRender::AUEscenaDemoTextRender(const SI32 iEscena)
: AUAppEscena()
, _iScene(iEscena)
, _enPrimerPlano(false)
, _rotating(false)
, _scaling(false)
, _scaleGrowing(true)
, _iCurrentText(0)
{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaDemoTextRender::AUEscenaDemoTextRender")
	//---------------------
	_mode		= ENEscenaDemoTextRenderMode_multiFormatText;
	//---------------------
	_capaRaiz	= new AUEscenaContenedor();
	_textos		= new AUArregloMutable();
	{
		_fondo		= new AUEscenaSprite();
		_fondo->establecerMultiplicadorColor8(0, 0, 0, 255);
		_fondo->establecerEscuchadorTouches(this, this);
		_capaRaiz->agregarObjetoEscena(_fondo);
	}
	{
		_capaTextos	= new AUEscenaContenedor();
		_capaRaiz->agregarObjetoEscena(_capaTextos);
	}
	{
		_btnRotate	= new AUEscenaSprite();
		_btnRotate->redimensionar(64.0f, 32.0f);
		_btnRotate->establecerMultiplicadorColor8(0, 255, 0, 255);
		_btnRotate->establecerEscuchadorTouches(this, this);
		_capaRaiz->agregarObjetoEscena(_btnRotate);
	}
	{
		_btnScale	= new AUEscenaSprite();
		_btnScale->redimensionar(64.0f, 32.0f);
		_btnScale->establecerMultiplicadorColor8(0, 0, 255, 255);
		_btnScale->establecerEscuchadorTouches(this, this);
		_capaRaiz->agregarObjetoEscena(_btnScale);
	}
	{
		_btnChngTxt	= new AUEscenaSprite();
		_btnChngTxt->redimensionar(64.0f, 32.0f);
		_btnChngTxt->establecerMultiplicadorColor8(125, 125, 125, 255);
		_btnChngTxt->establecerEscuchadorTouches(this, this);
		_capaRaiz->agregarObjetoEscena(_btnChngTxt);
	}
	//
	this->privOrganizarContenido();
	NBGestorAnimadores::agregarAnimador(this, this);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUEscenaDemoTextRender::~AUEscenaDemoTextRender(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaDemoTextRender::~AUEscenaDemoTextRender")
	NBGestorAnimadores::quitarAnimador(this);
	this->escenaQuitarDePrimerPlano();
	if(_textos != NULL) _textos->liberar(NB_RETENEDOR_THIS); _textos = NULL;
	if(_capaRaiz != NULL) _capaRaiz->liberar(NB_RETENEDOR_THIS); _capaRaiz = NULL;
	if(_fondo != NULL) _fondo->liberar(NB_RETENEDOR_THIS); _fondo = NULL;
	if(_capaTextos != NULL) _capaTextos->liberar(NB_RETENEDOR_THIS); _capaTextos = NULL;
	if(_btnRotate != NULL) _btnRotate->liberar(NB_RETENEDOR_THIS); _btnRotate = NULL;
	if(_btnScale != NULL) _btnScale->liberar(NB_RETENEDOR_THIS); _btnScale = NULL;
	if(_btnChngTxt != NULL) _btnChngTxt->liberar(NB_RETENEDOR_THIS); _btnChngTxt = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

void AUEscenaDemoTextRender::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	this->privOrganizarContenido();
}

void AUEscenaDemoTextRender::privOrganizarContenido(){
	const NBCajaAABB cajaEscena = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_GUI);
	//Redimensionar fondo
	_fondo->redimensionar(cajaEscena.xMin, cajaEscena.yMin, (cajaEscena.xMax - cajaEscena.xMin), (cajaEscena.yMax - cajaEscena.yMin));
	//Texts
	{
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		SI32 i = 0;
		/*if(_mode == ENEscenaDemoTextRenderMode_rasterFont){
			if(i >= _textos->conteo){
				AUFuenteRender* fuente = NBGestorTexturas::fuenteRaster("Roboto", (30 + i), false, false); NBASSERT(fuente != NULL) //Helvetica Neue
				AUEscenaTextoEditable* txt = new AUEscenaTextoEditable(fuente);
				txt->establecerAlineaciones(ENNBTextLineAlignH_Center, ENNBTextAlignV_Center);
				strTmp->vaciar();
				strTmp->agregarConFormato("texto de prueba ... TEXTO DE PRUEBA ... (%d)", (30 + i));
				txt->establecerTexto(strTmp->str());
				_capaTextos->agregarObjetoEscena(txt);
				_textos->agregarElemento(txt);
				txt->liberar(NB_RETENEDOR_THIS);
			}
			AUEscenaObjeto* txt = (AUEscenaObjeto*)_textos->elem(i);
			//txt->establecerTraslacion(cajaEscena.xMin + ((cajaEscena.xMax - cajaEscena.xMin) * 0.5f), cajaEscena.yMin + ((cajaEscena.yMax - cajaEscena.yMin) * 0.5f));
			txt->establecerVisible(true);
			i++;
		} else*/ if(_mode == ENEscenaDemoTextRenderMode_rasterFont || _mode == ENEscenaDemoTextRenderMode_textureFont){
			//Colocar los textos que alcancen en el alto de la pantalla
			const float yMin = cajaEscena.yMin + 32.0f;
			float ySup = cajaEscena.yMax - 32.0f;
			while(1){
				if(i >= _textos->conteo){
					AUFuenteRender* fuente = NULL;
					if(_mode == ENEscenaDemoTextRenderMode_rasterFont){
						fuente = NBGestorTexturas::fuenteRaster("Roboto", (10 + i), false, false); NBASSERT(fuente != NULL) //Helvetica Neue
					} else {
						fuente = NBGestorTexturas::fuenteTextura("Roboto", (10 + i), false, false); NBASSERT(fuente != NULL) //Helvetica Neue
					}
					AUEscenaTextoEditable* txt = new AUEscenaTextoEditable(fuente);
					txt->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_FromTop);
					txt->establecerTexto(AUEscenaDemoTextRender::privGetText(_iCurrentText, (10 + i), strTmp));
					_capaTextos->agregarObjetoEscena(txt);
					_textos->agregarElemento(txt);
					txt->liberar(NB_RETENEDOR_THIS);
				}
				AUEscenaObjeto* txt = (AUEscenaObjeto*)_textos->elem(i);
				const NBCajaAABB cajaLocal = txt->cajaAABBLocal();
				const float alto = (cajaLocal.yMax - cajaLocal.yMin);
				if((ySup - alto) > yMin){
					txt->establecerTraslacion(cajaEscena.xMin + 32.0f - cajaLocal.xMin, ySup - cajaLocal.yMax);
					txt->establecerVisible(true);
					ySup -= (alto + 10);
					i++;
				} else {
					break;
				}
			}
		/*} else if(_mode == ENEscenaDemoTextRenderMode_spritesText){
			//Colocar los textos que alcancen en el alto de la pantalla
			const float yMin = cajaEscena.yMin + 32.0f;
			float ySup = cajaEscena.yMax - 32.0f;
			while(1){
				if(i >= _textos->conteo){
					AUFuenteTextura* fuente = NBGestorTexturas::fuenteTextura("Roboto", (10 + i), false, false); NBASSERT(fuente != NULL) //Helvetica Neue
					AUEscenaTextoSprites* txt = new AUEscenaTextoSprites(fuente);
					txt->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_FromTop);
					txt->establecerTexto(AUEscenaDemoTextRender::privGetText(_iCurrentText, (10 + i), strTmp), ENEscenaTextoAnimacion_Alpha);
					_capaTextos->agregarObjetoEscena(txt);
					_textos->agregarElemento(txt);
					txt->liberar(NB_RETENEDOR_THIS);
				}
				AUEscenaObjeto* txt = (AUEscenaObjeto*)_textos->elem(i);
				const NBCajaAABB cajaLocal = txt->cajaAABBLocal();
				const float alto = (cajaLocal.yMax - cajaLocal.yMin);
				if((ySup - alto) > yMin){
					txt->establecerTraslacion(cajaEscena.xMin + 32.0f - cajaLocal.xMin, ySup - cajaLocal.yMax);
					txt->establecerVisible(true);
					ySup -= (alto + 10);
					i++;
				} else {
					break;
				}
			}*/
		} else if(_mode == ENEscenaDemoTextRenderMode_multiFormatText){
			const float yMin = cajaEscena.yMin + 32.0f;
			float ySup = cajaEscena.yMax - 32.0f;
			while(1){
				if(i >= _textos->conteo){
					AUFuenteTextura* fuente = NBGestorTexturas::fuenteTextura("Roboto", (10 + i), false, false); NBASSERT(fuente != NULL) //Helvetica Neue
					AUEscenaTextoEditable* txt = new AUEscenaTextoEditable(fuente);
					txt->establecerAlineaciones(ENNBTextLineAlignH_Left, ENNBTextAlignV_FromTop);
					AUEscenaDemoTextRender::privSetMultiformatText(_iCurrentText, (10 + i), strTmp, txt);
					_capaTextos->agregarObjetoEscena(txt);
					_textos->agregarElemento(txt);
					txt->liberar(NB_RETENEDOR_THIS);
				}
				AUEscenaObjeto* txt = (AUEscenaObjeto*)_textos->elem(i);
				const NBCajaAABB cajaLocal = txt->cajaAABBLocal();
				const float alto = (cajaLocal.yMax - cajaLocal.yMin);
				if((ySup - alto) > yMin){
					txt->establecerTraslacion(cajaEscena.xMin + 32.0f - cajaLocal.xMin, ySup - cajaLocal.yMax);
					txt->establecerVisible(true);
					ySup -= (alto + 10);
					i++;
				} else {
					break;
				}
			}
		}
		//Ocultar los textos que no alcanzaron en pantalla
		for(; i < _textos->conteo; i++){
			AUEscenaObjeto* txt = (AUEscenaObjeto*)_textos->elem(i);
			txt->establecerVisible(false);
		}
		strTmp->liberar(NB_RETENEDOR_THIS); strTmp = NULL;
	}
	//
	{
		const NBCajaAABB cajaLocal = _btnRotate->cajaAABBLocal();
		_btnRotate->establecerTraslacion(cajaEscena.xMax - 32.0f - cajaLocal.xMax, cajaEscena.yMax - 32.0f - cajaLocal.yMax);
	}
	{
		const NBCajaAABB cajaLocal = _btnScale->cajaAABBLocal();
		_btnScale->establecerTraslacion(cajaEscena.xMax - 32.0f - cajaLocal.xMax, cajaEscena.yMax - 32.0f - cajaLocal.yMax - 32.0f - 8.0f);
	}
	{
		const NBCajaAABB cajaLocal = _btnChngTxt->cajaAABBLocal();
		_btnChngTxt->establecerTraslacion(cajaEscena.xMax - 32.0f - cajaLocal.xMax, cajaEscena.yMax - 32.0f - 8.0f - 32.0f - cajaLocal.yMax - 32.0f - 8.0f);
	}
}

//

void AUEscenaDemoTextRender::tickAnimacion(float segsTranscurridos){
	//Rotate layer
	if(_rotating){
		float rot = _capaTextos->rotacion();
		rot += 45.0f * segsTranscurridos;
		while(rot > 360.f) rot -= 360.f;
		_capaTextos->establecerRotacion(rot);
	}
	//Scale layer
	if(_scaling){
		float scale = _capaTextos->escalacion().ancho;
		if(_scaleGrowing){
			scale += 0.25f * segsTranscurridos;
			if(scale > 4.0f) _scaleGrowing = !_scaleGrowing;
		} else {
			scale -= 0.25f * segsTranscurridos;
			if(scale < -4.0f) _scaleGrowing = !_scaleGrowing;
		}
		_capaTextos->establecerEscalacion(scale);
	}
}

//

void AUEscenaDemoTextRender::touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto){
	//
}

void AUEscenaDemoTextRender::touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	//
}

void AUEscenaDemoTextRender::touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	const NBCajaAABB cajaEscena = objeto->cajaAABBEnEscena();
	if(NBCAJAAABB_INTERSECTA_PUNTO(cajaEscena, posActualEscena.x, posActualEscena.y)){
		if(objeto == _fondo){
			NBColor8 color = _fondo->_propiedades.color8;
			if(color.b == 255){
				//Restar colores
				if(color.r != 0){
					color.r = (color.r > 70 ? (color.r - 70) : 0);
				} else if(color.g != 0) {
					color.g = (color.g > 70 ? (color.g - 70) : 0);
				} else {
					color.b = 0;
				}
			} else {
				//Sumar colores
				if(color.r != 255){
					color.r = (color.r < (255 - 70) ? (color.r + 70) : 255);
				} else if(color.g != 255) {
					color.g = (color.g < (255 - 70) ? (color.g + 70) : 255);
				} else {
					color.b = (color.b < (255 - 70) ? (color.b + 70) : 255);
				}
			}
			_fondo->establecerMultiplicadorColor8(color);
		} else if(objeto == _btnRotate){
			_rotating = !_rotating;
		} else if(objeto == _btnScale){
			_scaling = !_scaling;
		} else if(objeto == _btnChngTxt){
			AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			_iCurrentText++; if(_iCurrentText > 1) _iCurrentText = 0;
			UI32 i; for(i = 0; i < _textos->conteo; i++){
				AUEscenaObjeto* obj = (AUEscenaObjeto*)_textos->elem(i);
				/*if(obj->esClase(AUEscenaTextoSprites::idTipoClase)){
					AUEscenaTextoSprites* txt = (AUEscenaTextoSprites*)obj;
					txt->establecerTexto(AUEscenaDemoTextRender::privGetText(_iCurrentText, txt->font()->tamanoEscena(), strTmp), ENEscenaTextoAnimacion_Alpha);
				} else*/ if(obj->esClase(AUEscenaTextoEditable::idTipoClase)){
					AUEscenaTextoEditable* txt = (AUEscenaTextoEditable*)obj;
					if(_mode == ENEscenaDemoTextRenderMode_multiFormatText){
						AUEscenaDemoTextRender::privSetMultiformatText(_iCurrentText, txt->fuenteRender()->tamanoEscena(), strTmp, txt);
					} else {
						txt->establecerTexto(AUEscenaDemoTextRender::privGetText(_iCurrentText, txt->fuenteRender()->tamanoEscena(), strTmp));
					}
				} else {
					NBASSERT(FALSE)
				}
			}
			strTmp->liberar(NB_RETENEDOR_THIS);
		}
	} else {
		if(objeto == _btnRotate){
			_capaTextos->establecerRotacion(0);
		} else if(objeto == _btnScale){
			_capaTextos->establecerEscalacion(1);
		}
	}
}

//

bool AUEscenaDemoTextRender::escenaEnPrimerPlano(){
	return _enPrimerPlano;
}

void AUEscenaDemoTextRender::escenaColocarEnPrimerPlano(){
	if(!_enPrimerPlano){
		NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_GUI, _capaRaiz, NBColor8(255, 255, 255, 255));
		NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene,  this);
		this->privOrganizarContenido();
		_enPrimerPlano = true;
	}
}

void AUEscenaDemoTextRender::escenaQuitarDePrimerPlano(){
	if(_enPrimerPlano){
		NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene,  this);
		NBGestorEscena::quitarObjetoCapa(_iScene, _capaRaiz);
		_enPrimerPlano = false;
	}
}

//

void AUEscenaDemoTextRender::escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate) {
	//
}

//

bool AUEscenaDemoTextRender::escenaEstaAnimandoSalida(){
	return false;
}

void AUEscenaDemoTextRender::escenaAnimarSalida(){
	//
}

void AUEscenaDemoTextRender::escenaAnimarEntrada(){
	//
}

bool AUEscenaDemoTextRender::escenaPermitidoGirarPantalla(){
	return true;
}

//TECLAS

bool AUEscenaDemoTextRender::teclaPresionada(SI32 codigoTecla){
	return false;
}

bool AUEscenaDemoTextRender::teclaLevantada(SI32 codigoTecla){
	return false;
}

bool AUEscenaDemoTextRender::teclaEspecialPresionada(SI32 codigoTecla){
	return false;
}

//

const char* AUEscenaDemoTextRender::privGetText(const UI32 iText, const UI32 fontSz, AUCadenaMutable8* dst){
	dst->vaciar();
	switch (iText) {
		case 1:
			dst->agregarConFormato("%d) Esta es mi gran fuente!", fontSz);
			break;
		default:
			dst->agregarConFormato("texto de prueba ... TEXTO DE PRUEBA ... (%d)", fontSz);
			break;
	}
	return dst->str();
}

void AUEscenaDemoTextRender::privSetMultiformatText(const UI32 iText, const UI32 fontSz, AUCadenaMutable8* strTmp, AUEscenaTextoEditable* dst){
	strTmp->vaciar();
	dst->vaciarTexto();
	AUFuenteTextura* fuente = NBGestorTexturas::fuenteTextura("Roboto", fontSz, false, false); NBASSERT(fuente != NULL) //Helvetica Neue
	AUFuenteRaster* fuente2 = NBGestorTexturas::fuenteRaster("Special Elite", fontSz, false, false); NBASSERT(fuente2 != NULL) //Helvetica Neue
	AUFuenteTextura* fuente3 = NBGestorTexturas::fuenteTextura("Yellowtail", fontSz, false, false); NBASSERT(fuente3 != NULL) //Helvetica Neue
	switch (iText) {
		case 1:
			strTmp->agregarConFormato("%d) ", fontSz);
			dst->appendText(fuente, strTmp->str());
			dst->appendText(fuente2, "(raster)");
			dst->appendText(fuente, " es mi gran ");
			dst->appendText(fuente3, "fuente");
			dst->appendText(fuente, "!");
			break;
		default:
			dst->appendText(fuente2, "(raster)");
			dst->appendText(fuente, " de prueba ... ");
			dst->appendText(fuente3, "(TEXTURE) DE PRUEBA ...");
			strTmp->agregarConFormato("(%d)", fontSz);
			dst->appendText(fuente, strTmp->str());
			break;
	}
	dst->appendTextFinish();
	dst->setRngSelection(2, 3);
	//dst->setRngComposing(1, 6);
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUEscenaDemoTextRender)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUEscenaDemoTextRender, "AUEscenaDemoTextRender")
AUOBJMETODOS_CLONAR_NULL(AUEscenaDemoTextRender)






