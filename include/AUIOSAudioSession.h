//
//  AudioSession.h
//  Gameplay_iOS
//
//  Created by Nicaragua Binary on 21/02/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#if !defined(Gameplay_iOS_AudioSession_h) && !defined(CONFIG_NB_UNSUPPORT_AUDIO_IO)
#define Gameplay_iOS_AudioSession_h

#include <OpenAL/alc.h>
#include <AudioToolbox/AudioToolbox.h>

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
		void			establecerContextoOpenAL(ALCcontext* contextoOpenAL);
		bool			establecerCategoria(UInt32 kAudioSessionCategory_paramtro);
		bool			sobreescribirSalida(bool haciaParlantes);
		bool			permitirCapturaDesdeBluetooth(bool permitirBluetooth);
		//
		static void		escuchadorDeInterrupciones(void* datosDeUsuario, UInt32 estadoDeInterrupcion);
		static void		escuchadorDeEventos(void* datosDeUsuario, AudioSessionPropertyID idPropiedad, UInt32 tamValorPropiedad, const void* valorPropiedad);
	protected:
		bool			_sessionIniciada;		//
		bool			_sessionActiva;			//determina si la session esta activa (inclusive por interrupciones)
		bool			_sessionInterrumpida;	//determina si la session esta en una interrupcion
		bool			_sessionEstabaActivaAntesDeInterrumpir;
		UInt32			_categoriaActual;
		ALCcontext*		_contextoOpenAL;
};

#endif
