//
//  AudioSession.cpp
//  Gameplay_iOS
//
//  Created by Nicaragua Binary on 21/02/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "AUIOSAudioSession.h"
#include "AUAppNucleoPrecompilado.h"
#include "AUAppNucleoEncabezado.h"

#include <OpenAL/alc.h>
#import <AVFoundation/AVAudioSession.h>
//#include <AudioToolbox/AudioToolbox.h> //2025-07-06, deprecated on iOS7


#ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
@interface AUIOSAudioSessionDelegate : NSObject {
    AUIOSAudioSession* obj;
}
- (id) initWithObj:(AUIOSAudioSession*)obj;
- (void) interruption:(NSNotification*)notification;
@end
#endif

#ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
@implementation AUIOSAudioSessionDelegate
- (id) initWithObj:(AUIOSAudioSession*)pObj {
    obj = pObj;
    return [super init];
}

- (void) interruption:(NSNotification*)notification {
    // get the user info dictionary
    NSDictionary *interuptionDict = notification.userInfo;
    // get the AVAudioSessionInterruptionTypeKey enum from the dictionary
    NSInteger interuptionType = [[interuptionDict valueForKey:AVAudioSessionInterruptionTypeKey] integerValue];
    // decide what to do based on interruption type here...
    switch (interuptionType) {
        case AVAudioSessionInterruptionTypeBegan:
            if(obj != NULL){
                obj->sessionInterruptStarted();
            }
            break;
        case AVAudioSessionInterruptionTypeEnded:
            if(obj != NULL){
                obj->sessionInterruptEnded();
            }
            break;
        default:
            break;
    }
}
@end
#endif

#ifndef CONFIG_NB_UNSUPPORT_AUDIO_IO
AUIOSAudioSession::AUIOSAudioSession(){
	_categoriaActual		= ENAUIOSAudioSessionCat_Count;
	_sessionIniciada		= false;
	_sessionActiva			= false;
	_sessionInterrumpida	= false;
	_sessionEstabaActivaAntesDeInterrumpir	= false;
	//
    _delegate               = [[AUIOSAudioSessionDelegate alloc] initWithObj: this];
    //
    {
        NSError *error = nil;
        AVAudioSession* session = [AVAudioSession sharedInstance];
        if(![session setCategory:AVAudioSessionCategoryPlayback error:&error]){
            PRINTF_ERROR("iOS: no se pudo inicializar la sesion de audio: '%s'", [[error description] UTF8String]);
        } else {
            _sessionIniciada = true;
            //observe interruptions
            [[NSNotificationCenter defaultCenter] addObserver:_delegate
                                                         selector:@selector(interruption:)
                                                             name:AVAudioSessionInterruptionNotification
                                                           object:nil];
        }
    }
    //
    //2025-07-06, deprecated on iOS7
    /*if(AudioSessionInitialize(NULL, NULL, AUIOSAudioSession::escuchadorDeInterrupciones, this)!=0){ //run loop, run loop mode, metodo escuchador de interrupciones, datos que son pasados al metodo escuchador
		PRINTF_ERROR("iOS: no se pudo inicializar la sesion de audio\n");
	} else {
		_sessionIniciada = true;
		/ *CAMBIOS ESCUCHABLES
		 kAudioSessionProperty_AudioRouteChange
		 kAudioSessionProperty_CurrentHardwareOutputVolume
		 kAudioSessionProperty_AudioInputAvailable
		 kAudioSessionProperty_ServerDied
		 kAudioSessionProperty_InputSources
		 kAudioSessionProperty_OutputDestinations
		 kAudioSessionProperty_InputGainAvailable
		 kAudioSessionProperty_InputGainScalar* /
		//Registrar escuchador de cambios de salida
		AudioSessionPropertyID idCambioEscuchar;
		idCambioEscuchar = kAudioSessionProperty_AudioRouteChange;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_CurrentHardwareOutputVolume;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_AudioInputAvailable;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_ServerDied;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_InputSources;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_OutputDestinations;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_InputGainAvailable;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
		idCambioEscuchar = kAudioSessionProperty_InputGainScalar;
		AudioSessionAddPropertyListener (idCambioEscuchar, AUIOSAudioSession::escuchadorDeEventos, NULL);
	}*/
}

