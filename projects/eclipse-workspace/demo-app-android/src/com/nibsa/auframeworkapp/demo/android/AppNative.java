package com.serenehearts.android;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class AppNative {

	static {
		System.loadLibrary("sereneh");
	}
	//
	private static AppNative _instancia		= null;
	private static MainActivity  _activity 	= null;
	private static boolean _activityOnFocus	= false;
	private static AUInputConnection _currentInputConn = null;
	private static AUBillingHelper _billingHelper = null;
	private static float _displayXDpi 		= 72.0f;
	private static float _displayYDpi 		= 72.0f;
	private static float _displayFreq 		= 0.0f;
	
	//Metodos estaticos
	
	public static AppNative instanciaCompartida(){
		if(_instancia == null) _instancia = new AppNative();
		return _instancia;
	}
	
	public static void establecerParametros(MainActivity activity, float displayFreq, float displayXDpi, float displayYDpi, AUBillingHelper billingHelper){
		_activity		= activity;
		_billingHelper	= billingHelper;
		_displayFreq 	= displayFreq;
		_displayXDpi	= displayXDpi;
		_displayYDpi	= displayYDpi;
	}
	
	//Metodos de instancia
	
	private AppNative(){
		//
	}
	
	//
	
	public synchronized boolean inicializarMotor(Object context, Object assetsMgr, String docsPath, String cachePath, String archivoAbrir){
		return nativeInicializarMotor(context, assetsMgr, docsPath, cachePath, archivoAbrir);
	}
	
	public synchronized boolean inicializarEscena(){
		return nativeInicializarEscena(_displayFreq, _displayXDpi, _displayYDpi);
	}
	
	public synchronized boolean redimensionarEscena(int ancho, int alto){
		return nativeRedimensionarEscena(ancho, alto);
	}
	
	public synchronized void tickUnSegundo(){
		nativeTickUnSegundo();
		//if(_activity != null) System.out.println("Frcuencia pantalla: " + _activity.frecuenciaPantalla());
	}
	
	public synchronized boolean tickProducirYConumirRender(){
		return nativeTickProducirYConumirRender();
	}
	
	public void	activarPrioridadIdioma(String idIdioma){
		nativeActivarPrioridadIdioma(idIdioma);
	}
	
    public String dameIdJugadorLocal(){
    	String strId = "";
    	return strId;
    }
    
    public String dameNombreJugadorLocal(){
    	String strNom = "";
    	return strNom;
    }
    
    public boolean appTieneFoco(){
    	return _activityOnFocus;
    }
    
    public void appFocoObtenido(){
    	_activityOnFocus = true;
    	this.nativeAppFocoObtenido();
    	//Quitar el numero en el icono de App
    	//BadgeUtils.clearBadge(_activity);
    	//Asegurar que las notificaciones se carguen
    	///*NotificacionesAdmin notifAdmin = */NotificacionesAdmin.instanciaCompartida(_activity);
    }
  
    public void appFocoPerdido(){
    	this.nativeAppFocoPerdido();
    	_activityOnFocus = false;
    	//
    	//NotificacionesAdmin notifAdmin = NotificacionesAdmin.instanciaCompartida(_activity);
    	//notifAdmin.guardarDatos(_activity);
    }
    
    //
    
    public void abrirURLPlayStore(){
    	if(_activity != null) _activity.abrirURLPlayStore();
    }
    
    //
    //Consultar inventario de productos
    //
    
    public void storeAddProductSKU(String skuProducto){
    	if(_billingHelper != null){
    		_billingHelper.addProductSKU(skuProducto);
    	}
    }
    
    public boolean storeUpdateState(){
    	boolean r = false;
    	if(_billingHelper != null){
    		r = _billingHelper.updateStoreState();
    		//if(r == false) Log.e("AU", "Sin accesibilidad a producto (!billingInventarioConsultaExitosa).");
    	}
    	return r;
    }
    
    public int storeCurState(){
    	int r = -1;
    	if(_billingHelper != null){
    		r = _billingHelper.curStoreState();
    	}
 		return r;
 	}
    
    public String storeGetProductPropsName(String skuProducto){
    	if(_billingHelper != null){
    		return _billingHelper.inventoryGetProductName(skuProducto);
    	}
    	return "";
    }
    
    public String storeGetProductPropsDescription(String skuProducto){
    	if(_billingHelper != null){
    		return _billingHelper.inventoryGetProductDescription(skuProducto);
    	}
    	return "";
    }
    
    public String storeGetProductPropsPrice(String skuProducto){
    	if(_billingHelper != null){
    		return _billingHelper.inventoryGetProductPriceFormated(skuProducto);
    	}
    	return "";
    }
    
    public boolean storeGetProductPropsIsPurchased(String skuProducto){
    	if(_billingHelper != null){
    		return _billingHelper.inventoryGetProductIsPurchased(skuProducto);
    	}
    	return false;
    }
    
    //
    // Proceso de compra
    //
    public boolean storePurchaseProduct(final String skuProducto){
    	if(_billingHelper != null){
    		return _billingHelper.purchaseProduct(skuProducto);
    	}
    	return false;
    }
    
    public boolean storeRestorePurchases(){
    	if(_billingHelper != null){
    		return _billingHelper.restorePurchases();
    	}
    	return false;
    }
    
    public int storeCurActionState(final String skuProducto){
    	if(_billingHelper != null){
    		return _billingHelper.curActionState();
    	}
    	return -1;
    }
    
    //
    //
	
	public void touchIniciado(int identificadorEnOS, int posX, int posY){
		nativeTouchIniciado(identificadorEnOS, posX, posY);
	}
	
	public void touchMovido(int identificadorEnOS, int posX, int posY){
		nativeTouchMovido(identificadorEnOS, posX, posY);
	}
	
	public void touchFinalizado(int identificadorEnOS, int posX, int posY, boolean cancelado){
		nativeTouchFinalizado(identificadorEnOS, posX, posY, cancelado);
	}
	
	public void tecladoPerdioFoco(){
		nativeTecladoPerdioFoco();
	}
	
	//
	
	public AUInputConnection currentInputConn(){
		return _currentInputConn;
	}
	
	public void setCurrentInputConn(AUInputConnection conn){
		_currentInputConn = conn;
	}
	
	public void tecladoEnPantallaEstablecerAlto(float altoTeclado){
		nativeTecladoEnPantallaEstablecerAlto(altoTeclado);
	}
	
	public boolean teclaBackPresionada(){ //Tecla Volver de Android
		return nativeTeclaBackPresionada();
	}
	
	public boolean teclaBackspace(){
		return nativeTeclaBackspace();
	}
	
	public boolean teclaInsertarTexto(String pTexto){
		return nativeTeclaInsertarTexto(pTexto);
	}
	
	public boolean teclaMenuPresionada(){
		return nativeTeclaMenuPresionada();
	}
	
	//
	
	public void entradaExplicitBashPush(){
		this.nativeEntradaExplicitBashPush();
	}
	
	public void entradaExplicitBashPop(){
		this.nativeEntradaExplicitBashPop();
	}
	
	//
	
	public int[] entradaRangoSeleccion(){ //int[2] = [ini, tamano];
		return nativeEntradaRangoSeleccion();
	}
	
	public void entradaRangoSeleccionEstablecer(int primerCharDef, int conteoCharDefs){
		nativeEntradaRangoSeleccionEstablecer(primerCharDef, conteoCharDefs);
	}
	
	public int[] entradaRangoMarcado(){ //int[2] = [ini, tamano];
		return nativeEntradaRangoMarcado();
	}
	
	public void entradaRangoMarcadoEstablecer(int primerCharDef, int conteoCharDefs){
		nativeEntradaRangoMarcadoEstablecer(primerCharDef, conteoCharDefs);
	}
	
	public void entradaRangoDesmarcar(){
		nativeEntradaRangoDesmarcar();
	}
	
	public void entradaRangosModificadosCallback(int[] rngSel, int[] rngMrc){
		if(_currentInputConn != null){
			_currentInputConn.entradaRangosModificadosCallback(rngSel, rngMrc);
		}
	}
	
	//
	
	public String entradaContenido(){
		return nativeEntradaContenido();	
	}
	
	public String entradaContenidoMarcado(){
		return nativeEntradaContenidoMarcado();
	}
	
	public void entradaContenidoMarcadoReemplazar(String valor, int setNewComposingAs){
		nativeEntradaContenidoMarcadoReemplazar(valor, setNewComposingAs);
	}
	
	public String entradaContenidoSeleccion(){
		return nativeEntradaContenidoSeleccion();	
	}
	
	public String entradaContenidoAntesSeleccion(int conteoCharDefs){	
		return nativeEntradaContenidoAntesSeleccion(conteoCharDefs);
	}
	
	public String entradaContenidoDespuesSeleccion(int conteoCharDefs){
		return nativeEntradaContenidoDespuesSeleccion(conteoCharDefs);
	}
	
	public void entradaTextoSeleccionEliminarAlrededor(int conteoCharDefsAntes, int conteoCharDefsDespues){
		nativeEntradaTextoSeleccionEliminarAlrededor(conteoCharDefsAntes, conteoCharDefsDespues);
	}
	
	//
	
	public boolean tecladoVisible(){
		if(_activity != null) return _activity.tecladoVisible();
		return false;
    }

    public boolean tecladoMostrar(){
    	if(_activity != null) return _activity.tecladoMostrar();
    	return false;
    }

    public boolean tecladoOcultar(){
    	if(_activity != null) return _activity.tecladoOcultar();
    	return false;
    }
    
    //
    
    public void copy(File src, File dst) throws IOException {
        InputStream in = new FileInputStream(src);
        OutputStream out = new FileOutputStream(dst);
        // Transfer bytes from in to out
        byte[] buf = new byte[1024];
        int len;
        while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
        }
        in.close();
        out.close();
    }
    
    //Notificaciones
    public boolean notificacionLocalHabilitar(){
    	return true;
    }
    
    public boolean notificacionLocalAgregar(String grupo, int idInterno, String titulo, String contenido, String objTipo, int objId, int segsDesdeAhora){
    	boolean r = false;
    	//NotificacionesAdmin notifAdmin = NotificacionesAdmin.instanciaCompartida(_activity);
		//r = notifAdmin.programarNotificacion(_activity, grupo, idInterno, titulo, contenido, objTipo, objId, segsDesdeAhora);
    	return r;
    }

    public boolean notificacionLocalCancelar(String grupo, int idInterno) {
    	//NotificacionesAdmin notifAdmin = NotificacionesAdmin.instanciaCompartida(_activity);
		//notifAdmin.cancelarNotificacion(_activity, grupo, idInterno);
    	//return true;
    	return false;
    }
    
    public boolean notificacionLocalCancelarGrupo(String grupo) {
    	//NotificacionesAdmin notifAdmin = NotificacionesAdmin.instanciaCompartida(_activity);
		//notifAdmin.cancelarNotificacionesDeGrupo(_activity, grupo);
    	//return true;
    	return false;
    }
    
    public boolean notificacionLocalCancelarTodas() {
    	//NotificacionesAdmin notifAdmin = NotificacionesAdmin.instanciaCompartida(_activity);
		//notifAdmin.cancelarNotificacionesTodas(_activity);
    	//return true;
    	return false;
    }
    
    public void analizarIntent(Intent intent){
    	if(intent != null){
        	Bundle extras = intent.getExtras();
        	if(extras != null){
        		String grupo = extras.getString("grp");
        		int idInterno = extras.getInt("lid");
        		String objTip = extras.getString("objTip");
        		int objId = extras.getInt("objId");
        		if(grupo != null && objTip != null){
        			Log.w("AU", "Invocando Intent... '" + grupo + "':" + idInterno+" '"+objTip+"':"+objId);
        			this.notificacionLocalRecibida(grupo, idInterno, objTip, objId);
        		}
        	}
        }
    }
    
    public void notificacionLocalRecibida(String grp, int localId, String objTip, int objId){
    	this.nativeNotificacionLocalRecibida(grp, localId, objTip, objId);
    }
    
	//
	
	private native boolean	nativeInicializarMotor(Object context, Object assetsMgr, String docsPath, String cachePath, String archivoAbrir);
	private native boolean	nativeInicializarEscena(float freqPantalla, float displayXDpi, float displayYDpi);
	private native boolean	nativeRedimensionarEscena(int ancho, int alto);
	private native void		nativeTickUnSegundo();
	private native boolean	nativeTickProducirYConumirRender();
    private native void		nativeReproducirArchivo(String rutaArchivo);
    private native void		nativeAppFocoObtenido();
    private native void		nativeAppFocoPerdido();
    //
    private native void		nativeEnviarArchivo(String rutaArchivo);
    //
    public native void		nativeTouchIniciado(int identificadorEnOS, int posX, int posY);
	public native void		nativeTouchMovido(int identificadorEnOS, int posX, int posY);
	public native void		nativeTouchFinalizado(int identificadorEnOS, int posX, int posY, boolean cancelado);
	//
	public native void		nativeActivarPrioridadIdioma(String idIdioma);
	//
	private native void 	nativeTecladoPerdioFoco();
	private native void		nativeTecladoEnPantallaEstablecerAlto(float altoTeclado);
	private native boolean	nativeTeclaBackPresionada(); //Tecla volver de Android
	private native boolean	nativeTeclaBackspace();
	private native boolean	nativeTeclaInsertarTexto(String pTexto);
	private native boolean	nativeTeclaMenuPresionada();
	//
	private native void		nativeEntradaExplicitBashPush();
	private native void		nativeEntradaExplicitBashPop();
	//
	private native int[]	nativeEntradaRangoSeleccion(); //int[2] = [ini, tamano];
	private native void		nativeEntradaRangoSeleccionEstablecer(int primerCharDef, int conteoCharDefs);
	private native int[]	nativeEntradaRangoMarcado(); //int[2] = [ini, tamano];
	private native void		nativeEntradaRangoMarcadoEstablecer(int primerCharDef, int conteoCharDefs);
	private native void		nativeEntradaRangoDesmarcar();
	//
	private native String	nativeEntradaContenido();
	private native String	nativeEntradaContenidoMarcado();
	private native void		nativeEntradaContenidoMarcadoReemplazar(String valor, int setNewComposingAs);
	private native String	nativeEntradaContenidoSeleccion();
	private native String	nativeEntradaContenidoAntesSeleccion(int conteoCharDefs);
	private native String	nativeEntradaContenidoDespuesSeleccion(int conteoCharDefs);
	private native void		nativeEntradaTextoSeleccionEliminarAlrededor(int conteoCharDefsAntes, int conteoCharDefsDespues);
	//
	private native void		nativeNotificacionLocalRecibida(String grp, int localId, String objTip, int objId);
}
