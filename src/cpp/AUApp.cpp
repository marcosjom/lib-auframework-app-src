//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUApp.h"
#include "NBMngrNotifs.h"
#include "NBMngrAVCapture.h"
#include "NBMngrOSTools.h"
#include "NBMngrOSSecure.h"
#include "NBMngrBiometrics.h"
#include "NBMngrStore.h"
#include "NBMngrContacts.h"
#include "NBMngrOSTelephony.h"
#include "NBMngrGameKit.h"
#include "NBMngrPdfKit.h"
#include "NBMngrFbLogin.h"
#include "NBMngrGoogleLogin.h"

#if defined(__ANDROID__)
#	include <jni.h>
#	include <android/log.h>	//for __android_log_print()
#elif defined(__APPLE__) && TARGET_OS_IPHONE
#	include "nb/NBObjCMethods.h"	//for "objc_msgSend", "sel_registerName", ...
//#	include <objc/message.h>	//for "objc_msgSend"
//#	include <objc/objc.h>		//for "sel_registerName"
#endif

//------------------------------------------------------------
// Funciones con atributos
// El atributo 'constructor' invoca el metodo antes que 'main'
// El atributo 'destructor' invoca el metodo despues de 'main'
// http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
// "a relative priority, a constant integral expression currently bounded between 101 and 65535 inclusive"
//------------------------------------------------------------
#include "AUAppSobrecargaOperadoresNew.h"

#ifndef NB_METODO_INICIALIZADOR_CUERPO
#error "Falta inclusion. Aun no se ha definido la macro NB_METODO_INICIALIZADOR_CUERPO."
#endif

//Automaticamente invocada antes de 'main'
NB_METODO_INICIALIZADOR_CUERPO(AUApp_inicializarMemoria) {
	//PRINTF_INFO("\n\n++++ ------ MEMORIA INICIALIZADA MEDIANTE CONSTRUCTOR ------ ++++.\n\n");
	NBGestorMemoria::inicializar(1, (1024 * 1024 * (2.2f)), 8191,	//Zonas de memoria NUCLEO/AGIL: cantidadPregeneradas, bytesMinPorZona, registrosIndicesMinimoPorZona
								 1, (1024 * 1024 * (1.1f)), 512);	//Zonas de memoria TEMPORAL: cantidadPregeneradas, bytesMinPorZona, registrosIndicesMinimoPorZona
};

//Automaticamente invocada despues de 'main'
//PENDIENTE, rehabilitar para GCC
/*NB_METODO_FINALIZADOR(AUApp_finalizarMemoria, NB_PRIORIDAD_CONSTRUCTOR_BASE) {
	PRINTF_INFO("++++ ------ MEMORIA FINALIZADA MEDIANTE CONSTRUCTOR ------ ++++.\n");
	NBGestorMemoria::finalizar();
};*/

UI32 AUApp::_modulosActivos		= 0;
#ifdef __ANDROID__
AUAppGlueAndroidJNI* AUApp::_glueJNI = NULL;
#endif


void AUApp::inicializarEstadisticas(STAppEstadisticas* datos){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::inicializarEstadisticas")
	datos->ticksAnimEnSegAcum	= 0;
	datos->ticksRenderEnSegAcum	= 0;
	datos->ultFramesAnimEnSeg	= 0;
	datos->ultFramesRenderEnSeg	= 0;
	//
	datos->usTickAnimAnterior.tv_sec	= 0;
	datos->usTickAnimAnterior.tv_usec	= 0;
	datos->usTickRenderAnterior.tv_sec	= 0;
	datos->usTickRenderAnterior.tv_usec	= 0;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

AUApp::AUApp(const STAppCallbacks* callbacks, const char* nomCarpetaCache, const bool permitirActividadRedEnBG){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::AUApp")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUApp")
	NBASSERT(NBGestorMemoria::gestorInicializado());
	NBASSERT(NBGestorArchivos::gestorInicializado());
	_keyboardShowActive	= false;
	//Glues
	{
#		ifdef _WIN32
		_hWnd				= NULL;
#		endif
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		_launchOptions		= NULL;
		_app				= NULL;
		_window				= NULL;
		_viewController		= NULL;
#		endif
#		if defined(__ANDROID__)
		_jAppNative			= NULL;
		_jSrvcConnMon		= NULL;
		_jView				= NULL;
		_jWindowHasFocus	= false;
		_jViewHasFocus		= false;
#		endif
	}
	//
	_callbacks						= (*callbacks);
	_permitirActividadRedEnBG		= permitirActividadRedEnBG;
	//
	AUApp::inicializarEstadisticas(&_estadisticas);
	//
	_listnrsAppState		= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppStateListener*>();
#	ifdef __ANDROID__
	_listnrsAppSrvcConn		= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppSrvcConnListener*>();
	_listnrsAppActResult	= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppActResultListener*>();
	_listnrsReqPermResult	= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppReqPermResultListener*>();
	_listnrsAppIntent		= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppIntentListener*>();
#	endif
#	if defined(__APPLE__) && TARGET_OS_IPHONE
	_listnrsAppNotif		= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppNotifListener*>();
#	endif
	_listnrsAppOpenUrl		= new(ENMemoriaTipo_General) AUArregloNativoMutableP<AUAppOpenUrlListener*>();
	//-----------------------------------
	//SIMULAR SOLO EXISTENCIA DE PAQUETES
	//-----------------------------------
	//NBGestorArchivos::establecerSimularSoloRutasHaciaPaquetes(true);
	//
	//-----------------------------------
	//PREPARAR EL USO DE LA CACHE
	//-----------------------------------
	if(nomCarpetaCache != NULL){
		if(nomCarpetaCache[0] != '\0'){
			AUCadenaMutable8* rutaCacheLogica = new(ENMemoriaTipo_Temporal) AUCadenaMutable8(NBGestorArchivos::rutaHaciaRecursoEnCache(nomCarpetaCache));
			const char ultChar = rutaCacheLogica->str()[rutaCacheLogica->tamano() - 2];
			if(ultChar != '\\' && ultChar != '/') { rutaCacheLogica->agregar(rutaCacheLogica->indiceDe('/') != -1 ? '/' : '\\'); }
			//PRINTF_INFO("Estableciendo cache operativa: '%s'.\n", rutaCacheLogica->str());
			NBGestorArchivos::crearCarpeta(rutaCacheLogica->str());
			NBGestorArchivos::establecerRutaCacheOperativa(rutaCacheLogica->str());
			rutaCacheLogica->liberar(NB_RETENEDOR_THIS);
		}
	}
	//
	//
	//NBGestorDatos::inicializar();
	//NBGestorDatos::cargarDesdeArchivos(); _almacenConfig = NBGestorDatos::almacenDatos("config");
	NBGestorIdioma::inicializar();
	//
	NBGestorTeclas::inicializar();
	NBGestorTouches::inicializar();
	//
	PRINTF_INFO("-----------------------\n");
	PRINTF_INFO("Nicaragua Binary S.A. - AuApp\n");
	//
	_gameplays				= NULL;
	_resumenDebug			= NULL;
	_indiceEscenaRender		= -1;
	_juegoCargado			= false;
	_animacionPausada		= false;
	_renderizacionPausada	= false;
	_escalaHaciaHD			= 1.0f;
	_escalaHaciaHDEsManual	= false;
	//
	_ticksWithSameContentMin = 6;	//Minimun to avoid rendering the scene (3 to fill tripleBuffers, etc...)
	_ticksWithSameContent	= 0;	//Amount of ticks were content remained the same
	_secsWithSameContent	= 0.0f;
	//Autorotation
	_autorotVerifSecsAcum	= 0.0f;
	//Thread ticksPerSec
	NBHILO_MUTEX_INICIALIZAR(&_tickMutex);
	_tickCallsCur			= 0;
	_tickPerSecThread		= NULL;
	if(_tickPerSecThread == NULL){
		_tickPerSecThread	= new(ENMemoriaTipo_General) AUHilo("AUApp::tickPerSecThreadFunc");
		if(!_tickPerSecThread->crearHiloYEjecuta(&tickPerSecThreadFunc, this)){
			_tickPerSecThread->liberar(NB_RETENEDOR_THIS);
			_tickPerSecThread = NULL;
		}
	}
	{
		NBMngrFbLogin::init(this);
		NBMngrGoogleLogin::init(this);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

AUApp::~AUApp(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::~AUApp")
	{
		NBMngrGoogleLogin::finish();
		NBMngrFbLogin::finish();
	}
	NBHILO_MUTEX_ACTIVAR(&_tickMutex);
	if(_tickPerSecThread != NULL){
		_tickPerSecThread->marcarComoCanceladoYEsperar();
		_tickPerSecThread->liberar(NB_RETENEDOR_THIS);
		_tickPerSecThread = NULL;
	}
	if(_listnrsAppState != NULL){
		_listnrsAppState->vaciar();
		_listnrsAppState->liberar(NB_RETENEDOR_THIS);
		_listnrsAppState = NULL;
	}
#	ifdef __ANDROID__
	if(_listnrsAppSrvcConn != NULL){
		_listnrsAppSrvcConn->vaciar();
		_listnrsAppSrvcConn->liberar(NB_RETENEDOR_THIS);
		_listnrsAppSrvcConn = NULL;
	}
	if(_listnrsAppActResult != NULL){
		_listnrsAppActResult->vaciar();
		_listnrsAppActResult->liberar(NB_RETENEDOR_THIS);
		_listnrsAppActResult = NULL;
	}
	if(_listnrsReqPermResult != NULL){
		_listnrsReqPermResult->vaciar();
		_listnrsReqPermResult->liberar(NB_RETENEDOR_THIS);
		_listnrsReqPermResult = NULL;
	}
	if(_listnrsAppIntent != NULL){
		_listnrsAppIntent->vaciar();
		_listnrsAppIntent->liberar(NB_RETENEDOR_THIS);
		_listnrsAppIntent = NULL;
	}
#	endif
#	if defined(__APPLE__) && TARGET_OS_IPHONE
	if(_listnrsAppNotif != NULL){
		_listnrsAppNotif->vaciar();
		_listnrsAppNotif->liberar(NB_RETENEDOR_THIS);
		_listnrsAppNotif = NULL;
	}
#	endif
#	if defined(__APPLE__) && TARGET_OS_IPHONE
	if(_listnrsAppOpenUrl != NULL){
		_listnrsAppOpenUrl->vaciar();
		_listnrsAppOpenUrl->liberar(NB_RETENEDOR_THIS);
		_listnrsAppOpenUrl = NULL;
	}
#	endif
#	if defined(__APPLE__) && TARGET_OS_IPHONE
	{
		if(_launchOptions != NULL){
			void_objc_msgSend((id)_launchOptions, sel_registerName("release"));
			_launchOptions = NULL;
		}
		if(_app != NULL){
			void_objc_msgSend((id)_app, sel_registerName("release"));
			_app = NULL;
		}
		if(_window != NULL){
			void_objc_msgSend((id)_window, sel_registerName("release"));
			_window = NULL;
		}
		if(_viewController != NULL){
			void_objc_msgSend((id)_viewController, sel_registerName("release"));
			_viewController = NULL;
		}
	}
#	endif
#	if defined(__ANDROID__)
	{
		if(_glueJNI != NULL){
			JNIEnv* jEnv = (JNIEnv*)_glueJNI->curEnv(); NBASSERT(jEnv != NULL)
			if(jEnv != NULL){
				//Release global reference of AppNative
				if(_jAppNative != NULL){
					jEnv->DeleteGlobalRef((jobject)_jAppNative);
				}
				//Release global reference of the SrvcConnMon
				if(_jSrvcConnMon != NULL){
					jEnv->DeleteGlobalRef((jobject)_jSrvcConnMon);
				}
				//Release global reference of the view
				if(_jView != NULL){
					jEnv->DeleteGlobalRef((jobject)_jView);
				}
			}
		}
		_jAppNative		= NULL;
		_jSrvcConnMon	= NULL;
		_jView			= NULL;
	}
#	endif
	//En Graficos
	NBASSERT(!NBGestorGL::gestorInicializado())
	NBASSERT(!NBGestorAnimadores::gestorInicializado())
	NBASSERT(!NBGestorAnimaciones::gestorInicializado())
	NBASSERT(!NBGestorPersonajes::gestorInicializado())
	NBASSERT(!NBGestorTexturas::gestorInicializado())
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	NBASSERT(!NBGestorSonidos::gestorInicializado())
#	endif
	NBASSERT(!NBGestorEscena::gestorInicializado())
	NBASSERT(!NBGestorCuerpos::gestorInicializado())
#	ifndef CONFIG_NB_UNSUPPORT_BOX2D
	NBASSERT(!NBGestorFisica::gestorInicializado())
#	endif
	//EnNucleo
	NBASSERT(NBGestorArchivos::gestorInicializado())
	NBASSERT(NBGestorRed::gestorInicializado())
	NBASSERT(NBGestorHilos::gestorInicializado())
	//En constructor AUApp
	NBASSERT(NBGestorIdioma::gestorInicializado())
	NBASSERT(NBGestorTeclas::gestorInicializado())
	NBASSERT(NBGestorTouches::gestorInicializado())
	//NBASSERT(NBGestorDatos::gestorInicializado())
	//
	NBASSERT(_indiceEscenaRender == -1)
	this->finalizarJuego();
	NBASSERT(_resumenDebug == NULL)
	NBASSERT(_gameplays == NULL)
	//
	_datosHilos.finalizandoPrograma = true;
	//
	NBGestorTouches::finalizar();
	NBGestorTeclas::finalizar();
	//
	//NBGestorDatos::guardarHaciaArchivos();
	//NBGestorDatos::finalizar();
	NBGestorIdioma::finalizar();
	//
#	if defined(__ANDROID__)
	if(_glueJNI != NULL) delete _glueJNI; _glueJNI = NULL;
#	endif
	//
	AUApp::aplicarAutoliberaciones();
#	ifdef CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES
	if(NBGestorMemoria::cantidadBloquesReservados() != 0){
		PRINTF_INFO("%d PUNTEROS EN USO AL FINALIZAR AUAPP (deberia ser muy poco)\n", NBGestorMemoria::cantidadBloquesReservados());
		//NBGestorMemoria::debug_imprimePunterosEnUso();
		//PRINTF_INFO("FIN DE LISTA ------------------------\n");
		//
		/*FILE* flujo = fopen("./punteros_en_uso.csv", "wb");
		if(flujo != NULL){
			NBGestorMemoria::debug_volcarPunterosEnUsoCSV(flujo);
			fclose(flujo);
			PRINTF_INFO("Lista volcada a archivo: './punteros_en_uso.csv'\n");
		} else {
			PRINTF_INFO("No s epudo volcar lista de punteros a archivo: './punteros_en_uso.csv'\n");
		}*/
	} else {
		PRINTF_INFO("SIN PUNTEROS EN USO AL FINALIZAR GESTORES (salida limpia)\n");
	}
#	endif
	NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	NBHILO_MUTEX_FINALIZAR(&_tickMutex);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

//

UI32 AUApp::getTicksWithSameContentMin(){ //Minimun to avoid rendering the scene (2 to fill doubleBuffers, etc...)
	return _ticksWithSameContentMin;
}

UI32 AUApp::getTicksWithSameContent(){ //Amount of ticks were content remained the same
	return _ticksWithSameContent;
}

float AUApp::getSecsWithSameContent(){ //Ammount of seconds were content remained the same
	return _secsWithSameContent;
}

//-------
//- Glues
//-------

#ifdef _WIN32
HWND AUApp::getWindowHandle(){
	return _hWnd;
}
#endif

#ifdef _WIN32
void AUApp::setWindowHandle(HWND hWnd){
	_hWnd = hWnd;
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::setLaunchOptions(void* launchOptions /*NSDictionary*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setLaunchOptions")
	if(launchOptions != NULL){
		id_objc_msgSend((id)launchOptions, sel_registerName("retain"));
	}
	if(_launchOptions != NULL){
		void_objc_msgSend((id)_launchOptions, sel_registerName("release"));
	}
	 _launchOptions = launchOptions;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void* AUApp::getLaunchOptions(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getLaunchOptions")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _launchOptions;
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::setApplication(void* app /*UIApplication*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setApplication")
	if(app != NULL){
		id_objc_msgSend((id)app, sel_registerName("retain"));
	}
	if(_app != NULL){
		void_objc_msgSend((id)_app, sel_registerName("release"));
	}
	_app = app;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void* AUApp::getApplication(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getApplication")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _app;
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::setWindow(void* win /*UIWindow*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setWindow")
	if(win != NULL){
		id_objc_msgSend((id)win, sel_registerName("retain"));
	}
	if(_window != NULL){
		void_objc_msgSend((id)_window, sel_registerName("release"));
	}
	_window = win;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void* AUApp::getWindow(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getWindow")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _window;
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::setViewController(void* vc /*UIViewController*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setViewController")
	if(vc != NULL){
		id_objc_msgSend((id)vc, sel_registerName("retain"));
	}
	if(_viewController != NULL){
		void_objc_msgSend((id)_viewController, sel_registerName("release"));
	}
	_viewController = vc;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void* AUApp::getViewController(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getViewController")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _viewController;
}
#endif

#ifdef __ANDROID__
bool AUApp::linkToDefaultJni(void* jEnv, void* pActivity /*jobject*/){
	if(_glueJNI != NULL) delete _glueJNI;
	_glueJNI = new AUAppGlueAndroidJNI(jEnv, pActivity);
	PRINTF_INFO("linkToDefaultJni() => %llu.\n", (UI64)_glueJNI);
	return true;
}
#endif

#ifdef __ANDROID__
AUAppGlueAndroidJNI* AUApp::getDefaultGlueJNI(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getDefaultGlueJNI")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	NBASSERT(_glueJNI != NULL)
	return _glueJNI;
}
#endif

#ifdef __ANDROID__
AUAppGlueAndroidJNI* AUApp::getGlueJNI() {
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getGlueJNI")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return AUApp::getDefaultGlueJNI();
}
#endif

#ifdef __ANDROID__
bool AUApp::setAppNative(void* jAppNative /*jobject*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setAppNative")
	bool r = false;
	//Retain a global reference to the view
	if(_glueJNI != NULL){
		JNIEnv* jEnv = (JNIEnv*)_glueJNI->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			if(_jAppNative != NULL){
				jEnv->DeleteGlobalRef((jobject)_jAppNative);
				_jAppNative = NULL;
			}
			if(jAppNative != NULL){
				_jAppNative = jEnv->NewGlobalRef((jobject)jAppNative);
			}
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}
#endif

#ifdef __ANDROID__
void* /*jobject*/ AUApp::getAppNative(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getAppNative")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _jAppNative;
}
#endif

#ifdef __ANDROID__
bool AUApp::setSrvcConnMon(void* srvcConnMon /*jobject*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setSrvcConnMon")
	bool r = false;
	//Retain a global reference to the view
	if(_glueJNI != NULL){
		JNIEnv* jEnv = (JNIEnv*)_glueJNI->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			if(_jSrvcConnMon != NULL){
				jEnv->DeleteGlobalRef((jobject)_jSrvcConnMon);
				_jSrvcConnMon = NULL;
			}
			if(srvcConnMon != NULL){
				_jSrvcConnMon = jEnv->NewGlobalRef((jobject)srvcConnMon);
			}
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}
#endif

#ifdef __ANDROID__
void* /*jobject*/ AUApp::getSrvcConnMon(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getSrvcConnMon")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _jSrvcConnMon;
}
#endif

#ifdef __ANDROID__
bool AUApp::setView(void* pView /*jobject*/){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setView")
	bool r = false;
	//Retain a global reference to the view
	if(_glueJNI != NULL){
		JNIEnv* jEnv = (JNIEnv*)_glueJNI->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			if(_jView != NULL){
				jEnv->DeleteGlobalRef((jobject)_jView);
				_jView = NULL;
			}
			if(pView != NULL){
				_jView = jEnv->NewGlobalRef((jobject)pView);
			}
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}
#endif

#ifdef __ANDROID__
void* /*jobject*/ AUApp::getView(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::getView")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _jView;
}
#endif

/*
getWindowManager().getDefaultDisplay().getRefreshRate();
*/

#ifdef __ANDROID__
float AUApp::getDefaultDisplayRefreshRate(void* pEnv /*JNIEnv*/, void* pActivity){
	float r = 1;
	if(pEnv != NULL && pActivity != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		jobject jActivity = (jobject)pActivity;
		jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
		jclass clsWinMngr	= jEnv->FindClass("android/view/WindowManager"); NBASSERT(clsWinMngr != NULL)
		jclass clsDisplay	= jEnv->FindClass("android/view/Display"); NBASSERT(clsDisplay != NULL)
		if(clsActivity != NULL && clsWinMngr != NULL && clsDisplay != NULL){
			jmethodID mGetWinMngr	= jEnv->GetMethodID(clsActivity, "getWindowManager", "()Landroid/view/WindowManager;"); NBASSERT(mGetWinMngr != NULL)
			jmethodID mGetDisplay	= jEnv->GetMethodID(clsWinMngr, "getDefaultDisplay", "()Landroid/view/Display;"); NBASSERT(mGetDisplay != NULL)
			jmethodID mGetRefRate	= jEnv->GetMethodID(clsDisplay, "getRefreshRate", "()F"); NBASSERT(mGetRefRate != NULL)
			if(mGetWinMngr != NULL && mGetDisplay != NULL && mGetRefRate != NULL){
				jobject winMngr = jEnv->CallObjectMethod(jActivity, mGetWinMngr);
				if(winMngr != NULL){
					jobject display = jEnv->CallObjectMethod(winMngr, mGetDisplay);
					if(display != NULL){
						r = jEnv->CallFloatMethod(display, mGetRefRate);
						NBJNI_DELETE_REF_LOCAL(jEnv, display)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, winMngr)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsDisplay)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsWinMngr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
		}
	}
	return r;
}
#endif

/*
DisplayMetrics displayMetrics = new DisplayMetrics();
this.getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
displayMetrics.xdpi, displayMetrics.ydpi
*/

#ifdef __ANDROID__
NBTamano AUApp::getDefaultDisplayDensity(void* pEnv /*JNIEnv*/, void* pActivity){
	NBTamano r; r.ancho = 1; r.alto = 1;
	if(pEnv != NULL && pActivity != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		jobject jActivity = (jobject)pActivity;
		jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
		jclass clsWinMngr	= jEnv->FindClass("android/view/WindowManager"); NBASSERT(clsWinMngr != NULL)
		jclass clsDisplay	= jEnv->FindClass("android/view/Display"); NBASSERT(clsDisplay != NULL)
		jclass clsDisMetrcs	= jEnv->FindClass("android/util/DisplayMetrics"); NBASSERT(clsDisMetrcs != NULL)
		if(clsActivity != NULL && clsWinMngr != NULL && clsDisplay != NULL && clsDisMetrcs != NULL){
			jmethodID mGetWinMngr	= jEnv->GetMethodID(clsActivity, "getWindowManager", "()Landroid/view/WindowManager;"); NBASSERT(mGetWinMngr != NULL)
			jmethodID mGetDisplay	= jEnv->GetMethodID(clsWinMngr, "getDefaultDisplay", "()Landroid/view/Display;"); NBASSERT(mGetDisplay != NULL)
			jmethodID mGetMetrics	= jEnv->GetMethodID(clsDisplay, "getMetrics", "(Landroid/util/DisplayMetrics;)V"); NBASSERT(mGetMetrics != NULL)
			jmethodID mMetricsInit	= jEnv->GetMethodID(clsDisMetrcs, "<init>", "()V"); NBASSERT(mMetricsInit != NULL)
			jfieldID fXdpi			= jEnv->GetFieldID(clsDisMetrcs, "xdpi", "F"); NBASSERT(fXdpi != NULL)
			jfieldID fYdpi			= jEnv->GetFieldID(clsDisMetrcs, "ydpi", "F"); NBASSERT(fYdpi != NULL)
			if(mGetWinMngr != NULL && mGetDisplay != NULL && mGetMetrics != NULL && mMetricsInit != NULL){
				jobject winMngr = jEnv->CallObjectMethod(jActivity, mGetWinMngr);
				if(winMngr != NULL){
					jobject display = jEnv->CallObjectMethod(winMngr, mGetDisplay);
					if(display != NULL){
						jobject metrics = jEnv->NewObject(clsDisMetrcs, mMetricsInit);
						if(metrics != NULL){
							jEnv->CallVoidMethod(display, mGetMetrics, metrics);
							r.ancho	= jEnv->GetFloatField(metrics, fXdpi);
							r.alto	= jEnv->GetFloatField(metrics, fYdpi);
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, display)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, winMngr)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsDisMetrcs)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsDisplay)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsWinMngr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
		}
	}
	return r;
}
#endif

void AUApp::addAppStateListener(AUAppStateListener* listener){
	if(_listnrsAppState->indiceDe(listener) == -1){
		_listnrsAppState->agregarElemento(listener);
	}
}

void AUApp::removeAppStateListener(AUAppStateListener* listener){
	if(_listnrsAppState->indiceDe(listener) != -1){
		_listnrsAppState->quitarElemento(listener);
	}
}

void AUApp::appStateOnCreate(){
	SI32 i;
	for(i = (_listnrsAppState->conteo - 1); i >= 0; i--){
		_listnrsAppState->elem(i)->appStateOnCreate(this);
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "appStateOnCreate");
#	endif
}

void AUApp::appStateOnDestroy(){ // applicationWillTerminate
	SI32 i;
	for(i = (_listnrsAppState->conteo - 1); i >= 0; i--){
		_listnrsAppState->elem(i)->appStateOnDestroy(this);
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "appStateOnDestroy");
#	endif
}

void AUApp::appStateOnStart(){ //applicationDidBecomeActive
	SI32 i;
	for(i = (_listnrsAppState->conteo - 1); i >= 0; i--){
		_listnrsAppState->elem(i)->appStateOnStart(this);
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "appStateOnStart");
#	endif
}

void AUApp::appStateOnStop(){ //applicationWillResignActive
	SI32 i;
	for(i = (_listnrsAppState->conteo - 1); i >= 0; i--){
		_listnrsAppState->elem(i)->appStateOnStop(this);
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "appStateOnStop");
#	endif
}

void AUApp::appStateOnResume(){ //applicationWillEnterForeground
	//Start 'tickPerSec' thread
	if(_tickPerSecThread == NULL){
		_tickPerSecThread	= new(ENMemoriaTipo_General) AUHilo("AUApp::tickPerSecThreadFunc");
		if(!_tickPerSecThread->crearHiloYEjecuta(&tickPerSecThreadFunc, this)){
			_tickPerSecThread->liberar(NB_RETENEDOR_THIS);
			_tickPerSecThread = NULL;
		}
	}
	if(!_permitirActividadRedEnBG){
		NBGestorRed::establecerActividadRedPermitida(true);
	}
	if(NBGestorTexturas::gestorInicializado()){
		NBGestorTexturas::trabajadorSegundoPlanoIniciar();
	}
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppState->conteo - 1); i >= 0; i--){
			_listnrsAppState->elem(i)->appStateOnResume(this);
		}
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "appStateOnResume");
#	endif
}

void AUApp::appStateOnPause(){ //applicationDidEnterBackground
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppState->conteo - 1); i >= 0; i--){
			_listnrsAppState->elem(i)->appStateOnPause(this);
		}
	}
	//Stop 'tickPerSec' thread
	if(_tickPerSecThread != NULL){
		_tickPerSecThread->marcarComoCanceladoYEsperar();
		_tickPerSecThread->liberar(NB_RETENEDOR_THIS);
		_tickPerSecThread = NULL;
	}
	if(!_permitirActividadRedEnBG){
		NBGestorRed::establecerActividadRedPermitida(false);
	}
	if(NBGestorTexturas::gestorInicializado()){
		NBGestorTexturas::trabajadorSegundoPlanoDetenerYEsperar();
	}
	if(!_permitirActividadRedEnBG){
		NBGestorHilos::esperarHilosActivos();
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "appStateOnPause");
#	endif
	//
	const UI32 conteoLiberados = AUApp::liberarRecursosSinUso();
	PRINTF_INFO("%d recursos liberados (antes de perder el foco).\n", conteoLiberados);
}

// Service connection

#ifdef __ANDROID__
void AUApp::addAppSrvcConnListener(AUAppSrvcConnListener* listener){
	if(_listnrsAppSrvcConn->indiceDe(listener) == -1){
		_listnrsAppSrvcConn->agregarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::removeAppSrvcConnListener(AUAppSrvcConnListener* listener){
	if(_listnrsAppSrvcConn->indiceDe(listener) != -1){
		_listnrsAppSrvcConn->quitarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::appSrcvOnConnected(void* compName /*jobject*/, void* iBinder /*jobject*/){
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppSrvcConn->conteo - 1); i >= 0; i--){
			_listnrsAppSrvcConn->elem(i)->appSrcvOnConnected(this, compName, iBinder);
		}
	}
}
#endif

#ifdef __ANDROID__
void AUApp::appSrcvOnDisconnected(void* compName /*jobject*/){
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppSrvcConn->conteo - 1); i >= 0; i--){
			_listnrsAppSrvcConn->elem(i)->appSrcvOnDisconnected(this, compName);
		}
	}
}
#endif

