//
//  EAGLViewController.m
//  Gameplay_iOS
//
//  Created by Marcos Ortega on 29/09/12.
//
//

#import "EAGLViewController.h"
//
#import <ImageIO/ImageIO.h> //Para el procesamiento de imagenes (CGImageSourceCreateWithURL y otros)
//
#include "AUEscenaDemoTextRender.h"
#include "AUEscenaDemoTextBox.h"

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

@interface EAGLViewController ()

@end

@implementation EAGLViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil frame:(CGRect)frame {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        //------------------------
		//--- Identificar version de iOS
		//------------------------
		_versionIOS					= [[[UIDevice currentDevice] systemVersion] floatValue];
		//
		_tecladoPermitidoMostrar	= false;
		_tecladoEstaVisible			= false;
		//
		_ticksAcumuladosEnSegundo	= 0;
		_ticksAnimadoresAcum		= 0;
		_ticksPantalaAcum			= 0;
		//#################################
		//### Iniciar motor de audio
		//#################################
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		_sessionAudio				= new iOSAudioSession();
		//_sessionAudio->establecerCategoria(kAudioSessionCategory_PlayAndRecord);
		//_sessionAudio->sobreescribirSalida(true);
		_sessionAudio->establecerCategoria(_sessionAudio->otroSonidoSeEstaReproduciendo() ? kAudioSessionCategory_AmbientSound : kAudioSessionCategory_SoloAmbientSound);
		_sessionAudio->activarSesion();
#		endif
		//######################################
		//### Iniciar juego
		//######################################
		_vistaOpenGL								= [[EAGLView alloc] initWithFrame:frame];
		//
		STAppCallbacks appCallbacks;				AUApp::inicializarCallbacks(&appCallbacks);
		//Volcado
		appCallbacks.funcGenerarRenderBuffer		= EAGLviewGenerarDatosDeRenderBuffer;
		appCallbacks.funcGenerarRenderBufferParam	= (__bridge void*)_vistaOpenGL;
		appCallbacks.funcVolcarBuffer				= volcarBuffer;
		appCallbacks.funcVolcarBufferParam			= (__bridge void*)self;
		//Teclado
		appCallbacks.funcTecladoVisible				= tecladoVisible;
		appCallbacks.funcTecladoVisibleParam		= (__bridge void*)self;
		appCallbacks.funcTecladoMostrar				= tecladoMostrar;
		appCallbacks.funcTecladoMostrarParam		= (__bridge void*)self;
		appCallbacks.funcTecladoOcultar				= tecladoOcultar;
		appCallbacks.funcTecladoOcultarParam		= (__bridge void*)self;
		//
		_app										= new AUApp(&appCallbacks, "SereneHearts", true /*permitirActividadRedEnBG*/, NULL, NULL);
		[_vistaOpenGL establecerJuego:_app];
		if([_vistaOpenGL crearContextoOpenGL] == nil){
			PRINTF_ERROR("Creando contexto OpenGL\n"); NBASSERT(false);
		} else {
			self.view						= _vistaOpenGL;
			//
			if(!_app->inicializarMultimedia(LEER_PRECACHE, LEER_CACHE, ESCRIBIR_CACHE, true /*initGraphics*/, 60)){
				PRINTF_ERROR("No se pudo inicializar el motor multimedia\n"); NBASSERT(false);
			} else {
				//
#				ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
				//ALCcontext* contextoOpenAL = NULL; NBGestorSonidos::dameContexto(&contextoOpenAL);
				//_sessionAudio->establecerContextoOpenAL(contextoOpenAL);
#				endif
				//
				//_app->aplicarConfiguracion(); //[self activarIdiomaPreferido];
				//Determinar la escala de la pantalla
				const float pantallaEscala		= [_vistaOpenGL escalaParaAnchoNativo];
				const float pantallaDensidad	= [_vistaOpenGL pantallaPuntosPorPulgada];
				const float pantallaAncho		= [_vistaOpenGL pantallaAnchoLogico];
				const float pantallaAlto		= [_vistaOpenGL pantallaAltoLogico];
				const float pixelesTotal		= (pantallaAncho  * pantallaAlto);
				PRINTF_INFO("AUApp::densidad(%fppp).\n", pantallaDensidad);
				/*UIScreen* mainScreen		= [UIScreen mainScreen];
				float escalaUsarPantalla	= 1.0f;
				CGRect areaPantalla			= [mainScreen bounds]; PRINTF_INFO("AREA PANTALLA (%d x %d)\n", (int)areaPantalla.size.width, (int)areaPantalla.size.height);
				//float pixelesPantalla		= (areaPantalla.size.width * areaPantalla.size.height);
				if([mainScreen respondsToSelector:@selector(scale)]) escalaUsarPantalla = [mainScreen scale];*/
				//if(_app->configEscalaHaciaHDEsManual()) escalaUsarPantalla = _app->configEscalaHaciaHD();
				CAEAGLLayer *eaglLayer			= (CAEAGLLayer *)[_vistaOpenGL layer];
				eaglLayer.contentsScale			= pantallaEscala;
				_vistaOpenGL.contentScaleFactor	= pantallaEscala;
				//
				//iPad: 132dpi (~800K pixeles)
				//iPhone & iPodTouch & iPadMini: 163dpi
				//iPhone & iPodTouch Retina & iPadMini2: 326dpi (~600K - ~750K pixeles)
				//iPad Retina: 264dpi (~3M pixeles)
				//
				// Por cant pixeles:  iPhone: 153,600px, iPhoneRetina: 614,400px, iPhoneRetina4In: 727,040px, iPad: 786,432px, iPadRetina: 3,145,728
				// Por tamMin:        iPhone: 320(200%), iPhoneRetina: 640(100%), iPhoneRetina4In: 640(100%), iPad: 768(78.%), iPadRetina 1536(41%)
				const float escalaRecomendada 	= 1.0f; //(pantallaDensidad < (72.0f * 3.0f) ? 2.0f : pixelesTotal < 1500000 ? 1.0f : 0.5f);
				/*const float tamMinPantalla	= (areaPantalla.size.width < areaPantalla.size.height ? areaPantalla.size.width : areaPantalla.size.height) * escalaUsarPantalla;
				const float escalaEscena		= (tamMinPantalla < 640.0f ? (640.0f / tamMinPantalla) : tamMinPantalla > 768.0f ? (768.0f / tamMinPantalla) : 1.0f);*/
				//Inicializar ventana
				if(!_app->inicializarVentana(0, 0, pantallaDensidad, pantallaDensidad, ENGestorEscenaDestinoGl_RenderBuffer)){
					PRINTF_ERROR("No se pudo inicializar la ventana.\n");
				} else {
					const char* paquetes[] = {
						"paqFuentes.otf"
                        , "paqPNGx8_onlyAnims.png"
						, "paqPNGx4_onlyAnims.png"
						, "paqPNGx2_onlyAnims.png"
						, "paqPNGx1_onlyAnims.png"
						, "paqAnimaciones.xml"
						, "paqSonidos.wav"
						//Optionals (provided as download in some distributions)
                        , "paqPNGx8.png"
						, "paqPNGx4.png"
						, "paqPNGx2.png"
						, "paqPNGx1.png"
						, "paqMusicaOgg.wav"
					};
					const SI32 iPrimerPaqueteCargar = 0;
					const SI32 conteoPaquetesCargar = 12;
					//
					_escenas = new AUAppEscenasAdminSimple(_app->indiceEscenaRender(), ENGestorTexturaModo_cargaInmediata, PAQUETES_RUTA_BASE, &paquetes[iPrimerPaqueteCargar], conteoPaquetesCargar);
					if(!_app->inicializarJuego(_escenas)){
						PRINTF_ERROR("No se pudo inicializar el juego.\n");
					} else {
						PRINTF_INFO("Juego iniciado!\n");
						//Escuchar eventos de teclado
						[[NSNotificationCenter defaultCenter] addObserver:self
																 selector:@selector(keyboardWillShow:)
																	 name:UIKeyboardWillShowNotification object:nil];
						[[NSNotificationCenter defaultCenter] addObserver:self
																 selector:@selector(keyboardDidShow:)
																	 name:UIKeyboardDidShowNotification object:nil];
						[[NSNotificationCenter defaultCenter] addObserver:self
																 selector:@selector(keyboardWillHide:)
																	 name:UIKeyboardWillHideNotification object:nil];
						[[NSNotificationCenter defaultCenter] addObserver:self
																 selector:@selector(keyboardDidHide:)
																	 name:UIKeyboardDidHideNotification object:nil];
						[[NSNotificationCenter defaultCenter] addObserver:self
																 selector:@selector(keyboardWillChangeFrame:)
																	 name:UIKeyboardWillChangeFrameNotification object:nil];
						[[NSNotificationCenter defaultCenter] addObserver:self
																 selector:@selector(keyboardDidChangeFrame:)
																	 name:UIKeyboardDidChangeFrameNotification object:nil];
						//
						/*{
							AUEscenaDemoTextRender* demo = new AUEscenaDemoTextRender(_app->indiceEscenaRender());
							_escenas->escenaCargar(demo);
							demo->liberar(NB_RETENEDOR_THIS);
						}*/
						//
						{
							AUEscenaDemoTextBox* demo = new AUEscenaDemoTextBox(_app->indiceEscenaRender());
							_escenas->escenaCargar(demo);
							demo->liberar(NB_RETENEDOR_THIS);
						}
					}
					//iOS 6, StatusBarTintColor
					//[[UITabBar appearance] setTintColor:[UIColor redColor]];
					//[UINavigationBar appearance].tintColor = [UIColor blueColor];
					//self.navigationController.navigationBar.tintColor = [UIColor colorWithRed:215.0f/255.0f green:125.0f/255.0f blue:30.0f/255.0f alpha:1.0f];
					//self.navigationController.navigationBar.barStyle = UIBarStyleBlackOpaque;
					//Hacer "tick" para evitar la pantalla en blanco
					[self tickPantalla];
					[self tickPantalla];
				}
			}
		}
    }
    return self;
}

