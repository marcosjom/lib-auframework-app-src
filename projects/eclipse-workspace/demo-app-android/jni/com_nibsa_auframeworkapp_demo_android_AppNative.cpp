
#include "com_serenehearts_android_AppNative.h"

#include <android/asset_manager_jni.h>

#include "SereneNucleoPrecompilado.h"
#include "SereneNucleo.h"

#define LEER_PRECACHE					false
#define LEER_CACHE						true
#define ESCRIBIR_CACHE					true
#define PAQUETES_RUTA_BASE				"paq/"
#define PAQUETES_CARGAR					true
#define PAQUETES_CARGAR_PRECACHEADOS	false

//-----------------------
//-- Java Environments --
//-----------------------
typedef enum ENJEnvType_ {
	ENJEnvType_Main = 0,		//Main thread JEnv
	ENJEnvType_Touch,			//Touch thread JEnv
	ENJEnvType_TextInput,		//Text Input thread JEnv
	ENJEnvType_Count
} ENJEnvType;

typedef struct STJEnvRef_ {
	JNIEnv* env;
	jobject objOrig;
	jobject obj;
	const char* dbgRefName;
} STJEnvRef;

typedef struct STJEnvStack_ {
	NBHILO_MUTEX_CLASE	mutex;
	int					size;
	int					use;
	STJEnvRef		envs[8];
	STJEnvRef		envCurrent;
} STJEnvStack;

STJEnvStack	_jEnvStacks[ENJEnvType_Count];

void jEnvStacksInit(){
	int i;
	for(i = 0; i < ENJEnvType_Count; i++){
		STJEnvStack* stack = &_jEnvStacks[i];
		NBHILO_MUTEX_INICIALIZAR(&stack->mutex)
		stack->size		= 8;
		stack->use		= 0;
		stack->envCurrent.env = NULL;
		stack->envCurrent.objOrig = NULL;
		stack->envCurrent.obj = NULL;
		stack->envCurrent.dbgRefName = NULL;
	}
}

ENJEnvType jEnvStackPushIfNew(JNIEnv* pEnv, jobject pObj, const ENJEnvType envType, const char* dbgRefName){
	ENJEnvType r = ENJEnvType_Count;
	STJEnvStack* stack = &_jEnvStacks[envType];
	NBHILO_MUTEX_ACTIVAR(&stack->mutex)
	if(stack->use == 0){
		STJEnvRef* itm = &stack->envs[stack->use++];
		itm->env		= pEnv;
		itm->objOrig	= pObj;
		itm->obj		= pEnv->NewLocalRef(pObj);
		itm->dbgRefName = dbgRefName;
		stack->envCurrent = *itm;
		r = envType;
	} else {
		STJEnvRef* curEnv	= &stack->envs[stack->use - 1];
		if(curEnv->env != pEnv || curEnv->objOrig != pObj){
			STJEnvRef* itm = &stack->envs[stack->use++];
			itm->env		= pEnv;
			itm->objOrig	= pObj;
			itm->obj		= pEnv->NewLocalRef(pObj);
			itm->dbgRefName = dbgRefName;
			stack->envCurrent = *itm;
#			ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
			{
				PRINTF_WARNING("JavaEnvStack type(%d) pushed with more than 1 item (%d items):\n", envType, stack->use);
				int i;
				for(i = (stack->use - 1); i >= 0; i--){
					STJEnvRef* itm = &stack->envs[i];
					PRINTF_WARNING("  + Env #%d env(%llu) pObj(%llu) nObj(%llu) ref('%s').\n", (i + 1), (UI64)itm->env, (UI64)itm->objOrig, (UI64)itm->obj, itm->dbgRefName);
				}
			}
#			endif
			r = envType;
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&stack->mutex)
	return r;
}

void jEnvStackPop(const ENJEnvType envType){
	STJEnvStack* stack = &_jEnvStacks[envType];
	NBHILO_MUTEX_ACTIVAR(&stack->mutex)
	NBASSERT((stack->use > 0 && stack->envCurrent.env != NULL) || (stack->use <= 0 && stack->envCurrent.env == NULL))
	if(stack->envCurrent.env != NULL){
		NBASSERT(stack->use > 0 )
		if(stack->envCurrent.obj != NULL) stack->envCurrent.env->DeleteLocalRef(stack->envCurrent.obj);
		stack->use--;
		if(stack->use > 0){
			stack->envCurrent		= stack->envs[stack->use - 1];
		} else {
			stack->envCurrent.env		= NULL;
			stack->envCurrent.objOrig	= NULL;
			stack->envCurrent.obj		= NULL;
			stack->envCurrent.dbgRefName	= NULL;
		}
	}
	NBHILO_MUTEX_DESACTIVAR(&stack->mutex)
}

//

STMainCallbacks _callbacks;
//
bool _motorInicializado					= false;
bool _mediaInicializado					= false;
float _xDpiPantalla						= 72.0f;
float _yDpiPantalla						= 72.0f;
//
AUApp* _app								= NULL;
AUEscenasAdministrador* _escenas		= NULL;
AUCadenaMutable8* _strRutaCargar		= NULL;
char* _idJugadorLocal					= NULL;
char* _nomJugadorLocal					= NULL;
//
bool 		enviarArchivo(void* param, const char* rutaArchivo);
//Compras
//Store
bool 				storeUpdateState(void* param, AUArreglo* productsIds);
ENCompraResultado	storeCurState(void* param);
bool				storeGetProductProps(void* param, const char* idProducto, AUCadenaMutable8* guardarTituloLocalEn, AUCadenaMutable8* guardarDescripcionLocalEn, AUCadenaMutable8* guardarPrecioLocalEn);
bool				storeProductWasPurchased(void* param, const char* idProducto);
bool				storePurchaseProduct(void* param, const char* idProducto);
bool				storeRestorePurchases(void* param);
ENCompraResultado	storeCurActionState(void* param);
//
const char*	dameIdJugadorLocal(void* param);
const char*	dameNombreJugadorLocal(void* param);
bool		abrirURL(void* param, const char* url);
//Callabacks de teclado
bool tecladoVisible(void* param);
bool tecladoMostrar(void* param);
bool tecladoOcultar(void* param);
void inputRangosActualizadosCallback(void* param, const NBRangoI rngSel, const NBRangoI rngMrc);

//

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeActivarPrioridadIdioma(JNIEnv* pEnv, jobject pObj, jstring idiomaId){
	if(_app != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeActivarPrioridadIdioma");
		//
		const char* strIdioma = pEnv->GetStringUTFChars(idiomaId, 0);
		if(AUCadena8::esIgual(strIdioma, "es") || AUCadena8::esIgual(strIdioma, "ES")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_ES); PRINTF_INFO("Idioma prioritario: 'es'\n");
		} else if(AUCadena8::esIgual(strIdioma, "en") || AUCadena8::esIgual(strIdioma, "EN")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_EN); PRINTF_INFO("Idioma prioritario: 'en'\n");
		} else if(AUCadena8::esIgual(strIdioma, "fr") || AUCadena8::esIgual(strIdioma, "FR")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_FR); PRINTF_INFO("Idioma prioritario: 'fr'\n");
		} else if(AUCadena8::esIgual(strIdioma, "de") || AUCadena8::esIgual(strIdioma, "DE")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_DE); PRINTF_INFO("Idioma prioritario: 'de' (aleman)\n");
		} else if(AUCadena8::esIgual(strIdioma, "it") || AUCadena8::esIgual(strIdioma, "IT")){
			NBGestorIdioma::establecerPrioridadIdioma(0, ENIdioma_IT); PRINTF_INFO("Idioma prioritario: 'it'\n");
		}
		pEnv->ReleaseStringUTFChars(idiomaId, strIdioma);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

