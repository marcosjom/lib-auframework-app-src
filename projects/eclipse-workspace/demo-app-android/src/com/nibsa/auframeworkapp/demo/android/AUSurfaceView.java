package com.serenehearts.android;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.text.InputType;
import android.text.method.MetaKeyKeyListener;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

public class AUSurfaceView extends GLSurfaceView implements GLSurfaceView.Renderer, View.OnFocusChangeListener, android.view.ViewTreeObserver.OnGlobalLayoutListener {

	private MainActivity _activity 					= null;
	private AppNative _juego 						= null;
	//
	private Intent	_intentInicial					= null;
	private boolean _superficiePrimerPintada		= false;	//Determina si la superficie ya fue pintada por lo menos una vez
	private boolean _ventanaTieneFoco				= false;	//foco de la ventana
	private boolean _vistaTieneFoco					= false;	//foco de la Vista
	private boolean _tecladoRequerido 				= false;	//se solicita mostrar teclado
	private boolean _tecladoMostradoCmd				= false;	//estado del comando de mostrar (comando mostrar, comando ocultar)
	private boolean _tecladoMostradoVisualEsperando	= false;	//estado visual del teclado
	private boolean _tecladoMostradoVisual			= false;	//estado visual del teclado
	//
	private long metaState = 0;
	
	public AUSurfaceView(MainActivity activity, Intent intentInicial) {
		super(activity);
		Log.i("AU", "AUSurfaceView::constructor.");
		_activity 		= activity;
		_intentInicial	= intentInicial;
        _juego 			= AppNative.instanciaCompartida();
        this.setEGLContextClientVersion(1);
        //this.setEGLConfigChooser(bool needDepth);
        //this.setEGLConfigChooser(EGLConfigChooser)
        this.setEGLConfigChooser(8, 8, 8, 8, 0, 0); //R-G-B-A-depth-stencil. Default es: RGB_565.
        //this.setPreserveEGLContextOnPause(true); //Esto es dependiente de implementacion, y requiere API 11 (la minima a soportar es 9)
        this.setRenderer(this);
        this.setRenderMode(RENDERMODE_CONTINUOUSLY);
        //Captura de reclado
        this.setFocusable(true);
    	this.setFocusableInTouchMode(true);
    	this.setOnFocusChangeListener(this);
    	this.getViewTreeObserver().addOnGlobalLayoutListener(this);
    }
	
	//---------------------------------------
	//-- implements GLSurfaceView.Renderer
	//---------------------------------------
	@Override
	public synchronized void onSurfaceCreated(GL10 gl, EGLConfig config) { //synchronized
		if(!_juego.inicializarEscena()){ Log.i("AU", "AUSurfaceView::ERROR en inicializarMedia.");
		} else { Log.i("AU", "AUSurfaceView::inicializarMedia exitoso."); }
    }
	
	@Override
    public synchronized void onSurfaceChanged(GL10 gl, int w, int h) { //synchronized
		boolean exito = _juego.redimensionarEscena(w, h); 
		if(!exito) Log.e("AU", "AUSurfaceView::redimensionarEscena retorno 'false'.");
    }
	
	@Override
    public synchronized void onDrawFrame(GL10 gl) { //synchronized
    	if(_juego != null){
    		_juego.tickProducirYConumirRender();
    		//
    		if(!_superficiePrimerPintada){
    			_superficiePrimerPintada = true;
    			//Notificacion inicial recibida
    			if(_intentInicial != null){
    				_juego.analizarIntent(_intentInicial);
    			}
    		}
    	}
    }
	