- (void)dealloc {
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("Gameplay_iOSAppDelegate::dealloc")
	if(_app != NULL){
		_app->finalizarMultimedia();
		delete _app;
		_app = NULL;
	}
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	delete _sessionAudio;
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

- (void)finalizar {
	//######################################
	//### FINALIZACION DE JUEGO
	//######################################
	if(_app != NULL){
		_app->finalizarMultimedia();
		delete _app;
		_app = NULL;
	}
	//######################################
	//### FINALIZACION DE MOTOR
	//######################################
	if(_sincronizadorPantalla){
		[_sincronizadorPantalla invalidate]; _sincronizadorPantalla = nil;
	}
	if(_vistaOpenGL){
		[_vistaOpenGL destruirContextoOpenGL];
		[_vistaOpenGL removeFromSuperview];
		//[_vistaOpenGL release];
	}
	//Session de audio iOS
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	_sessionAudio->establecerContextoOpenAL(NULL);
	_sessionAudio->desactivarSesion();
#	endif
	#ifdef CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES
	PRINTF_INFO("-------------------------------------\n");
	PRINTF_INFO("PUNTEROS EN USO AL FINAL (deberia ser ninguno)\n");
	NBGestorMemoria::debug_imprimePunterosEnUso();
	PRINTF_INFO("FIN DE LISTA\n");
	PRINTF_INFO("-------------------------------------\n");
	#endif
}

/*- (ENBaseRotacionDispositivo) rotacionDispositivo {
	return _rotacionDispositivo;
}*/

- (void) focoObtenido {
	//
}

- (void) focoPerdido {
	//PARA PROBAR
	/*PRINTF_WARNING("---------------------------------------------\n");
	 PRINTF_WARNING("--- RECUERDA QUITAR ESTE CODIGO DE PRUEBA ---\n");
	 PRINTF_WARNING("---------------------------------------------\n");
	 _app->reinicializarMultimedia();*/
	//PARA PROBAR
	/*PRINTF_WARNING("---------------------------------------------\n");
	PRINTF_WARNING("--- RECUERDA QUITAR ESTE CODIGO DE PRUEBA ---\n");
	PRINTF_WARNING("---------------------------------------------\n");
	if(_app != NULL){
		PRINTF_INFO("Finalizando juego.\n");
		_app->finalizarJuego();	//Libera las escenas el resumeDebug
		PRINTF_INFO("Exito finalizando juego.\n");
	}
	if(_escenas != NULL){
		PRINTF_INFO("Liberando _escenas.\n");
		_escenas->liberar(NB_RETENEDOR_THIS);
		PRINTF_INFO("Exito liberando _escenas.\n");
		_escenas = NULL;
	}
	if(_app != NULL){
		PRINTF_INFO("Finalizando ventana.\n");
		_app->finalizarVentana();	//Destruye la escena
		PRINTF_INFO("Exito finalizando ventana.\n");
	}
	PRINTF_INFO("Finalizando graficos.\n");
	AUApp::finalizarMultimedia();
	PRINTF_INFO("Exito finalizando graficos.\n");
	if(_app != NULL){
		PRINTF_INFO("Liberando _app.\n");
		delete _app; //->liberar(NB_RETENEDOR_THIS);
		PRINTF_INFO("Exito liberando _app.\n");
		_app = NULL;
	}
	PRINTF_INFO("Finalizando nucleo.\n");
	AUApp::finalizarNucleo();
	PRINTF_INFO("Exito Finalizando nucleo.\n");*/
}

//-----------------------
// --
//-----------------------
- (void)viewDidLoad{
    [super viewDidLoad];
}

-(void)viewDidAppear:(BOOL)animated{
	[super viewDidAppear:animated];
}

- (void)didReceiveMemoryWarning{
	PRINTF_INFO("EAGLViewController, MemoryWarning!\n");
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
	//NBGestorEscena::liberarRecursosCacheRenderEscenas();
	const UI32 conteoLiberados = AUApp::liberarRecursosSinUso();
	PRINTF_INFO("%d recursos liberados (después de MemoryWarning).\n", conteoLiberados);
}

//

//Para iOS 5 y anteriores (deprecated, no funciona en iOS6)
#if __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_6_0
-(BOOL) shouldAutorotateToInterfaceOrientation: (UIInterfaceOrientation)interfaceOrientation {
	BOOL r = YES; //UIInterfaceOrientationIsLandscape(interfaceOrientation);
	return r;
}
#endif

//A partir de iOS 6 en adelante (no funciona en iOS 5 y anteriores)
- (BOOL) shouldAutorotate {
	BOOL r = YES;
	//BOOL r = NO;
    return r;
}

//A partir de iOS 6 en adelante (no funciona en iOS 5 y anteriores)
-(NSUInteger) supportedInterfaceOrientations {
	//PRINTF_INFO("###############################\nEAGLViewController::supportedInterfaceOrientations\n###############################\n");
	//IPAD DEFAULT: UIInterfaceOrientationMaskAll
	//IPHONE DEFAULT: UIInterfaceOrientationMaskAllButUpsideDown
	/*typedef enum {
	 UIInterfaceOrientationMaskPortrait				= (1 << UIInterfaceOrientationPortrait),
	 UIInterfaceOrientationMaskLandscapeLeft		= (1 << UIInterfaceOrientationLandscapeLeft),
	 UIInterfaceOrientationMaskLandscapeRight		= (1 << UIInterfaceOrientationLandscapeRight),
	 UIInterfaceOrientationMaskPortraitUpsideDown	= (1 << UIInterfaceOrientationPortraitUpsideDown),
	 UIInterfaceOrientationMaskLandscape			= (UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight),
	 UIInterfaceOrientationMaskAll					= (UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight | UIInterfaceOrientationMaskPortraitUpsideDown),
	 UIInterfaceOrientationMaskAllButUpsideDown		= (UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight),
	 } UIInterfaceOrientationMask;*/
	return UIInterfaceOrientationMaskAll; //(UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight); //UIInterfaceOrientationMaskAll /*UIInterfaceOrientationMaskLandscape*/;
}

- (UIInterfaceOrientation)preferredInterfaceOrientationForPresentation {
	//PRINTF_INFO("###############################\nEAGLViewController::preferredInterfaceOrientationForPresentation\n###############################\n");
	//IPAD DEFAULT: UIInterfaceOrientationMaskAll
	//IPHONE DEFAULT: UIInterfaceOrientationMaskAllButUpsideDown
	/*typedef enum {
	 UIInterfaceOrientationMaskPortrait				= (1 << UIInterfaceOrientationPortrait),
	 UIInterfaceOrientationMaskLandscapeLeft		= (1 << UIInterfaceOrientationLandscapeLeft),
	 UIInterfaceOrientationMaskLandscapeRight		= (1 << UIInterfaceOrientationLandscapeRight),
	 UIInterfaceOrientationMaskPortraitUpsideDown	= (1 << UIInterfaceOrientationPortraitUpsideDown),
	 UIInterfaceOrientationMaskLandscape			= (UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight),
	 UIInterfaceOrientationMaskAll					= (UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight | UIInterfaceOrientationMaskPortraitUpsideDown),
	 UIInterfaceOrientationMaskAllButUpsideDown		= (UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight),
	 } UIInterfaceOrientationMask;*/
	return UIInterfaceOrientationPortrait; //UIInterfaceOrientationLandscapeRight; //UIInterfaceOrientationLandscapeRight;
}

-(void)willRotateToInterfaceOrientation: (UIInterfaceOrientation)orientation duration:(NSTimeInterval)duration {
	//Status bar
	/*if(UIInterfaceOrientationIsLandscape(orientation)){
		[[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone];
	} else {
		[[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationNone];
	}*/
	//
	[super willRotateToInterfaceOrientation: orientation duration: duration];
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
	//Status bar
	/*if(size.height <= 640){
		[[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone];
	} else {
		[[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationNone];
	}*/
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

-(void) volcarRenderBufferHaciaPantalla {
	[_vistaOpenGL volcarRenderBufferHaciaPantalla];
}

- (void) activarIdiomaPreferido {
	//Detectar idiomas preferidos
	NSArray* idiomasPrefereidos = [NSLocale preferredLanguages];
	SI32 iIdioma, iPrioridad = 0;
	for(iIdioma=0; iIdioma<[idiomasPrefereidos count]; iIdioma++){
		NSString* idioma = [[NSLocale preferredLanguages] objectAtIndex:iIdioma];
		const char* strIdioma = [idioma UTF8String];
		PRINTF_INFO("IDIOMA PREFERIDO #%d: '%s'", iIdioma+1, strIdioma);
		if(AUCadena8::esIgual(strIdioma, "es") || AUCadena8::esIgual(strIdioma, "ES")){
			NBGestorIdioma::establecerPrioridadIdioma(iPrioridad++, ENIdioma_ES); PRINTF_INFO(" (prioridad #%d)", iPrioridad);
		} else if(AUCadena8::esIgual(strIdioma, "en") || AUCadena8::esIgual(strIdioma, "EN")){
			NBGestorIdioma::establecerPrioridadIdioma(iPrioridad++, ENIdioma_EN); PRINTF_INFO(" (prioridad #%d)", iPrioridad);
		} else if(AUCadena8::esIgual(strIdioma, "fr") || AUCadena8::esIgual(strIdioma, "FR")){
			NBGestorIdioma::establecerPrioridadIdioma(iPrioridad++, ENIdioma_FR); PRINTF_INFO(" (prioridad #%d)", iPrioridad);
		} else if(AUCadena8::esIgual(strIdioma, "de") || AUCadena8::esIgual(strIdioma, "DE")){
			NBGestorIdioma::establecerPrioridadIdioma(iPrioridad++, ENIdioma_DE); PRINTF_INFO(" (prioridad #%d)", iPrioridad);
		} else if(AUCadena8::esIgual(strIdioma, "it") || AUCadena8::esIgual(strIdioma, "IT")){
			NBGestorIdioma::establecerPrioridadIdioma(iPrioridad++, ENIdioma_IT); PRINTF_INFO(" (prioridad #%d)", iPrioridad);
		}
		PRINTF_INFO("\n");
	}
	//Establecer idioma manual
	AUAlmacenDatos* almacenConfig = NBGestorDatos::almacenDatos("config");
	bool idiomaEsManual = almacenConfig->valor("idiomaEsManual", false);
	if(idiomaEsManual){
		const char* strIdioma = almacenConfig->valor("idioma", "");
		if(AUCadena8::esIgual(strIdioma, "es") || AUCadena8::esIgual(strIdioma, "ES")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_ES); PRINTF_INFO(" Preservando idioma establecido manualmente: 'es'\n");
		} else if(AUCadena8::esIgual(strIdioma, "en") || AUCadena8::esIgual(strIdioma, "EN")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_EN); PRINTF_INFO(" Preservando idioma establecido manualmente: 'en'\n");
		} else if(AUCadena8::esIgual(strIdioma, "fr") || AUCadena8::esIgual(strIdioma, "FR")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_FR); PRINTF_INFO(" Preservando idioma establecido manualmente: 'fr'\n");
		} else if(AUCadena8::esIgual(strIdioma, "de") || AUCadena8::esIgual(strIdioma, "DE")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_DE); PRINTF_INFO(" Preservando idioma establecido manualmente: 'de' (aleman)\n");
		} else if(AUCadena8::esIgual(strIdioma, "it") || AUCadena8::esIgual(strIdioma, "IT")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_IT); PRINTF_INFO(" Preservando idioma establecido manualmente: 'it'\n");
		}
	}
	NBGestorIdioma::establecerPrioridadIdiomasEsManual(idiomaEsManual);
}

