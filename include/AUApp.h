//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUApp_h
#define AUApp_h

#include "AUAppNucleoEncabezado.h"
#include "AUAppEscenasResumenDebug.h"
#include "AUAppEscenasAdmin.h"
#include "NBGestorEscenaDef.h"
#include "AUAppI.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

//OS fileSystem glue
#if defined(_WIN32)
#	include "AUAppGlueWinTools.h"
#	include "AUAppGlueWinFiles.h"
#	include "AUAppGlueWinSecure.h"
#	include "AUAppGluePdfium.h"
#elif defined(__ANDROID__)
#	include "AUAppGlueAndroidJNI.h"
#	include "AUAppGlueAndroidTools.h"
#	include "AUAppGlueAndroidSecure.h"
#	include "AUAppGlueAndroidFiles.h"
#	include "AUAppGlueAndroidNotifs.h"
#	include "AUAppGlueAndroidAVCapture.h"
#	include "AUAppGlueAndroidContacts.h"
#	include "AUAppGlueAndroidBiometrics.h"
#	include "AUAppGlueAndroidStore.h"
#	include "AUAppGlueAndroidTelephony.h"
#	include "AUAppGlueAndroidKeyboard.h"
#	include "AUAppGluePdfium.h"
#elif defined(__QNX__) //BB10
#	include "AUAppGlueQnxFiles.h"
#elif defined(__APPLE__)
#	ifndef __TARGETCONDITIONALS__
#	error missing "TargetConditionals.h"
#	endif
#	if TARGET_OS_IPHONE
#		include "AUAppGlueIOSTools.h"
#		include "AUAppGlueAppleSecure.h"
#		include "AUAppGlueIOSFiles.h"
#		include "AUAppGlueIOSNotifs.h"
#		include "AUAppGlueIOSAVCapture.h"
#		include "AUAppGlueIOSStore.h"
#		include "AUAppGlueIOSTelephony.h"
#		include "AUAppGlueAppleContacts.h"
#		include "AUAppGlueIOSBiometrics.h"
#		include "AUAppGlueIOSKeyboard.h"
#		include "AUAppGlueIOSGameKit.h"
#		include "AUAppGlueApplePdfKit.h"
#	else
#		include "AUAppGlueOsxTools.h"
#		include "AUAppGlueOsxFiles.h"
#		include "AUAppGlueAppleSecure.h"	//OSX and iOS share the same keychain API
#		include "AUAppGlueAppleContacts.h"
#		include "AUAppGlueApplePdfKit.h"
#		include "AUAppGluePdfium.h"			//Tmp
#	endif
#endif

//#define APP_IMPLEMENTAR_BI_HILO	//hilo productor de escena + hilo de programa (consume escena, produce renderBuffer y consume renderBuffer)
//#define APP_IMPLEMENTAR_TRI_HILO	//hilo productor de escena + hilo consumidor escena (produce renderBuffer) + hilo de programa (consume renderBuffer)

#define DEBUG_RESUMEN_CREAR						false
#define DEBUG_RESUMEN_MOSTRAR_DATOS_TEXTO		true
#define DEBUG_RESUMEN_MOSTRAR_DATOS_GRAFICOS	false
#define DEBUG_RESUMEN_IMPRIMIR_EN_CONSOLA		false

#define AUAPP_BIT_MODULO_RED	1

