//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppNucleoEncabezado.h"
#include "AUIOSSurfaceView.h"
#include "AUIOSMonoViewController.h"
#include "AUApp.h"
#ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
#include "AUIOSAudioSession.h"
#endif
//
#include "NBMngrOSTools.h"
#include "NBMngrAVCapture.h"
#import <AVFoundation/AVFoundation.h> //for CMSampleBufferRef

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

//----------------
// Callbacks
//----------------

NBTamanoI AUIOSMonoViewControllerGenerarDatosDeRenderBuffer(const SI32 anchoGLNvo, const SI32 altoGLNvo, void* referenciaObjC);
void AUIOSMonoViewControllerVolcarBuffer(void* param, AUApp* juego, SI32 iEscenaRender);

//----------------
// AUIOSMonoViewController
//----------------

typedef struct AUIOSMonoViewData_ {
	UIView*				viewContainer;	//Container view (visual and secondary views are added to this)
	AUIOSSurfaceView*	viewVisual;	//Visual view
	EAGLContext*		glContext;
	SI32				iScene;
	float				loadedScale;	//layer scale during load (used to scale every screen-change)
	STNBSize			loadedPPI;		//pixels-per-inch set during load (scaled every screen-change)
	AUApp*				app;
	CADisplayLink*		diplayLink;
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	AUIOSAudioSession*	audioSession;
#	endif
} AUIOSMonoViewData;

@implementation AUIOSMonoViewController

+ (float) getDeviceFamilyBasePPI {
	float r = 160.0f;
	switch (UI_USER_INTERFACE_IDIOM()) {
		case UIUserInterfaceIdiomPad:
			r = 132.0f;
			break;
		case UIUserInterfaceIdiomPhone:
			r = 163.0f;
			break;
		default:
			r = 160.0f;
			break;
	}
	return r;
}