AUIOSAudioSession::~AUIOSAudioSession(){
	desactivarSesion();
    //
    if(_delegate != nil){
        [_delegate release];
        _delegate = nil;
    }
}

bool AUIOSAudioSession::activarSesion(){
	bool exito = false;
	if(_sessionInterrumpida){
		PRINTF_ERROR("iOS, no se pudo activar la sesion de audio mientras esta interrumpida\n");
	} else {
        NSError* err = nil;
        if(![[AVAudioSession sharedInstance] setActive:YES error:&err]){
            PRINTF_ERROR("iOS, no se pudo activar la sesion de audio: '%s'.\n", err == nil ? "unknown" : [[err description] UTF8String]);
        } else {
            PRINTF_INFO("iOS Sesion de audio activada\n");
            NBGestorSonidos::contextActivate();
            _sessionActiva = true;
            exito = true;
        }
        //2025-07-06, deprecated on iOS7
		/*if(AudioSessionSetActive(true)!=0){
			PRINTF_ERROR("iOS, no se pudo activar la sesion de audio\n");
		} else {
			PRINTF_INFO("iOS Sesion de audio activada\n");
			_sessionActiva = true;
			exito = true;
		}*/
	}
	return exito;
}

bool AUIOSAudioSession::desactivarSesion(){
	bool exito = false;
	if(_sessionInterrumpida){
		PRINTF_ERROR("iOS, no se pudo desactivar la sesion de audio mientras esta interrumpida\n");
	} else {
        NSError* err = nil;
        if(![[AVAudioSession sharedInstance] setActive:NO error:&err]){
            PRINTF_ERROR("iOS, no se pudo activar la sesion de audio: '%s'.\n", err == nil ? "unknown" : [[err description] UTF8String]);
        } else {
            PRINTF_INFO("iOS Sesion de audio activada\n");
            NBGestorSonidos::contextDeactivate();
            _sessionActiva = false;
            exito = true;
        }
        //2025-07-06, deprecated on iOS7
		/*if(AudioSessionSetActive(false)!=0){
			PRINTF_ERROR("iOS, no se pudo desactivar la sesion de audio\n");
		} else {
			PRINTF_INFO("iOS Sesion de audio desactivada\n");
			_sessionActiva = false;
			exito = true;
		}*/
	}
	return exito;
}

bool AUIOSAudioSession::sessionActivaYNoInterrumpida(){
	return _sessionActiva;
}

bool AUIOSAudioSession::sessionInterrumpida(){
	return _sessionInterrumpida;
}

void AUIOSAudioSession::sessionInterruptStarted(){
    PRINTF_INFO("Sonido iOS: interrupcion iniciada\n");
    _sessionEstabaActivaAntesDeInterrumpir = _sessionActiva;
    _sessionActiva        = false;
    _sessionInterrumpida    = true;
    NBGestorSonidos::contextDeactivate();
}

void AUIOSAudioSession::sessionInterruptEnded(){
    PRINTF_INFO("Sonido iOS: interrupcion finalizada\n");
    _sessionActiva        = _sessionEstabaActivaAntesDeInterrumpir;
    _sessionInterrumpida    = false;
    NBGestorSonidos::contextActivate();
}

bool AUIOSAudioSession::otroSonidoSeEstaReproduciendo(){
    bool r = false;
    AVAudioSession* session = [AVAudioSession sharedInstance];
    {
        r = [session isOtherAudioPlaying];
    }
    return r;
    //2025-07-06, deprecated on iOS7
    /*bool otroAudioPresente = false;
	UInt32 otroAudioSeEstaReproduciendo;
	UInt32 tamanoVariable = sizeof(otroAudioSeEstaReproduciendo);
	if(AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &tamanoVariable, &otroAudioSeEstaReproduciendo)!=0){
		PRINTF_ERROR("iOS: no se pudo identificar si hay otro audo en reproduccion\n");
	} else {
		otroAudioPresente = otroAudioSeEstaReproduciendo;
	}
	return otroAudioPresente;*/
}