- (void) crearTimers {
	if(!_sincronizadorPantalla){
		_sincronizadorPantalla = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(tickPantalla)];
		[_sincronizadorPantalla setFrameInterval:1]; //1=cada refrescamiento de pantalla (este numero es la cantidad de ciclos necesarios para disparar el evento)
		[_sincronizadorPantalla addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	}
	PRINTF_INFO("APP-TIMERS REANUDADOS.\n");
}

- (void) detenerTimers {
	if(_sincronizadorPantalla){
		[_sincronizadorPantalla invalidate]; _sincronizadorPantalla = nil;
	}
	PRINTF_INFO("APP-TIMERS DETENIDOS.\n");
}

-(void) tickPantalla {
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("APP::tickPantalla")
	NBASSERT(_app != NULL)
	if(_app != NULL){
		@autoreleasepool {
			_app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

//----------------------
//-- TECLADO
//----------------------

- (BOOL)canBecomeFirstResponder {
	return (NBGestorTeclas::keyboardShouldBeVisible() ? YES : NO);
}

- (BOOL)canResignFirstResponder {
	return YES;
}

-(bool)tecladoVisible {
	return _tecladoEstaVisible;
}

-(bool)tecladoMostrar {
	_tecladoPermitidoMostrar = true;
	return [self becomeFirstResponder];
}

-(bool)tecladoOcultar{
	_tecladoPermitidoMostrar = false;
	return [self resignFirstResponder];
}

- (void)keyboardWillShow:(NSNotification*)aNotification {
	PRINTF_INFO("Teclado mostrando.\n");
	_tecladoEstaVisible = true;
}

- (void)keyboardDidShow:(NSNotification*)aNotification {
	PRINTF_INFO("Teclado omstrado.\n");
	_tecladoEstaVisible = true;
}

- (void)keyboardWillHide:(NSNotification*)aNotification {
	PRINTF_INFO("Teclado ocultando.\n");
	_tecladoPermitidoMostrar	= false;
	_tecladoEstaVisible			= false;
	NBGestorTeclas::establecerTecladoEnPantallaAlto(0.0f, true);
	NBGestorTeclas::escuchadorRemover();
}

- (void)keyboardDidHide:(NSNotification*)aNotification {
	PRINTF_INFO("Teclado ocultado.\n");
	_tecladoPermitidoMostrar	= false;
	_tecladoEstaVisible			= false;
	NBGestorTeclas::establecerTecladoEnPantallaAlto(0.0f, true);
	NBGestorTeclas::escuchadorRemover();
}

- (void)keyboardWillChangeFrame:(NSNotification*)aNotification {
	PRINTF_INFO("Teclado cambiando area.\n");
}

- (void)keyboardDidChangeFrame:(NSNotification*)aNotification {
	NSDictionary* info = [aNotification userInfo];
	const CGRect rectTecladoBrto= [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
	const CGRect rectTeclado	= [self.view convertRect:rectTecladoBrto fromView:self.view.window];
	const float escalaPantalla	= [[self view] contentScaleFactor];
	const float altoEnPantalla	= (float)rectTeclado.size.height * escalaPantalla;
	PRINTF_INFO("Teclado ha cambiado area: orig(%f, %f) tam(%f, %f) escala(%f).\n", rectTeclado.origin.x, rectTeclado.origin.y, rectTeclado.size.width, rectTeclado.size.height, escalaPantalla);
	NBGestorTeclas::establecerTecladoEnPantallaAlto(altoEnPantalla, true);
}

//<UIKeyInput>

- (void)deleteBackward {
	//PRINTF_INFO("<<<UIKeyInput>>> deleteBackward.\n");
	NBGestorTeclas::entradaLockForBatch();
	{
		NBGestorTeclas::entradaBackspace(true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (BOOL)hasText {
	BOOL r = FALSE;
	NBGestorTeclas::entradaLockForBatch();
	{
		r = (NBGestorTeclas::entradaTieneTexto() ? YES : NO);
		//PRINTF_INFO("<<<UIKeyInput>>> hasText('%s').\n", (r ? "YES" : "NO"));
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (void)insertText:(NSString *)text {
	NBGestorTeclas::entradaLockForBatch();
	{
		//PRINTF_INFO("<<<UIKeyInput>>> insertText('%s').\n", [text UTF8String]);
		NBGestorTeclas::entradaIntroducirTexto([text UTF8String], true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

//<UITextInput>

//-------------
//<UITextInput>: Replacing and Returning Text
//-------------
- (NSString *)textInRange:(UITextRange *)range {
	NSString* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 ini = ((AUTextPosition*)range.start).pos;
		const SI32 fin = ((AUTextPosition*)range.end).pos;
		NBASSERT(ini <= fin)
		{
			if(range.empty){
				r = @"";
			} else {
				AUCadenaMutable8* strContenido = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				NBGestorTeclas::entradaTextoEnCharDefsContenido(ini, (fin - ini) + 1, strContenido);
				r = [NSString stringWithUTF8String:strContenido->str()];
				strContenido->liberar(NB_RETENEDOR_NULL);
			}
		}
		if(r == nil){
			//PRINTF_INFO("<<<UITextInput>>> textInRange(%d, %d%s) <== nil.\n", ini, fin, (range.empty? ", empty":""));
		} else {
			//PRINTF_INFO("<<<UITextInput>>> textInRange(%d, %d%s) <== '%s'.\n", ini, fin, (range.empty? ", empty":""), [r UTF8String]);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (void)replaceRange:(UITextRange *)range withText:(NSString *)text {
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 ini = ((AUTextPosition*)range.start).pos;
		const SI32 fin = ((AUTextPosition*)range.end).pos;
		{
			if(range.empty){
				//PRINTF_INFO("<<<UITextInput>>> replaceRange(%d, %d%s, '%s').\n", ini, fin, (range.empty? ", empty":""), [text UTF8String]);
			} else {
				AUCadenaMutable8* strContenido = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				NBGestorTeclas::entradaTextoEnCharDefsContenido(ini, (fin - ini) + 1, strContenido);
				//PRINTF_INFO("<<<UITextInput>>> replaceRange(%d, %d%s, '%s' ==> '%s').\n", ini, fin, (range.empty? ", empty":""), strContenido->str(), [text UTF8String]);
				strContenido->liberar(NB_RETENEDOR_NULL);
			}
		}
		NBASSERT(ini <= fin)
		{
			if(range.empty){
				NBGestorTeclas::entradaTextoEnCharDefsReemplazar(ini, 0, [text UTF8String], ENTextRangeSet_Word);
			} else {
				NBGestorTeclas::entradaTextoEnCharDefsReemplazar(ini, (fin - ini) + 1, [text UTF8String], ENTextRangeSet_Word);
			}
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (BOOL)shouldChangeTextInRange:(UITextRange *)range replacementText:(NSString *)text {
	const SI32 ini = ((AUTextPosition*)range.start).pos;
	const SI32 fin = ((AUTextPosition*)range.end).pos;
	//PRINTF_INFO("<<<UITextInput>>> shouldChangeTextInRange(%d, %d%s, '%s') <== YES.\n", ini, fin, (range.empty? ", empty":""), [text UTF8String]);
	NBASSERT(ini <= fin)
	return YES;
}

//-------------
//<UITextInput>: Working with Marked and Selected Text
//-------------
//@property(readwrite, copy) UITextRange *selectedTextRange;
-(UITextRange*) selectedTextRange {
	AUTextRange* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const NBRangoI rSel = NBGestorTeclas::entradaRangoSeleccionAbs();
		r = [[AUTextRange alloc] initWithRange:rSel];
		[r autorelease];
		//PRINTF_INFO("<<<UITextInput>>> selectedTextRange <= (%d, %d%s).\n", rSel.inicio, (rSel.inicio + rSel.tamano - 1), (r.empty? ", empty":""));
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setSelectedTextRange:(UITextRange*) range {
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 ini = ((AUTextPosition*)range.start).pos;
		const SI32 fin = ((AUTextPosition*)range.end).pos;
		//PRINTF_INFO("<<<UITextInput>>>+ setSelectedTextRange(%d, %d%s).\n", ini, fin, (range.empty? ", empty":""));
		NBASSERT(ini <= fin)
		{
			if(range.empty){
				NBGestorTeclas::entradaRangoSeleccionEstablecer(ini, 0);
			} else {
				NBGestorTeclas::entradaRangoSeleccionEstablecer(ini, (fin - ini) + 1);
			}
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

//@property(nonatomic, readonly) UITextRange *markedTextRange;
-(UITextRange*) markedTextRange {
	AUTextRange* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			const NBRangoI rSel = NBGestorTeclas::entradaRangoMarcadoAbs();
			if(rSel.tamano > 0){
				r = [[AUTextRange alloc] initWithRange:rSel];
				[r autorelease];
			}
		}
		if(r == nil){
			//PRINTF_INFO("<<<UITextInput>>> selectedTextRange <= nil.\n");
		} else {
			//PRINTF_INFO("<<<UITextInput>>> markedTextRange <= (%d, %d%s).\n", ((AUTextPosition*)[r start]).pos, ((AUTextPosition*)[r end]).pos, ([r isEmpty]? ", empty":""));
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (void)setMarkedText:(NSString *)markedText selectedRange:(NSRange)selectedRange {
	//PRINTF_INFO("<<<UITextInput>>>+ setMarkedText(%d, +%d ==> %s).\n", selectedRange.location, selectedRange.length, [markedText UTF8String]);
}

- (void)unmarkText {
	NBGestorTeclas::entradaLockForBatch();
	{
		//PRINTF_INFO("<<<UITextInput>>> unmarkText.\n");
		//NBGestorTeclas::entradaRangoDesmarcar();
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

//@property(nonatomic, copy) NSDictionary *markedTextStyle;
//NSString *const UITextInputTextBackgroundColorKey (UIColor); //Deprecated in iOS 8.0.
//NSString *const UITextInputTextColorKey (UIColor); //Deprecated in iOS 8.0.
//NSString *const UITextInputTextFontKey (UIFont); //Deprecated in iOS 8.0.
//@property(nonatomic) UITextStorageDirection selectionAffinity;
/*
 In the default implementation,
 if the selection is not at the end of the line, or if the selection is at the start of a paragraph for an empty line,
 a forward direction is assumed (UITextStorageDirectionForward);
 otherwise,
 a backward direction UITextStorageDirectionBackward is assumed.
 */

//-------------
//<UITextInput>: Computing Text Ranges and Text Positions
//-------------
- (UITextRange *)textRangeFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition {
	AUTextRange* r = [[AUTextRange alloc] initWithAUTextPositions:(AUTextPosition*)fromPosition posEnd:(AUTextPosition*)toPosition];
	[r autorelease];
	//PRINTF_INFO("<<<UITextInput>>> textRangeFromPosition(%d, %d) <== AUTextRange.\n", ((AUTextPosition*)fromPosition).pos, ((AUTextPosition*)toPosition).pos);
	return r;
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)position offset:(NSInteger)offset {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			const SI32 iPrimer = NBGestorTeclas::entradaIndiceCharDefPrimero();
			const SI32 iUltimo = NBGestorTeclas::entradaIndiceCharDefUltimo();
			const SI32 posResultado = (((AUTextPosition*)position).pos + offset);
			if(posResultado >= iPrimer && posResultado <= iUltimo){ //posResultado <= (iUltimo + 1)
				r = [[AUTextPosition alloc] initWithPos:posResultado];
				[r autorelease];
			}
		}
		if(r == nil){
			//PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d) <== nil.\n", ((AUTextPosition*)position).pos, (offset >=0 ? "+" : ""), offset);
		} else {
			//PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d) <== (%d).\n", ((AUTextPosition*)position).pos, (offset >=0 ? "+" : ""), offset, r.pos);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction offset:(NSInteger)offset {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			const SI32 iPrimer = NBGestorTeclas::entradaIndiceCharDefPrimero();
			const SI32 iUltimo = NBGestorTeclas::entradaIndiceCharDefUltimo();
			SI32 posResultado = 0;
			switch(direction){
				case UITextLayoutDirectionRight:
					posResultado = (((AUTextPosition*)position).pos + offset);
					break;
				case UITextLayoutDirectionLeft:
					posResultado = (((AUTextPosition*)position).pos - offset);
					break;
				case UITextLayoutDirectionUp:
					NBASSERT(false)
					break;
				case UITextLayoutDirectionDown:
					NBASSERT(false)
					break;
				default:
					NBASSERT(false)
					break;
			}
			if(posResultado >= iPrimer && posResultado <= iUltimo){ //posResultado <= (iUltimo + 1)
				r = [[AUTextPosition alloc] initWithPos:posResultado];
				[r autorelease];
			}
		}
		if(r == nil){
			//PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d, %s) <== nil .\n", ((AUTextPosition*)position).pos, (offset >=0 ? "+" : ""), offset, (direction == UITextLayoutDirectionRight ? "toRight" : direction == UITextLayoutDirectionLeft ? "toLeft" : direction == UITextLayoutDirectionUp ? "toUp" : direction == UITextLayoutDirectionDown ? "toDown" : "to????"));
		} else {
			//PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d, %s) <== (%d) .\n", ((AUTextPosition*)position).pos, (offset >=0 ? "+" : ""), offset, (direction == UITextLayoutDirectionRight ? "toRight" : direction == UITextLayoutDirectionLeft ? "toLeft" : direction == UITextLayoutDirectionUp ? "toUp" : direction == UITextLayoutDirectionDown ? "toDown" : "to????"), r.pos);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

//@property(nonatomic, readonly) UITextPosition *beginningOfDocument;
-(UITextPosition*) beginningOfDocument {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const UI32 rPos = NBGestorTeclas::entradaIndiceCharDefPrimero();
		r = [[AUTextPosition alloc] initWithPos:rPos];
		[r autorelease];
		//PRINTF_INFO("<<<UITextInput>>> beginningOfDocument <== (%d).\n", rPos);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

//@property(nonatomic, readonly) UITextPosition *endOfDocument;
-(UITextPosition*) endOfDocument {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const UI32 rPos = NBGestorTeclas::entradaIndiceCharDefUltimo();
		r = [[AUTextPosition alloc] initWithPos:rPos];
		[r autorelease];
		//PRINTF_INFO("<<<UITextInput>>> endOfDocument <== (%d).\n", rPos);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

//-------------
//<UITextInput>: Evaluating Text Positions
//-------------
- (NSComparisonResult)comparePosition:(UITextPosition *)position toPosition:(UITextPosition *)other {
	NSComparisonResult r = NSOrderedSame;
	const SI32 ini = ((AUTextPosition*)position).pos;
	const SI32 fin = ((AUTextPosition*)other).pos;
	if(ini < fin){
		r = NSOrderedAscending;
	} else if(ini > fin){
		r = NSOrderedDescending;
	}
	//PRINTF_INFO("<<<UITextInput>>> comparePosition(%d, %d) <== %s.\n", ini, fin, (r == NSOrderedAscending ? "Ascending" : r == NSOrderedDescending ? "Descending" : r == NSOrderedSame ? "Same" : "????"));
	return r;
}

- (NSInteger)offsetFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition {
	const SI32 ini = ((AUTextPosition*)fromPosition).pos;
	const SI32 fin = ((AUTextPosition*)toPosition).pos;
	//PRINTF_INFO("<<<UITextInput>>> offsetFromPosition(%d, %d) <== %d.\n", ini, fin, (fin - ini));
	return (fin - ini);
}

//-------------
//<UITextInput>: Determining Layout and Writing Direction
//-------------
/*- (UITextPosition *)positionWithinRange:(UITextRange *)range farthestInDirection:(UITextLayoutDirection)direction {
	
}

- (UITextRange *)characterRangeByExtendingPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction {
	
}
*/
- (UITextWritingDirection)baseWritingDirectionForPosition:(UITextPosition *)position inDirection:(UITextStorageDirection)direction {
	//UITextStorageDirectionForward = 0
	//UITextStorageDirectionBackward
	//
	//UITextWritingDirectionNatural = -1,
	//UITextWritingDirectionLeftToRight = 0,
	//UITextWritingDirectionRightToLeft,
	//PRINTF_INFO("<<<UITextInput>>> baseWritingDirectionForPosition(%d, %s) <== %s.\n", ((AUTextPosition *)position).pos, (direction == UITextStorageDirectionForward ? "Forward" : direction == UITextStorageDirectionBackward ? "Backward" : "???"), (_inputBaseDirection == UITextWritingDirectionNatural ? "Natural" : _inputBaseDirection == UITextWritingDirectionLeftToRight ? "LeftToRight" : _inputBaseDirection == UITextWritingDirectionRightToLeft ? "RightToLeft" : "???"));
	return _inputBaseDirection;
}

- (void)setBaseWritingDirection:(UITextWritingDirection)writingDirection forRange:(UITextRange *)range {
	//PRINTF_INFO("<<<UITextInput>>> setBaseWritingDirection(%s, %d, %d).\n", (writingDirection == UITextWritingDirectionNatural ? "Natural" : writingDirection == UITextWritingDirectionLeftToRight ? "LeftToRight" : writingDirection == UITextWritingDirectionRightToLeft ? "RightToLeft" : "???"), ((AUTextPosition*)range.start).pos, ((AUTextPosition*)range.end).pos);
	_inputBaseDirection = writingDirection;
}

//-------------
//<UITextInput>: Geometry and Hit-Testing Methods
//-------------
- (CGRect)firstRectForRange:(UITextRange *)range {
	CGRect r; r.origin.x = 0; r.origin.y = 0; r.size.width = 0; r.size.height = 0;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			AUTextRange* rango = (AUTextRange*)range;
			if(![rango isEmpty]){
				CAEAGLLayer *eaglLayer			= (CAEAGLLayer *)[_vistaOpenGL layer];
				CGFloat glContentScale			= eaglLayer.contentsScale;
				//
				const SI32 ini = [rango start].pos;
				const SI32 fin = [rango end].pos;
				NBASSERT(ini <= fin)
				const NBRectangulo rect = NBGestorTeclas::entradaPrimerRectanguloParaCharDefs(ini, (fin - ini + 1));
				r.origin.x = (float)rect.x / glContentScale;
				r.origin.y = (float)rect.y / glContentScale;
				r.size.width = (float)rect.ancho / glContentScale;
				r.size.height = (float)rect.alto / glContentScale;
			}
		}
		if(r.size.width == 0 && r.size.height == 0){
			//PRINTF_INFO("<<<UITextInput>>> firstRectForRange(%d, %d) <== empty.\n", ((AUTextPosition*)range.start).pos, ((AUTextPosition*)range.end).pos);
		} else {
			//PRINTF_INFO("<<<UITextInput>>> firstRectForRange(%d, %d) <== (%f, %f)-(+%f, +%f).\n", ((AUTextPosition*)range.start).pos, ((AUTextPosition*)range.end).pos, r.origin.x, r.origin.y, r.size.width, r.size.height);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (CGRect)caretRectForPosition:(UITextPosition *)position {
	CGRect r; r.origin.x = 0; r.origin.y = 0; r.size.width = 0; r.size.height = 0;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			CAEAGLLayer *eaglLayer			= (CAEAGLLayer *)[_vistaOpenGL layer];
			CGFloat glContentScale			= eaglLayer.contentsScale;
			//
			const SI32 pos = ((AUTextPosition*) position).pos;
			const NBRectangulo rect = NBGestorTeclas::entradaRectanguloParaCursor(pos);
			r.origin.x = (float)rect.x / glContentScale;
			r.origin.y = (float)rect.y / glContentScale;
			r.size.width = (float)rect.ancho / glContentScale;
			r.size.height = (float)rect.alto / glContentScale;
		}
		if(r.size.width == 0 && r.size.height == 0){
			//PRINTF_INFO("<<<UITextInput>>> caretRectForPosition(%d) <== empty.\n", ((AUTextPosition*)position).pos);
		} else {
			//PRINTF_INFO("<<<UITextInput>>> caretRectForPosition(%d) <== (%f, %f)-(+%f, +%f).\n", ((AUTextPosition*)position).pos, r.origin.x, r.origin.y, r.size.width, r.size.height);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

/*- (UITextPosition *)closestPositionToPoint:(CGPoint)point {
	//
}

- (NSArray *)selectionRectsForRange:(UITextRange *)range {
	//An array of UITextSelectionRect objects that encompass the selection.
}

- (UITextPosition *)closestPositionToPoint:(CGPoint)point withinRange:(UITextRange *)range {
	
}

- (UITextRange *)characterRangeAtPoint:(CGPoint)point {
	
}*/

//-------------
//<UITextInput>: Text Input Delegate and Text Input Tokenizer
//-------------
//@property(nonatomic, assign) id< UITextInputDelegate > inputDelegate;
- (id< UITextInputDelegate >) inputDelegate {
	//PRINTF_INFO("<<<UITextInput>>> inputDelegate.\n");
	return _inputDelegate;
}

- (void) setInputDelegate: (id< UITextInputDelegate >) delegate {
	//PRINTF_INFO("<<<UITextInput>>> setInputDelegate.\n");
	_inputDelegate = delegate;
}
//@property(nonatomic, readonly) id< UITextInputTokenizer > tokenizer;
- (id< UITextInputTokenizer >) tokenizer {
	UITextInputStringTokenizer* r = [[UITextInputStringTokenizer alloc] initWithTextInput:self];
	[r autorelease];
	//PRINTF_INFO("<<<UITextInput>>> tokenizer.\n");
	return r;
}

//-------------
//<UITextInput>: Returning the Text Input View
//-------------
//@property(nonatomic, readonly) UIView *textInputView
- (UIView*)textInputView {
	//PRINTF_INFO("<<<UITextInput>>> textInputView.\n");
	return _vistaOpenGL;
}

//----------------------
//-- TOUCHES
//----------------------
- (const char*) nombreFaseTouch: (UITouchPhase) fase {
	if(fase==UITouchPhaseBegan) return "Began";
	if(fase==UITouchPhaseMoved) return "Moved";
	if(fase==UITouchPhaseStationary) return "Stationary";
	if(fase==UITouchPhaseEnded) return "Ended";
	if(fase==UITouchPhaseCancelled) return "Cancelled";
	return "Desconcodido";
}

-(void) touchesBegan: (NSSet *) touches withEvent: (UIEvent *) event {
	//PRINTF_INFO("touchesBegan\n");
	if(_app != NULL){
		int iTouch = 0;
		CGFloat escalaGL = [[self view] contentScaleFactor];
		NSArray* arrTouches = [touches allObjects];
		for(iTouch=0; iTouch<[arrTouches count]; iTouch++){
			UITouch *touch		= [arrTouches objectAtIndex:iTouch];
			CGPoint posicion	= [touch locationInView: [self view]];
			//UITouchPhase fase	= [touch phase];
			NSUInteger hash		= [touch hash];
			//PRINTF_INFO("TOUCH INI (%d) %s \n", hash, (fase!=UITouchPhaseBegan)?[self nombreFaseTouch: fase]:"");
			_app->touchIniciado((UI32)hash, posicion.x * escalaGL, posicion.y * escalaGL);
		}
	}
}

-(void) touchesMoved: (NSSet *) touches withEvent: (UIEvent *) event {
	//PRINTF_INFO("touchesMoved\n");
	if(_app != NULL){
		int iTouch = 0;
		CGFloat escalaGL = [[self view] contentScaleFactor];
		NSArray* arrTouches = [touches allObjects];
		for(iTouch=0; iTouch<[arrTouches count]; iTouch++){
			UITouch *touch		= [arrTouches objectAtIndex:iTouch];
			CGPoint posicion	= [touch locationInView: [self view]];
			//UITouchPhase fase	= [touch phase];
			NSUInteger hash		= [touch hash];
			//PRINTF_INFO("TOUCH MOV (%d) %s\n", hash, (fase!=UITouchPhaseMoved)?[self nombreFaseTouch: fase]:"");
			_app->touchMovido((UI32)hash, posicion.x * escalaGL, posicion.y * escalaGL);
		}
	}
}

-(void) touchesEnded:(NSSet *) touches withEvent: (UIEvent *) event {
	//PRINTF_INFO("touchesEnded\n");
	if(_app != NULL){
		int iTouch = 0;
		CGFloat escalaGL = [[self view] contentScaleFactor];
		NSArray* arrTouches = [touches allObjects];
		for(iTouch=0; iTouch<[arrTouches count]; iTouch++){
			UITouch *touch		= [arrTouches objectAtIndex:iTouch];
			CGPoint posicion	= [touch locationInView: [self view]];
			//UITouchPhase fase	= [touch phase];
			NSUInteger hash		= [touch hash];
			//PRINTF_INFO("TOUCH FIN (%d) %s\n", hash, (fase!=UITouchPhaseEnded)?[self nombreFaseTouch: fase]:"");
			_app->touchFinalizado((UI32)hash, posicion.x * escalaGL, posicion.y * escalaGL, false);
		}
	}
}

-(void) touchesCancelled: (NSSet *) touches withEvent: (UIEvent *) event {
	//PRINTF_INFO("touchesCancelled\n");
	if(_app != NULL){
		int iTouch = 0;
		CGFloat escalaGL = [[self view] contentScaleFactor];
		NSArray* arrTouches = [touches allObjects];
		for(iTouch=0; iTouch<[arrTouches count]; iTouch++){
			UITouch *touch		= [arrTouches objectAtIndex:iTouch];
			CGPoint posicion	= [touch locationInView: [self view]];
			//UITouchPhase fase	= [touch phase];
			NSUInteger hash		= [touch hash];
			//PRINTF_INFO("TOUCH CANCEL (%d) %s\n", hash, (fase!=UITouchPhaseCancelled)?[self nombreFaseTouch: fase]:"");
			_app->touchFinalizado((UI32)hash, posicion.x * escalaGL, posicion.y * escalaGL, true);
		}
	}
}

@end

//Metodos C
void volcarBuffer(void* param, AUApp* juego, SI32 iEscenaRender){
	if(param) [((__bridge EAGLViewController*)param) volcarRenderBufferHaciaPantalla];
}

//Teclado
bool tecladoVisible(void* param){
	if(param) return [((__bridge EAGLViewController*)param) tecladoVisible];
	return false;
}

bool tecladoMostrar(void* param){
	if(param) return [((__bridge EAGLViewController*)param) tecladoMostrar];
	return false;
}

bool tecladoOcultar(void* param){
	if(param) return [((__bridge EAGLViewController*)param) tecladoOcultar];
	return false;
}

//----------------------------
//-- Procesamiento de texto --
//----------------------------
@implementation AUTextPosition
-(id) initWithPos:(SI32) pos {
	_pos = pos;
	return self;
}
-(SI32) pos {
	return _pos;
}
@end

@implementation AUTextRange
-(id) initWithRange:(NBRangoI)rango {
	_empty	= (rango.tamano <= 0 ? YES : NO);
	_start	= [[AUTextPosition alloc] initWithPos:rango.inicio];
	_end	= [[AUTextPosition alloc] initWithPos:(_empty ? rango.inicio : (rango.inicio + rango.tamano - 1))];
	return self;
}
-(id) initWithAUTextPositions:(AUTextPosition*)posStart posEnd:(AUTextPosition*)posEnd {
	_empty	= (posStart.pos > posEnd.pos ? YES : NO);
	_start	= [[AUTextPosition alloc] initWithPos:posStart.pos];
	_end	= [[AUTextPosition alloc] initWithPos:(_empty ? posStart.pos : posEnd.pos)];
	return self;
}
-(void) dealloc {
	if(_start != nil) [_start release]; _start = nil;
	if(_end != nil) [_end release]; _end = nil;
	[super dealloc];
}
-(AUTextPosition*) start {
	return _start;
}
-(AUTextPosition*) end {
	return _end;
}
-(BOOL) isEmpty {
	return _empty;
}
@end




