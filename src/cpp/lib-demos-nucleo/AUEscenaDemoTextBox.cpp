//
//  AUEscenaDemoTextBox.cpp
//  Gameplay_Mac
//
//  Created by Nicaragua Binary on 29/10/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "AUAppNucleoPrecompilado.h"
#include "AUEscenaDemoTextBox.h"

AUEscenaDemoTextBox::AUEscenaDemoTextBox(const SI32 iEscena) : AUAppEscena()
	, _enPrimerPlano(false)
	, _iScene(iEscena)
	//
	, _textboxsLayer(NULL)
	, _textboxs(this)
{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaDemoTextBox::AUEscenaDemoTextBox")
	_capaRaiz	= new(this) AUEscenaContenedor();
	_fondo		= new(this) AUEscenaSprite();
	_fondo->establecerMultiplicadorColor8(50, 50, 50, 255);
	_fondo->establecerEscuchadorTouches(this, this);
	_capaRaiz->agregarObjetoEscena(_fondo);
	//
	_textboxsLayer = new(this) AUEscenaContenedor();
	_capaRaiz->agregarObjetoEscena(_textboxsLayer);
	//
	const NBCajaAABB sceneBox = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_GUI);
	NBMargenes margins;
	margins.left = margins.right = NBGestorEscena::anchoPulgadasAEscena(_iScene, 0.05f);
	margins.bottom = margins.top = NBGestorEscena::altoPulgadasAEscena(_iScene, 0.05f);
	const float fntSz		= NBGestorEscena::altoPuntosAEscena(_iScene, 12.0f);
	AUFuenteRender* font	= NBGestorTexturas::fuenteTextura("Roboto", fntSz, false, false);
	const float width		= (sceneBox.xMax - sceneBox.xMin) * 0.8f;
	//Monoline
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Compose, font, width, 1.0f, margins);
		//txt->establecerTexto("Hola amigo mío!");
		txt->establecerTextoAyuda("Monoline");
		txt->establecerTextoPermitirMultilinea(false);
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	//Multilines
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Compose, font, width, 1.0f, margins);
		txt->establecerTextoAyuda("Multiline 1");
		txt->establecerTextoPermitirMultilinea(true);
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Compose, font, width, 2.0f, margins);
		txt->establecerTextoAyuda("Multiline 2");
		txt->establecerTextoPermitirMultilinea(true);
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Compose, font, width, 3.0f, margins);
		txt->establecerTextoAyuda("Multiline 3");
		txt->establecerTextoPermitirMultilinea(true);
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	//Password Monoline
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Password, font, width, 1.0f, margins);
		txt->establecerTextoAyuda("Pass ascii");
		txt->establecerTextoPermitirMultilinea(false);
		txt->setPasswordChar("*");
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Password, font, width, 1.0f, margins);
		txt->establecerTextoAyuda("Pass widechar");
		txt->establecerTextoPermitirMultilinea(false);
		txt->setPasswordChar("®");
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	//Password Multilines
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Password, font, width, 1.0f, margins);
		txt->establecerTextoAyuda("Pass multiline 1");
		txt->establecerTextoPermitirMultilinea(true);
		txt->setPasswordChar("*");
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Password, font, width, 2.0f, margins);
		txt->establecerTextoAyuda("Pass multiline 2");
		txt->establecerTextoPermitirMultilinea(true);
		txt->setPasswordChar("*");
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Password, font, width, 3.0f, margins);
		txt->establecerTextoAyuda("Pass multiline 3");
		txt->establecerTextoPermitirMultilinea(true);
		txt->setPasswordChar("*");
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	{
		AUITextBox* txt = new(this) AUITextBox(ENTextEditorType_Password, font, width, 1.0f, margins);
		txt->establecerTextoAyuda("Pass ascii (not hidden)");
		txt->establecerTextoPermitirMultilinea(false);
		_textboxsLayer->agregarObjetoEscena(txt);
		_textboxs.agregarElemento(txt);
		txt->liberar(NB_RETENEDOR_THIS);
	}
	//
	this->privOrganizarContenido();
	NBGestorAnimadores::agregarAnimador(this, this);
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

