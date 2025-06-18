//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueAndroidContacts.h"
//
#include "AUAppGlueAndroidJNI.h"
//Android and JNI headers
#include <jni.h>
//
#ifdef __ANDROID__
//is android
#endif

//

#define CONTACTS_PERM_ID			"android.permission.READ_CONTACTS"
#define CONTACTS_PREF_NAME			"NB_CONTACTS"
#define CONTACTS_PREF_DATA_NAME		"android.permission.READ_CONTACTS.requested"

//

struct AUAppGlueAndroidContactsData_;

BOOL AUAppGlueAndroidContacts_startSyncAll(void* pData);

//

class AUAppGlueAndroidContactsListener;

typedef struct AUAppGlueAndroidContactsData_ {
	AUAppI* app;
	//
	AUAppGlueAndroidContactsListener* listener;
	BOOL						requestingAuth;
	//
	BOOL						lstnrSynced;
	void*						lstnr;
	STNBContactLstnrMethods		lstnrMethods;
} AUAppGlueAndroidContactsData;

class AUAppGlueAndroidContactsListener: public AUAppReqPermResultListener {
	public:
		AUAppGlueAndroidContactsListener(AUAppGlueAndroidContactsData* data){
			_data = data;
		}
		virtual ~AUAppGlueAndroidContactsListener(){
			_data = NULL;
		}
		//AUAppReqPermResultListener
		void appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* data /*jintArray*/);
	private:
		AUAppGlueAndroidContactsData* _data;
};

//Calls

bool AUAppGlueAndroidContacts::create(AUAppI* app, STMngrContactsCalls* obj){
	AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)NBMemory_alloc(sizeof(AUAppGlueAndroidContactsData));
	NBMemory_setZeroSt(*data, AUAppGlueAndroidContactsData);
	NBMemory_setZeroSt(*obj, STMngrContactsCalls);
	data->app					= (AUAppI*)app;
	data->requestingAuth		= FALSE;
	data->listener				= new AUAppGlueAndroidContactsListener(data);
	data->app->addReqPermResultListener(data->listener);
	//
	data->lstnrSynced			= FALSE;
	data->lstnr					= NULL;
	NBMemory_setZero(data->lstnrMethods);
	//
	obj->funcCreate				= create;
	obj->funcCreateParam		= data;
	obj->funcDestroy			= destroy;
	obj->funcDestroyParam		= data;
	//
	obj->funcAuthStatus			= authStatus;
	obj->funcAuthStatusParam	= data;
	//
	obj->funcSetListener		= setListener;
	obj->funcSetListenerParam	= data;
	obj->funcStartSync			= startSync;
	obj->funcStartSyncParam		= data;
	obj->funcStartThumbnailReq	= NULL;
	obj->funcStartThumbnailReqParam	= NULL;
	//
	return true;
}

bool AUAppGlueAndroidContacts::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)pData;
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		{
			data->lstnrSynced = FALSE;
			data->lstnr = NULL;
			NBMemory_setZero(data->lstnrMethods);
		}
		//
		if(data->listener != NULL){
			data->app->removeReqPermResultListener(data->listener);
			delete data->listener;
			data->listener = NULL;
		}
		data->app = NULL;
		NBMemory_free(pData);
		r = true;
	}
	return r;
}

//

ENContactsAuthStatus AUAppGlueAndroidContacts::authStatus(void* pData, const BOOL requestIfNecesary){
	ENContactsAuthStatus r = ENContactsAuthStatus_Denied;
	AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)pData;
	if(data->requestingAuth){
		r = ENContactsAuthStatus_Requesting;
	} else {
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		//Query permission
		if(AUAppGlueAndroidJNI::isPermissionGranted(jEnv, jContext, CONTACTS_PERM_ID)){
			r = ENContactsAuthStatus_Authorized;
			if(!data->lstnrSynced && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
				if(AUAppGlueAndroidContacts_startSyncAll(data)){
					data->lstnrSynced = TRUE;
				}
			}
		} else if(AUAppGlueAndroidJNI::getAPICurrent(jEnv) < 23){
			//API 23 and before permissions were granted at install time (and cannot be requested)
			//PRINTF_INFO("Contacts, no permission '%s' and cannot request it (API 22-or-less)...\n", CONTACTS_PERM_ID);
			r = ENContactsAuthStatus_Denied;
		} else {
			//API 23+
			{
				AUCadenaMutable8* str = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				if(!AUAppGlueAndroidJNI::loadDataFromSharedPrefs(jEnv, jContext, CONTACTS_PREF_NAME, CONTACTS_PREF_DATA_NAME, str)){
					r = ENContactsAuthStatus_NotDetermined;
				} else {
					if(str->tamano() > 0){
						r = ENContactsAuthStatus_Denied;
					} else {
						r = ENContactsAuthStatus_NotDetermined;
					}
				}
				str->liberar(NB_RETENEDOR_THIS);
			}
			//Start request
			if(r == ENContactsAuthStatus_NotDetermined && requestIfNecesary){
				PRINTF_INFO("Contacts, starting permission request...\n");
				data->requestingAuth = TRUE;
				const char* permId = CONTACTS_PERM_ID;
				if(!AUAppGlueAndroidJNI::requestPermissions(jEnv, jContext, &permId, 1)){
					PRINTF_ERROR("Contacts, ... could not start permission request.\n");
					data->requestingAuth = FALSE;
					r = ENContactsAuthStatus_Denied;
				} else {
					PRINTF_INFO("Contacts, ... started permission request.\n");
				}
			}
		}
	}
	return r;
}

void AUAppGlueAndroidContacts::setListener(void* pData, void* lstnr, STNBContactLstnrMethods* lstnrMethods){
	AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)pData;
	//Set listener
	{
		data->lstnr = lstnr;
		if(lstnrMethods == NULL){
			NBMemory_setZero(data->lstnrMethods);
		} else {
			data->lstnrMethods = *lstnrMethods;
			if(!data->lstnrSynced && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
				if(AUAppGlueAndroidContacts::authStatus(pData, FALSE) == ENContactsAuthStatus_Authorized){
					if(!data->lstnrSynced){ //Inner validation 'AUAppGlueAndroidContacts::authStatus' can trigger a sync
						if(AUAppGlueAndroidContacts_startSyncAll(data)){
							data->lstnrSynced = TRUE;
						}
					}
				}
			}
		}
	}
}