AVAudioSessionCategory AUIOSAudioSession_enToAVAudioSessionCategory(const ENAUIOSAudioSessionCat cat){
    switch(cat){
        case ENAUIOSAudioSessionCat_Ambient: return AVAudioSessionCategoryAmbient;
        case ENAUIOSAudioSessionCat_SoloAmbient: return AVAudioSessionCategorySoloAmbient;
        case ENAUIOSAudioSessionCat_Playback: return AVAudioSessionCategoryPlayback;
        case ENAUIOSAudioSessionCat_Record: return AVAudioSessionCategoryRecord;
        case ENAUIOSAudioSessionCat_PlayAndRecord: return AVAudioSessionCategoryPlayAndRecord;
        case ENAUIOSAudioSessionCat_MultiRoute: return AVAudioSessionCategoryMultiRoute;
        default: break;
    }
    return AVAudioSessionCategoryAmbient;
}

//Para reproduccion de audio:
//Si hay otro audio reproduciendose se recomienda usar la categoria: kAudioSessionCategory_AmbientSound
//en caso contrario: kAudioSessionCategory_SoloAmbientSound
bool AUIOSAudioSession::establecerCategoria(const ENAUIOSAudioSessionCat pCat){
    bool r = false;
    NSError *err = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    AVAudioSessionCategory cat = AUIOSAudioSession_enToAVAudioSessionCategory(pCat);
    if(![session setCategory:cat error:&err]){
        PRINTF_ERROR("iOS: no se pudo estabecer la categoria de session de audio: '%s'\n", err == nil ? "unknown" : [[err description] UTF8String]);
    } else {
        r = true;
        _categoriaActual = pCat;
    }
    return r;
    //2025-07-06, deprecated on iOS7
	/*
	 kAudioSessionCategory_AmbientSound              = 'ambi',
	 kAudioSessionCategory_SoloAmbientSound          = 'solo',
	 kAudioSessionCategory_MediaPlayback             = 'medi',
	 kAudioSessionCategory_RecordAudio               = 'reca',
	 kAudioSessionCategory_PlayAndRecord             = 'plar',
	 kAudioSessionCategory_AudioProcessing           = 'proc'
	 * /
	bool exito = false;
	if(AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(kAudioSessionCategory_paramtro), &kAudioSessionCategory_paramtro)!=0){
		PRINTF_ERROR("iOS: no se pudo estabecer la categoria de session de audio\n");
	} else {
		exito = true;
		_categoriaActual = kAudioSessionCategory_paramtro;
	}
	return exito;*/
}

//Util para cuando se esta grabando audio y se desea reproducir los sonidos por los parlantes.
//O bien cuando el usuario coloca o quita los auriculares.
//Cuando se establecer una categoria que implica grabacion de audio, automaticamente
bool AUIOSAudioSession::sobreescribirSalida(bool haciaParlantes){
    bool r = false;
    NSError *err = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    if(![session overrideOutputAudioPort:(haciaParlantes ? AVAudioSessionPortOverrideSpeaker : AVAudioSessionPortOverrideNone) error:&err]){
        PRINTF_ERROR("iOS: no se pudo sobreescribir la ruta de salida de audio: '%s'.\n", err == nil ? "unknown" : [[err description] UTF8String]);
    }
    return r;
    //2025-07-06, deprecated on iOS7
	/*bool exito = false;
	UInt32 sobreescripcionDeRuta = (haciaParlantes?kAudioSessionOverrideAudioRoute_Speaker:kAudioSessionOverrideAudioRoute_None);
	if(AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(sobreescripcionDeRuta), &sobreescripcionDeRuta)){
		PRINTF_ERROR("iOS: no se pudo sobreescribir la ruta de salida de audio\n");
	} else {
		exito = true;
	}
	return exito;*/
}

