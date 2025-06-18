//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h" //No need for framework's classes?
//#include "AUFrameworkBaseDefiniciones.h" //for NBASSERT and PRINTF_* macros
#include "AUAppGlueAndroidJNI.h"
//Android and JNI headers
#include <jni.h>
#include "nb/core/NBThreadStorage.h"

//-------------------------
// AUAppGlueAndroidJNI
//-------------------------

//Thread local storage
typedef struct STAUAppGlueAndroidJNIThreadState_ {
	STNBArray stack; //STGlueAndroidJNIEnvVars
} STAUAppGlueAndroidJNIThreadState;

static STNBThreadStorage __statePerThread = NBThreadStorage_initialValue;
void __statePerThreadCreateMthd(void);
void __statePerThreadDestroyDataMthd(void* data);
//
void AUAppGlueAndroidJNIThreadState_init(STAUAppGlueAndroidJNIThreadState* obj);
void AUAppGlueAndroidJNIThreadState_destroy(STAUAppGlueAndroidJNIThreadState* obj);

#ifdef __ANDROID__
	//is android
#endif

/*AUAppGlueAndroidJNI::AUAppGlueAndroidJNI(void* pJvm / *JavaVM* /) {
	_jniVer	= -1;
	_jvm	= NULL;
	//
	{
		jint vers[] = {
			JNI_VERSION_1_6, / *JNI_VERSION_1_5,* / JNI_VERSION_1_4, / *JNI_VERSION_1_3,* / JNI_VERSION_1_2, JNI_VERSION_1_1
		};
		const char* vNames[] = {
			"JNI_VERSION_1_6", / *"JNI_VERSION_1_5",* / "JNI_VERSION_1_4", / *"JNI_VERSION_1_3",* / "JNI_VERSION_1_2", "JNI_VERSION_1_1"
		};
		SI32 i; const SI32 versSz = (sizeof(vers) / sizeof(vers[0]));
		for(i = 0; i < versSz; i++){
			JNIEnv* jEnv = NULL;
			jint rGetEnv = ((JavaVM*)pJvm)->GetEnv((void **)&jEnv, vers[i]);
			if(rGetEnv != JNI_OK){
				PRINTF_WARNING("AUAppGlueAndroidJNI, JVM does not support jni version %d '%s'.\n", vers[i], vNames[i]);
			} else {
				PRINTF_INFO("AUAppGlueAndroidJNI, JVM supports jni version %d '%s'.\n", vers[i], vNames[i]);
				break;
			}
		}
		if(i == versSz){
			PRINTF_ERROR("AUAppGlueAndroidJNI, JVM does not support ANU known jni version.\n");
			NBASSERT(false)
		} else {
			_jniVer = vers[i];
			_jvm	= pJvm;
		}
	}
	//
	pthread_mutex_init(&_envsMutex, NULL);
	_envsArr	= NULL;
	_envsUse	= 0;
	_envsSize	= 0;
}*/

