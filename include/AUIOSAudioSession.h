//
//  AudioSession.h
//  Gameplay_iOS
//
//  Created by Nicaragua Binary on 21/02/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#if !defined(Gameplay_iOS_AudioSession_h) && !defined(CONFIG_NB_UNSUPPORT_AUDIO_IO)
#define Gameplay_iOS_AudioSession_h

//ENAUIOSAudioSessionCat

typedef enum ENAUIOSAudioSessionCat_ {
    ENAUIOSAudioSessionCat_Ambient = 0
    , ENAUIOSAudioSessionCat_SoloAmbient
    , ENAUIOSAudioSessionCat_Playback
    , ENAUIOSAudioSessionCat_Record
    , ENAUIOSAudioSessionCat_PlayAndRecord
    , ENAUIOSAudioSessionCat_MultiRoute
    //
    , ENAUIOSAudioSessionCat_Count
} ENAUIOSAudioSessionCat;


class AUIOSAudioSession {
	public:
		AUIOSAudioSession();
		virtual ~AUIOSAudioSession();
		//
		bool			activarSesion();
		bool			desactivarSesion();
		bool			sessionActivaYNoInterrumpida();
		bool			sessionInterrumpida();
		//
		bool			otroSonidoSeEstaReproduciendo();
		bool			establecerCategoria(const ENAUIOSAudioSessionCat cat);
		bool			sobreescribirSalida(bool haciaParlantes);
		bool			permitirCapturaDesdeBluetooth(bool permitirBluetooth);
		//
        void            sessionInterruptStarted();
        void            sessionInterruptEnded();
        //2025-07-06, deprecated on iOS7
        //static void        escuchadorDeInterrupciones(void* datosDeUsuario, UInt32 estadoDeInterrupcion);
		//static void	escuchadorDeEventos(void* datosDeUsuario, AudioSessionPropertyID idPropiedad, UInt32 tamValorPropiedad, const void* valorPropiedad);
	protected:
		bool			_sessionIniciada;		//
		bool			_sessionActiva;			//determina si la session esta activa (inclusive por interrupciones)
		bool			_sessionInterrumpida;	//determina si la session esta en una interrupcion
		bool			_sessionEstabaActivaAntesDeInterrumpir;
        ENAUIOSAudioSessionCat _categoriaActual;
        id              _delegate;  //to handle interruptions
};

#endif
