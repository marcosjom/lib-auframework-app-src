//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueAndroidFiles.h"
#include "AUAppI.h"
//Android and JNI headers
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h> //for AAssetManager_fromJava

//-------------------------
// AUAppGlueAndroidFiles
//-------------------------

#ifdef __ANDROID__
	//is android
#endif

//

typedef struct AUAppGlueAndroidFilesData_ {
	AUAppI* app;
	void*	assetsMngr; //AAssetManager
	AUCadenaMutable8* strPaths[ENMngrFilesPathType_Count];
} AUAppGlueAndroidFilesData;

//Callbacks

bool AUAppGlueAndroidFiles::create(void* app, STMngrFilesCalls* obj){
	AUAppGlueAndroidFilesData* data = (AUAppGlueAndroidFilesData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidFilesData), ENMemoriaTipo_General);
	//Init vars
	{
		data->app			= (AUAppI*)app;
		data->assetsMngr	= NULL;
		SI32 i;
		for(i = 0; i < ENMngrFilesPathType_Count; i++){
			data->strPaths[i] = new AUCadenaMutable8();
		}
		//Set config from Android and JNI
		data->strPaths[ENMngrFilesPathType_Pkg]->establecer("apk:/");
		{
			AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
			JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
			if(jEnv != NULL){
				jobject jContext = (jobject)jniGlue->jActivity();
				NBASSERT(jEnv != NULL && jContext != NULL)
				if(jEnv != NULL && jContext != NULL){
					jclass clsContext = jEnv->FindClass("android/content/Context");
					if(clsContext != NULL){
						//Get Context's jni assets object
						{
							jmethodID mid = jEnv->GetMethodID(clsContext, "getResources", "()Landroid/content/res/Resources;");
							if (mid != NULL) {
								jobject jResObj = jEnv->CallObjectMethod(jContext, mid);
								if (jResObj != NULL) {
									jclass clsRes = jEnv->FindClass("android/content/res/Resources");
									if(clsRes != NULL){
										jmethodID mid = jEnv->GetMethodID(clsRes, "getAssets", "()Landroid/content/res/AssetManager;");
										if (mid != NULL) {
											jobject jAssMngrObj = jEnv->CallObjectMethod(jResObj, mid);
											if (jAssMngrObj != NULL) {
												data->assetsMngr = AAssetManager_fromJava(jEnv, jAssMngrObj);
											}
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, clsRes)
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jResObj)
								}
							}
						}
						//Get Context's docs path
						{
							jmethodID mid = jEnv->GetMethodID(clsContext, "getFilesDir", "()Ljava/io/File;");
							if (mid != NULL) {
								jobject jDirObj = jEnv->CallObjectMethod(jContext, mid);
								if(jDirObj != NULL){
									jclass clsFile = jEnv->FindClass("java/io/File");
									if(clsFile != NULL){
										jmethodID mid = jEnv->GetMethodID(clsFile, "getAbsolutePath", "()Ljava/lang/String;");
										if (mid != NULL) {
											jstring jDocPath = (jstring)jEnv->CallObjectMethod(jDirObj, mid);
											const char* utfStr = jEnv->GetStringUTFChars(jDocPath, 0);
											data->strPaths[ENMngrFilesPathType_Doc]->establecer(utfStr);
											data->strPaths[ENMngrFilesPathType_Lib]->establecer(utfStr);
											jEnv->ReleaseStringUTFChars(jDocPath, utfStr);
											NBJNI_DELETE_REF_LOCAL(jEnv, jDocPath)
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, clsFile)
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jDirObj)
								}
							}
						}
						//Get Context's cache path
						{
							jmethodID mid = jEnv->GetMethodID(clsContext, "getCacheDir", "()Ljava/io/File;");
							if (mid != NULL) {
								jobject jDirObj = jEnv->CallObjectMethod(jContext, mid);
								if(jDirObj != NULL){
									jclass clsFile = jEnv->FindClass("java/io/File");
									if(clsFile != NULL){
										jmethodID mid = jEnv->GetMethodID(clsFile, "getAbsolutePath", "()Ljava/lang/String;");
										if (mid != NULL) {
											jstring jCachePath = (jstring)jEnv->CallObjectMethod(jDirObj, mid);
											const char* utfStr = jEnv->GetStringUTFChars(jCachePath, 0);
											data->strPaths[ENMngrFilesPathType_Cache]->establecer(utfStr);
											jEnv->ReleaseStringUTFChars(jCachePath, utfStr);
											NBJNI_DELETE_REF_LOCAL(jEnv, jCachePath)
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, clsFile)
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jDirObj)
								}
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
					}
				}
			}
		}
	}
	//
	obj->funcCreate			= create;
	obj->funcCreateParam	= data;
	obj->funcDestroy		= destroy;
	obj->funcDestroyParam	= data;
	obj->funcGetPathPrefix	= getPathPrefix;
	obj->funcGetPathPrefixParam	= data;
	obj->funcOpenFile		= openFile;
	obj->funcOpenFileParam	= data;
	//
	return true;
}

bool AUAppGlueAndroidFiles::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidFilesData* data = (AUAppGlueAndroidFilesData*)pData;
		data->app = NULL;
		data->assetsMngr = NULL;
		{
			SI32 i;
			for(i = 0; i < ENMngrFilesPathType_Count; i++){
				if(data->strPaths[i] != NULL) data->strPaths[i]->liberar(NB_RETENEDOR_THIS); data->strPaths[i] = NULL;
			}
		}
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueAndroidFiles::getPathPrefix(void* pData, const ENMngrFilesPathType type, AUCadenaMutable8* dst){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidFilesData* data = (AUAppGlueAndroidFilesData*)pData;
		dst->establecer(data->strPaths[type]->str());
		r = true;
	}
	return r;
}

