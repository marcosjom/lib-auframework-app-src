//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUIOSMonoViewController_H
#define AUIOSMonoViewController_H

#import <UIKit/UIKit.h> //for UIViewController
#include "AUApp.h"
#include "AUAppEscenasAdmin.h"

typedef struct STDeviceScreenDef_ {
	float height;
	float width;
} STDeviceScreenDef;

typedef struct STDeviceDef_ {
	const char*			machineName; //utsname.machine value
	float				screenDpi;
	STDeviceScreenDef	screenSz;
} STDeviceDef;

@interface AUIOSMonoViewController : UIViewController {
	void* _opaqueData;
}

- (id) initWithApplication:(UIApplication *)application appName:(const char*)appName window:(UIWindow*)window frame:(CGRect)frame;
- (AUApp*) getApp;
- (SI32) iScene;
- (bool) setAppAdmin:(AUAppEscenasAdmin*)appAdmin;

@end

#endif
