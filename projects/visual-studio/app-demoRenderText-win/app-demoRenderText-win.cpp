/*
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */
//Prefix.pch
#include "AUFrameworkBaseStdAfx.h"
#include "AUFrameworkMediaStdAfx.h"
#include "AUAppNucleoPrecompilado.h"
#include "AUFrameworkBase.h"
#include "AUFrameworkMedia.h"
#include "AUAppNucleo.h"

#define USE_DEMO_TEXT_RENDER
//#define USE_DEMO_TEXTBOX

#include "AUDemosRegistro.h"
#if defined(USE_DEMO_TEXT_RENDER)
#	include "AUEscenaDemoTextRender.h"
#elif defined(USE_DEMO_TEXTBOX)
#	include "AUEscenaDemoTextBox.h"
#endif

#define AU_INICIO_PANTALLA_VERTICAL		false	//indica si la pantalla se carga en vertical u horizontal
#define AU_USA_ACELEROMETRO
#define LEER_PRECACHE					true		//Archivos binarios generados como parte del paquete
#define LEER_CACHE						false		//Archivos binarios generados durante una carga anterior y almacenados en la carpeta cache/tmp
#define ESCRIBIR_CACHE					true		//Archivos binarios generados durante una carga anterior y almacenados en la carpeta cache/tmp
#define PAQUETES_RUTA_BASE				""
#define PAQUETES_CARGAR					true
#define PAQUETES_CARGAR_PRECACHEADOS	false
//Resource.h
#define IDS_APP_TITLE				103

#define IDR_MAINFRAME				128
#define IDD_GAMETRIVIAWIN_DIALOG	102
#define IDD_ABOUTBOX				103
#define IDM_ABOUT					104
#define IDM_EXIT					105
#define IDI_GAMETRIVIAWIN			107
#define IDI_SMALL					108
#define IDC_GAMETRIVIAWIN			109
#define IDC_MYICON					2
#ifndef IDC_STATIC
#define IDC_STATIC					-1
#endif
// Valores predeterminados siguientes para nuevos objetos
//
#ifdef APSTUDIO_INVOKED
#	ifndef APSTUDIO_READONLY_SYMBOLS
#		define _APS_NO_MFC					130
#		define _APS_NEXT_RESOURCE_VALUE	129
#		define _APS_NEXT_COMMAND_VALUE		32771
#		define _APS_NEXT_CONTROL_VALUE		1000
#		define _APS_NEXT_SYMED_VALUE		110
#	endif
#endif
//targetver.h
#include <SDKDDKVer.h>
//stdafx.h
#define WIN32_LEAN_AND_MEAN             // Excluir material rara vez utilizado de encabezados de Windows
// Archivos de encabezado de Windows:
#include <windows.h>
// Archivos de encabezado en tiempo de ejecución de C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <windows.h>		// Header File For Windows
#include <Windowsx.h>		// Para las MACROS GET_X_LPARAM y GET_Y_LPARAM
#include <gl\gl.h>			// Header File For The OpenGL32 Library
//#include <gl\glu.h>		// Header File For The GLu32 Library
//#include <gl\glaux.h>		// Header File For The Glaux Library

#pragma comment (lib, "opengl32.lib")
//-//#pragma comment(lib, "glew32.lib")	//equivale a "opengl.lib"
//#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "Secur32.lib") // Para el GetUserNameEx

#define STR_ERROR_SHELL_EXECUTE(VAL) (VAL == 0 ? "OUT_OF_MEM_RESOURCES": VAL == ERROR_FILE_NOT_FOUND ? "ERROR_FILE_NOT_FOUND": VAL == ERROR_PATH_NOT_FOUND ? "ERROR_PATH_NOT_FOUND": VAL == ERROR_BAD_FORMAT ? "ERROR_BAD_FORMAT" : VAL == SE_ERR_ACCESSDENIED ? "SE_ERR_ACCESSDENIED" : VAL == SE_ERR_ASSOCINCOMPLETE ? "SE_ERR_ASSOCINCOMPLETE" : VAL == SE_ERR_DDEBUSY ? "SE_ERR_DDEBUSY": VAL == SE_ERR_DDEFAIL ? "SE_ERR_DDEFAIL" : VAL == SE_ERR_DDETIMEOUT ? "SE_ERR_DDETIMEOUT" : VAL == SE_ERR_DLLNOTFOUND ? "SE_ERR_DLLNOTFOUND" : VAL == SE_ERR_FNF ? "SE_ERR_FNF" : VAL == SE_ERR_NOASSOC ? "SE_ERR_NOASSOC": VAL == SE_ERR_OOM ? "SE_ERR_OOM" : VAL == SE_ERR_PNF ? "SE_ERR_PNF" : VAL == SE_ERR_SHARE ? "SE_ERR_SHARE" : "ERR_???")

