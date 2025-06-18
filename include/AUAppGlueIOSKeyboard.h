//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSKeyboard_H
#define AUAppGlueIOSKeyboard_H

#include "NBGestorTeclasDefs.h"
#include "AUAppI.h"

class AUAppGlueIOSKeyboard {
	public:
		//Calls
		static bool create(void* app, STMngrKeyboardCalls* obj);
		static bool destroy(void* data);
		static bool requestFocus(void* data);
		static bool setVisible(void* data, const bool visible);
		static bool restartKeyboard(void* data);
		static bool restartInputPre(void* data, const BOOL textChange, const BOOL rngSelChange, const BOOL rngMrkChange);
		static bool restartInputPost(void* data, const BOOL textChange, const BOOL rngSelChange, const BOOL rngMrkChange);
};

#endif
