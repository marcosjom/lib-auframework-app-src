//
//  EAGLViewController.h
//  Gameplay_iOS
//
//  Created by Marcos Ortega on 29/09/12.
//
//

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#import "EAGLView.h"
//AudioSession
#ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
#import "iOSAudioSession.h"
#endif
//
@interface EAGLViewController : UIViewController<UIKeyInput, UITextInput, UIDocumentInteractionControllerDelegate> {
	@private
		AUApp*						_app;
		AUAppEscenasAdminSimple*	_escenas;
		//
		float						_versionIOS;
		EAGLView*					_vistaOpenGL;
#		ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
		iOSAudioSession*			_sessionAudio;
#		endif
		//
		UI32						_ticksAcumuladosEnSegundo;
		UI32						_ticksAnimadoresAcum;
		UI32						_ticksPantalaAcum;
		CADisplayLink*				_sincronizadorPantalla;
		//
		float						_escalaHaciaHD;
		bool						_escalaHaciaHDEsManual;
		//
		bool						_mailEnviandoRepeticion;
		//
		bool						_tecladoPermitidoMostrar;
		bool						_tecladoEstaVisible;
		//
		id< UITextInputDelegate >	_inputDelegate;
		UITextWritingDirection		_inputBaseDirection;
}
//
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil frame:(CGRect)frame;
- (void)finalizar;
// Foco
- (void) focoObtenido;
- (void) focoPerdido;
// Configuracion
- (void) activarIdiomaPreferido;
// Timers
- (void) crearTimers;
- (void) detenerTimers;
- (void) tickPantalla;
@end

//Funciones C
void volcarBuffer(void* param, AUApp* juego, SI32 iEscenaRender);
//Teclado
bool tecladoVisible(void* param);
bool tecladoMostrar(void* param);
bool tecladoOcultar(void* param);

//----------------------------
//-- Procesamiento de texto --
//----------------------------
@interface AUTextPosition : UITextPosition {
	SI32 _pos;
}
-(id) initWithPos:(SI32) pos;
-(SI32) pos;
@end

@interface AUTextRange : UITextRange {
	AUTextPosition* _start;
	AUTextPosition* _end;
	BOOL _empty;
}
-(id) initWithRange:(NBRangoI)rango;
-(id) initWithAUTextPositions:(AUTextPosition*)posStart posEnd:(AUTextPosition*)posEnd;
-(void) dealloc;
-(AUTextPosition*) start;
-(AUTextPosition*) end;
-(BOOL) isEmpty;
@end

/*@interface AUTextInputStringTokenizer : UITextInputStringTokenizer {
	//
}
@end*/