//Para casos TRI o BI hilo
#if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	#ifndef CONFIG_NB_GESTION_MEMORIA_IMPLEMENTAR_MUTEX
	#warning "ADVERTENCIA DE CONFIGURACION: la APP utilizara multiples hilos PERO la Gestion de Memoria no es multihio."
	#endif
	#define APPHILO_CLASE							NBHILO_CLASE
	#define APPHILO_MUTEX_CLASE						NBHILO_MUTEX_CLASE
	#define APPHILO_MUTEX_INICIALIZAR(MUTEX)		NBHILO_MUTEX_INICIALIZAR(MUTEX)
	#define APPHILO_MUTEX_FINALIZAR(MUTEX)			NBHILO_MUTEX_FINALIZAR(MUTEX)
	#define APPHILO_COND_CLASE						NBHILO_COND_CLASE
	#define APPHILO_COND_INICIALIZAR(COND)			NBHILO_COND_INICIALIZAR(COND)
	#define APPHILO_COND_FINALIZAR(COND)			NBHILO_COND_FINALIZAR(COND)
	#define APPHILO_ACTIVA_MUTEX(mutex)				NBHILO_MUTEX_ACTIVAR(mutex);
	#define APPHILO_DESACTIVA_MUTEX(mutex)			NBHILO_MUTEX_DESACTIVAR(mutex);
	#define APPHILO_ESPERA_CONDICIONAL(cond, mutex)	NBHILO_COND_ESPERAR(cond, mutex);
	#define APPHILO_DISPARA_CONDICIONAL(cond)		NBHILO_COND_DISPARAR(cond);
#else
	#define APPHILO_ACTIVA_MUTEX(mutex)
	#define APPHILO_DESACTIVA_MUTEX(mutex)
	#define APPHILO_ESPERA_CONDICIONAL(cond, mutex)
	#define APPHILO_DISPARA_CONDICIONAL(cond)
#endif

//------------------------
// LINKING TO INTERNAL LIBRARIES
//Note: these linking methods are defined as MACROS instead of FUNCTIONS
//to give the linker better chance to strip the unnecesary code from
//the final result (and avoid libraries dependencies when linking).
//------------------------
#define AUApp_linkToInternalLibJpegRead()			NBGlueLibJpegRead_linkToMethods()		//Enable libjpeg read-methods
#define AUApp_linkToInternalLibJpegWrite()			NBGlueLibJpegWrite_linkToMethods()		//Enable libjpeg write-methods
//
#define AUApp_linkToInternalLibsAll()				AUApp_linkToInternalLibJpegRead; \
													AUApp_linkToInternalLibJpegWrite; \
													(0)