void AUAppGlueAndroidContacts::startSync(void* pData, const BOOL onlyIfNecesary){
	AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)pData;
	if((!data->lstnrSynced || !onlyIfNecesary) && data->lstnr != NULL && data->lstnrMethods.contactFound != NULL){
		if(AUAppGlueAndroidContacts::authStatus(pData, FALSE) == ENContactsAuthStatus_Authorized){
			if(!data->lstnrSynced){ //Inner validation 'AUAppGlueAndroidContacts::authStatus' can trigger a sync
				if(AUAppGlueAndroidContacts_startSyncAll(data)){
					data->lstnrSynced = TRUE;
				}
			}
		}
	}
}

/*
//Contacts
import android.net.Uri;
import android.provider.ContactsContract;
import android.content.ContentResolver;
import android.database.Cursor;
//
public void fetchContacts() {
	//
	String phoneNumber = null;
	String email = null;
	//
	Uri CONTENT_URI			= ContactsContract.Contacts.CONTENT_URI;
	String _ID				= ContactsContract.Contacts._ID;
	String DISPLAY_NAME		= ContactsContract.Contacts.DISPLAY_NAME;
	String HAS_PHONE_NUMBER	= ContactsContract.Contacts.HAS_PHONE_NUMBER;
	//
	Uri PhoneCONTENT_URI	= ContactsContract.CommonDataKinds.Phone.CONTENT_URI;
	String Phone_CONTACT_ID	= ContactsContract.CommonDataKinds.Phone.CONTACT_ID;
	String NUMBER			= ContactsContract.CommonDataKinds.Phone.NUMBER;
	//
	Uri EmailCONTENT_URI	=  ContactsContract.CommonDataKinds.Email.CONTENT_URI;
	String EmailCONTACT_ID	= ContactsContract.CommonDataKinds.Email.CONTACT_ID;
	String DATA				= ContactsContract.CommonDataKinds.Email.DATA;
	//
	StringBuffer output = new StringBuffer();
	//
	ContentResolver contentResolver = _activity.getContentResolver();
	//
	Cursor cursor = contentResolver.query(CONTENT_URI, null,null, null, null);
	// Loop for every contact in the phone
	System.out.println("Contacts, fetching...\n");
	if (cursor.getCount() > 0) {
		while (cursor.moveToNext()) {
			String contact_id = cursor.getString(cursor.getColumnIndex( _ID ));
			String name = cursor.getString(cursor.getColumnIndex( DISPLAY_NAME ));
			//
			output.setLength(0);
			//
			int hasPhoneNumber = Integer.parseInt(cursor.getString(cursor.getColumnIndex( HAS_PHONE_NUMBER )));
			
			if (hasPhoneNumber > 0) {
				output.append("First Name:" + name);
				// Query and loop for every phone number of the contact
				{
					Cursor phoneCursor = contentResolver.query(PhoneCONTENT_URI, null, Phone_CONTACT_ID + " = ?", new String[]{contact_id}, null);
					while (phoneCursor.moveToNext()) {
						phoneNumber = phoneCursor.getString(phoneCursor.getColumnIndex(NUMBER));
						output.append("\n Phone number:" + phoneNumber);
					}
					phoneCursor.close();
				}
				// Query and loop for every email of the contact
				{
					Cursor emailCursor = contentResolver.query(EmailCONTENT_URI, null, EmailCONTACT_ID + " = ?", new String[]{contact_id}, null);
					while (emailCursor.moveToNext()) {
						email = emailCursor.getString(emailCursor.getColumnIndex(DATA));
						output.append("\nEmail:" + email);
					}
					
					emailCursor.close();
				}
			}
			System.out.println(output);
		}
		System.out.println("Contacts, en fetching.\n");
	}
}*/