#define LEER_PRECACHE					true
#define LEER_CACHE						false
#define ESCRIBIR_CACHE					true
#define PAQUETES_RUTA_BASE				"paquetes/"
#define PAQUETES_CARGAR					true
#define PAQUETES_CARGAR_PRECACHEADOS	false

#define AU_INICIO_PANTALLA_VERTICAL		false //indica si la pantalla se carga en vertical u horizontal
#define AU_HIGH_DEFINITION				true
#define AU_ANCHO_PANTALLA				640		//210px=tamano iPhone en monitor LED AOC
#define AU_ALTO_PANTALLA				1136		//960		//300px=tamano iPhone en monitor LED AOC

#define AU_ID_TIMER_RENDER				1
//Juego
AUApp* _app								= NULL;
AUAppEscenasAdminSimple* _scenes		= NULL;
AUCadenaMutable8* _strRutaArchivoAbrir	= NULL;
bool	_shutingDown					= false;
bool	_minimized						= false;
//Estado de entradas
bool	_touchIsDown					= false;
UINT_PTR _timerRender					= NULL;
//
#define NB_NOM_MOSTRAR_TAM_MAX			1024
char _usuIdMostrar[NB_NOM_MOSTRAR_TAM_MAX];
char _usuNomMostrar[NB_NOM_MOSTRAR_TAM_MAX];

//
HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=FALSE;	// Fullscreen Flag Set To Fullscreen Mode By Default

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

GLvoid ReSizeGLScene(GLsizei width, GLsizei height){	// Resize And Initialize The GL Window
	_minimized = (width <= 0 || height <= 0);
	if (_app != NULL && !_minimized) {
		NBTamanoI wSize; wSize.ancho = width; wSize.alto = height;
		NBTamano ppiScreen; ppiScreen.ancho = 72.0f; ppiScreen.alto = 72.0f;
		_app->notificarRedimensionVentana(wSize, 1.0f, ppiScreen, ppiScreen);
	}
}

int InitGL(int anchoVentana, int altoVentana){			// All Setup For OpenGL Goes Here
	if (GLEW_OK != glewInit()){
		PRINTF_ERROR("GLEW, no inicializado (para las extensiones GL en Windows)\n"); NBASSERT(false);
	} else {
		PRINTF_INFO("GLEW Inicializado (para las extensiones GL en Windows)\n");
		if(!_app->inicializarMultimedia(LEER_PRECACHE, LEER_CACHE, ESCRIBIR_CACHE, true /*initGraphics*/, 60)){
			PRINTF_ERROR("No se pudo inicializar el motor grafico ALA_NICA\n"); NBASSERT(false)
		} else {
			NBTamanoI wSize; wSize.ancho = anchoVentana; wSize.alto = altoVentana;
			NBTamano ppiScreen; ppiScreen.ancho = 72.0f; ppiScreen.alto = 72.0f;
			if(!_app->inicializarVentana(wSize, ppiScreen, ppiScreen, ENGestorEscenaDestinoGl_Heredado)){
				PRINTF_ERROR("No se pudo inicializar la ventana ALA_NICA\n"); NBASSERT(false)
			} else {
				_scenes = new AUAppEscenasAdminSimple(_app->indiceEscenaRender(), ENGestorTexturaModo_cargaInmediata, "", NULL, 0);
				if(!_app->inicializarJuego(_scenes)){
					PRINTF_ERROR("No se pudo inicializar el juego ALA_NICA\n"); NBASSERT(false)
				} else {
					PRINTF_INFO("Juego iniciado!\n");
#					if defined(USE_DEMO_TEXT_RENDER)
					{
						AUEscenaDemoTextRender* demoTextRender = new AUEscenaDemoTextRender(_app->indiceEscenaRender());
						_scenes->escenaCargar(demoTextRender);
						demoTextRender->liberar(NB_RETENEDOR_THIS);
					}
#					elif defined(USE_DEMO_TEXTBOX)
					{
						AUEscenaDemoTextBox* demoTextBox = new AUEscenaDemoTextBox(_app->indiceEscenaRender());
						_scenes->escenaCargar(demoTextBox);
						demoTextBox->liberar(NB_RETENEDOR_THIS);
					}
#					endif
					return TRUE;// Initialization Went OK
				}
			}
		}
	}
	return FALSE;
}