	//---------------------------------------
	//-- Implements android.view.ViewTreeObserver.OnGlobalLayoutListener
	//---------------------------------------
	@Override
	public void onGlobalLayout(){
		Rect areaVisible 		= new Rect(); this.getWindowVisibleDisplayFrame(areaVisible);
		//Calcular el area no visible inferior
		final int vistaBottom 	= this.getRootView().getBottom();
		final int alturaNoVisibleInferior	= vistaBottom - areaVisible.bottom;
		//Calcular el area no visible superior
		//final int vistaTop 	= this.getRootView().getTop();
		//final int alturaNoVisibleSuperior	= areaVisible.top - vistaTop;
		//Ignorar este calculo (no es preciso)
		//int heightDiff = this.getRootView().getHeight() - (areaVisible.bottom - areaVisible.top);
		//Log.w("AU", "FOCO, onGlobalLayout, areaVisible("+areaVisible.top+", "+areaVisible.bottom+") areaVisibleRaiz("+raizTop+", "+raizBottom+") aluraRaiz("+this.getRootView().getHeight()+").");
	    if (alturaNoVisibleInferior > 0) {
	    	System.out.println("FOCO, onGlobalLayout, teclado esta visible.");
	    	_tecladoMostradoVisual = true;
	    	if(_tecladoMostradoVisualEsperando){
	    		_tecladoMostradoVisualEsperando = (_tecladoMostradoCmd != _tecladoMostradoVisual);
	    		if(!_tecladoMostradoVisualEsperando) System.out.println("FOCO, estado visual de teclado se sincronizo con estado deseado (visible).");
	    	}
	    } else {
	    	System.out.println("FOCO, onGlobalLayout, teclado esta oculto.");
	    	_tecladoMostradoVisual = false;
	    	if(_tecladoMostradoVisualEsperando){
	    		_tecladoMostradoVisualEsperando = (_tecladoMostradoCmd != _tecladoMostradoVisual);
	    		if(!_tecladoMostradoVisualEsperando) System.out.println("FOCO, estado visual de teclado se sincronizo con estado deseado (oculto).");
	    	} else if(_tecladoMostradoCmd){
	    		tecladoOcultar();
	    		_juego.tecladoPerdioFoco();
	    	}
	    }
	    Log.w("AU", "FOCO, onGlobalLayout, areaNoVisibleAbajo("+alturaNoVisibleInferior+") viewHeight("+(areaVisible.bottom - areaVisible.top)+", "+areaVisible.bottom+" - "+areaVisible.top+").");
	    _juego.tecladoEnPantallaEstablecerAlto(alturaNoVisibleInferior);
	}
	
	private void consumirFocoTeclado(){
		boolean tecladoMostrarCmd = (_ventanaTieneFoco && _vistaTieneFoco && _tecladoRequerido);
		if(_tecladoMostradoCmd != tecladoMostrarCmd){
			_tecladoMostradoCmd = tecladoMostrarCmd;
			//
			_tecladoMostradoVisualEsperando = (_tecladoMostradoCmd != _tecladoMostradoVisual);
			if(_tecladoMostradoVisualEsperando) System.out.println("FOCO, se requiere sincronizar el estado visual del teclado hacia "+(tecladoMostrarCmd ? "visible" : "oculto")+".");
			else System.out.println("FOCO, estado visual del teclado ya estaba sincronizado en "+(tecladoMostrarCmd ? "visible" : "oculto")+".");
			//
			if(tecladoMostrarCmd){
				Log.i("AU", "Mostrando teclado por FOCO.\n");
				_tecladoMostradoCmd = true;
				InputMethodManager imm = (InputMethodManager) _activity.getSystemService(Context.INPUT_METHOD_SERVICE);
				if(!imm.showSoftInput(this, 0)){ //imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0); // SHOW_IMPLICIT, SHOW_FORCED (lo mantiene abierto aun al cambiar de App)
					Log.e("AU", "InputMethodManager.showSoftInput - ERROR.\n");
				} else {
					Log.i("AU", "InputMethodManager.showSoftInput - EXITO.\n");
				}
			} else {
				Log.i("AU", "Ocultando teclado por FOCO.\n");
				_tecladoMostradoCmd = false;
				InputMethodManager imm = (InputMethodManager) _activity.getSystemService(Context.INPUT_METHOD_SERVICE);
				if(!imm.hideSoftInputFromWindow(this.getWindowToken(), 0)){
					Log.e("AU", "InputMethodManager.hideSoftInputFromWindow - ERROR.\n");
				} else {
					Log.i("AU", "InputMethodManager.hideSoftInputFromWindow - EXITO.\n");
				}
				_juego.tecladoPerdioFoco();
			}
		}
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		//int action = event.getAction();
		// get pointer index from the event object
	    int actionIndex = event.getActionIndex();
	    // get pointer ID
	    int pointerId = event.getPointerId(actionIndex);
	    // get masked (not specific to a pointer) action
	    int actionMask = event.getActionMasked();
	    //
	    //Log.i("AU", "Touch, action("+nombreEventoTouch(action)+") index("+actionIndex+") pointerId("+pointerId+") actionMask("+nombresEventosMascaraTouch(actionMask)+")");
	    switch (actionMask) {
	    	case MotionEvent.ACTION_DOWN:
	    	case MotionEvent.ACTION_POINTER_DOWN:
	    		_juego.touchIniciado(pointerId, (int)event.getX(actionIndex), (int)event.getY(actionIndex));
	    		break; 
	    	case MotionEvent.ACTION_MOVE: // a pointer was moved
	    		_juego.touchMovido(pointerId, (int)event.getX(actionIndex), (int)event.getY(actionIndex));
	    		break;
	    	case MotionEvent.ACTION_UP:
	    	case MotionEvent.ACTION_POINTER_UP:
	    	case MotionEvent.ACTION_CANCEL:
	    		_juego.touchFinalizado(pointerId, (int)event.getX(actionIndex), (int)event.getY(actionIndex), actionMask == MotionEvent.ACTION_CANCEL);
	    		break;
	    }
		return true;
	}
	
