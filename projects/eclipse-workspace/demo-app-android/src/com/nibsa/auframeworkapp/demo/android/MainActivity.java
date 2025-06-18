package com.serenehearts.android;


//import java.io.File;
//import java.io.FileInputStream;
//import java.io.FileOutputStream;
import java.util.Locale;

import com.serenehearts.android.R;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.app.Activity;
import android.content.Intent;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.WindowManager;
import android.widget.LinearLayout;

public class MainActivity extends Activity {
    //
	private AUSimpleLayout _mainLayout = null;
	private AppNative _juego		= null;
	private AUSurfaceView _vistaGL	= null;
	//Compras
	private AUBillingHelper _billingHelper = null;
	//
	private AURunnableTick _runnableTickUnSegundo 	= null;
	private Handler _manejadorMensajes 				= new Handler();
	//
	//----------------------------------
	//-- Clase de ayuda, para ejecutar tareas periodicamente
	//----------------------------------
	private class AURunnableTick implements Runnable {
		private int 	_msEspera 	= 1000;
		private Handler _manejador 	= null;
		private Runnable _ejecutar	= null;
		private boolean _finalizando = false;
		//
		public AURunnableTick(int msEspera, Handler manejador, Runnable ejecutar){
			_msEspera	= msEspera;
			_manejador 	= manejador;
			_ejecutar	= ejecutar;
		}
		//
		public void marcarFinalizando(){
			_finalizando = true;
		}
		//
		@Override
		public void run() {
			while(!_finalizando){
                _manejador.post(_ejecutar);
				try {
                    Thread.sleep(_msEspera);
                } catch (InterruptedException e) {
                	Log.e("AU", "Excepcion al dormir el AURunnableTick: " + e.toString() + ".");
                	Thread.currentThread().interrupt();
                }
			}
		}
	}
	
