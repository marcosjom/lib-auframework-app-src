//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueAndroidNotifs.h"
//Android and JNI headers
#include <jni.h>

//-------------------------
// AUAppGlueAndroidNotifs
//-------------------------

#ifdef __ANDROID__
	//is android
#endif

class AUAppGlueAndroidNotifsAppListener;

typedef struct AUAppGlueAndroidNotifsData_ {
	AUAppI* app;
	AUAppGlueAndroidNotifsAppListener* listener;
	//Check
	struct {
		BOOL clsNotFoundPrintedFirebase;	//message of class not found
	} jniCheck;
	//Remote notifs
	struct {
		jobject		jTskTkn;	//current request taskId for token
		STNBString	idd;		//
		STNBString	token;		//
	} remote;
} AUAppGlueAndroidNotifsData;

class AUAppGlueAndroidNotifsAppListener: public AUAppIntentListener, public AUAppActResultListener {
	public:
		AUAppGlueAndroidNotifsAppListener(AUAppGlueAndroidNotifsData* data){
			_data = data;
		}
		virtual ~AUAppGlueAndroidNotifsAppListener(){
			_data = NULL;
		}
		//AUAppIntentListener
		void appIntentReceived(AUAppI* app, void* intent /*jobject::Intent*/){
			AUAppGlueAndroidNotifs::analyzeIntent(_data, intent);
		}
		//AUAppActResultListener
		void appActResultReceived(AUAppI* app, SI32 request, SI32 response, void* intent /*jobject::Intent*/){
			AUAppGlueAndroidNotifs::analyzeIntent(_data, intent);
		}
	private:
		AUAppGlueAndroidNotifsData* _data;
};

//

bool AUAppGlueAndroidNotifs::analyzeIntent(void* pData, void* pIntent /*jobject*/){
	bool r = false;
	if(pData != NULL && pIntent != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				if(pIntent != NULL){
					jobject jIntent		= (jobject) pIntent;
					jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
					jclass clsBundle	= jEnv->FindClass("android/os/Bundle"); NBASSERT(clsBundle != NULL)
					if(clsIntent != NULL && clsBundle != NULL){
						jmethodID mGetExtras	= jEnv->GetMethodID(clsIntent, "getExtras", "()Landroid/os/Bundle;"); NBASSERT(mGetExtras != NULL)
						jmethodID mGetInt		= jEnv->GetMethodID(clsBundle, "getInt", "(Ljava/lang/String;)I"); NBASSERT(mGetInt != NULL)
						jmethodID mGetString	= jEnv->GetMethodID(clsBundle, "getString", "(Ljava/lang/String;)Ljava/lang/String;"); NBASSERT(mGetString != NULL)
						if(mGetExtras != NULL && mGetInt != NULL && mGetString != NULL){
							jobject bundle = jEnv->CallObjectMethod(jIntent, mGetExtras);
							if(bundle == NULL){
								PRINTF_INFO("AUAppGlueAndroidNotifs, analyzeIntent, NOT a nbType intent (no bundle).\n");
							} else {
								jstring jStrTitType	= jEnv->NewStringUTF("nbType");
								jstring nbType = (jstring)jEnv->CallObjectMethod(bundle, mGetString, jStrTitType);
								if(nbType == NULL){
									PRINTF_INFO("AUAppGlueAndroidNotifs, analyzeIntent, NOT a nbType intent (no 'nbType' extra).\n");
								} else {
									const char*  strNbType = jEnv->GetStringUTFChars(nbType, 0);
									PRINTF_INFO("AUAppGlueAndroidNotifs, analyzeIntent, is a nbType '%s' intent.\n", strNbType);
									const bool isAlarm		= AUCadena8::cadenasSonIguales(strNbType, "localNotifAlarm");
									const bool isAcepted	= AUCadena8::cadenasSonIguales(strNbType, "localNotifAcepted");
									const bool isRejected	= AUCadena8::cadenasSonIguales(strNbType, "localNotifRejected");
									if(isAlarm || isAcepted || isRejected){
										jstring jStrTitUId	= jEnv->NewStringUTF("uid");
										jstring jStrTitGId	= jEnv->NewStringUTF("grpId");
										jstring jStrTitNId	= jEnv->NewStringUTF("notifId");
										jstring jStrTitTit	= jEnv->NewStringUTF("title");
										jstring jStrTitCnt	= jEnv->NewStringUTF("content");
										jstring jStrTitDat	= jEnv->NewStringUTF("data");
										jint uid			= jEnv->CallIntMethod(bundle, mGetInt, jStrTitUId);
										jstring grpId		= (jstring)jEnv->CallObjectMethod(bundle, mGetString, jStrTitGId);
										jint notifId		= jEnv->CallIntMethod(bundle, mGetInt, jStrTitNId);
										jstring title		= (jstring)jEnv->CallObjectMethod(bundle, mGetString, jStrTitTit);
										jstring content		= (jstring)jEnv->CallObjectMethod(bundle, mGetString, jStrTitCnt);
										jstring data		= (jstring)jEnv->CallObjectMethod(bundle, mGetString, jStrTitDat);
										//
										const char* strGrpId = NULL;	if(grpId != NULL) strGrpId = jEnv->GetStringUTFChars(grpId, 0);
										const char* strTitle = NULL;	if(title != NULL) strTitle = jEnv->GetStringUTFChars(title, 0);
										const char* strContent = NULL;	if(content != NULL) strContent = jEnv->GetStringUTFChars(content, 0);
										const char* strData = NULL;		if(data != NULL) strData = jEnv->GetStringUTFChars(data, 0);
										//
										if(isAlarm){
											//Show local notification (ToDo: do not show if the app is has focus)
											if(AUAppGlueAndroidNotifs::notifShow(jEnv, jContext, uid, strGrpId, notifId, strTitle, strContent, strData)){
												NBMngrNotifs::addNotifRcvd(ENNotifType_Local, uid, strGrpId, notifId, strData);
												r = true;
											}
										} else if(isAcepted || isRejected){
											if(isAcepted){
												NBMngrNotifs::setLaunchNotif(ENNotifType_Local, uid, strGrpId, notifId, strData);
											} else {
												NBASSERT(isRejected)
												//NBMngrNotifs::removeFromQueue(ENNotifType_Local, uid);
											}
											r = true;
										}
										//
										if(strData != NULL){ jEnv->ReleaseStringUTFChars(data, strData); NBJNI_DELETE_REF_LOCAL(jEnv, data) }
										if(strContent != NULL){ jEnv->ReleaseStringUTFChars(content, strContent); NBJNI_DELETE_REF_LOCAL(jEnv, content) }
										if(strTitle != NULL){ jEnv->ReleaseStringUTFChars(title, strTitle); NBJNI_DELETE_REF_LOCAL(jEnv, title) }
										if(strGrpId != NULL){ jEnv->ReleaseStringUTFChars(grpId, strGrpId); NBJNI_DELETE_REF_LOCAL(jEnv, grpId) }
										//
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitDat)
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitCnt)
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitTit)
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitNId)
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitGId)
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitUId)
									}
									jEnv->ReleaseStringUTFChars(nbType, strNbType);
									NBJNI_DELETE_REF_LOCAL(jEnv, nbType)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrTitType)
								NBJNI_DELETE_REF_LOCAL(jEnv, bundle)
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsBundle)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
					}
				}
			}
		}
	}
	PRINTF_INFO("AUAppGlueAndroidNotifs::analyzeIntent, consumed(%s).\n", r ? "YES" : "NO");
	return r;
}


//JAVA CODE
/*{
	String launcherClassName = getLauncherClassName(context);
	//Samsung
	{
		Intent intent = new Intent("android.intent.action.BADGE_COUNT_UPDATE");
		intent.putExtra("badge_count", count);
		intent.putExtra("badge_count_package_name", context.getPackageName());
		intent.putExtra("badge_count_class_name", launcherClassName);
		context.sendBroadcast(intent);
	}
	//Sony
	{
		Intent intent = new Intent();
		intent.setAction("com.sonyericsson.home.action.UPDATE_BADGE");
		intent.putExtra("com.sonyericsson.home.intent.extra.badge.ACTIVITY_NAME", launcherClassName);
		intent.putExtra("com.sonyericsson.home.intent.extra.badge.SHOW_MESSAGE", true);
		intent.putExtra("com.sonyericsson.home.intent.extra.badge.MESSAGE", String.valueOf(count));
		intent.putExtra("com.sonyericsson.home.intent.extra.badge.PACKAGE_NAME", context.getPackageName());
		context.sendBroadcast(intent);
	}
}*/

