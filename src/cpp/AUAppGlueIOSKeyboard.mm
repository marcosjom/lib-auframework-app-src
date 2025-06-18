//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSKeyboard.h"
#include "NBGestorTeclas.h"
#include "AUAppGlueIOSKeyInput.h"

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

//-------------------------
// AUAppGlueIOSKeyboard
//-------------------------

typedef struct AUAppGlueIOSKeybData_ {
	AUAppI*					app;
	AUAppGlueIOSKeyInput*	curItf;
} AUAppGlueIOSKeybData;

//Calls

bool AUAppGlueIOSKeyboard::create(void* app, STMngrKeyboardCalls* obj){
	AUAppGlueIOSKeybData* data = (AUAppGlueIOSKeybData*)NBGestorMemoria::reservarMemoria(sizeof(AUAppGlueIOSKeybData), ENMemoriaTipo_General);
	data->app		= (AUAppI*)app;
	data->curItf	= nil;
	//
	obj->funcCreate					= create;
	obj->funcCreateParam			= data;
	obj->funcDestroy				= destroy;
	obj->funcDestroyParam			= data;
	obj->funcRequestFocus			= requestFocus;
	obj->funcRequestFocusParam		= data;
	obj->funcSetVisible				= setVisible;
	obj->funcSetVisibleParam		= data;
	obj->funcRestartKeyboard		= restartKeyboard;
	obj->funcRestartKeyboardParam	= data;
	obj->funcUpdateKeyboardCursor			= NULL;
	obj->funcUpdateKeyboardCursorParam		= NULL;
	obj->funcRestartInputPre		= restartInputPre;
	obj->funcRestartInputPreParam	= data;
	obj->funcRestartInputPost		= restartInputPost;
	obj->funcRestartInputPostParam	= data;
	//
	return true;
}

bool AUAppGlueIOSKeyboard::destroy(void* pData){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSKeybData* data = (AUAppGlueIOSKeybData*)pData;
		if(data->app != NULL){
			data->app = NULL;
		}
		if(data->curItf != nil){
			[data->curItf release];
			data->curItf = nil;
		}
		NBGestorMemoria::liberarMemoria(pData);
		r = true;
	}
	return r;
}

bool AUAppGlueIOSKeyboard::requestFocus(void* pData){
	bool r = false;
	if(pData != NULL){
		//Activate the windows focus
		//(asume iOS will manage keyboard behavior)
		NBGestorTeclas::keyboardWinFocusChanged(true);
	}
	return r;
}

BOOL AUAppGlueIOSKeyboard_updateInput_(AUAppGlueIOSKeybData* data, const BOOL isInputEnabled){
	BOOL r = FALSE;
	AUAppGlueIOSKeyInput* prev = data->curItf;
	data->curItf = nil;
	@autoreleasepool {
		//Create new
		if(isInputEnabled){
			if(data->curItf == nil){
				UIWindow* win = (UIWindow*)data->app->getWindow();
				UIViewController* vc = (UIViewController*)data->app->getViewController();
				if(win != nil && vc != nil){
					UIView* view = vc.view;
					if(view != nil){
						data->curItf = [[AUAppGlueIOSKeyInput alloc] initWithVisualView:view];
						[view insertSubview:data->curItf atIndex: 0];
						//[view addSubview:data->curItf];
						PRINTF_INFO("AUAppGlueIOSKeyInput created (isFirstReponder: %d).\n", [data->curItf isFirstResponder]);
					}
				}
			}
			if(data->curItf != nil){
				if([data->curItf isFirstResponder]){
					r = TRUE;
				} else {
					PRINTF_INFO("AUAppGlueIOSKeyInput becomingFirstResponder: (%llu, isFirstReponder: %d).\n", (UI64)data->curItf, [data->curItf isFirstResponder]);
					if([data->curItf becomeFirstResponder]){
						PRINTF_INFO("AUAppGlueIOSKeyInput becameFirstReponder: (isFirstReponder: %d).\n", [data->curItf isFirstResponder]);
						r = TRUE;
					} else {
						PRINTF_ERROR("AUAppGlueIOSKeyInput becomeFirstResponder failed: (isFirstReponder: %d).\n", [data->curItf isFirstResponder]);
					}
				}
			}
		}
		//Release prev
		{
			if(prev != nil){
				if([prev isFirstResponder]){
					[prev resignFirstResponder];
				}
				[prev removeFromSuperview];
				[prev release];
				PRINTF_INFO("AUAppGlueIOSKeyInput destroyed (%llu).\n", (UI64)prev);
				
			}
			if(!r && !isInputEnabled){
				r = TRUE;
			}
		}
	}
	return r;
}

bool AUAppGlueIOSKeyboard::setVisible(void* pData, const bool visible){
	bool r = false;
	if(pData != NULL){
		AUAppGlueIOSKeybData* data = (AUAppGlueIOSKeybData*)pData;
		r = AUAppGlueIOSKeyboard_updateInput_(data, visible);
	}
	return r;
}

bool AUAppGlueIOSKeyboard::restartKeyboard(void* pData){
	PRINTF_INFO("AUAppGlueIOSKeyboard::restartKeyboard.\n");
	AUAppGlueIOSKeybData* data = (AUAppGlueIOSKeybData*)pData;
	if(data->curItf != nil){
		AUAppGlueIOSKeyboard_updateInput_(data, TRUE);
	}
	return true;
}

bool AUAppGlueIOSKeyboard::restartInputPre(void* pData, const BOOL textChange, const BOOL rngSelChange, const BOOL rngMrkChange){
	PRINTF_INFO("AUAppGlueIOSKeyboard::restartInputPre.\n");
	AUAppGlueIOSKeybData* data = (AUAppGlueIOSKeybData*)pData;
	if(data->curItf != nil){
		[data->curItf notifyChangePre:textChange selChanged:rngSelChange];
	}
	return true;
}

bool AUAppGlueIOSKeyboard::restartInputPost(void* pData, const BOOL textChange, const BOOL rngSelChange, const BOOL rngMrkChange){
	PRINTF_INFO("AUAppGlueIOSKeyboard::restartInputPost.\n");
	AUAppGlueIOSKeybData* data = (AUAppGlueIOSKeybData*)pData;
	if(data->curItf != nil){
		[data->curItf notifyChangePost:textChange selChanged:rngSelChange];
	}
	return true;
}