	//----------------------------------
	//-- MainActivity
	//----------------------------------
    @Override 
    protected void onCreate(Bundle savedInstanceState) {
    	Log.i("AU", "onCreate (inicio)");
        super.onCreate(savedInstanceState);
        //
        this.setContentView(R.layout.activity_main);
        //Intent
        Intent intent = this.getIntent();
        //Metricas de la pantalla
        DisplayMetrics displayMetrics = new DisplayMetrics();
        this.getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        //Iniciar el servicio de compras
        _billingHelper = new AUBillingHelper(this);
        _billingHelper.onCreate();
        //Configurar el AppNative
        AppNative.establecerParametros(this, getWindowManager().getDefaultDisplay().getRefreshRate(), displayMetrics.xdpi, displayMetrics.ydpi, _billingHelper);
        _juego 		= AppNative.instanciaCompartida(); //Log.i("AU", "docs: " + getFilesDir().getAbsolutePath() + " , cache: " + getCacheDir().getAbsolutePath());
        if(!_juego.inicializarMotor(this, getResources().getAssets(), getFilesDir().getAbsolutePath(), getCacheDir().getAbsolutePath(), "")){
        	Log.e("AU", "ERROR en inicializarMotor.");
        } else {
        	Log.i("AU", "InicializarMotor exitoso.");
        	_juego.activarPrioridadIdioma(Locale.getDefault().getLanguage());
        	_vistaGL 		= new AUSurfaceView(this, intent);
        	_vistaGL.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));
            //Configurar layout
            _mainLayout = new AUSimpleLayout(this);
            if(_vistaGL != null) _mainLayout.addView(_vistaGL);
            //
            //TestView vistaPrueba = new TestView(this);
            //vistaPrueba.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));
            //_mainLayout.addView(vistaPrueba);
            //
            this.setContentView(_mainLayout);
            //Asegurar que el telefono no se bloquee
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
        Log.i("AU", "onCreate (fin)"); 
    }
    
    public float frecuenciaPantalla(){
    	return getWindowManager().getDefaultDisplay().getRefreshRate();
    }
    
    @Override
    public void onDestroy() {
    	Log.i("AU", "onDestroy");
    	_billingHelper.onDestroy();
    	super.onDestroy();
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        //getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    @Override
    protected void onStart(){
    	Log.i("AU", "onStart");
    	super.onStart();
    	inicializarTimers();
    	//
    	Intent intent = this.getIntent();
    	String archivoAbrir = "";
    	Uri intentData	= intent.getData();
        if(intentData != null){
        	archivoAbrir = intentData.toString();
        	Log.i("AU", "Archivo a abrir onStart: " + archivoAbrir + ".\n");
        }
    }
    
    @Override
    protected void onStop(){
    	Log.i("AU", "onStop");
    	super.onStop(); 
    	//
    	finalizarTimers(); 
    }
    
    @Override
    protected void onResume(){
    	Log.i("AU", "onResume");
    	super.onResume();
    	if(_vistaGL != null) _vistaGL.onResume(); //TEMPORALMENTE deshabilitado (probando login con Google+)
    	inicializarTimers();
    }
    
    @Override 
    protected void onPause(){
    	Log.i("AU", "onPause");
    	super.onPause();
    	if(_vistaGL != null) _vistaGL.onPause(); //TEMPORALMENTE deshabilitado (probando login con Google+)
    	finalizarTimers();
    }
    
    @Override
    protected void onNewIntent(Intent intent){
    	Log.i("AU", "onNewIntent");
        super.onNewIntent(intent);
        _juego.analizarIntent(intent);
    }

    @Override
    protected void onActivityResult(int request, int response, Intent data) {
    	//System.out.println("onActivityResult req("+request+") resp("+response+") data("+data+")");
    	System.out.println("onActivityResult, start.");
    	System.out.println("onActivityResult, sending result to super.");
        super.onActivityResult(request, response, data);
        //Compras
        if(_billingHelper != null){
        	System.out.println("onActivityResult, sending result to billingHelper.");
        	_billingHelper.onActivityResult(request, response, data);
        }
        System.out.println("onActivityResult, end.");
    }
    
    
    /*@Override
    public void onBackPressed() {
    	Log.i("AU", "onBackPressed");
    	if(_juego != null){
    		if(!_juego.teclaBackPresionada()){
    			Log.w("AU", "Invocando super.onBackPressed().");
    			super.onBackPressed();
    		}
    	}
    }*/
	
    /*
    Movido a AUAurfaceView
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	Log.i("AU", "onKeyDown(" + keyCode + ")");
        if (keyCode == KeyEvent.KEYCODE_BACK) {
        	Log.i("AU", "onKeyDown(KEYCODE_BACK)");
        	if(_juego != null){
        		if(_juego.teclaBackPresionada()){
        			Log.i("AU", "onKeyDown(KEYCODE_BACK) consumido!");
        			return true;
        		} else {
        			Log.i("AU", "onKeyDown(KEYCODE_BACK) NO-consumido!");
        		}
        	}
        }
        return super.onKeyDown(keyCode, event);
    }*/
	
    private void inicializarTimers(){
		if(_runnableTickUnSegundo == null){
			_runnableTickUnSegundo = new AURunnableTick(1000, _manejadorMensajes, new Runnable(){
            	public void run() {
            		if(_juego != null && _vistaGL != null){ _juego.tickUnSegundo(); }
            	}
            });
			new Thread(_runnableTickUnSegundo).start();
			Log.i("AU", "_runnableTickUnSegundo creado.");
			if(_juego != null){
				_juego.appFocoObtenido();
			}
		}
	}

    private void finalizarTimers(){
		if(_runnableTickUnSegundo != null){
			_runnableTickUnSegundo.marcarFinalizando(); _runnableTickUnSegundo = null;
			Log.i("AU", "_runnableTickUnSegundo eliminado.");
			if(_juego != null){
				_juego.appFocoPerdido();
			}
		}
	}
    
    //
    
	public void abrirURLPlayStore(){
    	final String appPackageName = getPackageName(); // getPackageName() from Context or Activity object
    	try {
    	    startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=" + appPackageName)));
    	} catch (android.content.ActivityNotFoundException anfe) {
    	    startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("http://play.google.com/store/apps/details?id=" + appPackageName)));
    	}
    }
	
    //
	
    String urlConValorParaParametro(String urlOrig, String nomParam, String valParam){
    	String strIniParm = nomParam + "=";
    	//Buscar el inicio del parametro
    	int iniParam = -1;
    	do {
    		iniParam			= urlOrig.indexOf(strIniParm, iniParam + 1);
    		if(iniParam < 0){
    			break;
    		} else if(iniParam > 0){
    			char c = urlOrig.charAt(iniParam - 1);
    			if(c == '?' || c == '&'){
    				break;
    			}
    		}
    	} while(true);
    	//Buscar el fin del parametro
    	int posFinParam = -1;
    	if(iniParam > 0){
    		int posAnd = urlOrig.indexOf('&', iniParam + 1);
    		int posLoc = urlOrig.indexOf('#', iniParam + 1);
    		if(posAnd > 0) if(posFinParam == -1 || posFinParam > posAnd) posFinParam = posAnd;
    		if(posLoc > 0 ) if(posFinParam == -1 || posFinParam > posLoc) posFinParam = posLoc;
    		if(posFinParam == -1) posFinParam = urlOrig.length();
    	}
    	//Generar URL
    	if(iniParam <= 0){
    		if(urlOrig.indexOf('?') < 0){
    			return urlOrig + "?" + nomParam + "=" + valParam;
    		} else {
    			return urlOrig + "&" + nomParam + "=" + valParam;
    		}
    	} else {
    		return urlOrig.substring(0, iniParam + strIniParm.length()) + valParam + urlOrig.substring(posFinParam);
    	}
    }
    
    
    //Eventos de teclado
    public boolean tecladoVisible(){
    	if(_vistaGL != null) return _vistaGL.tecladoVisible();
    	return false;
    }

    public boolean tecladoMostrar(){
    	if(_vistaGL != null) return _vistaGL.tecladoMostrar();
    	return false;
    }

    public boolean tecladoOcultar(){
    	if(_vistaGL != null) return _vistaGL.tecladoOcultar();
    	return false;
    }
	
    
}