AUAppGlueAndroidJNI::AUAppGlueAndroidJNI(void* pEnv /*JNIEnv*/, void* pActivity /*jobject*/) {
	JNIEnv* jEnv = (JNIEnv*)pEnv;
	//
	PRINTF_INFO("AUAppGlueAndroidJNI, AUAppGlueAndroidJNI-start.\n");
	//
	NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
	//
	_jniVer		= -1;
	_jvm		= NULL;
	_jActivity	= NULL; if(pActivity != NULL) _jActivity = (jobject)jEnv->NewGlobalRef((jobject)pActivity);
	_jHandler	= NULL;
	//
	if(jEnv != NULL){
		JavaVM* jvm = NULL;
		if(jEnv->GetJavaVM(&jvm) != 0){
			PRINTF_ERROR("AUAppGlueAndroidJNI, could not obtain jni's JavaVirtualMachine.\n");
		} else {
			jint vers[] = {
				JNI_VERSION_1_6, /*JNI_VERSION_1_5,*/ JNI_VERSION_1_4, /*JNI_VERSION_1_3,*/ JNI_VERSION_1_2, JNI_VERSION_1_1
			};
			const char* vNames[] = {
				"JNI_VERSION_1_6", /*"JNI_VERSION_1_5",*/ "JNI_VERSION_1_4", /*"JNI_VERSION_1_3",*/ "JNI_VERSION_1_2", "JNI_VERSION_1_1"
			};
			SI32 i; const SI32 versSz = (sizeof(vers) / sizeof(vers[0]));
			for(i = 0; i < versSz; i++){
				JNIEnv* jEnv = NULL;
				jint rGetEnv = jvm->GetEnv((void **)&jEnv, vers[i]);
				if(rGetEnv != JNI_OK){
					PRINTF_WARNING("AUAppGlueAndroidJNI, JVM does not support jni version %d '%s'.\n", vers[i], vNames[i]);
				} else {
					PRINTF_INFO("AUAppGlueAndroidJNI, JVM supports jni version %d '%s'.\n", vers[i], vNames[i]);
					break;
				}
			}
			if(i == versSz){
				PRINTF_ERROR("AUAppGlueAndroidJNI, JVM does not support ANU known jni version.\n");
				NBASSERT(false)
			} else {
				_jniVer = vers[i];
				_jvm	= jvm;
				//Create main hadler
				{
					jclass clsHndlr = jEnv->FindClass("android/os/Handler"); NBASSERT(clsHndlr != NULL)
					if(clsHndlr != NULL){
						jmethodID mInitHndlr = jEnv->GetMethodID(clsHndlr, "<init>", "()V"); NBASSERT(mInitHndlr != NULL)
						if(mInitHndlr != NULL){
							jobject jHandler = jEnv->NewObject(clsHndlr, mInitHndlr); NBASSERT(jHandler != NULL)
							if(jHandler != NULL){
								_jHandler = jEnv->NewGlobalRef(jHandler);
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsHndlr)
					}
				}
			}
		}
	}
	//
	//pthread_mutex_init(&_envsMutex, NULL);
	//_envsArr	= NULL;
	//_envsUse	= 0;
	//_envsSize	= 0;
	//
	PRINTF_INFO("AUAppGlueAndroidJNI, AUAppGlueAndroidJNI-end _jvm(%llu).\n", (UI64)_jvm);
}

AUAppGlueAndroidJNI::~AUAppGlueAndroidJNI(){
	PRINTF_INFO("AUAppGlueAndroidJNI, ~AUAppGlueAndroidJNI-start.\n");
	JNIEnv* jEnv = (JNIEnv*)this->curEnv(); NBASSERT(jEnv != NULL)
	if(jEnv != NULL){
		if(_jActivity != NULL){
			jEnv->DeleteGlobalRef((jobject)_jActivity);
			_jActivity = NULL;
		}
		if(_jHandler != NULL){
			jEnv->DeleteGlobalRef((jobject)_jHandler);
			_jHandler = NULL;
		}
	}
	//
	_jvm = NULL;
	/*
	//Clear data (should be empty)
	//pthread_mutex_lock(&_envsMutex);
	{
		NBASSERT(_envsUse == 0)
		if(_envsArr != NULL){
			SI32 i = 0;
			{
				for(i = 0; i < _envsUse; i++){
					STGlueAndroidJNIEnv* data = &_envsArr[i];
					if(data->stackVarsArr != NULL){
						SI32 i;
						for(i = 0; i < data->stackVarsUse; i++){
							NBASSERT(data->stackVarsArr[i] != NULL)
							if(data->stackVarsArr[i] != NULL){
								STGlueAndroidJNIEnvVars* vars = data->stackVarsArr[i];
								vars->refName	= NULL;
								vars->jObj		= NULL;
								vars->jCls		= NULL;
								//
								free(data->stackVarsArr[i]);
								data->stackVarsArr[i] = NULL;
							}
						}
						free(data->stackVarsArr);
						data->stackVarsArr = NULL;
					}
					data->stackVarsUse = 0;
					data->stackVarsSize = 0;
				}
			}
			free(_envsArr);
			_envsArr = NULL;
		}
		_envsUse = 0;
		_envsSize = 0;
	}
	pthread_mutex_unlock(&_envsMutex);
	pthread_mutex_destroy(&_envsMutex);
	*/
	PRINTF_INFO("AUAppGlueAndroidJNI, ~AUAppGlueAndroidJNI-end.\n");
}

//

STNBAndroidJniItf AUAppGlueAndroidJNI::getAndroidJniItf(){
	STNBAndroidJniItf r;
	NBMemory_setZeroSt(r, STNBAndroidJniItf);
	r.jniVersion	= AUAppGlueAndroidJNI::jniVersion_;
	r.jActivity		= AUAppGlueAndroidJNI::jActivity_;
	r.curEnv		= AUAppGlueAndroidJNI::curEnv_;
	return r;
}

//

SI32 AUAppGlueAndroidJNI::jniVersion() const {
	return _jniVer;
}

void* /*jobject*/ AUAppGlueAndroidJNI::jActivity() const {
	return _jActivity;
}

void* /*jobject*/ AUAppGlueAndroidJNI::jHandler() const {
	return _jHandler;
}


//

/*void* / *JNIEnv* / AUAppGlueAndroidJNI::attachCurrentThread(const char* refName){
	return this->privAttachCurrentThread(NULL, NULL, refName);
}*/

//for instance calls
/*void* / *JNIEnv* / AUAppGlueAndroidJNI::attachCurrentThreadWithObj(void* jObj / *jobject* /, const char* refName){
	return this->privAttachCurrentThread(jObj, NULL, refName);
}*/

//for static calls
/*void* / *JNIEnv* / AUAppGlueAndroidJNI::attachCurrentThreadWithCls(void* jCls / *jclass* /, const char* refName){
	return this->privAttachCurrentThread(NULL, jCls, refName);
}*/

/*void* / *JNIEnv* / AUAppGlueAndroidJNI::privAttachCurrentThread(void* jObj / *jobject* /, void* jCls / *jclass* /, const char* refName){
	void* r = NULL;
	{
		JNIEnv* jEnv = NULL;
		const jint rs = ((JavaVM*)_jvm)->AttachCurrentThread(&jEnv, NULL);
		if(rs != JNI_OK){
			PRINTF_ERROR("AUAppGlueAndroidJNI, attachCurrentThread could not obtain new JEnv.\n");
			NBASSERT(false)
		} else {
			NBASSERT(jEnv != NULL)
			this->privEnvVarsPush(jEnv, jObj, jCls, refName);
			r = jEnv;
		}
	}
	return r;
}*/

/*bool AUAppGlueAndroidJNI::detachCurrentThread(void* jEnv / *JNIEnv* /){
	bool r = false;
	this->envVarsPop(jEnv);
	const jint rs = ((JavaVM*)_jvm)->DetachCurrentThread();
	if(rs != JNI_OK){
		PRINTF_ERROR("AUAppGlueAndroidJNI, detachCurrentThread unsuccess.\n");
		NBASSERT(false)
	} else {
		r = true;
	}
	return r;
}*/

//

void __statePerThreadCreateMthd(void){
	//PRINTF_INFO("__statePerThreadCreateMthd.\n");
	NBThreadStorage_create(&__statePerThread);
	NBASSERT(NBThreadStorage_getData(&__statePerThread) == NULL)
}

void __statePerThreadDestroyDataMthd(void* data){
	//PRINTF_INFO("__statePerThreadDestroyDataMthd.\n");
	if(data != NULL){
		STAUAppGlueAndroidJNIThreadState* st = (STAUAppGlueAndroidJNIThreadState*)data;
		AUAppGlueAndroidJNIThreadState_destroy(st);
		NBMemory_free(st);
		st = NULL;
	}
}

void AUAppGlueAndroidJNIThreadState_init(STAUAppGlueAndroidJNIThreadState* obj){
	NBMemory_setZeroSt(*obj, STAUAppGlueAndroidJNIThreadState);
	NBArray_init(&obj->stack, sizeof(STGlueAndroidJNIThreadEnv), NULL); //STGlueAndroidJNIThreadEnv
}

void AUAppGlueAndroidJNIThreadState_destroy(STAUAppGlueAndroidJNIThreadState* obj){
	NBASSERT(obj->stack.use == 0)
	{
		//SI32 i; for(i = 0 ; i < obj->stack.use; i++){
		//	STGlueAndroidJNIThreadEnv* itm = NBArray_itmPtrAtIndex(&obj->stack, STGlueAndroidJNIThreadEnv, i);
		//}
		NBArray_empty(&obj->stack);
		NBArray_release(&obj->stack);
		PRINTF_WARNING("AUAppGlueAndroidJNIThreadState_destroy.\n");
	}
}

//

SI32 AUAppGlueAndroidJNI::envVarsPush(void* jEnv /*JNIEnv*/, const char* refName){
	return this->privEnvVarsPush(jEnv, NULL, NULL, refName);
}

SI32 AUAppGlueAndroidJNI::envVarsPushWithObj(void* jEnv /*JNIEnv*/, void* jObj /*jobject*/, const char* refName){
	return this->privEnvVarsPush(jEnv, jObj, NULL, refName);
}

SI32 AUAppGlueAndroidJNI::envVarsPushWithCls(void* jEnv /*JNIEnv*/, void* jCls /*jclass*/, const char* refName){
	return this->privEnvVarsPush(jEnv, NULL, jCls, refName);
}

SI32 AUAppGlueAndroidJNI::privEnvVarsPush(void* jEnv /*JNIEnv*/, void* jObj /*jobject*/, void* jCls /*jclass*/, const char* refName){
	SI32 rStackPos = -1;
	{
		NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
		{
			STAUAppGlueAndroidJNIThreadState* tState = (STAUAppGlueAndroidJNIThreadState*)NBThreadStorage_getData(&__statePerThread);
			if(tState == NULL){
				//Set initial data
				tState = NBMemory_allocType(STAUAppGlueAndroidJNIThreadState);
				NBMemory_setZeroSt(*tState, STAUAppGlueAndroidJNIThreadState);
				AUAppGlueAndroidJNIThreadState_init(tState);
				NBThreadStorage_setData(&__statePerThread, tState, __statePerThreadDestroyDataMthd);
				NBASSERT(NBThreadStorage_getData(&__statePerThread) != NULL)
			}
			//Push env
			NBASSERT(tState != NULL)
			if(tState != NULL){
				STGlueAndroidJNIThreadEnv env;
				NBMemory_setZeroSt(env, STGlueAndroidJNIThreadEnv);
				env.jEnv		= jEnv; //JNIEnv
				env.jObj		= jObj;	//jobject, for instance calls
				env.jCls		= jCls;	//jclass, for static calls
				if(refName != NULL){
					env.refName	= NBString_strNewBuffer(refName);
				}
				NBArray_addValue(&tState->stack, env);
				rStackPos = (tState->stack.use - 1);
			}
		}
	}
	/*pthread_mutex_lock(&_envsMutex);
	//Find curr env
	SI32 i = 0;
	{
		for(i = 0; i < _envsUse; i++){
			STGlueAndroidJNIEnv* data = &_envsArr[i];
			if(data->jEnv == jEnv){
				//Add new vars to stack
				if(data->stackVarsUse == data->stackVarsSize){
					//Grow array
					data->stackVarsSize += 4;
					STGlueAndroidJNIEnvVars** newArr = (STGlueAndroidJNIEnvVars**)malloc(sizeof(STGlueAndroidJNIEnvVars*) * data->stackVarsSize);
					if(data->stackVarsArr != NULL){
						SI32 i; for(i = 0; i < data->stackVarsUse; i++) newArr[i] = data->stackVarsArr[i];
						free(data->stackVarsArr);
					}
					data->stackVarsArr = newArr;
				}
				//Add data to stack
				{
					NBASSERT(data->stackVarsUse > 0) //never empty, should be deleted after last item pop.
					STGlueAndroidJNIEnvVars* varsData = (STGlueAndroidJNIEnvVars*)malloc(sizeof(STGlueAndroidJNIEnvVars));
					varsData->refName	= refName;
					varsData->jObj		= jObj;
					varsData->jCls		= jCls;
					data->stackVarsArr[data->stackVarsUse++] = varsData;
					rStackPos = (data->stackVarsUse - 1);
#					ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
					if(data->stackVarsUse == 1){
						PRINTF_INFO("AUAppGlueAndroidJNI, env(%d) stack(%d) push at '%s'.\n", i, rStackPos, refName);
					} else {
						const STGlueAndroidJNIEnvVars* prevData = data->stackVarsArr[data->stackVarsUse - 2];
						PRINTF_INFO("AUAppGlueAndroidJNI, env(%d) stack(%d) push at '%s' (prev '%s').\n", i, rStackPos, refName, (prevData->refName != NULL ? prevData->refName : "[unnamed]"));
					}
#					endif
				}
				break;
			}
		}
	}
	//Add new env
	if(i == _envsUse){
		if(_envsUse == _envsSize){
			//Grow array
			_envsSize += 4;
			STGlueAndroidJNIEnv* newArr = (STGlueAndroidJNIEnv*) malloc(sizeof(STGlueAndroidJNIEnv) * _envsSize);
			if(_envsArr != NULL){
				SI32 i; for(i = 0; i < _envsUse; i++) newArr[i] = _envsArr[i];
				free(_envsArr);
			}
			_envsArr = newArr;
		}
		//Add data
		{
			STGlueAndroidJNIEnvVars* varsData = (STGlueAndroidJNIEnvVars*)malloc(sizeof(STGlueAndroidJNIEnvVars));
			varsData->refName	= refName;
			varsData->jObj		= jObj;
			varsData->jCls		= jCls;
			//
			STGlueAndroidJNIEnv data;
			data.jEnv			= jEnv;
			data.stackVarsUse	= 1;
			data.stackVarsSize	= 4;
			data.stackVarsArr	= (STGlueAndroidJNIEnvVars**) malloc(sizeof(STGlueAndroidJNIEnvVars*) * _envsSize);
			data.stackVarsArr[0] = varsData;
			_envsArr[_envsUse++] = data;
			rStackPos = 0;
#			ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
			if(_envsUse > 1){
				STGlueAndroidJNIEnv* prevEnv = &_envsArr[_envsUse - 2];
				NBASSERT(prevEnv->stackVarsUse > 0) //never empty, should be deleted after last item pop.
				if(prevEnv->stackVarsUse > 0){
					const STGlueAndroidJNIEnvVars* prevLastData = prevEnv->stackVarsArr[prevEnv->stackVarsUse - 1];
					PRINTF_INFO("AUAppGlueAndroidJNI, parallel env(%d) stack(0) pushed at '%s' (other env is at '%s').\n", (_envsUse - 1), refName, (prevLastData->refName != NULL ? prevLastData->refName : "[unnamed]"));
				} else {
					PRINTF_INFO("AUAppGlueAndroidJNI, parallel env(%d) stack(0) pushed at '%s' (other env with empty stack).\n", (_envsUse - 1), refName);
				}
			}
#			endif
		}
	}
	pthread_mutex_unlock(&_envsMutex);
	NBASSERT(rStackPos >= 0)*/
	return rStackPos;
}

SI32 AUAppGlueAndroidJNI::envVarsPop(void* jEnv /*JNIEnv*/){
	SI32 rStackPos = -1;
	NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
	{
		STAUAppGlueAndroidJNIThreadState* tState = (STAUAppGlueAndroidJNIThreadState*)NBThreadStorage_getData(&__statePerThread);
		if(tState == NULL){
			PRINTF_ERROR("AUAppGlueAndroidJNI::envVarsPop with no push (thread state is NULL).\n");
			NBASSERT(tState != NULL)
		} else if(tState->stack.use <= 0){
			PRINTF_ERROR("AUAppGlueAndroidJNI::envVarsPop with no push (stack is empty).\n");
			NBASSERT(tState->stack.use > 0)
		} else {
			//Remove last item from stack
			//STGlueAndroidJNIThreadEnv* itm = NBArray_itmPtrAtIndex(&tState->stack, STGlueAndroidJNIThreadEnv, tState->stack.use - 1);
			NBArray_removeItemAtIndex(&tState->stack, tState->stack.use - 1);
			rStackPos = tState->stack.use;
		}
	}
	/*pthread_mutex_lock(&_envsMutex);
	//Find curr env
	SI32 i;
	for(i = 0; i < _envsUse; i++){
		STGlueAndroidJNIEnv* data = &_envsArr[i];
		if(data->jEnv == jEnv){
			NBASSERT(data->stackVarsUse > 0) //never empty, should be deleted after last item pop.
			if(data->stackVarsUse > 0){
				STGlueAndroidJNIEnvVars* varsData = data->stackVarsArr[data->stackVarsUse - 1];
				free(varsData);
				data->stackVarsUse--;
				rStackPos = data->stackVarsUse;
			} else {
				PRINTF_ERROR("AUAppGlueAndroidJNI, envVarsPop with an empty stack (missing envVarsPush or extra envVarsPop?).\n");
			}
			break;
		}
	}
	if(i == _envsUse){
		NBASSERT(false)
		PRINTF_ERROR("AUAppGlueAndroidJNI, envVarsPop env not found (wrong 'jEnv' param or extra envVarsPop?).\n");
	} else {
		NBASSERT(_envsUse > 0)
		if(_envsUse > 0){
			//Remove env if stack is empty
			STGlueAndroidJNIEnv* data = &_envsArr[i];
			if(data->stackVarsUse == 0){
				if(data->stackVarsArr != NULL) free(data->stackVarsArr); data->stackVarsArr = NULL;
				data->stackVarsSize = 0;
				data->jEnv = NULL;
				//Fill hole (move items)
				_envsUse--;
				SI32 i2; for(i2 = i; i2 < _envsUse; i2++) _envsArr[i2] = _envsArr[i2 + 1];
#				ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
				//if(_envsUse == 0){ PRINTF_INFO("AUAppGlueAndroidJNI, ALL ENVS RELEASED.\n"); }
#				endif
			}
		}
	}
	pthread_mutex_unlock(&_envsMutex);
	NBASSERT(rStackPos >= 0)*/
	return rStackPos;
}

void* /*JNIEnv*/ AUAppGlueAndroidJNI::curEnvCalculated(){
	void* r = NULL;
	NBASSERT(_jvm != NULL)
	if(_jvm != NULL){
		JNIEnv* jEnv = NULL;
		jint rGetEnv = ((JavaVM*)_jvm)->GetEnv((void **)&jEnv, _jniVer);
		if (rGetEnv == JNI_OK) {
			//Current thread is attached to JVM, current JENV returned.
			r = jEnv;
		} else if (rGetEnv == JNI_EDETACHED) {
			//Current thread is NOT attached to JVM, current JENV returned
			//Can be attached with 'AttachCurrentThread' to get a JENV.
			PRINTF_ERROR("AUAppGlueAndroidJNI, current thread is NOT attached to JVM.\n");
		} else if (rGetEnv == JNI_EVERSION) {
			//Current JNI_VERSION is not supported.
			PRINTF_ERROR("AUAppGlueAndroidJNI, JNI_VERSION is not supported.\n");
		}
		NBASSERT(r != NULL)
	}
	return r;
}

void* /*JNIEnv*/ AUAppGlueAndroidJNI::curEnv(){
	void* r = NULL;
	NBThreadStorage_initOnce(&__statePerThread, __statePerThreadCreateMthd);
	{
		STAUAppGlueAndroidJNIThreadState* tState = (STAUAppGlueAndroidJNIThreadState*)NBThreadStorage_getData(&__statePerThread);
		if(tState == NULL){
			PRINTF_ERROR("AUAppGlueAndroidJNI::curEnv with no push (thread state is NULL).\n");
			r = this->curEnvCalculated();
		} else if(tState->stack.use <= 0){
			PRINTF_ERROR("AUAppGlueAndroidJNI::curEnv with no push (stack is empty).\n");
			r = this->curEnvCalculated();
		} else {
			//Obtain jenv
			STGlueAndroidJNIThreadEnv* itm = NBArray_itmPtrAtIndex(&tState->stack, STGlueAndroidJNIThreadEnv, tState->stack.use - 1);
			r = itm->jEnv;
		}
	}
	NBASSERT(r != NULL);
	return r;
}

/*bool AUAppGlueAndroidJNI::curEnvVars(STGlueAndroidJNIEnvVars* dst, void** dstEnv / *JNIEnv* /){
	bool r = false;
	if(_jvm != NULL){
		JNIEnv* jEnv = NULL;
		jint rGetEnv = ((JavaVM*)_jvm)->GetEnv((void **)&jEnv, _jniVer);
		if (rGetEnv == JNI_OK) {
			//Current thread is attached to JVM, current JENV returned.
			//Search env
			SI32 i;
			for(i = 0; i < _envsUse; i++){
				STGlueAndroidJNIEnv* data = &_envsArr[i];
				if(data->jEnv == jEnv){
					NBASSERT(data->stackVarsUse > 0) //never empty, should be deleted after last item pop.
					if(data->stackVarsUse > 0){
						if(dst != NULL) *dst = *(data->stackVarsArr[data->stackVarsUse - 1]);
						if(dstEnv != NULL) *dstEnv = jEnv;
						r = true;
					}
					break;
				}
			}
		} else if (rGetEnv == JNI_EDETACHED) {
			//Current thread is NOT attached to JVM, current JENV returned
			//Can be attached with 'AttachCurrentThread' to get a JENV.
			PRINTF_ERROR("AUAppGlueAndroidJNI, current thread is NOT attached to JVM.\n");
		} else if (rGetEnv == JNI_EVERSION) {
			//Current JNI_VERSION is not supported.
			PRINTF_ERROR("AUAppGlueAndroidJNI, JNI_VERSION is not supported.\n");
		}
	}
	return r;
}*/

//Runnable

BOOL AUAppGlueAndroidJNI::addRunableForMainThread(void* jAppNative /*jobject*/, AURunnableRunMthd func, void* funcParam){
	BOOL r = FALSE;
	JNIEnv* jEnv = (JNIEnv*)this->curEnv();
	if(jEnv != NULL && jAppNative != NULL){
		jclass clsAppNative	= jEnv->FindClass("com/auframework/AppNative"); NBASSERT(clsAppNative != NULL)
		if(clsAppNative != NULL){
			jmethodID mAddRunn = jEnv->GetMethodID(clsAppNative, "addNativeRunnable", "(JJ)Z"); NBASSERT(mAddRunn != NULL)
			const jboolean rr = jEnv->CallBooleanMethod((jobject)jAppNative, mAddRunn, (jlong)func, (jlong)funcParam);
			if(rr){
				r = TRUE;
			}
		}
		NBJNI_DELETE_REF_LOCAL(jEnv, clsAppNative)
	}
	/*if(jEnv != NULL && _jActivity != NULL){
		jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
		jclass clsHndlr		= jEnv->FindClass("android/os/Handler"); NBASSERT(clsHndlr != NULL)
		jclass clsRunable	= jEnv->FindClass("com/auframework/AppNative$AURunnable"); NBASSERT(clsRunable != NULL)
		if(clsContext != NULL && clsHndlr != NULL && clsRunable != NULL){
			jmethodID mGetLooper = jEnv->GetMethodID(clsContext, "getMainLooper", "()Landroid/os/Looper;"); NBASSERT(mGetLooper != NULL)
			jmethodID mInitRunbl = jEnv->GetMethodID(clsRunable, "<init>", "(JJ)V"); NBASSERT(mInitRunbl != NULL)
			jmethodID mInitHndlr = jEnv->GetMethodID(clsHndlr, "<init>", "(Landroid/os/Looper;)V"); NBASSERT(mInitHndlr != NULL)
			jmethodID mPost = jEnv->GetMethodID(clsHndlr, "post", "(Ljava/lang/Runnable;)Z"); NBASSERT(mPost != NULL)
			jobject jMainLooper = jEnv->CallObjectMethod((jobject)_jActivity, mGetLooper);
			if(jMainLooper != NULL){
				jobject jHandler = jEnv->NewObject(clsHndlr, mInitHndlr, jMainLooper);
				if(jHandler != NULL){
					jobject jRunble = jEnv->NewObject(clsRunable, mInitRunbl, (jlong)func, (jlong)funcParam);
					if(jRunble != NULL){
						PRINTF_INFO("jHandler(%lld) mInitHndlr(%lld) mPost(%lld) jRunble(%lld).\n", (SI64)jHandler, (SI64)mInitHndlr, (SI64)mPost, (SI64)jRunble);
						const jboolean rr = jEnv->CallBooleanMethod(jHandler, mPost, jRunble);
						if(rr){
							r = TRUE;
						}
					}
					//NBJNI_DELETE_REF_LOCAL(jEnv, jRunble)
				}
				//NBJNI_DELETE_REF_LOCAL(jEnv, jHandler)
			}
		}
		NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
		NBJNI_DELETE_REF_LOCAL(jEnv, clsHndlr)
		NBJNI_DELETE_REF_LOCAL(jEnv, clsRunable)
	}*/
	return r;
}

//--------------
// Android utils
//--------------

/*
 public static final String PREFS_NAME = "AUNotificaciones";
 public static final String PREFS_NAME_DATA = "jsonData";
 
 guardarDatos
 //
 SharedPreferences settings = contexto.getSharedPreferences(PREFS_NAME, 0);
 SharedPreferences.Editor editor = settings.edit();
 editor.putString(PREFS_NAME_DATA, str.toString());
 editor.commit();
 */

bool AUAppGlueAndroidJNI::saveDataToSharedPrefs(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const char* prefName, const char* dataName, const char* data){
	bool r = false;
	if(pEnv != NULL && pContext != NULL && prefName != NULL && dataName != NULL){
		if(prefName[0] != '\0' && dataName[0] != '\0'){
			JNIEnv* jEnv		= (JNIEnv*) pEnv;
			jobject jContext	= (jobject) pContext;
			jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
			jclass clsPrefs		= jEnv->FindClass("android/content/SharedPreferences"); NBASSERT(clsPrefs != NULL)
			jclass clsPrefsEdit	= jEnv->FindClass("android/content/SharedPreferences$Editor"); NBASSERT(clsPrefsEdit != NULL)
			if(clsContext != NULL && clsPrefs != NULL && clsPrefsEdit != NULL){
				jmethodID mGetPrefs	= jEnv->GetMethodID(clsContext, "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;"); NBASSERT(mGetPrefs != NULL)
				jmethodID mEdit		= jEnv->GetMethodID(clsPrefs, "edit", "()Landroid/content/SharedPreferences$Editor;"); NBASSERT(mEdit != NULL)
				jmethodID mPutStr	= jEnv->GetMethodID(clsPrefsEdit, "putString", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;"); NBASSERT(mPutStr != NULL)
				jmethodID mCommit	= jEnv->GetMethodID(clsPrefsEdit, "commit", "()Z"); NBASSERT(mCommit != NULL)
				if(mGetPrefs != NULL && mEdit != NULL && mPutStr != NULL && mCommit != NULL){
					jstring jStrName	= jEnv->NewStringUTF(prefName);
					jobject prefs		= jEnv->CallObjectMethod(jContext, mGetPrefs, jStrName, (jint)0); NBASSERT(prefs != NULL)
					if(prefs != NULL){
						jobject editor = jEnv->CallObjectMethod(prefs, mEdit); NBASSERT(editor != NULL)
						if(editor != NULL){
							jstring jStrName = jEnv->NewStringUTF(dataName);
							jstring jStrData = jEnv->NewStringUTF(data);
							jEnv->CallObjectMethod(editor, mPutStr, jStrName, jStrData);
							const jboolean c = jEnv->CallBooleanMethod(editor, mCommit); NBASSERT(c == JNI_TRUE)
							if(c == JNI_TRUE){
								PRINTF_INFO("AUAppGlueAndroidJNI, saveDataToSharedPrefs, saved '%s':'%s' = '%s'.\n", prefName, dataName, data);
								r = true;
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jStrData)
							NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
							NBJNI_DELETE_REF_LOCAL(jEnv, editor)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, prefs)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsPrefsEdit)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsPrefs)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			}
		}
	}
	return r;
}

/*
 cargarDatos
 SharedPreferences settings = contexto.getSharedPreferences(PREFS_NAME, 0);
 String jsonStr = settings.getString(PREFS_NAME_DATA, "");
 */

bool AUAppGlueAndroidJNI::loadDataFromSharedPrefs(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const char* prefName, const char* dataName, AUCadenaMutable8* dst){
	bool r = false;
	if(pEnv != NULL && pContext != NULL && prefName != NULL && dataName != NULL){
		if(prefName[0] != '\0' && dataName[0] != '\0'){
			JNIEnv* jEnv		= (JNIEnv*) pEnv;
			jobject jContext	= (jobject) pContext;
			jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
			jclass clsPrefs		= jEnv->FindClass("android/content/SharedPreferences"); NBASSERT(clsPrefs != NULL)
			if(clsContext != NULL && clsPrefs != NULL){
				jmethodID mGetPrefs	= jEnv->GetMethodID(clsContext, "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;"); NBASSERT(mGetPrefs != NULL)
				jmethodID mGetStr	= jEnv->GetMethodID(clsPrefs, "getString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;"); NBASSERT(mGetStr != NULL)
				if(mGetPrefs != NULL && mGetStr != NULL){
					jstring jStrName	= jEnv->NewStringUTF(prefName);
					jobject prefs		= jEnv->CallObjectMethod(jContext, mGetPrefs, jStrName, (jint)0); NBASSERT(prefs != NULL)
					if(prefs != NULL){
						jstring jStrName	= jEnv->NewStringUTF(dataName);
						jstring jStrParam	= jEnv->NewStringUTF("");
						jstring jStr		= (jstring)jEnv->CallObjectMethod(prefs, mGetStr, jStrName, jStrParam); NBASSERT(jStr != NULL)
						if(jStr != NULL){
							const char* str = jEnv->GetStringUTFChars(jStr, 0);
							if(dst != NULL) dst->agregar(str);
							//PRINTF_INFO("AUAppGlueAndroidJNI, loadDataToSharedPrefs, loaded '%s':'%s' = '%s'.\n", prefName, dataName, str);
							jEnv->ReleaseStringUTFChars(jStr, str);
							NBJNI_DELETE_REF_LOCAL(jEnv, jStr)
							r = true;
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrParam)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
						NBJNI_DELETE_REF_LOCAL(jEnv, prefs)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsPrefs)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			}
		}
	}
	return r;
}

//--------------
//Static methods (for C callers)
//--------------

SI32 AUAppGlueAndroidJNI::jniVersion_(void* obj){
	return ((AUAppGlueAndroidJNI*)obj)->jniVersion();
}

void* /*jobject*/ AUAppGlueAndroidJNI::jActivity_(void* obj){
	return ((AUAppGlueAndroidJNI*)obj)->jActivity();
}

void* /*JNIEnv*/ AUAppGlueAndroidJNI::curEnv_(void* obj){
	return ((AUAppGlueAndroidJNI*)obj)->curEnv();
}

//--------------
// Android permissions (API 23+ requires explicit permission from user)
//--------------

//
//context.checkPermission(permission, android.os.Process.myPid(), Process.myUid());
//
BOOL AUAppGlueAndroidJNI::isPermissionGranted(void* pEnv /*JNIEnv*/, void* /*jobject*/ pContext, const char* permFullName){
	BOOL r = FALSE;
	JNIEnv* jEnv		= (JNIEnv*)pEnv;
	jobject jContext	= (jobject)pContext;
	jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
	jclass clsAProc		= jEnv->FindClass("android/os/Process"); NBASSERT(clsAProc != NULL)
	if(clsContext != NULL && clsAProc != NULL){
		jmethodID mMyPid	= jEnv->GetStaticMethodID(clsAProc, "myPid", "()I"); NBASSERT(mMyPid != NULL)
		jmethodID mMyUid	= jEnv->GetStaticMethodID(clsAProc, "myUid", "()I"); NBASSERT(mMyUid != NULL)
		jmethodID mCheck	= jEnv->GetMethodID(clsContext, "checkPermission", "(Ljava/lang/String;II)I"); NBASSERT(mCheck != NULL)
		if(mMyPid != NULL && mMyUid != NULL && mCheck != NULL){
			const jint myPid = jEnv->CallStaticIntMethod(clsAProc, mMyPid);
			const jint myUid = jEnv->CallStaticIntMethod(clsAProc, mMyUid);
			//PRINTF_INFO("Process myPid(%d) myUid(%d).\n", myPid, myUid);
			{
				jstring jPermName = jEnv->NewStringUTF(permFullName);
				//PRINTF_INFO("isPermissionGranted('%s')?.\n", permFullName);
				const jint rr = jEnv->CallIntMethod(jContext, mCheck, jPermName, myPid, myUid);
				//android/content/pm/PackageManager.PERMISSION_GRANTED, Constant Value: 0 (0x00000000)
				//android/content/pm/PackageManager.PERMISSION_DENIED, Constant Value: -1 (0xffffffff)
				r = (rr == 0 ? TRUE : FALSE);
				//PRINTF_INFO("isPermissionGranted('%s') = '%s'.\n", permFullName, r ? "YES" : "NO");
				NBJNI_DELETE_REF_LOCAL(jEnv, jPermName)
			}
		}
		//Release
		NBJNI_DELETE_REF_LOCAL(jEnv, clsAProc)
		NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
	}
	return r;
}

BOOL AUAppGlueAndroidJNI::requestPermissions(void* pEnv /*JNIEnv*/, void* /*jobject*/ pActivity, const char** perms, const SI32 permsSz){
	BOOL r = FALSE;
	JNIEnv* jEnv		= (JNIEnv*)pEnv;
	jobject jActivity	= (jobject)pActivity;
	jclass clsString	= jEnv->FindClass("java/lang/String"); NBASSERT(clsString != NULL)
	jclass clsActivity	= jEnv->FindClass("android/app/Activity"); NBASSERT(clsActivity != NULL)
	if(clsString != NULL && clsActivity != NULL){
		jmethodID mReqPerm	= jEnv->GetMethodID(clsActivity, "requestPermissions", "([Ljava/lang/String;I)V"); NBASSERT(mReqPerm != NULL)
		if(mReqPerm != NULL){
			jobjectArray jPerms =  (jobjectArray)jEnv->NewObjectArray(permsSz, clsString, jEnv->NewStringUTF(""));
			//Populate array
			{
				SI32 i; for(i = 0; i < permsSz; i++){
					jEnv->SetObjectArrayElement(jPerms, i, jEnv->NewStringUTF(perms[i]));
				}
			}
			//Call
			jEnv->CallVoidMethod(jActivity, mReqPerm, jPerms, (jint)12321);
			NBJNI_DELETE_REF_LOCAL(jEnv, jPerms)
			r = TRUE;
		}
		NBJNI_DELETE_REF_LOCAL(jEnv, clsString)
		NBJNI_DELETE_REF_LOCAL(jEnv, clsActivity)
	}
	return r;
}

//--------------
// Android utils
//--------------

void* AUAppGlueAndroidJNI::getNewInstanceOfClass(void* pEnv /*JNIEnv*/, void* pClassObj /*jobject*/){
	void* r = NULL;
	if(pEnv != NULL && pClassObj != NULL){
		JNIEnv* jEnv	= (JNIEnv*)pEnv;
		jobject jClass	= (jobject) pClassObj;
		jclass clsClass	= jEnv->FindClass("java/lang/Class"); NBASSERT(clsClass != NULL)
		if(clsClass != NULL){
			jmethodID mGetName	= jEnv->GetMethodID(clsClass, "getCanonicalName", "()Ljava/lang/String;"); NBASSERT(mGetName != NULL)
			jmethodID mNewInst	= jEnv->GetMethodID(clsClass, "newInstance", "()Ljava/lang/Object;"); NBASSERT(mNewInst != NULL)
			if(mGetName != NULL && mNewInst != NULL){
				jstring jClsName = (jstring)jEnv->CallObjectMethod(jClass, mGetName); NBASSERT(jClsName != NULL)
				if(jClsName != NULL){
					jobject newInstance = jEnv->CallObjectMethod(jClass, mNewInst); NBASSERT(newInstance != NULL)
					if(newInstance == NULL){
						if(jEnv->ExceptionCheck()){ jEnv->ExceptionDescribe(); jEnv->ExceptionClear(); } //consume Exception
						const char* strClsName = jEnv->GetStringUTFChars(jClsName, 0);
						PRINTF_ERROR("AUAppGlueAndroidStore, could not create new instance of '%s'.\n", strClsName);
						jEnv->ReleaseStringUTFChars(jClsName, strClsName);
					} else {
						r = newInstance;
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jClsName)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsClass)
		}
	}
	return r;
}

/*
 //JAVA CODE
 private static String getLauncherClassName(Context context) {
	PackageManager pm = context.getPackageManager();
	Intent intent = new Intent(Intent.ACTION_MAIN);
	intent.addCategory(Intent.CATEGORY_LAUNCHER);
	List<ResolveInfo> resolveInfos = pm.queryIntentActivities(intent, 0);
	for (ResolveInfo resolveInfo : resolveInfos) {
 String pkgName = resolveInfo.activityInfo.applicationInfo.packageName;
 if (pkgName.equalsIgnoreCase(context.getPackageName())) {
 String className = resolveInfo.activityInfo.name;
 return className;
 }
	}
	return null;
 }
 */

void* /*jstring*/ AUAppGlueAndroidJNI::getPackageName(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/){
	jstring r = NULL;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
		if(clsContext != NULL){
			jmethodID mGetPkgName = jEnv->GetMethodID(clsContext, "getPackageName", "()Ljava/lang/String;"); NBASSERT(mGetPkgName != NULL)
			if(mGetPkgName != NULL){
				r = (jstring)jEnv->CallObjectMethod(jContext, mGetPkgName);
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
		}
	}
	return r;
}

void* /*jstring*/ AUAppGlueAndroidJNI::getLauncherClassName(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/){
	jstring launchClsName = NULL;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jclass clsString	= jEnv->FindClass("java/lang/String");
		jclass clsContext	= jEnv->FindClass("android/content/Context");
		jclass clsPkgMngr	= jEnv->FindClass("android/content/pm/PackageManager");
		jclass clsResInfo	= jEnv->FindClass("android/content/pm/ResolveInfo");
		jclass clsActvInfo	= jEnv->FindClass("android/content/pm/ActivityInfo");
		jclass clsAppInfo	= jEnv->FindClass("android/content/pm/ApplicationInfo");
		jclass clsIntent	= jEnv->FindClass("android/content/Intent");
		jclass clsList		= jEnv->FindClass("java/util/List");
		if(clsString == NULL || clsContext == NULL || clsPkgMngr == NULL || clsResInfo == NULL || clsActvInfo == NULL || clsAppInfo == NULL || clsIntent == NULL || clsList == NULL){
			PRINTF_ERROR("AUAppGlueAndroidJNI, getLauncherClassName, NOT all jclass obtained.\n");
		} else {
			//PRINTF_INFO("AUAppGlueAndroidJNI, getLauncherClassName ALL jclass obtained.\n");
			jmethodID mEqIgnoreCase		= jEnv->GetMethodID(clsString, "equalsIgnoreCase", "(Ljava/lang/String;)Z");
			jmethodID mGetPkgName		= jEnv->GetMethodID(clsContext, "getPackageName", "()Ljava/lang/String;");
			jmethodID mGetPkgMngr		= jEnv->GetMethodID(clsContext, "getPackageManager", "()Landroid/content/pm/PackageManager;");
			jmethodID mIntentInit		= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;)V");
			jmethodID mIntAddCat		= jEnv->GetMethodID(clsIntent, "addCategory", "(Ljava/lang/String;)Landroid/content/Intent;");
			jfieldID fIntActMain		= jEnv->GetStaticFieldID(clsIntent, "ACTION_MAIN", "Ljava/lang/String;"); //"android.intent.action.MAIN"
			jfieldID fIntCatLaunch		= jEnv->GetStaticFieldID(clsIntent, "CATEGORY_LAUNCHER", "Ljava/lang/String;"); //"android.intent.category.LAUNCHER"
			jmethodID mPmQueryActs		= jEnv->GetMethodID(clsPkgMngr, "queryIntentActivities", "(Landroid/content/Intent;I)Ljava/util/List;"); //List<ResolveInfo> queryIntentActivities (Intent intent, int flags)
			jmethodID mListSize			= jEnv->GetMethodID(clsList, "size", "()I");
			jmethodID mListGet			= jEnv->GetMethodID(clsList, "get", "(I)Ljava/lang/Object;");
			jfieldID fResActInfo		= jEnv->GetFieldID(clsResInfo, "activityInfo", "Landroid/content/pm/ActivityInfo;");
			jfieldID fActInfoAppInfo	= jEnv->GetFieldID(clsActvInfo, "applicationInfo", "Landroid/content/pm/ApplicationInfo;");
			jfieldID fActInfoName		= jEnv->GetFieldID(clsActvInfo, "name", "Ljava/lang/String;");
			jfieldID fAppInfoPkgName	= jEnv->GetFieldID(clsAppInfo, "packageName", "Ljava/lang/String;");
			if(mGetPkgName == NULL || mGetPkgMngr == NULL || mIntentInit == NULL || mIntAddCat == NULL || fIntActMain == NULL || fIntCatLaunch == NULL || mPmQueryActs == NULL || mListSize == NULL || mListGet == NULL || fResActInfo == NULL || fActInfoAppInfo == NULL || fAppInfoPkgName == NULL){
				PRINTF_ERROR("AUAppGlueAndroidJNI, getLauncherClassName, NOT all methods/fields obtained.\n");
			} else {
				//PRINTF_INFO("AUAppGlueAndroidJNI, getLauncherClassName ALL methods/fields obtained.\n");
				jstring pkgName			= (jstring)jEnv->CallObjectMethod(jContext, mGetPkgName);
				jobject objPM			= jEnv->CallObjectMethod(jContext, mGetPkgMngr);
				jstring actMainStr		= (jstring)jEnv->GetStaticObjectField(clsIntent, fIntActMain);
				jstring catLncherStr	= (jstring)jEnv->GetStaticObjectField(clsIntent, fIntCatLaunch);
				if(pkgName == NULL || objPM == NULL || actMainStr == NULL || catLncherStr == NULL){
					PRINTF_ERROR("AUAppGlueAndroidJNI, getLauncherClassName, getPackageManager, ACTION_MAIN or CATEGORY_LAUNCHER returned NULL.\n");
				} else {
					//PRINTF_INFO("AUAppGlueAndroidJNI, getLauncherClassName, getPackageManager, ACTION_MAIN or CATEGORY_LAUNCHER obtained.\n");
					jobject objIntent = jEnv->NewObject(clsIntent, mIntentInit, actMainStr);
					jEnv->CallObjectMethod(objIntent, mIntAddCat, catLncherStr);
					jobject objListRes = jEnv->CallObjectMethod(objPM, mPmQueryActs, objIntent, 0);
					if(objListRes == NULL){
						PRINTF_ERROR("AUAppGlueAndroidJNI, getLauncherClassName, queryIntentActivities returned NULL.\n");
					} else {
						jint listSz = jEnv->CallIntMethod(objListRes, mListSize);
						//PRINTF_INFO("AUAppGlueAndroidJNI, getLauncherClassName, resolveInfos have %d items.\n", (SI32)listSz);
						jint i = 0;
						for(i = 0; i < listSz; i++){
							jobject resolveInfo = jEnv->CallObjectMethod(objListRes, mListGet, i); NBASSERT(resolveInfo != NULL)
							if(resolveInfo != NULL){
								jobject activityInfo = jEnv->GetObjectField(resolveInfo, fResActInfo); NBASSERT(activityInfo != NULL)
								if(activityInfo != NULL){
									jobject appInfo = jEnv->GetObjectField(activityInfo, fActInfoAppInfo); NBASSERT(appInfo != NULL)
									if(appInfo != NULL){
										jstring name = (jstring) jEnv->GetObjectField(appInfo, fAppInfoPkgName); NBASSERT(name != NULL)
										if(name != NULL){
											if(jEnv->CallBooleanMethod(pkgName, mEqIgnoreCase, name) == JNI_TRUE){
												launchClsName = (jstring) jEnv->GetObjectField(activityInfo, fActInfoName); NBASSERT(launchClsName != NULL)
												//PRINTF_INFO("AUAppGlueAndroidJNI, getLauncherClassName, className found at index %d.\n", (SI32)i);
												break;
											}
											NBJNI_DELETE_REF_LOCAL(jEnv, name)
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, appInfo)
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, activityInfo)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, resolveInfo)
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, objListRes)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, catLncherStr)
					NBJNI_DELETE_REF_LOCAL(jEnv, actMainStr)
					NBJNI_DELETE_REF_LOCAL(jEnv, objPM)
					NBJNI_DELETE_REF_LOCAL(jEnv, pkgName)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsList)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsAppInfo)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsActvInfo)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsResInfo)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPkgMngr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsString)
		}
	}
	return launchClsName;
}

