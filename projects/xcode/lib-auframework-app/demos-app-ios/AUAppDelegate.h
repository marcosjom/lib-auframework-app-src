//
//  AUAppDelegate.h
//  game-alanica-ios
//
//  Created by Marcos Ortega on 17/03/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "EAGLViewController.h"

@interface AUAppDelegate : UIResponder <UIApplicationDelegate> {
	EAGLViewController*			_vistaOpenGLControlador;
}

//@property (nonatomic, retain)

@property (nonatomic, retain) IBOutlet UIWindow *window;
//@property (assign) IBOutlet AUGLView *vistaGL;
//@property (strong, nonatomic) UIWindow *window;

@end
