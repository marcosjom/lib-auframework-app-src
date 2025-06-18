//
//  AUGLView.m
//  AUApp-tests
//
//  Created by Marcos Ortega on 10/05/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

//#define USE_DEMO_TEXT_RENDER
#define USE_DEMO_TEXTBOX

#import "AUGLView.h"
#if defined(USE_DEMO_TEXT_RENDER)
#	include "AUEscenaDemoTextRender.h"
#elif defined(USE_DEMO_TEXTBOX)
#	include "AUEscenaDemoTextBox.h"
#endif

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

@interface AUGLView (InternalMethods)

- (void)inicializarGL:(NSRect)frame;
- (CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime;
- (void)drawFrame;

@end

@implementation AUGLView

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext){
	// go back to Obj-C for easy access to instance variables
	CVReturn result = [(AUGLView*)displayLinkContext getFrameForTime:outputTime];
	return result;
}

- (void)awakeFromNib {
	STAppCallbacks appCallbacks;
	AUApp::inicializarCallbacks(&appCallbacks);
	_app		= new AUApp(&appCallbacks, "AUTestRenderText", false /*permitirActividadRedEnBG*/);
	if(!_app->inicializarMultimedia(LEER_PRECACHE, LEER_CACHE, ESCRIBIR_CACHE, true /*initGraphics*/, 60)){
		PRINTF_ERROR("No se pudo inicializar el motor multimedia\n"); NBASSERT(false);
	} else {
		{
			PRINTF_INFO("Screens -------------.\n");
			float scaleMax = 1.0f;
			NSArray *screens = [NSScreen screens];
			NSUInteger screenCount = screens.count;
			for (int i = 0; i < screenCount; i++) {
				NSScreen* screen = screens[i];
				float scale = 1.0f;
				NSSize resDpi, sizePx;
				NBMemory_setZero(resDpi);
				NBMemory_setZero(sizePx);
				NSDictionary* deviceDesc = [screen deviceDescription];
				if(deviceDesc){
					NSValue* resObj = [deviceDesc objectForKey:NSDeviceResolution];
					NSValue* szObj = [deviceDesc objectForKey:NSDeviceSize];
					if(resObj){ resDpi = [resObj sizeValue]; }
					if(szObj){ sizePx = [szObj sizeValue]; }
				}
				if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)]) {
					scale = [screen backingScaleFactor];
					if (scaleMax < scale){ scaleMax = scale; }
				}
				PRINTF_INFO("Screen #%d / %d: %sscale(%f) dpi(%f, %f) px(%f, %f).\n", (i + 1), (SI32)screenCount, (screen == [[self window] screen] ? "[IS-CURRENT] " : ""), scale, resDpi.width, resDpi.height, sizePx.width, sizePx.height);
			}
			PRINTF_INFO("SCREENS TOTAL: %d scaleMax(%f).\n", (SI32)screenCount, scaleMax);
			PRINTF_INFO("------------- screens.\n");
		}
		NSDictionary* deviceDescription = [[self window] deviceDescription];
		NSSize resolution = [[deviceDescription objectForKey:NSDeviceResolution] sizeValue]; PRINTF_INFO("Resolucion de plantalla dpi(%f, %f).\n", resolution.width, resolution.height);
		NSSize tamano = [[deviceDescription objectForKey:NSDeviceSize] sizeValue]; PRINTF_INFO("Tamano de plantalla (%f, %f).\n", tamano.width, tamano.height);
		if(resolution.width < 72.0f) resolution.width = 72.0f;
		if(resolution.height < 72.0f) resolution.height = 72.0f;
		//Referencia: Monitor Benq G2220HDA a (1920 x 1080)pix es (102.5 x 102.5)dpi
		//
		NSSize    viewBounds	= [self bounds].size;
		NBTamanoI wSize; wSize.ancho = viewBounds.width; wSize.alto = viewBounds.height;
		NBTamano ppiScreen; ppiScreen.ancho = resolution.width; ppiScreen.alto = resolution.height;
		if(!_app->inicializarVentana(wSize, ppiScreen, ppiScreen, ENGestorEscenaDestinoGl_Heredado)){
			PRINTF_ERROR("No se pudo inicializar la ventana.\n");
		} else {
			NBASSERT(_app->indiceEscenaRender() >= 0)
			_escenas = new AUAppEscenasAdminSimple(_app->indiceEscenaRender(), ENGestorTexturaModo_cargaInmediata, "", NULL, 0);
			if(!_app->inicializarJuego(_escenas)){
				PRINTF_ERROR("No se pudo inicializar el juego.\n");
			} else {
				PRINTF_INFO("Juego iniciado!\n");
#				if defined(USE_DEMO_TEXT_RENDER)
				{
					AUEscenaDemoTextRender* demoTextRender = new AUEscenaDemoTextRender(_app->indiceEscenaRender());
					_escenas->escenaCargar(demoTextRender);
					demoTextRender->liberar(NB_RETENEDOR_THIS);
				}
#				elif defined(USE_DEMO_TEXTBOX)
				{
					AUEscenaDemoTextBox* demoTextBox = new AUEscenaDemoTextBox(_app->indiceEscenaRender());
					_escenas->escenaCargar(demoTextBox);
					demoTextBox->liberar(NB_RETENEDOR_THIS);
				}
#				endif
			}
		}
	}
	//
	// activate the display link
	CVDisplayLinkStart(displayLink);
}