//------------------------
// LINKING TO EXTERNAL LIBRARIES
//Note: these linking methods are defined as MACROS instead of FUNCTIONS
//to give the linker better chance to strip the unnecesary code from
//the final result (and avoid libraries dependencies when linking).
//------------------------
#if defined(_WIN32)
//	WINDOWS
#	define AUApp_linkToDefaultOSTools(APP)			NBMngrOSTools::setGlue(APP, AUAppGlueWinTools::create)
#	define AUApp_linkToDefaultOSSecure(APP)			NBMngrOSSecure::setGlue(APP, AUAppGlueWinSecure::create)
#	define AUApp_linkToDefaultPkgFilesystem(APP)	NBGestorArchivos::setGlue(APP, AUAppGlueWinFiles::create)
#	define AUApp_linkToDefaultKeyboard(APP)			(0)
#	define AUApp_linkToDefaultNotifSystem(APP)		(0)
#	define AUApp_linkToDefaultAVCapture(APP)		(0)
#	define AUApp_linkToDefaultStore(APP)			(0)
#	define AUApp_linkToDefaultTelephony(APP)		(0)
#	define AUApp_linkToDefaultContacts(APP)			(0)
#	define AUApp_linkToDefaultBiometrics(APP)		(0)
#	define AUApp_linkToDefaultGameKit(APP)			(0)
#	define AUApp_linkToDefaultPdfKit(APP)			NBMngrPdfKit::setGlue(APP, AUAppGluePdfium::create)
#elif defined(__ANDROID__)
//	ANDROID
#	define AUApp_linkToDefaultOSTools(APP)			NBMngrOSTools::setGlue(APP, AUAppGlueAndroidTools::create)
#	define AUApp_linkToDefaultOSSecure(APP)			NBMngrOSSecure::setGlue(APP, AUAppGlueAndroidSecure::create)
#	define AUApp_linkToDefaultPkgFilesystem(APP)	NBGestorArchivos::setGlue(APP, AUAppGlueAndroidFiles::create)
#	define AUApp_linkToDefaultKeyboard(APP)			NBGestorTeclas::setGlue(APP, AUAppGlueAndroidKeyboard::create)
#	define AUApp_linkToDefaultNotifSystem(APP)		NBMngrNotifs::setGlue(APP, AUAppGlueAndroidNotifs::create)
#	define AUApp_linkToDefaultAVCapture(APP)		NBMngrAVCapture::setGlue(APP, AUAppGlueAndroidAVCapture::create)
#	define AUApp_linkToDefaultStore(APP)			NBMngrStore::setGlue(APP, AUAppGlueAndroidStore::create)
#	define AUApp_linkToDefaultTelephony(APP)		NBMngrOSTelephony::setGlue(APP, AUAppGlueAndroidTelephony::create)
#	define AUApp_linkToDefaultContacts(APP)			NBMngrContacts::setGlue(APP, AUAppGlueAndroidContacts::create)
#	define AUApp_linkToDefaultBiometrics(APP)		//NBMngrBiometrics::setGlue(APP, AUAppGlueAndroidBiometrics::create) 2023-01-07, disabled until code is updated and fixed
#	define AUApp_linkToDefaultGameKit(APP)			(0)
#	define AUApp_linkToDefaultPdfKit(APP)			NBMngrPdfKit::setGlue(APP, AUAppGluePdfium::create)
#elif defined(__QNX__)
//	BB10
#	define AUApp_linkToDefaultOSTools(APP)			(0)
#	define AUApp_linkToDefaultOSSecure(APP)			(0)
#	define AUApp_linkToDefaultPkgFilesystem(APP)	NBGestorArchivos::setGlue(APP, AUAppGlueQnxFiles::create)
#	define AUApp_linkToDefaultKeyboard(APP)			(0)
#	define AUApp_linkToDefaultNotifSystem(APP)		(0)
#	define AUApp_linkToDefaultAVCapture(APP)		(0)
#	define AUApp_linkToDefaultStore(APP)			(0)
#	define AUApp_linkToDefaultTelephony(APP)		(0)
#	define AUApp_linkToDefaultContacts(APP)			(0)
#	define AUApp_linkToDefaultBiometrics(APP)		(0)
#	define AUApp_linkToDefaultGameKit(APP)			(0)
#	define AUApp_linkToDefaultPdfKit(APP)			(0)
#elif defined(__APPLE__)
#	ifndef __TARGETCONDITIONALS__
#	error missing "TargetConditionals.h"
#	endif
#	if TARGET_OS_IPHONE
//	IOS
#	define AUApp_linkToDefaultOSTools(APP)			NBMngrOSTools::setGlue(APP, AUAppGlueIOSTools::create)
#	define AUApp_linkToDefaultOSSecure(APP)			NBMngrOSSecure::setGlue(APP, AUAppGlueAppleSecure::create)
#	define AUApp_linkToDefaultPkgFilesystem(APP)	NBGestorArchivos::setGlue(APP, AUAppGlueIOSFiles::create)
#	define AUApp_linkToDefaultKeyboard(APP)			NBGestorTeclas::setGlue(APP, AUAppGlueIOSKeyboard::create)
#	define AUApp_linkToDefaultNotifSystem(APP)		NBMngrNotifs::setGlue(APP, AUAppGlueIOSNotifs::create)
#	define AUApp_linkToDefaultAVCapture(APP)		NBMngrAVCapture::setGlue(APP, AUAppGlueIOSAVCapture::create)
#	define AUApp_linkToDefaultStore(APP)			NBMngrStore::setGlue(APP, AUAppGlueIOSStore::create)
#	define AUApp_linkToDefaultTelephony(APP)		NBMngrOSTelephony::setGlue(APP, AUAppGlueIOSTelephony::create)
#	define AUApp_linkToDefaultContacts(APP)			NBMngrContacts::setGlue(APP, AUAppGlueAppleContacts::create)
#	define AUApp_linkToDefaultBiometrics(APP)		NBMngrBiometrics::setGlue(APP, AUAppGlueIOSBiometrics::create)
#	define AUApp_linkToDefaultGameKit(APP)			NBMngrGameKit::setGlue(APP, AUAppGlueIOSGameKit::create)
#	define AUApp_linkToDefaultPdfKit(APP)			NBMngrPdfKit::setGlue(APP, AUAppGlueApplePdfKit::create)
#	else
//	OSX
#	define AUApp_linkToDefaultOSTools(APP)			NBMngrOSTools::setGlue(APP, AUAppGlueOsxTools::create)
#	define AUApp_linkToDefaultOSSecure(APP)			NBMngrOSSecure::setGlue(APP, AUAppGlueAppleSecure::create)
#	define AUApp_linkToDefaultPkgFilesystem(APP)	NBGestorArchivos::setGlue(APP, AUAppGlueOsxFiles::create)
#	define AUApp_linkToDefaultKeyboard(APP)			(0)
#	define AUApp_linkToDefaultNotifSystem(APP)		(0)
#	define AUApp_linkToDefaultAVCapture(APP)		(0)
#	define AUApp_linkToDefaultStore(APP)			(0)
#	define AUApp_linkToDefaultTelephony(APP)		(0)
#	define AUApp_linkToDefaultContacts(APP)			NBMngrContacts::setGlue(APP, AUAppGlueAppleContacts::create)
#	define AUApp_linkToDefaultBiometrics(APP)		(0)
#	define AUApp_linkToDefaultGameKit(APP)			(0)
#	define AUApp_linkToDefaultPdfKit(APP)			NBMngrPdfKit::setGlue(APP, AUAppGlueApplePdfKit::create) //NBMngrPdfKit::setGlue(APP, AUAppGluePdfium::create)
#	endif
#else
//	UNDEFINED OS
#	define AUApp_linkToDefaultOSTools(APP)			(0)
#	define AUApp_linkToDefaultOSSecure(APP)			(0)
#	define AUApp_linkToDefaultPkgFilesystem(APP)	(0)
#	define AUApp_linkToDefaultKeyboard(APP)			(0)
#	define AUApp_linkToDefaultNotifSystem(APP)		(0)
#	define AUApp_linkToDefaultAVCapture(APP)		(0)
#	define AUApp_linkToDefaultStore(APP)			(0)
#	define AUApp_linkToDefaultTelephony(APP)		(0)
#	define AUApp_linkToDefaultContacts(APP)			(0)
#	define AUApp_linkToDefaultBiometrics(APP)		(0)
#	define AUApp_linkToDefaultGameKit(APP)			(0)
#	define AUApp_linkToDefaultPdfKit(APP)			(0)
#endif