GLvoid KillGLWindow(GLvoid){							// Properly Kill The Window
	if (fullscreen){									// Are We In Fullscreen Mode?
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}
	if (hRC){											// Do We Have A Rendering Context?
		if (!wglMakeCurrent(NULL,NULL)){				// Are We Able To Release The DC And RC Contexts?
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		if (!wglDeleteContext(hRC)){					// Are We Able To Delete The RC?
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}
	if (hDC && !ReleaseDC(hWnd,hDC)){					// Are We Able To Release The DC
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}
	if (hWnd && !DestroyWindow(hWnd)){					// Are We Able To Destroy The Window?
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}
	if (!UnregisterClass("OpenGL",hInstance)){			// Are We Able To Unregister Class
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag){
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASSEX	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMETRIVIAWIN));			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name
	wc.hIconSm			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAMETRIVIAWIN));

	if (!RegisterClassEx(&wc)){									// Attempt To Register The Window Class
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen){											// Attempt Fullscreen Mode?
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL){
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES){
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			} else {
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen){											// Are We Still In Fullscreen Mode?
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	} else {
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	//Habilitar el arrastre de archivos a la ventana
	DragAcceptFiles(hWnd, TRUE);

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd))){						// Did We Get A Device Context?
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC, &pfd))){// Did Windows Find A Matching Pixel Format?
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd)){		// Are We Able To Set The Pixel Format?
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC))){				// Are We Able To Get A Rendering Context?
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC)){					// Try To Activate The Rendering Context
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	if (!InitGL(width, height)){									// Initialize Our Newly Created GL Window
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	} else {
		//ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg){									// Check For Windows Messages
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam)){					// Check Minimization State
				active = TRUE;						// Program Is Active
			} else {
				active = FALSE;						// Program Is No Longer Active
			}
			return 0;								// Return To The Message Loop
		}
		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam){						// Check System Calls
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}
		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}
		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}
		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}
		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	}
	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void volcarBuffer(void* param, AUApp* app, SI32 iEscenaRender){
	SwapBuffers(*((HDC*)param));				// Swap Buffers (Double Buffering)
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;	// Windows Message Structure
	_usuIdMostrar[0] = '\0';
	_usuNomMostrar[0] = '\0';
	//------------------------
	//--- Invocar inicializadores
	//--- (solo en VisualStudio, en GCC son autoinvocados)
	//------------------------
	#ifdef _MSC_VER
		AUApp_inicializarMemoria();
		//
		AUFrameworkBaseInicializar();
		AUFrameworkMediaRegistrar();
		AUAppNucleoRegistrar();
		AUDemosRegistrar();
	#endif
	//------------------------
	//--- Imprimir lista de parametros (informativo)
	//------------------------
	//int iParam; for(iParam=0; iParam<argc; iParam++) PRINTF_INFO("Parametro exe %d: '%s'\n", iParam, argv[iParam]);
	//------------------------
	//--- Identificar endianes del procesador actual
	//------------------------
	UI16 datoPruebaEndianes = 1; UI8* valoresBytes = (UI8*)&datoPruebaEndianes;
	PRINTF_INFO("El dispositivo es %s-ENDIEN (%d bytes por puntero)\n", (valoresBytes[0]==0 && valoresBytes[1]!=0)?"BIG":"LITTLE", (SI32)sizeof(void*));
	//------------------------
	//--- Ejecutar programa
	//------------------------
	if(!AUApp::inicializarNucleo(AUAPP_BIT_MODULO_RED)){
		PRINTF_ERROR("No se pudo inicializar el nucleo\n");
		return 1;
	} else {
		STAppCallbacks appCallbacks;
		AUApp::inicializarCallbacks(&appCallbacks);
		appCallbacks.funcVolcarBuffer		= volcarBuffer;
		appCallbacks.funcVolcarBufferParam	= &hDC;
		//
		_app = new AUApp(&appCallbacks, "AUTestRenderText", false /*permitirActividadRedEnBG*/);
		// Ask The User Which Screen Mode They Prefer
		/*if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO){
			fullscreen = FALSE;	// FullScreen Mode
		} else {
			fullscreen = TRUE;	// Windowed Mode
		}*/
		//HABILITAR LA IMPRESION DE MENSAJES PRINTF
		#ifndef CONFIG_NB_DESHABILITAR_IMPRESIONES_PRINTF
		AllocConsole();
		freopen("CONIN$", "r",stdin);
		freopen("CONOUT$", "w",stdout);
		freopen("CONOUT$", "w",stderr);
		#endif
		// Create Our OpenGL Window
		if (!CreateGLWindow("Aula Trivia", AU_ALTO_PANTALLA , AU_ANCHO_PANTALLA, 16, fullscreen)){
			return 0;									// Quit If Window Was Not Created
		}
		//
		_touchIsDown	= false;
		//Crear timer de renderizado
		_timerRender = SetTimer(hWnd, AU_ID_TIMER_RENDER, 0, NULL);
		//
		while(!_shutingDown && GetMessage(&msg, NULL, 0, 0) > 0){
			if(msg.message == WM_QUIT){
				_shutingDown = true;
			} else {
				switch(msg.message){
					case WM_LBUTTONDOWN:
					case WM_MBUTTONDOWN:
					case WM_RBUTTONDOWN:
						{
							int x = GET_X_LPARAM(msg.lParam);
							int y = GET_Y_LPARAM(msg.lParam);
							//PRINTF_INFO("BUTTON-DOWN x(%d) y(%d)\n", x, y);
							if(_touchIsDown){
								_app->touchFinalizado(1, x, y, false);
							}
							_app->touchIniciado(1, x, y);
							_touchIsDown = true;
						}
						break;
					case WM_LBUTTONUP:
					case WM_MBUTTONUP:
					case WM_RBUTTONUP:
						{
							int x = GET_X_LPARAM(msg.lParam);
							int y = GET_Y_LPARAM(msg.lParam);
							//PRINTF_INFO("BUTTON-UP x(%d) y(%d)\n", x, y);
							if(_touchIsDown){
								_app->touchFinalizado(1, x, y, false);
								_touchIsDown = false;
							}
						}
						break;
					case WM_MOUSEMOVE:
						{
							//_shutingDown = true;
							int x = GET_X_LPARAM(msg.lParam);
							int y = GET_Y_LPARAM(msg.lParam);
							//PRINTF_INFO("MOUSEMOVE x(%d) y(%d).\n", x, y);
							if(_touchIsDown){
								_app->touchMovido(1, x, y);
							}
						}
						break;
					case WM_MOUSELEAVE:
						break;
					case WM_TIMER:
						if(msg.wParam == AU_ID_TIMER_RENDER && !_minimized){
							const float msSobrantesDeCiclo = _app->tickJuego(ENAUAppTickTipo_TimerManual, false); //PRINTF_INFO("msSobranteDeCiclo(%f).\n", msSobrantesDeCiclo);
							const float msEsperar			= msSobrantesDeCiclo - ((1000.0f / 60.0f) * 0.05f);
							SetTimer(hWnd, AU_ID_TIMER_RENDER, (msEsperar > 0 ? msEsperar : 0), NULL);
							if (keys[VK_ESCAPE]) {				// Was ESC Pressed?
								_shutingDown = true;				// ESC Signalled A Quit
							}
						}
						break;
					case WM_DROPFILES:
						{
							HDROP hDrop = (HDROP)msg.wParam;
							const UINT cantArchivos = DragQueryFile ( hDrop, -1, NULL, 0 );
							if(cantArchivos > 0){
								const UINT tamStrRuta = DragQueryFile ( hDrop, 0, NULL, 0);
								if(tamStrRuta > 0){
									char* strRuta = (char*)malloc( tamStrRuta + 1);
									if(DragQueryFile ( hDrop, 0, strRuta, tamStrRuta + 1) > 0){
										strRuta[tamStrRuta] = '\0';
										if(_strRutaArchivoAbrir == NULL){
											_strRutaArchivoAbrir = new AUCadenaMutable8(strRuta);
										} else {
											_strRutaArchivoAbrir->establecer(strRuta);
											PRINTF_INFO("Se ha programado cargar el archivo: '%s'.\n", strRuta);
										}
									}
									free(strRuta);
								}
							}
							DragFinish ( hDrop );
						}
						break;
					default:
						break;
				}
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		// Shutdown
		KillTimer(hWnd, AU_ID_TIMER_RENDER);
		KillGLWindow();									// Kill The Window
		return (msg.wParam);							// Exit The Program
	}
	return 0;
}