//Compras

//Store
bool storeUpdateState(void* param, AUArreglo* productsIds){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL); //storeCurState
		if(cls != NULL){
			jmethodID methodAddProd = env->env->GetMethodID(cls, "storeAddProductSKU", "(Ljava/lang/String;)V"); NBASSERT(methodAddProd != NULL);
			jmethodID methodUpdate 	= env->env->GetMethodID(cls, "storeUpdateState", "()Z"); NBASSERT(methodUpdate != NULL);
			if(methodAddProd != NULL && methodUpdate != NULL){
				//Add products
				{
					SI32 i; const SI32 count = productsIds->conteo;
					for(i = 0; i < count; i++){
						AUCadena8* strId = (AUCadena8*) productsIds->elem(i);
						jstring jstrIdProd = env->env->NewStringUTF(strId->str());
						env->env->CallVoidMethod(env->obj, methodAddProd, jstrIdProd);
					}
				}
				//Start update
				{
					jboolean jresult 	= env->env->CallBooleanMethod(env->obj, methodUpdate);
					r = (jresult == JNI_TRUE);
				}
			}
		}
	}
	return r;
}

ENCompraResultado storeCurState(void* param){
	ENCompraResultado r = ENCompraResultado_error;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL); //storeCurState
		if(cls != NULL){
			jmethodID methodState 	= env->env->GetMethodID(cls, "storeCurState", "()I"); NBASSERT(methodState != NULL);
			if(methodState != NULL){
				jboolean jresult 	= env->env->CallIntMethod(env->obj, methodState);
				if(jresult >= 0 && jresult < ENCompraResultado_conteo){
					r = (ENCompraResultado)jresult;
				}
			}
		}
	}
	return r;
}

