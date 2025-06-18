//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueWinFiles_H
#define AUAppGlueWinFiles_H

#include "NBGestorArchivos.h"

class AUAppGlueWinFiles {
	public:
		//Calls
		static bool create(void* app, STMngrFilesCalls* obj);
		static bool destroy(void* data);
		static bool getPathPrefix(void* data, const ENMngrFilesPathType type, AUCadenaMutable8* dst);
};

#endif
