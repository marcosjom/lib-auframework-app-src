
#include "AUAppNucleoPrecompilado.h"
#include "com_auframework_AppNative.h"
#include <jni.h>
#include <android/log.h>
#include "AUAppGlueAndroidJNI.h"
#include "AUApp.h"
#include "NBMngrOSTools.h"
#include "NBMngrStore.h"

//#define JNI_STACK_PRINTF(STR)				__android_log_print(ANDROID_LOG_INFO, "AU", "JNI STACK:" STR);
#define JNI_STACK_PRINTF(STR)				(0);

#define JNI_STACK_KEYINPUT_PRINTF(STR)		__android_log_print(ANDROID_LOG_INFO, "AU", "JNI KEYINPUT STACK:" STR);
//#define JNI_STACK_KEYINPUT_PRINTF(STR)	(0);

#define LEER_PRECACHE					false
#define LEER_CACHE						true
#define ESCRIBIR_CACHE					true

//-----------------------
//-- Java Environments --
//-----------------------

bool _initedEngine	= false;
bool _initedMMedia	= false;
bool _initedSurface	= false;
AUApp* _app			= NULL;
//Global Refs (ToDo: release refs)
jobject _activity	= NULL;
jobject _appNative	= NULL;
jobject _delegate	= NULL;
jobject _srvConnMon	= NULL;

//ToDo: reactivate any reference to:
//NBGestorRefranero::
//AUEscenasAdministrador* _escenas		= NULL;

//Called when the native library is loaded
//(must return the JNI version needed by the native library).
/*jint JNI_OnLoad(JavaVM* vm, void* reserved){
    PRINTF_INFO("JNI_OnLoad.\n");
    return JNI_VERSION_1_6;
}*/

//Called when the class loader containing the native library is garbage collected
/*void JNI_OnUnload(JavaVM *vm, void *reserved){
    PRINTF_INFO("JNI_OnUnload.\n");
}*/

jobject Java_com_auframework_AppNative_getActivity(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("getActivity")
    jobject r = NULL;
    jclass clsObj = pEnv->GetObjectClass(pObj); NBASSERT(clsObj != NULL)
    if(clsObj != NULL){
        jfieldID fActivity = pEnv->GetFieldID(clsObj, "_activity", "Landroid/app/Activity;"); NBASSERT(fActivity != NULL)
        if(fActivity != NULL){
            r = pEnv->GetObjectField(pObj, fActivity);
        }
    }
    return r;
}

jobject Java_com_auframework_AppNative_getDelegate(JNIEnv *pEnv, jobject pObj){
	JNI_STACK_PRINTF("getDelegate")
	jobject r = NULL;
	jclass clsObj = pEnv->GetObjectClass(pObj); NBASSERT(clsObj != NULL)
	if(clsObj != NULL){
		jfieldID fDelegate = pEnv->GetFieldID(clsObj, "_delegate", "Lcom/auframework/AppNative$Delegate;"); NBASSERT(fDelegate != NULL)
		if(fDelegate != NULL){
			r = pEnv->GetObjectField(pObj, fDelegate);
		}
	}
	return r;
}