bool AUAppGlueAndroidNotifs::setAppBadgeNumber(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const SI32 number){
	bool r = false;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		//
		jstring launcherClassName = (jstring)AUAppGlueAndroidJNI::getLauncherClassName(jEnv, jContext);
		if(launcherClassName == NULL){
			PRINTF_ERROR("AUAppGlueAndroidNotifs, setAppBadgeNumber could not obtain launcherClassName.\n");
		} else {
			jclass clsIntent	= jEnv->FindClass("android/content/Intent");
			jclass clsContext	= jEnv->FindClass("android/content/Context");
			if(clsIntent == NULL || clsContext == NULL){
				PRINTF_ERROR("AUAppGlueAndroidNotifs, setAppBadgeNumber NOT all jclass obtained.\n");
			} else {
				//PRINTF_INFO("AUAppGlueAndroidNotifs, setAppBadgeNumber ALL jclass obtained.\n");
				jmethodID mCtxtPkgName	= jEnv->GetMethodID(clsContext, "getPackageName", "()Ljava/lang/String;");
				jmethodID mCtxtSendBrd	= jEnv->GetMethodID(clsContext, "sendBroadcast", "(Landroid/content/Intent;)V");
				jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "()V");
				jmethodID mIntentInit2	= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;)V");
				jmethodID mIntSetAction	= jEnv->GetMethodID(clsIntent, "setAction", "(Ljava/lang/String;)Landroid/content/Intent;");
				jmethodID mIntPutStr	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;");
				jmethodID mIntPutBool	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;Z)Landroid/content/Intent;");
				jmethodID mIntPutInt	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;I)Landroid/content/Intent;");
				if(mCtxtPkgName == NULL || mCtxtSendBrd == NULL || mIntentInit == NULL || mIntentInit2 == NULL || mIntSetAction == NULL || mIntPutStr == NULL || mIntPutBool == NULL || mIntPutInt == NULL){
					PRINTF_ERROR("AUAppGlueAndroidNotifs, setAppBadgeNumber NOT all methods obtained.\n");
				} else {
					//PRINTF_INFO("AUAppGlueAndroidNotifs, setAppBadgeNumber ALL methods obtained.\n");
					//Samsung
					{
						jstring jStrUpdt	= jEnv->NewStringUTF("android.intent.action.BADGE_COUNT_UPDATE");
						jstring jStrCount	= jEnv->NewStringUTF("badge_count");
						jstring jStrPkg		= jEnv->NewStringUTF("badge_count_package_name");
						jstring jStrCls		= jEnv->NewStringUTF("badge_count_class_name");
						jstring jStrPkgName	= (jstring)jEnv->CallObjectMethod(jContext, mCtxtPkgName);
						//
						jobject intent = jEnv->NewObject(clsIntent, mIntentInit2, jStrUpdt);
						jEnv->CallObjectMethod(intent, mIntPutInt, jStrCount, number);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrPkg, jStrPkgName);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrCls, launcherClassName);
						jEnv->CallVoidMethod(jContext, mCtxtSendBrd, intent);
						NBJNI_DELETE_REF_LOCAL(jEnv, intent)
						//
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrPkgName)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrCls)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrPkg)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrCount)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrUpdt)
					}
					//Sony
					{
						AUCadenaMutable8* strInt = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
						strInt->agregarNumerico(number);
						jstring jStrAction	= jEnv->NewStringUTF("com.sonyericsson.home.action.UPDATE_BADGE");
						jstring jStrExtAct	= jEnv->NewStringUTF("com.sonyericsson.home.intent.extra.badge.ACTIVITY_NAME");
						jstring jStrExtShow = jEnv->NewStringUTF("com.sonyericsson.home.intent.extra.badge.SHOW_MESSAGE");
						jstring jStrExtMsg	= jEnv->NewStringUTF("com.sonyericsson.home.intent.extra.badge.MESSAGE");
						jstring jStrExtPkg	= jEnv->NewStringUTF("com.sonyericsson.home.intent.extra.badge.PACKAGE_NAME");
						jstring jStrNumber	= jEnv->NewStringUTF(strInt->str());
						jstring jStrPkgName	= (jstring)jEnv->CallObjectMethod(jContext, mCtxtPkgName);
						jobject intent		= jEnv->NewObject(clsIntent, mIntentInit);
						jEnv->CallObjectMethod(intent, mIntSetAction, jStrAction);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrExtAct, launcherClassName);
						jEnv->CallObjectMethod(intent, mIntPutBool, jStrExtShow, (number == 0 ? JNI_TRUE : JNI_FALSE));
						jEnv->CallObjectMethod(intent, mIntPutInt, jStrExtMsg, jStrNumber);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrExtPkg, jStrPkgName);
						jEnv->CallVoidMethod(jContext, mCtxtSendBrd, intent);
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrPkgName)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNumber)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrExtPkg)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrExtMsg)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrExtShow)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrExtAct)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrAction)
						strInt->liberar(NB_RETENEDOR_THIS);
					}
					PRINTF_INFO("AUAppGlueAndroidNotifs, setAppBadgeNumber set to %d (intent).\n", number);
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
				NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
			}
		}
	}
	return r;
}


/*public boolean programarNotificacion(Context contexto, String grupo, int localId, String titulo, String contenido, String objTipo, int objId, int segsDesdeAhora) {
	boolean r = false;
	//Programar alarma
	AlarmManager alarmManager = (AlarmManager) contexto.getSystemService(Context.ALARM_SERVICE);
	if(alarmManager != null){
		try {
			//Agregar nueva
			Notificacion obj = new Notificacion();
			obj.uniqueId = ++_uidSecuencial;
			obj.grupoId = grupo;
			obj.localId = localId;
			obj.titulo	= titulo;
			obj.contenido = contenido;
			obj.objTipo	= objTipo;
			obj.objId	= objId;
			//
			Intent notifIntent = new Intent("android.media.action.DISPLAY_NOTIFICATION");
			notifIntent.putExtra("au_uid", obj.uniqueId);
			notifIntent.putExtra("grp", obj.grupoId);
			notifIntent.putExtra("lid", obj.localId);
			notifIntent.putExtra("tit", obj.titulo);
			notifIntent.putExtra("cnt", obj.contenido);
			notifIntent.putExtra("objTip", obj.objTipo);
			notifIntent.putExtra("objId", obj.objId);
			notifIntent.addCategory("android.intent.category.DEFAULT");
			PendingIntent notifPendingIntent = PendingIntent.getBroadcast(contexto, obj.uniqueId, notifIntent, PendingIntent.FLAG_UPDATE_CURRENT);
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT && false) {
				// only for kitkat and newer versions
				//Note: only alarms for which there is a strong demand for exact-time delivery
				//(such as an alarm clock ringing at the requested time) should be scheduled as exact.
				//Applications are strongly discouraged from using exact alarms unnecessarily as
				//they reduce the OS's ability to minimize battery use.
				alarmManager.setExact(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + (segsDesdeAhora * 1000), notifPendingIntent);
			} else {
				alarmManager.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + (segsDesdeAhora * 1000), notifPendingIntent);
			}
			obj.existeAlarma = true;
			_lista.add(obj);
			r = true;
			Log.w("AU", "Notificaciones, NOTIFICACION agregada dentro de " + segsDesdeAhora + "segs: '" + titulo + "', '" + contenido + "'");
		} catch (Exception e) {
			Log.e("AU", "Notificaciones, AlarmManager set exception. " + e.toString());
		}
	}
	return r;
}*/