#define AUApp_linkToDefaultsAll(APP)				AUApp_linkToDefaultOSTools(APP); \
													AUApp_linkToDefaultOSSecure(APP); \
													AUApp_linkToDefaultPkgFilesystem(APP); \
													AUApp_linkToDefaultKeyboard(APP); \
													AUApp_linkToDefaultNotifSystem(APP); \
													AUApp_linkToDefaultAVCapture(APP); \
													AUApp_linkToDefaultStore(APP); \
													AUApp_linkToDefaultTelephony(APP); \
													AUApp_linkToDefaultContacts(APP); \
													AUApp_linkToDefaultBiometrics(APP); \
													AUApp_linkToDefaultGameKit(APP); \
													AUApp_linkToDefaultPdfKit(APP); \
													(0)

class AUApp;

enum ENAUAppTickTipo {
	ENAUAppTickTipo_TimerManual = 0,
	ENAUAppTickTipo_SincPantalla
};

struct STAppEstadisticas {
	UI32	ticksAnimEnSegAcum;
	UI32	ticksRenderEnSegAcum;
	//
	UI32	ultFramesAnimEnSeg;
	UI32	ultFramesRenderEnSeg;
	//
	struct timeval usTickAnimAnterior;
	struct timeval usTickRenderAnterior;
};

