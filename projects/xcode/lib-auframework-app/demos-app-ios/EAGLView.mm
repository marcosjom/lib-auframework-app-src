//
//  EAGLView.m
//  OpenGLES_iPhone
//
//  Created by mmalc Crawford on 11/18/10.
//  Copyright 2010 Apple Inc. All rights reserved.
//

#import "EAGLView.h"

@implementation EAGLView

// You must implement this method
+ (Class) layerClass {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::layerClass")
    //NSLog(@"EAGLView::layerClass");
	AU_GESTOR_PILA_LLAMADAS_POP_3
    return [CAEAGLLayer class];
}

- (void) dealloc {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::dealloc")
    //NSLog(@"EAGLView::dealloc");
	[self destruirContextoOpenGL];
    //[super dealloc];
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

/****************************
 *** CONTEXTO OPEN GL
 ****************************/

- (void) establecerJuego:(AUApp*)juego {
	_juego = juego;
}

- (EAGLContext*) crearContextoOpenGL {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::crearContextoOpenGL")
	NBASSERT(_contextoOpenGL == NULL) //Si falla, se ha llamado mas de una vez
	//NSAutoreleasePool *pool			= [[NSAutoreleasePool alloc] init];
	//Habilitar multitouches
	self.multipleTouchEnabled		= YES;
	CAEAGLLayer *eaglLayer			= (CAEAGLLayer *)[self layer];
	eaglLayer.opaque				= YES;
	eaglLayer.drawableProperties	= [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
	_contextoOpenGL = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	if(_contextoOpenGL == nil){
		PRINTF_ERROR("Creando contexto OpenGL\n"); NBASSERT(false);
	} else {
		if(![EAGLContext setCurrentContext:_contextoOpenGL]){
			PRINTF_ERROR("Estableciendo contexto OpenGL principal\n"); NBASSERT(false);
		} else {
			PRINTF_INFO("EXITO creando contexto OpenGL\n");
		}
	}
	//
	//iPad: 132dpi (~800K pixeles)
	//iPhone & iPodTouch & iPadMini: 163dpi
	//iPhone & iPodTouch Retina & iPadMini2: 326dpi (~600K - ~750K pixeles)
	//iPad Retina: 264dpi (~3M pixeles)
	//
	UIScreen* mainScreen			= [UIScreen mainScreen];
	_pantallaEscalaParaTamanoNativo	= 1.0f; if([mainScreen respondsToSelector:@selector(scale)]) _pantallaEscalaParaTamanoNativo = [mainScreen scale];
	if(UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
		_pantallaPuntosPorPulgada	= 132 * _pantallaEscalaParaTamanoNativo; if(_pantallaEscalaParaTamanoNativo>1.0f) PRINTF_INFO("iOS PANTALLA RETINA DE iPAD DETECTADA ESCALA(%f)\n", _pantallaEscalaParaTamanoNativo);
	} else if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
		_pantallaPuntosPorPulgada	= 163 * _pantallaEscalaParaTamanoNativo; if(_pantallaEscalaParaTamanoNativo>1.0f) PRINTF_INFO("iOS PANTALLA RETINA DE iPHONE/iPOD DETECTADA ESCALA(%f)\n", _pantallaEscalaParaTamanoNativo);
	} else {
		_pantallaPuntosPorPulgada	= 160 * _pantallaEscalaParaTamanoNativo; if(_pantallaEscalaParaTamanoNativo>1.0f) PRINTF_INFO("iOS PANTALLA RETINA [dispositivo desconocido] DETECTADA ESCALA(%f)\n", _pantallaEscalaParaTamanoNativo);
	}
	float versionIOS				= [[[UIDevice currentDevice] systemVersion] floatValue];
	if(versionIOS>=3.2f){
		_pantallaAnchoLogico		= mainScreen.currentMode.size.width / _pantallaEscalaParaTamanoNativo;
		_pantallaAltoLogico			= mainScreen.currentMode.size.height / _pantallaEscalaParaTamanoNativo;
		_pantallaAnchoNativo		= mainScreen.currentMode.size.width;
		_pantallaAltoNativo			= mainScreen.currentMode.size.height;
	} else {
		_pantallaAnchoLogico		= 320;
		_pantallaAltoLogico			= 480;
		_pantallaAnchoNativo		= 320;
		_pantallaAltoNativo			= 480;
	}
	//
	//[pool release];
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _contextoOpenGL;
}

- (EAGLContext*) contextoOpenGL {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::contextoOpenGL")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _contextoOpenGL;
}

- (SI32) indiceEscenaRender {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::indiceEscenaRender")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return _indiceEscenaRender;
}

- (bool) activarContextoOpenGL{
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::activarContextoOpenGL")
	bool r = false;
	if(_contextoOpenGL){
		r = [EAGLContext setCurrentContext:_contextoOpenGL];
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

- (void) destruirContextoOpenGL {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::destruirContextoOpenGL")
	if(_contextoOpenGL){
		if(_indiceEscenaRender!=-1){
			NBGestorEscena::liberarEscena(_indiceEscenaRender);
		}
		NBGestorTexturas::finalizar();
		NBGestorEscena::finalizar();
		NBGestorGL::finalizar();
		//[_contextoOpenGL release];
		_contextoOpenGL = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

/****************************
 *** BUFFERS OPEN GL
 ****************************/

- (NBTamanoI) generarDatosDeRenderBufferActual:(SI32)anchoGLNvo altoNvo:(SI32)altoGLNvo {
	GLint anchoBuffer = 0, altoBuffer = 0; float escalaAplicar = 1.0f;
	//
	NBTamanoI tamanoRenderBuffer;
	CAEAGLLayer* eaglLayer			= (CAEAGLLayer *)[self layer];
	if(anchoGLNvo > 0 && altoGLNvo > 0){
		float mayorDimPantalla		= (_pantallaAnchoLogico > _pantallaAltoLogico ? _pantallaAnchoLogico : _pantallaAltoLogico);
		float mayorDimNueva			= (anchoGLNvo > altoGLNvo ? anchoGLNvo : altoGLNvo);
		escalaAplicar				= mayorDimNueva / mayorDimPantalla;
		if(_bufferGlEscala != escalaAplicar){
			eaglLayer.contentsScale		= escalaAplicar;
			self.contentScaleFactor		= escalaAplicar;
			PRINTF_INFO("iOS GENERANDO NUEVO RENDER BUFFER escalaGL(%f) tamGLSolicitado(%d, %d)\n", escalaAplicar, anchoGLNvo, altoGLNvo);
			NBASSERT(escalaAplicar == 0.5f || escalaAplicar == 1.0f || escalaAplicar == 2.0f)
		}
	}
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &anchoBuffer);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &altoBuffer);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
	PRINTF_INFO("Generando RenderBuffer con escalarPantallaGL(%f) buscandoTamanoGL(%d, %d) tamActual(%d, %d)\n", self.contentScaleFactor, anchoGLNvo, altoGLNvo, _bufferGlAncho, _bufferGlAlto);
	NBASSERT([EAGLContext currentContext] == _contextoOpenGL)
	VERIFICA_ERROR_GL("renderbufferStorage(antes)")
	if(![_contextoOpenGL renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer]){
		GL_CMD_EJECUTADO("renderbufferStorage")
		PRINTF_ERROR("Generando el RENDERBUFFER con renderbufferStorage\n");
		tamanoRenderBuffer.ancho	= 0;
		tamanoRenderBuffer.alto		= 0;
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &anchoBuffer);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &altoBuffer);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
		PRINTF_INFO("Datos de render buffer generados a dimensiones (%d, %d)\n", anchoBuffer, altoBuffer);
		NBASSERT(false);
	} else {
		GL_CMD_EJECUTADO("renderbufferStorage")
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &anchoBuffer);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &altoBuffer);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
		PRINTF_INFO("Datos de render buffer generados a dimensiones (%d, %d)\n", anchoBuffer, altoBuffer);
		tamanoRenderBuffer.ancho	= anchoBuffer;
		tamanoRenderBuffer.alto		= altoBuffer;
		NBASSERT(anchoBuffer > 0 && altoBuffer > 0)
	}
	_bufferGlEscala = escalaAplicar;
	_bufferGlAncho = anchoBuffer;
	_bufferGlAlto = altoBuffer;
	return tamanoRenderBuffer;
}