bool storeGetProductProps(void* param, const char* idProducto, AUCadenaMutable8* guardarTituloLocalEn, AUCadenaMutable8* guardarDescripcionLocalEn, AUCadenaMutable8* guardarPrecioLocalEn){
	bool r = false;
	if(guardarTituloLocalEn != NULL) guardarTituloLocalEn->vaciar();
	if(guardarDescripcionLocalEn != NULL) guardarDescripcionLocalEn->vaciar();
	if(guardarPrecioLocalEn != NULL) guardarPrecioLocalEn->vaciar();
	//
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID methodNombre 	= env->env->GetMethodID(cls, "storeGetProductPropsName", "(Ljava/lang/String;)Ljava/lang/String;"); NBASSERT(methodNombre != NULL);
			jmethodID methodDesc 	= env->env->GetMethodID(cls, "storeGetProductPropsDescription", "(Ljava/lang/String;)Ljava/lang/String;"); NBASSERT(methodDesc != NULL);
			jmethodID methodPrecio 	= env->env->GetMethodID(cls, "storeGetProductPropsPrice", "(Ljava/lang/String;)Ljava/lang/String;"); NBASSERT(methodPrecio != NULL);
			if(methodNombre != NULL && methodDesc != NULL && methodPrecio != NULL){
				jstring strSkuProucto = env->env->NewStringUTF(idProducto);
				jstring jStrNom 	= (jstring) env->env->CallObjectMethod(env->obj, methodNombre, strSkuProucto);
				jstring jStrDesc 	= (jstring) env->env->CallObjectMethod(env->obj, methodDesc, strSkuProucto);
				jstring jStrPrecio	= (jstring) env->env->CallObjectMethod(env->obj, methodPrecio, strSkuProucto);
				//
				const char* strNom = env->env->GetStringUTFChars(jStrNom, 0);
				const char* strDesc = env->env->GetStringUTFChars(jStrDesc, 0);
				const char* strPrecio = env->env->GetStringUTFChars(jStrPrecio, 0);
				//
				PRINTF_INFO("Datos de producto '%s' obtenidos nom('%s') desc('%s') precio('%s').\n", idProducto, strNom, strDesc, strPrecio);
				if(guardarTituloLocalEn != NULL) guardarTituloLocalEn->establecer(strNom);
				if(guardarDescripcionLocalEn != NULL) guardarDescripcionLocalEn->establecer(strDesc);
				if(guardarPrecioLocalEn != NULL) guardarPrecioLocalEn->establecer(strPrecio);
				bool cadenasNoVacias = (strNom[0] != '\0' && strDesc[0] != '\0' && strPrecio[0] != '\0');
				//
				env->env->ReleaseStringUTFChars(jStrNom, strNom);
				env->env->ReleaseStringUTFChars(jStrDesc, strDesc);
				env->env->ReleaseStringUTFChars(jStrPrecio, strPrecio);
				r = true;
			}
		}
	}
	return r;
}

bool storeProductWasPurchased(void* param, const char* idProducto){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID methodPurchased = env->env->GetMethodID(cls, "storeGetProductPropsIsPurchased", "(Ljava/lang/String;)Z"); NBASSERT(methodPurchased != NULL);
			if(methodPurchased != NULL){
				jstring strSkuProucto = env->env->NewStringUTF(idProducto);
				const jboolean isPurchased = env->env->CallBooleanMethod(env->obj, methodPurchased, strSkuProucto);
				r = (isPurchased == JNI_TRUE);
				PRINTF_INFO("Datos de producto '%s' obtenidos comprado('%d').\n", idProducto, (isPurchased == JNI_TRUE ? 1 : 0));
			}
		}
	}
	return r;
}

bool storePurchaseProduct(void* param, const char* idProducto){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "storePurchaseProduct", "(Ljava/lang/String;)Z"); NBASSERT(method != NULL);
			if(method != NULL){
				jboolean jresult = env->env->CallBooleanMethod(env->obj, method, env->env->NewStringUTF(idProducto));
				r = (jresult = JNI_TRUE);
			}
		}
	}
	return r;
}

bool storeRestorePurchases(void* param){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "storeRestorePurchases", "()Z"); NBASSERT(method != NULL);
			if(method != NULL){
				jboolean jresult = env->env->CallBooleanMethod(env->obj, method);
				r = (jresult = JNI_TRUE);
			}
		}
	}
	return r;
}

ENCompraResultado storeCurActionState(void* param){
	ENCompraResultado r = ENCompraResultado_error;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "storeCurActionState", "(Ljava/lang/String;)I"); NBASSERT(method != NULL);
			if(method != NULL){
				jint jresult = env->env->CallIntMethod(env->obj, method);
				if(jresult >= 0 && jresult < ENCompraResultado_conteo){
					r = (ENCompraResultado)jresult;
				}
			}
		}
	}
	return r;
}

//