struct STAppHilos {
	bool						hiloPrincipalIniciado;
	AUApp*						app;
	//
	bool						finalizandoPrograma;
	bool						modoRenderSecuencial;			//Utilizado cuando esta pendiente rotar la pantalla o se estan cargando recursos
	bool						renderbufferLlenado;
	struct {
		UI32					stackUse;
		NBTamanoI				size;
		float					sceneScale;
		NBTamano				ppiScreen;	//pixel-per-inch
		NBTamano				dpiScene;	//dot-per-inch
	} redim;
	//
	#if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	bool						hiloProductorEscenaTrabajando;
	AUHilo*						hiloProductorEscena;			//La animacion se ejecutará en pararlelo al renderizado, asi se aprovecharan los ciclos de espera a la GPU
	APPHILO_MUTEX_CLASE			mutexBufferesEscena;			//Semaforo para acceder/editar los estados de lo bufferes de escena
	APPHILO_MUTEX_CLASE			mutexEsperaProducirEscena;		//Semaforo para la espera a que se llene un buffer escena
	APPHILO_MUTEX_CLASE			mutexEsperaConsumirEscena;		//Semaforo para la espera a que se vacie un buffer escena
	APPHILO_COND_CLASE			condEsperaProducirEscena;		//...que se llene un buffer escena
	APPHILO_COND_CLASE			condEsperaConsumirEscena;		//...que se vacie un buffer escena
	#endif
	#ifdef APP_IMPLEMENTAR_TRI_HILO
	SI32						ticksCargandoRecursos;
	bool						hiloConsumidorEscenaTrabajando;
	AUHilo*						hiloConsumidorEscena;			//La animacion se ejecutará en pararlelo al renderizado, asi se aprovecharan los ciclos de espera a la GPU
	APPHILO_MUTEX_CLASE			mutexEsperaConsumirRender;		//Semaforo para la espera a que se vuelque el renderBuffer a pantalla
	APPHILO_COND_CLASE			condEsperaConsumirRender;		//...que se vuelque el renderBuffer a pantalla
	#endif
};

//
// Funciones con atributos
// El atributo 'constructor' invoca el metodo antes que 'main'
// El atributo 'destructor' invoca el metodo despues de 'main'
// http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
// "a relative priority, a constant integral expression currently bounded between 101 and 65535 inclusive"
//

#ifndef NB_METODO_INICIALIZADOR_DEF
#error "Falta inclusion. Aun no se ha definido la macro NB_METODO_INICIALIZADOR_DEF."
#endif

NB_METODO_INICIALIZADOR_DEF(AUApp_inicializarMemoria, NB_PRIORIDAD_CONSTRUCTOR_BASE);

//Buffer
typedef void (*PTRfuncVolcarBuffer)(void* param, AUApp* app, SI32 iEscenaRender);

//Audio
typedef void (*PTRfuncAudioActivateSession)(void* param);
typedef void (*PTRfuncAudioDeactivateSession)(void* param);
typedef void (*PTRfuncAudioUpdateTrackInfo)(void* param);
typedef void (*PTRfuncAudioUpdateTrackState)(void* param);