SI32 AUAppGlueAndroidJNI::getResDrawable(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const char* drawableName){
	SI32 r = 0;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jstring pkgName		= (jstring)AUAppGlueAndroidJNI::getPackageName(jEnv, jContext); NBASSERT(pkgName != NULL)
		if(pkgName != NULL){
			//R.drawable.[drawableName]
			const char* strPkgName = jEnv->GetStringUTFChars(pkgName, 0);
			AUCadenaMutable8* clsName = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
			jclass clsDrawable = NULL;
			//Find class (drawable or mipmaps)
			{
				clsName->establecer(strPkgName);
				clsName->reemplazar('.', '/');
				clsName->agregar("/R$drawable");
				clsDrawable = jEnv->FindClass(clsName->str());
				if(clsDrawable == NULL){
					if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
					PRINTF_ERROR("AUAppGlueAndroidJNI, getResDrawableIconLauncher, no '%s' class found.\n", clsName->str());
					clsName->establecer(strPkgName);
					clsName->reemplazar('.', '/');
					clsName->agregar("/R$mipmap");
					clsDrawable = jEnv->FindClass(clsName->str());
					if(clsDrawable == NULL){
						if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
						PRINTF_ERROR("AUAppGlueAndroidJNI, getResDrawableIconLauncher, no '%s' class found.\n", clsName->str());
					}
				}
			}
			//
			if(clsDrawable != NULL) {
				jfieldID fIcLauncher = jEnv->GetStaticFieldID(clsDrawable, drawableName, "I");
				if(fIcLauncher == NULL){
					if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
					PRINTF_ERROR("AUAppGlueAndroidJNI, getResDrawableIconLauncher, no '%s' static member found at '%s'.\n", drawableName, clsName->str());
				} else {
					r = jEnv->GetStaticIntField(clsDrawable, fIcLauncher);
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsDrawable)
			}
			clsName->liberar(NB_RETENEDOR_THIS);
			jEnv->ReleaseStringUTFChars(pkgName, strPkgName);
		}
	}
	return r;
}