bool AUAppGlueAndroidNotifs::notifSchedule(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const SI32 uid, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* data){
	bool r = false;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jclass clsSystem	= jEnv->FindClass("java/lang/System"); NBASSERT(clsSystem != NULL)
		jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
		jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
		jclass clsAlarmMngr	= jEnv->FindClass("android/app/AlarmManager"); NBASSERT(clsAlarmMngr != NULL)
		jclass clsPendInt	= jEnv->FindClass("android/app/PendingIntent"); NBASSERT(clsPendInt != NULL)
		if(clsIntent == NULL || clsContext == NULL || clsAlarmMngr == NULL || clsPendInt == NULL || clsSystem == NULL){
			PRINTF_ERROR("AUAppGlueAndroidNotifs, notifSchedule NOT all jclass obtained.\n");
		} else {
			//PRINTF_INFO("AUAppGlueAndroidNotifs, notifSchedule ALL classes obtained.\n");
			jmethodID mGetSysSrvc	= jEnv->GetMethodID(clsContext, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"); NBASSERT(mGetSysSrvc != NULL)
			jfieldID fAlarmSrvc		= jEnv->GetStaticFieldID(clsContext, "ALARM_SERVICE", "Ljava/lang/String;"); NBASSERT(fAlarmSrvc != NULL)
			jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mIntentInit != NULL)
			jmethodID mIntPutStr	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mIntPutStr != NULL)
			jmethodID mIntPutInt	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;I)Landroid/content/Intent;"); NBASSERT(mIntPutInt != NULL)
			jmethodID mIntAddCat	= jEnv->GetMethodID(clsIntent, "addCategory", "(Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mIntAddCat != NULL)
			jmethodID mGetBroadcst	= jEnv->GetStaticMethodID(clsPendInt, "getBroadcast", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;"); NBASSERT(mGetBroadcst != NULL)
			jfieldID fUpdateCurr	= jEnv->GetStaticFieldID(clsPendInt, "FLAG_UPDATE_CURRENT", "I"); NBASSERT(fUpdateCurr != NULL)
			jfieldID fRTC_WAKEUP	= jEnv->GetStaticFieldID(clsAlarmMngr, "RTC_WAKEUP", "I"); NBASSERT(fRTC_WAKEUP != NULL)
			jmethodID mGetCurTimeMs	= jEnv->GetStaticMethodID(clsSystem, "currentTimeMillis", "()J"); NBASSERT(mGetCurTimeMs != NULL)
			if(mGetSysSrvc == NULL || fAlarmSrvc == NULL || mIntentInit == NULL || mIntPutStr == NULL || mIntPutInt == NULL || mIntAddCat == NULL || mGetBroadcst == NULL || fUpdateCurr == NULL || fRTC_WAKEUP == NULL || mGetCurTimeMs == NULL){
				PRINTF_ERROR("AUAppGlueAndroidNotifs, notifSchedule NOT all methods obtained.\n");
			} else {
				//PRINTF_INFO("AUAppGlueAndroidNotifs, notifSchedule ALL methods obtained.\n");
				//KITKAT = 19 (0x00000013)
				jstring alarmSrvc	= (jstring)jEnv->GetStaticObjectField(clsContext, fAlarmSrvc); NBASSERT(alarmSrvc != NULL)
				jobject alarmMngr	= jEnv->CallObjectMethod(jContext, mGetSysSrvc, alarmSrvc); NBASSERT(alarmMngr != NULL)
				jint flagUpdateCur	= jEnv->GetStaticIntField(clsPendInt, fUpdateCurr); NBASSERT(flagUpdateCur == 0x08000000) //FLAG_UPDATE_CURRENT = 134217728 (0x08000000)
				jint rtcWakeup		= jEnv->GetStaticIntField(clsAlarmMngr, fRTC_WAKEUP); NBASSERT(rtcWakeup == 0x00000000) //RTC_WAKEUP = 0 (0x00000000)
				const SI32 apiCur	= AUAppGlueAndroidJNI::getAPICurrent(pEnv); NBASSERT(apiCur > 0)
				const SI32 kitkat	= AUAppGlueAndroidJNI::getAPIByName(pEnv, "KITKAT"); NBASSERT(kitkat > 0)
				if(alarmSrvc != NULL && alarmMngr != NULL && apiCur > 0 && kitkat > 0){
					jstring jStrName = jEnv->NewStringUTF("android.media.action.DISPLAY_NOTIFICATION");
					jobject intent = jEnv->NewObject(clsIntent, mIntentInit, jStrName); NBASSERT(intent != NULL)
					if(intent != NULL){
						jstring jStrCat			= jEnv->NewStringUTF("android.intent.category.DEFAULT");
						jstring jStrNameType	= jEnv->NewStringUTF("nbType");
						jstring jStrNameUId		= jEnv->NewStringUTF("uid");
						jstring jStrNameGId		= jEnv->NewStringUTF("grpId");
						jstring jStrNameNId		= jEnv->NewStringUTF("notifId");
						jstring jStrNameTit		= jEnv->NewStringUTF("title");
						jstring jStrNameCont	= jEnv->NewStringUTF("content");
						jstring jStrNameData	= jEnv->NewStringUTF("data");
						//
						jstring jStrType		= jEnv->NewStringUTF("localNotifAlarm");
						jstring jStrGId			= jEnv->NewStringUTF(grpId);
						jstring jStrTit			= jEnv->NewStringUTF(title);
						jstring jStrCont		= jEnv->NewStringUTF(content);
						jstring jStrData		= jEnv->NewStringUTF(data);
						//
						jEnv->CallObjectMethod(intent, mIntAddCat, jStrCat);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrNameType, jStrType);
						jEnv->CallObjectMethod(intent, mIntPutInt, jStrNameUId, (jint)uid);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrNameGId, jStrGId);
						jEnv->CallObjectMethod(intent, mIntPutInt, jStrNameNId, (jint)notifId);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrNameTit, jStrTit);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrNameCont, jStrCont);
						jEnv->CallObjectMethod(intent, mIntPutStr, jStrNameData, jStrData);
						jobject pendIntent = jEnv->CallStaticObjectMethod(clsPendInt, mGetBroadcst, jContext, uid /*unique id*/, intent, flagUpdateCur); NBASSERT(pendIntent != NULL)
						if(pendIntent != NULL){
							jlong curTimeMs = jEnv->CallStaticLongMethod(clsSystem, mGetCurTimeMs);
							if(apiCur >= kitkat && false){
								//Note: only alarms for which there is a strong demand for exact-time delivery
								//(such as an alarm clock ringing at the requested time) should be scheduled as exact.
								//Applications are strongly discouraged from using exact alarms unnecessarily as
								//they reduce the OS's ability to minimize battery use.
								jmethodID mSetAlarm	= jEnv->GetMethodID(clsAlarmMngr, "setExact", "(IJLandroid/app/PendingIntent;)V"); NBASSERT(mSetAlarm != NULL)
								if(mSetAlarm != NULL){
									jEnv->CallVoidMethod(alarmMngr, mSetAlarm, rtcWakeup, curTimeMs + (secsFromNow * 1000), pendIntent);
									PRINTF_INFO("AUAppGlueAndroidNotifs, alarm id %d type %d programed using KITKAT+ API (%d) API (intent): curTimeMs %lld + %d secs.\n", uid, rtcWakeup, apiCur, (SI64)curTimeMs, secsFromNow);
									r = true;
								}
							} else {
								jmethodID mSetAlarm	= jEnv->GetMethodID(clsAlarmMngr, "set", "(IJLandroid/app/PendingIntent;)V"); NBASSERT(mSetAlarm != NULL)
								if(mSetAlarm != NULL){
									jEnv->CallVoidMethod(alarmMngr, mSetAlarm, rtcWakeup, curTimeMs + (secsFromNow * 1000), pendIntent);
									PRINTF_INFO("AUAppGlueAndroidNotifs, alarm id %d type %d programed using pre-KITKAT API (%d) API (intent): curTimeMs %lld + %d secs.\n", uid, rtcWakeup, apiCur, (SI64)curTimeMs, secsFromNow);
									r = true;
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, pendIntent)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrData)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrCont)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrTit)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrGId)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrType)
						//
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameData)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameCont)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameTit)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameNId)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameGId)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameUId)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameType)
						NBJNI_DELETE_REF_LOCAL(jEnv, jStrCat)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
					NBJNI_DELETE_REF_LOCAL(jEnv, alarmMngr)
					NBJNI_DELETE_REF_LOCAL(jEnv, alarmSrvc)
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPendInt)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsAlarmMngr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsSystem)
		}
	}
	return r;
}

/*public void mostrarNotificacion(Context contexto, Notificacion obj){
	if(contexto != null && obj != null){
		NotificationManager notifyMgr = (NotificationManager) contexto.getSystemService(MainActivity.NOTIFICATION_SERVICE);
		if(notifyMgr == null){
			Log.e("AU", "Notificaciones, alarma recibida, pero NotificationManager es null.");
		} else {
			NotificationCompat.Builder mBuilder = new NotificationCompat.Builder(contexto);
			mBuilder.setSmallIcon(R.drawable.ic_launcher);
			if(obj.titulo.length() != 0) mBuilder.setContentTitle(obj.titulo);
			if(obj.contenido.length() != 0) mBuilder.setContentText(obj.contenido);
			mBuilder.setAutoCancel(true);
			//mBuilder.setWhen(System.currentTimeMillis());
			//Api level 11 (Android 3.0)
			/ *Notification noti = new Notification.Builder(mContext)
			.setContentTitle("New mail from " + sender.toString())
			.setContentText(subject)
			.setSmallIcon(R.drawable.new_mail)
			.setLargeIcon(aBitmap)
			.build();* /
			//
			Intent resultIntent = new Intent(contexto, MainActivity.class);
			resultIntent.putExtra("au_uid", obj.uniqueId);
			resultIntent.putExtra("grp", obj.grupoId);
			resultIntent.putExtra("lid", obj.localId);
			resultIntent.putExtra("tit", obj.titulo);
			resultIntent.putExtra("cnt", obj.contenido);
			resultIntent.putExtra("objTip", obj.objTipo);
			resultIntent.putExtra("objId", obj.objId);
			//PendingIntent, permite que el sistema ejecute mi Activity con mis permisos de App.
			PendingIntent resultPendingIntent = PendingIntent.getActivity(contexto, obj.uniqueId, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
			mBuilder.setContentIntent(resultPendingIntent);
			//
			notifyMgr.notify("AU", obj.uniqueId, mBuilder.build());
			obj.existeNotificacion = true;
			Log.w("AU", "Notificaciones, alarma recibida, NOTIFICACION mostrada.");
			//Actualizar el numero en el icono de App
			BadgeUtils.setBadge(contexto, 1);
		}
	}
}*/