bool AUIOSAudioSession::permitirCapturaDesdeBluetooth(bool permitirBluetooth){
    bool r = false;
    NBASSERT(FALSE) //unimplemented yet!
    /*AVAudioSession *session = [AVAudioSession sharedInstance];
    if(![session  error:&err]){
        PRINTF_ERROR("iOS: no se pudo sobreescribir la ruta de salida de audio: '%s'.\n", err == nil ? "unknown" : [[err description] UTF8String]);
    }*/
    return r;
    //2025-07-06, deprecated on iOS7
	/*bool exito = false;
	UInt32 bluetoothPermitido = (permitirBluetooth?1:0);
	if(AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryEnableBluetoothInput, sizeof(bluetoothPermitido), &bluetoothPermitido)){
		PRINTF_ERROR("iOS: no se pudo establecer que esta permitida captura desde bluetooth\n");
	} else {
		exito = true;
	}
	return exito;*/
}

//2025-07-06, deprecated on iOS7
/*
void AUIOSAudioSession::escuchadorDeInterrupciones(void* datosDeUsuario, UInt32 estadoDeInterrupcion){
	AUIOSAudioSession* sessionAudio = (AUIOSAudioSession*)datosDeUsuario;
	if(estadoDeInterrupcion==kAudioSessionBeginInterruption){
		PRINTF_INFO("Sonido iOS: interrupcion iniciada\n");
		sessionAudio->_sessionEstabaActivaAntesDeInterrumpir = sessionAudio->_sessionActiva;
		sessionAudio->_sessionActiva		= false;
		sessionAudio->_sessionInterrumpida	= true;
		if(sessionAudio->_contextoOpenAL != NULL) alcMakeContextCurrent(NULL);
	} else if(estadoDeInterrupcion==kAudioSessionEndInterruption){
		PRINTF_INFO("Sonido iOS: interrupcion finalizada\n");
		sessionAudio->_sessionActiva		= sessionAudio->_sessionEstabaActivaAntesDeInterrumpir;
		sessionAudio->_sessionInterrumpida	= false;
		if(sessionAudio->_contextoOpenAL != NULL) alcMakeContextCurrent(sessionAudio->_contextoOpenAL);
	}
}*/

