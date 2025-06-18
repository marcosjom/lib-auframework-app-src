package com.auframework;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.CorrectionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputContentInfo;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class AppNative {
    //
    //Fields accesed from JNI (do not rename)
    private Activity _activity = null;
    private AppNative.Delegate _delegate = null;
    private AppNative.ServiceConnectionMonitor _srvConnMon = null;
    //
    public AppNative(Activity activity, AppNative.Delegate delegate){
        _activity   = activity;
        _delegate   = delegate;
        _srvConnMon = new AppNative.ServiceConnectionMonitor();
    }
    //
    public native synchronized boolean setView(View view);
    //
    public native synchronized boolean onCreate(Bundle savedInstanceState);
    //
    public native synchronized void onDestroy();
    //
    public native synchronized void onStart();
    //
    public native synchronized void onStop();
    //
    public native synchronized void onResume();
    //
    public native synchronized void onPause();
    //
    public native synchronized boolean onNewIntent(Intent intent);
    //
    public native synchronized void onActivityResult(int request, int response, Intent data);

    //------------------------
    //- Scenes link
    //------------------------
    public static interface Delegate {
        //Configure the app
        public void appConfigurePost(long cPointerApp);
        //Configure graphics
        public void graphicsConfigurePost(long cPointerApp);
        //Create the C/C++ scene manager and return the c-pointer as a java-long.
        public long scenesManagerCreate(long cPointerApp);
        //Destroy the scene manager
        public void scenesManagerDestroy(long cPointerApp, long cPointerScenesMngr);
    }

    //------------------------
    //- Service bindgin listener
    //------------------------
    public static class ServiceConnectionMonitor implements ServiceConnection {
        @Override
        public native synchronized void onServiceConnected(ComponentName name, IBinder service);
        @Override
        public native synchronized void onServiceDisconnected(ComponentName name);
    }

    //------------------------
    //- Alarm receiver helper
    //------------------------
    public static class AlarmReceiver extends BroadcastReceiver {
        @Override
        public native synchronized void onReceive(Context context, Intent intent);
    }

    //------------------------
    //- Surface View
    //------------------------
    public static class AUSurfaceView extends GLSurfaceView implements GLSurfaceView.Renderer, View.OnFocusChangeListener, android.view.ViewTreeObserver.OnGlobalLayoutListener {
        //
        public AUSurfaceView(Context context) {
            super(context);
            Log.i("AU", "AUSurfaceView()");
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
        public native synchronized void onSurfaceCreated(GL10 gl, EGLConfig config);
        @Override
        public native synchronized void onSurfaceChanged(GL10 gl, int w, int h);
        @Override
        public native synchronized void onDrawFrame(GL10 gl);

        //---------------------------------------
        //-- Implements android.view.ViewTreeObserver.OnGlobalLayoutListener
        //---------------------------------------
        @Override
        public void onGlobalLayout(){
			/*Rect areaVisible 		= new Rect(); this.getWindowVisibleDisplayFrame(areaVisible);
			//Calcular el area no visible inferior
			final int vistaBottom 	= this.getRootView().getBottom();
			final int alturaNoVisibleInferior	= vistaBottom - areaVisible.bottom;
			//Calcular el area no visible superior
			//final int vistaTop 	= this.getRootView().getTop();
			//final int alturaNoVisibleSuperior	= areaVisible.top - vistaTop;
			//Ignorar este calculo (no es preciso)
			//int heightDiff = this.getRootView().getHeight() - (areaVisible.bottom - areaVisible.top);
			//Log.i("AU", "FOCO, onGlobalLayout, areaVisible("+areaVisible.top+", "+areaVisible.bottom+") areaVisibleRaiz("+raizTop+", "+raizBottom+") aluraRaiz("+this.getRootView().getHeight()+").");
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
					//this.tecladoOcultar();
					_nativeLink.tecladoPerdioFoco();
				}
			}
			//Log.i("AU", "FOCO, onGlobalLayout, areaNoVisibleAbajo("+alturaNoVisibleInferior+") viewHeight("+(areaVisible.bottom - areaVisible.top)+", "+areaVisible.bottom+" - "+areaVisible.top+").");
			_nativeLink.tecladoEnPantallaEstablecerAlto(alturaNoVisibleInferior);*/
        }
        @Override
        public native synchronized boolean onTouchEvent(MotionEvent event);
        @Override
        public native synchronized boolean onCheckIsTextEditor();
        @Override
        public synchronized InputConnection onCreateInputConnection(EditorInfo outAttrs){
            return this.onCreateInputConnectionWithClass(outAttrs, AppNative.AUInputConnection.class);
        }
        private native InputConnection onCreateInputConnectionWithClass(EditorInfo outAttrs, Class objClass);
        @Override
        public native synchronized boolean onKeyDown(int keyCode, KeyEvent event);
        @Override
        public native synchronized boolean onKeyUp(int keyCode, KeyEvent event);
        @Override
        public native synchronized void onWindowFocusChanged(boolean hasFocus);
        @Override
        public native synchronized void onFocusChange(View v, boolean hasFocus);
    }

    //------------------------
    //- Input connection helper
    //------------------------
    //- http://developer.android.com/reference/android/view/inputmethod/InputConnection.html
    //- An editor needs to interact with the IME,
    //- receiving commands through this InputConnection interface,
    //- and sending commands through InputMethodManager.
    //------------------------
    public static class AUInputConnection implements InputConnection {
        //
        public AUInputConnection() {
            //
        }
        //
        @Override
        public native boolean beginBatchEdit();
        @Override
        public native boolean endBatchEdit();
        @Override
        public native boolean clearMetaKeyStates(int states);
        @Override
        public native boolean commitCompletion(CompletionInfo text);
        @Override
        public native boolean commitCorrection(CorrectionInfo correctionInfo);
        @Override
        public native boolean commitText(CharSequence text, int newCursorPosition);
        @Override
        public native boolean deleteSurroundingText(int beforeLength, int afterLength);
        @Override
        public native boolean finishComposingText();
        @Override
        public native int getCursorCapsMode(int reqModes);
        @Override
        public native ExtractedText getExtractedText(ExtractedTextRequest request, int flags);
        @Override
        public native CharSequence getSelectedText(int flags);
        @Override
        public native CharSequence getTextAfterCursor(int length, int flags);
        @Override
        public native CharSequence getTextBeforeCursor(int length, int flags);
        @Override
        public native boolean performContextMenuAction(int id);
        @Override
        public native boolean performEditorAction(int actionCode);
        @Override
        public native boolean performPrivateCommand(String action, Bundle data);
        @Override
        public native boolean reportFullscreenMode(boolean enabled);
        @Override
        public native boolean requestCursorUpdates(int cursorUpdateMode);
        @Override
        public native boolean sendKeyEvent(KeyEvent event);
        @Override
        public native boolean setComposingRegion(int start, int end);
        @Override
        public native boolean setComposingText(CharSequence text, int newCursorPosition);
        @Override
        public native boolean setSelection(int start, int end);
        @Override
        public native void closeConnection();
        @Override
        public native boolean commitContent(InputContentInfo inputContentInfo, int flags, Bundle opts);
        @Override
        public native boolean deleteSurroundingTextInCodePoints(int beforeLength, int afterLength);
        @Override
        public native Handler getHandler();
    }
}