//Activity Result

#ifdef __ANDROID__
void AUApp::addAppActivityResultListener(AUAppActResultListener* listener){
	if(_listnrsAppActResult->indiceDe(listener) == -1){
		_listnrsAppActResult->agregarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::removeAppActivityResultListener(AUAppActResultListener* listener){
	if(_listnrsAppActResult->indiceDe(listener) != -1){
		_listnrsAppActResult->quitarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::appActResultReceived(const SI32 reqCode, const SI32 result, void* jIntent /*jobject*/){
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppActResult->conteo - 1); i >= 0; i--){
			_listnrsAppActResult->elem(i)->appActResultReceived(this, reqCode, result, jIntent);
		}
	}
}
#endif

//ReqPerm result

#ifdef __ANDROID__
void AUApp::addReqPermResultListener(AUAppReqPermResultListener* listener){
	if(_listnrsReqPermResult->indiceDe(listener) == -1){
		_listnrsReqPermResult->agregarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::removeReqPermResultListener(AUAppReqPermResultListener* listener){
	if(_listnrsReqPermResult->indiceDe(listener) != -1){
		_listnrsReqPermResult->quitarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::appReqPermResult(const SI32 request, void* perms /*jobjectArray*/, void* grantResutls /*jintArray*/){
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsReqPermResult->conteo - 1); i >= 0; i--){
			_listnrsReqPermResult->elem(i)->appReqPermResult(this, request, perms, grantResutls);
		}
	}
}
#endif


// Intent

#ifdef __ANDROID__
void AUApp::addAppIntentListener(AUAppIntentListener* listener){
	if(_listnrsAppIntent->indiceDe(listener) == -1){
		_listnrsAppIntent->agregarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::removeAppIntentListener(AUAppIntentListener* listener){
	if(_listnrsAppIntent->indiceDe(listener) != -1){
		_listnrsAppIntent->quitarElemento(listener);
	}
}
#endif

#ifdef __ANDROID__
void AUApp::appIntentReceived(void* intent /*jobject::Intent*/){
	PRINTF_INFO("AUApp::appIntentReceived (with %d listeners).\n", _listnrsAppIntent->conteo);
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppIntent->conteo - 1); i >= 0; i--){
			_listnrsAppIntent->elem(i)->appIntentReceived(this, intent);
		}
	}
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::addAppNotifListener(AUAppNotifListener* listener){
	if(_listnrsAppNotif->indiceDe(listener) == -1){
		_listnrsAppNotif->agregarElemento(listener);
	}
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::removeAppNotifListener(AUAppNotifListener* listener){
	if(_listnrsAppNotif->indiceDe(listener) != -1){
		_listnrsAppNotif->quitarElemento(listener);
	}
}
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
void AUApp::appLocalNotifReceived(void* notif /*UILocalNotification*/){
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppNotif->conteo - 1); i >= 0; i--){
			_listnrsAppNotif->elem(i)->appLocalNotifReceived(this, notif);
		}
	}
}
#endif

//

void AUApp::addAppOpenUrlListener(AUAppOpenUrlListener* listener){
	if(_listnrsAppOpenUrl->indiceDe(listener) == -1){
		_listnrsAppOpenUrl->agregarElemento(listener);
	}
}

void AUApp::removeAppOpenUrlListener(AUAppOpenUrlListener* listener){
	if(_listnrsAppOpenUrl->indiceDe(listener) != -1){
		_listnrsAppOpenUrl->quitarElemento(listener);
	}
}

bool AUApp::broadcastOpenUrl(const char* plainUrl, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	//Notify listeners
	STNBUrl url;
	NBUrl_init(&url);
	if(NBUrl_parse(&url, plainUrl)){
		SI32 i;
		for(i = (_listnrsAppOpenUrl->conteo - 1); i >= 0; i--){
			if(_listnrsAppOpenUrl->elem(i)->appOpenUrl(this, &url, usrData, usrDataSz)){
				r = true;
			}
		}
	}
	NBUrl_release(&url);
	return r;
}

