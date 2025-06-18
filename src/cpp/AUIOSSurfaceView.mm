//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUIOSSurfaceView.h"

@implementation AUIOSSurfaceView

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

// This return value defines the type of
// object returned by '[id layer]' method.
+ (Class) layerClass {
	return [CAEAGLLayer class];
	//AVCaptureVideoPreviewLayer
}

@end