//-----------------------
//-- Serial query method (contacts are queried, the all mailboxes are queried, then all emials are queried)
//-----------------------
BOOL AUAppGlueAndroidContacts_startSyncAll(void* pData){
	BOOL r = FALSE;
	AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		{
			jclass clsString			= jEnv->FindClass("java/lang/String"); NBASSERT(clsString != NULL)
			jclass clsContext			= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
			jclass clsContResolver		= jEnv->FindClass("android/content/ContentResolver"); NBASSERT(clsContResolver != NULL)
			jclass clsContract			= jEnv->FindClass("android/provider/ContactsContract"); NBASSERT(clsContract != NULL)
			jclass clsContacts			= jEnv->FindClass("android/provider/ContactsContract$Contacts"); NBASSERT(clsContacts != NULL)
			jclass clsBaseCols			= jEnv->FindClass("android/provider/BaseColumns"); NBASSERT(clsBaseCols != NULL)
			jclass clsCtcCols			= jEnv->FindClass("android/provider/ContactsContract$ContactsColumns"); NBASSERT(clsCtcCols != NULL)
			jclass clsRawCtcCols		= jEnv->FindClass("android/provider/ContactsContract$RawContactsColumns"); NBASSERT(clsRawCtcCols != NULL)
			jclass clsTypesPhone		= jEnv->FindClass("android/provider/ContactsContract$CommonDataKinds$Phone"); NBASSERT(clsTypesPhone != NULL)
			jclass clsTypesEmail		= jEnv->FindClass("android/provider/ContactsContract$CommonDataKinds$Email"); NBASSERT(clsTypesEmail != NULL)
			jclass clsTypesCommCols		= jEnv->FindClass("android/provider/ContactsContract$CommonDataKinds$CommonColumns"); NBASSERT(clsTypesCommCols != NULL)
			jclass clsCursor			= jEnv->FindClass("android/database/Cursor"); NBASSERT(clsCursor != NULL)
			if(clsString != NULL && clsContext != NULL && clsContResolver != NULL && clsContract != NULL && clsContacts != NULL && clsBaseCols != NULL && clsCtcCols != NULL && clsRawCtcCols != NULL && clsTypesPhone != NULL && clsTypesEmail != NULL && clsCursor != NULL && clsTypesCommCols != NULL){
				jmethodID mGetContentResolver = jEnv->GetMethodID(clsContext, "getContentResolver", "()Landroid/content/ContentResolver;"); NBASSERT(mGetContentResolver != NULL)
				jobject jContentResolver = jEnv->CallObjectMethod(jContext, mGetContentResolver); NBASSERT(jContentResolver != NULL)
				//Base fields
				jfieldID fCONTENT_URI	= jEnv->GetStaticFieldID(clsContacts, "CONTENT_URI", "Landroid/net/Uri;"); NBASSERT(fCONTENT_URI != NULL) //PRINTF_INFO("fCONTENT_URI.\n");
				jfieldID f_ID			= jEnv->GetStaticFieldID(clsContacts, "_ID", "Ljava/lang/String;"); NBASSERT(f_ID != NULL) //PRINTF_INFO("f_ID.\n");
				jfieldID fDISPLAY_NAME	= jEnv->GetStaticFieldID(clsContacts, "DISPLAY_NAME", "Ljava/lang/String;"); NBASSERT(fDISPLAY_NAME != NULL) //PRINTF_INFO("fDISPLAY_NAME.\n");
				jfieldID fHAS_PHONE_NUMBER = jEnv->GetStaticFieldID(clsContacts, "HAS_PHONE_NUMBER", "Ljava/lang/String;"); NBASSERT(fHAS_PHONE_NUMBER != NULL) //PRINTF_INFO("fHAS_PHONE_NUMBER.\n");
				jobject jCONTENT_URI	= jEnv->GetStaticObjectField(clsContacts, fCONTENT_URI); NBASSERT(jCONTENT_URI != NULL) //PRINTF_INFO("jCONTENT_URI.\n");
				jobject j_ID			= jEnv->GetStaticObjectField(clsBaseCols, f_ID); NBASSERT(j_ID != NULL) //PRINTF_INFO("j_ID.\n");
				jstring jDISPLAY_NAME	= (jstring)jEnv->GetStaticObjectField(clsCtcCols, fDISPLAY_NAME); NBASSERT(jDISPLAY_NAME != NULL) //PRINTF_INFO("jDISPLAY_NAME.\n");
				jstring jHAS_PHONE_NUMBER = (jstring)jEnv->GetStaticObjectField(clsCtcCols, fHAS_PHONE_NUMBER); NBASSERT(jHAS_PHONE_NUMBER != NULL) //PRINTF_INFO("jHAS_PHONE_NUMBER.\n");
				//Phone fields
				jfieldID fpCONTENT_URI	= jEnv->GetStaticFieldID(clsTypesPhone, "CONTENT_URI", "Landroid/net/Uri;"); NBASSERT(fpCONTENT_URI != NULL) //PRINTF_INFO("fpCONTENT_URI.\n");
				jfieldID fpCONTACT_ID	= jEnv->GetStaticFieldID(clsTypesPhone, "CONTACT_ID", "Ljava/lang/String;"); NBASSERT(fpCONTACT_ID != NULL) //PRINTF_INFO("fpCONTACT_ID.\n");
				jfieldID fpNUMBER		= jEnv->GetStaticFieldID(clsTypesPhone, "NUMBER", "Ljava/lang/String;"); NBASSERT(fpNUMBER != NULL) //PRINTF_INFO("fpNUMBER.\n");
				jobject jpCONTENT_URI	= jEnv->GetStaticObjectField(clsTypesPhone, fpCONTENT_URI); NBASSERT(jpCONTENT_URI != NULL) //PRINTF_INFO("jpCONTENT_URI.\n");
				jstring jpCONTACT_ID	= (jstring)jEnv->GetStaticObjectField(clsRawCtcCols, fpCONTACT_ID); NBASSERT(jpCONTACT_ID != NULL) //PRINTF_INFO("jpCONTACT_ID.\n");
				jstring jpNUMBER		= (jstring)jEnv->GetStaticObjectField(clsTypesPhone, fpNUMBER); NBASSERT(jpNUMBER != NULL) //PRINTF_INFO("jpNUMBER.\n");
				const char* strPCONTACT_ID = jEnv->GetStringUTFChars(jpCONTACT_ID, 0);
				//Email fields
				jfieldID fmCONTENT_URI	= jEnv->GetStaticFieldID(clsTypesEmail, "CONTENT_URI", "Landroid/net/Uri;"); NBASSERT(fmCONTENT_URI != NULL) //PRINTF_INFO("fmCONTENT_URI.\n");
				jfieldID fmCONTACT_ID	= jEnv->GetStaticFieldID(clsTypesEmail, "CONTACT_ID", "Ljava/lang/String;"); NBASSERT(fmCONTACT_ID != NULL) //PRINTF_INFO("fmCONTACT_ID.\n");
				jfieldID fmDATA			= jEnv->GetStaticFieldID(clsTypesEmail, "DATA", "Ljava/lang/String;"); NBASSERT(fmDATA != NULL) //PRINTF_INFO("fmDATA.\n");
				jobject jmCONTENT_URI	= jEnv->GetStaticObjectField(clsTypesEmail, fmCONTENT_URI); NBASSERT(jmCONTENT_URI != NULL) //PRINTF_INFO("jmCONTENT_URI.\n");
				jstring jmCONTACT_ID	= (jstring)jEnv->GetStaticObjectField(clsRawCtcCols, fmCONTACT_ID); NBASSERT(jmCONTACT_ID != NULL) //PRINTF_INFO("jmCONTACT_ID.\n");
				jstring jmDATA			= (jstring)jEnv->GetStaticObjectField(clsTypesCommCols, fmDATA); NBASSERT(jmDATA != NULL) //PRINTF_INFO("jmDATA.\n");
				const char* strMCONTACT_ID = jEnv->GetStringUTFChars(jmCONTACT_ID, 0);
				//
				if(jContentResolver != NULL){
					jmethodID mQuery	= jEnv->GetMethodID(clsContResolver, "query", "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;"); NBASSERT(mQuery != NULL)
					jobject jCursor		= jEnv->CallObjectMethod(jContentResolver, mQuery, jCONTENT_URI, NULL, NULL, NULL, NULL); //PRINTF_INFO("jCursor.\n");
					if(jCursor != NULL){
						jmethodID mGetCount	= jEnv->GetMethodID(clsCursor, "getCount", "()I"); NBASSERT(mGetCount != NULL) //PRINTF_INFO("mGetCount.\n");
						jmethodID mMoveNext = jEnv->GetMethodID(clsCursor, "moveToNext", "()Z"); NBASSERT(mMoveNext != NULL) //PRINTF_INFO("mMoveNext.\n");
						jmethodID mGetColIdx = jEnv->GetMethodID(clsCursor, "getColumnIndex", "(Ljava/lang/String;)I"); NBASSERT(mGetColIdx != NULL) //PRINTF_INFO("mGetColIdx.\n");
						jmethodID mGetStr	= jEnv->GetMethodID(clsCursor, "getString", "(I)Ljava/lang/String;"); NBASSERT(mGetStr != NULL) //PRINTF_INFO("mGetStr.\n");
						jmethodID mClose	= jEnv->GetMethodID(clsCursor, "close", "()V"); NBASSERT(mClose != NULL) //PRINTF_INFO("mClose.\n");
						const jint count	= jEnv->CallIntMethod(jCursor, mGetCount);
						//PRINTF_INFO("%d contacts returned by query.\n", count);
						if(count > 0){
							STNBArraySorted contacts;
							NBArraySorted_init(&contacts, sizeof(STNBContact), NBCompare_STNBContact);
							{
								const jint idIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, j_ID);
								const jint nameIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jDISPLAY_NAME);
								const jint hasPhoneIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jHAS_PHONE_NUMBER);
								while(jEnv->CallBooleanMethod(jCursor, mMoveNext)){
									jstring jUID		= (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, idIdx); NBASSERT(jUID != NULL)
									jstring jName		= (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, nameIdx);
									jstring jHasPhone	= (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, hasPhoneIdx);
									//
									const char* strUID = ""; if(jUID != NULL) strUID = jEnv->GetStringUTFChars(jUID, 0);
									const char* strName = ""; if(jName != NULL) strName = jEnv->GetStringUTFChars(jName, 0);
									const char* strHasPhone = "0"; if(jHasPhone != NULL) strHasPhone = jEnv->GetStringUTFChars(jHasPhone, 0);
									if(!NBString_strIsEmpty(strUID) && !NBString_strIsEmpty(strName)){
										STNBContact c;
										NBMemory_setZeroSt(c, STNBContact);
										//Set values
										NBString_strFreeAndNewBuffer(&c.uid, strUID);
										NBString_strFreeAndNewBuffer(&c.givenName, strName);
										//Add contact
										NBArraySorted_addValue(&contacts, c);
									}
									if(jUID != NULL && strUID != NULL) jEnv->ReleaseStringUTFChars(jUID, strUID);
									if(jName != NULL && strName != NULL) jEnv->ReleaseStringUTFChars(jName, strName);
									if(jHasPhone != NULL && strHasPhone != NULL) jEnv->ReleaseStringUTFChars(jHasPhone, strHasPhone);
									//
									NBJNI_DELETE_REF_LOCAL(jEnv, jUID)
									NBJNI_DELETE_REF_LOCAL(jEnv, jName)
									NBJNI_DELETE_REF_LOCAL(jEnv, jHasPhone)
								}
							}
							//Phone numbers
							{
								jobject jCursor = jEnv->CallObjectMethod(jContentResolver, mQuery, jpCONTENT_URI, NULL, NULL, NULL, NULL); //PRINTF_INFO("jCursor.\n");
								if(jCursor != NULL){
									const jint count = jEnv->CallIntMethod(jCursor, mGetCount);
									PRINTF_INFO("%d phones returned by query.\n", count);
									if(count > 0){
										const jint uidIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jpCONTACT_ID);
										const jint numberIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jpNUMBER);
										while(jEnv->CallBooleanMethod(jCursor, mMoveNext)){
											jstring jUID = (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, uidIdx);
											jstring jNUMBER = (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, numberIdx);
											if(jUID != NULL && jNUMBER != NULL){
												const char* strUID = jEnv->GetStringUTFChars(jUID, 0);
												const char* strNUMBER = jEnv->GetStringUTFChars(jNUMBER, 0);
												//PRINTF_INFO("uid('%s') phone('%s').\n", strUID, strNUMBER);
												if(!NBString_strIsEmpty(strUID) && !NBString_strIsEmpty(strNUMBER)){
													STNBContact srch;
													NBMemory_setZeroSt(srch, STNBContact);
													srch.uid = (char*)strUID;
													{
														const SI32 idx = NBArraySorted_indexOf(&contacts, &srch, sizeof(srch), NULL);
														if(idx < 0){
															PRINTF_INFO("uid('%s') phone('%s') (contact not found).\n", strUID, strNUMBER);
														} else {
															STNBContact* ctc = NBArraySorted_itmPtrAtIndex(&contacts, STNBContact, idx);
															STNBArray arr;
															NBArray_init(&arr, sizeof(char*), NULL);
															//Add current
															if(ctc->phoneNumbers != NULL){
																if(ctc->phoneNumbersSz > 0){
																	NBArray_addItems(&arr, ctc->phoneNumbers, sizeof(ctc->phoneNumbers[0]), ctc->phoneNumbersSz);
																}
																NBMemory_free(ctc->phoneNumbers);
															}
															//Add new
															{
																char* cpy = NBString_strNewBuffer(strNUMBER);
																NBArray_addValue(&arr, cpy);
															}
															//Swap buffers
															{
																ctc->phoneNumbers	= (char**)NBArray_data(&arr);
																ctc->phoneNumbersSz	= (SI32)arr.use;
																NBArray_resignToBuffer(&arr);
															}
															NBArray_release(&arr);
														}
													}
												}
												jEnv->ReleaseStringUTFChars(jUID, strUID);
												jEnv->ReleaseStringUTFChars(jNUMBER, strNUMBER);
												NBJNI_DELETE_REF_LOCAL(jEnv, jUID)
												NBJNI_DELETE_REF_LOCAL(jEnv, jNUMBER)
											}
										}
									}
									jEnv->CallVoidMethod(jCursor, mClose);
									NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
								}
							}
							//Emails
							{
								jobject jCursor = jEnv->CallObjectMethod(jContentResolver, mQuery, jmCONTENT_URI, NULL, NULL, NULL, NULL); //PRINTF_INFO("jCursor.\n");
								if(jCursor != NULL){
									const jint count = jEnv->CallIntMethod(jCursor, mGetCount);
									PRINTF_INFO("%d emails returned by query.\n", count);
									if(count > 0){
										const jint uidIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jmCONTACT_ID);
										const jint dataIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jmDATA);
										while(jEnv->CallBooleanMethod(jCursor, mMoveNext)){
											jstring jUID = (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, uidIdx);
											jstring jDATA = (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, dataIdx);
											if(jUID != NULL && jDATA != NULL){
												const char* strUID = jEnv->GetStringUTFChars(jUID, 0);
												const char* strDATA = jEnv->GetStringUTFChars(jDATA, 0);
												//PRINTF_INFO("email('%s').\n", strDATA);
												if(!NBString_strIsEmpty(strUID) && !NBString_strIsEmpty(strDATA)){
													STNBContact srch;
													NBMemory_setZeroSt(srch, STNBContact);
													srch.uid = (char*)strUID;
													{
														const SI32 idx = NBArraySorted_indexOf(&contacts, &srch, sizeof(srch), NULL);
														if(idx < 0){
															PRINTF_INFO("uid('%s') email('%s') (contact not found).\n", strUID, strDATA);
														} else {
															STNBContact* ctc = NBArraySorted_itmPtrAtIndex(&contacts, STNBContact, idx);
															STNBArray arr;
															NBArray_init(&arr, sizeof(char*), NULL);
															//Add current
															if(ctc->emailAddresses != NULL){
																if(ctc->emailAddressesSz > 0){
																	NBArray_addItems(&arr, ctc->emailAddresses, sizeof(ctc->emailAddresses[0]), ctc->emailAddressesSz);
																}
																NBMemory_free(ctc->emailAddresses);
															}
															//Add new
															{
																char* cpy = NBString_strNewBuffer(strDATA);
																NBArray_addValue(&arr, cpy);
															}
															//Swap buffers
															{
																ctc->emailAddresses	= (char**)NBArray_data(&arr);
																ctc->emailAddressesSz	= (SI32)arr.use;
																NBArray_resignToBuffer(&arr);
															}
															NBArray_release(&arr);
														}
													}
												}
												jEnv->ReleaseStringUTFChars(jUID, strUID);
												jEnv->ReleaseStringUTFChars(jDATA, strDATA);
												NBJNI_DELETE_REF_LOCAL(jEnv, jUID)
												NBJNI_DELETE_REF_LOCAL(jEnv, jDATA)
											}
										}
									}
									jEnv->CallVoidMethod(jCursor, mClose);
									NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
								}
							}
							{
								SI32 i; for(i = 0; i < contacts.use; i++){
									STNBContact* ctc = NBArraySorted_itmPtrAtIndex(&contacts, STNBContact, i);
									//PRINTF_INFO("uid('%s') name('%s') phones(%d) emails(%d).\n", ctc->uid, ctc->givenName, ctc->phoneNumbersSz, ctc->emailAddressesSz);
									(*data->lstnrMethods.contactFound)(data->lstnr, ctc);
									NBStruct_stRelease(NBContact_getSharedStructMap(), ctc, sizeof(*ctc));
								}
								PRINTF_INFO("%d contacts synced.\n", contacts.use);
								NBArraySorted_empty(&contacts);
							}
							NBArraySorted_release(&contacts);
						}
						//Close and release
						jEnv->CallVoidMethod(jCursor, mClose);
						NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
						jCursor = NULL;
						//
						r = TRUE;
					}
				}
				jEnv->ReleaseStringUTFChars(jpCONTACT_ID, strPCONTACT_ID);
				jEnv->ReleaseStringUTFChars(jmCONTACT_ID, strMCONTACT_ID);
				//
				//Release
				NBJNI_DELETE_REF_LOCAL(jEnv, jCONTENT_URI)
				NBJNI_DELETE_REF_LOCAL(jEnv, j_ID)
				NBJNI_DELETE_REF_LOCAL(jEnv, jDISPLAY_NAME)
				NBJNI_DELETE_REF_LOCAL(jEnv, jHAS_PHONE_NUMBER)
				//
				NBJNI_DELETE_REF_LOCAL(jEnv, jpCONTENT_URI)
				NBJNI_DELETE_REF_LOCAL(jEnv, jpCONTACT_ID)
				NBJNI_DELETE_REF_LOCAL(jEnv, jpNUMBER)
				//
				NBJNI_DELETE_REF_LOCAL(jEnv, jmCONTENT_URI)
				NBJNI_DELETE_REF_LOCAL(jEnv, jmCONTACT_ID)
				NBJNI_DELETE_REF_LOCAL(jEnv, jmDATA)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsString)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContResolver)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContract)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContacts)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsBaseCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsCtcCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsRawCtcCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTypesPhone)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTypesEmail)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTypesCommCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsCursor)
		}
	}
	return r;
}