bool AUApp::broadcastOpenUrlImage(const char* plainUrl, const SI32 rotFromIntended, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	//Notify listeners
	STNBUrl url;
	NBUrl_init(&url);
	if(NBUrl_parse(&url, plainUrl)){
		SI32 i;
		for(i = (_listnrsAppOpenUrl->conteo - 1); i >= 0; i--){
			if(_listnrsAppOpenUrl->elem(i)->appOpenUrlImage(this, &url, rotFromIntended, usrData, usrDataSz)){
				r = true;
			}
		}
	}
	NBUrl_release(&url);
	return r;
}

bool AUApp::broadcastOpenFileData(const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppOpenUrl->conteo - 1); i >= 0; i--){
			if(_listnrsAppOpenUrl->elem(i)->appOpenFileData(this, data, dataSz, usrData, usrDataSz)){
				r = true;
			}
		}
	}
	return r;
}

bool AUApp::broadcastOpenFileImageData(const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppOpenUrl->conteo - 1); i >= 0; i--){
			if(_listnrsAppOpenUrl->elem(i)->appOpenFileImageData(this, data, dataSz, rotDegFromIntended, usrData, usrDataSz)){
				r = true;
			}
		}
	}
	return r;
}
		
bool AUApp::broadcastOpenBitmap(const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz){
	bool r = false;
	//Notify listeners
	{
		SI32 i;
		for(i = (_listnrsAppOpenUrl->conteo - 1); i >= 0; i--){
			if(_listnrsAppOpenUrl->elem(i)->appOpenBitmap(this, bmp, rotDegFromIntended, usrData, usrDataSz)){
				r = true;
			}
		}
	}
	return r;
}

//