bool AUAppGlueAndroidNotifs::notifShow(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, const SI32 uid, const char* grpId /*jstring*/, const SI32 notifId /*jint*/, const char* title /*jstring*/, const char* content /*jstring*/, const char* data /*jstring*/){
	bool r = false;
	if(pEnv != NULL && pContext != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jstring jLaunchCls	= (jstring)AUAppGlueAndroidJNI::getLauncherClassName(pEnv, pContext); NBASSERT(jLaunchCls != NULL)
		if(jLaunchCls != NULL){
			const char* utfLaunchCls = jEnv->GetStringUTFChars(jLaunchCls, 0);
			AUCadenaMutable8* strLaunchClass = new(ENMemoriaTipo_Temporal) AUCadenaMutable8(utfLaunchCls);
			strLaunchClass->reemplazar('.', '/'); PRINTF_INFO("AUAppGlueAndroidNotifs, launch class: '%s'.\n", strLaunchClass->str());
			jclass clsLauncher	= jEnv->FindClass(strLaunchClass->str()); NBASSERT(clsLauncher != NULL)
			if(clsLauncher != NULL){
				jobject notifMngr	= (jobject) AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "NOTIFICATION_SERVICE");
				if(notifMngr != NULL){
					jclass clsContext	= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
					jclass clsNotifMngr	= jEnv->FindClass("android/app/NotificationManager"); NBASSERT(clsNotifMngr != NULL)
					jclass clsNotif		= jEnv->FindClass("android/app/Notification"); NBASSERT(clsNotif != NULL)
					jclass clsPendInt	= jEnv->FindClass("android/app/PendingIntent"); NBASSERT(clsPendInt != NULL)
					jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
					jclass clsBitmap	= jEnv->FindClass("android/graphics/Bitmap"); NBASSERT(clsBitmap != NULL)
					jclass clsBmpFact	= jEnv->FindClass("android/graphics/BitmapFactory"); NBASSERT(clsBmpFact != NULL)
					if(clsContext != NULL && clsNotifMngr != NULL && clsNotif != NULL && clsPendInt != NULL && clsIntent != NULL && clsBitmap != NULL && clsBmpFact != NULL){
						jmethodID cntxGetRes	= jEnv->GetMethodID(clsContext, "getResources", "()Landroid/content/res/Resources;"); NBASSERT(cntxGetRes != NULL)
						jfieldID fDefSound		= jEnv->GetStaticFieldID(clsNotif, "DEFAULT_SOUND", "I"); NBASSERT(fDefSound != NULL)
						jfieldID fDefVibrate	= jEnv->GetStaticFieldID(clsNotif, "DEFAULT_VIBRATE", "I"); NBASSERT(fDefVibrate != NULL)
						jfieldID fDefLights		= jEnv->GetStaticFieldID(clsNotif, "DEFAULT_LIGHTS", "I"); NBASSERT(fDefLights != NULL)
						jfieldID fUpdateCurr	= jEnv->GetStaticFieldID(clsPendInt, "FLAG_UPDATE_CURRENT", "I"); NBASSERT(fUpdateCurr != NULL)
						jmethodID mGetActivity	= jEnv->GetStaticMethodID(clsPendInt, "getActivity", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;"); NBASSERT(mGetActivity != NULL)
						jmethodID mNotify		= jEnv->GetMethodID(clsNotifMngr, "notify", "(Ljava/lang/String;ILandroid/app/Notification;)V"); NBASSERT(mNotify != NULL)
						jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "(Landroid/content/Context;Ljava/lang/Class;)V"); NBASSERT(mIntentInit != NULL)
						jmethodID mIntPutStr	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mIntPutStr != NULL)
						jmethodID mIntPutInt	= jEnv->GetMethodID(clsIntent, "putExtra", "(Ljava/lang/String;I)Landroid/content/Intent;"); NBASSERT(mIntPutInt != NULL)
						jmethodID mBmpFactDecde	= jEnv->GetStaticMethodID(clsBmpFact, "decodeResource", "(Landroid/content/res/Resources;I)Landroid/graphics/Bitmap;"); NBASSERT(mBmpFactDecde != NULL)
						if(cntxGetRes != NULL && fDefSound != NULL && fDefVibrate != NULL && fDefLights != NULL && fUpdateCurr != NULL && mGetActivity != NULL && mNotify != NULL && mIntentInit != NULL && mIntPutStr != NULL && mIntPutInt != NULL && mBmpFactDecde != NULL){
							//Build Intent
							jint defSound		= jEnv->GetStaticIntField(clsNotif, fDefSound);
							jint defVibrate		= jEnv->GetStaticIntField(clsNotif, fDefVibrate);
							jint defLights		= jEnv->GetStaticIntField(clsNotif, fDefLights);
							jobject intentAcept = jEnv->NewObject(clsIntent, mIntentInit, jContext, clsLauncher /*lanchClass*/); NBASSERT(intentAcept != NULL)
							//jobject intentDelete = jEnv->NewObject(clsIntent, mIntentInit, jContext, clsLauncher /*lanchClass*/); NBASSERT(intentDelete != NULL)
							if(intentAcept != NULL /*&& intentDelete != NULL*/){
								jstring jStrNameType	= jEnv->NewStringUTF("nbType");
								jstring jStrNameUId		= jEnv->NewStringUTF("uid");
								jstring jStrNameGId		= jEnv->NewStringUTF("grpId");
								jstring jStrNameNId		= jEnv->NewStringUTF("notifId");
								jstring jStrNameTit		= jEnv->NewStringUTF("title");
								jstring jStrNameCont	= jEnv->NewStringUTF("content");
								jstring jStrNameData	= jEnv->NewStringUTF("data");
								//
								jstring jStrTypeAcpt	= jEnv->NewStringUTF("localNotifAcepted");
								jstring jStrTypeRjct	= jEnv->NewStringUTF("localNotifRejected");
								jstring jStrGId			= jEnv->NewStringUTF(grpId);
								jstring jStrTit			= jEnv->NewStringUTF(title);
								jstring jStrCont		= jEnv->NewStringUTF(content);
								jstring jStrData		= jEnv->NewStringUTF(data);
								//
								jEnv->CallObjectMethod(intentAcept, mIntPutStr, jStrNameType, jStrTypeAcpt);
								jEnv->CallObjectMethod(intentAcept, mIntPutInt, jStrNameUId, (jint)uid);
								jEnv->CallObjectMethod(intentAcept, mIntPutStr, jStrNameGId, jStrGId);
								jEnv->CallObjectMethod(intentAcept, mIntPutInt, jStrNameNId, (jint)notifId);
								jEnv->CallObjectMethod(intentAcept, mIntPutStr, jStrNameTit, jStrTit);
								jEnv->CallObjectMethod(intentAcept, mIntPutStr, jStrNameCont, jStrCont);
								jEnv->CallObjectMethod(intentAcept, mIntPutStr, jStrNameData, jStrData);
								//
								/*
								 jEnv->CallObjectMethod(intentDelete, mIntPutStr, jStrNameType, jStrTypeRjct);
								 jEnv->CallObjectMethod(intentDelete, mIntPutInt, jStrNameUId, (jint)uid);
								 jEnv->CallObjectMethod(intentDelete, mIntPutStr, jStrNameGId, jStrGId);
								 jEnv->CallObjectMethod(intentDelete, mIntPutInt, jStrNameNId, (jint)notifId);
								 jEnv->CallObjectMethod(intentDelete, mIntPutStr, jStrNameTit, jStrTit);
								 jEnv->CallObjectMethod(intentDelete, mIntPutStr, jStrNameCont, jStrCont);
								 jEnv->CallObjectMethod(intentDelete, mIntPutStr, jStrNameData, jStrData);
								 */
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrData)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrCont)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrTit)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrGId)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrTypeRjct)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrTypeAcpt)
								//
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameData)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameCont)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameTit)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameNId)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameGId)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameUId)
								NBJNI_DELETE_REF_LOCAL(jEnv, jStrNameType)
								//--------------------
								//- Build PendingIntent
								//--------------------
								jint flagUpdateCur		= jEnv->GetStaticIntField(clsPendInt, fUpdateCurr); NBASSERT(flagUpdateCur == 0x08000000) //FLAG_UPDATE_CURRENT = 134217728 (0x08000000)
								jobject pendIntAcept	= jEnv->CallStaticObjectMethod(clsPendInt, mGetActivity, jContext, uid /*unique id*/, intentAcept, flagUpdateCur); NBASSERT(pendIntAcept != NULL)
								//jobject pendIntDelete	= jEnv->CallStaticObjectMethod(clsPendInt, mGetActivity, jContext, uid /*unique id*/, intentDelete, flagUpdateCur); NBASSERT(pendIntDelete != NULL)
								if(pendIntAcept != NULL /*&& pendIntDelete != NULL*/){
									jobject notif = NULL;
									//--------------------
									//- Build notification
									//--------------------
									jclass clsNotifBldr	= jEnv->FindClass("android/support/v4/app/NotificationCompat$Builder");
									if(clsNotifBldr != NULL){
										//-------------------------------------------
										//- Package 'android.support.v4' is available
										//-------------------------------------------
										jmethodID mBldrInit	= jEnv->GetMethodID(clsNotifBldr, "<init>", "(Landroid/content/Context;)V"); NBASSERT(mBldrInit != NULL)
										jmethodID mSetTit	= jEnv->GetMethodID(clsNotifBldr, "setContentTitle", "(Ljava/lang/CharSequence;)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetTit != NULL)
										jmethodID mSetTxt	= jEnv->GetMethodID(clsNotifBldr, "setContentText", "(Ljava/lang/CharSequence;)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetTxt != NULL)
										jmethodID mSetPInt	= jEnv->GetMethodID(clsNotifBldr, "setContentIntent", "(Landroid/app/PendingIntent;)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetPInt != NULL)
										jmethodID mSetDInt	= jEnv->GetMethodID(clsNotifBldr, "setDeleteIntent", "(Landroid/app/PendingIntent;)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetDInt != NULL)
										jmethodID mSetDefs	= jEnv->GetMethodID(clsNotifBldr, "setDefaults", "(I)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetDefs != NULL)
										jmethodID mSetSIco	= jEnv->GetMethodID(clsNotifBldr, "setSmallIcon", "(I)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetSIco != NULL)
										jmethodID mSetLIco	= jEnv->GetMethodID(clsNotifBldr, "setLargeIcon", "(Landroid/graphics/Bitmap;)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetLIco != NULL)
										jmethodID mSetSnd	= jEnv->GetMethodID(clsNotifBldr, "setSound", "(Landroid/net/Uri;)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetSnd != NULL)
										jmethodID mSetACanc	= jEnv->GetMethodID(clsNotifBldr, "setAutoCancel", "(Z)Landroid/support/v4/app/NotificationCompat$Builder;"); NBASSERT(mSetACanc != NULL)
										jmethodID mBuild	= jEnv->GetMethodID(clsNotifBldr, "build", "()Landroid/app/Notification;"); NBASSERT(mBuild != NULL)
										if(mBldrInit == NULL || mSetTit == NULL || mSetTxt == NULL || mSetPInt == NULL || mSetDInt == NULL || mSetSIco == NULL || mSetLIco == NULL || mSetDefs == NULL || mSetSnd == NULL || mSetACanc == NULL || mBuild == NULL){
											PRINTF_ERROR("AUAppGlueAndroidNotifs, notifShow NOT all methods obtained (with NotificationCompat.Builder).\n");
										} else {
											jobject builder	= jEnv->NewObject(clsNotifBldr, mBldrInit, jContext); NBASSERT(builder != NULL)
											if(builder != NULL){
												BOOL error = FALSE;
												//Title
												if(title != NULL){
													if(title[0] != '\0'){
														jstring jStrName = jEnv->NewStringUTF(title);
														jEnv->CallObjectMethod(builder, mSetTit, jStrName);
														NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
														//PRINTF_INFO("AUAppGlueAndroidNotifs, set title '%s'.\n", title);
													}
												}
												//Content
												if(content != NULL){
													if(content[0] != '\0'){
														jstring jStrName = jEnv->NewStringUTF(content);
														jEnv->CallObjectMethod(builder, mSetTxt, jStrName);
														NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
														//PRINTF_INFO("AUAppGlueAndroidNotifs, set content '%s'.\n", content);
													}
												}
												//Icon-small
												if(!error){
													const SI32 icSmall = AUAppGlueAndroidJNI::getResDrawable(pEnv, jContext, "ic_notif_small");
													if(icSmall != 0){
														jEnv->CallObjectMethod(builder, mSetSIco, (jint)icSmall);
														//PRINTF_INFO("AUAppGlueAndroidNotifs, set icon %d.\n", icSmall);
													} else {
														PRINTF_WARNING("AUAppGlueAndroidNotifs, drawable 'ic_notif_small' not found.\n");
														if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
														error = TRUE;
													}
												}
												//Icon-large
												if(!error){
													const SI32 icLarge = AUAppGlueAndroidJNI::getResDrawable(pEnv, jContext, "ic_notif_large");
													if(icLarge != 0){
														//Get Context's jni assets object
														jobject jResObj = jEnv->CallObjectMethod(jContext, cntxGetRes);
														if (jResObj != NULL) {
															jobject jBmp = jEnv->CallStaticObjectMethod(clsBmpFact, mBmpFactDecde, jResObj, (jint)icLarge);
															if(jBmp != NULL){
																jEnv->CallObjectMethod(builder, mSetLIco, jBmp);
															}
														}
													} else {
														PRINTF_WARNING("AUAppGlueAndroidNotifs, drawable 'ic_notif_large' not found.\n");
														if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
														error = TRUE;
													}
												}
												//Sound (using setDefaults instead)
												/*if(!error){
													jobject jSound = (jobject) AUAppGlueAndroidJNI::getRingtoneUri(pEnv, "TYPE_NOTIFICATION");
													if(jSound != NULL){
												 jEnv->CallObjectMethod(builder, mSetSnd, jSound);
												 PRINTF_INFO("AUAppGlueAndroidNotifs, set sound.\n");
													}
												 }*/
												if(error){
													PRINTF_ERROR("AUAppGlueAndroidNotifs, could not build notification (with NotificationCompat.Builder).\n");
												} else {
													//Defaults
													jEnv->CallObjectMethod(builder, mSetDefs, defSound + defVibrate);
													//Autocancel
													jEnv->CallObjectMethod(builder, mSetACanc, JNI_TRUE);
													//PendingIntent
													jEnv->CallObjectMethod(builder, mSetPInt, pendIntAcept);
													//jEnv->CallObjectMethod(builder, mSetDInt, pendIntDelete);
													//
													notif = jEnv->CallObjectMethod(builder, mBuild); NBASSERT(notif != NULL)
													PRINTF_INFO("AUAppGlueAndroidNotifs, Notification build (with NotificationCompat.Builder).\n");
												}
											}
										}
										NBJNI_DELETE_REF_LOCAL(jEnv, clsNotifBldr)
									} else {
										if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception 'android/support/v4/app/NotificationCompat$Builder' not found
										//---------------------------------------------
										//Package 'android.support.v4' is NOT available
										//---------------------------------------------
										const SI32 apiCur		= AUAppGlueAndroidJNI::getAPICurrent(pEnv); NBASSERT(apiCur > 0)
										const SI32 honeyComb	= AUAppGlueAndroidJNI::getAPIByName(pEnv, "HONEYCOMB"); NBASSERT(honeyComb > 0)
										if(apiCur > 0 && honeyComb > 0 && apiCur >= honeyComb){
											jclass clsNotifBldr	= jEnv->FindClass("android/app/Notification$Builder"); NBASSERT(clsNotifBldr != NULL)
											if(clsNotifBldr == NULL || clsIntent == NULL || clsNotifMngr == NULL){
												PRINTF_ERROR("AUAppGlueAndroidNotifs, notifShow NOT all classes obtained (with Notification.Builder).\n");
											} else {
												jmethodID mBldrInit	= jEnv->GetMethodID(clsNotifBldr, "<init>", "(Landroid/content/Context;)V"); NBASSERT(mBldrInit != NULL)
												jmethodID mSetTit	= jEnv->GetMethodID(clsNotifBldr, "setContentTitle", "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;"); NBASSERT(mSetTit != NULL)
												jmethodID mSetTxt	= jEnv->GetMethodID(clsNotifBldr, "setContentText", "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;"); NBASSERT(mSetTxt != NULL)
												jmethodID mSetPInt	= jEnv->GetMethodID(clsNotifBldr, "setContentIntent", "(Landroid/app/PendingIntent;)Landroid/app/Notification$Builder;"); NBASSERT(mSetPInt != NULL)
												jmethodID mSetDInt	= jEnv->GetMethodID(clsNotifBldr, "setDeleteIntent", "(Landroid/app/PendingIntent;)Landroid/app/Notification$Builder;"); NBASSERT(mSetDInt != NULL)
												jmethodID mSetSIco	= jEnv->GetMethodID(clsNotifBldr, "setSmallIcon", "(I)Landroid/app/Notification$Builder;"); NBASSERT(mSetSIco != NULL)
												jmethodID mSetLIco	= jEnv->GetMethodID(clsNotifBldr, "setLargeIcon", "(Landroid/graphics/Bitmap;)Landroid/app/Notification$Builder;"); NBASSERT(mSetLIco != NULL)
												jmethodID mSetDefs	= jEnv->GetMethodID(clsNotifBldr, "setDefaults", "(I)Landroid/app/Notification$Builder;"); NBASSERT(mSetDefs != NULL)
												jmethodID mSetSnd	= jEnv->GetMethodID(clsNotifBldr, "setSound", "(Landroid/net/Uri;)Landroid/app/Notification$Builder;"); NBASSERT(mSetSnd != NULL)
												jmethodID mSetACanc	= jEnv->GetMethodID(clsNotifBldr, "setAutoCancel", "(Z)Landroid/app/Notification$Builder;"); NBASSERT(mSetACanc != NULL)
												jmethodID mBuild	= jEnv->GetMethodID(clsNotifBldr, "build", "()Landroid/app/Notification;"); NBASSERT(mBuild != NULL)
												if(mBldrInit == NULL || mSetTit == NULL || mSetTxt == NULL || mSetSIco == NULL || mSetLIco == NULL || mSetDefs == NULL || mSetSnd == NULL || mSetACanc == NULL || mBuild == NULL){
													PRINTF_ERROR("AUAppGlueAndroidNotifs, notifShow NOT all methods obtained (with Notification.Builder).\n");
												} else {
													jobject builder	= jEnv->NewObject(clsNotifBldr, mBldrInit, jContext); NBASSERT(builder != NULL)
													if(builder != NULL){
														BOOL error = FALSE;
														//Title
														if(title != NULL){
															if(title[0] != '\0'){
																jstring jStrName = jEnv->NewStringUTF(title);
																jEnv->CallObjectMethod(builder, mSetTit, jStrName);
																NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
																//PRINTF_INFO("AUAppGlueAndroidNotifs, set title '%s'.\n", title);
															}
														}
														//Content
														if(content != NULL){
															if(content[0] != '\0'){
																jstring jStrName = jEnv->NewStringUTF(content);
																jEnv->CallObjectMethod(builder, mSetTxt, jStrName);
																NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
																//PRINTF_INFO("AUAppGlueAndroidNotifs, set content '%s'.\n", content);
															}
														}
														//Icon-small
														if(!error){
															const SI32 icSmall = AUAppGlueAndroidJNI::getResDrawable(pEnv, jContext, "ic_notif_small");
															if(icSmall != 0){
																jEnv->CallObjectMethod(builder, mSetSIco, (jint)icSmall);
																//PRINTF_INFO("AUAppGlueAndroidNotifs, set icon %d.\n", icSmall);
															} else {
																PRINTF_WARNING("AUAppGlueAndroidNotifs, drawable 'ic_notif_small' not found.\n");
																if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
																error = TRUE;
															}
														}
														//Icon-large
														if(!error){
															const SI32 icLarge = AUAppGlueAndroidJNI::getResDrawable(pEnv, jContext, "ic_notif_large");
															if(icLarge != 0){
																//Get Context's jni assets object
																jobject jResObj = jEnv->CallObjectMethod(jContext, cntxGetRes);
																if (jResObj != NULL) {
																	jobject jBmp = jEnv->CallStaticObjectMethod(clsBmpFact, mBmpFactDecde, jResObj, (jint)icLarge);
																	if(jBmp != NULL){
																		jEnv->CallObjectMethod(builder, mSetLIco, jBmp);
																	}
																}
															} else {
																PRINTF_WARNING("AUAppGlueAndroidNotifs, drawable 'ic_notif_large' not found.\n");
																if(jEnv->ExceptionCheck()) jEnv->ExceptionClear(); //consume Exception
																error = TRUE;
															}
														}
														//Sound (using setDefaults instead)
														/*if(!error){
															jobject jSound = (jobject) AUAppGlueAndroidJNI::getRingtoneUri(pEnv, "TYPE_NOTIFICATION");
															if(jSound != NULL){
														 jEnv->CallObjectMethod(builder, mSetSnd, jSound);
														 PRINTF_INFO("AUAppGlueAndroidNotifs, set sound.\n");
															}
														 }*/
														if(error){
															PRINTF_ERROR("AUAppGlueAndroidNotifs, Could not build notification (with Notification.Builder).\n");
														} else {
															//Defaults
															jEnv->CallObjectMethod(builder, mSetDefs, defSound + defVibrate);
															//Autocancel
															jEnv->CallObjectMethod(builder, mSetACanc, JNI_TRUE);
															//PendingIntent
															jEnv->CallObjectMethod(builder, mSetPInt, pendIntAcept);
															//jEnv->CallObjectMethod(builder, mSetDInt, pendIntDelete);
															//
															notif = jEnv->CallObjectMethod(builder, mBuild); NBASSERT(notif != NULL)
															PRINTF_INFO("AUAppGlueAndroidNotifs, Notification build (with Notification.Builder).\n");
														}
													}
												}
												NBJNI_DELETE_REF_LOCAL(jEnv, clsNotifBldr)
											}
										} else {
											PRINTF_ERROR("AUAppGlueAndroidNotifs, NOT showing notification, current device is previous to Android 3.0.\n");
											/*jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
											 jclass clsNotifMngr	= jEnv->FindClass("android/app/NotificationManager"); NBASSERT(clsNotifMngr != NULL)
											 if(clsIntent == NULL || clsNotif == NULL || clsNotifMngr == NULL){
											 PRINTF_ERROR("AUAppGlueAndroidNotifs, notifShow NOT all classes obtained.\n");
											 } else {
											 jfieldID fIcon		= jEnv->GetFieldID(clsNotif, "icon", "I"); NBASSERT(fIcon != NULL)
											 jfieldID fCategory	= jEnv->GetFieldID(clsNotif, "category", "Ljava/lang/String;"); NBASSERT(fCategory != NULL)
											 jfieldID fVisibilty	= jEnv->GetFieldID(clsNotif, "visibility", "I"); NBASSERT(fVisibilty != NULL)
											 jfieldID fCatAlarm	= jEnv->GetStaticFieldID(clsNotif, "CATEGORY_ALARM", "Ljava/lang/String;"); NBASSERT(fCatAlarm != NULL)
											 jfieldID fVisPublic	= jEnv->GetStaticFieldID(clsNotif, "VISIBILITY_PUBLIC", "I"); NBASSERT(fVisPublic != NULL)
											 if(fIcon == NULL || fCategory == NULL || fVisibilty == NULL || fCatAlarm == NULL || fVisPublic == NULL){
											 PRINTF_ERROR("AUAppGlueAndroidNotifs, notifShow NOT all methods obtained.\n");
											 } else {
											 
											 }
											 NBJNI_DELETE_REF_LOCAL(jEnv, clsNotifMngr)
											 NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
											 }*/
										}
									}
									//Show notification
									if(notif == NULL){
										PRINTF_ERROR("AUAppGlueAndroidNotifs, alarm id %d notifShow failed (could not build Notification object): '%s': '%s'.\n", uid, (title != NULL ? title : ""), (content != NULL ? content : ""));
									} else {
										jstring jStrName = jEnv->NewStringUTF((grpId != NULL ? grpId : ""));
										jEnv->CallVoidMethod(notifMngr, mNotify, jStrName, uid /*unique id*/, notif);
										NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
										PRINTF_INFO("AUAppGlueAndroidNotifs, alarm id %d notifShow done: '%s': '%s'.\n", uid, (title != NULL ? title : ""), (content != NULL ? content : ""));
										NBJNI_DELETE_REF_LOCAL(jEnv, notif)
										r = true;
									}
									//NBJNI_DELETE_REF_LOCAL(jEnv, pendIntDelete)
									NBJNI_DELETE_REF_LOCAL(jEnv, pendIntAcept)
								}
							}
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsBmpFact)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsBitmap)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsPendInt)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsNotif)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsNotifMngr)
						NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
					}
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsLauncher)
			}
			//
			jEnv->ReleaseStringUTFChars(jLaunchCls, utfLaunchCls);
		}
	}
	return r;
}

