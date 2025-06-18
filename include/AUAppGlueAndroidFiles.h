//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAndroidFiles_H
#define AUAppGlueAndroidFiles_H

#include "NBGestorArchivos.h"
#include "AUAppGlueAndroidJNI.h"

class AUAppGlueAndroidFiles {
	public:
		//Calls
		static bool create(void* app, STMngrFilesCalls* obj);
		static bool destroy(void* data);
		static bool getPathPrefix(void* data, const ENMngrFilesPathType type, AUCadenaMutable8* dst);
		static AUArchivo* openFile(void* data, const char* path, const ENArchivoModo fileMode, const ENMemoriaTipo typeMemResidence);
};

class AUArchivoFisicoAPK: public AUArchivo  {
	public:
		AUArchivoFisicoAPK(void* aasset /*AAsset*/);
		virtual ~AUArchivoFisicoAPK();
		//
		void		lock();
		void		unlock();
		//
		ENFileOp	lastOperation() const;
		SI32		leer(void* destino, SI32 tamBloque, SI32 cantBloques, AUArchivo* punteroRedundanteTemporal);
		SI32		escribir(const void* fuente, SI32 tamBloque, SI32 cantBloques, AUArchivo* punteroRedundanteTemporal);
		SI32		posicionActual() const;
		bool		moverDesdeInicio(const SI32 posicionDestino);
		bool		moverDesdePosActual(const SI32 cantidadBytes);
		bool		moverDesdePosFinal(const SI32 cantidadBytes);
		void		rebobinar();
		void		cerrar();
		//
		AUOBJMETODOS_CLASESID_DEFINICION
		AUOBJMETODOS_CLASESNOMBRES_DEFINICION
		AUOBJMETODOS_CLONAR_DEFINICION
	private:
		NBHILO_MUTEX_CLASE	_mutex;
#		ifdef CONFIG_NB_INCLUIR_VALIDACIONES_ASSERT
		SI32				_mutexLockCount;
#		endif
		//Propiedades
		void*				_aasset /*AAsset*/;
		SI32				_posActual;
		ENFileOp			_lastOp;
		//
		AUCadenaMutable8*	_strPaths[ENMngrFilesPathType_Count];
};

#endif
