//
//  EAGLView.h
//  OpenGLES_iPhone
//
//  Created by mmalc Crawford on 11/18/10.
//  Copyright 2010 Apple Inc. All rights reserved.
//

@class EAGLContext;

enum ENPantallaDef {
	ENPantallaDef_Simple = 0,	//Pantalla de iPhone sin retina (640 max)
	ENPantallaDef_High,			//Pantalla iPhone con retina y iPad sin Retina (960 y 1024 max respectivamente)
	ENPantallaDef_SuperHigh		//Pantalla de iPad con retina (2048 max)
};


// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView {
	@private
		EAGLContext*	_contextoOpenGL;
		AUApp*			_juego;
		//Propiedades de pantalla
		float			_pantallaEscalaParaTamanoNativo;
		float			_pantallaPuntosPorPulgada;
		float			_pantallaAnchoLogico;
		float			_pantallaAltoLogico;
		float			_pantallaAnchoNativo;
		float			_pantallaAltoNativo;
		float			_bufferGlEscala;
		GLint			_bufferGlAncho;
		GLint			_bufferGlAlto;
		SI32			_indiceEscenaRender; 
}

- (void) establecerJuego:(AUApp*)juego;
- (EAGLContext*) crearContextoOpenGL;

- (EAGLContext*) contextoOpenGL;
- (bool) activarContextoOpenGL;
- (void) destruirContextoOpenGL;

//- (EAGLContext*) crearContextoOpenGLSecundario;
//- (EAGLContext*) contextoOpenGLSecundario;
//- (bool) activarContextoOpenGLSecundario;
//- (void) destruirContextoOpenGLSecundario;

- (float)		escalaParaAnchoNativo;
- (float)		pantallaPuntosPorPulgada;
- (float)		pantallaAnchoLogico;
- (float)		pantallaAltoLogico;
//
- (SI32)		indiceEscenaRender;
- (NBTamanoI)	generarDatosDeRenderBufferActual:(SI32)anchoGLNvo altoNvo:(SI32)altoGLNvo;
- (bool)		volcarRenderBufferHaciaPantalla;

@end

NBTamanoI EAGLviewGenerarDatosDeRenderBuffer(const SI32 anchoGLNvo, const SI32 altoGLNvo, void* referenciaObjC);