//2025-07-06, deprecated on iOS7
/*
void AUIOSAudioSession::escuchadorDeEventos(void* datosDeUsuario, AudioSessionPropertyID idPropiedad, UInt32 tamValorPropiedad, const void* valorPropiedad){
	/ *CAMBIOS ESCUCHABLES
	 kAudioSessionProperty_AudioRouteChange (iOS 2.0)
	 kAudioSessionProperty_CurrentHardwareOutputVolume (iOS 2.1)
	 kAudioSessionProperty_AudioInputAvailable (iOS 2.2)
	 kAudioSessionProperty_ServerDied (iOS 3.0)
	 kAudioSessionProperty_InputSources (iOS 5.0)
	 kAudioSessionProperty_OutputDestinations (iOS 5.0)
	 kAudioSessionProperty_InputGainAvailable (iOS 5.0)
	 kAudioSessionProperty_InputGainScalar (iOS 5.0)* /
	if(idPropiedad==kAudioSessionProperty_CurrentHardwareOutputVolume){ //(iOS 2.1)
		Float32 nuevoVolumenSalida			= *((Float32*)valorPropiedad);
		PRINTF_INFO("Sonido iOS: volumen de salida: %f\n", nuevoVolumenSalida);
	} else if(idPropiedad==kAudioSessionProperty_AudioInputAvailable){ //(iOS 2.2)
		UInt32 dispositivoCapturaDisp		= *((UInt32*)valorPropiedad);
		PRINTF_INFO("Sonido iOS: dispositivo de captura disponible (%s)\n", dispositivoCapturaDisp==0?"NO":"SI");
	} else if(idPropiedad==kAudioSessionProperty_ServerDied){ //(iOS 3.0)
		UInt32 servidorSonidoHaMuerto		= *((UInt32*)valorPropiedad);
		PRINTF_INFO("Sonido iOS: servidor de sonido ha muerto (%s)\n", servidorSonidoHaMuerto==0?"NO":"SI");
	} else if(idPropiedad==kAudioSessionProperty_InputGainAvailable){ //(iOS 5.0)
		UInt32 disponibleVolumenCaptura		= *((UInt32*)valorPropiedad);
		PRINTF_INFO("Sonido iOS: disponible volumen de captura (%s)\n", disponibleVolumenCaptura==0?"NO":"SI");
	} else if(idPropiedad==kAudioSessionProperty_InputGainScalar){ //(iOS 5.0)
		Float32 nuevoVolumenCaptura			= *((Float32*)valorPropiedad);
		PRINTF_INFO("Sonido iOS: volumen de captura: %f\n", nuevoVolumenCaptura);
	} else if(idPropiedad==kAudioSessionProperty_AudioRouteChange){ //(iOS 2.0)
		CFDictionaryRef refDatosCambioRuta	= (CFDictionaryRef)valorPropiedad;
		CFNumberRef refRazonCambioRuta		= (CFNumberRef) CFDictionaryGetValue (refDatosCambioRuta, CFSTR(kAudioSession_AudioRouteChangeKey_Reason));
		SInt32 razonCambioRuta;				CFNumberGetValue(refRazonCambioRuta, kCFNumberSInt32Type, &razonCambioRuta);
		AUCadenaMutable8* strCambioRuta		= new(ENMemoriaTipo_Temporal) AUCadenaMutable8("Sonido iOS: cambio de ruta de salida de audio ");
		if(razonCambioRuta==kAudioSessionRouteChangeReason_OldDeviceUnavailable){
			strCambioRuta->agregar(" (razon: dispositivo anterior no disponible)");
		} else if(razonCambioRuta==kAudioSessionRouteChangeReason_CategoryChange){
			strCambioRuta->agregar(" (razon: cambio de catergoria)");
		} else if(razonCambioRuta==kAudioSessionRouteChangeReason_NewDeviceAvailable){
			strCambioRuta->agregar(" (razon: nuevo dispositivo disponible)");
		} else if(razonCambioRuta==kAudioSessionRouteChangeReason_NoSuitableRouteForCategory){
			strCambioRuta->agregar(" (razon: sin ruta elegible para categoria)");
		} else if(razonCambioRuta==kAudioSessionRouteChangeReason_Override){
			strCambioRuta->agregar(" (razon: redireccionamiento de salida)");
		} else if(razonCambioRuta==kAudioSessionRouteChangeReason_WakeFromSleep){
			strCambioRuta->agregar(" (razon: dispositivo despertado)");
		} else if(razonCambioRuta==kAudioSessionRouteChangeReason_Unknown){
			strCambioRuta->agregar(" (razon: desconocida para iOS)");
		} else {
			strCambioRuta->agregar(" (razon: desconocida)");
		}
		/ *
		 enum {
		 kAudioSessionRouteChangeReason_Unknown                    = 0,
		 kAudioSessionRouteChangeReason_NewDeviceAvailable         = 1,
		 kAudioSessionRouteChangeReason_OldDeviceUnavailable       = 2,
		 kAudioSessionRouteChangeReason_CategoryChange             = 3,
		 kAudioSessionRouteChangeReason_Override                   = 4,
		 // this enum has no constant with a value of 5
		 kAudioSessionRouteChangeReason_WakeFromSleep              = 6,
		 kAudioSessionRouteChangeReason_NoSuitableRouteForCategory = 7
		 };
		 * /
		strCambioRuta->agregar("\n");
		PRINTF_INFO("%s", strCambioRuta->str());
		strCambioRuta->liberar(NB_RETENEDOR_NULL);
	}
}*/

#endif


