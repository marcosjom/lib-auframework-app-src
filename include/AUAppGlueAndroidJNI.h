//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAndroidJNI_H
#define AUAppGlueAndroidJNI_H

#include "AUAppNucleoEncabezado.h" // for AUCadenaMutable8
#include <pthread.h>

#define NBJNI_DELETE_REF_LOCAL(JENV, JVAR) if((JVAR) != NULL) (JENV)->DeleteLocalRef((JVAR)); (JVAR) = NULL;

//

typedef void (*AURunnableRunMthd)(void* param);

//

typedef struct STGlueAndroidJNIThreadEnv_ {
	void*		jEnv; //JNIEnv
	char*		refName;
	//
	void*		jObj;		//jobject, for instance calls
	void*		jCls;		//jclass, for static calls
} STGlueAndroidJNIThreadEnv;

//

/*typedef struct STGlueAndroidJNIEnvVars_ {
	const char* refName;
	void*	jObj;		//jobject, for instance calls
	void*	jCls;		//jclass, for static calls
} STGlueAndroidJNIEnvVars;*/

/*typedef struct STGlueAndroidJNIEnv_ {
	void*						jEnv; //JNIEnv
	//Stack of vars
	STGlueAndroidJNIEnvVars**	stackVarsArr;
	SI32						stackVarsUse;
	SI32						stackVarsSize;
} STGlueAndroidJNIEnv;*/

class AUAppGlueAndroidJNI {
	public:
		//AUAppGlueAndroidJNI(void* jvm /*JavaVM*/);
		AUAppGlueAndroidJNI(void* jEnv /*JNIEnv*/, void* jActivity /*jobject*/);
		virtual ~AUAppGlueAndroidJNI();
		//
		STNBAndroidJniItf getAndroidJniItf();
		//
		SI32	jniVersion() const;
		void* /*jobject*/ jActivity() const;
		void* /*jobject*/ jHandler() const;
		//
		//void* /*JNIEnv*/ attachCurrentThread(const char* refName);
		//void* /*JNIEnv*/ attachCurrentThreadWithObj(void* jObj /*jobject*/, const char* refName); //for instance calls
		//void* /*JNIEnv*/ attachCurrentThreadWithCls(void* jCls /*jclass*/, const char* refName); //for static calls
		//bool	detachCurrentThread(void* jEnv /*JNIEnv*/);
		//
		SI32	envVarsPush(void* jEnv /*JNIEnv*/, const char* refName);
		SI32	envVarsPushWithObj(void* jEnv /*JNIEnv*/, void* jObj /*jobject*/, const char* refName);
		SI32	envVarsPushWithCls(void* jEnv /*JNIEnv*/, void* jCls /*jclass*/, const char* refName);
		SI32	envVarsPop(void* jEnv /*JNIEnv*/);
		void* /*JNIEnv*/ curEnvCalculated(); //queried to the JVM
		void* /*JNIEnv*/ curEnv();
		//bool	curEnvVars(STGlueAndroidJNIEnvVars* dst, void** dstEnv /*JNIEnv*/);
		//Runnable
		BOOL	addRunableForMainThread(void* jAppNative /*jobject*/, AURunnableRunMthd func, void* funcParam);
		//--------------
		//Static methods (for C callers)
		//--------------
		static SI32	jniVersion_(void* obj);
		static void* /*jobject*/ jActivity_(void* obj);
		static void* /*JNIEnv*/ curEnv_(void* obj);
		//--------------
		// Android permissions (API 23+ requires explicit permission from user)
		//--------------
		static BOOL					isPermissionGranted(void* jEnv /*JNIEnv*/, void* /*jobject*/ context, const char* permFullName);
		static BOOL					requestPermissions(void* jEnv /*JNIEnv*/, void* /*jobject*/ activity, const char** perms, const SI32 permsSz);
		//--------------
		// Android utils
		//--------------
		static void*				getNewInstanceOfClass(void* jEnv /*JNIEnv*/, void* jClassObj /*jobject*/);
		static void* /*jstring*/	getPackageName(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/);
		static void* /*jstring*/	getLauncherClassName(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/);
		static SI32					getAPICurrent(void* jEnv /*JNIEnv*/);
		static SI32					getAPIByName(void* jEnv /*JNIEnv*/, const char* name);
		static SI32					getResDrawable(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, const char* drawableName);
		static void*				getSystemServiceByConstantName(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, const char* constantName);
		static void*				getRingtoneUri(void* jEnv /*JNIEnv*/, const char* type);
		static bool					saveDataToSharedPrefs(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const char* prefName, const char* dataName, const char* data);
		static bool					loadDataFromSharedPrefs(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const char* prefName, const char* dataName, AUCadenaMutable8* dst);
	private:
		SI32					_jniVer; /*jint*/
		void*					_jvm; /*JavaVM*/
		void*					_jActivity; /*jobject*/
		void*					_jHandler; //Shared handler for thread workload (like camera activity)
		//JNIEnvs
		//pthread_mutex_t		_envsMutex;
		//STGlueAndroidJNIEnv*	_envsArr;
		//SI32					_envsUse;
		//SI32					_envsSize;
		void* /*JNIEnv*/		privAttachCurrentThread(void* jObj /*jobject*/, void* jCls /*jclass*/, const char* refName);
		SI32					privEnvVarsPush(void* jEnv /*JNIEnv*/, void* jObj /*jobject*/, void* jCls /*jclass*/, const char* refName);
};

#endif