- (id) initWithApplication:(UIApplication *)application appName:(const char*)appName window:(UIWindow*)window frame:(CGRect)frame {
	self		= [super init];
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUIOSMonoViewController::initWithFrame:")
	PRINTF_INFO("AUIOSMonoViewController::initWithApplication:appName:window:frame.\n");
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)malloc(sizeof(AUIOSMonoViewData));
	const float basePPI		= [AUIOSMonoViewController getDeviceFamilyBasePPI];
	data->viewContainer	= nil;
	data->viewVisual	= nil;
	data->glContext		= nil;
	data->iScene		= -1;
	data->loadedScale	= 1.0f;
	data->loadedPPI		= NBST_P(STNBSize, basePPI, basePPI );
	data->app			= NULL;
	data->diplayLink	= nil;
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	data->audioSession	= NULL;
#	endif
	//
	_opaqueData			= data;
	//Add container view
	{
		UIView* view				= [[UIView alloc] initWithFrame:frame];
		view.multipleTouchEnabled	= YES;
		self.view					= view;
		data->viewContainer 		= view;
	}
	//Init layer
	{
		AUIOSSurfaceView* view		= [[AUIOSSurfaceView alloc] initWithFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];
		view.multipleTouchEnabled	= YES;
		view.autoresizingMask		= (UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleBottomMargin);
		data->viewVisual			= view; [view retain];
		[data->viewContainer addSubview:view];
		//Create glContexts
		{
			NBASSERT([AUIOSSurfaceView layerClass] == [CAEAGLLayer class])
			if([AUIOSSurfaceView layerClass] == [CAEAGLLayer class]){
				CAEAGLLayer* glLayer		= (CAEAGLLayer *)[view layer];
				glLayer.opaque				= YES;
				glLayer.drawableProperties	= [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
				data->glContext				= [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
				if(data->glContext == nil){
					PRINTF_ERROR("Could not create 'EAGLContext'.\n"); NBASSERT(false);
				} else {
					if(![EAGLContext setCurrentContext:data->glContext]){
						PRINTF_ERROR("Could not set the current 'EAGLContext'.\n"); NBASSERT(false);
					} else {
						PRINTF_INFO("'EAGLContext' created.\n");
#						ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
						data->audioSession	= new AUIOSAudioSession();
						//data->audioSession->establecerCategoria(kAudioSessionCategory_PlayAndRecord);
						//data->audioSession->sobreescribirSalida(true);
						data->audioSession->establecerCategoria(data->audioSession->otroSonidoSeEstaReproduciendo() ? ENAUIOSAudioSessionCat_Ambient : ENAUIOSAudioSessionCat_SoloAmbient);
#						endif
						//
						STAppCallbacks appCallbacks;
						AUApp::inicializarCallbacks(&appCallbacks);
						//Volcado
						appCallbacks.funcGenerarRenderBuffer		= AUIOSMonoViewControllerGenerarDatosDeRenderBuffer;
						appCallbacks.funcGenerarRenderBufferParam	= (__bridge void*)self;
						appCallbacks.funcVolcarBuffer				= AUIOSMonoViewControllerVolcarBuffer;
						appCallbacks.funcVolcarBufferParam			= (__bridge void*)self;
						//
						data->app	= new AUApp(&appCallbacks, appName, false /*permitirActividadRedEnBG*/);
						if(!data->app->inicializarMultimedia(false /*LEER_PRECACHE*/, false /*LEER_CACHE*/, false /*ESCRIBIR_CACHE*/, true /*initGraphics*/, 60)){
							PRINTF_ERROR("No se pudo inicializar el motor grafico\n");
							NBASSERT(false);
						} else {
#                           ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
                            data->audioSession->activarSesion();
#                           endif
							//Sync layer scale with backing scale (enable high-res)
							UIScreen* curScreen	= [window screen];
							if(curScreen == nil){
								PRINTF_INFO("----curScreen is nil, using mainScreen");
								curScreen		= [UIScreen mainScreen]; NBASSERT(curScreen != nil)
							}
							const float screenScale = [curScreen scale]; //scale (the scale the app was launched); nativeScale ( the scale for the screen, not allways the app will be launched at screen scale)
							{
								glLayer.contentsScale	= screenScale;
								view.contentScaleFactor	= screenScale;
								PRINTF_INFO("----screenScale(%.2f).\n", screenScale);
								data->loadedScale		= screenScale;
								data->loadedPPI			= NBST_P(STNBSize, basePPI, basePPI );
							}
							//Init screen
							//CGRect viewBounds			= [view bounds];
							//CGRect layerBounds		= [glLayer bounds]; //logic
							NBTamanoI wSize; NBTAMANO_ESTABLECER(wSize, 0, 0) //Will be defined by callback
							//NBTamanoI wSize; NBTAMANO_ESTABLECER(wSize, viewBounds.size.width * screenScale, viewBounds.size.height * screenScale)
							NBTamano wPPI; NBTAMANO_ESTABLECER(wPPI, basePPI * screenScale, basePPI * screenScale)
							NBTamano wDPI; NBTAMANO_ESTABLECER(wDPI, (wPPI.ancho * 72.0f) / 100.0f, (wPPI.alto * 72.0f) / 100.0f)
							if(!data->app->inicializarVentana(wSize, wPPI, wPPI, ENGestorEscenaDestinoGl_RenderBuffer)){
								PRINTF_ERROR("Could not init the window.\n");
								NBASSERT(false);
							} else {
								PRINTF_INFO("App window created.\n");
								data->iScene		= data->app->indiceEscenaRender();
								//Observe events
								{
									NSNotificationCenter* notifCenter = [NSNotificationCenter defaultCenter];
									//App notifications
									[notifCenter addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:application];
									[notifCenter addObserver:self selector:@selector(applicationDidEnterBackground:) name:UIApplicationDidEnterBackgroundNotification object:application];
									[notifCenter addObserver:self selector:@selector(applicationWillEnterForeground:) name:UIApplicationWillEnterForegroundNotification object:application];
									[notifCenter addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:application];
									[notifCenter addObserver:self selector:@selector(applicationDidReceiveMemoryWarning:) name:UIApplicationDidReceiveMemoryWarningNotification object:application];
									[notifCenter addObserver:self selector:@selector(applicationWillTerminate:) name:UIApplicationWillTerminateNotification object:application];
									//Keyboard notifications
									[notifCenter addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
									[notifCenter addObserver:self selector:@selector(keyboardDidShow:) name:UIKeyboardDidShowNotification object:nil];
									[notifCenter addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
									[notifCenter addObserver:self selector:@selector(keyboardDidHide:) name:UIKeyboardDidHideNotification object:nil];
									[notifCenter addObserver:self selector:@selector(keyboardWillChangeFrame:) name:UIKeyboardWillChangeFrameNotification object:nil];
									[notifCenter addObserver:self selector:@selector(keyboardDidChangeFrame:) name:UIKeyboardDidChangeFrameNotification object:nil];
									//
									data->app->appStateOnCreate();
								}
							}
						}
					}
				}
			}
		}
		data->viewContainer.contentScaleFactor = data->viewVisual.contentScaleFactor;
		[view release];
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return self;
}

- (void) dealloc {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUIOSMonoViewController::dealloc")
	PRINTF_INFO("AUIOSMonoViewController::dealloc.\n");
	if(_opaqueData != NULL){
		AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
		[[NSNotificationCenter defaultCenter] removeObserver:self];
		if(data->diplayLink != nil) [data->diplayLink invalidate]; data->diplayLink = nil;
		if(data->app != NULL) delete data->app; data->app = NULL;
		if(data->glContext != nil) [data->glContext release]; data->glContext = nil;
		if(data->viewVisual != nil) [data->viewVisual release]; data->viewVisual = nil;
		if(data->viewContainer != nil) [data->viewContainer release]; data->viewContainer = nil;
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		if(data->audioSession != NULL){
			data->audioSession->desactivarSesion();
			delete data->audioSession;
			data->audioSession = NULL;
		}
#		endif
		free(_opaqueData);
		_opaqueData = NULL;
	}
	[super dealloc];
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

//-----------------
// Public methods
//-----------------

- (AUApp*) getApp {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUIOSMonoViewController::getApp")
	AU_GESTOR_PILA_LLAMADAS_POP_3
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	return data->app;
}

- (SI32) iScene {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUIOSMonoViewController::iScene")
	SI32 r = -1;
	if(_opaqueData != NULL){
		AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
		r = data->iScene;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

- (bool) setAppAdmin:(AUAppEscenasAdmin*)appAdmin {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUIOSMonoViewController::setAppAdmin")
	bool r = false;
	if(_opaqueData != NULL){
		AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
		@autoreleasepool {
			if(!data->app->inicializarJuego(appAdmin)){
				PRINTF_ERROR("Could not setAppAdmin.\n");
			} else {
				PRINTF_INFO("setAppAdmin success!\n");
				data->app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
				data->app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
				r = true;
			}
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_3
	return r;
}

//-----------------
// Status bar
//-----------------

-(UIStatusBarStyle) preferredStatusBarStyle {
	UIStatusBarStyle r = UIStatusBarStyleDefault;
	const ENStatusBarStyle style = NBMngrOSTools::getBarStyle();
	if (@available(iOS 13.0, *)) {
		r = (style == ENStatusBarStyle_Light ? UIStatusBarStyleLightContent : UIStatusBarStyleDarkContent);
	} else {
		r = (style == ENStatusBarStyle_Light ? UIStatusBarStyleLightContent : UIStatusBarStyleDefault);
	}
	return r;
}

//-----------------
// Events
//-----------------

- (void)viewDidLayoutSubviews {
	PRINTF_INFO("AUIOSMonoViewController::viewDidLayoutSubviews.\n");
	@autoreleasepool {
		AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
		const float basePPI		= [AUIOSMonoViewController getDeviceFamilyBasePPI];
		//
		NBASSERT([self view] == data->viewContainer)
		NBASSERT(data->viewContainer != nil)
		NBASSERT(data->viewVisual != nil)
		if(data->viewVisual != nil){
			UIView* view		= data->viewVisual;
			CAEAGLLayer* layer	= (CAEAGLLayer *)[view layer];
			UIScreen* curScreen	= [[view window] screen];
			if(curScreen == nil){
				PRINTF_INFO("----curScreen is nil, using mainScreen");
				curScreen		= [UIScreen mainScreen]; NBASSERT(curScreen != nil)
			}
			const float screenScale = [curScreen scale]; //scale (the scale the app was launched); nativeScale ( the scale for the screen, not allways the app will be launched at screen scale)
			{
				view.contentScaleFactor	= screenScale;
				layer.contentsScale		= screenScale;
				PRINTF_INFO("----screenScale(%.2f).\n", screenScale);
			}
			//Init screen
			//CGRect viewBounds			= [view bounds];
			//CGRect layerBounds			= [layer bounds]; //logic
			const float relWinScale		= (data->loadedScale / screenScale);
			NBTamanoI wSize;	NBTAMANO_ESTABLECER(wSize, 0, 0) //Will be defined by callback
			NBTamano wPPI;		NBTAMANO_ESTABLECER(wPPI, basePPI * screenScale, basePPI * screenScale)
			NBTamano wDPI;		NBTAMANO_ESTABLECER(wDPI, (wPPI.ancho * 72.0f) / 100.0f, (wPPI.alto * 72.0f) / 100.0f)
			//NBTamano wPPIScale; NBTAMANO_ESTABLECER(wPPIScale, data->loadedPPI.width / wPPI.ancho, data->loadedPPI.height / wPPI.alto)
			//NBTamano wPPIToUse; NBTAMANO_ESTABLECER(wPPIToUse, (wPPI.ancho * relWinScale) / wPPIScale.ancho, (wPPI.alto * relWinScale) / wPPIScale.alto)
			//NBTamano wDPIToUse; NBTAMANO_ESTABLECER(wDPIToUse, (wPPIToUse.ancho * 72.0f) / 100.0f, (wPPIToUse.alto * 72.0f) / 100.0f)
			data->app->notificarRedimensionVentana(wSize, relWinScale, wPPI, wPPI);
			/*
			 UIView* view		= [self view]; NBASSERT(view != nil)
			 UIScreen* curScreen	= [window screen];
			 if(curScreen == nil){
			 PRINTF_INFO("----curScreen is nil, using mainScreen");
			 curScreen		= [UIScreen mainScreen]; NBASSERT(curScreen != nil)
			 }
			 const float screenScale = [curScreen scale]; //scale (the scale the app was launched); nativeScale ( the scale for the screen, not allways the app will be launched at screen scale)
			 {
			 glLayer.contentsScale	= screenScale;
			 view.contentScaleFactor	= screenScale;
			 PRINTF_INFO("----screenScale(%.2f).\n", screenScale);
			 data->loadedScale		= screenScale;
			 data->loadedPPI			= NBST_P(STNBSize, basePPI, basePPI );
			 }
			 //Init screen
			 CGRect viewBounds			= [view bounds];
			 CGRect layerBounds			= [glLayer bounds]; //logic
			 NBTamanoI wSize; NBTAMANO_ESTABLECER(wSize, 0, 0) //Will be defined by callback
			 //NBTamanoI wSize; NBTAMANO_ESTABLECER(wSize, viewBounds.size.width * screenScale, viewBounds.size.height * screenScale)
			 NBTamano wPPI; NBTAMANO_ESTABLECER(wPPI, basePPI * screenScale, basePPI * screenScale)
			 NBTamano wDPI; NBTAMANO_ESTABLECER(wDPI, (wPPI.ancho * 72.0f) / 100.0f, (wPPI.alto * 72.0f) / 100.0f)
			 */
		}
	}
}

//-----------------
// Touches
//-----------------

-(void) touchesBegan: (NSSet *) touches withEvent: (UIEvent *) event {
	UIView* view = self.view;
	if(view != nil){
		@autoreleasepool {
			SI32 i = 0;
			CGFloat escalaGL		= [view contentScaleFactor];
			NSArray* arrTouches		= [touches allObjects];
			for(i = 0; i < [arrTouches count]; i++){
				UITouch *touch		= [arrTouches objectAtIndex:i];
				CGPoint posicion	= [touch locationInView: view];
				//UITouchPhase fase	= [touch phase];
				//NSUInteger hash	= [touch hash];
				PRINTF_INFO("View, TOUCH INI (%llu) (%.2f, %.2f)\n", (UI64)touch, posicion.x, posicion.y);
				if(NBGestorTouches::gestorInicializado()){
					NBGestorTouches::touchIniciar((UI64)touch, posicion.x * escalaGL, posicion.y * escalaGL);
				}
			}
		}
	}
}

-(void) touchesMoved: (NSSet *) touches withEvent: (UIEvent *) event {
	UIView* view = self.view;
	if(view != nil){
		@autoreleasepool {
			SI32 i = 0;
			CGFloat escalaGL		= [view contentScaleFactor];
			NSArray* arrTouches		= [touches allObjects];
			for(i = 0; i < [arrTouches count]; i++){
				UITouch *touch		= [arrTouches objectAtIndex:i];
				CGPoint posicion	= [touch locationInView: view];
				//non reported touches
				{
					NSArray<UITouch *>* missedTouches = [event coalescedTouchesForTouch:touch];
					if(missedTouches && [missedTouches count] > 1){ //it includes the touch itself
						const NSUInteger count = (NSUInteger)[missedTouches count]  - 1; //it includes the touch itself
						NSUInteger i; for(i = 0; i < count; i++){
							UITouch* touch2 = [missedTouches objectAtIndex: i];
							if(touch2){
								CGPoint pos2 = [touch2 locationInView: view];
								PRINTF_INFO("View, TOUCH MOV-MISSED (%llu) (%.2f, %.2f)\n", (UI64)touch2, pos2.x, pos2.y);
								if(NBGestorTouches::gestorInicializado()){
									NBGestorTouches::touchMover((UI64)touch, pos2.x * escalaGL, pos2.y * escalaGL);
								}
							}
						}
					}
				}
				//UITouchPhase fase	= [touch phase];
				//NSUInteger hash	= [touch hash];
				PRINTF_INFO("View, TOUCH MOV (%llu) (%.2f, %.2f)\n", (UI64)touch, posicion.x, posicion.y);
				if(NBGestorTouches::gestorInicializado()){
					NBGestorTouches::touchMover((UI64)touch, posicion.x * escalaGL, posicion.y * escalaGL);
				}
			}
		}
	}
}

-(void) touchesEnded:(NSSet *) touches withEvent: (UIEvent *) event {
	UIView* view = self.view;
	if(view != nil){
		@autoreleasepool {
			SI32 i = 0;
			CGFloat escalaGL		= [view contentScaleFactor];
			NSArray* arrTouches		= [touches allObjects];
			for(i = 0; i < [arrTouches count]; i++){
				UITouch *touch		= [arrTouches objectAtIndex:i];
				CGPoint posicion	= [touch locationInView: view];
				//non reported touches
				{
					NSArray<UITouch *>* missedTouches = [event coalescedTouchesForTouch:touch];
					if(missedTouches && [missedTouches count] > 1){ //it includes the touch itself
						const NSUInteger count = (NSUInteger)[missedTouches count] - 1; //it includes the touch itself
						NSUInteger i; for(i = 0; i < count; i++){
							UITouch* touch2 = [missedTouches objectAtIndex: i];
							if(touch2){
								CGPoint pos2 = [touch2 locationInView: view];
								PRINTF_INFO("View, TOUCH MOV-MISSED (%llu) (%.2f, %.2f)\n", (UI64)touch2, pos2.x, pos2.y);
								if(NBGestorTouches::gestorInicializado()){
									NBGestorTouches::touchMover((UI64)touch, pos2.x * escalaGL, pos2.y * escalaGL);
								}
							}
						}
					}
				}
				//UITouchPhase fase	= [touch phase];
				//NSUInteger hash	= [touch hash];
				PRINTF_INFO("View, TOUCH FIN (%llu) (%.2f, %.2f)\n", (UI64)touch, posicion.x, posicion.y);
				if(NBGestorTouches::gestorInicializado()){
					NBGestorTouches::touchFinalizar((UI64)touch, posicion.x * escalaGL, posicion.y * escalaGL, false);
				}
			}
		}
	}
}

-(void) touchesCancelled: (NSSet *) touches withEvent: (UIEvent *) event {
	UIView* view = self.view;
	if(view != nil){
		@autoreleasepool {
			SI32 i = 0;
			CGFloat escalaGL		= [view contentScaleFactor];
			NSArray* arrTouches		= [touches allObjects];
			for(i = 0; i < [arrTouches count]; i++){
				UITouch *touch		= [arrTouches objectAtIndex:i];
				CGPoint posicion	= [touch locationInView: view];
				//UITouchPhase fase	= [touch phase];
				//NSUInteger hash	= [touch hash];
				PRINTF_INFO("View, TOUCH CANCEL (%llu) (%.2f, %.2f)\n", (UI64)touch, posicion.x, posicion.y);
				if(NBGestorTouches::gestorInicializado()){
					NBGestorTouches::touchFinalizar((UI64)touch, posicion.x * escalaGL, posicion.y * escalaGL, true);
				}
			}
		}
	}
}

/*- (void)touchesEstimatedPropertiesUpdated: (NSSet<UITouch *> *)touches {
	PRINTF_INFO("touchesEstimatedPropertiesUpdated called.\n");
	[super touchesEstimatedPropertiesUpdated: touches];
}*/
	
//-----------------
// Notifications (Application)
//-----------------

- (void)applicationDidBecomeActive:(NSNotification*)notif {
	PRINTF_INFO("AUIOSMonoViewController::applicationDidBecomeActive.\n");
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	if(!data->diplayLink){
		@autoreleasepool {
			data->diplayLink = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(tickScreen)];
			//[data->diplayLink setPreferredFramesPerSecond:0];
			//[data->diplayLink setFrameInterval:1]; //1=cada refrescamiento de pantalla (este numero es la cantidad de ciclos necesarios para disparar el evento)
			[data->diplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
	}
	data->app->appStateOnStart();
}

- (void)applicationDidEnterBackground:(NSNotification*)notif {
	PRINTF_INFO("AUIOSMonoViewController::applicationDidEnterBackground.\n");
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	data->app->appStateOnPause();
}

- (void)applicationWillEnterForeground:(NSNotification*)notif {
	PRINTF_INFO("AUIOSMonoViewController::applicationWillEnterForeground.\n");
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	data->app->appStateOnResume();
}

- (void)applicationWillResignActive:(NSNotification*)notif {
	PRINTF_INFO("AUIOSMonoViewController::applicationWillResignActive.\n");
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	if(data->diplayLink){
		@autoreleasepool {
			[data->diplayLink invalidate];
			data->diplayLink = nil;
		}
	}
	data->app->appStateOnStop();
}

- (void)applicationWillTerminate:(NSNotification*)notif {
	PRINTF_INFO("AUIOSMonoViewController::applicationWillTerminate.\n");
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	data->app->appStateOnDestroy();
}

- (void)applicationDidReceiveMemoryWarning:(NSNotification*)notif {
	PRINTF_INFO("AUIOSMonoViewController::applicationDidReceiveMemoryWarning!\n");
}

//-----------------
// Notifications (Keyboard)
//-----------------

- (void)keyboardWillShow:(NSNotification*)aNotification {
	//PRINTF_INFO("AUIOSMonoViewController::keyboardWillShow.\n");
	//NSLog(@"%@", aNotification);
}

- (void)keyboardDidShow:(NSNotification*)aNotification {
	//PRINTF_INFO("AUIOSMonoViewController::keyboardDidShow.\n");
	//NSLog(@"%@", aNotification);
}

- (void)keyboardWillHide:(NSNotification*)aNotification {
	//PRINTF_INFO("AUIOSMonoViewController::keyboardWillHide.\n");
	//NSLog(@"%@", aNotification);
	NBGestorTeclas::establecerTecladoEnPantallaAlto(0.0f, true);
	NBGestorTeclas::escuchadorRemover();
}

- (void)keyboardDidHide:(NSNotification*)aNotification {
	//PRINTF_INFO("AUIOSMonoViewController::keyboardDidHide.\n");
	//NSLog(@"%@", aNotification);
	NBGestorTeclas::establecerTecladoEnPantallaAlto(0.0f, true);
	NBGestorTeclas::escuchadorRemover();
}

- (void)keyboardWillChangeFrame:(NSNotification*)aNotification {
	//PRINTF_INFO("AUIOSMonoViewController::keyboardWillChangeFrame.\n");
	//NSLog(@"%@", aNotification);
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	if(data->viewVisual != nil){
		@autoreleasepool {
			NSDictionary* info			= [aNotification userInfo];
			const CGRect keybRect		= [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
			const CGRect commonRect		= [data->viewVisual convertRect:keybRect fromView:data->viewVisual.window];
			const float contentScale	= [data->viewVisual contentScaleFactor];
			const float sceneHeight		= (float)commonRect.size.height * contentScale;
			//PRINTF_INFO("Teclado cambiara area: orig(%f, %f) tam(%f, %f) escala(%f).\n", commonRect.origin.x, commonRect.origin.y, commonRect.size.width, commonRect.size.height, contentScale);
			NBGestorTeclas::establecerTecladoEnPantallaAlto(sceneHeight, true);
		}
	}
}

- (void)keyboardDidChangeFrame:(NSNotification*)aNotification {
	//PRINTF_INFO("AUIOSMonoViewController::keyboardDidChangeFrame.\n");
	//NSLog(@"%@", aNotification);
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	if(data->viewVisual != nil){
		@autoreleasepool {
			NSDictionary* info = [aNotification userInfo];
			const CGRect keybRect		= [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
			const CGRect commonRect		= [data->viewVisual convertRect:keybRect fromView:data->viewVisual.window];
			const float contentScale	= [data->viewVisual contentScaleFactor];
			const float sceneHeight		= (float)commonRect.size.height * contentScale;
			//PRINTF_INFO("Teclado ha cambiado area: orig(%f, %f) tam(%f, %f) escala(%f).\n", commonRect.origin.x, commonRect.origin.y, commonRect.size.width, commonRect.size.height, contentScale);
			NBGestorTeclas::establecerTecladoEnPantallaAlto(sceneHeight, true);
		}
	}
}

//-----------------
// Display link method
//-----------------

-(void) tickScreen {
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUIOSMonoViewController::tickScreen")
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	if(data->app != NULL){
		@autoreleasepool {
			data->app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

//-----------------
// Interfaces orientations
//-----------------

//Only for iOS5-
#if __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_6_0
-(BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation {
	//UIInterfaceOrientationMaskPortrait			= (1 << UIInterfaceOrientationPortrait),
	//UIInterfaceOrientationMaskLandscapeLeft		= (1 << UIInterfaceOrientationLandscapeLeft),
	//UIInterfaceOrientationMaskLandscapeRight		= (1 << UIInterfaceOrientationLandscapeRight),
	//UIInterfaceOrientationMaskPortraitUpsideDown	= (1 << UIInterfaceOrientationPortraitUpsideDown),
	const UI32 orient = NBMngrOSTools::orientationsMask();
	const NSUInteger orient2 = ((orient & ENAppOrientationBit_Portrait) ? UIInterfaceOrientationMaskPortrait : 0)
	| ((orient & ENAppOrientationBit_LandscapeLeftBtn) ? UIInterfaceOrientationMaskLandscapeLeft : 0)
	| ((orient & ENAppOrientationBit_LandscapeRightBtn) ? UIInterfaceOrientationMaskLandscapeRight : 0)
	| ((orient & ENAppOrientationBit_PortraitInverted) ? UIInterfaceOrientationMaskPortraitUpsideDown : 0);
	return (((1 << interfaceOrientation) & orient2) != 0) ? YES : NO;
}
#endif

//Only iOS6+
- (BOOL) shouldAutorotate {
	return (NBMngrOSTools::canAutorotate() ? YES : NO);
	//return YES;
}

//Only iOS6+
-(NSUInteger) supportedInterfaceOrientations {
	//UIInterfaceOrientationMaskPortrait			= (1 << UIInterfaceOrientationPortrait),
	//UIInterfaceOrientationMaskLandscapeLeft		= (1 << UIInterfaceOrientationLandscapeLeft),
	//UIInterfaceOrientationMaskLandscapeRight		= (1 << UIInterfaceOrientationLandscapeRight),
	//UIInterfaceOrientationMaskPortraitUpsideDown	= (1 << UIInterfaceOrientationPortraitUpsideDown),
	const UI32 orient = NBMngrOSTools::orientationsMask();
	const NSUInteger orient2 = ((orient & ENAppOrientationBit_Portrait) ? UIInterfaceOrientationMaskPortrait : 0)
	| ((orient & ENAppOrientationBit_LandscapeLeftBtn) ? UIInterfaceOrientationMaskLandscapeLeft : 0)
	| ((orient & ENAppOrientationBit_LandscapeRightBtn) ? UIInterfaceOrientationMaskLandscapeRight : 0)
	| ((orient & ENAppOrientationBit_PortraitInverted) ? UIInterfaceOrientationMaskPortraitUpsideDown : 0);
	return orient2;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
	UIInterfaceOrientation r = UIInterfaceOrientationPortrait;
	const UI32 orient = NBMngrOSTools::getOrientationPrefered();
	if(orient & ENAppOrientationBit_Portrait){
		r = UIInterfaceOrientationPortrait;
	} else if(orient & ENAppOrientationBit_LandscapeLeftBtn){
		r = UIInterfaceOrientationLandscapeLeft;
	} else if(orient & ENAppOrientationBit_LandscapeRightBtn){
		r = UIInterfaceOrientationLandscapeRight;
	} else if(orient & ENAppOrientationBit_PortraitInverted){
		r = UIInterfaceOrientationPortraitUpsideDown;
	} else {
		r = UIInterfaceOrientationPortrait;
	}
	return r;
}

//-----------------
// Callbacks (ToDo: move to 'glue' classes)
//-----------------

- (NBTamanoI) generateRenderBuffer:(SI32)anchoGLNvo altoNvo:(SI32)altoGLNvo {
	AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
	NBTamanoI r; r.ancho = r.alto = 0;
	NBASSERT(data->viewVisual != nil)
	if(data->viewVisual != nil){
		@autoreleasepool {
			NBASSERT([[data->viewVisual layer] class] == [CAEAGLLayer class])
			if([[data->viewVisual layer] class] == [CAEAGLLayer class]){
				AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
				GLint glWidth = 0, glHeight = 0;
				glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &glWidth);		GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
				glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &glHeight);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
				NBASSERT([EAGLContext currentContext] == data->glContext)
				VERIFICA_ERROR_GL("renderbufferStorage(antes)")
				if(![data->glContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)[data->viewVisual layer]]){
					GL_CMD_EJECUTADO("renderbufferStorage")
					PRINTF_ERROR("Generando el RENDERBUFFER con renderbufferStorage\n");
					glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &glWidth);		GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
					glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &glHeight);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
					PRINTF_INFO("Datos de render buffer generados a dimensiones (%d, %d)\n", glWidth, glHeight);
					NBASSERT(false);
				} else {
					GL_CMD_EJECUTADO("renderbufferStorage")
					glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &glWidth);		GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
					glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &glHeight);	GL_CMD_EJECUTADO("glGetRenderbufferParameterivOES")
					PRINTF_INFO("Datos de render buffer generados a dimensiones (%d, %d)\n", glWidth, glHeight);
					r.ancho	= glWidth;
					r.alto	= glHeight;
					NBASSERT(glWidth > 0 && glHeight > 0)
				}
			}
		}
	}
	return r;
}

-(void) swapRenderBuffer {
	if(_opaqueData != NULL){
		AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
		if(NBGestorEscena::registroEscenaOcupado(data->iScene)){
			@autoreleasepool {
				STGestorEscenaEscena propsEscenaRender = NBGestorEscena::propiedadesEscena(data->iScene);
				STGestorEscenaFrameBuffer propsFramebufferRender = NBGestorEscena::propiedadesFramebuffer(propsEscenaRender.iFramebufferEscena);
				//Es preferible llamada la funcion OpenGL a utlizar el NBGestorGL
				//Porque esta se llama desde un segundo contexto y se alteraria la cacheEstado del contexto de trabajo.
				UI32 idRenderBuffer = propsFramebufferRender.renderbufferOriginal->idRenderBufferGl();
				NBGestorGL::bindRenderbufferEXT(GL_RENDERBUFFER_EXT, idRenderBuffer);
				GL_CMD_EJECUTADO("bindRenderbufferEXT(GL_RENDERBUFFER_EXT, %d)", (SI32)idRenderBuffer);
				[data->glContext presentRenderbuffer:GL_RENDERBUFFER_EXT];
				GL_CMD_EJECUTADO("presentRenderbuffer(GL_RENDERBUFFER_EXT)");
			}
		}
	}
}

@end

//Metodos C
NBTamanoI AUIOSMonoViewControllerGenerarDatosDeRenderBuffer(const SI32 anchoGLNvo, const SI32 altoGLNvo, void* referenciaObjC){
	return [((__bridge AUIOSMonoViewController*)referenciaObjC) generateRenderBuffer:anchoGLNvo altoNvo:altoGLNvo];
}

void AUIOSMonoViewControllerVolcarBuffer(void* param, AUApp* juego, SI32 iEscenaRender){
	if(param) [((__bridge AUIOSMonoViewController*)param) swapRenderBuffer];
}
