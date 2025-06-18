//
//  AppDelegate.h
//  app-test-renderText
//
//  Created by Marcos Ortega on 26/11/14.
//
//

#import <Cocoa/Cocoa.h>
#import "AUGLView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet AUGLView *vistaGL;

@end