#ifdef __ANDROID__
void AUApp::onWindowFocusChanged(void* pEnv /*JNIEnv*/, const SI32 hasFocus){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::onWindowFocusChanged")
	PRINTF_INFO("AUApp, onWindowFocusChanged(%d) ... and viewFocus(%d).\n", (hasFocus ? 1 : 0), (_jViewHasFocus ? 1 : 0));
	if(_jWindowHasFocus != hasFocus){
		const bool focusBef = (_jWindowHasFocus && _jViewHasFocus);
		_jWindowHasFocus = hasFocus;
		const bool focusNew = (_jWindowHasFocus && _jViewHasFocus);
		if(focusBef != focusNew){
			NBGestorTeclas::keyboardWinFocusChanged(focusNew);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif


#ifdef __ANDROID__
void AUApp::onFocusChange(void* pEnv /*JNIEnv*/, void* pView /*jobject*/, const SI32 hasFocus){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::onFocusChange")
	PRINTF_INFO("AUAppGlueAndroidKeyboard, onFocusChange(%d) ... and windowFocus(%d).\n", (hasFocus ? 1 : 0), (_jWindowHasFocus ? 1 : 0));
	if(_jViewHasFocus != hasFocus){
		const bool focusBef = (_jWindowHasFocus && _jViewHasFocus);
		_jViewHasFocus = hasFocus;
		const bool focusNew = (_jWindowHasFocus && _jViewHasFocus);
		if(focusBef != focusNew){
			NBGestorTeclas::keyboardWinFocusChanged(focusNew);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

#ifdef __ANDROID__
void AUApp::setKeyboardHeight(void* pEnv /*JNIEnv*/, float height, bool overlapsContent){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::setKeyboardHeight")
	PRINTF_INFO("AUAppGlueAndroidKeyboard, setKeyboardHeight(%f)%s.\n", height, overlapsContent ? "+overlapsContent" : "");
	NBGestorTeclas::establecerTecladoEnPantallaAlto(height, true);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

/*public boolean onTouchEvent(MotionEvent event) {
	int actionIndex = event.getActionIndex();
	int pointerId = event.getPointerId(actionIndex);
	int actionMask = event.getActionMasked();
	switch (actionMask) {
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN:
			_juego.touchIniciado(pointerId, (int)event.getX(actionIndex), (int)event.getY(actionIndex));
			break;
		case MotionEvent.ACTION_MOVE: // a pointer was moved
			_juego.touchMovido(pointerId, (int)event.getX(actionIndex), (int)event.getY(actionIndex));
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP:
		case MotionEvent.ACTION_CANCEL:
			_juego.touchFinalizado(pointerId, (int)event.getX(actionIndex), (int)event.getY(actionIndex), actionMask == MotionEvent.ACTION_CANCEL);
			break;
	}
	return true;
}*/

#ifdef __ANDROID__
bool AUApp::onTouchEvent(void* pEnv, void* pEvent){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::onTouchEvent")
	bool r = false;
	if(pEnv != NULL && pEvent != NULL){
		JNIEnv* jEnv = (JNIEnv*)pEnv;
		jobject jEvent = (jobject)pEvent;
		jclass clsMotEv	= jEnv->FindClass("android/view/MotionEvent"); NBASSERT(clsMotEv != NULL)
		if(clsMotEv != NULL){
			jmethodID mEvGetActIds	= jEnv->GetMethodID(clsMotEv, "getActionIndex", "()I"); NBASSERT(mEvGetActIds != NULL)
			jmethodID mEvGetPtrId	= jEnv->GetMethodID(clsMotEv, "getPointerId", "(I)I"); NBASSERT(mEvGetPtrId != NULL)
			jmethodID mEvGetActMask	= jEnv->GetMethodID(clsMotEv, "getActionMasked", "()I"); NBASSERT(mEvGetActMask != NULL)
			jmethodID mEvGetX	= jEnv->GetMethodID(clsMotEv, "getX", "(I)F"); NBASSERT(mEvGetX != NULL)
			jmethodID mEvGetY	= jEnv->GetMethodID(clsMotEv, "getY", "(I)F"); NBASSERT(mEvGetY != NULL)
			jfieldID fActDown	= jEnv->GetStaticFieldID(clsMotEv, "ACTION_DOWN", "I"); NBASSERT(fActDown != NULL)
			jfieldID fActPDown	= jEnv->GetStaticFieldID(clsMotEv, "ACTION_POINTER_DOWN", "I"); NBASSERT(fActPDown != NULL)
			jfieldID fActMove	= jEnv->GetStaticFieldID(clsMotEv, "ACTION_MOVE", "I"); NBASSERT(fActMove != NULL)
			jfieldID fActUp		= jEnv->GetStaticFieldID(clsMotEv, "ACTION_UP", "I"); NBASSERT(fActUp != NULL)
			jfieldID fActPUp	= jEnv->GetStaticFieldID(clsMotEv, "ACTION_POINTER_UP", "I"); NBASSERT(fActPUp != NULL)
			jfieldID fActCanc	= jEnv->GetStaticFieldID(clsMotEv, "ACTION_CANCEL", "I"); NBASSERT(fActCanc != NULL)
			if(mEvGetActIds != NULL && mEvGetPtrId != NULL && mEvGetActMask != NULL && mEvGetX != NULL && mEvGetY != NULL && fActDown != NULL && fActPDown != NULL && fActMove != NULL && fActUp != NULL && fActPUp != NULL && fActPUp != NULL && fActCanc != NULL){
				jint actIndx	= jEnv->CallIntMethod(jEvent, mEvGetActIds);
				jint ptrId		= jEnv->CallIntMethod(jEvent, mEvGetPtrId, actIndx);
				jint actMask	= jEnv->CallIntMethod(jEvent, mEvGetActMask);
				jfloat x		= jEnv->CallFloatMethod(jEvent, mEvGetX, actIndx);
				jfloat y		= jEnv->CallFloatMethod(jEvent, mEvGetY, actIndx);
				jint actDown	= jEnv->GetStaticIntField(clsMotEv, fActDown);
				jint actPDown	= jEnv->GetStaticIntField(clsMotEv, fActPDown);
				jint actMove	= jEnv->GetStaticIntField(clsMotEv, fActMove);
				jint actUp		= jEnv->GetStaticIntField(clsMotEv, fActUp);
				jint actPUp		= jEnv->GetStaticIntField(clsMotEv, fActPUp);
				jint actCanc	= jEnv->GetStaticIntField(clsMotEv, fActCanc);
				if(actMask == actDown || actMask == actPDown){
					if(NBGestorTouches::gestorInicializado()){
						NBGestorTouches::touchIniciar(ptrId, x, y);
					}
				} else if(actMask == actMove){
					if(NBGestorTouches::gestorInicializado()){
						NBGestorTouches::touchMover(ptrId, x, y);
					}
				} else if(actMask == actUp || actMask == actPUp  || actMask == actCanc){
					if(NBGestorTouches::gestorInicializado()){
						NBGestorTouches::touchFinalizar(ptrId, x, y, (actMask == actCanc));
					}
				}
				r = true;
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsMotEv)
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}
#endif

#ifdef __ANDROID__
bool AUApp::onKeyDown(void* pEnv /*JNIEnv*/, const SI32 keyCode, void* pEvent /*jobject*/){
	bool r = false;
	//if(pEnv != NULL){
		//JNIEnv* jEnv = (JNIEnv*)pEnv;
	//}
	return r;
}
#endif

/*if(keyCode == KeyEvent.KEYCODE_BACK) {
	if(_juego != null){
		if(_juego.teclaBackPresionada()){
			return true;
		} else {
			this._activity.onBackPressed();
			return true;
		}
	}
} else if(keyCode == KeyEvent.KEYCODE_MENU){
	if(_juego != null){
		if(_juego.teclaMenuPresionada()){
			return true;
		}
	}
}*/
 
#ifdef __ANDROID__
bool AUApp::onKeyUp(void* pEnv /*JNIEnv*/, const SI32 keyCode, void* pEvent /*jobject*/){
	bool r = false;
	if(pEnv != NULL){
		//https://developer.android.com/reference/android/view/KeyEvent.html
		NBASSERT((54 - 29) == ('Z' - 'A'))
		NBASSERT((16 - 7) == ('9' - '0'))
		if(keyCode >= 29 && keyCode <= 54){
			//'A' - 'Z'
			NBASSERT((54 - 29) == ('Z' - 'A'))
			char str[2] = { (char)('A' + (keyCode - 29)), '\0'};
			NBGestorTeclas::entradaLockForBatch();
			NBGestorTeclas::entradaIntroducirTexto(str, false);
			NBGestorTeclas::entradaUnlockFromBatch();
			r = true;
		} else if(keyCode >= 7 && keyCode <= 16){
			//'0' - '9'
			NBASSERT((16 - 7) == ('9' - '0'))
			char str[2] = { (char)('0' + (keyCode - 7)), '\0'};
			NBGestorTeclas::entradaLockForBatch();
			NBGestorTeclas::entradaIntroducirTexto(str, false);
			NBGestorTeclas::entradaUnlockFromBatch();
			r = true;
		} else {
			switch(keyCode) {
				case 4: //KEYCODE_BACK
					r = this->teclaEspecialPresionada(AU_TECLA_REGRESAR);
					break;
				case 5: //KEYCODE_CALL
					break;
				case 6: //KEYCODE_ENDCALL
					break;
				case 62: //KEYCODE_SPACE
					break;
				case 66: //KEYCODE_ENTER
					NBGestorTeclas::entradaLockForBatch();
					NBGestorTeclas::entradaIntroducirTexto("\n", false);
					NBGestorTeclas::entradaUnlockFromBatch();
					break;
				case 67: //KEYCODE_DEL
					NBGestorTeclas::entradaLockForBatch();
					NBGestorTeclas::entradaBackspace(false);
					NBGestorTeclas::entradaUnlockFromBatch();
					r = true;
					break;
				case 82: //KEYCODE_MENU
					r = this->teclaEspecialPresionada(AU_TECLA_MENU);
					break;
				default:
					break;
			}
		}
		/*JNIEnv* jEnv = (JNIEnv*)pEnv;
		jobject jEvent = (jobject)pEvent;
		jclass clsKeyEv	= jEnv->FindClass("android/view/KeyEvent"); NBASSERT(clsKeyEv != NULL)
		if(clsKeyEv != NULL){
			jfieldID fKeyBack	= jEnv->GetStaticFieldID(clsKeyEv, "KEYCODE_BACK", "I"); NBASSERT(fKeyBack != NULL)
			jfieldID fKeyMenu	= jEnv->GetStaticFieldID(clsKeyEv, "KEYCODE_MENU", "I"); NBASSERT(fKeyMenu != NULL)
			if(fKeyBack != NULL && fKeyMenu != NULL){
				const jint keyBack	= jEnv->GetStaticIntField(clsKeyEv, fKeyBack);
				const jint keyMenu	= jEnv->GetStaticIntField(clsKeyEv, fKeyMenu);
				if(keyCode == keyBack){
					r = this->teclaEspecialPresionada(AU_TECLA_REGRESAR);
				} else if(keyCode == keyMenu){
					r = this->teclaEspecialPresionada(AU_TECLA_MENU);
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsKeyEv)
		}*/
	}
	return r;
}
#endif


void AUApp::inicializarCallbacks(STAppCallbacks* callbacks){
	//Buffer
	callbacks->funcGenerarRenderBuffer		= NULL;
	callbacks->funcGenerarRenderBufferParam	= NULL;
	callbacks->funcVolcarBuffer				= NULL;
	callbacks->funcVolcarBufferParam		= NULL;
	//Audio
	callbacks->funcAudioActivateSession		= NULL;
	callbacks->funcAudioActivateSessionParam= NULL;
	callbacks->funcAudioDeactivateSession	= NULL;
	callbacks->funcAudioDeactivateSessionParam = NULL;
	callbacks->funcAudioUpdateTrackInfo		= NULL;
	callbacks->funcAudioUpdateTrackInfoParam = NULL;
	callbacks->funcAudioUpdateTrackState	= NULL;
	callbacks->funcAudioUpdateTrackStateParam = NULL;
}



bool AUApp::inicializarNucleo(const UI32 mascaraModulos){
	NBGestorAnimadores::inicializar(60.0f); //TODO: update animas freq to screen freq
#	if defined(__ANDROID__)
	{
		AUAppGlueAndroidJNI* glueJNI = AUApp::getDefaultGlueJNI(); NBASSERT(glueJNI != NULL)
		const STNBAndroidJniItf jniItf = glueJNI->getAndroidJniItf();
		PRINTF_INFO("NBGestorArchivos::inicializar-prev glueJNI(%lld).\n", (UI64)glueJNI);
		NBGestorArchivos::inicializar(&jniItf, glueJNI);
		PRINTF_INFO("NBGestorArchivos::inicializar-after.\n");
	}
#	else
	NBGestorArchivos::inicializar();
#	endif
	if((mascaraModulos & AUAPP_BIT_MODULO_RED) != 0){
		NBGestorRed::inicializar();
		_modulosActivos |= AUAPP_BIT_MODULO_RED;
	}
	NBGestorHilos::inicializar();
	NBMngrNotifs::init();
	NBMngrAVCapture::init();
	NBMngrOSTools::init();
	NBMngrOSSecure::init();
	NBMngrStore::init();
	NBMngrContacts::init();
	NBMngrOSTelephony::init();
	NBMngrBiometrics::init();
	NBMngrPdfKit::init();
	return true;
}

void AUApp::finalizarNucleo(){
	NBMngrPdfKit::finish();
	NBMngrBiometrics::finish();
	NBMngrOSTelephony::finish();
	NBMngrContacts::finish();
	NBMngrStore::finish();
	NBMngrAVCapture::finish();
	NBMngrOSSecure::finish();
	NBMngrOSTools::finish();
	NBMngrNotifs::finish();
	NBGestorHilos::finalizar();
	if((_modulosActivos & AUAPP_BIT_MODULO_RED) != 0){
		NBGestorRed::finalizar();
		_modulosActivos &= ~AUAPP_BIT_MODULO_RED;
	}
	NBGestorArchivos::finalizar();
	if(NBGestorAnimadores::gestorInicializado()){
		NBGestorAnimadores::finalizar();
	}
}

bool AUApp::inicializarMultimedia(const bool leerPrecache, const bool leerCache, const bool escribirCache, const bool initGraphics, const float pantallaFrecuencia){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::inicializarMultimedia")
	bool exito = false;
	AUCadenaMutable8* rutaTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	//Start GL-independent components
	{
		if(leerCache || escribirCache){
			rutaTmp->vaciar(); rutaTmp->agregar(NBGestorArchivos::rutaHaciaRecursoEnCacheLogica("Fuentes/"));
			NBGestorArchivos::crearCarpeta(rutaTmp->str());
			rutaTmp->vaciar(); rutaTmp->agregar(NBGestorArchivos::rutaHaciaRecursoEnCacheLogica("Animaciones/"));
			NBGestorArchivos::crearCarpeta(rutaTmp->str());
			rutaTmp->vaciar(); rutaTmp->agregar(NBGestorArchivos::rutaHaciaRecursoEnCacheLogica("Cuerpos/"));
			NBGestorArchivos::crearCarpeta(rutaTmp->str());
		}
		//Filesystem and fonts
		NBGestorFuentes::inicializar();
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		NBGestorSonidos::inicializar(0, 60);
		NBGestorSonidos::establecerPrefijoRutas("Sonidos/", ENAudioTipo_Sonidos);
		NBGestorSonidos::establecerPrefijoRutas("SonidosAud/", ENAudioTipo_SonidosAud);
		NBGestorSonidos::establecerPrefijoRutas("SonidosOgg/", ENAudioTipo_SonidosOgg);
		NBGestorSonidos::establecerPrefijoRutas("Musica/", ENAudioTipo_Musica);
		NBGestorSonidos::establecerPrefijoRutas("MusicaAud/", ENAudioTipo_MusicaAud);
		NBGestorSonidos::establecerPrefijoRutas("MusicaOgg/", ENAudioTipo_MusicaOgg);
#		endif
	}
	//
	if(!initGraphics){
		exito = true;
	} else {
		if(!NBGestorGL::inicializar()){
			PRINTF_ERROR("AUApp, NBGestorGL::inicializar() failed.\n");
		} else {
			AUCargadorCuerpos* intermediarioCargaCuerpos			= new(ENMemoriaTipo_Nucleo) AUCargadorCuerpos();
			AUCargadorAnimaciones* intermediarioCargaAnimaciones	= new(ENMemoriaTipo_Nucleo) AUCargadorAnimaciones();
			NBGestorAnimaciones::inicializar(leerPrecache, leerCache, escribirCache, intermediarioCargaCuerpos); NBGestorAnimaciones::establecerPrefijoRutas("Animaciones/", "Animaciones/");
			NBGestorPersonajes::inicializar();
			NBGestorTexturas::inicializar(ENTexturaModoHilo_MultiHilo, 1024, 512);
			NBGestorEscena::inicializar(pantallaFrecuencia);
			NBGestorCuerpos::inicializar(leerPrecache, leerCache, escribirCache, intermediarioCargaAnimaciones);	NBGestorCuerpos::establecerPrefijoRutas("Cuerpos/", "Cuerpos/");
			NBMngrFonts::init();
#			ifndef CONFIG_NB_UNSUPPORT_BOX2D
			NBGestorFisica::inicializar(2, 2);
#			endif
			//NBGestorNiveles::inicializar(leerPrecache, leerCache, escribirCache);	NBGestorNiveles::establecerPrefijoRutas("Niveles/", "Niveles/");
			intermediarioCargaCuerpos->liberar(NB_RETENEDOR_NULL);
			intermediarioCargaAnimaciones->liberar(NB_RETENEDOR_NULL);
			exito = true;
		}
	}
	rutaTmp->liberar(NB_RETENEDOR_NULL);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return exito;
}

SI32 AUApp::conteoGestoresInicializados(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::conteoGestoresInicializados")
	SI32 r = 0;
	//En constructor AUApp
	if(NBGestorIdioma::gestorInicializado()) r++;
	if(NBGestorTeclas::gestorInicializado()) r++;
	if(NBGestorTouches::gestorInicializado()) r++;
	if(NBGestorDatos::gestorInicializado()) r++;
	//EnNucleo
	if(NBGestorArchivos::gestorInicializado()) r++;
	if(NBGestorRed::gestorInicializado()) r++;
	if(NBGestorHilos::gestorInicializado()) r++;
	if(NBGestorFuentes::gestorInicializado()) r++;
	//En Graficos
	if(NBGestorGL::gestorInicializado()) r++;
	if(NBGestorAnimadores::gestorInicializado()) r++;
	if(NBGestorAnimaciones::gestorInicializado()) r++;
	if(NBGestorPersonajes::gestorInicializado()) r++;
	if(NBGestorTexturas::gestorInicializado()) r++;
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	if(NBGestorSonidos::gestorInicializado()) r++;
#	endif
	if(NBGestorEscena::gestorInicializado()) r++;
	if(NBGestorCuerpos::gestorInicializado()) r++;
#	ifndef CONFIG_NB_UNSUPPORT_BOX2D
	if(NBGestorFisica::gestorInicializado()) r++;
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}
	
UI32 AUApp::liberarRecursosSinUso(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::liberarRecursosSinUso")
	UI32 r = 0;
	while(1){
		UI32 conteoLiberados = 0;
		AUApp::aplicarAutoliberaciones();
		if(NBGestorCuerpos::gestorInicializado()){
			conteoLiberados += NBGestorCuerpos::liberarPlantillasSinReferencias();
		}
		if(NBGestorAnimaciones::gestorInicializado()){
			conteoLiberados += NBGestorAnimaciones::liberarAnimacionesSinReferencias();
		}
		if(NBGestorTexturas::gestorInicializado()){
			conteoLiberados += NBGestorTexturas::liberarTexturasSinReferencias();
		}
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		if(NBGestorSonidos::gestorInicializado()){
			conteoLiberados += NBGestorSonidos::liberarBufferesSinReferencias();
		}
#		endif
		//conteoLiberados +=  //PEDIENTE: aplicar autoliberaciones
		if(conteoLiberados == 0) break;
		r += conteoLiberados;
	};
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}
	
bool AUApp::reinicializarMultimedia(const bool elimObjetosAnteriores){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::reinicializarMultimedia")
	bool r = false;
	PRINTF_INFO("AUApp::reinicializarMultimedia()\n");
	if(NBGestorGL::gestorInicializado()){
		NBGestorGL::reiniciarContextoGrafico(elimObjetosAnteriores);
	}
	if(NBGestorTexturas::gestorInicializado()){
		NBGestorTexturas::reiniciarContextoGrafico(elimObjetosAnteriores);
	}
	if(NBGestorEscena::gestorInicializado()){
		NBGestorEscena::reiniciarContextoGrafico(elimObjetosAnteriores);
	}
	//Force buffers to be filled (content render)
	this->_ticksWithSameContent = 0;
#	ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "AU", "reinicializarMultimedia");
#	endif
	//
	r = true;
	return r;
}

void AUApp::finalizarMultimedia(){
	//LIBERAR RECURSOS SIN USO
	AUApp::aplicarAutoliberaciones();
	//LIBERAR GESTORES INICIALIZADOS JUNTOS A LOS GRAFICOS
	if(NBGestorEscena::gestorInicializado()){
		NBGestorEscena::liberarRecursosCacheRenderEscenas();
	}
	AUApp::aplicarAutoliberaciones();
	UI16 conteoLiberados;
	do {
		conteoLiberados = 0;
		//if(NBGestorNiveles::gestorInicializado()){
		//conteoLiberados += NBGestorNiveles::liberarPlantillasSinReferencias();
		//}
		if(NBGestorCuerpos::gestorInicializado()){
			conteoLiberados += NBGestorCuerpos::liberarPlantillasSinReferencias();
		}
		if(NBGestorAnimaciones::gestorInicializado()){
			conteoLiberados += NBGestorAnimaciones::liberarAnimacionesSinReferencias();
		}
		if(NBGestorTexturas::gestorInicializado()){
			conteoLiberados += NBGestorTexturas::liberarTexturasSinReferencias();
		}
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		if(NBGestorSonidos::gestorInicializado()){
			conteoLiberados += NBGestorSonidos::liberarBufferesSinReferencias();
		}
#		endif
		AUApp::aplicarAutoliberaciones();
	} while(conteoLiberados!=0);
	//
	//#ifdef CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES
	//PRINTF_INFO("-------------------------------------\n");
	//PRINTF_INFO("PUNTEROS EN USO AL LIBERAR RECURSOS\n");
	//NBGestorMemoria::debug_imprimePunterosEnUso();
	//PRINTF_INFO("FIN DE LISTA ------------------------\n");
	//#endif
	if(NBMngrFonts::isInited()){
		NBMngrFonts::finish();
	}
	if(NBGestorEscena::gestorInicializado()){
		NBGestorEscena::finalizar();
	}
	//if(NBGestorNiveles::gestorInicializado()){
	//NBGestorNiveles::finalizar();
	//}
	if(NBGestorPersonajes::gestorInicializado()){
		AUApp::aplicarAutoliberaciones();
		NBGestorPersonajes::finalizar();
	}
	if(NBGestorAnimaciones::gestorInicializado()){
		AUApp::aplicarAutoliberaciones();
		NBGestorAnimaciones::finalizar();
	}
	if(NBGestorCuerpos::gestorInicializado()){
		NBGestorCuerpos::finalizar();
	}
#	ifndef CONFIG_NB_UNSUPPORT_BOX2D
	if(NBGestorFisica::gestorInicializado()){
		NBGestorFisica::finalizar();
	}
#	endif
	if(NBGestorTexturas::gestorInicializado()){
		AUApp::aplicarAutoliberaciones();
		NBGestorTexturas::finalizar();
	}
	if(NBGestorGL::gestorInicializado()){
		NBGestorGL::finalizar();
	}
	//
#	ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
	if(NBGestorSonidos::gestorInicializado()){
		AUApp::aplicarAutoliberaciones();
		NBGestorSonidos::liberarBufferesSinReferencias();
		NBGestorSonidos::finalizar();
	}
#	endif
	if(NBGestorFuentes::gestorInicializado()){
		NBGestorFuentes::finalizar();
	}
}

/*float AUApp::configEscalaHaciaHD(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::configEscalaHaciaHD")
	float r = _almacenConfig->valor("escalaHD", 1.0f);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}*/

/*bool AUApp::configEscalaHaciaHDEsManual(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::configEscalaHaciaHDEsManual")
	bool r = _almacenConfig->valor("escalaHDEsManual", false);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}*/

/*const char* AUApp::configIdioma(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::configIdioma")
	const char* r = _almacenConfig->valor("idioma", "");
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}*/

/*bool AUApp::configIdiomaEsManual(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::configIdiomaEsManual")
	bool r = _almacenConfig->valor("idiomaEsManual", false);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}*/

/*void AUApp::aplicarConfiguracion(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::aplicarConfiguracion")
	AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
	//Escena
	NBGestorEscena::establecerAyudasParaPantallasPequenas(_almacenConfig->valor("magAyudaAct", true));
	NBGestorEscena::pantallaEstablecerPermitidoRotar(_almacenConfig->valor("rotarPantAct", false));
	NBGestorEscena::establecerCuadrosPorSegundo(_almacenConfig->valor("fpsDef", 60.0f));
	NBGestorEscena::establecerLucesSombrasActivos(_almacenConfig->valor("luzSombAct", true));
	_escalaHaciaHD				= _almacenConfig->valor("escalaHD", 1.0f);
	_escalaHaciaHDEsManual		= _almacenConfig->valor("escalaHDEsManual", false);
	//Idioma
	SI32 iPriodidad;
	for(iPriodidad=0; iPriodidad<ENIdioma_Conteo; iPriodidad++){
		strTmp->vaciar(); strTmp->agregarConFormato("idioma%d", iPriodidad);
		SI32 idiomaEnPrioridad = _almacenConfig->valor(strTmp->str(), -1);
		if(idiomaEnPrioridad>=0 && idiomaEnPrioridad<ENIdioma_Conteo){
			NBGestorIdioma::establecerPrioridadIdioma(iPriodidad, (ENIdioma)idiomaEnPrioridad);
		}
	}
	//Establecer idioma manual
	bool idiomaEsManual = _almacenConfig->valor("idiomaEsManual", false);
	if(idiomaEsManual){
		const char* strIdioma = _almacenConfig->valor("idioma", "");
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
	//Sonidos
	//NBGestorSonidos::establecerGrupoAudioHabilitado(ENAudioGrupo_Efectos, _almacenConfig->valor("sndFX", true));
	//NBGestorSonidos::establecerGrupoAudioHabilitado(ENAudioGrupo_Musica, _almacenConfig->valor("sndAmb", true));
	//NBGestorSonidos::establecerGrupoAudioMultiplicadorVolumen(ENAudioGrupo_Efectos, _almacenConfig->valor("volFX", 0.5f));
	//NBGestorSonidos::establecerGrupoAudioMultiplicadorVolumen(ENAudioGrupo_Musica, _almacenConfig->valor("volAmb", 0.9f));
	//
	strTmp->liberar(NB_RETENEDOR_THIS);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}*/

BOOL AUApp::privConfigureForResolution(const NBTamanoI wSize, const NBTamano ppiScreen, const NBTamano dpiScene){
	BOOL r = FALSE;
	if(NBGestorTexturas::gestorInicializado()){
		NBASSERT(ppiScreen.ancho > 0.0f && ppiScreen.alto > 0.0f)
		NBASSERT(dpiScene.ancho > 0.0f && dpiScene.alto > 0.0f)
		const float pppScreenAvg = (ppiScreen.ancho + ppiScreen.alto) / 2.0f;
		const float pppSceneAvg = (dpiScene.ancho + dpiScene.alto) / 2.0f;
		//Monitor LCD: 100.5dpi
		//iPad: 132dpi (~800K pixeles)
		//iPhone & iPodTouch & iPadMini: 163dpi
		//iPhone & iPodTouch Retina & iPadMini2: 326dpi (~600K - ~750K pixeles)
		//iPhone6 Plus: 401dpi (1920 x 1080)
		//iPad Retina: 264dpi (~3M pixeles)
		//LG-G3: 530dpi
		//
		// Por cant pixeles:  iPhone: 153,600px, iPhoneRetina: 614,400px, iPhoneRetina4In: 727,040px, iPad: 786,432px, iPadRetina: 3,145,728
		// Por tamMin:        iPhone: 320(200%), iPhoneRetina: 640(100%), iPhoneRetina4In: 640(100%), iPad: 768(78.%), iPadRetina 1536(41%)
		//
		//Generar la pantalla
		_escalaHaciaHD = 1.0f; //La configuracion manual tiene prioridad sobre las propiedadesd el dispositivo
		PRINTF_INFO("PANTALLA-NATIVA TAMANO(%u x %u) DENSIDAD(%f x %f) PIXELES(%d).\n", wSize.ancho, wSize.alto, ppiScreen.ancho, ppiScreen.alto, (wSize.ancho * wSize.alto));
		//Ruta de texturas (escala escena)
		if(pppSceneAvg < 100.0f){
			PRINTF_INFO("ESCALA HACIA ESCENA HD (PNG/X1): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppSceneAvg);
			NBGestorTexturas::establecerTamanosTexturasDeAmbito(NBGestorTexturas::indiceAmbitoTexturasActual(), 512, 512);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X1/", 1, 1.0, ENTexturaResolucion_Escena);
		} else if(pppSceneAvg < 195.0f){ //190, because some devices are 200 x 199 dpi (do not validate against 200 dpi).
			PRINTF_INFO("ESCALA HACIA ESCENA HD (PNG/X2): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppSceneAvg);
			NBGestorTexturas::establecerTamanosTexturasDeAmbito(NBGestorTexturas::indiceAmbitoTexturasActual(), 1024, 512);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X2/", 2, 1.0f, ENTexturaResolucion_Escena);
		} else if(pppSceneAvg < 395.0f){ //390, because some devices are 400 x 399 dpi (do not validate against 400 dpi).
			PRINTF_INFO("ESCALA HACIA ESCENA HD (PNG/X4): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppSceneAvg);
			NBGestorTexturas::establecerTamanosTexturasDeAmbito(NBGestorTexturas::indiceAmbitoTexturasActual(), 2048, 1024);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X4/", 4, 1.0f, ENTexturaResolucion_Escena);
		} else {
			PRINTF_INFO("ESCALA HACIA ESCENA HD (PNG/X8): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppSceneAvg);
			NBGestorTexturas::establecerTamanosTexturasDeAmbito(NBGestorTexturas::indiceAmbitoTexturasActual(), 4096, 2048);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X8/", 8, 1.0f, ENTexturaResolucion_Escena);
		}
		//Ruta de texturas (escala pantalla)
		if(pppScreenAvg < 100.0f){
			PRINTF_INFO("ESCALA HACIA PANTALLA (PNG/X1): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppScreenAvg);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X1/", 1, 1.0, ENTexturaResolucion_Pantalla);
		} else if(pppScreenAvg < 195.0f){ //190, because some devices are 200 x 199 dpi (do not validate against 200 dpi).
			PRINTF_INFO("ESCALA HACIA PANTALLA (PNG/X2): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppScreenAvg);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X2/", 2, 1.0f, ENTexturaResolucion_Pantalla);
		} else if(pppScreenAvg < 395.0f){ //390, because some devices are 400 x 399 dpi (do not validate against 400 dpi).
			PRINTF_INFO("ESCALA HACIA PANTALLA (PNG/X4): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppScreenAvg);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X4/", 4, 1.0f, ENTexturaResolucion_Pantalla);
		} else {
			PRINTF_INFO("ESCALA HACIA PANTALLA (PNG/X8): %f (%s, %.1f dpi)\n", _escalaHaciaHD, _escalaHaciaHDEsManual ? "manual" : "automatica", pppScreenAvg);
			NBGestorTexturas::establecerPrefijoRutas("PNG/X8/", 8, 1.0f, ENTexturaResolucion_Pantalla);
		}
		//Fonts
		{
			NBGestorFuentes::establecerPrefijoRutasFuentes("Fuentes/", "Fuentes/");
			//
			NBMngrFonts::setPPI(NBST_P(STNBSize, dpiScene.ancho / _escalaHaciaHD, dpiScene.alto / _escalaHaciaHD));
			//NBMngrFonts::updateFonts();
		}
		r = TRUE;
	}
	return r;
}

bool AUApp::inicializarVentana(const NBTamanoI wSize, const NBTamano ppiScreen, const NBTamano dpiScene, const ENGestorEscenaDestinoGl destinoGL){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::inicializarVentana")
	bool r = false;
	if(this->privConfigureForResolution(wSize, ppiScreen, dpiScene)){
		if(NBGestorEscena::gestorInicializado()){
			//(altoPulgadas * _escenas[iEscena].puntosPorPulgada.alto) * _escenas[iEscena].escalaParaHD;
			PRINTF_INFO("AUApp::inicializarVentana wSize(%d, %d) ppiScreen(%f, %f) dpiScene(%f, %f).\n", wSize.ancho, wSize.alto, ppiScreen.ancho, ppiScreen.alto, dpiScene.ancho, dpiScene.alto);
			_indiceEscenaRender = NBGestorEscena::crearEscena(wSize.ancho, wSize.alto, _escalaHaciaHD, /*0.5f*/1.0f, ppiScreen.ancho, ppiScreen.alto, dpiScene.ancho, dpiScene.alto, COLOR_RGBA8, destinoGL, _callbacks.funcGenerarRenderBuffer, _callbacks.funcGenerarRenderBufferParam); //PRINTF_INFO("ESCENA PRINCIPAL tiene indice (%d)\n", _indiceEscenaRender);
			r = true;
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}

void AUApp::finalizarVentana(){
	if(_indiceEscenaRender != -1){
		if(NBGestorEscena::gestorInicializado()){
			NBGestorEscena::liberarEscena(_indiceEscenaRender);
		}
		_indiceEscenaRender = -1;
	}
}

bool AUApp::inicializarJuego(AUAppEscenasAdmin* gameplays){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::inicializarJuego")
	bool exito = false;
	//
	if(gameplays != NULL){
		NBASSERT(gameplays->esClase(AUAppEscenasAdmin::idTipoClase))
		gameplays->retener(NB_RETENEDOR_THIS);
	}
	if(_gameplays != NULL) _gameplays->liberar(NB_RETENEDOR_THIS);
	_gameplays = gameplays;
	//Resumen debug
	if(DEBUG_RESUMEN_CREAR){
		_resumenDebug	= new(ENMemoriaTipo_Nucleo) AUEscenaResumenDebug(_indiceEscenaRender);
		NBColor8 colorCapa;	NBCOLOR_ESTABLECER(colorCapa, 255, 255, 255, 255)
		NBGestorEscena::agregarObjetoCapa(_indiceEscenaRender, ENGestorEscenaGrupo_DebugFrontal, _resumenDebug, colorCapa);
	}
	//Timer para acumular datos de depuracion/optimizacion
	if(_gameplays != NULL){
		//SINCRONIZACION DE HILOS
		_datosHilos.hiloPrincipalIniciado	= false;
		_datosHilos.app						= this;
		_datosHilos.finalizandoPrograma		= false;
		_datosHilos.modoRenderSecuencial	= false;
		_datosHilos.renderbufferLlenado		= false;
		{
			_datosHilos.redim.stackUse			= 0;
			_datosHilos.redim.size.ancho		= 0;
			_datosHilos.redim.size.alto			= 0;
			_datosHilos.redim.sceneScale		= 0.0f;
			_datosHilos.redim.ppiScreen.ancho	= 0;	//pixel-per-inch
			_datosHilos.redim.ppiScreen.alto	= 0;	//pixel-per-inch
			_datosHilos.redim.dpiScene.ancho	= 0;	//dot-per-inch
			_datosHilos.redim.dpiScene.alto		= 0;	//dot-per-inch
		}
		#if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
		_datosHilos.hiloProductorEscenaTrabajando = false;
		APPHILO_MUTEX_INICIALIZAR(&_datosHilos.mutexBufferesEscena);
		APPHILO_MUTEX_INICIALIZAR(&_datosHilos.mutexEsperaProducirEscena);
		APPHILO_MUTEX_INICIALIZAR(&_datosHilos.mutexEsperaConsumirEscena);
		APPHILO_COND_INICIALIZAR(&_datosHilos.condEsperaProducirEscena);
		APPHILO_COND_INICIALIZAR(&_datosHilos.condEsperaConsumirEscena);
		#endif
		#if defined(APP_IMPLEMENTAR_TRI_HILO)
		_datosHilos.ticksCargandoRecursos			= 0;
		_datosHilos.hiloConsumidorEscenaTrabajando	= false;
		APPHILO_MUTEX_INICIALIZAR(&_datosHilos.mutexEsperaConsumirRender);
		APPHILO_COND_INICIALIZAR(&_datosHilos.condEsperaConsumirRender);
		#endif
		//CREAR HILOS DE RENDERIZADO Y ANIMACION
		#if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
		_datosHilos.hiloProductorEscena = new AUHilo("AUApp::hiloProductorEscena");
		if(!_datosHilos.hiloProductorEscena->crearHiloYEjecuta(funcProductorDatosEscena, &_datosHilos)){
			PRINTF_ERROR("crearHiloYEjecuta, no se pudo crear el hilo PRODUCTOR-ESCENA\n");
			NBASSERT(false) //no se pudo crear el hilo de animacion
		} else {
			PRINTF_INFO("HILO PRODUCTOR-ESCENA creado\n");
		}
		#endif
		#if defined(APP_IMPLEMENTAR_TRI_HILO)
		_datosHilos.hiloConsumidorEscena = new AUHilo("AUApp::hiloConsumidorEscena");
		if(_datosHilos.hiloConsumidorEscena->crearHiloYEjecuta(funcConsumidorDatosEscena, &_datosHilos)){
			PRINTF_ERROR("crearHiloYEjecuta, no se pudo crear el hilo CONSUMIDOR-ESCENA\n");
			NBASSERT(false) //no se pudo crear el hilo de animacion
		} else {
			PRINTF_INFO("HILO CONSUMIDOR-ESCENA creado\n");
		}
		#endif
		_datosHilos.hiloPrincipalIniciado		= true;
		hiloIniciado();
		exito = true;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return exito;
}

void AUApp::finalizarJuego(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::finalizarJuego")
	//Esperar los hilos
	_datosHilos.finalizandoPrograma = true;
#	if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	while(_datosHilos.hiloProductorEscenaTrabajando){
		_datosHilos.finalizandoPrograma = true;
		NBHILO_SLEEP_MS(50);
	}
	if(_datosHilos.hiloProductorEscena != NULL) _datosHilos.hiloProductorEscena->liberar(NB_RETENEDOR_NULL); _datosHilos.hiloProductorEscena = NULL;
#	endif
#	if defined(APP_IMPLEMENTAR_TRI_HILO)
	while(_datosHilos.hiloConsumidorEscenaTrabajando){
		_datosHilos.finalizandoPrograma = true;
		NBHILO_SLEEP_MS(50);
	}
	if(_datosHilos.hiloConsumidorEscena != NULL) _datosHilos.hiloConsumidorEscena->liberar(NB_RETENEDOR_NULL); _datosHilos.hiloConsumidorEscena = NULL;
#	endif
	//Liberar recursos
	if(_gameplays != NULL){
		_gameplays->liberar(NB_RETENEDOR_THIS);
		_gameplays = NULL;
	}
	if(_resumenDebug != NULL){
		if(_indiceEscenaRender != -1){
			NBGestorEscena::quitarObjetoCapa(_indiceEscenaRender, _resumenDebug);
		}
		_resumenDebug->liberar(NB_RETENEDOR_THIS);
		_resumenDebug = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

//
	
void AUApp::audioActivarSesion(){
	if(_callbacks.funcAudioActivateSession != NULL){
		(*_callbacks.funcAudioActivateSession)(_callbacks.funcAudioActivateSessionParam);
	}
}
	
void AUApp::audioDesactivarSesion(){
	if(_callbacks.funcAudioDeactivateSession != NULL){
		(*_callbacks.funcAudioDeactivateSession)(_callbacks.funcAudioDeactivateSessionParam);
	}
}
	
void AUApp::audioUpdateTrackInfo(){
	if(_callbacks.funcAudioUpdateTrackInfo != NULL){
		(*_callbacks.funcAudioUpdateTrackInfo)(_callbacks.funcAudioUpdateTrackInfoParam);
	}
}
	
void AUApp::audioUpdateTrackState(){
	if(_callbacks.funcAudioUpdateTrackState != NULL){
		(*_callbacks.funcAudioUpdateTrackState)(_callbacks.funcAudioUpdateTrackStateParam);
	}
}

//
	
SI32 AUApp::indiceEscenaRender(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::indiceEscenaRender")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _indiceEscenaRender;
}

float AUApp::escalaHaciaHD(){
	return _escalaHaciaHD;
}
	
AUAppEscenasAdmin* AUApp::gameplays(){
	return _gameplays;
}

//

void AUApp::tickPerSecThreadFunc(void* param){
	AUApp* obj = (AUApp*)param;
	SI32 msAcum = 0;
	while(!obj->_tickPerSecThread->cancelado()){
		//Accum
		NBHILO_SLEEP_MS(50)
		msAcum += 50;
		//Execute
		if(msAcum > 1000 && !obj->_tickPerSecThread->cancelado()){
			NBHILO_MUTEX_ACTIVAR(&obj->_tickMutex);
			if(obj->_juegoCargado){
				obj->_estadisticas.ultFramesAnimEnSeg	= obj->_estadisticas.ticksAnimEnSegAcum;
				obj->_estadisticas.ultFramesRenderEnSeg	= obj->_estadisticas.ticksRenderEnSegAcum;
				obj->_estadisticas.ticksAnimEnSegAcum	= 0;
				obj->_estadisticas.ticksRenderEnSegAcum	= 0;
				//PRINTF_INFO("TickSeg anim(%d) render(%d).\n", obj->_estadisticas.ultFramesAnimEnSeg, obj->_estadisticas.ultFramesRenderEnSeg);
				//
				if(obj->_resumenDebug != NULL){
					CICLOS_CPU_TIPO ciclos; CICLOS_CPU_HILO(ciclos)
					obj->_resumenDebug->tickSegundoRecopilarInformacion(ciclos, DEBUG_RESUMEN_MOSTRAR_DATOS_TEXTO, DEBUG_RESUMEN_MOSTRAR_DATOS_GRAFICOS, DEBUG_RESUMEN_IMPRIMIR_EN_CONSOLA);
				}
			}
			NBHILO_MUTEX_DESACTIVAR(&obj->_tickMutex);
			msAcum = 0;
		}
	}
}

float AUApp::tickJuego(const ENAUAppTickTipo tipoTick, const bool acumularFramesSaltadas){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::tickJuego")
	float msSobrantes = 0.0f;
	UI32 tickCallsCur = 0;
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		NBASSERT(_tickCallsCur >= 0)
		tickCallsCur = ++_tickCallsCur;
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	//Execute if this is the only active call
	if(tickCallsCur == 1){
		if(tipoTick == ENAUAppTickTipo_SincPantalla){
			/*if(acumularFramesSaltadas){
			 struct timeval tiempoIni; gettimeofday(&tiempoIni, NULL);
			 if(tiempoIni.tv_sec >= _usTickAnterior.tv_sec){
			 const time_t segsDiff	= (tiempoIni.tv_sec - _usTickAnterior.tv_sec);
			 if(segsDiff < 2) msEsteTick	= ((float)segsDiff * 1000.0f) + (((float)tiempoIni.tv_usec - (float)_usTickAnterior.tv_usec) / 1000.0f);
			 }
			 _usTickAnterior = tiempoIni;
			 }*/
#			if defined(APP_IMPLEMENTAR_TRI_HILO)
			funcConsumidorDatosRender(&_datosHilos);
#			elif defined(APP_IMPLEMENTAR_BI_HILO)
			funcConsumidorDatosEscena(&_datosHilos);
			funcConsumidorDatosRender(&_datosHilos);
#			else
			funcProductorDatosEscena(&_datosHilos);
			funcConsumidorDatosEscena(&_datosHilos);
			funcConsumidorDatosRender(&_datosHilos);
#			endif
		} else /*if(tipoTick == ENAUAppTickTipo_TimerManual)*/{
			CICLOS_CPU_TIPO cicloIni; CICLOS_CPU_HILO(cicloIni)
#			if defined(APP_IMPLEMENTAR_TRI_HILO)
			funcConsumidorDatosRender(&_datosHilos);
#			elif defined(APP_IMPLEMENTAR_BI_HILO)
			funcConsumidorDatosEscena(&_datosHilos);
			funcConsumidorDatosRender(&_datosHilos);
#			else
			funcProductorDatosEscena(&_datosHilos);
			funcConsumidorDatosEscena(&_datosHilos);
			funcConsumidorDatosRender(&_datosHilos);
#			endif
			CICLOS_CPU_TIPO cicloFin; CICLOS_CPU_HILO(cicloFin)
			CICLOS_CPU_TIPO ciclosCpuPorSeg; CICLOS_CPU_POR_SEGUNDO(ciclosCpuPorSeg);
			const float msPorTick	= (1000.0f / (float)NBGestorAnimadores::ticksPorSegundo());
			const float msEsteTick	= ((float)(cicloFin - cicloIni) * 1000.0f) / (float)ciclosCpuPorSeg;
			//PRINTF_INFO("Tick ha consumido: %.3f msegs (el %.1f%%).\n", msEsteTick, 100.0f * msEsteTick / msPorTick );
			msSobrantes = (msPorTick - msEsteTick);
		}
		/*float msSobrante			= (msPorTick - msEsteTick);
		 if(msSobrante < 0.0f && acumularFramesSaltadas){
		 _framesSeguidasEnRetraso++;
		 if(_framesSeguidasEnRetraso>30){
		 _framesAcumRetraso += (-msSobrante / msPorTick);
		 if(_framesAcumRetraso>2.0f) _framesAcumRetraso = 2.0f; //Maximo acumular dos ticks
		 }
		 } else {
		 _framesSeguidasEnRetraso	= 0;
		 _framesAcumRetraso			= 0.0f;
		 }*/
		//if(msSobrante<0.0f) PRINTF_INFO("msPorTick(%f) msEsteTick(%f) msSobrante(%f) _framesAcumRetraso(%f)\n", msPorTick, msEsteTick, (msPorTick - msEsteTick), _framesAcumRetraso);
	}
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		_tickCallsCur--; NBASSERT(_tickCallsCur >= 0)
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return msSobrantes;
}

void AUApp::tickSoloSonidos(const float segsTranscurridos){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::tickSoloSonidos")
	UI32 tickCallsCur = 0;
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		NBASSERT(_tickCallsCur >= 0)
		tickCallsCur = ++_tickCallsCur;
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	//Execute if this is the only active call
	if(tickCallsCur == 1){
#		if defined(APP_IMPLEMENTAR_TRI_HILO)
		NBASSERT(false)
#		elif defined(APP_IMPLEMENTAR_BI_HILO)
		NBASSERT(false)
#		else
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		if(NBGestorSonidos::gestorInicializado()){
			NBGestorSonidos::tickSonido(segsTranscurridos);
		}
#		endif
		NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE() //Verify pointers before tick
		NBGestorAnimadores::difundeTick(segsTranscurridos, ENGestorAnimadoresGrupo_Nucleo);
		NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE() //Verify pointers after tick
		AUApp::aplicarAutoliberaciones();
		NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE() //Verify pointers after autoreleases
#		endif
	}
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		_tickCallsCur--; NBASSERT(_tickCallsCur >= 0)
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
	
void AUApp::tickSoloProducirRender(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::tickSoloProducirRender")
	UI32 tickCallsCur = 0;
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		NBASSERT(_tickCallsCur >= 0)
		tickCallsCur = ++_tickCallsCur;
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	//Execute if this is the only active call
	if(tickCallsCur == 1){
#		if defined(APP_IMPLEMENTAR_TRI_HILO)
		NBASSERT(false)
#		elif defined(APP_IMPLEMENTAR_BI_HILO)
		NBASSERT(false)
#		else
		funcProductorDatosEscena(&_datosHilos);
#		endif
	}
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		_tickCallsCur--; NBASSERT(_tickCallsCur >= 0)
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::tickSoloConsumirRender(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::tickSoloConsumirRender")
	UI32 tickCallsCur = 0;
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		NBASSERT(_tickCallsCur >= 0)
		tickCallsCur = ++_tickCallsCur;
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	//Execute if this is the only active call
	if(tickCallsCur == 1){
#		if defined(APP_IMPLEMENTAR_TRI_HILO)
		NBASSERT(false)
#		elif defined(APP_IMPLEMENTAR_BI_HILO)
		NBASSERT(false)
#		else
		funcConsumidorDatosEscena(&_datosHilos);
		funcConsumidorDatosRender(&_datosHilos);
#		endif
	}
	{
		NBHILO_MUTEX_ACTIVAR(&_tickMutex);
		_tickCallsCur--; NBASSERT(_tickCallsCur >= 0)
		NBHILO_MUTEX_DESACTIVAR(&_tickMutex);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::hiloIniciado() {
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::hiloIniciado")
	NBASSERT(_juegoCargado==false)
	bool nuevoEstado = _datosHilos.hiloPrincipalIniciado;
	//
#	if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	nuevoEstado = nuevoEstado && _datosHilos.hiloProductorEscenaTrabajando;
#	endif
	//
#	if defined(APP_IMPLEMENTAR_TRI_HILO)
	nuevoEstado = nuevoEstado && _datosHilos.hiloConsumidorEscenaTrabajando;
#	endif
	//
	_juegoCargado = nuevoEstado;
	//
#	if defined(APP_IMPLEMENTAR_TRI_HILO)
	if(nuevoEstado) PRINTF_INFO("APP TODOS LOS HILOS (TRIO) HAN SIDO INICIADOS\n");
#	elif defined(APP_IMPLEMENTAR_BI_HILO)
	if(nuevoEstado) PRINTF_INFO("APP TODOS LOS HILOS (BI) HAN SIDO INICIADOS\n");
#	else
	if(nuevoEstado) PRINTF_INFO("APP TODOS LOS HILOS (MONO) HAN SIDO INICIADOS\n");
#	endif
	//
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::aplicarAutoliberaciones(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::aplicarAutoliberaciones")
	AUObjeto** arrObjetos	= (AUObjeto**)NBGestorAUObjetos::arregloObjetosPendientesAutoliberar();
	int conteoObjetos		= NBGestorAUObjetos::cantidadObjetosPendientesAutoliberar();
	int iObj;
	for(iObj=0; iObj<conteoObjetos; iObj++){
		arrObjetos[iObj]->aplicarAutoLiberacionesPendientes();
	}
	NBGestorAUObjetos::resetearArregloAutoliberar();
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::notificarRedimensionVentana(const NBTamanoI wSize, const float sceneScale, const NBTamano ppiScreen, const NBTamano dpiScene){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::notificarRedimensionVentana")
	//PENDIENTE: implementar un mutex para acceder a estos datos
	_datosHilos.redim.stackUse++;
	_datosHilos.redim.size			= wSize;
	_datosHilos.redim.sceneScale	= sceneScale;
	_datosHilos.redim.ppiScreen		= ppiScreen;	//pixel-per-inch
	_datosHilos.redim.dpiScene		= dpiScene;		//dot-per-inch
	_datosHilos.redim.sceneScale	= sceneScale;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::redimensionarEscena(const NBTamanoI wSize, const float sceneScale, const NBTamano ppiScreen, const NBTamano dpiScene){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::redimensionarEscena")
	PRINTF_INFO("AUApp::redimensionarEscena wSize(%d, %d) sceneScale(%f) ppiScreen(%f, %f) dpiScene(%f, %f).\n", wSize.ancho, wSize.alto, sceneScale, ppiScreen.ancho, ppiScreen.alto, dpiScene.ancho, dpiScene.alto);
	//
	if(NBGestorEscena::gestorInicializado()){
		NBASSERT((wSize.alto > 0 && wSize.alto > 0) || _callbacks.funcGenerarRenderBuffer != NULL);
		const NBRectangulo puertoVisionAnt	= NBGestorEscena::puertoDeVision(_indiceEscenaRender);
		const NBCajaAABB cajaEscenaAnt		= NBGestorEscena::cajaProyeccionGrupo(_indiceEscenaRender, ENGestorEscenaGrupo_Escena);
		if(sceneScale != 0.0f){
			_escalaHaciaHD = sceneScale;
		}
		{
			STNBViewPortSize before, after;
			NBMemory_setZero(before);
			NBMemory_setZero(after);
			{
				const NBTamano ppi		= NBGestorEscena::pixelsPorPulgadaPantalla(_indiceEscenaRender);
				const NBTamano dpi		= NBGestorEscena::puntosPorPulgada(_indiceEscenaRender);
				const NBRectangulo rect	= NBGestorEscena::puertoDeVision(_indiceEscenaRender);
				before.ppi.width	= ppi.ancho;
				before.ppi.height	= ppi.alto;
				before.dpi.width	= dpi.ancho;
				before.dpi.height	= dpi.alto;
				before.rect.x		= rect.x;
				before.rect.y		= rect.y;
				before.rect.width	= rect.ancho;
				before.rect.height	= rect.alto;
				{
					NBGestorEscena::redimensionarEscena(_indiceEscenaRender, wSize.ancho, wSize.alto, _escalaHaciaHD, ppiScreen, dpiScene, 0.5f, false, _callbacks.funcGenerarRenderBuffer, _callbacks.funcGenerarRenderBufferParam);
					{
						const NBTamano ppi		= NBGestorEscena::pixelsPorPulgadaPantalla(_indiceEscenaRender);
						const NBTamano dpi		= NBGestorEscena::puntosPorPulgada(_indiceEscenaRender);
						const NBRectangulo rect	= NBGestorEscena::puertoDeVision(_indiceEscenaRender);
						after.ppi.width		= ppi.ancho;
						after.ppi.height	= ppi.alto;
						after.dpi.width		= dpi.ancho;
						after.dpi.height	= dpi.alto;
						after.rect.x		= rect.x;
						after.rect.y		= rect.y;
						after.rect.width	= rect.ancho;
						after.rect.height	= rect.alto;
					}
					//
					this->privConfigureForResolution(wSize, ppiScreen, dpiScene);
				}
				//
				{
					const NBRectangulo puertoVisionNvo	= NBGestorEscena::puertoDeVision(_indiceEscenaRender);
					float mitadAnchoNvo					= (float)puertoVisionNvo.ancho / 2.0f;
					float mitadAltoNvo					= (float)puertoVisionNvo.alto / 2.0f;
					float dimMaximaNva					= (puertoVisionNvo.ancho > puertoVisionNvo.alto ? puertoVisionNvo.ancho : puertoVisionNvo.alto);
					float escalaHaciaHD					= (dimMaximaNva >= 960.0f ? 1.0f : 960.0f / dimMaximaNva); //PRINTF_INFO("Escala de contenido %f\n", escalaContenido);
					NBPunto centroProyAnt;				NBCAJAAABB_CENTRO(centroProyAnt, cajaEscenaAnt);
					NBTamano tamanoProyAnt;				NBCAJAAABB_TAMANO(tamanoProyAnt, cajaEscenaAnt);
					NBCajaAABB cajaEscenaNva;
					if(NBCAJAAABB_ESTA_VACIA(cajaEscenaAnt) || cajaEscenaAnt.xMin!=cajaEscenaAnt.xMin /*(validacion NaN)*/ ||  tamanoProyAnt.ancho==0.0f || tamanoProyAnt.alto==0.0f){
						//PRINTF_INFO("Iniciando caja de proyeccion.\n");
						cajaEscenaNva.xMin				= -mitadAnchoNvo * escalaHaciaHD;
						cajaEscenaNva.xMax				= mitadAnchoNvo * escalaHaciaHD;
						cajaEscenaNva.yMin				= -mitadAltoNvo * escalaHaciaHD;
						cajaEscenaNva.yMax				= mitadAltoNvo * escalaHaciaHD;
					} else {
						//PRINTF_INFO("Redimensionando caja de proyeccion existente.\n");
						NBTamano unidadesGLPorPixelAnt;
						unidadesGLPorPixelAnt.ancho		= tamanoProyAnt.ancho / puertoVisionAnt.ancho;
						unidadesGLPorPixelAnt.alto		= tamanoProyAnt.alto / puertoVisionAnt.alto;
						cajaEscenaNva.xMin				= (centroProyAnt.x - (mitadAnchoNvo * unidadesGLPorPixelAnt.ancho));
						cajaEscenaNva.xMax				= (centroProyAnt.x + (mitadAnchoNvo * unidadesGLPorPixelAnt.ancho));
						cajaEscenaNva.yMin				= (centroProyAnt.y - (mitadAltoNvo * unidadesGLPorPixelAnt.alto));
						cajaEscenaNva.yMax				= (centroProyAnt.y + (mitadAltoNvo * unidadesGLPorPixelAnt.alto));
					}
					//PRINTF_INFO("Cambio de caja proyeccion de (%f, %f)-(%f, %f) a (%f, %f)-(%f, %f)\n", cajaEscenaAnt.xMin, cajaEscenaAnt.yMin, cajaEscenaAnt.xMax, cajaEscenaAnt.yMax, cajaEscenaNva.xMin, cajaEscenaNva.yMin, cajaEscenaNva.xMax, cajaEscenaNva.yMax);
					//PRINTF_INFO("Cambio de puerto de vision de (%f, %f) a (%f, %f)\n", puertoVisionAnt.ancho, puertoVisionAnt.alto, puertoVisionNvo.ancho, puertoVisionNvo.alto);
					NBGestorEscena::normalizaCajaProyeccionEscena(_indiceEscenaRender);
					NBGestorEscena::establecerCajaProyeccion(_indiceEscenaRender, cajaEscenaNva.xMin, cajaEscenaNva.xMax, cajaEscenaNva.yMin, cajaEscenaNva.yMax);
					NBGestorEscena::notificarObjetosTamanoEscena(_indiceEscenaRender, before, after);
				}
			}
		}
		//Force buffers to be filled (content render)
		this->_ticksWithSameContent = 0;
#		ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "AU", "redimensionarEscena");
#		endif
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

bool AUApp::teclaPresionada(SI32 tecla){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::teclaPresionada")
	bool r = false;
	if(_gameplays != NULL){
		r = _gameplays->teclaPresionada(tecla);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}

bool AUApp::teclaLevantada(SI32 tecla){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::teclaLevantada")
	bool r = false;
	if(_gameplays != NULL){
		r = _gameplays->teclaLevantada(tecla);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}

bool AUApp::teclaEspecialPresionada(SI32 codigoTecla){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::teclaEspecialPresionada")
	//AU_TECLA_REGRESAR | AU_TECLA_MENU
	bool r = false;
	if(_gameplays != NULL){
		r = _gameplays->teclaEspecialPresionada(codigoTecla);
	}
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return r;
}

bool AUApp::juegoCargado(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::juegoCargado")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _juegoCargado;
}

bool AUApp::renderPausado(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::renderPausado")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _renderizacionPausada;
}

void AUApp::renderPausar(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::renderPausar")
	_renderizacionPausada = true;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::renderReanudar(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::renderReanudar")
	_renderizacionPausada = false;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}


bool AUApp::animadorPausado(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::animadorPausado")
	AU_GESTOR_PILA_LLAMADAS_POP_APP
	return _animacionPausada;
}

void AUApp::animadorPausar(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::animadorPausar")
	_animacionPausada = true;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::animadorReanudar(){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("AUApp::animadorReanudar")
	_animacionPausada = false;
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

void AUApp::funcProductorDatosEscena(void* datos){
	STAppHilos* datosSincronizadoHilos = (STAppHilos*)datos;
	AUApp* app	= datosSincronizadoHilos->app;
	NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
	//
	//[datosSincronizadoHilos->vistaOpenGL activarContextoOpenGL]; //Pendiente de implementar
	#if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	SI32 ciclosEspera = 0;
	datosSincronizadoHilos->hiloProductorEscenaTrabajando = true;
	app->hiloIniciado();
	PRINTF_INFO("AUAPP-Productor, HILO PRODUCTOR de escenas iniciado\n");
	#endif
	NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
	//Keyboard (do this here to avoid multiple show/hide keyboard during ant the same tick)
	if(NBGestorTeclas::gestorInicializado()){
		const bool shouldBeVisible = NBGestorTeclas::keyboardShouldBeVisible();
		const bool isVisible = NBGestorTeclas::keyboardIsVisible();
		if(shouldBeVisible != isVisible){
			NBGestorTeclas::keyboardSetVisible(shouldBeVisible);
		}
	}
	NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
	//
	#if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	while(!datosSincronizadoHilos->finalizandoPrograma)
	#endif
	{
		//NBASSERT(app->_framesAcumRetraso >= 0)
		BOOL mustRenderScene = TRUE;
		const UI16 framesAcumRetrasoProcesar	= 0; //app->_framesAcumRetraso;
		UI16 framesFaltanProducir				= framesAcumRetrasoProcesar + 1; //PRINTF_INFO("FRAMES A PRODUCIR: %d.\n", framesFaltanProducir);
		NBEstadoCicloAnimacion estadoCicloJuego;
		estadoCicloJuego.inicializar();
		// --------------------------------------
		// EJECUTAR ACCIONES QUE REQUIEREN BUFFERES VACIOS (modo mono-hilo)
		// Y NOTIFICAR AL PRODUCTOR QUE HE CONSUMIDO UN BUFFER (posiblemente liberandolo para su escritura)
		// --------------------------------------
		{
			//REDIMENSIONAMIENTO DE VENTANA
			{
				//NOTA: se estan intentando mover el redimensionamiento de la ventana hacia antes de render, pero la notificacion debe coordinarse.
				const UI32 redimsPendientesProcesar = datosSincronizadoHilos->redim.stackUse; //Usar una variable de apoyo, en caso que otro hio este escribiendo en la variable.
				if(redimsPendientesProcesar != 0){
					NBASSERT((datosSincronizadoHilos->redim.size.ancho > 0 && datosSincronizadoHilos->redim.size.alto > 0) || datosSincronizadoHilos->app->_callbacks.funcGenerarRenderBuffer != NULL)
					//PRINTF_INFO("AUAPP-Consumidor, app->redimensionarEscena.\n");
					app->redimensionarEscena(datosSincronizadoHilos->redim.size, datosSincronizadoHilos->redim.sceneScale, datosSincronizadoHilos->redim.ppiScreen, datosSincronizadoHilos->redim.dpiScene);
					NBASSERT(redimsPendientesProcesar <= datosSincronizadoHilos->redim.stackUse)
					datosSincronizadoHilos->redim.stackUse -= redimsPendientesProcesar;
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				}
			}
			//AUTO-ROTATION
			/*{
				const float  ticksAnimPorSegundo = NBGestorAnimadores::ticksPorSegundo();
				app->_autorotVerifSecsAcum += (1.0f / ticksAnimPorSegundo);
				if(app->_autorotVerifSecsAcum >= 1.0f){
					app->_autorotVerifSecsAcum = 0.0f;
					if(NBMngrOSTools::canAutorotate()){
						const UI32 orientMask = NBMngrOSTools::orientationsMask();
						if((orientMask & ENAppOrientationBits_All) != 0){
							const ENAppOrientationBit orientCur = NBMngrOSTools::getOrientation();
							if((orientCur & ENAppOrientationBits_All) != 0){
								if((orientCur & orientMask) == 0){ //Current orientation is not supported
									if(orientMask & ENAppOrientationBit_Portrait){
										NBMngrOSTools::setOrientation(ENAppOrientationBit_Portrait);
									} else if(orientMask & ENAppOrientationBit_LandscapeLeftBtn){
										NBMngrOSTools::setOrientation(ENAppOrientationBit_LandscapeLeftBtn);
									} else if(orientMask & ENAppOrientationBit_LandscapeRightBtn){
										NBMngrOSTools::setOrientation(ENAppOrientationBit_LandscapeRightBtn);
									} else if(orientMask & ENAppOrientationBit_PortraitInverted){
										NBMngrOSTools::setOrientation(ENAppOrientationBit_PortraitInverted);
									}
								}
							}
						}
					}
				}
			}*/
			//REDIMENSIONAMIENTO DE VENTANA, CAMBIO DE RESOLUCION Y CARGA DE ESCENAS
			if(NBGestorEscena::gestorInicializado() && NBGestorTexturas::gestorInicializado()){
				//Ningun buffer lleno, ni en proceso de ser llenado
				if(NBGestorEscena::bufferDatosConteoLlenos() == 0 && NBGestorEscena::bufferDatosConteoBloqueados() == 0) {
					//CONSUMIR BUFFERES DE CARGA DE TEXTURAS EN SEGUNDO PLANO
					NBGestorTexturas::texCargandoSegundoPlanoProcesarBufferes();
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
					//CAMBIO DE RESOLUCION
					/*NBASSERT(NBGestorEscena::bufferDatosConteoLlenos() == 0 && NBGestorEscena::bufferDatosConteoBloqueados() == 0)
					const ENGestorEscenaCambioDef estaCambioDefincion = NBGestorEscena::cambioDefinicionEstado();
					if(escenaProducida && estaCambioDefincion < ENGestorEscenaCambioDef_Conteo){ //escenaProducida para asegurar que se presente el mensaje en GUI
						if(estaCambioDefincion == ENGestorEscenaCambioDef_AplicarCambio){
							//PRINTF_INFO("AUAPP-Consumidor, aplicando cambio de resolucion.\n");
							/ *float escalaDestino = NBGestorEscena::cambioDefinicionEscalaHaciaHD();
							 if(escalaDestino==1.0f){
							 NBGestorTexturas::establecerPrefijoRutas("PNG/X2/", 2, 1.0f); PRINTF_INFO("BIBLIOTECA DE TEXTURAS CAMBIADA A [X2]\n");
							 //en iOS: [datosSincronizadoHilos->vistaOpenGL redimensionarEscena:true];
							 NBGestorTexturas::recargarTexturas();
							 } else if(escalaDestino==2.0f){
							 NBGestorTexturas::establecerPrefijoRutas("PNG/X1/", 1, 2.0f); PRINTF_INFO("BIBLIOTECA DE TEXTURAS CAMBIADA A [X1]\n");
							 //en iOS: [datosSincronizadoHilos->vistaOpenGL redimensionarEscena:false];
							 NBGestorTexturas::recargarTexturas();
							 } else {
							 NBASSERT(false)
							 }* /
							//DESCARTAR LA ESCENA PRODUCIDA ANTERIORMENTE
#							ifdef APP_IMPLEMENTAR_TRI_HILO
							APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
							datosSincronizadoHilos->renderbufferLlenado = false;
							APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
#							else
							datosSincronizadoHilos->renderbufferLlenado = false;
#							endif
						}
						NBGestorEscena::cambioDefinicionMoverHaciaEstadoSiguiente();
						NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
					}*/
					//CARGA DE ESCENAS
					NBASSERT(NBGestorEscena::bufferDatosConteoLlenos() == 0 && NBGestorEscena::bufferDatosConteoBloqueados() == 0)
					if(app->_gameplays != NULL){
						NBASSERT(app->_gameplays->esClase(AUAppEscenasAdmin::idTipoClase))
						const bool procesarCarga = (app->_gameplays->escenasEnProcesoDeCarga() || app->_gameplays->escenasTieneAccionesPendiente());
						if(procesarCarga){
							//PRINTF_INFO("AUAPP-Consumidor, disparar inicio de carga de escena.\n");
							//Carga en segundo plano
							SI32 conteoCiclosCarga = 0;
							bool continuarConCarga = true;
							float  ticksAnimPorSegundo = NBGestorAnimadores::ticksPorSegundo();
							while(app->_gameplays->escenasEnProcesoDeCarga() && continuarConCarga){
								continuarConCarga = (app->_gameplays->tickProcesoCarga(1.0f / ticksAnimPorSegundo) /*&& datosSincronizadoHilos->ticksCargandoRecursos<1*/);
								conteoCiclosCarga++;
							}
							//if(conteoCiclosCarga!=0) PRINTF_INFO("%d cargas-recursos en %d ticks\n", conteoCiclosCarga, datosSincronizadoHilos->ticksCargandoRecursos);
							//datosSincronizadoHilos->ticksCargandoRecursos = 0;
							//Carga de nuevos gameplays
							app->_gameplays->escenasEjecutaAccionesPendientes();
							NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
						}
						//
#						if defined(CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES) && defined(CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA) && defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS)
						//Imprimir punteros cuendo termine de cargar la escena
						if(procesarCarga){
							if(!(app->_gameplays->enProcesoDeCarga() || app->_gameplays->tieneAccionesPendientes())){
								//NBGestorMemoria::debug_imprimePunterosEnUso();
								/*AUApp::debugImprimirPunterosAgrupadosDeZonaMemoria(ENMemoriaTipo_Temporal, true);
								 AUApp::debugImprimirPunterosAgrupadosDeZonaMemoria(ENMemoriaTipo_General, true);
								 AUApp::debugImprimirPunterosAgrupadosDeZonaMemoria(ENMemoriaTipo_Nucleo, true);*/
								NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
							}
						}
#						endif
					}
					NBASSERT(NBGestorEscena::bufferDatosConteoLlenos() == 0 && NBGestorEscena::bufferDatosConteoBloqueados() == 0)
				}
			}
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		}
		//Bloquear buffer
		{
			APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
			if(NBGestorEscena::gestorInicializado()){
				NBGestorEscena::bufferDatosEnEscrituraBloquear();
			}
			APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		}
		//
		estadoCicloJuego.marcarInicioCicloJuego();
		while(framesFaltanProducir != 0){
			STNBSceneModelsResult models;
			NBMemory_setZeroSt(models, STNBSceneModelsResult);
			// --------------------------------------
			// ACTUALIZAR MATRICES Y PRODUCIR ESCENA
			// --------------------------------------
			//ACTUALIZAR MATRICES Y MODELOS
#			if defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT) && (defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO))
			if(NBGestorEscena::gestorInicializado()){
				NBGestorEscena::debugMarcarBloqueoModelosEscena(true); //Para depurar que ningun objeto se modifique durante la creacion de modelos
			}
#			endif
			estadoCicloJuego.marcarIniMatricesActualizadas();
			if(NBGestorEscena::gestorInicializado()){
				NBGestorEscena::actualizarMatricesYModelos(&models);
				if(models.countScenesDirty != 0 || models.countModelsDirty != 0 || models.countTexChanged != 0){
					//if(app->_ticksWithSameContent > 0 && app->_ticksWithSameContentMin <= app->_ticksWithSameContent){
					//	PRINTF_INFO("AUApp::funcProductorDatosEscena, render restored after %d ticks paused.\n", (app->_ticksWithSameContent - app->_ticksWithSameContentMin));
					//}
					app->_ticksWithSameContent = 0;
				} else {
					app->_ticksWithSameContent++;
					if(app->_ticksWithSameContentMin <= app->_ticksWithSameContent){
						//if(app->_ticksWithSameContentMin == 0 || app->_ticksWithSameContentMin == app->_ticksWithSameContent){
						//	PRINTF_INFO("AUApp::funcProductorDatosEscena, render paused after %d ticks unchanged.\n", app->_ticksWithSameContent);
						//}
						mustRenderScene = FALSE;
					}
				}
				//PRTINF
				/*if(models.countScenesDirty != 0 || models.countModelsDirty != 0){
					STNBString str;
					NBString_init(&str);
					if(models.countScenes != 0){
						if(str.length > 0) NBString_concat(&str, ", ");
						NBString_concat(&str, " scenes("); NBString_concatUI32(&str, models.countScenes); NBString_concat(&str, ")");
					}
					if(models.countScenesDirty != 0){
						if(str.length > 0) NBString_concat(&str, ", ");
						NBString_concat(&str, " sceneRootChanged("); NBString_concatUI32(&str, models.countScenesDirty); NBString_concat(&str, ")");
					}
					if(models.countModels != 0){
						if(str.length > 0) NBString_concat(&str, ", ");
						NBString_concat(&str, " models("); NBString_concatUI32(&str, models.countModels); NBString_concat(&str, ")");
					}
					if(models.countModelsDirty != 0){
						if(str.length > 0) NBString_concat(&str, ", ");
						NBString_concat(&str, " modelsChanged("); NBString_concatUI32(&str, models.countModelsDirty); NBString_concat(&str, ")");
					}
					if(str.length > 0){
						PRINTF_INFO("actualizarMatricesYModelos result: %s.\n", str.str);
					}
					NBString_release(&str);
				}*/
			}
			estadoCicloJuego.marcarFinMatricesActualizadas();
			if(framesFaltanProducir == 1){ //En el ultimo ciclo
				//ACTUALIZAR VERTICES GL
				//PRINTF_INFO("AUAPP-Productor, consumirModelosYProducirBufferRender.\n");
				{
					estadoCicloJuego.marcarIniVerticesGLActualizados();
					if(NBGestorEscena::gestorInicializado()){
						if(mustRenderScene){
							NBGestorEscena::consumirModelosYProducirBufferRender();
						}
					}
					estadoCicloJuego.marcarFinVerticesGLActualizados();
				}
				//NOTIFICAR AL CONSUMIDOR QUE HE PRODUCIDO UN BUFFER (para su lectura)
				//PRINTF_INFO("AUAPP-Productor, bufferDatosEnEscrituraLlenar + bufferDatosEnEscrituraDesbloquear.\n");
				{
					APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaProducirEscena);
					APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
					if(NBGestorEscena::gestorInicializado()){
						if(mustRenderScene){
							NBGestorEscena::bufferDatosEnEscrituraLlenar(); //PRINTF_INFO("+++ BUFFER PRODUCIDO: %d llenos\n", NBGestorEscena::bufferDatosConteoLlenos());
						} else {
							NBGestorEscena::bufferDatosEnEscrituraDescartar(); //PRINTF_INFO("+++ BUFFER DESCARTADO: %d llenos\n", NBGestorEscena::bufferDatosConteoLlenos());
						}
						NBGestorEscena::bufferDatosEnEscrituraDesbloquear(); //PRINTF_INFO("BUFFER-ESCENA PRODUCIDO\n"); NBGestorEscena::debugImprimirEstadoBufferes();
					}
					APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
					APPHILO_DISPARA_CONDICIONAL(&datosSincronizadoHilos->condEsperaProducirEscena);
					APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaProducirEscena);
				}
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
#			if defined(CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT) && (defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO))
			if(NBGestorEscena::gestorInicializado()){
				NBGestorEscena::debugMarcarBloqueoModelosEscena(false); //Para depurar que ningun objeto se modifique durante la creacion de modelos
			}
#			endif
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			// --------------------------------------
			// ANIMAR ESCENA
			// --------------------------------------
			float ticksAnimPorSegundo = NBGestorAnimadores::ticksPorSegundo();
			//FISICA LIQUIDOS
#			ifndef CONFIG_NB_UNSUPPORT_BOX2D
			//PRINTF_INFO("AUAPP-Productor, NBGestorFisica::tickAnimacionesAdicionalesMundos.\n");
			estadoCicloJuego.marcarIniFisicaLiquidos();
			NBGestorFisica::tickAnimacionesAdicionalesMundos(1.0f / ticksAnimPorSegundo);
			estadoCicloJuego.marcarFinFisicaLiquidos();
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
#			endif
			//FISICA SOLIDOS
#			ifndef CONFIG_NB_UNSUPPORT_BOX2D
			//PRINTF_INFO("AUAPP-Productor, NBGestorFisica::tickAnimacionFisicaMundos.\n");
			estadoCicloJuego.marcarIniFisicaSolidos();
			NBGestorFisica::tickAnimacionFisicaMundos(1.0f / ticksAnimPorSegundo);
			estadoCicloJuego.marcarFinFisicaSolidos();
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
#			endif
			//UNIONES DE CUERPOS CON SUS OBJETOS_ESCENA
#			ifndef CONFIG_NB_UNSUPPORT_BOX2D
			//PRINTF_INFO("AUAPP-Productor, NBGestorFisica::ejecutarUnionesCuerposConObjetosEscena.\n");
			estadoCicloJuego.marcarIniFisicaUniones();
			NBGestorFisica::ejecutarUnionesCuerposConObjetosEscena();
			estadoCicloJuego.marcarFinFisicaUniones();
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
#			endif
			//Animaciones
			estadoCicloJuego.marcarIniAnimacionesEjecutadas();
#			ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
			if(NBGestorSonidos::gestorInicializado()){
				//PRINTF_INFO("AUAPP-Productor, NBGestorSonidos::tickSonido.\n");
				NBGestorSonidos::tickSonido(1.0f / ticksAnimPorSegundo);
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
#			endif
			//PRINTF_INFO("AUAPP-Productor, NBGestorTeclas::tickProcesar.\n");
			if(NBGestorTeclas::gestorInicializado()){
				NBGestorTeclas::tickProcesarComandos();
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
			if(NBGestorTouches::gestorInicializado()){
				NBGestorTouches::debugTick(1.0f / ticksAnimPorSegundo);
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
			//Contacts
			if(NBMngrContacts::isInited()){
				NBMngrContacts::tick(1.0f / ticksAnimPorSegundo);
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
			if(NBGestorAnimadores::gestorInicializado()){
				const float secs = (1.0f / ticksAnimPorSegundo);
				NBGestorAnimadores::difundeTick(secs, ENGestorAnimadoresGrupo_Conteo);
				if(app->_ticksWithSameContentMin != 0 && app->_ticksWithSameContentMin < app->_ticksWithSameContent){
					app->_secsWithSameContent += secs;
				} else {
					app->_secsWithSameContent = 0.0f;
				}
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
			if(NBGestorEscena::gestorInicializado()){
				NBGestorEscena::touchesProcesar();
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
			estadoCicloJuego.marcarFinAnimacionesEjecutadas();
			//Si no es el ultimo ciclo de produccion, descartar datos
			if(framesFaltanProducir != 1){
				if(NBGestorEscena::gestorInicializado()){
					NBGestorEscena::bufferDatosEnEscrituraVaciar();
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				}
			}
			//Aplicar autoliberaciones de objetos
			{
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				AUApp::aplicarAutoliberaciones();
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			}
			//
			app->_estadisticas.ticksAnimEnSegAcum++;
			//
			framesFaltanProducir--;
		}
		//app->_framesAcumRetraso -= framesAcumRetrasoProcesar;
		estadoCicloJuego.marcarFinCicloJuego();
		if(app->_resumenDebug != NULL){
			app->_resumenDebug->acumularCiclosAnimacion(estadoCicloJuego);
		}
		// --------------------------------------
		// ESPERAR A QUE EXISTA UN BUFFER LIBRE
		// --------------------------------------
#		if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
		APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirEscena);
		bool esperar = false; ciclosEspera = 0;
		do {
			APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
			esperar = (datosSincronizadoHilos->modoRenderSecuencial && (NBGestorEscena::bufferDatosConteoLlenos()!=0 || NBGestorEscena::bufferDatosConteoBloqueados()!=0)); //Si se esta en modo secuencial (esperar a que todos los bufferes sean consumidos)
			if(!esperar){
				if(NBGestorEscena::gestorInicializado()){
					esperar = (!NBGestorEscena::bufferDatosEnEscrituraMoverHaciaSiguienteNoBloqueado()); //Si el modo secuencial no limita, entonces intentar mover al siguiente buffer
				}
			}
			APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
			if(esperar) {
				APPHILO_ESPERA_CONDICIONAL(&datosSincronizadoHilos->condEsperaConsumirEscena, &datosSincronizadoHilos->mutexEsperaConsumirEscena);
			}
			ciclosEspera++;
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		} while(esperar); if(ciclosEspera>2) PRINTF_INFO("%d ciclos esperando consumo de escena (%s)\n", ciclosEspera, datosSincronizadoHilos->modoRenderSecuencial?"secuencial":"no-secuencial");
		APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirEscena);
#		else
		if(NBGestorEscena::gestorInicializado()){
			NBGestorEscena::bufferDatosEnEscrituraMoverHaciaSiguiente();
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		}
#		endif
		NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		//PRINTF_INFO("AUAPP-Productor, NBGestorEscena::bufferDatosEnEscrituraMoverHaciaSiguiente.\n");
	}
#	if defined(APP_IMPLEMENTAR_TRI_HILO) || defined(APP_IMPLEMENTAR_BI_HILO)
	PRINTF_INFO("AUApp, FIN DE HILO PRODUCTOR RENDER\n");
	datosSincronizadoHilos->hiloProductorEscenaTrabajando = false;
#	endif
}

void AUApp::funcConsumidorDatosEscena(void* datos){
	STAppHilos* datosSincronizadoHilos = (STAppHilos*)datos;
	AUApp* app	= datosSincronizadoHilos->app;
	SI32 ciclosEspera	= 0;
	NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
	//
	//[datosSincronizadoHilos->vistaOpenGL activarContextoOpenGL]; //Pendiente de implementar
	#if defined(APP_IMPLEMENTAR_TRI_HILO)
	datosSincronizadoHilos->hiloConsumidorEscenaTrabajando = true;
	app->hiloIniciado();
	PRINTF_INFO("AUApp-Consumidor, HILO CONSUMIDOR de escenas iniciado\n");
	#endif
	//Ciclo de renderizado
	#if defined(APP_IMPLEMENTAR_TRI_HILO)
	while(!datosSincronizadoHilos->finalizandoPrograma){
	#endif
		//ESPERAR A QUE SE PRODUZCA UN BUFFER (si el siguiente no esta lleno)
		{
			APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaProducirEscena);
			bool esperar = false; ciclosEspera = 0;
			do {
				APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
				esperar = false;
				if(NBGestorEscena::gestorInicializado()){
					esperar = (!NBGestorEscena::bufferDatosEnLecturaMoverHaciaSiguienteNoBloqueado());
				}
#				if !defined(APP_IMPLEMENTAR_TRI_HILO) && !defined(APP_IMPLEMENTAR_BI_HILO)
				NBASSERT(!esperar) //En mono-hilo, este ciclo seria infinito
#				endif
				APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
				if(esperar){ APPHILO_ESPERA_CONDICIONAL(&datosSincronizadoHilos->condEsperaProducirEscena, &datosSincronizadoHilos->mutexEsperaProducirEscena); }
				ciclosEspera++;
			} while(esperar); //if(ciclosEspera>1) PRINTF_INFO("%d ciclos esperando produccion de escena\n", ciclosEspera);
			{
				APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
				if(NBGestorEscena::gestorInicializado()){
					//PRINTF_INFO("AUAPP-Consumidor, NBGestorEscena::bufferDatosEnLecturaBloquear.\n");
					NBGestorEscena::bufferDatosEnLecturaBloquear();
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				}
				APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena);
			}
			APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaProducirEscena);
		}
		//
		datosSincronizadoHilos->modoRenderSecuencial = false;
		if(NBGestorTexturas::gestorInicializado()){
			NBGestorTexturas::texSyncAny();
			datosSincronizadoHilos->modoRenderSecuencial = (NBGestorTexturas::texCargandoSegundoPlanoConteo() != 0);
		}
		if(!datosSincronizadoHilos->modoRenderSecuencial){
			if(app->_gameplays != NULL){
				NBASSERT(app->_gameplays->esClase(AUAppEscenasAdmin::idTipoClase))
				datosSincronizadoHilos->modoRenderSecuencial = (datosSincronizadoHilos->redim.stackUse != 0) || app->_gameplays->escenasEnProcesoDeCarga() || app->_gameplays->escenasTieneAccionesPendiente();
			} else {
				datosSincronizadoHilos->modoRenderSecuencial = (datosSincronizadoHilos->redim.stackUse != 0); //en IOS [datosSincronizadoHilos->vistaOpenGL pendienteRedimensionarEscena]
			}
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		}
		//
		//NBASSERT(datosSincronizadoHilos->renderbufferLlenado==false)
		//
		//
		ENGestorEscenaBufferEstado buffState = ENGestorEscenaBufferEstado_Vacio;
		bool escenaProducida = false;
		if(NBGestorEscena::gestorInicializado()){
			buffState = NBGestorEscena::bufferDatosEnLecturaEstado();
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
		}
		if(buffState == ENGestorEscenaBufferEstado_Lleno){
			escenaProducida = true;
			// --------------------------------------
			// RENDERIZAR ESCENA
			// --------------------------------------
			//PRINTF_INFO("AUAPP-Consumidor, NBGestorEscena::consumirBufferRenderYEnviarComandosDibujo.\n");
			if(app->_resumenDebug != NULL){
				NBEstadoCicloRender estadoCicloRender; estadoCicloRender.inicializar();
				estadoCicloRender.marcarIniComandosGLEnviados();
				if(NBGestorEscena::gestorInicializado()){
					NBGestorEscena::consumirBufferRenderYEnviarComandosDibujo();
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				}
				estadoCicloRender.marcarFinComandosGLEnviados();
				app->_resumenDebug->acumularCiclosRender(estadoCicloRender);
				NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			} else {
				if(NBGestorEscena::gestorInicializado()){
					NBGestorEscena::consumirBufferRenderYEnviarComandosDibujo();
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				}
			}
			app->_estadisticas.ticksRenderEnSegAcum++;
			//NOTIFICAR AL CONSUMIDOR DE RENDER QUE SE HA PRODUCIDO UNO
			#if defined(APP_IMPLEMENTAR_TRI_HILO)
			APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
			#endif
			datosSincronizadoHilos->renderbufferLlenado = escenaProducida;
			#if defined(APP_IMPLEMENTAR_TRI_HILO)
			APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
			#endif
		}
		// --------------------------------------
		// EJECUTAR ACCIONES QUE REQUIEREN BUFFERES VACIOS (modo mono-hilo)
		// Y NOTIFICAR AL PRODUCTOR QUE HE CONSUMIDO UN BUFFER (posiblemente liberandolo para su escritura)
		// --------------------------------------
		{
			APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirEscena); //mutexCondBufferConsumido
			{
				APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena); //mutexCondBufferConsumido
				if(NBGestorEscena::gestorInicializado()){
					//PRINTF_INFO("AUAPP-Consumidor, NBGestorEscena::bufferDatosEnLecturaVaciar.\n");
					NBGestorEscena::bufferDatosEnLecturaVaciar(); //PRINTF_INFO("--- BUFFER CONSUMIDO: %d llenos\n", NBGestorEscena::bufferDatosConteoLlenos());
					NBGestorEscena::bufferDatosEnLecturaDesbloquear(); //PRINTF_INFO("BUFFER-ESCENA CONSUMIDO\n"); NBGestorEscena::debugImprimirEstadoBufferes();
					NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
				}
				APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexBufferesEscena); //mutexCondBufferConsumido
			}
			NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
			APPHILO_DISPARA_CONDICIONAL(&datosSincronizadoHilos->condEsperaConsumirEscena);
			APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirEscena);
		}
#		if defined(APP_IMPLEMENTAR_TRI_HILO)
		//ESPERAR A QUE EL RENDER SEA VOLCADO EN PANTALLA
		APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
		ciclosEspera = 0;
		while(datosSincronizadoHilos->renderbufferLlenado){
			APPHILO_ESPERA_CONDICIONAL(&datosSincronizadoHilos->condEsperaConsumirRender, &datosSincronizadoHilos->mutexEsperaConsumirRender);
			ciclosEspera++;
		} //if(ciclosEspera>1) PRINTF_INFO("%d ciclos esperando consumo de renderBuffer\n", ciclosEspera);
		APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
		NB_GESTOR_AUOBJETOS_VALIDATE_ALL_OBJETS_TO_BE_ALIVE()
#		endif
	//
#	if defined(APP_IMPLEMENTAR_TRI_HILO)
	}
	PRINTF_INFO("AUApp, FIN DE HILO CONSUMIDOR RENDER\n");
	datosSincronizadoHilos->hiloConsumidorEscenaTrabajando = false;
#	endif
}

void AUApp::funcConsumidorDatosRender(void* datos){
	STAppHilos* datosSincronizadoHilos = (STAppHilos*)datos;
	AUApp* app	= datosSincronizadoHilos->app;
	//VOLCAR RENDERIZADO HACIA PANTALLA (si el render esta lleno)
	#if defined(APP_IMPLEMENTAR_TRI_HILO)
	APPHILO_ACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
	#endif
	if(datosSincronizadoHilos->renderbufferLlenado && app->_callbacks.funcVolcarBuffer != NULL){
		//PRINTF_INFO("AUAPP-Render, funcVolcarBuffer.\n");
		if(app->_resumenDebug != NULL){
			NBEstadoCicloVolcado estadoCicloVolcado;
			estadoCicloVolcado.marcarIniEscenaVolcadaHaciaPantalla();
			(*app->_callbacks.funcVolcarBuffer)(app->_callbacks.funcVolcarBufferParam, app, app->_indiceEscenaRender);
			estadoCicloVolcado.marcarFinEscenaVolcadaHaciaPantalla();
			app->_resumenDebug->acumularCiclosVolcado(estadoCicloVolcado);
		} else {
			(*app->_callbacks.funcVolcarBuffer)(app->_callbacks.funcVolcarBufferParam, app, app->_indiceEscenaRender);
		}
		datosSincronizadoHilos->renderbufferLlenado = false;
	} //else { PRINTF_INFO("ADVERTENCIA: ignorando tickPantalla porque no habia un renderBuffer producido\n"); }
	#if defined(APP_IMPLEMENTAR_TRI_HILO)
	//Para que el hilo de produccion deje de cargar y continue produciendo
	if(datosSincronizadoHilos->modoRenderSecuencial){
		datosSincronizadoHilos->ticksCargandoRecursos++;
	} else {
		datosSincronizadoHilos->ticksCargandoRecursos = 0;
	}
	APPHILO_DISPARA_CONDICIONAL(&datosSincronizadoHilos->condEsperaConsumirRender);
	APPHILO_DESACTIVA_MUTEX(&datosSincronizadoHilos->mutexEsperaConsumirRender);
	#endif
}

#if defined(CONFIG_NB_GESTOR_MEMORIA_REGISTRAR_BLOQUES) && defined(CONFIG_NB_GESTOR_MEMORIA_IMPLEMENTAR_GRUPOS_ZONAS_MEMORIA) && defined(CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS)
struct STGruposNombres {
	AUCadenaMutable8* nombreGrupo;
	SI32 cantidadRepeticiones;
	UI32 bytesTotal;
	//COMPARACIONES CON REGISTROS
	bool operator==(const STGruposNombres &otroRegistro) const {
		return (nombreGrupo->esIgual(otroRegistro.nombreGrupo->str()));
	}
	bool operator!=(const STGruposNombres &otroRegistro) const {
		return !(nombreGrupo->esIgual(otroRegistro.nombreGrupo->str()));
	}
};
//
void AUApp::debugImprimirPunterosAgrupadosDeZonaMemoria(ENMemoriaTipo tipoZonaMemoria, bool agruparPorNombresDeClase){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("APP::imprimirPunterosAgrupadosDeZonaMemoria")
	//Imprimir la cantidad de punteros agrupadas por nombres
	AUArregloNativoMutableP<STGruposNombres>* gruposNombres = new(ENMemoriaTipo_Temporal) AUArregloNativoMutableP<STGruposNombres>();
	const STZonaMemoria* zonasMem		= NBGestorMemoria::zonasMemoria();
	const NBDescBloque*	bloqueMem		= NBGestorMemoria::bloquesReservados();
	SI32 cantBloquesMem					= NBGestorMemoria::cantidadBloquesReservados();
	SI32 iBloq2;
	PRINTF_INFO("------------------------------------------------------\n");
	PRINTF_INFO("OBJETOS DE ZONAS DE MEMORIA '%s' AGRUPADOS POR NOMBRES\n", NBGestorMemoria::nombreTipoMemoria(tipoZonaMemoria));
	for(iBloq2=0; iBloq2<cantBloquesMem; iBloq2++){
		if(bloqueMem[iBloq2].usado){
			SI32 iZona2;
			for(iZona2=0; iZona2<NB_GESTOR_MEMORIA_CANTIDAD_REGISTROS_ZONA_MEMORIA; iZona2++){
				if(zonasMem[iZona2].registroOcupado && zonasMem[iZona2].tipoMemoria==tipoZonaMemoria){
					if(NBZonaMemoriaConIndiceLibres::punteroEstaDentroDeZonaDatos(zonasMem[iZona2].datos, bloqueMem[iBloq2].puntero)){
						const char* nombrePuntero2 = bloqueMem[iBloq2].nombreReferencia;
						#ifdef CONFIG_NB_GESTOR_AUOBJETOS_REGISTRAR_TODOS
						if(agruparPorNombresDeClase){
							if(NBGestorAUObjetos::esObjeto(bloqueMem[iBloq2].puntero)){
								nombrePuntero2 = ((AUObjeto*)bloqueMem[iBloq2].puntero)->nombreUltimaClase();
							}
						}
						#endif
						SI32 indiceNombre = -1;
						SI32 iCadena;
						for(iCadena=0; iCadena<gruposNombres->conteo && indiceNombre==-1; iCadena++){
							if(AUCadena8::esIgual(gruposNombres->elemento[iCadena].nombreGrupo->str(), nombrePuntero2)){
								indiceNombre = iCadena;
							}
						}
						if(indiceNombre==-1){
							STGruposNombres nuevoGrupo;
							nuevoGrupo.nombreGrupo			= new(ENMemoriaTipo_Temporal) AUCadenaMutable8(nombrePuntero2);
							nuevoGrupo.cantidadRepeticiones	= 0;
							nuevoGrupo.bytesTotal			= 0;
							SI32 iBloq;
							for(iBloq=0; iBloq<cantBloquesMem; iBloq++){
								if(bloqueMem[iBloq].usado){
									SI32 iZona;
									for(iZona=0; iZona<NB_GESTOR_MEMORIA_CANTIDAD_REGISTROS_ZONA_MEMORIA; iZona++){
										if(zonasMem[iZona].registroOcupado && zonasMem[iZona].tipoMemoria==tipoZonaMemoria){
											if(NBZonaMemoriaConIndiceLibres::punteroEstaDentroDeZonaDatos(zonasMem[iZona].datos, bloqueMem[iBloq].puntero)){
												const char* nombrePuntero = bloqueMem[iBloq].nombreReferencia;
												#ifdef CONFIG_NB_GESTOR_AUOBJETOS_REGISTRAR_TODOS
												if(agruparPorNombresDeClase){
													if(NBGestorAUObjetos::esObjeto(bloqueMem[iBloq].puntero)){
														nombrePuntero = ((AUObjeto*)bloqueMem[iBloq].puntero)->nombreUltimaClase();
													}
												}
												#endif
												if(AUCadena8::esIgual(nombrePuntero2, nombrePuntero)){
													nuevoGrupo.cantidadRepeticiones++;
													nuevoGrupo.bytesTotal += (UI32)bloqueMem[iBloq].bytes;
												}
											}
										}
									}
								}
							}
							NBASSERT(nuevoGrupo.cantidadRepeticiones!=0)
							gruposNombres->agregarElemento(nuevoGrupo);
						}
					}
				}
			}
		}
	}
	PRINTF_INFO("------------------------------------------------------\n");
	//Imprimir ordenado por mayor tamano en bytes
	SI32 conteoRepeticionesTotal = 0; UI32 bytesTotalZona = 0;
	SI32 iGrupoImpreso = 0;
	while(gruposNombres->conteo!=0){
		SI32 iGrupo, iGrupoMayor = 0; UI32 bytesMayor = 0;
		for(iGrupo=0; iGrupo<gruposNombres->conteo; iGrupo++){
			if(gruposNombres->elemento[iGrupo].bytesTotal>bytesMayor){
				iGrupoMayor = iGrupo;
				bytesMayor	= gruposNombres->elemento[iGrupo].bytesTotal;
			}
		}
		STGruposNombres datosGrupo = gruposNombres->elemento[iGrupoMayor];
		conteoRepeticionesTotal += datosGrupo.cantidadRepeticiones;
		bytesTotalZona += datosGrupo.bytesTotal;
		PRINTF_INFO("%d) %d bytes (%d x %d bytes prom): '%s'\n", ++iGrupoImpreso, (SI32)datosGrupo.bytesTotal, datosGrupo.cantidadRepeticiones, (SI32)datosGrupo.bytesTotal/(SI32)datosGrupo.cantidadRepeticiones, datosGrupo.nombreGrupo->str());
		datosGrupo.nombreGrupo->liberar(NB_RETENEDOR_NULL);
		gruposNombres->quitarElementoEnIndice(iGrupoMayor);
	}
	gruposNombres->liberar(NB_RETENEDOR_NULL);
	PRINTF_INFO("%d punteros en %d grupos (%d bytes total)\n", conteoRepeticionesTotal, iGrupoImpreso, (SI32)bytesTotalZona);
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

#ifdef CONFIG_NB_GESTOR_AUOBJETOS_REGISTRAR_TODOS
void AUApp::debugVolcarObjetosCSV(FILE* flujo){
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("APP::debugVolcarObjetosCSV")
	NBGestorAUObjetos::objetosArrayLock();
	{
		char nameBuff[1024];
		AUObjeto** arr = (AUObjeto**)NBGestorAUObjetos::objetosArray();
		SI32 i; const SI32 count = NBGestorAUObjetos::objetosArrayUse();
		for(i = 0; i < count; i++){
			AUObjeto* obj = arr[i];
			if(obj != NULL){
#				ifdef CONFIG_NB_GESTOR_MEMORIA_GUARDAR_NOMBRES_PUNTEROS
				NBGestorMemoria::nombresMemoria(obj, nameBuff, 1024);
#				else
				nameBuff[0] = '\0';
#				endif
				fprintf(flujo, "\"%d\",\"%s\",\"%d\",\"%d\"\n", (i + 1), nameBuff, obj->conteoReferencias(), obj->conteoAutoliberacionesPendientes());
				obj->debug_imprimeInfo();
			}
		}
	}
	NBGestorAUObjetos::objetosArrayUnlock();
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}
#endif