- (id)initWithCoder:(NSCoder*)aDecoder {
	self = [super initWithCoder:aDecoder];
	if (self){
		[self inicializarGL: [self frame]];
	}
	return self;
}

- (id)initWithFrame:(NSRect)frame {
	[self inicializarGL: frame];
	return self;
}

- (void)inicializarGL:(NSRect)frame {
	//Context-setup
	NSOpenGLPixelFormatAttribute attribs[] = {
		//NSOpenGLPFAWindow,			//If present, this attribute indicates that only renderers that are capable of rendering to a window are considered. This attribute is implied if neither NSOpenGLPFAFullScreen nor NSOpenGLPFAOffScreen is specified.
		NSOpenGLPFAColorSize, 32,
		//NSOpenGLPFAAccelerated,		//If present, this attribute indicates that only hardware-accelerated renderers are considered. If not present, accelerated renderers are still preferred.
		NSOpenGLPFADoubleBuffer,
		//NSOpenGLPFASingleRenderer,	//If present, this attribute indicates that a single rendering engine is chosen. On systems with multiple screens, this disables OpenGL’s ability to drive different monitors through different graphics accelerator cards with a single context. This attribute is not generally useful.
		0
	};
	NSOpenGLPixelFormat* windowedPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	if (windowedPixelFormat == nil) {
		NSLog(@"Unable to create windowed pixel format.");
		exit(0);
	}
	self = [super initWithFrame:frame pixelFormat:windowedPixelFormat];
	if (self == nil) {
		NSLog(@"Unable to create a windowed OpenGL context.");
		exit(0);
	}
	[windowedPixelFormat release];
	//
	// set synch to VBL to eliminate tearing
	GLint    vblSynch = 1; [[self openGLContext] setValues:&vblSynch forParameter:NSOpenGLCPSwapInterval];
	// set up the display link
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetOutputCallback(displayLink, MyDisplayLinkCallback, self);
	CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
	CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
	//Para evitar que glClear genere error en la primera invocacion.
	glSwapAPPLE();
}