/****************************
 *** RENDERIZADO
 ****************************/

- (bool) volcarRenderBufferHaciaPantalla {
	bool exito = false;
	//if(![self activarContextoOpenGL]){
	//	NSLog(@"Error en 'renderizar', no se pudo activar el contextoOpenGL\n");
	//} else {
	if(NBGestorEscena::registroEscenaOcupado(_indiceEscenaRender)){
		STGestorEscenaEscena propsEscenaRender = NBGestorEscena::propiedadesEscena(_indiceEscenaRender);
		STGestorEscenaFrameBuffer propsFramebufferRender = NBGestorEscena::propiedadesFramebuffer(propsEscenaRender.iFramebufferEscena);
		//Es preferible llamada la funcion OpenGL a utlizar el NBGestorGL
		//Porque esta se llama desde un segundo contexto y se alteraria la cacheEstado del contexto de trabajo.
		UI32 idRenderBuffer = propsFramebufferRender.renderbufferOriginal->idRenderBufferGl();
		NBGestorGL::bindRenderbufferEXT(GL_RENDERBUFFER_EXT, idRenderBuffer);
		GL_CMD_EJECUTADO("bindRenderbufferEXT(GL_RENDERBUFFER_EXT, %d)", (SI32)idRenderBuffer);
    	[_contextoOpenGL presentRenderbuffer:GL_RENDERBUFFER_EXT]; 
		GL_CMD_EJECUTADO("presentRenderbuffer(GL_RENDERBUFFER_EXT)");
		exito = true;
	}
	//}
	return exito;
}

/****************************
 *** EVENTOS DE LA VISTA
 ****************************/

- (void) layoutSubviews {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("EAGLView::layoutSubviews")
	if(_juego != NULL){
		_juego->notificarRedimensionVentana(0, 0);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

- (float) escalaParaAnchoNativo {
	return _pantallaEscalaParaTamanoNativo;
}

- (float) pantallaPuntosPorPulgada {
	return _pantallaPuntosPorPulgada;
}

- (float) pantallaAnchoLogico {
	return _pantallaAnchoLogico;
}

- (float) pantallaAltoLogico{
	return _pantallaAltoLogico;
}


@end

NBTamanoI EAGLviewGenerarDatosDeRenderBuffer(const SI32 anchoGLNvo, const SI32 altoGLNvo, void* referenciaObjC){ //UI32 idFrameBuffer, UI32 idRenderBuffer
	//En el caso de iOS, se ignoran los parametros:
	//idGlFrameBuffer: porque la EAGL genera el renderBuffer en el frameBuffer actual.
	//ancho y alto: porque la EAGL genera el renderBuffer segun la vista.
	return [((__bridge id)referenciaObjC) generarDatosDeRenderBufferActual:anchoGLNvo altoNvo:altoGLNvo];
}
