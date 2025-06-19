//
//  AUGLView.h
//
//  Created by Marcos Ortega on 10/05/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// for display link
#import <QuartzCore/QuartzCore.h>

@interface AUGLView : NSOpenGLView /*<NSDraggingDestination>*/ {
    @private
    CVDisplayLinkRef        displayLink;
    double                    deltaTime;
    double                    outputTime;
    //
    AUApp*                    _app;
    AUAppEscenasAdminSimple* _escenas;
    SI32                    _indiceEscenaRender;
    bool                    _mousePresionado;
    NSMutableString*        _rutaArchivoCargar;
    //
    AUArregloMutable*        _dragPaths;
};

@property (assign) IBOutlet NSWindow *ventana;

- (BOOL) abrirArchivo:(NSString*)rutaArchivo;

@end