/*private void privCancelarNotificacion(Context contexto, AlarmManager alarmManager, NotificationManager notifyMgr, Notificacion obj){
	//Remover alarma
	Intent notifIntent = new Intent("android.media.action.DISPLAY_NOTIFICATION");
	notifIntent.addCategory("android.intent.category.DEFAULT");
	PendingIntent notifPendingIntent = PendingIntent.getBroadcast(contexto, obj.uniqueId, notifIntent, PendingIntent.FLAG_UPDATE_CURRENT);
	try {
		alarmManager.cancel(notifPendingIntent);
		obj.existeAlarma = false;
	} catch (Exception e) {
		Log.e("AU", "Notificaciones, alarmManager was not canceled. " + e.toString());
	}
}*/

bool AUAppGlueAndroidNotifs::notifCancel(void* pEnv /*JNIEnv*/, void* pContext /*jobject*/, void* pAlarmMngr /*jobject*/, const SI32 uid){
	bool r = false;
	if(pEnv != NULL && pContext != NULL && pAlarmMngr != NULL){
		JNIEnv* jEnv		= (JNIEnv*) pEnv;
		jobject jContext	= (jobject) pContext;
		jobject jAlarmMngr	= (jobject) pAlarmMngr;
		//
		jclass clsIntent	= jEnv->FindClass("android/content/Intent"); NBASSERT(clsIntent != NULL)
		jclass clsPendInt	= jEnv->FindClass("android/app/PendingIntent"); NBASSERT(clsPendInt != NULL)
		jclass clsAlarmMngr	= jEnv->FindClass("android/app/AlarmManager"); NBASSERT(clsAlarmMngr != NULL)
		if(clsIntent != NULL && clsPendInt != NULL && clsAlarmMngr != NULL){
			jmethodID mIntentInit	= jEnv->GetMethodID(clsIntent, "<init>", "(Ljava/lang/String;)V"); NBASSERT(mIntentInit != NULL)
			jmethodID mIntAddCat	= jEnv->GetMethodID(clsIntent, "addCategory", "(Ljava/lang/String;)Landroid/content/Intent;"); NBASSERT(mIntAddCat != NULL)
			jfieldID fUpdateCurr	= jEnv->GetStaticFieldID(clsPendInt, "FLAG_UPDATE_CURRENT", "I"); NBASSERT(fUpdateCurr != NULL)
			jmethodID mGetBroadcst	= jEnv->GetStaticMethodID(clsPendInt, "getBroadcast", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;"); NBASSERT(mGetBroadcst != NULL)
			jmethodID mAlarmCancel	= jEnv->GetMethodID(clsAlarmMngr, "cancel", "(Landroid/app/PendingIntent;)V"); NBASSERT(mAlarmCancel != NULL)
			if(mIntentInit != NULL && mIntAddCat != NULL && fUpdateCurr != NULL && mGetBroadcst != NULL && mAlarmCancel != NULL){
				jstring jStrName	= jEnv->NewStringUTF("android.media.action.DISPLAY_NOTIFICATION");
				jobject intent		= jEnv->NewObject(clsIntent, mIntentInit, jStrName); NBASSERT(intent != NULL)
				if(intent != NULL){
					jstring jStrName	= jEnv->NewStringUTF("android.intent.category.DEFAULT");
					jEnv->CallObjectMethod(intent, mIntAddCat, jStrName);
					jint flagUpdateCur	= jEnv->GetStaticIntField(clsPendInt, fUpdateCurr); NBASSERT(flagUpdateCur == 0x08000000) //FLAG_UPDATE_CURRENT = 134217728 (0x08000000)
					jobject pendIntent	= jEnv->CallStaticObjectMethod(clsPendInt, mGetBroadcst, jContext, uid /*unique id*/, intent, flagUpdateCur); NBASSERT(pendIntent != NULL)
					if(pendIntent != NULL){
						jEnv->CallVoidMethod(jAlarmMngr, mAlarmCancel, pendIntent);
						if(jEnv->ExceptionCheck()){
							jEnv->ExceptionClear(); //consume Exception
							PRINTF_WARNING("AUAppGlueAndroidNotifs, exception after alarmManager.cancel(%d).\n", uid);
						} else  {
							PRINTF_INFO("AUAppGlueAndroidNotifs, alarm id %d canceled.\n", uid);
							r = true;
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, pendIntent)
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, jStrName)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsAlarmMngr)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsPendInt)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsIntent)
		}
	}
	return r;
}