AUArchivo* AUAppGlueAndroidFiles::openFile(void* pData, const char* path, const ENArchivoModo fileMode, const ENMemoriaTipo typeMemResidence){
	AUArchivo* r = NULL;
	if(pData != NULL){
		AUAppGlueAndroidFilesData* data = (AUAppGlueAndroidFilesData*)pData;
		if(fileMode == ENArchivoModo_SoloLectura){
			if(data->assetsMngr != NULL){
				if(path[0] != '\0'){
					if(path[1] != '\0'){
						if(path[2] != '\0'){
							if(path[3] != '\0'){
								if(path[4] != '\0'){
									if(path[0] == 'a' && path[1] == 'p' && path[2] == 'k' && path[3] == ':' && path[4] == '/'){
										AAsset* file = AAssetManager_open((AAssetManager*)data->assetsMngr, &path[5], AASSET_MODE_RANDOM);
										if(file == NULL){
											//PRINTF_ERROR("AUAppGlueAndroidFiles, no se pudo abrir flujo en APK '%s'\n", path);
										} else {
											r = new(typeMemResidence) AUArchivoFisicoAPK(file); NB_DEFINE_NOMBRE_PUNTERO(flujo, "AUAppGlueAndroidFiles::flujoAPK")
											r->autoLiberar(NB_RETENEDOR_NULL);
											PRINTF_INFO("AUArchivoFisicoAPK, AAsset from APK opened for '%s'.\n", path);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return r;
}

//-------------------------
// AUArchivoFisicoAPK
//-------------------------

AUArchivoFisicoAPK::AUArchivoFisicoAPK(void* aasset /*AAsset*/) : AUArchivo() {
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::AUArchivoFisicoAPK")
	NB_DEFINE_NOMBRE_PUNTERO(this, "AUArchivoFisicoAPK")
	_aasset			= aasset;
	_posActual		= 0;
	_lastOp			= ENFileOp_None;
	//if(_aasset != NULL) _posActual = ftell(_aasset);
	NBHILO_MUTEX_INICIALIZAR(&_mutex)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	_mutexLockCount	= 0;
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP
}

AUArchivoFisicoAPK::~AUArchivoFisicoAPK(){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::~AUArchivoFisicoAPK")
	this->cerrar();
	//NBASSERT(_mutexLockCount == 0)
	NBHILO_MUTEX_FINALIZAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP
}

//

void AUArchivoFisicoAPK::lock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::lock")
	NBHILO_MUTEX_ACTIVAR(&_mutex)
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	_mutexLockCount++;
#	endif
	AU_GESTOR_PILA_LLAMADAS_POP
}

void AUArchivoFisicoAPK::unlock(){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::unlock")
#	ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
	NBASSERT(_mutexLockCount > 0)
	_mutexLockCount--;
#	endif
	NBHILO_MUTEX_DESACTIVAR(&_mutex)
	AU_GESTOR_PILA_LLAMADAS_POP
}

//

ENFileOp AUArchivoFisicoAPK::lastOperation() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::lastOperation")
	AU_GESTOR_PILA_LLAMADAS_POP
	return _lastOp;
}

SI32 AUArchivoFisicoAPK::leer(void* destino, SI32 tamBloque, SI32 cantBloques, AUArchivo* punteroRedundanteTemporal){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::leer")
	NBASSERT(_mutexLockCount > 0)
	NBASSERT(this==punteroRedundanteTemporal);
	SI32 bytesLeidos		= AAsset_read((AAsset*)_aasset, destino, tamBloque * cantBloques); NBASSERT(bytesLeidos>=0);
	_posActual				+= bytesLeidos;
	//if(bytesLeidos != (tamBloque * cantBloques)){
	//	PRINTF_INFO("FREAD fallo terminando en posicion: %d (_posActual %d)\n", (SI32)ftell(_aasset), _posActual);
	//}
	//NBASSERT(ftell(_aasset)==_posActual);
	_lastOp					= ENFileOp_Read;
	AU_GESTOR_PILA_LLAMADAS_POP
	return (bytesLeidos / tamBloque);
}

SI32 AUArchivoFisicoAPK::escribir(const void* fuente, SI32 tamBloque, SI32 cantBloques, AUArchivo* punteroRedundanteTemporal){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::escribir")
	NBASSERT(_mutexLockCount > 0)
	NBASSERT(false) //No permitido en un APK
	//_lastOp = ENFileOp_Write;
	AU_GESTOR_PILA_LLAMADAS_POP
	return 0;
}

SI32 AUArchivoFisicoAPK::posicionActual() const {
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::posicionActual")
	NBASSERT(_mutexLockCount > 0)
	//NBASSERT(ftell(_aasset)==_posActual);
	AU_GESTOR_PILA_LLAMADAS_POP
	return _posActual;
}

bool AUArchivoFisicoAPK::moverDesdeInicio(const SI32 posicionDestino){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::moverDesdeInicio")
	NBASSERT(_mutexLockCount > 0)
	NBASSERT(posicionDestino>=0);
	bool exito = true;
	if(_posActual!=posicionDestino){
		SI32 rSeek = AAsset_seek((AAsset*)_aasset, posicionDestino, SEEK_SET);
		if(rSeek >= 0){
			NBASSERT(rSeek == posicionDestino)
			_posActual = posicionDestino;
		} else {
			exito = false;
			//_posActual = ftell(_aasset);
			PRINTF_ERROR("moviendo posicion SEEK_SET archivo fisicoAPK: posFinal(%d) posicionDestino(%d)\n", _posActual, posicionDestino);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP
	return exito;
}

bool AUArchivoFisicoAPK::moverDesdePosActual(const SI32 cantidadBytes){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::moverDesdePosActual")
	NBASSERT(_mutexLockCount > 0)
	bool exito = true;
	if(cantidadBytes!=0){
		SI32 rSeek = AAsset_seek((AAsset*)_aasset, cantidadBytes, SEEK_CUR);
		if(rSeek >= 0){
			NBASSERT(rSeek == (_posActual + cantidadBytes))
			_posActual += cantidadBytes;
		} else {
			exito = false;
			//_posActual = ftell(_aasset);
			PRINTF_ERROR("moviendo posicion SEEK_CUR archivo fisico: cantidadBytes(%d) posFinal(%d)\n", cantidadBytes, _posActual);
		}
	}
	AU_GESTOR_PILA_LLAMADAS_POP
	return exito;
}

bool AUArchivoFisicoAPK::moverDesdePosFinal(const SI32 cantidadBytes){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::moverDesdePosFinal")
	NBASSERT(_mutexLockCount > 0)
	bool exito = true; PRINTF_INFO("moverDesdePosFinalAPK-ini.\n");
	if(cantidadBytes!=0){
		SI32 rSeek = AAsset_seek((AAsset*)_aasset, cantidadBytes, SEEK_END);
		if(rSeek >= 0){
			_posActual = cantidadBytes;
		} else {
			exito = false;
			//_posActual = ftell(_aasset);
			PRINTF_ERROR("moviendo posicion SEEK_END archivo fisico: cantidadBytes(%d) posFinal(%d)\n", cantidadBytes, _posActual);
		}
	}
	PRINTF_INFO("moverDesdePosFinalAPK-fin.\n");
	AU_GESTOR_PILA_LLAMADAS_POP
	return exito;
}

void AUArchivoFisicoAPK::rebobinar(){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::rebobinar")
	NBASSERT(_mutexLockCount > 0)
	SI32 rSeek = AAsset_seek((AAsset*)_aasset, 0, SEEK_SET); //rewind(_aasset);
	NBASSERT(rSeek==0)
	_posActual = 0; //NBASSERT(_posActual==ftell(_aasset));
	AU_GESTOR_PILA_LLAMADAS_POP
}

void AUArchivoFisicoAPK::cerrar(){
	AU_GESTOR_PILA_LLAMADAS_PUSH("AUArchivoFisicoAPK::cerrar")
	if(_aasset != NULL){
		AAsset_close((AAsset*)_aasset);
		_aasset = NULL;
	}
	AU_GESTOR_PILA_LLAMADAS_POP
}

//

AUOBJMETODOS_CLASESID_UNICLASE(AUArchivoFisicoAPK)
AUOBJMETODOS_CLASESNOMBRES_UNICLASE(AUArchivoFisicoAPK, "AUArchivoFisicoAPK")
AUOBJMETODOS_CLONAR_NULL(AUArchivoFisicoAPK)

