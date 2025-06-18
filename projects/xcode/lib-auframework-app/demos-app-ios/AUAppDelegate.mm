//
//  AUAppDelegate.m
//  game-alanica-ios
//
//  Created by Marcos Ortega on 17/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#import "AUAppDelegate.h"

@implementation AUAppDelegate

@synthesize window = _window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    //Override point for customization after application launch.
	_vistaOpenGLControlador			= [[EAGLViewController alloc] initWithNibName:nil bundle:nil frame:_window.frame];
	_window.rootViewController		= _vistaOpenGLControlador;
	//
	[_window addSubview:_vistaOpenGLControlador.view];
	[_window makeKeyAndVisible];
	//
	[UIApplication sharedApplication].idleTimerDisabled = YES;
	//
    return YES;
}

- (void)dealloc {
	AU_GESTOR_PILA_LLAMADAS_PUSH_APP("Gameplay_iOSAppDelegate::dealloc")
	//[_window release];
	//[super dealloc];
	AU_GESTOR_PILA_LLAMADAS_POP_APP
}

- (void)applicationWillTerminate:(UIApplication *)application{
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
	/*
	 Called when the application is about to terminate.
	 Save data if appropriate.
	 See also applicationDidEnterBackground:.
	 */
	[_vistaOpenGLControlador finalizar];
}

- (void)applicationDidBecomeActive:(UIApplication *)application{
	// Handle the user leaving the app while the Facebook login dialog is being shown
	// For example: when the user presses the iOS "home" button while the login dialog is active
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
	[_vistaOpenGLControlador focoObtenido];
	[_vistaOpenGLControlador crearTimers];
	//Reset App Badge number
}

- (void)applicationWillResignActive:(UIApplication *)application{
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
	[_vistaOpenGLControlador detenerTimers];
	[_vistaOpenGLControlador focoPerdido];
}

- (void)applicationWillEnterForeground:(UIApplication *)application{
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
	//[_vistaOpenGLControlador crearTimers];
}


- (void)applicationDidEnterBackground:(UIApplication *)application{
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
	//[_vistaOpenGLControlador detenerTimers];
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
	PRINTF_INFO("AppDelegate, Memory warning!\n");
	//[super applicationDidReceiveMemoryWarning: application];
}

@end