//Calls

bool AUAppGlueAndroidNotifs::create(AUAppI* app, STMngrNotifsCalls* obj){
	AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueAndroidNotifsData), ENMemoriaTipo_General);
	NBMemory_setZeroSt(*data, AUAppGlueAndroidNotifsData);
	NBMemory_setZeroSt(*obj, STMngrNotifsCalls);
	data->app = app;
	data->listener = new AUAppGlueAndroidNotifsAppListener(data);
	data->app->addAppIntentListener(data->listener);
	//Remote
	{
		data->remote.jTskTkn	= NULL;
		NBString_init(&data->remote.idd);
		NBString_init(&data->remote.token);
	}
	//
	obj->funcCreate				= create;
	obj->funcCreateParam		= data;
	obj->funcDestroy			= destroy;
	obj->funcDestroyParam		= data;
	//
	obj->funcLoadData			= loadData;
	obj->funcLoadDataParam		= data;
	obj->funcSaveData			= saveData;
	obj->funcSaveDataParam		= data;
	obj->funcSetBadgeNumber		= setBadgeNumber;
	obj->funcSetBadgeNumberParam = data;
	obj->funcGetAuthStatus		= getAuthStatus;
	obj->funcGetAuthStatusParam = data;
	//Local notifications
	obj->funcLocalRescheduleAll	= localRescheduleAll;
	obj->funcLocalRescheduleAllParam = data;
	obj->funcLocalEnable		= localEnable;
	obj->funcLocalEnableParam	= data;
	obj->funcLocalAdd			= localAdd;
	obj->funcLocalAddParam		= data;
	obj->funcLocalCancel		= localCancel;
	obj->funcLocalCancelParam	= data;
	obj->funcLocalCancelGrp		= localCancelGrp;
	obj->funcLocalCancelGrpParam = data;
	obj->funcLocalCancelAll		= localCancelAll;
	obj->funcLocalCancelAllParam = data;
	//remote notifications
	obj->funcRemoteGetToken		= remoteGetToken;
	obj->funcRemoteGetTokenParam = data;
	obj->funcRemoteSetToken		= NULL; //ToDo
	obj->funcRemoteSetTokenParam = NULL;
	//
	return true;
}