//-----------------------
//-- Nested query method (contacts are queried, and mailboxes are queries per contactId)
//-----------------------
/*BOOL AUAppGlueAndroidContacts_startSyncAll___(void* pData){
	BOOL r = FALSE;
	AUAppGlueAndroidContactsData* data = (AUAppGlueAndroidContactsData*)pData;
	if(data != NULL){
		AUAppGlueAndroidJNI* jniGlue = data->app->getGlueJNI();
		JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
		jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
		{
			jclass clsString			= jEnv->FindClass("java/lang/String"); NBASSERT(clsString != NULL)
			jclass clsContext			= jEnv->FindClass("android/content/Context"); NBASSERT(clsContext != NULL)
			jclass clsContResolver		= jEnv->FindClass("android/content/ContentResolver"); NBASSERT(clsContResolver != NULL)
			jclass clsContract			= jEnv->FindClass("android/provider/ContactsContract"); NBASSERT(clsContract != NULL)
			jclass clsContacts			= jEnv->FindClass("android/provider/ContactsContract$Contacts"); NBASSERT(clsContacts != NULL)
			jclass clsBaseCols			= jEnv->FindClass("android/provider/BaseColumns"); NBASSERT(clsBaseCols != NULL)
			jclass clsCtcCols			= jEnv->FindClass("android/provider/ContactsContract$ContactsColumns"); NBASSERT(clsCtcCols != NULL)
			jclass clsRawCtcCols		= jEnv->FindClass("android/provider/ContactsContract$RawContactsColumns"); NBASSERT(clsRawCtcCols != NULL)
			jclass clsTypesPhone		= jEnv->FindClass("android/provider/ContactsContract$CommonDataKinds$Phone"); NBASSERT(clsTypesPhone != NULL)
			jclass clsTypesEmail		= jEnv->FindClass("android/provider/ContactsContract$CommonDataKinds$Email"); NBASSERT(clsTypesEmail != NULL)
			jclass clsTypesCommCols		= jEnv->FindClass("android/provider/ContactsContract$CommonDataKinds$CommonColumns"); NBASSERT(clsTypesCommCols != NULL)
			jclass clsCursor			= jEnv->FindClass("android/database/Cursor"); NBASSERT(clsCursor != NULL)
			if(clsString != NULL && clsContext != NULL && clsContResolver != NULL && clsContract != NULL && clsContacts != NULL && clsBaseCols != NULL && clsCtcCols != NULL && clsRawCtcCols != NULL && clsTypesPhone != NULL && clsTypesEmail != NULL && clsCursor != NULL && clsTypesCommCols != NULL){
				jmethodID mGetContentResolver = jEnv->GetMethodID(clsContext, "getContentResolver", "()Landroid/content/ContentResolver;"); NBASSERT(mGetContentResolver != NULL)
				jobject jContentResolver = jEnv->CallObjectMethod(jContext, mGetContentResolver); NBASSERT(jContentResolver != NULL)
				//Base fields
				jfieldID fCONTENT_URI	= jEnv->GetStaticFieldID(clsContacts, "CONTENT_URI", "Landroid/net/Uri;"); NBASSERT(fCONTENT_URI != NULL) //PRINTF_INFO("fCONTENT_URI.\n");
				jfieldID f_ID			= jEnv->GetStaticFieldID(clsContacts, "_ID", "Ljava/lang/String;"); NBASSERT(f_ID != NULL) //PRINTF_INFO("f_ID.\n");
				jfieldID fDISPLAY_NAME	= jEnv->GetStaticFieldID(clsContacts, "DISPLAY_NAME", "Ljava/lang/String;"); NBASSERT(fDISPLAY_NAME != NULL) //PRINTF_INFO("fDISPLAY_NAME.\n");
				jfieldID fHAS_PHONE_NUMBER = jEnv->GetStaticFieldID(clsContacts, "HAS_PHONE_NUMBER", "Ljava/lang/String;"); NBASSERT(fHAS_PHONE_NUMBER != NULL) //PRINTF_INFO("fHAS_PHONE_NUMBER.\n");
				jobject jCONTENT_URI	= jEnv->GetStaticObjectField(clsContacts, fCONTENT_URI); NBASSERT(jCONTENT_URI != NULL) //PRINTF_INFO("jCONTENT_URI.\n");
				jobject j_ID			= jEnv->GetStaticObjectField(clsBaseCols, f_ID); NBASSERT(j_ID != NULL) //PRINTF_INFO("j_ID.\n");
				jstring jDISPLAY_NAME	= (jstring)jEnv->GetStaticObjectField(clsCtcCols, fDISPLAY_NAME); NBASSERT(jDISPLAY_NAME != NULL) //PRINTF_INFO("jDISPLAY_NAME.\n");
				jstring jHAS_PHONE_NUMBER = (jstring)jEnv->GetStaticObjectField(clsCtcCols, fHAS_PHONE_NUMBER); NBASSERT(jHAS_PHONE_NUMBER != NULL) //PRINTF_INFO("jHAS_PHONE_NUMBER.\n");
				//Phone fields
				jfieldID fpCONTENT_URI	= jEnv->GetStaticFieldID(clsTypesPhone, "CONTENT_URI", "Landroid/net/Uri;"); NBASSERT(fpCONTENT_URI != NULL) //PRINTF_INFO("fpCONTENT_URI.\n");
				jfieldID fpCONTACT_ID	= jEnv->GetStaticFieldID(clsTypesPhone, "CONTACT_ID", "Ljava/lang/String;"); NBASSERT(fpCONTACT_ID != NULL) //PRINTF_INFO("fpCONTACT_ID.\n");
				jfieldID fpNUMBER		= jEnv->GetStaticFieldID(clsTypesPhone, "NUMBER", "Ljava/lang/String;"); NBASSERT(fpNUMBER != NULL) //PRINTF_INFO("fpNUMBER.\n");
				jobject jpCONTENT_URI	= jEnv->GetStaticObjectField(clsTypesPhone, fpCONTENT_URI); NBASSERT(jpCONTENT_URI != NULL) //PRINTF_INFO("jpCONTENT_URI.\n");
				jstring jpCONTACT_ID	= (jstring)jEnv->GetStaticObjectField(clsRawCtcCols, fpCONTACT_ID); NBASSERT(jpCONTACT_ID != NULL) //PRINTF_INFO("jpCONTACT_ID.\n");
				jstring jpNUMBER		= (jstring)jEnv->GetStaticObjectField(clsTypesPhone, fpNUMBER); NBASSERT(jpNUMBER != NULL) //PRINTF_INFO("jpNUMBER.\n");
				const char* strPCONTACT_ID = jEnv->GetStringUTFChars(jpCONTACT_ID, 0);
				//Email fields
				jfieldID fmCONTENT_URI	= jEnv->GetStaticFieldID(clsTypesEmail, "CONTENT_URI", "Landroid/net/Uri;"); NBASSERT(fmCONTENT_URI != NULL) //PRINTF_INFO("fmCONTENT_URI.\n");
				jfieldID fmCONTACT_ID	= jEnv->GetStaticFieldID(clsTypesEmail, "CONTACT_ID", "Ljava/lang/String;"); NBASSERT(fmCONTACT_ID != NULL) //PRINTF_INFO("fmCONTACT_ID.\n");
				jfieldID fmDATA			= jEnv->GetStaticFieldID(clsTypesEmail, "DATA", "Ljava/lang/String;"); NBASSERT(fmDATA != NULL) //PRINTF_INFO("fmDATA.\n");
				jobject jmCONTENT_URI	= jEnv->GetStaticObjectField(clsTypesEmail, fmCONTENT_URI); NBASSERT(jmCONTENT_URI != NULL) //PRINTF_INFO("jmCONTENT_URI.\n");
				jstring jmCONTACT_ID	= (jstring)jEnv->GetStaticObjectField(clsRawCtcCols, fmCONTACT_ID); NBASSERT(jmCONTACT_ID != NULL) //PRINTF_INFO("jmCONTACT_ID.\n");
				jstring jmDATA			= (jstring)jEnv->GetStaticObjectField(clsTypesCommCols, fmDATA); NBASSERT(jmDATA != NULL) //PRINTF_INFO("jmDATA.\n");
				const char* strMCONTACT_ID = jEnv->GetStringUTFChars(jmCONTACT_ID, 0);
				//
				if(jContentResolver != NULL){
					jmethodID mQuery	= jEnv->GetMethodID(clsContResolver, "query", "(Landroid/net/Uri;[Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)Landroid/database/Cursor;"); NBASSERT(mQuery != NULL)
					jobject jCursor		= jEnv->CallObjectMethod(jContentResolver, mQuery, jCONTENT_URI, NULL, NULL, NULL, NULL); //PRINTF_INFO("jCursor.\n");
					if(jCursor != NULL){
						jmethodID mGetCount	= jEnv->GetMethodID(clsCursor, "getCount", "()I"); NBASSERT(mGetCount != NULL) //PRINTF_INFO("mGetCount.\n");
						jmethodID mMoveNext = jEnv->GetMethodID(clsCursor, "moveToNext", "()Z"); NBASSERT(mMoveNext != NULL) //PRINTF_INFO("mMoveNext.\n");
						jmethodID mGetColIdx = jEnv->GetMethodID(clsCursor, "getColumnIndex", "(Ljava/lang/String;)I"); NBASSERT(mGetColIdx != NULL) //PRINTF_INFO("mGetColIdx.\n");
						jmethodID mGetStr	= jEnv->GetMethodID(clsCursor, "getString", "(I)Ljava/lang/String;"); NBASSERT(mGetStr != NULL) //PRINTF_INFO("mGetStr.\n");
						jmethodID mClose	= jEnv->GetMethodID(clsCursor, "close", "()V"); NBASSERT(mClose != NULL) //PRINTF_INFO("mClose.\n");
						const jint count	= jEnv->CallIntMethod(jCursor, mGetCount);
						//PRINTF_INFO("%d contacts returned by query.\n", count);
						if(count > 0){
							STNBArray phones, emails;
							STNBString filterP, filterM;
							NBArray_init(&phones, sizeof(char*), NULL);
							NBArray_init(&emails, sizeof(char*), NULL);
							NBString_init(&filterP);
							NBString_init(&filterM);
							NBString_concat(&filterP, strPCONTACT_ID);
							NBString_concat(&filterP, " = ?");
							NBString_concat(&filterM, strMCONTACT_ID);
							NBString_concat(&filterM, " = ?");
							{
								jstring jFilterP = jEnv->NewStringUTF(filterP.str);
								jstring jFilterM = jEnv->NewStringUTF(filterM.str);
								const jint idIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, j_ID);
								const jint nameIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jDISPLAY_NAME);
								const jint hasPhoneIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jHAS_PHONE_NUMBER);
								while(jEnv->CallBooleanMethod(jCursor, mMoveNext)){
									jstring jUID		= (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, idIdx); NBASSERT(jUID != NULL)
									jstring jName		= (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, nameIdx);
									jstring jHasPhone	= (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, hasPhoneIdx);
									//
									const char* strUID = ""; if(jUID != NULL) strUID = jEnv->GetStringUTFChars(jUID, 0);
									const char* strName = ""; if(jName != NULL) strName = jEnv->GetStringUTFChars(jName, 0);
									const char* strHasPhone = "0"; if(jHasPhone != NULL) strHasPhone = jEnv->GetStringUTFChars(jHasPhone, 0);
									if(!NBString_strIsEmpty(strUID) && !NBString_strIsEmpty(strName)){
										STNBContact c;
										NBMemory_setZeroSt(c, STNBContact);
										//Set values
										NBString_strFreeAndNewBuffer(&c.uid, strUID);
										NBString_strFreeAndNewBuffer(&c.givenName, strName);
										//Subqueries
										{
											jstring jStrEmpty	= jEnv->NewStringUTF("");
											jstring jStrUID		= jEnv->NewStringUTF(strUID);
											jobjectArray jUIDs	= (jobjectArray)jEnv->NewObjectArray(1, clsString, jStrEmpty);
											jEnv->SetObjectArrayElement(jUIDs, 0, jStrUID);
											//Phone numbers
											if(!NBString_strIsEqual(strHasPhone, "0")){
												jobject jCursor = jEnv->CallObjectMethod(jContentResolver, mQuery, jpCONTENT_URI, NULL, jFilterP, jUIDs, NULL); //PRINTF_INFO("jCursor.\n");
												if(jCursor != NULL){
													const jint count = jEnv->CallIntMethod(jCursor, mGetCount);
													//PRINTF_INFO("%d phones returned by query.\n", count);
													if(count > 0){
														const jint numberIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jpNUMBER);
														while(jEnv->CallBooleanMethod(jCursor, mMoveNext)){
															jstring jNUMBER = (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, numberIdx);
															if(jNUMBER != NULL){
																const char* strNUMBER = jEnv->GetStringUTFChars(jNUMBER, 0);
																//PRINTF_INFO("phone('%s').\n", strNUMBER);
																if(!NBString_strIsEmpty(strNUMBER)){
																	char* cpy = NBString_strNewBuffer(strNUMBER);
																	NBArray_addValue(&phones, cpy);
																}
																jEnv->ReleaseStringUTFChars(jNUMBER, strNUMBER);
																NBJNI_DELETE_REF_LOCAL(jEnv, jNUMBER)
																jNUMBER = NULL;
															}
														}
													}
													jEnv->CallVoidMethod(jCursor, mClose);
													NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
												}
											}
											//Emails
											{
												jobject jCursor = jEnv->CallObjectMethod(jContentResolver, mQuery, jmCONTENT_URI, NULL, jFilterM, jUIDs, NULL); //PRINTF_INFO("jCursor.\n");
												if(jCursor != NULL){
													const jint count = jEnv->CallIntMethod(jCursor, mGetCount);
													//PRINTF_INFO("%d emails returned by query.\n", count);
													if(count > 0){
														const jint dataIdx = jEnv->CallIntMethod(jCursor, mGetColIdx, jmDATA);
														while(jEnv->CallBooleanMethod(jCursor, mMoveNext)){
															jstring jDATA = (jstring)jEnv->CallObjectMethod(jCursor, mGetStr, dataIdx);
															if(jDATA != NULL){
																const char* strDATA = jEnv->GetStringUTFChars(jDATA, 0);
																//PRINTF_INFO("email('%s').\n", strDATA);
																if(!NBString_strIsEmpty(strDATA)){
																	char* cpy = NBString_strNewBuffer(strDATA);
																	NBArray_addValue(&emails, cpy);
																}
																jEnv->ReleaseStringUTFChars(jDATA, strDATA);
																NBJNI_DELETE_REF_LOCAL(jEnv, jDATA)
																jDATA = NULL;
															}
														}
													}
													jEnv->CallVoidMethod(jCursor, mClose);
													NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
												}
											}
											NBJNI_DELETE_REF_LOCAL(jEnv, jStrUID)
											NBJNI_DELETE_REF_LOCAL(jEnv, jStrEmpty)
											NBJNI_DELETE_REF_LOCAL(jEnv, jUIDs)
										}
										{
											if(phones.use > 0){
												c.phoneNumbers		= (char**)NBArray_data(&phones);
												c.phoneNumbersSz	= (SI32)phones.use;
												NBArray_resignToBuffer(&phones);
											}
											if(emails.use > 0){
												c.emailAddresses	= (char**)NBArray_data(&emails);
												c.emailAddressesSz	= (SI32)emails.use;
												NBArray_resignToBuffer(&emails);
											}
										}
										PRINTF_INFO("uid('%s') name('%s') phones(%d) emails(%d).\n", c.uid, c.givenName, c.phoneNumbersSz, c.emailAddressesSz);
										//Notify
										(*data->lstnrMethods.contactFound)(data->lstnr, &c);
										//Clear data
										NBStruct_stRelease(NBContact_getSharedStructMap(), &c, sizeof(c));
									}
									if(jUID != NULL && strUID != NULL) jEnv->ReleaseStringUTFChars(jUID, strUID);
									if(jName != NULL && strName != NULL) jEnv->ReleaseStringUTFChars(jName, strName);
									if(jHasPhone != NULL && strHasPhone != NULL) jEnv->ReleaseStringUTFChars(jHasPhone, strHasPhone);
									//
									NBJNI_DELETE_REF_LOCAL(jEnv, jUID)
									NBJNI_DELETE_REF_LOCAL(jEnv, jName)
									NBJNI_DELETE_REF_LOCAL(jEnv, jHasPhone)
								}
								NBJNI_DELETE_REF_LOCAL(jEnv, jFilterP)
								NBJNI_DELETE_REF_LOCAL(jEnv, jFilterM)
							}
							{
								SI32 i; for(i = 0; i < phones.use; i++){
									char* v = NBArray_itmValueAtIndex(&phones, char*, i);
									if(v != NULL) NBMemory_free(v);
								}
								NBArray_empty(&phones);
							}
							{
								SI32 i; for(i = 0; i < emails.use; i++){
									char* v = NBArray_itmValueAtIndex(&emails, char*, i);
									if(v != NULL) NBMemory_free(v);
								}
								NBArray_empty(&emails);
							}
							NBArray_release(&phones);
							NBArray_release(&emails);
							NBString_release(&filterP);
							NBString_release(&filterM);
						}
						//Close and release
						jEnv->CallVoidMethod(jCursor, mClose);
						NBJNI_DELETE_REF_LOCAL(jEnv, jCursor)
						jCursor = NULL;
						//
						r = TRUE;
					}
				}
				jEnv->ReleaseStringUTFChars(jpCONTACT_ID, strPCONTACT_ID);
				jEnv->ReleaseStringUTFChars(jmCONTACT_ID, strMCONTACT_ID);
				//
				//Release
				NBJNI_DELETE_REF_LOCAL(jEnv, jCONTENT_URI)
				NBJNI_DELETE_REF_LOCAL(jEnv, j_ID)
				NBJNI_DELETE_REF_LOCAL(jEnv, jDISPLAY_NAME)
				NBJNI_DELETE_REF_LOCAL(jEnv, jHAS_PHONE_NUMBER)
				//
				NBJNI_DELETE_REF_LOCAL(jEnv, jpCONTENT_URI)
				NBJNI_DELETE_REF_LOCAL(jEnv, jpCONTACT_ID)
				NBJNI_DELETE_REF_LOCAL(jEnv, jpNUMBER)
				//
				NBJNI_DELETE_REF_LOCAL(jEnv, jmCONTENT_URI)
				NBJNI_DELETE_REF_LOCAL(jEnv, jmCONTACT_ID)
				NBJNI_DELETE_REF_LOCAL(jEnv, jmDATA)
			}
			NBJNI_DELETE_REF_LOCAL(jEnv, clsString)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContext)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContResolver)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContract)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsContacts)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsBaseCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsCtcCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsRawCtcCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTypesPhone)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTypesEmail)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsTypesCommCols)
			NBJNI_DELETE_REF_LOCAL(jEnv, clsCursor)
		}
	}
	return r;
}*/

