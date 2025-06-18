//
//  main.m
//  app-test-renderText
//
//  Created by Marcos Ortega on 26/11/14.
//
//

#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[]) {
	int r = 0;
	if(!AUApp::inicializarNucleo(AUAPP_BIT_MODULO_RED)){
		PRINTF_ERROR("No se pudo incializar NUCLEO\n");
		return -1;
	} else {
		//NBGestorArchivos::establecerRutaPrefijoPaquete("./"); //TEMPORAL, para generar cache de fuentes
		//NBGestorArchivos::establecerRutaCacheOperativa("./"); //TEMPORAL, para generar cache de fuentes
		//------------------------
		//--- Inicializar numeros aleatorios
		//------------------------
		srand(time(NULL));
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
		//--- Cilco de App
		//------------------------
		r = NSApplicationMain(argc, argv);
	}
	AUApp::finalizarNucleo();
	return r;
}