SI32 AUAppGlueAndroidJNI::getAPICurrent(void* pEnv /*JNIEnv*/){
	SI32 r = 0;
	if(pEnv != NULL){
		JNIEnv* jEnv = (JNIEnv*) pEnv;
		jclass clsVERSION	= jEnv->FindClass("android/os/Build$VERSION"); NBASSERT(clsVERSION != NULL)
		if(clsVERSION != NULL){
			jfieldID fSdkInt = jEnv->GetStaticFieldID(clsVERSION, "SDK_INT", "I"); NBASSERT(fSdkInt != NULL)
			if(fSdkInt != NULL){
				r = jEnv->GetStaticIntField(clsVERSION, fSdkInt);
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsVERSION)
		}
	}
	return r;
}

SI32 AUAppGlueAndroidJNI::getAPIByName(void* pEnv /*JNIEnv*/, const char* name){
	SI32 r = (10000 - 1); // CUR_DEVELOPMENT - 1
	//https://developer.android.com/reference/android/os/Build.VERSION_CODES.html
	if(AUCadena8::esIgual(name, "BASE")){
		r = 0x00000001; //1
	} else if(AUCadena8::esIgual(name, "BASE_1_1")){
		r = 0x00000002; //2, 1.1
	} else if(AUCadena8::esIgual(name, "CUPCAKE")){
		r = 0x00000003; //3, 1.5
	} else if(AUCadena8::esIgual(name, "CUR_DEVELOPMENT")){
		r = 0x00002710; //10000
	} else if(AUCadena8::esIgual(name, "DONUT")){
		r = 0x00000004; //4, 1.6
	} else if(AUCadena8::esIgual(name, "ECLAIR")){
		r = 0x00000005; //5, 2.0
	} else if(AUCadena8::esIgual(name, "ECLAIR_0_1")){
		r = 0x00000006; //6, 2.01
	} else if(AUCadena8::esIgual(name, "ECLAIR_MR1")){
		r = 0x00000007; //7, 2.1
	} else if(AUCadena8::esIgual(name, "FROYO")){
		r = 0x00000008; //8, 2.2
	} else if(AUCadena8::esIgual(name, "GINGERBREAD")){
		r = 0x00000009; //9, 2.3
	} else if(AUCadena8::esIgual(name, "GINGERBREAD_MR1")){
		r = 0x0000000a; //10, 2.3.3
	} else if(AUCadena8::esIgual(name, "HONEYCOMB")){
		r = 0x0000000b; //11, 3.0
	} else if(AUCadena8::esIgual(name, "HONEYCOMB_MR1")){
		r = 0x0000000c; //12, 3.1
	} else if(AUCadena8::esIgual(name, "HONEYCOMB_MR2")){
		r = 0x0000000d; //13, 3.2
	} else if(AUCadena8::esIgual(name, "ICE_CREAM_SANDWICH")){
		r = 0x0000000e; //14, 4.0
	} else if(AUCadena8::esIgual(name, "ICE_CREAM_SANDWICH_MR1")){
		r = 0x0000000f; //15, 4.0.3
	} else if(AUCadena8::esIgual(name, "JELLY_BEAN")){
		r = 0x00000010; //16, 4.1
	} else if(AUCadena8::esIgual(name, "JELLY_BEAN_MR1")){
		r = 0x00000011; //17, 4.2
	} else if(AUCadena8::esIgual(name, "JELLY_BEAN_MR2")){
		r = 0x00000012; //18, 4.3
	} else if(AUCadena8::esIgual(name, "KITKAT")){
		r = 0x00000013; //19, 4.4
	} else if(AUCadena8::esIgual(name, "KITKAT_WATCH")){
		r = 0x00000014; //20, 4.4W
	} else if(AUCadena8::esIgual(name, "LOLLIPOP")){
		r = 0x00000015; //21, Lollipop
	} else if(AUCadena8::esIgual(name, "LOLLIPOP_MR1")){
		r = 0x00000016; //22, Lollipop
	} else if(AUCadena8::esIgual(name, "M")){
		r = 0x00000017; //23, Marshmallow
	} else if(AUCadena8::esIgual(name, "N")){
		r = 0x00000018; //24, Nougat
	} else if(AUCadena8::esIgual(name, "N_MR1")){
		r = 0x00000019; //25, Nougat++
	} else if(AUCadena8::esIgual(name, "O")){
		r = 0x0000001a; //26
    } else if(AUCadena8::esIgual(name, "O_MR1")){
        r = 0x0000001b; //27
    } else if(AUCadena8::esIgual(name, "P")){
        r = 0x0000001c; //28
    } else if(AUCadena8::esIgual(name, "Q")){
        r = 0x0000001d; //29
    } else if(AUCadena8::esIgual(name, "R")){
        r = 0x0000001e; //30
    } else if(AUCadena8::esIgual(name, "S")){
        r = 0x0000001f; //31
    } else if(AUCadena8::esIgual(name, "S_V2")){
        r = 0x00000020; //32
    } else if(AUCadena8::esIgual(name, "TIRAMISU")){
        r = 0x00000021; //33
	} else {
		if(pEnv != NULL){
			JNIEnv* jEnv = (JNIEnv*) pEnv;
			jclass clsVERCODES	= jEnv->FindClass("android/os/Build$VERSION_CODES"); NBASSERT(clsVERCODES != NULL)
			if(clsVERCODES != NULL){
				jfieldID fver	= jEnv->GetStaticFieldID(clsVERCODES, name, "I");
				if(fver == NULL){
					PRINTF_ERROR("AUAppGlueAndroidJNI, unknown BuildVersionCode for '%s' at runtime.\n", name);
					if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
				} else {
					r = jEnv->GetStaticIntField(clsVERCODES, fver);
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsVERCODES)
			}
		}
	}
	return r;
}

