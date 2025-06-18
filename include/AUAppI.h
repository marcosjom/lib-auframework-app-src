//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppI_h
#define AUAppI_h

#include "nb/2d/NBBitmap.h"
#include "nb/net/NBUrl.h"

#if defined(_WIN32)
//WINDOWS
#elif defined(__ANDROID__)
//ANDROID
#	include "AUAppGlueAndroidJNI.h"
#elif defined(__QNX__)
//BB10
#elif defined(__APPLE__)
#	include "TargetConditionals.h"
#	if TARGET_OS_IPHONE
//	IOS
#	else
//	OSX
#	endif
#endif

class AUAppI;
	
class AUAppStateListener {
	public:
		virtual void	appStateOnCreate(AUAppI* app) = 0;
		virtual void	appStateOnDestroy(AUAppI* app) = 0;	//applicationWillTerminate
		virtual void	appStateOnStart(AUAppI* app) = 0;	//applicationDidBecomeActive
		virtual void	appStateOnStop(AUAppI* app) = 0;	//applicationWillResignActive
		virtual void	appStateOnResume(AUAppI* app) = 0;	//applicationWillEnterForeground
		virtual void	appStateOnPause(AUAppI* app) = 0;	//applicationDidEnterBackground
};

#ifdef __ANDROID__
class AUAppSrvcConnListener {
	public:
		virtual void	appSrcvOnConnected(AUAppI* app, void* compName /*jobject::ComponentName*/, void* binder /*jobject::IBinder*/) = 0;
		virtual void	appSrcvOnDisconnected(AUAppI* app, void* compName /*jobject::ComponentName*/) = 0;
};
#endif

#ifdef __ANDROID__
class AUAppActResultListener {
	public:
		virtual void	appActResultReceived(AUAppI* app, SI32 request, SI32 response, void* data /*jobject::Intent*/) = 0;
};
#endif

#ifdef __ANDROID__
class AUAppReqPermResultListener {
	public:
		virtual void	appReqPermResult(AUAppI* app, const SI32 request, void* perms /*jobjectArray*/, void* data /*jintArray*/) = 0;
};
#endif

#ifdef __ANDROID__
class AUAppIntentListener {
	public:
		virtual void	appIntentReceived(AUAppI* app, void* intent /*jobject::Intent*/) = 0;
};
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
class AUAppNotifListener {
	public:
		virtual void	appLocalNotifReceived(AUAppI* app, void* notif /*UILocalNotification*/) = 0;
};
#endif

class AUAppOpenUrlListener {
	public:
		virtual bool	appOpenUrl(AUAppI* app, const STNBUrl* url, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool	appOpenUrlImage(AUAppI* app, const STNBUrl* url, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool	appOpenFileData(AUAppI* app, const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool	appOpenFileImageData(AUAppI* app, const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool	appOpenBitmap(AUAppI* app, const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz) = 0;
};

class AUAppI {
	public:
		//Energy saving
		virtual UI32		getTicksWithSameContentMin() = 0;	//Minimun to avoid rendering the scene (2 to fill doubleBuffers, etc...)
		virtual UI32		getTicksWithSameContent() = 0;		//Amount of ticks were content remained the same
		virtual float		getSecsWithSameContent() = 0;		//Ammount of seconds were content remained the same
		//
		virtual SI32		indiceEscenaRender() = 0;
		virtual void		addAppStateListener(AUAppStateListener* listener) = 0;
		virtual void		removeAppStateListener(AUAppStateListener* listener) = 0;
#		ifdef _WIN32
		virtual HWND		getWindowHandle() = 0;
		virtual void		setWindowHandle(HWND hWnd) = 0;
#		endif
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		virtual void		setLaunchOptions(void* launchOptions /*NSDictionary*/) = 0;
		virtual void*		getLaunchOptions() = 0;
		virtual void		setApplication(void* app /*UIApplication*/) = 0;
		virtual void*		getApplication() = 0;
		virtual void		setWindow(void* win /*UIWindow*/) = 0;
		virtual void*		getWindow() = 0;
		virtual void		setViewController(void* vc /*UIViewController*/) = 0;
		virtual void*		getViewController() = 0;
#		endif
#		ifdef __ANDROID__
		virtual AUAppGlueAndroidJNI*	getGlueJNI() = 0;
		virtual bool					setAppNative(void* jAppNative /*jobject*/);
		virtual void* /*jobject*/		getAppNative();
		virtual bool					setSrvcConnMon(void* srvcConnMon /*jobject*/) = 0;
		virtual void* /*jobject*/		getSrvcConnMon() = 0;
		virtual bool					setView(void* pView /*jobject*/) = 0;
		virtual void* /*jobject*/		getView() = 0;
#		endif
#		ifdef __ANDROID__
		virtual bool		onKeyDown(void* jEnv /*JNIEnv*/, const SI32 keyCode, void* jEvent /*jobject*/);
		virtual bool		onKeyUp(void* jEnv /*JNIEnv*/, const SI32 keyCode, void* jEvent /*jobject*/);
#		endif
#		ifdef __ANDROID__
		virtual void		addAppSrvcConnListener(AUAppSrvcConnListener* listener) = 0;
		virtual void		removeAppSrvcConnListener(AUAppSrvcConnListener* listener) = 0;
#		endif
#		ifdef __ANDROID__
		virtual void		addAppActivityResultListener(AUAppActResultListener* listener) = 0;
		virtual void		removeAppActivityResultListener(AUAppActResultListener* listener) = 0;
#		endif
#		ifdef __ANDROID__
		virtual void		addReqPermResultListener(AUAppReqPermResultListener* listener) = 0;
		virtual void		removeReqPermResultListener(AUAppReqPermResultListener* listener) = 0;
#		endif
#		ifdef __ANDROID__
		virtual void		addAppIntentListener(AUAppIntentListener* listener) = 0;
		virtual void		removeAppIntentListener(AUAppIntentListener* listener) = 0;
#		endif
#		if defined(__APPLE__) && TARGET_OS_IPHONE
		virtual void		addAppNotifListener(AUAppNotifListener* listener) = 0;
		virtual void		removeAppNotifListener(AUAppNotifListener* listener) = 0;
#		endif
		virtual void		addAppOpenUrlListener(AUAppOpenUrlListener* listener) = 0;
		virtual void		removeAppOpenUrlListener(AUAppOpenUrlListener* listener) = 0;
		virtual bool		broadcastOpenUrl(const char* plainUrl, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool		broadcastOpenUrlImage(const char* plainUrl, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool		broadcastOpenFileData(const void* data, const UI32 dataSz, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool		broadcastOpenFileImageData(const void* data, const UI32 dataSz, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz) = 0;
		virtual bool		broadcastOpenBitmap(const STNBBitmap* bmp, const SI32 rotDegFromIntended, const void* usrData, const UI32 usrDataSz) = 0;
};

#endif