jobject Java_com_auframework_AppNative_getServiceConnectionMonitor(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("getServiceConnectionMonitor")
    jobject r = NULL;
    jclass clsObj = pEnv->GetObjectClass(pObj); NBASSERT(clsObj != NULL)
    if(clsObj != NULL){
        jfieldID fSrvConnMon = pEnv->GetFieldID(clsObj, "_srvConnMon", "Lcom/auframework/AppNative$ServiceConnectionMonitor;"); NBASSERT(fSrvConnMon != NULL)
        if(fSrvConnMon != NULL){
            r = pEnv->GetObjectField(pObj, fSrvConnMon);
        }
    }
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_setView(JNIEnv *pEnv, jobject pObj, jobject pView){
    JNI_STACK_PRINTF("setView")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "setView");
    jboolean r = (_app->setView(pView) ? JNI_TRUE : JNI_FALSE);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_onCreate(JNIEnv *pEnv, jobject pObj, jobject savedInstanceState /*Bundle*/){
    JNI_STACK_PRINTF("onCreate")
    jboolean exito 	= JNI_FALSE;
	if(_initedEngine){
		PRINTF_WARNING("MOTOR ya esta inicializado.\n");
		exito = JNI_TRUE;
	} else {
		PRINTF_INFO("MOTOR inicializando.\n");
        //
		//------------------------
		//--- Identificar endianes del procesador actual
		//------------------------
		UI16 datoPruebaEndianes = 1; UI8* valoresBytes = (UI8*)&datoPruebaEndianes;
		PRINTF_INFO("El dispositivo es %s-ENDIEN (%d bytes por puntero)\n", (valoresBytes[0]==0 && valoresBytes[1]!=0)?"BIG":"LITTLE", (SI32)sizeof(void*));
		//------------------------------
		//--- Validar fecha de ejecucion
		//------------------------------
		/*SI32 anoFechaMin = 2014, mesFechaMin = 03, diaFechaMin = 01;
		SI32 anoFechaMax = 2014, mesFechaMax = 03, diaFechaMax = 31;
		time_t now = time(0); tm* localtm = localtime(&now);
		//int	tm_mday;	day of the month [1-31]
		//int	tm_mon;		months since January [0-11]
		//int	tm_year;	years since 1900
		SI32 anoFechaAct = 1900 + localtm->tm_year, mesFechaAct = 1 + localtm->tm_mon, diaFechaAct = localtm->tm_mday;
		if((anoFechaAct<anoFechaMin || (anoFechaAct==anoFechaMin && mesFechaAct<mesFechaMin) || (anoFechaAct==anoFechaMin && mesFechaAct==mesFechaMin && diaFechaAct<diaFechaMin))
			|| (anoFechaAct>anoFechaMax || (anoFechaAct==anoFechaMax && mesFechaAct>mesFechaMax) || (anoFechaAct==anoFechaMax && mesFechaAct==mesFechaMax && diaFechaAct>diaFechaMax))){
			char c;
			PRINTF_ERROR("Prototipo ha caducado\n");
			PRINTF_INFO("Contacte a info@ostoklab.com para obtener un prototipo mas reciente\n");
			PRINTF_INFO("Presione ENTER para salir\n");
			scanf("%c", &c);
			return JNI_FALSE;
		}*/
		//
		_app 				= NULL;
		jobject activity	= Java_com_auframework_AppNative_getActivity(pEnv, pObj);
		jobject delegate	= Java_com_auframework_AppNative_getDelegate(pEnv, pObj);
		{
			AUApp::linkToDefaultJni(pEnv, activity);
			const SI32 qEnvsStart = AUApp::getDefaultGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onCreate");
			//Configure App (ex: start network)
			{
				jclass cls = pEnv->FindClass("com/auframework/AppNative$Delegate"); NBASSERT(cls != NULL)
				if(cls != NULL){
					jmethodID mid = pEnv->GetMethodID(cls, "appContextCreate", "()V"); NBASSERT(mid != NULL)
					if(mid != NULL) {
						pEnv->CallVoidMethod(delegate, mid);
					}
				}
			}
			//
			if(!AUApp::inicializarNucleo(AUAPP_BIT_MODULO_RED)){
				PRINTF_ERROR("ERROR, AUApp::inicializarNucleo ha fallado.\n");
			} else {
				PRINTF_INFO("AUApp::inicializarNucleo success.\n");
				//Identificar firma de paquete
				/*NBSHA1 shaFirma;
				{
					const char* strFirma = "";
					//MD5_CTX md5Firma; MD5Init(&md5Firma);
					JNIEnv* env = pEnv;
					jclass cls = env->FindClass("android/content/ContextWrapper");
					//this.getPackageManager();
					jmethodID mid = env->GetMethodID(cls, "getPackageManager", "()Landroid/content/pm/PackageManager;");
					if (mid != NULL) {
						jobject pm = env->CallObjectMethod(pContext, mid);
						if (pm != NULL) {
							//this.getPackageName();
							mid = env->GetMethodID(cls, "getPackageName", "()Ljava/lang/String;");
							if (mid != NULL) {
								jstring packageName = (jstring)env->CallObjectMethod(pContext, mid);
								// packageManager->getPackageInfo(packageName, GET_SIGNATURES);
								cls = env->GetObjectClass(pm);
								mid  = env->GetMethodID(cls, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
								jobject packageInfo = env->CallObjectMethod(pm, mid, packageName, 0x40); //GET_SIGNATURES = 64;
								cls = env->GetObjectClass(packageInfo);
								jfieldID fid = env->GetFieldID(cls, "signatures", "[Landroid/content/pm/Signature;");
								jobjectArray signatures = (jobjectArray)env->GetObjectField(packageInfo, fid);
								jobject sign = env->GetObjectArrayElement(signatures, 0);
								cls = env->GetObjectClass(sign);
								mid = env->GetMethodID(cls, "toCharsString", "()Ljava/lang/String;");
								if(mid != NULL){
									jstring signString = (jstring)env->CallObjectMethod(sign, mid);
									strFirma = jstringTostring(env, signString);
									//
									shaFirma.procesar(strFirma, AUCadena8::tamano(strFirma));
									shaFirma.terminar();
									//
									//MD5Update(&md5Firma, (unsigned char*)strFirma, AUCadena8::tamano(strFirma));
									//MD5Final(&md5Firma);
								}
							}
						}
					}
				}*/
				//
				/*
				Firmas de certificados:
				Debug
				MD5:  A2AD4274-CFB4B0A3-0A2BF7C6-F03501DB
				SHA1: 17CBA9D68F-2CA6F7081D-90BDF81CEE-88596B5711
				Release
				MD5:  558B2E6E-55DEE1CF-EAC823B5-C8A84603
				SHA1: 8DA909384E-CD24FEA3AE-CAF997A8F7-E87F36BCAB
				Release (Serene Hearts)
				MD5:  00000000-00000000-00000000-00000000
				SHA1: 479C85A480825AAF6372A6033B783FBFD52757FD
				*/
				/*if(AUCadena8::esIgual(shaFirma.hashHex(), "17CBA9D68F2CA6F7081D90BDF81CEE88596B5711")
				   || AUCadena8::esIgual(shaFirma.hashHex(), "8DA909384ECD24FEA3AECAF997A8F7E87F36BCAB")
				   || AUCadena8::esIgual(shaFirma.hashHex(), "479C85A480825AAF6372A6033B783FBFD52757FD")
				   )*/{
					jobject srvConnMon	= Java_com_auframework_AppNative_getServiceConnectionMonitor(pEnv, pObj); NBASSERT(srvConnMon != NULL)
					//Global refs (ToDo: release refs)
					{
						_activity	= pEnv->NewGlobalRef(activity);
						_appNative	= pEnv->NewGlobalRef(pObj);
						_delegate	= pEnv->NewGlobalRef(delegate);
						_srvConnMon	= pEnv->NewGlobalRef(srvConnMon);
					}
                    //Create APp
					{
						STAppCallbacks appCallbacks;
						AUApp::inicializarCallbacks(&appCallbacks);
						_app = new AUApp(&appCallbacks, "AUApp", true /*permitirActividadRedEnBG*/);
					}
					//Configure App
					{
						_app->setAppNative(pObj);
						_app->setSrvcConnMon(srvConnMon);
					}
					_initedEngine		= true;
					//Configure App (link with methods)
					{
						jclass cls = pEnv->FindClass("com/auframework/AppNative$Delegate"); NBASSERT(cls != NULL)
						if(cls != NULL){
							jmethodID mid = pEnv->GetMethodID(cls, "appConfigurePost", "(J)V"); NBASSERT(mid != NULL)
							if(mid != NULL) {
								pEnv->CallVoidMethod(delegate, mid, (jlong)_app);
							}
						}
					}
					//
                    _app->appStateOnCreate();
					exito = JNI_TRUE;
				}
			}
			const SI32 qEnvsEnd = AUApp::getDefaultGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
		}
	}
	return exito;
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_onDestroy(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("onDestroy")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onDestroy");
	_app->appStateOnDestroy();
	//Configure App (ex: start network)
	{
		jobject delegate	= Java_com_auframework_AppNative_getDelegate(pEnv, pObj);
		jclass cls = pEnv->FindClass("com/auframework/AppNative$Delegate"); NBASSERT(cls != NULL)
		if(cls != NULL){
			jmethodID mid = pEnv->GetMethodID(cls, "appContextDestroy", "()V"); NBASSERT(mid != NULL)
			if(mid != NULL) {
				pEnv->CallVoidMethod(delegate, mid);
			}
		}
	}
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_onStart(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("onStart")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onStart");
    _app->appStateOnStart();
	//ToDo: get initial Intent usgin "this.getIntent()";
	/*if(pIntent != NULL){
		PRINTF_INFO("onCreate(Intent).\n");
		_app->appIntentReceived(pEnv, pIntent);
	}*/
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_onStop(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("onStop")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onStop");
    _app->appStateOnStop();
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_onResume(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("onResume")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onResume");
    _app->appStateOnResume();
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_onPause(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("onPause")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onPause");
    _app->appStateOnPause();
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_onNewIntent(JNIEnv *pEnv, jobject pObj, jobject pIntent) {
    JNI_STACK_PRINTF("onNewIntent")
	PRINTF_INFO("onNewIntent(Intent).\n");
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onNewIntent");
    _app->appIntentReceived(pIntent);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv);
    NBASSERT(qEnvsEnd == qEnvsStart)
	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_onActivityResult(JNIEnv *pEnv, jobject pObj, jint reqCode, jint resp, jobject pIntent){
    JNI_STACK_PRINTF("onActivityResult")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onActivityResult");
    _app->appActResultReceived(reqCode, resp, pIntent);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_onRequestPermissionsResult(JNIEnv *pEnv, jobject pObj, jint reqCode, jobjectArray perms, jintArray grantResults){
	JNI_STACK_PRINTF("onRequestPermissionsResult")
	PRINTF_INFO("onRequestPermissionsResult.\n");
	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onRequestPermissionsResult");
	_app->appReqPermResult(reqCode, perms, grantResults);
	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_openFilePath(JNIEnv *pEnv, jobject pObj, jobject pFilepath, jobject pParams){
	jboolean r = FALSE;
	JNI_STACK_PRINTF("openFilePath")
	PRINTF_INFO("openFilePath.\n");
	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "openFilePath");
	jstring filepath = (jstring)pFilepath;
	jstring params = (jstring)pParams;
	if(filepath != NULL){
		STNBFilePickerData pp;
		NBMemory_setZeroSt(pp, STNBFilePickerData);
		//Load params
		if(params != NULL){
			const char* paramsData = pEnv->GetStringUTFChars(params, 0);
			NBStruct_stReadFromJsonStr(paramsData, NBString_strLenBytes(paramsData), NBFilePickerData_getSharedStructMap(), &pp, sizeof(pp));
			pEnv->ReleaseStringUTFChars(params, paramsData);
		}
		//Call
		{
			const char* plainUrl = pEnv->GetStringUTFChars(filepath, 0);
			r = _app->broadcastOpenUrl(plainUrl, pp.userData, pp.userDataSz);
			pEnv->ReleaseStringUTFChars(filepath, plainUrl);
		}
		NBStruct_stRelease(NBFilePickerData_getSharedStructMap(), &pp, sizeof(pp));
	}
	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_nativeFinalizar(JNIEnv *pEnv, jobject pObj){
    JNI_STACK_PRINTF("nativeFinalizar")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "nativeFinalizar");
    //
	PRINTF_INFO("-----------------------\n");
	PRINTF_INFO("--- nativeFinalizar ---\n");
	PRINTF_INFO("-----------------------\n");
	//
	PRINTF_INFO("Finalizando juego.\n");
	_app->finalizarJuego();	//Libera las escenas el resumeDebug
	PRINTF_INFO("Exito finalizando juego.\n");
    //
	/*if(_escenas != NULL){
		PRINTF_INFO("Liberando _escenas.\n");
		_escenas->liberar(NB_RETENEDOR_THIS);
		PRINTF_INFO("Exito liberando _escenas.\n");
		_escenas = NULL;
	}*/
    //
	PRINTF_INFO("Finalizando ventana.\n");
	_app->finalizarVentana();	//Destruye la escena
	PRINTF_INFO("Exito finalizando ventana.\n");
	//
	if(_initedMMedia){
		PRINTF_INFO("Finalizando multimedia.\n");
		_app->finalizarMultimedia();
		PRINTF_INFO("Exito finalizando multimedia.\n");
		_initedMMedia = false;
	}
	if(_initedEngine){
		if(_app != NULL){
			PRINTF_INFO("Liberando _app.\n");
			delete _app; //->liberar(NB_RETENEDOR_THIS);
			PRINTF_INFO("Exito liberando _app.\n");
			_app = NULL;
		}
		//NBGestorRefranero::finalizar();
		PRINTF_INFO("Finalizando nucleo.\n");
		AUApp::finalizarNucleo();
		PRINTF_INFO("Exito finalizando nucleo.\n");
		_initedEngine = false;
	}
	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	return JNI_TRUE;
}

//---------------
//- ServiceConnectionMonitor
//---------------
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceConnected(JNIEnv* pEnv, jobject pObj, jobject compName, jobject service){
    JNI_STACK_PRINTF("onServiceConnected")
    PRINTF_INFO("onServiceDisconnected");
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "serviceConnected");
    _app->appSrcvOnConnected(compName, service);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024ServiceConnectionMonitor_onServiceDisconnected(JNIEnv* pEnv, jobject pObj, jobject compName){
    JNI_STACK_PRINTF("serviceDisconnected")
    PRINTF_INFO("onServiceDisconnected");
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "serviceDisconnected");
    _app->appSrcvOnDisconnected(compName);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

//---------------
//- AlarmReceiver
//---------------
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AlarmReceiver_onReceive(JNIEnv *pEnv, jobject pObj, jobject pContext, jobject pIntent) {
    JNI_STACK_PRINTF("onReceive")
	PRINTF_INFO("onReceive(Intent).\n");
    if (_app != NULL) {
        const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onReceive");
        _app->appIntentReceived(pIntent);
        const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv);
        NBASSERT(qEnvsEnd == qEnvsStart)
    }
}

//---------------
//- AURunable
//---------------

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AURunnable_runNative(JNIEnv *pEnv, jobject pObj, jlong func, jlong funcParam){
	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "runNative");
	if(func != 0){
		AURunnableRunMthd func2 = (AURunnableRunMthd)func;
		(func2)((void*)funcParam);
	}
	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

//---------------
//- AUSurfaceView
//---------------

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onSurfaceCreated(JNIEnv *pEnv, jobject pObj, jobject pGl, jobject pConfig){
    JNI_STACK_PRINTF("onSurfaceCreated")
    jboolean r = JNI_FALSE;
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onSurfaceCreated");
    //
    if(!_initedMMedia){
        PRINTF_INFO("++++ INICIALIZANDO ESCENA ++++\n");
        //Inicializar media
        const float dispRefRate = AUApp::getDefaultDisplayRefreshRate(pEnv, _app->getGlueJNI()->jActivity()); PRINTF_INFO("onSurfaceCreated, refreshRate: %f.\n", dispRefRate);
        if(!_app->inicializarMultimedia(LEER_PRECACHE, LEER_CACHE, ESCRIBIR_CACHE, true/*initGraphics*/, dispRefRate)){
            PRINTF_ERROR("ERROR, no se pudo inicializar el motor grafico.\n");
        } else {
			//Configure graphics (post)
			{
				NBASSERT(_delegate != NULL)
				jclass cls = pEnv->FindClass("com/auframework/AppNative$Delegate"); NBASSERT(cls != NULL)
				if(cls != NULL){
					jmethodID mid = pEnv->GetMethodID(cls, "graphicsConfigurePost", "(J)V"); NBASSERT(mid != NULL)
					if(mid != NULL) {
						pEnv->CallVoidMethod(_delegate, mid, (jlong)_app);
					}
				}
			}
            _initedMMedia = true;
            r = JNI_TRUE;
        }
    } else {
        PRINTF_INFO("++++ RE-INICIALIZANDO ESCENA ++++\n");
        if(!_app->reinicializarMultimedia(false)){
            PRINTF_ERROR("ERROR, no se pudo REinicializar el motor grafico.\n");
        } else {
            r = JNI_TRUE;
        }
    }
    //
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onSurfaceChanged(JNIEnv *pEnv, jobject pObj, jobject pGl, jint pWidth, jint pHeight){
    JNI_STACK_PRINTF("nativeRedimensionarEscena")
    //PRINTF_INFO("nativeRedimensionarEscena(%d, %d).\n", ancho, alto);
    jboolean r = JNI_FALSE;
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "nativeRedimensionarEscena");
    if(_app == NULL){
        PRINTF_ERROR("Invocacion a nativeRedimensionarEscena con '_app == NULL'.\n");
    } else {
        if(_initedSurface) {
			float screenDensityScale    = 1.0f;
            NBTamano pppReal            = AUApp::getDefaultDisplayDensity(pEnv, _app->getGlueJNI()->jActivity());
			NBTamano pppForMetrics;		NBTAMANO_ESTABLECER(pppForMetrics, (pppReal.ancho * screenDensityScale), (pppReal.alto * screenDensityScale))
            NBTamanoI screenSz;			NBTAMANO_ESTABLECER(screenSz, pWidth, pHeight)
			PRINTF_INFO("onSurfaceChanged, displayDepth: (%f, %f).\n", pppReal.ancho, pppReal.alto);
            _app->notificarRedimensionVentana(screenSz, 1.0f, pppReal, pppForMetrics);
            r = JNI_TRUE;
        } else {
            //iPad: 132dpi (~800K pixeles)
            //iPhone & iPodTouch & iPadMini: 163dpi
            //iPhone & iPodTouch Retina & iPadMini2: 326dpi (~600K - ~750K pixeles)
            //iPad Retina: 264dpi (~3M pixeles)
            float screenDensityScale    = 1.0f;
            NBTamano pppReal            = AUApp::getDefaultDisplayDensity(pEnv, _app->getGlueJNI()->jActivity());
            PRINTF_INFO("onSurfaceChanged, displayDepth: (%f, %f).\n", pppReal.ancho, pppReal.alto);
            //Fix, visuals too small on iPad screen:
            //Forcing double size on iPad screens
            /*{
             * const float pantallaAnchoInch	= (ancho / pppReal.ancho);
				const float pantallaAltoInch	= (alto / pppReal.alto);
                const float szInchesSquared		= (pantallaAnchoInch * pantallaAnchoInch) + (pantallaAltoInch * pantallaAltoInch);
                if(szInchesSquared > (7.0f * 7.0f)){
                    screenDensityScale = 2.0f;
                }
            }*/
            NBTamano pppForMetrics;	NBTAMANO_ESTABLECER(pppForMetrics, (pppReal.ancho * screenDensityScale), (pppReal.alto * screenDensityScale))
            NBTamanoI screenSz;		NBTAMANO_ESTABLECER(screenSz, pWidth, pHeight)
            if(!_app->inicializarVentana(screenSz, pppReal, pppForMetrics, ENGestorEscenaDestinoGl_Heredado)){
                PRINTF_ERROR("_app->inicializarVentana(%d, %d) ha fallado.\n", pWidth, pHeight);
            } else {
                PRINTF_INFO("_app->inicializarVentana(%d, %d) exitoso.\n", pWidth, pHeight);
				//Create scenesManager
				AUAppEscenasAdmin* scenesMngr = NULL;
				{
					NBASSERT(_delegate != NULL)
					jclass cls = pEnv->FindClass("com/auframework/AppNative$Delegate"); NBASSERT(cls != NULL)
					if(cls != NULL){
						jmethodID mid = pEnv->GetMethodID(cls, "scenesManagerCreate", "(J)J"); NBASSERT(mid != NULL)
						if(mid != NULL) {
							scenesMngr = (AUAppEscenasAdmin*)pEnv->CallLongMethod(_delegate, mid, (jlong)_app);
						}
					}
				}
				NBASSERT(scenesMngr != NULL)
				if(scenesMngr != NULL){
					if(!_app->inicializarJuego(scenesMngr)){
						PRINTF_ERROR("_app->inicializarJuego() ha fallado.\n");
					} else {
						PRINTF_INFO("_app->inicializarJuego(%d, %d) exitoso.\n", pWidth, pHeight);
						r = JNI_TRUE;
					}
				}
            }
			_initedSurface = true;
        }
    }
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return r;
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onDrawFrame(JNIEnv *pEnv, jobject pObj, jobject pGl){
    JNI_STACK_PRINTF("onDrawFrame")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onDrawFrame");
    _app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_doFrameNative(JNIEnv *pEnv, jobject pObj, jlong frameTimeNanos){
	JNI_STACK_PRINTF("doFrameNative")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "doFrameNative");
	//PRINTF_INFO("doFrameNative after %ld nanoseconds.\n", frameTimeNanos);
    //_app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onTouchEvent(JNIEnv* pEnv, jobject pObj, jobject pEvent){
    JNI_STACK_PRINTF("onTouchEvent")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onTouchEvent");
    jboolean r = (_app->onTouchEvent(pEnv, pEvent) ? JNI_TRUE : JNI_FALSE);
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onCheckIsTextEditor(JNIEnv* pEnv, jobject pObj){
    JNI_STACK_PRINTF("onCheckIsTextEditor")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onCheckIsTextEditor");
    //Nothing
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return JNI_FALSE;
}

jint getStaticIntMember(JNIEnv* pEnv, jclass clsInType, const char* memberName){
	jint r = 0;
	if(clsInType != NULL){
		jfieldID jFld = pEnv->GetStaticFieldID(clsInType, memberName, "I"); NBASSERT(jFld != NULL)
		if(jFld == NULL){
			PRINTF_ERROR("Could not find static int-member '%s' at class.\n", memberName);
			if(pEnv->ExceptionCheck()) pEnv->ExceptionClear(); //consume Exception
		} else {
			r = pEnv->GetStaticIntField(clsInType, jFld);
		}
	}
	return r;
}

JNIEXPORT jobject /*InputConnection*/ JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onCreateInputConnectionWithClass(JNIEnv * pEnv, jobject pObj, jobject outAttrs /*EditorInfo*/, jobject objClass /*Class*/){
    JNI_STACK_PRINTF("onCreateInputConnectionWithClass")
    jobject r = NULL;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onCreateInputConnectionWithClass");
		jclass clsEdInfo = pEnv->FindClass("android/view/inputmethod/EditorInfo"); NBASSERT(clsEdInfo != NULL)
		jclass clsInType = pEnv->FindClass("android/text/InputType"); NBASSERT(clsInType != NULL)
		if(clsEdInfo != NULL && clsInType != NULL) {
			jfieldID fImeOpt    = pEnv->GetFieldID(clsEdInfo, "imeOptions", "I"); NBASSERT(fImeOpt != NULL)
			jfieldID fInType    = pEnv->GetFieldID(clsEdInfo, "inputType", "I"); NBASSERT(fInType != NULL)
			jfieldID fActLbl  	= pEnv->GetFieldID(clsEdInfo, "actionLabel", "Ljava/lang/CharSequence;"); NBASSERT(fActLbl != NULL)
			jfieldID fSelStart  = pEnv->GetFieldID(clsEdInfo, "initialSelStart", "I"); NBASSERT(fSelStart != NULL)
			jfieldID fSelEnd  	= pEnv->GetFieldID(clsEdInfo, "initialSelEnd", "I"); NBASSERT(fSelEnd != NULL)
			if(fImeOpt != NULL && fInType != NULL && fActLbl != NULL && fSelStart != NULL && fSelEnd != NULL){
				const NBRangoI rng = NBGestorTeclas::entradaRangoSeleccionAbs();
				pEnv->SetIntField(outAttrs, fSelStart, rng.inicio);
				pEnv->SetIntField(outAttrs, fSelEnd, rng.inicio + rng.tamano);
				pEnv->SetObjectField(outAttrs, fActLbl, pEnv->NewStringUTF("OK"));
				//
				if(AUAppGlueAndroidJNI::getAPICurrent(pEnv) >= AUAppGlueAndroidJNI::getAPIByName(pEnv, "HONEYCOMB")){ //Api 11
					jfieldID fNoFullScrenn	= pEnv->GetStaticFieldID(clsEdInfo, "IME_FLAG_NO_FULLSCREEN", "I"); NBASSERT(fNoFullScrenn != NULL)
					if(fNoFullScrenn != NULL){
						jint noFullScrenn = pEnv->GetStaticIntField(clsEdInfo, fNoFullScrenn);
						pEnv->SetIntField(outAttrs, fImeOpt, noFullScrenn);
					}
				}
				//Build type
				//Note: activate all "TYPE_TEXT_FLAG_MULTI_LINE" to enable hidding the keyboard with "enter"
				{
					jint inputType = 0; BOOL isMultiline = FALSE;
					const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(&isMultiline);
					switch (nbType) {
						case ENTextEditorType_Literal:	//Text will be literal
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_TEXT") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_VARIATION_NORMAL") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_NO_SUGGESTIONS") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE"); // (isMultiline ? getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE") : 0);
							break;
						case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_TEXT") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_VARIATION_PASSWORD") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_NO_SUGGESTIONS") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE"); //(isMultiline ? getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE") : 0);
							break;
						case ENTextEditorType_Compose:	//Text will be autocompleted
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_TEXT") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_VARIATION_LONG_MESSAGE") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_CAP_SENTENCES") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_AUTO_COMPLETE") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE"); // (isMultiline ? getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE") : 0);
							break;
						case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_TEXT") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_VARIATION_EMAIL_ADDRESS") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE");
							break;
						case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_PHONE") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE");
							break;
						case ENTextEditorType_Integer:	//Integer number
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_NUMBER") | getStaticIntMember(pEnv, clsInType, "TYPE_NUMBER_FLAG_SIGNED") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE");
							break;
						case ENTextEditorType_Decimal:	//Decimal number
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_NUMBER") | getStaticIntMember(pEnv, clsInType, "TYPE_NUMBER_FLAG_SIGNED") | getStaticIntMember(pEnv, clsInType, "TYPE_NUMBER_FLAG_DECIMAL") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE");
							break;
						case ENTextEditorType_Name:		//Names (first letter capitalized)
							inputType = getStaticIntMember(pEnv, clsInType, "TYPE_CLASS_TEXT") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_VARIATION_PERSON_NAME") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_CAP_WORDS") | getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE"); //(isMultiline ? getStaticIntMember(pEnv, clsInType, "TYPE_TEXT_FLAG_MULTI_LINE") : 0);
							break;
						default:
							break;
					}
					if(inputType != 0){
						pEnv->SetIntField(outAttrs, fInType, inputType);
					}
				}
				//
				r = (jobject)AUAppGlueAndroidJNI::getNewInstanceOfClass(pEnv, objClass); NBASSERT(r != NULL)
			}
			/*
			 int	IME_FLAG_FORCE_ASCII	Flag of imeOptions: used to request an IME that is capable of inputting ASCII characters.
			 int	IME_FLAG_NAVIGATE_NEXT	Flag of imeOptions: used to specify that there is something interesting that a forward navigation can focus on.
			 int	IME_FLAG_NAVIGATE_PREVIOUS	Flag of imeOptions: like IME_FLAG_NAVIGATE_NEXT, but specifies there is something interesting that a backward navigation can focus on.
			 int	IME_FLAG_NO_ACCESSORY_ACTION	Flag of imeOptions: used in conjunction with one of the actions masked by IME_MASK_ACTION, this indicates that the action should not be available as an accessory button on the right of the extracted text when the input method is full-screen.
			 int	IME_FLAG_NO_ENTER_ACTION	Flag of imeOptions: used in conjunction with one of the actions masked by IME_MASK_ACTION.
			 int	IME_FLAG_NO_EXTRACT_UI	Flag of imeOptions: used to specify that the IME does not need to show its extracted text UI.
			 int	IME_FLAG_NO_FULLSCREEN	Flag of imeOptions: used to request that the IME never go into fullscreen mode.
			 */
			/*
			 int	TYPE_CLASS_DATETIME	Class for dates and times.
			 int	TYPE_CLASS_NUMBER	Class for numeric text.
			 int	TYPE_CLASS_PHONE	Class for a phone number.
			 int	TYPE_CLASS_TEXT	Class for normal text.
			 //
			 TYPE_DATETIME_VARIATION_DATE	Default variation of TYPE_CLASS_DATETIME: allows entering only a date.
			 TYPE_DATETIME_VARIATION_NORMAL	Default variation of TYPE_CLASS_DATETIME: allows entering both a date and time.
			 TYPE_DATETIME_VARIATION_TIME	Default variation of TYPE_CLASS_DATETIME: allows entering only a time.
			 //
			 TYPE_NUMBER_FLAG_DECIMAL		Flag of TYPE_CLASS_NUMBER: the number is decimal, allowing a decimal point to provide fractional values.
			 TYPE_NUMBER_FLAG_SIGNED			Flag of TYPE_CLASS_NUMBER: the number is signed, allowing a positive or negative sign at the start.
			 TYPE_NUMBER_VARIATION_NORMAL	Default variation of TYPE_CLASS_NUMBER: plain normal numeric text.
			 TYPE_NUMBER_VARIATION_PASSWORD	Variation of TYPE_CLASS_NUMBER: entering a numeric password.
			 //
			 int	TYPE_TEXT_FLAG_AUTO_COMPLETE	Flag for TYPE_CLASS_TEXT: the text editor (which means the application) is performing auto-completion of the text being entered based on its own semantics, which it will present to the user as they type.
			 int	TYPE_TEXT_FLAG_AUTO_CORRECT	Flag for TYPE_CLASS_TEXT: the user is entering free-form text that should have auto-correction applied to it.
			 int	TYPE_TEXT_FLAG_CAP_CHARACTERS	Flag for TYPE_CLASS_TEXT: capitalize all characters.
			 int	TYPE_TEXT_FLAG_CAP_SENTENCES	Flag for TYPE_CLASS_TEXT: capitalize the first character of each sentence.
			 int	TYPE_TEXT_FLAG_CAP_WORDS	Flag for TYPE_CLASS_TEXT: capitalize the first character of every word.
			 int	TYPE_TEXT_FLAG_IME_MULTI_LINE	Flag for TYPE_CLASS_TEXT: the regular text view associated with this should not be multi-line, but when a fullscreen input method is providing text it should use multiple lines if it can.
			 int	TYPE_TEXT_FLAG_MULTI_LINE	Flag for TYPE_CLASS_TEXT: multiple lines of text can be entered into the field.
			 int	TYPE_TEXT_FLAG_NO_SUGGESTIONS	Flag for TYPE_CLASS_TEXT: the input method does not need to display any dictionary-based candidates.
			 //
			 int	TYPE_TEXT_VARIATION_EMAIL_ADDRESS	Variation of TYPE_CLASS_TEXT: entering an e-mail address.
			 int	TYPE_TEXT_VARIATION_EMAIL_SUBJECT	Variation of TYPE_CLASS_TEXT: entering the subject line of an e-mail.
			 int	TYPE_TEXT_VARIATION_FILTER	Variation of TYPE_CLASS_TEXT: entering text to filter contents of a list etc.
			 int	TYPE_TEXT_VARIATION_LONG_MESSAGE	Variation of TYPE_CLASS_TEXT: entering the content of a long, possibly formal message such as the body of an e-mail.
			 int	TYPE_TEXT_VARIATION_NORMAL	Default variation of TYPE_CLASS_TEXT: plain old normal text.
			 int	TYPE_TEXT_VARIATION_PASSWORD	Variation of TYPE_CLASS_TEXT: entering a password.
			 int	TYPE_TEXT_VARIATION_PERSON_NAME	Variation of TYPE_CLASS_TEXT: entering the name of a person.
			 int	TYPE_TEXT_VARIATION_PHONETIC	Variation of TYPE_CLASS_TEXT: entering text for phonetic pronunciation, such as a phonetic name field in contacts.
			 int	TYPE_TEXT_VARIATION_POSTAL_ADDRESS	Variation of TYPE_CLASS_TEXT: entering a postal mailing address.
			 int	TYPE_TEXT_VARIATION_SHORT_MESSAGE	Variation of TYPE_CLASS_TEXT: entering a short, possibly informal message such as an instant message or a text message.
			 int	TYPE_TEXT_VARIATION_URI	Variation of TYPE_CLASS_TEXT: entering a URI.
			 int	TYPE_TEXT_VARIATION_VISIBLE_PASSWORD	Variation of TYPE_CLASS_TEXT: entering a password, which should be visible to the user.
			 int	TYPE_TEXT_VARIATION_WEB_EDIT_TEXT	Variation of TYPE_CLASS_TEXT: entering text inside of a web form.
			 int	TYPE_TEXT_VARIATION_WEB_EMAIL_ADDRESS	Variation of TYPE_CLASS_TEXT: entering e-mail address inside of a web form.
			 int	TYPE_TEXT_VARIATION_WEB_PASSWORD	Variation of TYPE_CLASS_TEXT: entering password inside of a web form.
			 */
		}
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onKeyDown(JNIEnv *pEnv, jobject pObj, jint pKeyCode, jobject pKeyEvent){
    JNI_STACK_PRINTF("onKeyDown")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onKeyDown");
    jboolean r = (_app->onKeyDown(pEnv, pKeyCode, pKeyEvent) ? JNI_TRUE: JNI_FALSE);
    if(r == JNI_FALSE){
        //Call super
        jclass clsObj = pEnv->GetObjectClass(pObj); NBASSERT(clsObj != NULL)
        jclass clsSupr = pEnv->GetSuperclass(clsObj); NBASSERT(clsSupr != NULL)
        jmethodID mSuper = pEnv->GetMethodID(clsSupr, "onKeyDown", "(ILandroid/view/KeyEvent;)Z"); NBASSERT(mSuper != NULL)
        r = pEnv->CallNonvirtualBooleanMethod(pObj, clsSupr, mSuper, pKeyCode, pKeyEvent);
    }
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onKeyUp(JNIEnv *pEnv, jobject pObj, jint pKeyCode, jobject pKeyEvent){
    JNI_STACK_PRINTF("onKeyUp")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onKeyUp");
    jboolean r = (_app->onKeyUp(pEnv, pKeyCode, pKeyEvent) ? JNI_TRUE: JNI_FALSE);
    if(r == JNI_FALSE){
        //Call super
        jclass clsObj = pEnv->GetObjectClass(pObj); NBASSERT(clsObj != NULL)
        jclass clsSupr = pEnv->GetSuperclass(clsObj); NBASSERT(clsSupr != NULL)
        jmethodID mSuper = pEnv->GetMethodID(clsSupr, "onKeyUp", "(ILandroid/view/KeyEvent;)Z"); NBASSERT(mSuper != NULL)
        r = pEnv->CallNonvirtualBooleanMethod(pObj, clsSupr, mSuper, pKeyCode, pKeyEvent);
    }
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
    return r;
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onWindowFocusChanged(JNIEnv *pEnv, jobject pObj, jboolean hasFocus){
    JNI_STACK_PRINTF("onWindowFocusChanged")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onWindowFocusChanged");
    _app->onWindowFocusChanged(pEnv, (SI32)hasFocus);
    {
        //Call super
        jclass clsObj = pEnv->GetObjectClass(pObj); NBASSERT(clsObj != NULL)
        jclass clsSupr = pEnv->GetSuperclass(clsObj); NBASSERT(clsSupr != NULL)
        jmethodID mSuper = pEnv->GetMethodID(clsSupr, "onWindowFocusChanged",  "(Z)V"); NBASSERT(mSuper != NULL)
        pEnv->CallNonvirtualVoidMethod(pObj, clsSupr, mSuper, hasFocus);
    }
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_onFocusChange(JNIEnv *pEnv, jobject pObj, jobject pView, jboolean hasFocus){
    JNI_STACK_PRINTF("onFocusChange")
    const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "onFocusChange");
    _app->onFocusChange(pEnv, pView, (SI32)hasFocus);
    //No super method (this is a interface method)
    const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUSurfaceView_setKeyboardHeight(JNIEnv *pEnv, jobject pObj, jfloat height, jboolean overlapsContent){
	JNI_STACK_PRINTF("setKeyboardHeight")
	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "setKeyboardHeight");
	_app->setKeyboardHeight(pEnv, height, overlapsContent);
	//No super method (this is a interface method)
	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
}

//---------------
//- CameraDevice.StateCallback
//---------------
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onClosed(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	JNI_STACK_PRINTF("CameraStateListener_onClosed")
	PRINTF_INFO("CameraStateListener_onClosed.\n");
	AUAppGlueAndroidAVCapture::CameraStateListener_onClosed(pEnv, pObj, jCamera, dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onDisconnected(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	JNI_STACK_PRINTF("CameraStateListener_onDisconnected")
	PRINTF_INFO("CameraStateListener_onDisconnected.\n");
	AUAppGlueAndroidAVCapture::CameraStateListener_onDisconnected(pEnv, pObj, jCamera, dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onError(JNIEnv *pEnv, jobject pObj, jobject jCamera, jint error, jlong dataPtr){
	JNI_STACK_PRINTF("CameraStateListener_onError")
	PRINTF_INFO("CameraStateListener_onError.\n");
	AUAppGlueAndroidAVCapture::CameraStateListener_onError(pEnv, pObj, jCamera, error, dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CameraStateListener__1onOpened(JNIEnv *pEnv, jobject pObj, jobject jCamera, jlong dataPtr){
	JNI_STACK_PRINTF("CameraStateListener_onOpened")
	PRINTF_INFO("CameraStateListener_onOpened.\n");
	AUAppGlueAndroidAVCapture::CameraStateListener_onOpened(pEnv, pObj, jCamera, dataPtr);
}

//------------------------
//- CameraCaptureSession.StateCallback (API 21)
//------------------------
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onActive(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onActive")
	PRINTF_INFO("CaptureStateListener_onActive.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onActive(pEnv, pObj, jSession, dataPtr);
	PRINTF_INFO("CaptureStateListener_onActive (end).\n");
}
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onCaptureQueueEmpty(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onCaptureQueueEmpty")
	PRINTF_INFO("CaptureStateListener_onCaptureQueueEmpty.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onCaptureQueueEmpty(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onClosed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onClosed")
	PRINTF_INFO("CaptureStateListener_onClosed.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onClosed(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onConfigureFailed(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onConfigureFailed")
	PRINTF_INFO("CaptureStateListener_onConfigureFailed.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onConfigureFailed(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onConfigured(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onConfigured")
	PRINTF_INFO("CaptureStateListener_onConfigured.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onConfigured(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onReady(JNIEnv *pEnv, jobject pObj, jobject jSession, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onReady")
	PRINTF_INFO("CaptureStateListener_onReady.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onReady(pEnv, pObj, jSession, dataPtr);
}
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024CaptureStateListener__1onSurfacePrepared(JNIEnv *pEnv, jobject pObj, jobject jSession, jobject jSurface, jlong dataPtr){
	JNI_STACK_PRINTF("CaptureStateListener_onSurfacePrepared")
	PRINTF_INFO("CaptureStateListener_onSurfacePrepared.\n");
	AUAppGlueAndroidAVCapture::CaptureStateListener_onSurfacePrepared(pEnv, pObj, jSession, jSurface, dataPtr);
}
//------------------------
//- ImageReader.OnImageAvailableListener (API 19+)
//------------------------
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024ImageReaderListener__1onImageAvailable(JNIEnv *pEnv, jobject pObj, jobject jReader, jlong dataPtr){
	JNI_STACK_PRINTF("ImageReaderListener_onImageAvailable")
	//PRINTF_INFO("ImageReaderListener_onImageAvailable.\n");
	AUAppGlueAndroidAVCapture::ImageReaderListener_onImageAvailable(pEnv, pObj, jReader, dataPtr);
}
//------------------------
//- FingerprintManager.AuthenticationCallback (API 23-28)
//------------------------
//private native void _onAuthenticationError(int errorCode, CharSequence errString, long dataPtr);
//private native void _onAuthenticationFailed(long dataPtr);
//private native void _onAuthenticationHelp(int helpCode, CharSequence helpString, long dataPtr);
//private native void _onAuthenticationSucceeded(android.hardware.fingerprint.FingerprintManager.AuthenticationResult result, long dataPtr);

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationError(JNIEnv *pEnv, jobject pObj, jint errorCode, jobject errString, jlong dataPtr){
	JNI_STACK_PRINTF("FingerAuthListener_onAuthenticationError")
	PRINTF_INFO("FingerAuthListener_onAuthenticationError.\n");
	AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationError(pEnv, pObj, errorCode, errString, dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationFailed(JNIEnv *pEnv, jobject pObj, jlong dataPtr){
	JNI_STACK_PRINTF("FingerAuthListener_onAuthenticationFailed")
	PRINTF_INFO("FingerAuthListener_onAuthenticationFailed.\n");
	AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationFailed(pEnv, pObj, dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationHelp(JNIEnv *pEnv, jobject pObj, jint helpCode, jobject errString, jlong dataPtr){
	JNI_STACK_PRINTF("FingerAuthListener_onAuthenticationHelp")
	PRINTF_INFO("FingerAuthListener_onAuthenticationHelp.\n");
	AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationHelp(pEnv, pObj, helpCode, errString, dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024FingerAuthListener__1onAuthenticationSucceeded(JNIEnv *pEnv, jobject pObj, jobject result, jlong dataPtr){
	JNI_STACK_PRINTF("FingerAuthListener_onAuthenticationSucceeded")
	PRINTF_INFO("FingerAuthListener_onAuthenticationSucceeded.\n");
	AUAppGlueAndroidBiometrics::FingerAuthListener_onAuthenticationSucceeded(pEnv, pObj, result, dataPtr);
}

//------------------------
// Billing library listener
//------------------------

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingSetupFinished(JNIEnv *pEnv, jobject pObj, jobject billingResult, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onBillingSetupFinished")
	PRINTF_INFO("AUBillingClientStateListener_onBillingSetupFinished.\n");
	AUAppGlueAndroidStore::onBillingSetupFinished((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onBillingServiceDisconnected(JNIEnv *pEnv, jobject pObj, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onBillingServiceDisconnected")
	PRINTF_INFO("AUBillingClientStateListener_onBillingServiceDisconnected.\n");
	AUAppGlueAndroidStore::onBillingServiceDisconnected((void*)pEnv, (void*)pObj, (void*)dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onSkuDetailsResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject skuDetailsList, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onSkuDetailsResponse")
	PRINTF_INFO("AUBillingClientStateListener_onSkuDetailsResponse.\n");
	AUAppGlueAndroidStore::onSkuDetailsResponse((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)skuDetailsList, (void*)dataPtr);
}

//v4
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onQueryPurchasesResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchases, jlong dataPtr){
    JNI_STACK_PRINTF("AUBillingClientStateListener_onQueryPurchasesResponse")
    PRINTF_INFO("AUBillingClientStateListener_onQueryPurchasesResponse.\n");
    AUAppGlueAndroidStore::onQueryPurchasesResponse((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)purchases, (void*)dataPtr);
}

//v3
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchasesUpdated(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchases, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onPurchasesUpdated")
	PRINTF_INFO("AUBillingClientStateListener_onPurchasesUpdated.\n");
	AUAppGlueAndroidStore::onPurchasesUpdated((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)purchases, (void*)dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onAcknowledgePurchaseResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onAcknowledgePurchaseResponse")
	PRINTF_INFO("AUBillingClientStateListener_onAcknowledgePurchaseResponse.\n");
	AUAppGlueAndroidStore::onAcknowledgePurchaseResponse((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)dataPtr);
}

JNIEXPORT void Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onConsumeResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchaseToken, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onConsumeResponse")
	PRINTF_INFO("AUBillingClientStateListener_onConsumeResponse.\n");
	AUAppGlueAndroidStore::onConsumeResponse((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)purchaseToken, (void*)dataPtr);
}

JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUBillingClientStateListener__1onPurchaseHistoryResponse(JNIEnv *pEnv, jobject pObj, jobject billingResult, jobject purchaseHistoryRecordList, jlong dataPtr){
	JNI_STACK_PRINTF("AUBillingClientStateListener_onPurchaseHistoryResponse")
	PRINTF_INFO("AUBillingClientStateListener_onPurchaseHistoryResponse.\n");
	AUAppGlueAndroidStore::onPurchaseHistoryResponse((void*)pEnv, (void*)pObj, (void*)billingResult, (void*)purchaseHistoryRecordList, (void*)dataPtr);
}

//---------------
//- AUInputConnection
//---------------

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_beginBatchEdit(JNIEnv* pEnv, jobject pObj){
    JNI_STACK_KEYINPUT_PRINTF("beginBatchEdit")
	jboolean r = JNI_FALSE;
	NBGestorTeclas::entradaLockForBatch();
	{
    	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "beginBatchEdit");
    	const SI32 depth = NBGestorTeclas::entradaExplicitBashPush();
    	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
		r = (depth > 0 ? JNI_TRUE : JNI_FALSE);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_endBatchEdit(JNIEnv* pEnv, jobject pObj){
    JNI_STACK_KEYINPUT_PRINTF("entradaExplicitBashPop")
	jboolean r = JNI_FALSE;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "entradaExplicitBashPop");
		const SI32 depth = NBGestorTeclas::entradaExplicitBashPop();
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
		r = (depth > 0 ? JNI_TRUE : JNI_FALSE);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_clearMetaKeyStates(JNIEnv* pEnv, jobject pObj, jint states){
    JNI_STACK_KEYINPUT_PRINTF("clearMetaKeyStates")
	NBGestorTeclas::entradaLockForBatch();
	{
    	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "clearMetaKeyStates");
    	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitCompletion(JNIEnv* pEnv, jobject pObj, jobject /*CompletionInfo*/ text){
    JNI_STACK_KEYINPUT_PRINTF("commitCompletion")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "commitCompletion");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitCorrection(JNIEnv* pEnv, jobject pObj, jobject /*CorrectionInfo*/ correctionInfo){
    JNI_STACK_KEYINPUT_PRINTF("commitCorrection")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "commitCorrection");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitText(JNIEnv* pEnv, jobject pObj, jobject /*CharSequence*/ text, jint newCursorPosition){
    JNI_STACK_KEYINPUT_PRINTF("commitText")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart	= _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "commitText");
		const NBRangoI rng		= NBGestorTeclas::entradaRangoMarcadoAbs();
		{
			jclass clzCharSeq	= pEnv->GetObjectClass(text);
			jmethodID toString	= pEnv->GetMethodID(clzCharSeq, "toString", "()Ljava/lang/String;");
			jobject jStr		= pEnv->CallObjectMethod(text, toString);
			{
				const char* str8 = pEnv->GetStringUTFChars((jstring)jStr, 0);
				PRINTF_INFO("commitText: '%s'.\n", str8);
				if(rng.tamano == 0){
					NBGestorTeclas::entradaIntroducirTexto(str8, false);
				} else {
					NBGestorTeclas::entradaTextoMarcadoReemplazar(str8, ENTextRangeSet_None);
				}
				pEnv->ReleaseStringUTFChars((jstring)jStr, str8);
			}
		}
		const SI32 qEnvsEnd		= _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingText(JNIEnv* pEnv, jobject pObj, jint beforeLength, jint afterLength){
    JNI_STACK_KEYINPUT_PRINTF("deleteSurroundingText")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "deleteSurroundingText");
		NBGestorTeclas::entradaTextoSeleccionEliminarAlrededor((SI32)beforeLength, (SI32)afterLength, false);
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_finishComposingText(JNIEnv* pEnv, jobject pObj){
    JNI_STACK_KEYINPUT_PRINTF("finishComposingText")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "finishComposingText");
		NBGestorTeclas::entradaRangoDesmarcar();
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jint JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getCursorCapsMode(JNIEnv* pEnv, jobject pObj, jint reqModes){
    JNI_STACK_KEYINPUT_PRINTF("getCursorCapsMode")
	jint r = reqModes;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "getCursorCapsMode");
		{
			jclass clsInputType = pEnv->FindClass("android/text/InputType"); NBASSERT(clsInputType != NULL)
			if(clsInputType != NULL) {
				//InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS
				//InputType.TYPE_TEXT_FLAG_CAP_SENTENCES
				//InputType.TYPE_TEXT_FLAG_CAP_WORDS
				jfieldID fFlag	= pEnv->GetStaticFieldID(clsInputType, "TYPE_TEXT_FLAG_CAP_SENTENCES", "I"); NBASSERT(fFlag != NULL)
				if(fFlag != NULL){
					r = pEnv->GetStaticIntField(clsInputType, fFlag);
				}
			}
		}
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT jobject /*ExtractedText*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getExtractedText(JNIEnv* pEnv, jobject pObj, jobject /*ExtractedTextRequest*/ request, jint flags){
    JNI_STACK_KEYINPUT_PRINTF("getExtractedText")
    jobject r = NULL;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "getExtractedText");
		{
			jclass clsExTxt = pEnv->FindClass("android/view/inputmethod/ExtractedText"); NBASSERT(clsExTxt != NULL)
			if(clsExTxt != NULL) {
				jmethodID mInit		= pEnv->GetMethodID(clsExTxt, "<init>", "()V"); NBASSERT(mInit != NULL)
				jfieldID fFlags     = pEnv->GetFieldID(clsExTxt, "flags", "I"); NBASSERT(fFlags != NULL)
				jfieldID fPartStart	= pEnv->GetFieldID(clsExTxt, "partialStartOffset", "I"); NBASSERT(fPartStart != NULL)
				jfieldID fPartEnd	= pEnv->GetFieldID(clsExTxt, "partialEndOffset", "I"); NBASSERT(fPartEnd != NULL)
				jfieldID fStart     = pEnv->GetFieldID(clsExTxt, "startOffset", "I"); NBASSERT(fStart != NULL)
				jfieldID fSelStart	= pEnv->GetFieldID(clsExTxt, "selectionStart", "I"); NBASSERT(fSelStart != NULL)
				jfieldID fSelEnd	= pEnv->GetFieldID(clsExTxt, "selectionEnd", "I"); NBASSERT(fSelEnd != NULL)
				jfieldID fText  	= pEnv->GetFieldID(clsExTxt, "text", "Ljava/lang/CharSequence;"); NBASSERT(fText != NULL)
				if(mInit != NULL && fFlags != NULL && fPartStart != NULL && fPartEnd != NULL && fStart != NULL && fSelStart != NULL && fSelEnd != NULL && fText != NULL){
					r = pEnv->NewObject(clsExTxt, mInit); NBASSERT(r != NULL)
					if(r != NULL){
						pEnv->SetIntField(r, fFlags, 0);
						pEnv->SetIntField(r, fPartStart, 0);
						pEnv->SetIntField(r, fPartEnd, 0);
						pEnv->SetIntField(r, fStart, 0);
						//
						const NBRangoI rng = NBGestorTeclas::entradaRangoSeleccionAbs();
						AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
						NBGestorTeclas::entradaTexto(strTmp);
						pEnv->SetObjectField(r, fText, pEnv->NewStringUTF(strTmp->str()));
						strTmp->liberar(NB_RETENEDOR_THIS);
						//
						pEnv->SetIntField(r, fPartStart, rng.inicio);
						pEnv->SetIntField(r, fPartEnd, rng.inicio + rng.tamano - 1);
					}
				}
			}
		}
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT jobject /*CharSequence*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getSelectedText(JNIEnv* pEnv, jobject pObj, jint flags){
    JNI_STACK_KEYINPUT_PRINTF("getSelectedText")
	jstring r = NULL;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "getSelectedText");
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoSeleccion(strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar(NB_RETENEDOR_THIS);
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT /*CharSequence*/ jobject JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getTextAfterCursor(JNIEnv* pEnv, jobject pObj, jint length, jint flags){
    JNI_STACK_KEYINPUT_PRINTF("getTextAfterCursor")
    jstring r = NULL;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "getTextAfterCursor");
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoDespuesDeSeleccion((SI32)length, strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar(NB_RETENEDOR_THIS);
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT jobject /*CharSequence*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getTextBeforeCursor(JNIEnv* pEnv, jobject pObj, jint length, jint flags){
    JNI_STACK_KEYINPUT_PRINTF("getTextBeforeCursor")
    jstring r = NULL;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "getTextBeforeCursor");
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoAntesDeSeleccion((SI32)length, strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar(NB_RETENEDOR_THIS);
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return r;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_performContextMenuAction(JNIEnv* pEnv, jobject pObj, jint pId){
    JNI_STACK_KEYINPUT_PRINTF("performContextMenuAction")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "performContextMenuAction");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_performEditorAction(JNIEnv* pEnv, jobject pObj, jint actionCode){
    JNI_STACK_KEYINPUT_PRINTF("performEditorAction")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "performEditorAction");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_performPrivateCommand(JNIEnv* pEnv, jobject pObj, jobject /*String*/ action, jobject /*Bundle*/ data){
    JNI_STACK_KEYINPUT_PRINTF("performPrivateCommand")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "performPrivateCommand");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_reportFullscreenMode(JNIEnv* pEnv, jobject pObj, jboolean enabled){
    JNI_STACK_KEYINPUT_PRINTF("reportFullscreenMode")
	NBGestorTeclas::entradaLockForBatch();
	{
    	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "reportFullscreenMode");
    	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

// (API 25+)
JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_requestCursorUpdates(JNIEnv* pEnv, jobject pObj, jint cursorUpdateMode){
    JNI_STACK_KEYINPUT_PRINTF("requestCursorUpdates")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "requestCursorUpdates");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_sendKeyEvent(JNIEnv* pEnv, jobject pObj, jobject /*KeyEvent*/ event){
    JNI_STACK_KEYINPUT_PRINTF("sendKeyEvent")
	jboolean r = FALSE;
	const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "sendKeyEvent");
	{
		jclass clsKeyEv = pEnv->FindClass("android/view/KeyEvent"); NBASSERT(clsKeyEv != NULL)
		if(clsKeyEv != NULL) {
			jmethodID mGetAct	= pEnv->GetMethodID(clsKeyEv, "getAction", "()I"); NBASSERT(mGetAct != NULL)
			jmethodID mGetKey	= pEnv->GetMethodID(clsKeyEv, "getKeyCode", "()I"); NBASSERT(mGetKey != NULL)
			if(mGetAct != NULL && mGetKey != NULL){
				jint action         = pEnv->CallIntMethod(event, mGetAct);
				jint keyCode        = pEnv->CallIntMethod(event, mGetKey);
				PRINTF_INFO("action(%d) keyCode(%d).\n", action, keyCode);
				switch (action) {
					case 0: //ACTION_DOWN
						if(_app->onKeyDown(pEnv, keyCode, NULL)){
							r = TRUE;
						}
						break;
					case 1: //ACTION_UP
						if(_app->onKeyUp(pEnv, keyCode, NULL)){
							r = TRUE;
						}
						break;
					case 2: //ACTION_MULTIPLE
						//This constant was deprecated in API level 29.
						break;
					default:
						break;
				}
			}
		}
	}
	const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	//ToDo: remove
	/*{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "sendKeyEvent");
		{
			jclass clsKeyEv = pEnv->FindClass("android/view/KeyEvent"); NBASSERT(clsKeyEv != NULL)
			if(clsKeyEv != NULL) {
				jmethodID mGetAct	= pEnv->GetMethodID(clsKeyEv, "getAction", "()I"); NBASSERT(mGetAct != NULL)
				jmethodID mGetKey	= pEnv->GetMethodID(clsKeyEv, "getKeyCode", "()I"); NBASSERT(mGetKey != NULL)
				if(mGetAct != NULL && mGetKey != NULL){
					jint action         = pEnv->CallIntMethod(event, mGetAct);
					jint keyCode        = pEnv->CallIntMethod(event, mGetKey);
					PRINTF_INFO("action(%d) keyCode(%d).\n", action, keyCode);
					switch (action) {
						case 0: //ACTION_DOWN
							
							break;
						case 1: //ACTION_UP
							break;
						case 2: //ACTION_MULTIPLE
							//This constant was deprecated in API level 29.
							break;
						default:
							break;
					}
					if(action == 0){
						
					}
				}
			}
		}
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}*/
    return r;
}

// (API 9+)
JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_setComposingRegion(JNIEnv* pEnv, jobject pObj, jint start, jint end){
    JNI_STACK_KEYINPUT_PRINTF("setComposingRegion")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "setComposingRegion");
		PRINTF_INFO("setComposingRegion(%d, %d) aka(%d, +%d).\n", start, end, start, (end - start));
		if(start > end){ jint tmp = start; start = end; end = tmp; }
		NBGestorTeclas::entradaRangoMarcadoEstablecer(start, (end - start + 1));
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_setComposingText(JNIEnv* pEnv, jobject pObj, jobject /*CharSequence*/ text, jint newCursorPosition){
    JNI_STACK_KEYINPUT_PRINTF("setComposingText")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart	= _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "setComposingText");
		{
			jclass clzCharSeq	= pEnv->GetObjectClass(text);
			jmethodID toString	= pEnv->GetMethodID(clzCharSeq, "toString", "()Ljava/lang/String;");
			jobject jStr		= pEnv->CallObjectMethod(text, toString);
			{
				const char* str8 = pEnv->GetStringUTFChars((jstring)jStr, 0);
				NBGestorTeclas::entradaTextoMarcadoReemplazar(str8, ENTextRangeSet_Current);
				pEnv->ReleaseStringUTFChars((jstring)jStr, str8);
			}
		}
		const SI32 qEnvsEnd		= _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_setSelection(JNIEnv* pEnv, jobject pObj, jint start, jint end){
    JNI_STACK_KEYINPUT_PRINTF("setSelection")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "setSelection");
		if(start > end){ jint tmp = start; start = end; end = tmp; }
		NBGestorTeclas::entradaRangoSeleccionEstablecer(start, (end - start + 1));
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_TRUE;
}

// (API 24+)
JNIEXPORT void JNICALL Java_com_auframework_AppNative_00024AUInputConnection_closeConnection(JNIEnv* pEnv, jobject pObj){
    JNI_STACK_KEYINPUT_PRINTF("closeConnection")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "closeConnection");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

// (API 25+)
JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_commitContent(JNIEnv* pEnv, jobject pObj, jobject /*InputContentInfo*/ inputContentInfo, jint flags, jobject /*Bundle*/ opts){
    JNI_STACK_KEYINPUT_PRINTF("commitContent")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "commitContent");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

// (API 24+)
JNIEXPORT jboolean JNICALL Java_com_auframework_AppNative_00024AUInputConnection_deleteSurroundingTextInCodePoints(JNIEnv* pEnv, jobject pObj, jint beforeLength, jint afterLength){
    JNI_STACK_KEYINPUT_PRINTF("deleteSurroundingTextInCodePoints")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "deleteSurroundingTextInCodePoints");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return JNI_FALSE;
}

// (API 24+)
JNIEXPORT jobject /*Handler*/ JNICALL Java_com_auframework_AppNative_00024AUInputConnection_getHandler(JNIEnv* pEnv, jobject pObj){
    JNI_STACK_KEYINPUT_PRINTF("getHandler")
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 qEnvsStart = _app->getGlueJNI()->envVarsPushWithObj(pEnv, pObj, "getHandler");
		const SI32 qEnvsEnd = _app->getGlueJNI()->envVarsPop(pEnv); NBASSERT(qEnvsEnd == qEnvsStart)
	}
	NBGestorTeclas::entradaUnlockFromBatch();
    return NULL;
}

// ---------------------------------
// --- Metodos callbacks
// ---------------------------------

/*
Tipos para los methods descriptors
BaseType Character	 Type	 Interpretation
B	 byte	 signed byte
C	 char	 Unicode character
D	 double	 double-precision floating-point value
F	 float	 single-precision floating-point value
I	 int	 integer
J	 long	 long integer
L<classname>;	 reference	 an instance of class <classname>
S	 short	 signed short
Z	 boolean	 true or false
[	 reference	 one array dimension
Ljava/lang/String;
 */

//