void* AUAppGlueAndroidJNI::getSystemServiceByConstantName(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const char* constantName){
	jobject r = NULL;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
		if(clsContext != NULL){
			jfieldID fSrvcName		= jEnv->GetStaticFieldID(clsContext, constantName, "Ljava/lang/String;"); NBASSERT(fSrvcName != NULL)
			jmethodID mGetSysSrvc	= jEnv->GetMethodID(clsContext, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"); NBASSERT(mGetSysSrvc != NULL)
			if(fSrvcName != NULL && mGetSysSrvc != NULL){
				jstring jSrvcName	= (jstring)jEnv->GetStaticObjectField(clsContext, fSrvcName); NBASSERT(jSrvcName != NULL)
				r = jEnv->CallObjectMethod(jContext, mGetSysSrvc, jSrvcName);
				if(r == NULL){
					PRINTF_ERROR("AUAppGlueAndroidJNI, service '%s' not found", constantName);
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, jSrvcName)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
		}
	}
	return r;
}


void* AUAppGlueAndroidJNI::getRingtoneUri(void* pEnv /*JNIEnv*/, const char* type){ //"TYPE_NOTIFICATION"
	jobject r = NULL;
	if(pEnv != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jclass clsRingMngrt	= jEnv->FindClass("android/media/RingtoneManager"); NBASSERT(clsRingMngrt != NULL)
		if(clsRingMngrt != NULL){
			jfieldID fType	= jEnv->GetStaticFieldID(clsRingMngrt, type, "I");
			if(fType == NULL){
				if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
				PRINTF_INFO("AUAppGlueAndroidJNI, getRingtoneUri NOT found for('%s').\n", type);
			} else {
				jmethodID mGetDefUri = jEnv->GetStaticMethodID(clsRingMngrt, "getDefaultUri", "(I)Landroid/net/Uri;"); NBASSERT(mGetDefUri != NULL)
				if(mGetDefUri != NULL){
					r = jEnv->CallStaticObjectMethod(clsRingMngrt, mGetDefUri, jEnv->GetStaticIntField(clsRingMngrt, fType));
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsRingMngrt)
		}
	}
	return r;
}