	//Captura de eventos de teclado
	/*@Override
	public boolean onCheckIsTextEditor(){ //Habilita el mostrar o no el teclado
		return true; //_tecladoPermitir;
	}*/
	
	//Captura de teclado
	public boolean onCheckIsTextEditor(){
		return false; //true
	}
	
	public InputConnection onCreateInputConnection(EditorInfo outAttrs){
		/*
		int	IME_FLAG_FORCE_ASCII	Flag of imeOptions: used to request an IME that is capable of inputting ASCII characters.
		int	IME_FLAG_NAVIGATE_NEXT	Flag of imeOptions: used to specify that there is something interesting that a forward navigation can focus on.
		int	IME_FLAG_NAVIGATE_PREVIOUS	Flag of imeOptions: like IME_FLAG_NAVIGATE_NEXT, but specifies there is something interesting that a backward navigation can focus on.
		int	IME_FLAG_NO_ACCESSORY_ACTION	Flag of imeOptions: used in conjunction with one of the actions masked by IME_MASK_ACTION, this indicates that the action should not be available as an accessory button on the right of the extracted text when the input method is full-screen.
		int	IME_FLAG_NO_ENTER_ACTION	Flag of imeOptions: used in conjunction with one of the actions masked by IME_MASK_ACTION.
		int	IME_FLAG_NO_EXTRACT_UI	Flag of imeOptions: used to specify that the IME does not need to show its extracted text UI.
		int	IME_FLAG_NO_FULLSCREEN	Flag of imeOptions: used to request that the IME never go into fullscreen mode.
		*/
		/*
		int	TYPE_CLASS_DATETIME	Class for dates and times.
		int	TYPE_CLASS_NUMBER	Class for numeric text.
		int	TYPE_CLASS_PHONE	Class for a phone number.
		int	TYPE_CLASS_TEXT	Class for normal text.
		//
		int	TYPE_TEXT_FLAG_AUTO_COMPLETE	Flag for TYPE_CLASS_TEXT: the text editor (which means the application) is performing auto-completion of the text being entered based on its own semantics, which it will present to the user as they type.
		int	TYPE_TEXT_FLAG_AUTO_CORRECT	Flag for TYPE_CLASS_TEXT: the user is entering free-form text that should have auto-correction applied to it.
		int	TYPE_TEXT_FLAG_CAP_CHARACTERS	Flag for TYPE_CLASS_TEXT: capitalize all characters.
		int	TYPE_TEXT_FLAG_CAP_SENTENCES	Flag for TYPE_CLASS_TEXT: capitalize the first character of each sentence.
		int	TYPE_TEXT_FLAG_CAP_WORDS	Flag for TYPE_CLASS_TEXT: capitalize the first character of every word.
		int	TYPE_TEXT_FLAG_IME_MULTI_LINE	Flag for TYPE_CLASS_TEXT: the regular text view associated with this should not be multi-line, but when a fullscreen input method is providing text it should use multiple lines if it can.
		int	TYPE_TEXT_FLAG_MULTI_LINE	Flag for TYPE_CLASS_TEXT: multiple lines of text can be entered into the field.
		int	TYPE_TEXT_FLAG_NO_SUGGESTIONS	Flag for TYPE_CLASS_TEXT: the input method does not need to display any dictionary-based candidates.
		//
		int	TYPE_TEXT_VARIATION_EMAIL_ADDRESS	Variation of TYPE_CLASS_TEXT: entering an e-mail address.
		int	TYPE_TEXT_VARIATION_EMAIL_SUBJECT	Variation of TYPE_CLASS_TEXT: entering the subject line of an e-mail.
		int	TYPE_TEXT_VARIATION_FILTER	Variation of TYPE_CLASS_TEXT: entering text to filter contents of a list etc.
		int	TYPE_TEXT_VARIATION_LONG_MESSAGE	Variation of TYPE_CLASS_TEXT: entering the content of a long, possibly formal message such as the body of an e-mail.
		int	TYPE_TEXT_VARIATION_NORMAL	Default variation of TYPE_CLASS_TEXT: plain old normal text.
		int	TYPE_TEXT_VARIATION_PASSWORD	Variation of TYPE_CLASS_TEXT: entering a password.
		int	TYPE_TEXT_VARIATION_PERSON_NAME	Variation of TYPE_CLASS_TEXT: entering the name of a person.
		int	TYPE_TEXT_VARIATION_PHONETIC	Variation of TYPE_CLASS_TEXT: entering text for phonetic pronunciation, such as a phonetic name field in contacts.
		int	TYPE_TEXT_VARIATION_POSTAL_ADDRESS	Variation of TYPE_CLASS_TEXT: entering a postal mailing address.
		int	TYPE_TEXT_VARIATION_SHORT_MESSAGE	Variation of TYPE_CLASS_TEXT: entering a short, possibly informal message such as an instant message or a text message.
		int	TYPE_TEXT_VARIATION_URI	Variation of TYPE_CLASS_TEXT: entering a URI.
		int	TYPE_TEXT_VARIATION_VISIBLE_PASSWORD	Variation of TYPE_CLASS_TEXT: entering a password, which should be visible to the user.
		int	TYPE_TEXT_VARIATION_WEB_EDIT_TEXT	Variation of TYPE_CLASS_TEXT: entering text inside of a web form.
		int	TYPE_TEXT_VARIATION_WEB_EMAIL_ADDRESS	Variation of TYPE_CLASS_TEXT: entering e-mail address inside of a web form.
		int	TYPE_TEXT_VARIATION_WEB_PASSWORD	Variation of TYPE_CLASS_TEXT: entering password inside of a web form.
		*/
		//outAttrs.imeOptions	= EditorInfo.IME_FLAG_NO_EXTRACT_UI;
		//outAttrs.inputType	= InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS; 
		InputMethodManager imm	= (InputMethodManager)_activity.getSystemService(Context.INPUT_METHOD_SERVICE);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) { //Api level 11
			outAttrs.imeOptions		= EditorInfo.IME_FLAG_NO_FULLSCREEN;
		}
		outAttrs.inputType 		= /*InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE | InputType.TYPE_TEXT_FLAG_AUTO_CORRECT | InputType.TYPE_TEXT_FLAG_CAP_SENTENCES*/ InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
		outAttrs.actionLabel	= "OK";
		int[] selRango			= _juego.entradaRangoSeleccion();
		if(selRango != null){
			outAttrs.initialSelStart = selRango[0];
			outAttrs.initialSelStart = selRango[0] + selRango[1];
		}
		AUInputConnection r 	= new AUInputConnection(imm, this, _juego, this, true);
		_juego.setCurrentInputConn(r);
		return r;
	}
		
	public boolean onKeyDown (int keyCode, KeyEvent event){
		long newState = MetaKeyKeyListener.handleKeyDown(metaState, keyCode, event);
		if(metaState == newState){ // Normal Key press
			int c = event.getUnicodeChar(MetaKeyKeyListener.getMetaState(newState));
			int[] test = new int[1];test[0] = c;
			Log.v("AU","Got: "+new String(test,0,1));
			metaState = MetaKeyKeyListener.adjustMetaAfterKeypress(metaState);
		} else {
			metaState = newState;
		}
		return  true;
	}
	
	public boolean onKeyUp (int keyCode, KeyEvent event){
		metaState = MetaKeyKeyListener.handleKeyUp(metaState, keyCode, event);
		Log.v("AU","Key up meta	: "+MetaKeyKeyListener.getMetaState(metaState));
		//
		if (keyCode == KeyEvent.KEYCODE_BACK) {
        	if(_juego != null){
        		if(_juego.teclaBackPresionada()){
        			Log.i("AU", "onKeyDown(KEYCODE_BACK) consumido!");
        			return true;
        		} else {
        			Log.i("AU", "onKeyDown(KEYCODE_BACK) NO-consumido!");
        			this._activity.onBackPressed();
        			return true;
        		}
        	}
        } else if(keyCode == KeyEvent.KEYCODE_MENU){
        	if(_juego != null){
        		if(_juego.teclaMenuPresionada()){
        			Log.i("AU", "onKeyDown(KEYCODE_MENU) consumido!");
        			return true;
        		} else {
        			Log.i("AU", "onKeyDown(KEYCODE_MENU) NO-consumido!");
        			//return false;
        		}
        	}
        }
		return super.onKeyUp(keyCode, event);
	}
	
	/*@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttrs){ //Crea la interaccion con el teclado (software o hardware)
		outAttrs.actionLabel = null;
	    outAttrs.label = "Test text";
	    outAttrs.inputType = InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
	    outAttrs.imeOptions = EditorInfo.IME_ACTION_DONE;
		Log.i("AU", "AUSurfaceView::onCreateInputConnection");
		_gestorTeclado = new GestorEntradaTeclado(this, true / *fullEditor* /);
		return _gestorTeclado;
	}*/
	
	/*@Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        Log.d("AU", "onCreateInputConnection");
        BaseInputConnection fic = new BaseInputConnection(this, false / *true* /);
        outAttrs.actionLabel = null;
        outAttrs.inputType = InputType.TYPE_CLASS_TEXT; //TYPE_NULL; //TYPE_CLASS_TEXT;
        outAttrs.imeOptions = EditorInfo.IME_ACTION_NEXT;
        return fic;
    }*/

    /*@Override
    public boolean onCheckIsTextEditor() {
        Log.d("AU", "onCheckIsTextEditor");
        return true;
    }*/
	
	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        _ventanaTieneFoco = hasFocus;
        if(hasFocus) {
        	Log.i("AU", "Ventana ha ADQUIRIDO EL FOCO.\n");
        	this.consumirFocoTeclado();
		} else {
			Log.i("AU", "Ventana ha PERDIDO EL FOCO.\n");
			//Nota: a veces se pierde el foco por un mensaje del teclado.
			//Si se manda a ocultar el teclado inmediatamente, el usuario no podra introducir texto al App.
			//Ademas, "consumirFocoTeclado" es inecesario debido a que el App oculta el teclado automaticamente cuando pierde el foco.
			//this.consumirFocoTeclado(); //No ejecutar esta linea
		}
    }
	
	//implements View.OnFocusChangeListener
	public void onFocusChange(View v, boolean hasFocus) {
		if(v == this){
			_vistaTieneFoco = hasFocus;
			if(hasFocus) { Log.i("AU", "Superficie ha ADQUIRIDO EL FOCO.\n");
			} else { Log.i("AU", "Superficie ha PERDIDO EL FOCO.\n"); }
			this.consumirFocoTeclado();
		} else {
			System.out.println("Vista desconocida ha cambiado FOCO("+hasFocus+").\n");
		}
    }
	
	public boolean tecladoVisible(){
		return _tecladoRequerido; //(_gestorTeclado != null);
	}
	
	public boolean tecladoMostrar(){
		_tecladoRequerido = true; System.out.println("AUSurfaceView::tecladoMostrar");
		this.requestFocus();
		this.consumirFocoTeclado();
		return true;
	}
	
	public boolean tecladoOcultar(){
		_tecladoRequerido = false; System.out.println("AUSurfaceView::tecladoOcultar");
		this.consumirFocoTeclado();
		return true;
	}
	
}