typedef struct STAppCallbacks_ {
	//Buffer
	PTRfuncGeneraRenderbuffer	funcGenerarRenderBuffer;
	void*						funcGenerarRenderBufferParam;
	PTRfuncVolcarBuffer			funcVolcarBuffer;
	void*						funcVolcarBufferParam;
	//Audio
	PTRfuncAudioActivateSession	funcAudioActivateSession;
	void*						funcAudioActivateSessionParam;
	PTRfuncAudioDeactivateSession funcAudioDeactivateSession;
	void*						funcAudioDeactivateSessionParam;
	PTRfuncAudioUpdateTrackInfo	funcAudioUpdateTrackInfo;
	void*						funcAudioUpdateTrackInfoParam;
	PTRfuncAudioUpdateTrackState funcAudioUpdateTrackState;
	void*						funcAudioUpdateTrackStateParam;
} STAppCallbacks;

class AUApp: public AUAppI {
	public:
		//
		AUApp(const STAppCallbacks* callbacks, const char* nomCarpetaCache, const bool permitirActividadRedEnBG);
		virtual ~AUApp();
		//Links (default glues)
		//
		UI32					getTicksWithSameContentMin();	//Minimun to avoid rendering the scene (2 to fill doubleBuffers, etc...)
		UI32					getTicksWithSameContent();		//Amount of ticks were content remained the same
		float					getSecsWithSameContent();		//Ammount of seconds were content remained the same
#		ifdef _WIN32
		HWND					getWindowHandle();
		void					setWindowHandle(HWND hWnd);
#		endif
		//iOS specifics
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		void					setLaunchOptions(void* launchOptions /*NSDictionary*/);
		void*					getLaunchOptions();
		void					setApplication(void* app /*UIApplication*/);
		void*					getApplication();
		void					setWindow(void* win /*UIWindow*/);
		void*					getWindow();
		void					setViewController(void* vc /*UIViewController*/);
		void*					getViewController();
#		endif
#		ifdef __ANDROID__
		static bool				linkToDefaultJni(void* jEnv, void* pActivity /*jobject*/);
		static AUAppGlueAndroidJNI*	getDefaultGlueJNI();
		AUAppGlueAndroidJNI*	getGlueJNI();
		bool					setAppNative(void* jAppNative /*jobject*/);
		void* /*jobject*/		getAppNative();
		bool					setSrvcConnMon(void* srvcConnMon /*jobject*/);
		void* /*jobject*/		getSrvcConnMon();
		bool					setView(void* pView /*jobject*/);
		void* /*jobject*/		getView();
#		endif
		//App state changes
		void					addAppStateListener(AUAppStateListener* listener);
		void					removeAppStateListener(AUAppStateListener* listener);
		void					appStateOnCreate();
		void					appStateOnDestroy(); //applicationWillTerminate
		void					appStateOnStart();	//applicationDidBecomeActive
		void					appStateOnStop();	//applicationWillResignActive
		void					appStateOnResume();	//applicationWillEnterForeground
		void					appStateOnPause();	//applicationDidEnterBackground
		//Android specifics
#		ifdef __ANDROID__
		void					addAppSrvcConnListener(AUAppSrvcConnListener* listener);
		void					removeAppSrvcConnListener(AUAppSrvcConnListener* listener);
		void					appSrcvOnConnected(void* compName /*jobject*/, void* iBinder /*jobject*/);
		void					appSrcvOnDisconnected(void* compName /*jobject*/);
#		endif
#		ifdef __ANDROID__
		void					addAppActivityResultListener(AUAppActResultListener* listener);
		void					removeAppActivityResultListener(AUAppActResultListener* listener);
		void					appActResultReceived(const SI32 reqCode, const SI32 result, void* jIntent /*jobject*/);
#		endif
#		ifdef __ANDROID__
		void					addReqPermResultListener(AUAppReqPermResultListener* listener);
		void					removeReqPermResultListener(AUAppReqPermResultListener* listener);
		void					appReqPermResult(const SI32 request, void* perms /*jobjectArray*/, void* grantResutls /*jintArray*/);
#		endif
#		ifdef __ANDROID__
		void					addAppIntentListener(AUAppIntentListener* listener);
		void					removeAppIntentListener(AUAppIntentListener* listener);
		void					appIntentReceived(void* intent /*jobject::Intent*/);
#		endif
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		void					addAppNotifListener(AUAppNotifListener* listener);
		void					removeAppNotifListener(AUAppNotifListener* listener);
		void					appLocalNotifReceived(void* notif /*UILocalNotification*/);
#		endif
		void					addAppOpenUrlListener(AUAppOpenUrlListener* listener);
		void					removeAppOpenUrlListener(AUAppOpenUrlListener* listener);
		bool					broadcastOpenUrl(const char* plainUrl, const void* usrData, const UI32 usrDataSz);
		bool					broadcastOpenUrlImage(const char* plainUrl, const SI32 rotFromIntended, const void* usrData, const UI32 usrDataSz);
		bool					broadcastOpenFileData(const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz);
		bool					broadcastOpenFileImageData(const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz);
		bool					broadcastOpenBitmap(const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz);
#		ifdef __ANDROID__
		static float			getDefaultDisplayRefreshRate(void* jEnv /*JNIEnv*/, void* jActivity);
		static NBTamano			getDefaultDisplayDensity(void* jEnv /*JNIEnv*/, void* jActivity);
		void					onWindowFocusChanged(void* jEnv, const SI32 hasFocus);
		void					onFocusChange(void* jEnv /*JNIEnv*/, void* jView /*jobject*/, const SI32 hasFocus);
		bool					onTouchEvent(void* jEnv /*JNIEnv*/, void* jEvent /*jobject*/);
		bool					onKeyDown(void* jEnv /*JNIEnv*/, const SI32 keyCode, void* jEvent /*jobject*/);
		bool					onKeyUp(void* jEnv /*JNIEnv*/, const SI32 keyCode, void* jEvent /*jobject*/);
		void					setKeyboardHeight(void* pEnv, float height, bool overlapsContent);
#		endif
		//
		static void				inicializarCallbacks(STAppCallbacks* callbacks);
		//
		static bool				inicializarNucleo(const UI32 mascaraModulos);
		static void				finalizarNucleo();
		//
		bool					inicializarMultimedia(const bool leerPrecache, const bool leerCache, const bool escribirCache, const bool initGraphics, const float pantallaFrecuencia);
		bool					reinicializarMultimedia(const bool elimObjetosAnteriores);
		void					finalizarMultimedia();
		//
		static SI32				conteoGestoresInicializados();
		static void				aplicarAutoliberaciones();
		static UI32				liberarRecursosSinUso();
		//
		bool					inicializarVentana(const NBTamanoI wSize, const NBTamano ppiScreen, const NBTamano dpiScene, const ENGestorEscenaDestinoGl destinoGL);
		void					finalizarVentana();
		bool					inicializarJuego(AUAppEscenasAdmin* gameplays);
		void					finalizarJuego();
		//
		void					audioActivarSesion();
		void					audioDesactivarSesion();
		void					audioUpdateTrackInfo();
		void					audioUpdateTrackState();
		//
		SI32					indiceEscenaRender();
		float					escalaHaciaHD();
		AUAppEscenasAdmin*		gameplays();
		//
		void					notificarRedimensionVentana(const NBTamanoI wSize, const float sceneScale, const NBTamano ppiScreen, const NBTamano dpiScene);
		//
		static void				tickPerSecThreadFunc(void* param);
		float					tickJuego(const ENAUAppTickTipo tipoTick, const bool acumularFramesSaltadas);
		void					tickSoloSonidos(const float segsTranscurridos);
		void					tickSoloProducirRender();
		void					tickSoloConsumirRender();
		bool					teclaPresionada(SI32 tecla);
		bool					teclaLevantada(SI32 tecla);
		bool					teclaEspecialPresionada(SI32 codigoTecla);
		//
		bool					juegoCargado();
		//
		bool					renderPausado();
		void					renderPausar();
		void					renderReanudar();
		//
		bool					animadorPausado();
		void					animadorPausar();
		void					animadorReanudar();
		//
		//void					aplicarConfiguracion();
		//float					configEscalaHaciaHD();
		//bool					configEscalaHaciaHDEsManual();
		//const char*			configIdioma();
		//bool					configIdiomaEsManual();
		//
#		if defined(CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES) && defined(CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA) && defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS)
		static void				debugImprimirPunterosAgrupadosDeZonaMemoria(ENMemoriaTipo tipoZonaMemoria, bool agruparPorNombresDeClase);
#		endif
#		ifdef CONFIG_NB_GESTOR_AUOBJETOS_REGISTRAR_TODOS
		static void				debugVolcarObjetosCSV(FILE* flujo);
#		endif
	private:
		static UI32				_modulosActivos;
#		ifdef _WIN32
		HWND					_hWnd;
#		endif
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		void*					_launchOptions;
		void*					_app;
		void*					_window;
		void*					_viewController;
#		endif
#		ifdef __ANDROID__
		static AUAppGlueAndroidJNI* _glueJNI;
		void*					_jAppNative; /*jobject*/
		void*					_jSrvcConnMon; /*jobject*/
		void*					_jView; /*jobject*/
		bool					_jWindowHasFocus; /*jobject*/
		bool					_jViewHasFocus; /*jobject*/
#		endif
		//
		AUAppEscenasAdmin*		_gameplays;
		AUEscenaResumenDebug*	_resumenDebug;
		//AUAlmacenDatos*		_almacenConfig;
		STAppEstadisticas		_estadisticas;
		AUArregloNativoMutableP<AUAppStateListener*>* _listnrsAppState;
#		ifdef __ANDROID__
		AUArregloNativoMutableP<AUAppSrvcConnListener*>* _listnrsAppSrvcConn;
		AUArregloNativoMutableP<AUAppActResultListener*>* _listnrsAppActResult;
		AUArregloNativoMutableP<AUAppReqPermResultListener*>* _listnrsReqPermResult;
		AUArregloNativoMutableP<AUAppIntentListener*>* _listnrsAppIntent;
#		endif
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		AUArregloNativoMutableP<AUAppNotifListener*>* _listnrsAppNotif;
#		endif
		AUArregloNativoMutableP<AUAppOpenUrlListener*>* _listnrsAppOpenUrl;
		//
		bool					_juegoCargado;
		bool					_animacionPausada;
		bool					_renderizacionPausada;
		bool					_permitirActividadRedEnBG;
		STAppCallbacks			_callbacks;
		SI32					_indiceEscenaRender;
		SI32					_ticksPantalaAcum;
		UI32					_ticksWithSameContentMin;	//Minimun to avoid rendering the scene (2 to fill doubleBuffers, etc...)
		UI32					_ticksWithSameContent;		//Amount of ticks were content remained the same
		float					_secsWithSameContent;		//Ammount of seconds were content remained the same
		//
		float					_escalaHaciaHD;
		bool					_escalaHaciaHDEsManual;
		//
		NBHILO_MUTEX_CLASE		_tickMutex;
		SI32					_tickCallsCur;
		AUHilo*					_tickPerSecThread;
		STAppHilos				_datosHilos;
		bool					_keyboardShowActive;
		//Autorotation
		float					_autorotVerifSecsAcum;
		//
		static void				inicializarEstadisticas(STAppEstadisticas* datos);
		void					hiloIniciado();
		void					redimensionarEscena(const NBTamanoI wSize, const float sceneScale, const NBTamano ppiScreen, const NBTamano dpiScene);
		//
		BOOL					privConfigureForResolution(const NBTamanoI wSize, const NBTamano ppiScreen, const NBTamano dpiScene);
		//
		static void				funcProductorDatosEscena(void* datos);
		static void				funcConsumidorDatosEscena(void* datos);
		static void				funcConsumidorDatosRender(void* datos);
};

#endif