char* jstringTostring(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0)
    {
        rtn = (char*)malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

//
JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeInicializarMotor(JNIEnv *pEnv, jobject pObj, jobject pContext, jobject assetManager, jstring docsPath, jstring cachePath, jstring archivoAbrir){
	jboolean exito 	= JNI_FALSE;
	//
	jEnvStacksInit();
	//
	if(_motorInicializado){
		PRINTF_WARNING("MOTOR ya esta inicializado.\n");
		exito = JNI_TRUE;
	} else {
		PRINTF_INFO("MOTOR inicializando.\n");
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
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeInicializarMotor");
		//
		STMainCallbacksInicializar(&_callbacks);
		_callbacks.funcEnviarArchivo				= enviarArchivo;
		_callbacks.funcEnviarArchivoParam			= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		//Store
		_callbacks.funcStoreUpdateState				= storeUpdateState;
		_callbacks.funcStoreUpdateStateParam		= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		_callbacks.funcStoreCurState				= storeCurState;
		_callbacks.funcStoreCurStateParam			= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		_callbacks.funcStoreGetProductProps			= storeGetProductProps;
		_callbacks.funcStoreGetProductPropsParam	= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		_callbacks.funcStoreProductWasPurchased		= storeProductWasPurchased;
		_callbacks.funcStoreProductWasPurchasedParam= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		_callbacks.funcStorePurchaseProduct			= storePurchaseProduct;
		_callbacks.funcStorePurchaseProductParam	= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		_callbacks.funcStoreRestorePurchases		= storeRestorePurchases;
		_callbacks.funcStoreRestorePurchasesParam	= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		_callbacks.funcStoreCurActionState			= storeCurActionState;
		_callbacks.funcStoreCurActionStateParam		= &_jEnvStacks[ENJEnvType_Main].envCurrent;
		//
		_app 		= NULL;
		_escenas	= NULL;
		{
			const char* nativeDocsPath 	= pEnv->GetStringUTFChars(docsPath, 0);
			const char* nativeCachePath = pEnv->GetStringUTFChars(cachePath, 0);
			if(!AUApp::inicializarNucleo(AAssetManager_fromJava(pEnv, assetManager), nativeDocsPath, nativeCachePath, AUAPP_BIT_MODULO_RED)){
				PRINTF_ERROR("ERROR, AUApp::inicializarNucleo ha fallado.\n");
			} else {
				//Archivo a abrir
				const char* strRutaAbrir= pEnv->GetStringUTFChars(archivoAbrir, 0);
				_strRutaCargar			= new AUCadenaMutable8(strRutaAbrir);
				pEnv->ReleaseStringUTFChars(archivoAbrir, strRutaAbrir);
				//
				NBSereneGestor::inicializar(true /*windowIsFixedSize*/);
				//
				//Identificar firma de paquete
				NBSHA1 shaFirma;
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
				}
				//
				/*
				Firmas de certificados:
				Debug
				MD5:  A2AD4274-CFB4B0A3-0A2BF7C6-F03501DB
				SHA1: 17CBA9D68F-2CA6F7081D-90BDF81CEE-88596B5711
				Release
				MD5:  558B2E6E-55DEE1CF-EAC823B5-C8A84603
				SHA1: 8DA909384E-CD24FEA3AE-CAF997A8F7-E87F36BCAB
				*/
				if(AUCadena8::esIgual(shaFirma.hashHex(), "17CBA9D68F2CA6F7081D90BDF81CEE88596B5711") || AUCadena8::esIgual(shaFirma.hashHex(), "8DA909384ECD24FEA3AECAF997A8F7E87F36BCAB")){
					STAppCallbacks appCallbacks;				AUApp::inicializarCallbacks(&appCallbacks);
					//Teclado
					appCallbacks.funcTecladoVisible				= tecladoVisible;
					appCallbacks.funcTecladoVisibleParam		= &_jEnvStacks[ENJEnvType_Main].envCurrent;
					appCallbacks.funcTecladoMostrar				= tecladoMostrar;
					appCallbacks.funcTecladoMostrarParam		= &_jEnvStacks[ENJEnvType_Main].envCurrent;
					appCallbacks.funcTecladoOcultar				= tecladoOcultar;
					appCallbacks.funcTecladoOcultarParam		= &_jEnvStacks[ENJEnvType_Main].envCurrent;
					appCallbacks.funcInputRangosModif			= inputRangosActualizadosCallback;
					appCallbacks.funcInputRangosModifParam		= &_jEnvStacks[ENJEnvType_Main].envCurrent;
					//
					_app = new AUApp(&appCallbacks, "SereneHearts");
					_motorInicializado		= true;
					exito 					= JNI_TRUE;
				}
			}
			pEnv->ReleaseStringUTFChars(docsPath, nativeDocsPath);
			pEnv->ReleaseStringUTFChars(cachePath, nativeCachePath);
		}
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return exito;
}

JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeInicializarEscena(JNIEnv *pEnv, jobject pObj, jfloat freqPantalla, jfloat xDpiPantalla, jfloat yDpiPantalla){
	jboolean r = JNI_FALSE;
	const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeInicializarEscena");
	//
	if(!_mediaInicializado){
		PRINTF_INFO("++++ INICIALIZANDO ESCENA ++++\n");
		//Inicializar media
		_xDpiPantalla	= xDpiPantalla;
		_yDpiPantalla	= yDpiPantalla;
		if(!AUApp::inicializarGraficos(LEER_PRECACHE, LEER_CACHE, ESCRIBIR_CACHE, freqPantalla)){
			PRINTF_ERROR("ERROR, no se pudo inicializar el motor grafico.\n");
		} else {
			//NBGestorRefranero::inicializarDespuesDeMotor();
			_mediaInicializado = true;
			r = JNI_TRUE;
		}
	} else {
		PRINTF_INFO("++++ RE-INICIALIZANDO ESCENA ++++\n");
		if(!AUApp::reinicializarGraficos(false)){
			PRINTF_ERROR("ERROR, no se pudo REinicializar el motor grafico.\n");
		} else {
			r = JNI_TRUE;
		}
	}
	//
	if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	return r;
}

JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeRedimensionarEscena(JNIEnv *pEnv, jobject pObj, jint ancho, jint alto){
	//PRINTF_INFO("nativeRedimensionarEscena(%d, %d).\n", ancho, alto);
	jboolean r = JNI_FALSE;
	const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeRedimensionarEscena");
	if(_app == NULL){
		PRINTF_ERROR("Invocacion a nativeRedimensionarEscena con '_app==NULL'.\n");
	} else {
		if(_escenas == NULL){
			//iPad: 132dpi (~800K pixeles)
			//iPhone & iPodTouch & iPadMini: 163dpi
			//iPhone & iPodTouch Retina & iPadMini2: 326dpi (~600K - ~750K pixeles)
			//iPad Retina: 264dpi (~3M pixeles)
			if(!_app->inicializarVentana(ancho, alto, _xDpiPantalla, _yDpiPantalla, ENGestorEscenaDestinoGl_Heredado)){
				PRINTF_ERROR("_app->inicializarVentana(%d, %d) ha fallado.\n", ancho, alto);
			} else {
				PRINTF_INFO("_app->inicializarVentana(%d, %d) exitoso.\n", ancho, alto);
				//
				NBASSERT(_app->indiceEscenaRender() >= 0)
				const char* paquetes[] = {
					"paqFuentes.otf"
					, "paqPNGx4_onlyAnims.png"
					, "paqPNGx2_onlyAnims.png"
					, "paqPNGx1_onlyAnims.png"
					, "paqAnimaciones.xml"
					, "paqSonidos.wav"
					//Optionals (provided as download in some distributions)
					, "paqPNGx4.png"
					, "paqPNGx2.png"
					, "paqPNGx1.png"
					, "paqMusicaOgg.wav"
				};
				const SI32 iPrimerPaqueteCargar = 0;
				const SI32 conteoPaquetesCargar = 10;
				//
				_escenas = new AUEscenasAdministrador(_app->indiceEscenaRender(), &_callbacks, PAQUETES_RUTA_BASE, &paquetes[iPrimerPaqueteCargar], conteoPaquetesCargar);
				if(!_app->inicializarJuego(_escenas)){
					PRINTF_ERROR("_app->inicializarJuego() ha fallado.\n");
				} else {
					PRINTF_INFO("_app->inicializarJuego(%d, %d) exitoso.\n", ancho, alto);
					r = JNI_TRUE;
				}
			}
		} else {
			_app->notificarRedimensionPantalla(ancho, alto);
			r = JNI_TRUE;
		}
	}
	if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	return r;
}

JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeFinalizar(JNIEnv *pEnv, jobject pObj){
	const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeFinalizar");
	//
	PRINTF_INFO("-----------------------\n");
	PRINTF_INFO("--- nativeFinalizar ---\n");
	PRINTF_INFO("-----------------------\n");
	//
	if(_idJugadorLocal != NULL) NBGestorMemoria::liberarMemoria(_idJugadorLocal); _idJugadorLocal = NULL;
	if(_nomJugadorLocal != NULL) NBGestorMemoria::liberarMemoria(_nomJugadorLocal); _nomJugadorLocal = NULL;
	if(_strRutaCargar != NULL) _strRutaCargar->liberar(); _strRutaCargar = NULL;
	if(_app != NULL){
		PRINTF_INFO("Finalizando juego.\n");
		_app->finalizarJuego();	//Libera las escenas el resumeDebug
		PRINTF_INFO("Exito finalizando juego.\n");
	}
	if(_escenas != NULL){
		PRINTF_INFO("Liberando _escenas.\n");
		_escenas->liberar();
		PRINTF_INFO("Exito liberando _escenas.\n");
		_escenas = NULL;
	}
	if(_app != NULL){
		PRINTF_INFO("Finalizando ventana.\n");
		_app->finalizarVentana();	//Destruye la escena
		PRINTF_INFO("Exito finalizando ventana.\n");
	}
	if(_mediaInicializado){
		PRINTF_INFO("Finalizando graficos.\n");
		AUApp::finalizarGraficos();
		PRINTF_INFO("Exito finalizando graficos.\n");
		_mediaInicializado = false;
	}
	if(_motorInicializado){
		if(_app != NULL){
			PRINTF_INFO("Liberando _app.\n");
			delete _app; //->liberar();
			PRINTF_INFO("Exito liberando _app.\n");
			_app = NULL;
		}
		NBSereneGestor::finalizar();
		PRINTF_INFO("Finalizando nucleo.\n");
		AUApp::finalizarNucleo();
		PRINTF_INFO("Exito finalizando nucleo.\n");
		_motorInicializado = false;
	}
	if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeAppFocoObtenido(JNIEnv *pEnv, jobject pObj){
	if(_app != NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeAppFocoObtenido");
		//
		_app->focoExclusivoObtenido();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeAppFocoPerdido(JNIEnv *pEnv, jobject pObj){
	if(_app != NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeAppFocoPerdido");
		//
		_app->focoExclusivoPerdido();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTickUnSegundo(JNIEnv *pEnv, jobject pObj){
	if(_app != NULL && _escenas != NULL){
		//const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeTickUnSegundo");
		//
		_app->tickUnSegundo();
		//
		//if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT jboolean Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTickProducirYConumirRender(JNIEnv *pEnv, jobject pObj){
	jboolean r = JNI_FALSE;
	const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeTickProducirYConumirRender");
	//
	if(_app == NULL){
		PRINTF_ERROR("ERROR, invocaci—n a nativeTickProducir con '_app==NULL'.\n");
	} else {
		if(_escenas == NULL){
			PRINTF_ERROR("ERROR, invocaci—n a nativeTickProducir con '_escenas==NULL'.\n");
		} else {
			_app->tickJuego(ENAUAppTickTipo_SincPantalla, false);
			r = JNI_TRUE;
		}
	}
	if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	return r;
}

//---------------------------
//-- Touch
//---------------------------

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTouchIniciado(JNIEnv *pEnv, jobject pObj, jint id, jint x, jint y){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Touch, "nativeTouchIniciado");
		//
		_app->touchIniciado(id, x, y);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTouchMovido(JNIEnv *pEnv, jobject pObj, jint id, jint x, jint y){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Touch, "nativeTouchMovido");
		//
		_app->touchMovido(id, x, y);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTouchFinalizado(JNIEnv *pEnv, jobject pObj, jint id, jint x, jint y, jboolean cancel){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Touch, "nativeTouchFinalizado");
		//
		_app->touchFinalizado(id, x, y, (cancel ? true : false));
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

//---------------------------
//-- Teclado
//---------------------------

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTecladoPerdioFoco(JNIEnv *pEnv, jobject pObj){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeTecladoPerdioFoco");
		//
		NBGestorTeclas::escuchadorRemover();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTecladoEnPantallaEstablecerAlto(JNIEnv *pEnv, jobject pObj, jfloat altoTecladoEnPantalla){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeTecladoEnPantallaEstablecerAlto");
		//
		NBGestorTeclas::establecerTecladoEnPantallaAlto(altoTecladoEnPantalla, true);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTeclaBackPresionada(JNIEnv *pEnv, jobject pObj){
	jboolean r = JNI_FALSE;
	PRINTF_INFO("nativeTeclaBackPresionada (1).\n");
	if(_app!=NULL && _escenas != NULL){
		PRINTF_INFO("nativeTeclaBackPresionada (2).\n");
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeTeclaBackPresionada");
		//
		r = (_app->teclaEspecialPresionada(AU_TECLA_REGRESAR) ? JNI_TRUE: JNI_FALSE);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTeclaBackspace(JNIEnv *pEnv, jobject pObj){
	jboolean r = JNI_FALSE;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeTeclaBackspace");
		//
		NBGestorTeclas::entradaBackspace(false);
		r = JNI_TRUE;
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTeclaInsertarTexto(JNIEnv *pEnv, jobject pObj, jstring pTexto){
	jboolean r = JNI_FALSE;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeTeclaInsertarTexto");
		//
		const char* strTexto = pEnv->GetStringUTFChars(pTexto, 0);
		NBGestorTeclas::entradaIntroducirTexto(strTexto, false);
		pEnv->ReleaseStringUTFChars(pTexto, strTexto);
		r = JNI_TRUE;
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}


JNIEXPORT jboolean JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeTeclaMenuPresionada(JNIEnv *pEnv, jobject pObj){
	jboolean r = JNI_FALSE;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeTeclaMenuPresionada");
		//
		r = (_app->teclaEspecialPresionada(AU_TECLA_MENU) ? JNI_TRUE: JNI_FALSE);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

//---------------------------
//-- Text Input
//---------------------------

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaExplicitBashPush(JNIEnv *pEnv, jobject pObj){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoSeleccion");
		//
		NBGestorTeclas::entradaExplicitBashPush();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaExplicitBashPop(JNIEnv *pEnv, jobject pObj){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoSeleccion");
		//
		NBGestorTeclas::entradaExplicitBashPop();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT jintArray	JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaRangoSeleccion(JNIEnv *pEnv, jobject pObj){ //int[2] = [ini, tamano];
	jintArray r = NULL;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoSeleccion");
		//
		const NBRangoI rng = NBGestorTeclas::entradaRangoSeleccion();
		int rVals[2] = {rng.inicio, rng.tamano};
		r = pEnv->NewIntArray(2);
		pEnv->SetIntArrayRegion(r, 0, 2, rVals);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaRangoSeleccionEstablecer(JNIEnv *pEnv, jobject pObj, jint primerCharDef, jint conteoCharDefs){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoSeleccionEstablecer");
		//
		NBGestorTeclas::entradaRangoSeleccionEstablecer(primerCharDef, conteoCharDefs);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT jintArray	JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaRangoMarcado(JNIEnv *pEnv, jobject pObj){ //int[2] = [ini, tamano];
	jintArray r = NULL;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoMarcado");
		//
		const NBRangoI rng = NBGestorTeclas::entradaRangoMarcado();
		int rVals[2] = {rng.inicio, rng.tamano};
		r = pEnv->NewIntArray(2);
		pEnv->SetIntArrayRegion(r, 0, 2, rVals);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaRangoMarcadoEstablecer(JNIEnv *pEnv, jobject pObj, jint primerCharDef, jint conteoCharDefs){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoMarcadoEstablecer");
		//
		NBGestorTeclas::entradaRangoMarcadoEstablecer(primerCharDef, conteoCharDefs);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaRangoDesmarcar(JNIEnv *pEnv, jobject pObj){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaRangoDesmarcar");
		//
		NBGestorTeclas::entradaRangoDesmarcar();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

//

JNIEXPORT jstring JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaContenido(JNIEnv *pEnv, jobject pObj){
	jstring r = NULL;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaContenido");
		//
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTexto(strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT jstring JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaContenidoMarcado(JNIEnv *pEnv, jobject pObj){
	jstring r = NULL;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaContenidoMarcado");
		//
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoMarcado(strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaContenidoMarcadoReemplazar(JNIEnv *pEnv, jobject pObj, jstring valor, jint setComposingAs){
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaContenidoMarcadoReemplazar");
		//
		const char* strValor = pEnv->GetStringUTFChars(valor, NULL);//Java String to C Style string
		NBGestorTeclas::entradaTextoMarcadoReemplazar(strValor, (const ENTextRangeSet)setComposingAs);
		pEnv->ReleaseStringUTFChars(valor, strValor);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

JNIEXPORT jstring JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaContenidoSeleccion(JNIEnv *pEnv, jobject pObj){
	jstring r = NULL;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaContenidoSeleccion");
		//
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoSeleccion(strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT jstring JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaContenidoAntesSeleccion(JNIEnv *pEnv, jobject pObj, jint conteoCharDefs){
	jstring r = NULL;
	if(_app!=NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaContenidoAntesSeleccion");
		//
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoAntesDeSeleccion(conteoCharDefs, strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT jstring JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaContenidoDespuesSeleccion(JNIEnv *pEnv, jobject pObj, jint conteoCharDefs){
	jstring r = NULL;
	if(_app != NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaContenidoDespuesSeleccion");
		//
		AUCadenaMutable8* strTmp = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
		NBGestorTeclas::entradaTextoDespuesDeSeleccion(conteoCharDefs, strTmp);
		r = pEnv->NewStringUTF(strTmp->str());
		strTmp->liberar();
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
	return r;
}

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeEntradaTextoSeleccionEliminarAlrededor(JNIEnv *pEnv, jobject pObj, jint conteoCharDefsAntes, jint conteoCharDefsDespues){
	if(_app != NULL && _escenas != NULL){
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_TextInput, "nativeEntradaTextoSeleccionEliminarAlrededor");
		//
		NBGestorTeclas::entradaTextoSeleccionEliminarAlrededor(conteoCharDefsAntes, conteoCharDefsDespues, false);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
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

JNIEXPORT void JNICALL Java_com_nibsa_auframeworkapp_demo_android_AppNative_nativeNotificacionLocalRecibida(JNIEnv *pEnv, jobject pObj, jstring grp, jint localId, jstring objTip, jint objId){
	if(_app == NULL || _escenas == NULL){
		PRINTF_ERROR("Notificacion, ignorando (sin _app/_escenas).\n");
	} else {
		const ENJEnvType jEnvPushed = jEnvStackPushIfNew(pEnv, pObj, ENJEnvType_Main, "nativeNotificacionLocalRecibida");
		//
		const char* strGrp = pEnv->GetStringUTFChars(grp, 0);
		const char* strObjTip = pEnv->GetStringUTFChars(objTip, 0);
		_app->procesarNotificacionLocal(strGrp, localId, strObjTip, objId);
		pEnv->ReleaseStringUTFChars(objTip, strObjTip);
		pEnv->ReleaseStringUTFChars(grp, strGrp);
		//
		if(jEnvPushed < ENJEnvType_Count) jEnvStackPop(jEnvPushed);
	}
}

//

bool enviarArchivo(void* param, const char* rutaArchivo){
	STJEnvRef* env = (STJEnvRef*)param;
	bool r = false;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL)
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "enviarArchivo", "(Ljava/lang/String;)V"); NBASSERT(method != NULL)
			if(method != NULL){
				/*const MapaDeBitsDesciptor props = mapaBits->propiedades();
				//Convertir los daros RGBA a ARGB
				BYTE* datosARGB = (BYTE*)mapaBits->datos();
				UI32 y, x;
				for(y=0; y<props.alto; y++){
					BYTE* pBytePix = &datosARGB[y * props.bytesPorLinea];
					BYTE* pBytePixSigUlt = &datosARGB[(y * props.bytesPorLinea) + (props.ancho * (props.bitsPorPixel / 8))];
					while(pBytePix < pBytePixSigUlt){
						//RGBA a ARGB
						const BYTE r = pBytePix[0];
						//const BYTE g = pBytePix[1];
						const BYTE b = pBytePix[2];
						//const BYTE a = pBytePix[3];
						pBytePix[0] = b;
						//pBytePix[1] = g;
						pBytePix[2] = r;
						//pBytePix[3] = a;
						pBytePix += 4;
					}
				}
				jintArray jPixeles = env->env->NewIntArray(props.ancho * props.alto); NBASSERT(props.bitsPorPixel==32) NBASSERT((props.bytesPorLinea % 4)==0)
				env->env->SetIntArrayRegion(jPixeles, 0, props.ancho * props.alto, (jint*)datosARGB);
				jstring jStrMsg = env->env->NewStringUTF(mensaje);
				env->env->CallVoidMethod(env->obj, method, jStrMsg, (jint)score, jPixeles, (jint)props.ancho, (jint)props.alto);
				NBGestorJuego::procesaFinRedactorSocial(true, false);
				PRINTF_INFO("enviarCaptura(8).\n");*/
				jstring jStrPath = env->env->NewStringUTF(rutaArchivo);
				env->env->CallVoidMethod(env->obj, method, jStrPath);
				PRINTF_INFO("enviarCaptura(8).\n");
				r = true;
			}
		}
	}
	return r;
}

const char* dameIdJugadorLocal(void* param){
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL)
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "dameIdJugadorLocal", "()Ljava/lang/String;"); NBASSERT(method != NULL)
			if(method != NULL){
				jstring jStrId = (jstring)env->env->CallObjectMethod(env->obj, method); NBASSERT(jStrId != NULL)
				//
				const char* strId = env->env->GetStringUTFChars(jStrId, 0);
				int tamStr = AUCadena8::tamano(strId);
				if(_idJugadorLocal != NULL) NBGestorMemoria::liberarMemoria(_idJugadorLocal);
				_idJugadorLocal = (char*)NBGestorMemoria::reservarMemoria(sizeof(char) * (tamStr + 1), ENMemoriaTipo_Nucleo);
				memcpy(_idJugadorLocal, strId, tamStr); _idJugadorLocal[tamStr] = '\0';
				env->env->ReleaseStringUTFChars(jStrId, strId);
				return _idJugadorLocal;
			}
		}
	}
	return NULL;
}

const char* dameNombreJugadorLocal(void* param){
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL)
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "dameNombreJugadorLocal", "()Ljava/lang/String;"); NBASSERT(method != NULL)
			if(method != NULL){
				jstring jStrNom = (jstring)env->env->CallObjectMethod(env->obj, method); NBASSERT(jStrNom != NULL)
				//
				const char* strNom = env->env->GetStringUTFChars(jStrNom, 0);
				int tamStr = AUCadena8::tamano(strNom);
				if(_nomJugadorLocal != NULL) NBGestorMemoria::liberarMemoria(_nomJugadorLocal);
				_nomJugadorLocal = (char*)NBGestorMemoria::reservarMemoria(sizeof(char) * (tamStr + 1), ENMemoriaTipo_Nucleo);
				memcpy(_nomJugadorLocal, strNom, tamStr); _nomJugadorLocal[tamStr] = '\0';
				env->env->ReleaseStringUTFChars(jStrNom, strNom);
				return _nomJugadorLocal;
			}
		}
	}
	return NULL;
}

bool abrirURL(void* param, const char* url){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
		NBASSERT(env->env != NULL && env->obj != NULL)
		if(env->env != NULL && env->obj != NULL){
			jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
			if(cls != NULL){
				jmethodID method = env->env->GetMethodID(cls, "abrirURLPlayStore", "()V"); NBASSERT(method != NULL);
				if(method != NULL){
					env->env->CallVoidMethod(env->obj, method);
					r = true;
				}
			}
		}
	return r;
};

//Eventos de teclado

bool tecladoVisible(void* param){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "tecladoVisible", "()Z"); NBASSERT(method != NULL);
			if(method != NULL){
				r = (env->env->CallBooleanMethod(env->obj, method) == JNI_TRUE);
			}
		}
	}
	return r;
}

bool tecladoMostrar(void* param){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "tecladoMostrar", "()Z"); NBASSERT(method != NULL);
			if(method != NULL){
				r = (env->env->CallBooleanMethod(env->obj, method) == JNI_TRUE);
			}
		}
	}
	return r;
}

bool tecladoOcultar(void* param){
	bool r = false;
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "tecladoOcultar", "()Z"); NBASSERT(method != NULL);
			if(method != NULL){
				r = (env->env->CallBooleanMethod(env->obj, method) == JNI_TRUE);
			}
		}
	}
	return r;
}

void inputRangosActualizadosCallback(void* param, const NBRangoI rngSel, const NBRangoI rngMrc){
	STJEnvRef* env = (STJEnvRef*)param;
	NBASSERT(env->env != NULL && env->obj != NULL)
	if(env->env != NULL && env->obj != NULL){
		jclass cls = env->env->GetObjectClass(env->obj); NBASSERT(cls != NULL);
		if(cls != NULL){
			jmethodID method = env->env->GetMethodID(cls, "entradaRangosModificadosCallback", "([I[I)V"); NBASSERT(method != NULL);
			if(method != NULL){
				jintArray jArrSel = env->env->NewIntArray(2);
				jintArray jArrMrc = env->env->NewIntArray(2);
				jint cArr[4] = {rngSel.inicio, rngSel.tamano, rngMrc.inicio, rngMrc.tamano};
				env->env->SetIntArrayRegion(jArrSel, 0, 2, &cArr[0]);
				env->env->SetIntArrayRegion(jArrMrc, 0, 2, &cArr[2]);
				env->env->CallVoidMethod(env->obj, method, jArrSel, jArrMrc);
			}
		}
	}
}