// Listener methods

void AUAppGlueAndroidContactsListener::appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* grantsResults /*jintArray*/){
	//PRINTF_INFO("AUAppGlueAndroidContactsListener::appReqPermResult.\n");
	AUAppGlueAndroidJNI* jniGlue = _data->app->getGlueJNI();
	JNIEnv* jEnv		= (JNIEnv*)jniGlue->curEnv(); NBASSERT(jEnv != NULL)
	jobject jContext	= (jobject)jniGlue->jActivity(); NBASSERT(jContext != NULL)
	{
		jobjectArray jPerms	= (jobjectArray)perms;
		jintArray jGrants	= (jintArray)grantsResults;
		jint* iGrants		= jEnv->GetIntArrayElements(jGrants, 0);
		SI32 i; const SI32 count = (SI32)jEnv->GetArrayLength(jPerms); NBASSERT(count == (SI32)jEnv->GetArrayLength(jGrants))
		for(i = 0; i < count; i++){
			jobject jPerm	= jEnv->GetObjectArrayElement(jPerms, i);
			jint granted	= iGrants[i];
			const char* utf8 = jEnv->GetStringUTFChars((jstring)jPerm, 0);
			if(NBString_strIsEqual(utf8, CONTACTS_PERM_ID)){
				//Save "already asked" value
				AUAppGlueAndroidJNI::saveDataToSharedPrefs(jEnv, jContext, CONTACTS_PREF_NAME, CONTACTS_PREF_DATA_NAME, "YES");
				//
				_data->requestingAuth = FALSE;
			}
			//PRINTF_INFO("AVCapture, Perm #%d / %d: '%s' (%s).\n", (i + 1), count, utf8, granted ? "GRANTED" : "DENIED");
			jEnv->ReleaseStringUTFChars((jstring)jPerm, utf8);
		}
	}
}

