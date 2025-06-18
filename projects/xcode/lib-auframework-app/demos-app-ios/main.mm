//
//  main.m
//  app-auframework-demo-ios
//
//  Created by Marcos Ortega on 11/06/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AUAppDelegate.h"

int main(int argc, char * argv[]){
	int r = 0;
	if(!AUApp::inicializarNucleo(AUAPP_BIT_MODULO_RED)){
		PRINTF_ERROR("No se pudo incializar NUCLEO\n");
		return -1;
	} else {
		//------------------------
		//--- Imprimir lista de parametros (informativo)
		//------------------------
		int iParam; for(iParam=0; iParam<argc; iParam++) PRINTF_INFO("Parametro exe %d: '%s'\n", iParam, argv[iParam]);
		//------------------------
		//--- Identificar endianes del procesador actual
		//------------------------
		UI16 datoPruebaEndianes = 1; UI8* valoresBytes = (UI8*)&datoPruebaEndianes;
		PRINTF_INFO("El dispositivo es %s-ENDIEN (%d bytes por puntero)\n", (valoresBytes[0]==0 && valoresBytes[1]!=0)?"BIG":"LITTLE", (SI32)sizeof(void*));
		//------------------------
		//--- Inicializar numeros aleatorios
		//------------------------
		srand(time(NULL));
		//------------------------
		//--- Ciclo de App
		//------------------------
		@autoreleasepool {
			r = UIApplicationMain(argc, argv, nil, NSStringFromClass([AUAppDelegate class]));
		}
	}
	AUApp::finalizarNucleo();
	return r;
}
