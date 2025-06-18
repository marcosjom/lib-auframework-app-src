//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueAndroidKeyboard_H
#define AUAppGlueAndroidKeyboard_H

#include "NBGestorTeclasDefs.h"
#include "AUAppI.h"

class AUAppGlueAndroidKeyboard {
	public:
		static bool requestFocus(void* jEnv /*JNIEnv*/, void* jView /*jobject*/);
		static bool setKeyboardVisible(void* jEnv /*JNIEnv*/, void* jContext /*jobject*/, void* jView /*jobject*/, const bool visible);
		//Calls
		static bool create(void* app, STMngrKeyboardCalls* obj);
		static bool destroy(void* data);
		static bool requestFocus(void* data);
		static bool setVisible(void* data, const bool visible);
		static bool restartKeyboard(void* data);
		static bool updateKeyboardCursor(void* data, const SI32 rngSelStart, const SI32 rngSelSz, const SI32 rngMrkStart, const SI32 rngMrkSz, const char* rngMrkText);
};

#endif