bool AUAppGlueAndroidNotifs::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		//
		if(data->listener != NULL){
			data->app->removeAppIntentListener(data->listener);
			delete data->listener;
			data->listener = NULL;
		}
		//Remote
		{
			if(data->remote.jTskTkn != NULL){
				jEnv->DeleteGlobalRef(data->remote.jTskTkn);
				data->remote.jTskTkn = NULL;
			}
			NBString_release(&data->remote.idd);
			NBString_release(&data->remote.token);
		}
		data->app = NULL;
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueAndroidNotifs::loadData(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(AUAppGlueAndroidJNI::loadDataFromSharedPrefs(jEnv, jContext, "NB_NOTIFS", "DATA", str)){
					if(NBMngrNotifs::lockedLoadFromJSON(str->str())){
						r = true;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::saveData(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(NBMngrNotifs::lockedSaveToJSON(str)){
					if(AUAppGlueAndroidJNI::saveDataToSharedPrefs(jEnv, jContext, "NB_NOTIFS", "DATA", str->str())){
						r = true;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::setBadgeNumber(void* pData, const SI32 num){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				if(AUAppGlueAndroidNotifs::setAppBadgeNumber(jEnv, jContext, num)){
					r = true;
				}
			}
		}
	}
	return r;
}

ENAppNotifAuthState AUAppGlueAndroidNotifs::getAuthStatus(void* pData, const ENAppNotifAuthQueryMode reqMode){
	return ENAppNotifAuthState_Authorized;
}

//Local notifications

bool AUAppGlueAndroidNotifs::localRescheduleAll(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				//
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::localEnable(void* pData){
	//Is automaric enabled on Android?
	return true;

}

bool AUAppGlueAndroidNotifs::localAdd(void* pData, const SI32 uniqueId, const char* grpId, const SI32 notifId, const UI32 secsFromNow, const char* title, const char* content, const char* notifData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				if(AUAppGlueAndroidNotifs::notifSchedule(jEnv, jContext, uniqueId, grpId, notifId, secsFromNow, title, content, notifData)){
					r = true;
				}
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::localCancel(void* pData, const char* grpId, const SI32 notifId){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				SI32 sz = 0;
				//Find uniqueIds and cancel notifications
				const STAppNotif* notifs = NBMngrNotifs::lockedGetNotifsQueue(&sz);
				if(notifs != NULL && sz > 0){
					SI32 i;
					jobject alarmMngr = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "ALARM_SERVICE");
					if(alarmMngr != NULL){
						for(i = 0; i < sz; i++){
							const STAppNotif* n = &notifs[i];
							if(n->notifId == notifId){
								if(n->grpId->esIgual(grpId)){
									if(AUAppGlueAndroidNotifs::notifCancel(jEnv, jContext, alarmMngr, n->uniqueId)){
										//
									}
								}
							}
						}
					}
				}
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::localCancelGrp(void* pData, const char* grpId){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				SI32 sz = 0;
				//Find uniqueIds and cancel notifications
				const STAppNotif* notifs = NBMngrNotifs::lockedGetNotifsQueue(&sz);
				if(notifs != NULL && sz > 0){
					SI32 i;
					jobject alarmMngr = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "ALARM_SERVICE");
					if(alarmMngr != NULL){
						for(i = 0; i < sz; i++){
							const STAppNotif* n = &notifs[i];
							if(n->grpId->esIgual(grpId)){
								if(AUAppGlueAndroidNotifs::notifCancel(jEnv, jContext, alarmMngr, n->uniqueId)){
									//
								}
							}
						}
					}
				}
				r = true;
			}
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::localCancelAll(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		if(jEnv != NULL){
			jobject jContext = (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
			if(jContext != NULL){
				SI32 sz = 0;
				//Find uniqueIds and cancel notifications
				const STAppNotif* notifs = NBMngrNotifs::lockedGetNotifsQueue(&sz);
				if(notifs != NULL && sz > 0){
					SI32 i;
					jobject alarmMngr = (jobject)AUAppGlueAndroidJNI::getSystemServiceByConstantName(jEnv, jContext, "ALARM_SERVICE");
					if(alarmMngr != NULL){
						for(i = 0; i < sz; i++){
							const STAppNotif* n = &notifs[i];
							if(AUAppGlueAndroidNotifs::notifCancel(jEnv, jContext, alarmMngr, n->uniqueId)){
								//
							}
						}
					}
				}
				r = true;
			}
		}
	}
	return r;
}



//remote notifications

BOOL AUAppGlueAndroidNotifs_consumeTaskResult(AUAppGlueAndroidNotifsData* data, AUAppGlueAndroidJNI* jniGlue, JNIEnv* jEnv, STNBString* dstToken){
	BOOL r = FALSE;
	if(data != NULL && jniGlue != NULL && jEnv != NULL){
		if(data->remote.jTskTkn != NULL){
			jclass clsTask = jEnv->FindClass("com/google/android/gms/tasks/Task");
			if(clsTask == NULL){
				if(!data->jniCheck.clsNotFoundPrintedFirebase){
					PRINTF_ERROR("AUAppGlueAndroidNotifs, Task class not found: 'com/google/android/gms/tasks/Task'.\n");
					data->jniCheck.clsNotFoundPrintedFirebase = TRUE;
				}
				//No Firebase found
				if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
			} else {
				jmethodID mIsComplete	= jEnv->GetMethodID(clsTask, "isComplete", "()Z"); NBASSERT(mIsComplete != NULL)
				jboolean isComplete		= jEnv->CallBooleanMethod(data->remote.jTskTkn, mIsComplete);
				if(isComplete){
					jmethodID mIsSuccessful	= jEnv->GetMethodID(clsTask, "isSuccessful", "()Z"); NBASSERT(mIsSuccessful != NULL)
					jboolean isSuccessful	= jEnv->CallBooleanMethod(data->remote.jTskTkn, mIsSuccessful);
					if(!isSuccessful){
						PRINTF_ERROR("AUAppGlueAndroidNotifs, task for 'getInstanceId' failed.\n");
					} else {
						jclass clsRslt = jEnv->FindClass("com/google/firebase/iid/InstanceIdResult");
						if(clsRslt == NULL){
							if(!data->jniCheck.clsNotFoundPrintedFirebase){
								PRINTF_ERROR("AUAppGlueAndroidNotifs, Task class not found: 'com/google/firebase/iid/InstanceIdResult'.\n");
								data->jniCheck.clsNotFoundPrintedFirebase = TRUE;
							}
							//No Firebase found
							if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
						} else {
							jmethodID mGetResult = jEnv->GetMethodID(clsTask, "getResult", "()Ljava/lang/Object;"); NBASSERT(mGetResult != NULL)
							jstring jResult		= (jstring)jEnv->CallObjectMethod(data->remote.jTskTkn, mGetResult);
							if(jResult == NULL){
								PRINTF_ERROR("AUAppGlueAndroidNotifs, task for 'getResult' is null.\n");
							} else {
								jmethodID mGetId	= jEnv->GetMethodID(clsRslt, "getId", "()Ljava/lang/String;"); NBASSERT(mGetId != NULL)
								jmethodID mGetTkn	= jEnv->GetMethodID(clsRslt, "getToken", "()Ljava/lang/String;"); NBASSERT(mGetTkn != NULL)
								{
									jstring jID = (jstring)jEnv->CallObjectMethod(jResult, mGetId);
									if(jID != NULL){
										const char* utfID = jEnv->GetStringUTFChars(jID, 0);
										PRINTF_INFO("AUAppGlueAndroidNotifs, id('%s').\n", utfID);
										NBString_set(&data->remote.idd, utfID);
										jEnv->ReleaseStringUTFChars(jID, utfID);
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jID)
								}
								{
									jstring jTkn = (jstring)jEnv->CallObjectMethod(jResult, mGetTkn);
									if(jTkn != NULL){
										const char* utfTkn = jEnv->GetStringUTFChars(jTkn, 0);
										PRINTF_INFO("AUAppGlueAndroidNotifs, token('%s').\n", utfTkn);
										NBString_set(&data->remote.token, utfTkn);
										if(dstToken != NULL){
											NBString_setBytes(dstToken, data->remote.token.str, data->remote.token.length);
										}
										jEnv->ReleaseStringUTFChars(jTkn, utfTkn);
										r = TRUE;
									}
									NBJNI_DELETE_REF_LOCAL(jEnv, jTkn)
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jResult)
						}
						NBJNI_DELETE_REF_LOCAL(jEnv, clsRslt)
					}
					//Release current task
					{
						jEnv->DeleteGlobalRef(data->remote.jTskTkn);
						data->remote.jTskTkn = NULL;
					}
				}
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTask)
		}
	}
	return r;
}

bool AUAppGlueAndroidNotifs::remoteGetToken(void* pData, const ENAppNotifTokenQueryMode mode, STNBString* dst){
	bool r = false;
	if(pData != NULL){
		BOOL jTskWasActive = FALSE;
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv = (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		//Consume task result
		if(jEnv != NULL){
			if(AUAppGlueAndroidNotifs_consumeTaskResult(data, jniGlue, jEnv, dst)){
				r = TRUE;
			}
		}
		//Feed task result
		{
			if(dst != NULL){
				NBString_setBytes(dst, data->remote.token.str, data->remote.token.length);
			}
			r = TRUE;
		}
		//Start request
		if(data->remote.jTskTkn == NULL && (mode == ENAppNotifTokenQueryMode_ForceNewRequest || (mode == ENAppNotifTokenQueryMode_CacheAndRequestIfNecesary && data->remote.token.length <= 0))){
			r = FALSE;
			if(jEnv != NULL){
				jclass clsFbId = jEnv->FindClass("com/google/firebase/iid/FirebaseInstanceId");
				if(clsFbId == NULL){
					if(!data->jniCheck.clsNotFoundPrintedFirebase){
						PRINTF_ERROR("AUAppGlueAndroidNotifs, FireBase class not found: 'com/google/firebase/iid/FirebaseInstanceId'.\n");
						data->jniCheck.clsNotFoundPrintedFirebase = TRUE;
					}
					//No Firebase found
					if(jEnv->ExceptionCheck()){ jEnv->ExceptionClear(); }
				} else {
					jmethodID mGetInst	= jEnv->GetStaticMethodID(clsFbId, "getInstance", "()Lcom/google/firebase/iid/FirebaseInstanceId;"); NBASSERT(mGetInst != NULL)
					jmethodID mGetInstId = jEnv->GetMethodID(clsFbId, "getInstanceId", "()Lcom/google/android/gms/tasks/Task;"); NBASSERT(mGetInstId != NULL)
					jmethodID mGetId	= jEnv->GetMethodID(clsFbId, "getId", "()Ljava/lang/String;"); NBASSERT(mGetId != NULL)
					jobject jInst		= jEnv->CallStaticObjectMethod(clsFbId, mGetInst);
					if(jInst != NULL){
						{
							jobject jTsk = jEnv->CallObjectMethod(jInst, mGetInstId);
							if(jTsk != NULL){
								NBASSERT(data->remote.jTskTkn == NULL)
								data->remote.jTskTkn = jEnv->NewGlobalRef(jTsk);
								//Consume task result
								if(AUAppGlueAndroidNotifs_consumeTaskResult(data, jniGlue, jEnv, dst)){
									r = TRUE;
								}
							}
							NBJNI_DELETE_REF_LOCAL(jEnv, jTsk)
						}
					}
					NBJNI_DELETE_REF_LOCAL(jEnv, jInst)
				}
				NBJNI_DELETE_REF_LOCAL(jEnv, clsFbId)
			}
		}
	}
	return r;
}

/*void AUAppGlueAndroidNotifs::remoteSetToken(void* pData, const void* token, const UI32 tokenSz){
	if(pData != NULL){
		AUAppGlueAndroidNotifsData* data = (AUAppGlueAndroidNotifsData*)pData;
		if(token == NULL || tokenSz <= 0){
			//if(data->remote.status == ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken){
			//	data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_Error;
			//}
		} else {
			PRINTF_INFO("AUAppGlueAndroidNotifs, remoteSetToken(%d bytes).\n", tokenSz);
			NBString_setBytes(&data->remote.token, (const char*)token, tokenSz);
			//if(data->remote.status == ENAppGlueIOSNotifsRemoteTokenStatus_RequestingToken){
			//	data->remote.status = ENAppGlueIOSNotifsRemoteTokenStatus_Sucess;
			//}
		}
	}
}*/