- (CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime {
	// deltaTime is unused in this bare bones demo, but here's how to calculate it using display link info
	//deltaTime = 1.0 / (outputTime->rateScalar * (double)outputTime->videoTimeScale / (double)outputTime->videoRefreshPeriod);
	[self drawFrame];
	return kCVReturnSuccess;
}

- (void)reshape {
	NSSize    viewBounds = [self bounds].size;
	NSOpenGLContext    *currentContext = [self openGLContext];
	if([currentContext view] != nil){
		[currentContext makeCurrentContext];
		// remember to lock the context before we touch it since display link is threaded
		CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
		// let the context know we've changed size
		[[self openGLContext] update];
		//
		if(_app != NULL){
			_app->notificarRedimensionVentana(viewBounds.width, viewBounds.height);
		}
		CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
	}
}

- (void)drawRect:(NSRect)rect {
	[self drawFrame];
}

- (void)drawFrame {
	NSOpenGLContext    *currentContext = [self openGLContext];
	if([currentContext view] != nil){
		[currentContext makeCurrentContext];
		// must lock GL context because display link is threaded
		CGLLockContext((CGLContextObj)[currentContext CGLContextObj]);
		if(_app != NULL){
			//Bug en OSX (depende del SDK y el OS):
			//A veces el frameBuffer es undefined al arancar el App o al cambiar a pantalla completa.
			//Para evitar que el primer glClear produzca error, se debe verificar el estado del frameBuffer heredado (fbId=0)
			NBGestorGL::bindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			const GLenum estadoFBO = NBGestorGL::checkFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if(estadoFBO == GL_FRAMEBUFFER_COMPLETE_EXT){
				@autoreleasepool {
					_app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
				}
			}
		}
		//glViewport(0, 0, viewWidth, viewHeight);
		// Draw something that changes over time to prove to yourself that it's really updating in a tight loop
		//glClearColor(
		//			 sin(CFAbsoluteTimeGetCurrent()),
		//			 sin(7.0*CFAbsoluteTimeGetCurrent()),
		//			 sin(CFAbsoluteTimeGetCurrent()/3.0),0);
		//glClear(GL_COLOR_BUFFER_BIT);
		// draw here
		[currentContext flushBuffer];
		CGLUnlockContext((CGLContextObj)[currentContext CGLContextObj]);
	}
}

- (void)dealloc {
	CVDisplayLinkRelease(displayLink);
	if(_app != NULL){
		_app->finalizarMultimedia();
		delete _app;
		_app = NULL;
	}
	[super dealloc];
}

// ------------------------------
// -- Manejo de eventos de teclado
// ------------------------------

- (void)keyDown:(NSEvent *)theEvent {
    [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}

- (void)insertNewline:(id)sender {
	NBGestorTeclas::entradaLockForBatch();
	{
		NBGestorTeclas::entradaIntroducirTexto("\n", true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (void)insertTab:(id)sender {
	NBGestorTeclas::entradaLockForBatch();
	{
		NBGestorTeclas::entradaIntroducirTexto("\t", true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (void)insertText:(id)aString {
	NBGestorTeclas::entradaLockForBatch();
	{
		NBGestorTeclas::entradaIntroducirTexto([aString UTF8String], true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (void)deleteBackward:(id)sender {
	NBGestorTeclas::entradaLockForBatch();
	{
		NBGestorTeclas::entradaBackspace(true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

// ------------------------------
// -- Manejo de eventos de mouse
// ------------------------------
- (BOOL)acceptsFirstResponder {
    return YES;
}


- (void)mouseDown:(NSEvent *)theEvent {
	//Screen
	const NSRect viewBounds		= [self bounds];
	const NSPoint viewPoint		= [self convertPoint:[theEvent locationInWindow] fromView:nil];
	//OpenGL
	const NSRect curBounds		= [self convertRectToBacking:viewBounds]; //openGL bounds
	const NSPoint curPoint		= [self convertPointToBacking:viewPoint];
	const float yPosInv			= (curBounds.size.height - curPoint.y);
	PRINTF_INFO("mouseDown viewPoint(%f, %f) curPoint(%f, %f) rel(%f, %f).\n", viewPoint.x, viewPoint.y, curPoint.x, yPosInv, (curPoint.x - curBounds.origin.x) / curBounds.size.width, (yPosInv - curBounds.origin.y) / curBounds.size.height);
	if(_app != NULL){
		if(_mousePresionado) _app->touchFinalizado(1, curPoint.x, yPosInv, false);
		_app->touchIniciado(1, curPoint.x, yPosInv);
		_mousePresionado = true;
	}
}

- (void)mouseUp:(NSEvent *)theEvent {
	//Screen
	const NSRect viewBounds		= [self bounds];
	const NSPoint viewPoint		= [self convertPoint:[theEvent locationInWindow] fromView:nil];
	//OpenGL
	const NSRect curBounds		= [self convertRectToBacking:viewBounds]; //openGL bounds
	const NSPoint curPoint		= [self convertPointToBacking:viewPoint];
	const float yPosInv			= (curBounds.size.height - curPoint.y);
	//PRINTF_INFO("mouseUp viewPoint(%f, %f) curPoint(%f, %f) rel(%f, %f).\n", viewPoint.x, viewPoint.y, curPoint.x, yPosInv, (curPoint.x - curBounds.origin.x) / curBounds.size.width, (yPosInv - curBounds.origin.y) / curBounds.size.height);
	if(_app != NULL){
		if(_mousePresionado) _app->touchFinalizado(1, curPoint.x, yPosInv, false);
		_mousePresionado = false;
	}
}

- (void)mouseDragged:(NSEvent *)theEvent {
	//Screen
	const NSRect viewBounds		= [self bounds];
	const NSPoint viewPoint		= [self convertPoint:[theEvent locationInWindow] fromView:nil];
	//OpenGL
	const NSRect curBounds		= [self convertRectToBacking:viewBounds]; //openGL bounds
	const NSPoint curPoint		= [self convertPointToBacking:viewPoint];
	const float yPosInv			= (curBounds.size.height - curPoint.y);
	//PRINTF_INFO("mouseDragged viewPoint(%f, %f) curPoint(%f, %f) rel(%f, %f).\n", viewPoint.x, viewPoint.y, curPoint.x, yPosInv, (curPoint.x - curBounds.origin.x) / curBounds.size.width, (yPosInv - curBounds.origin.y) / curBounds.size.height);
	if(_app != NULL){
		if(_mousePresionado) _app->touchMovido(1, curPoint.x, yPosInv);
	}
}

@end