AUEscenaDemoTextBox::~AUEscenaDemoTextBox(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUEscenaDemoTextBox::~AUEscenaDemoTextBox")
	NBGestorAnimadores::quitarAnimador(this);
	this->escenaQuitarDePrimerPlano();
	//
	if(_textboxsLayer != NULL) _textboxsLayer->liberar(NB_RETENEDOR_THIS); _textboxsLayer = NULL;
	if(_capaRaiz != NULL) _capaRaiz->liberar(NB_RETENEDOR_THIS); _capaRaiz = NULL;
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//

void AUEscenaDemoTextBox::puertoDeVisionModificado(const SI32 iEscena, const STNBViewPortSize before, const STNBViewPortSize after){
	this->privOrganizarContenido();
}

void AUEscenaDemoTextBox::privOrganizarContenido(){
	const NBCajaAABB cajaEscena = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_GUI);
	//Organizar textbox
	{
		const float marginV = NBGestorEscena::altoPulgadasAEscena(_iScene, 0.25f);
		const NBCajaAABB sceneBox = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_GUI);
		const float sceneWidth = (sceneBox.xMax - sceneBox.xMin);
		const float width = sceneWidth * 0.8f;
		float yTop = cajaEscena.yMax - marginV;
		SI32 i; const SI32 count = _textboxs.conteo;
		for(i = 0; i < count; i++){
			AUITextBox* txt		= (AUITextBox*)_textboxs.elem(i);
			const float boxH	= txt->boxHeightInLines();
			txt->establecerTamanoCaja(width, boxH);
			const NBCajaAABB box = txt->cajaAABBLocalCalculada();
			txt->establecerTraslacion(cajaEscena.xMin - box.xMin + (sceneWidth * 0.05f), yTop - box.yMax);
			yTop -= (box.yMax - box.yMin) + marginV;
		}
	}
	//Redimensionar fondo
	{
		_fondo->redimensionar(cajaEscena.xMin, cajaEscena.yMin, (cajaEscena.xMax - cajaEscena.xMin), (cajaEscena.yMax - cajaEscena.yMin));
	}
	//
	this->privScrollV(0.0f);
}

//

void AUEscenaDemoTextBox::privScrollV(const float delta){
	const float marginV = NBGestorEscena::altoPulgadasAEscena(_iScene, 0.25f);
	const NBCajaAABB sceneBox = NBGestorEscena::cajaProyeccionGrupo(_iScene, ENGestorEscenaGrupo_GUI);
	const NBCajaAABB box = _textboxsLayer->cajaAABBLocalCalculada();
	NBPunto pos = _textboxsLayer->traslacion();
	pos.y += delta;
	//
	if((pos.y + box.yMin) > (sceneBox.yMin + marginV)){
		pos.y = sceneBox.yMin + marginV - box.yMin;
	}
	if((pos.y + box.yMax) < (sceneBox.yMax - marginV)){
		pos.y = sceneBox.yMax - marginV - box.yMax;
	}
	_textboxsLayer->establecerTraslacion(pos.x, pos.y);
}

void AUEscenaDemoTextBox::tickAnimacion(float segsTranscurridos){
	
}

void AUEscenaDemoTextBox::touchIniciado(STGTouch* touch, const NBPunto &posTouchEscena, AUEscenaObjeto* objeto){
	
}

void AUEscenaDemoTextBox::touchMovido(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	if(objeto == _fondo){
		this->privScrollV(posActualEscena.y - posAnteriorEscena.y);
	}
}

void AUEscenaDemoTextBox::touchFinalizado(STGTouch* touch, const NBPunto &posInicialEscena, const NBPunto &posAnteriorEscena, const NBPunto &posActualEscena, AUEscenaObjeto* objeto){
	const NBCajaAABB cajaEscena = objeto->cajaAABBEnEscena();
	if(NBCAJAAABB_INTERSECTA_PUNTO(cajaEscena, posActualEscena.x, posActualEscena.y)){
		//
	}
}

//

bool AUEscenaDemoTextBox::escenaEnPrimerPlano(){
	return _enPrimerPlano;
}

void AUEscenaDemoTextBox::escenaColocarEnPrimerPlano(){
	if(!_enPrimerPlano){
		NBGestorEscena::agregarObjetoCapa(_iScene, ENGestorEscenaGrupo_GUI, _capaRaiz, NBColor8(255, 255, 255, 255));
		NBGestorEscena::agregarEscuchadorCambioPuertoVision(_iScene,  this);
		this->privOrganizarContenido();
		_enPrimerPlano = true;
	}
}

void AUEscenaDemoTextBox::escenaQuitarDePrimerPlano(){
	if(_enPrimerPlano){
		NBGestorEscena::quitarEscuchadorCambioPuertoVision(_iScene,  this);
		NBGestorEscena::quitarObjetoCapa(_iScene, _capaRaiz);
		_enPrimerPlano = false;
	}
}

//

void AUEscenaDemoTextBox::escenaGetOrientations(UI32* dstMask, ENAppOrientationBit* dstPrefered, ENAppOrientationBit* dstToApplyOnce, BOOL* dstAllowAutoRotate){
    //
}

//

bool AUEscenaDemoTextBox::escenaEstaAnimandoSalida(){
	return false;
}

void AUEscenaDemoTextBox::escenaAnimarSalida(){
	//
}

void AUEscenaDemoTextBox::escenaAnimarEntrada(){
	//
}

bool AUEscenaDemoTextBox::escenaPermitidoGirarPantalla(){
	return true;
}

//TECLAS

bool AUEscenaDemoTextBox::teclaPresionada(SI32 codigoTecla){
	return false;
}

bool AUEscenaDemoTextBox::teclaLevantada(SI32 codigoTecla){
	return false;
}

bool AUEscenaDemoTextBox::teclaEspecialPresionada(SI32 codigoTecla){
	return false;
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUEscenaDemoTextBox)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUEscenaDemoTextBox, "AUEscenaDemoTextBox")
AUOBJMETODOS_CLONAR_NULL(AUEscenaDemoTextBox)






