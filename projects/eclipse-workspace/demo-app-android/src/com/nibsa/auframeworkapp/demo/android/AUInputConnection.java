package com.serenehearts.android;

import android.util.Log;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.CompletionInfo;
//import android.view.inputmethod.CorrectionInfo;
import android.view.inputmethod.ExtractedText;
import android.view.inputmethod.ExtractedTextRequest;
import android.view.inputmethod.InputMethodManager;

public class AUInputConnection extends BaseInputConnection {
	/*
	http://developer.android.com/reference/android/view/inputmethod/InputConnection.html
	An editor needs to interact with the IME,
	receiving commands through this InputConnection interface,
	and sending commands through InputMethodManager.
	*/
	private InputMethodManager _imm = null;
	private View _appView = null;
	private AppNative _appNative = null;
	private int _batchesInProgress = 0;
	//
	public static int ENTextRangeSet_None	= 0;
	public static int ENTextRangeSet_Current= 1;
	public static int ENTextRangeSet_Word	= 2;
	//
	public AUInputConnection(InputMethodManager imm, View view, AppNative appNative, View vista, boolean fullEditor) {
		super(vista, fullEditor);
		//Log.w("AUInput","InputConn: nueva.");
		_imm = imm; //(InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		_appView = view;
		_appNative = appNative;
	}
	//
	public void entradaRangosModificadosCallback(int[] rngSel, int[] rngMrc){
		if(_batchesInProgress > 0){
			////Log.w("AUInput","InputConn: entradaRangosModificadosCallback(IGNORED - batch-in-progress).");
		} else if(rngSel != null && rngMrc != null){
			////Log.w("AUInput","InputConn: entradaRangosModificadosCallback/updateSelection("+rngSel[0]+", "+(rngSel[0] + rngSel[1])+") - ("+rngMrc[0]+", "+(rngMrc[0] + rngMrc[1])+").");
			_imm.updateSelection(_appView, rngSel[0], rngSel[0] + rngSel[1], rngMrc[0], rngMrc[0] + rngMrc[1]);
			//if(GET_EXTRACTED_TEXT_MONITOR) updateExtractedText(View, int, ExtractedText);
		}
	}
	//
	@Override
	public boolean beginBatchEdit(){
		boolean r = true;
		_appNative.entradaExplicitBashPush();
		boolean rSuper = super.beginBatchEdit();
		_batchesInProgress++;
		//Log.w("AUInput","InputConn: beginBatchEdit("+_batchesInProgress+") <== "+r+" ("+(_batchesInProgress == 1 ? "first" : "nested")+") super("+rSuper+").");
		return r;
	}
	
	@Override
	public boolean endBatchEdit(){
		boolean rSuper =
				super.endBatchEdit();
		boolean r = (_batchesInProgress > 1);
		_batchesInProgress--;
		//
		String txtSuper = "";
		CharSequence before = super.getTextBeforeCursor(9999, 0);
		CharSequence after = super.getTextAfterCursor(9999, 0);
		CharSequence selection = super.getSelectedText(0);
		if(before != null) txtSuper += before.toString();
		txtSuper += "|";
		if(selection != null){
			if(selection.length() != 0) txtSuper += selection + "|";
		}
		if(after != null) txtSuper += after.toString();
		//
		String strState = "";
		int[] rngSel = _appNative.entradaRangoSeleccion(); strState += " "; if(rngSel != null) strState += "Sel("+rngSel[0]+", +"+rngSel[1]+")"; else strState += "Sel(null)";
		int[] rngMrc = _appNative.entradaRangoMarcado(); strState += " "; if(rngMrc != null) strState += "Mrc("+rngMrc[0]+", +"+rngMrc[1]+")"; else strState += "Mrc(null)";
		String contenido = _appNative.entradaContenido(); strState += " "; if(contenido != null) strState += "txt('" + contenido + "')"; else strState += "txt(null)";
		String contenidoMrc = _appNative.entradaContenidoMarcado(); strState += " "; if(contenidoMrc != null) strState += "mrc('" + contenidoMrc + "')"; else strState += "mrc(null)";
		String contenidoSel = _appNative.entradaContenidoSeleccion(); strState += " "; if(contenidoSel != null) strState += "sel('" + contenidoSel + "')"; else strState += "sel(null)";
		strState += " valSuper('" + txtSuper + "')";
		//Log.w("AUInput","InputConn: nativeState:" + strState);
		//
		//if(_batchesInProgress == 0){
			//int[] rngSel = _appNative.entradaRangoSeleccion();
			//int[] rngMrc = _appNative.entradaRangoMarcado();
			if(rngSel != null && rngMrc != null){
				////Log.w("AUInput","InputConn: updateSelection("+rngSel[0]+", "+(rngSel[0] + rngSel[1])+") - ("+rngMrc[0]+", "+(rngMrc[0] + rngMrc[1])+").");
				_imm.updateSelection(_appView, rngSel[0], rngSel[0] + rngSel[1], rngMrc[0], rngMrc[0] + rngMrc[1]);
				//if(GET_EXTRACTED_TEXT_MONITOR) updateExtractedText(View, int, ExtractedText);
			}
			//_appNative.entradaHabilitarNotificaciones(true);
		//}
		_appNative.entradaExplicitBashPop();
		//Log.w("AUInput","InputConn: endBatchEdit("+(_batchesInProgress + 1)+") <== "+r+" ("+(r ? "nested" : "last")+") super("+rSuper+").");
		return r;
	}
	
	@Override
	public boolean clearMetaKeyStates(int states){
		boolean r = super.clearMetaKeyStates(states);
		//Log.w("AUInput","InputConn: clearMetaKeyStates("+states+") <== "+r+".");
		return r;
	}
	
	@Override
	public boolean commitCompletion(CompletionInfo text){
		/*
		Editor authors: if you never call displayCompletions(CompletionInfo[]) 
		or displayCompletions(View, CompletionInfo[]) then 
		a well-behaved IME should never call this on your input connection
		, but be ready to deal with misbehaving IMEs without crashing.
		*/
		boolean r = super.commitCompletion(text);
		//Log.w("AUInput","InputConn: commitCompletion("+text+") <== "+r+".");
		return r;
	}
	
	/*@Override
	public boolean commitCorrection(CorrectionInfo correctionInfo){
		/ *
		Calling this method will cause the editor to call
		onUpdateSelection(int, int, int, int, int, int)
		on the current IME after the batch input is over.
		* /
		boolean r = super.commitCorrection(correctionInfo);
		//Log.w("AUInput","InputConn: commitCorrection("+correctionInfo+") <== "+r+".");
		return r;
	}*/
	
	@Override
	public boolean commitText(CharSequence text, int newCursorPosition){
		/*
		If there is no composing text when this method is called,
		the new text is inserted at the cursor position,
		removing text inside the selection if any.
		Calling this method will cause the editor to call
		onUpdateSelection(int, int, int, int, int, int)
		on the current IME after the batch input is over.
		This behaves like calling setComposingText(text, newCursorPosition) then finishComposingText().
		*/
		boolean r = super.commitText(text, newCursorPosition);
		//Log.w("AUInput","InputConn, commitText: '" + text + "' <== "+r+".");
		int[] rngMrc = _appNative.entradaRangoMarcado();
		if(rngMrc[1] == 0){
			_appNative.teclaInsertarTexto(text.toString());
		} else {
			_appNative.entradaContenidoMarcadoReemplazar(text.toString(), ENTextRangeSet_None);
		}
		return r;
	}
	
	@Override
	public boolean deleteSurroundingText(int beforeLength, int afterLength){
		/*
		Delete beforeLength characters of text before the current cursor position,
		and delete afterLength characters of text after the current cursor position,
		excluding the selection. 
		*/
		/*
		NOTA RELEVANTE: En algunos teclados virtuales de ANDROID
		este metodo es llamado en lugar de "onKeyDown"+"onKeyUp" para "KEYCODE_DEL".
		*/
		boolean r = super.deleteSurroundingText(beforeLength, afterLength);
		//Log.w("AUInput","InputConn, deleteSurroundingText: before("+beforeLength+") after("+afterLength+") <== "+r+".");
		_appNative.entradaTextoSeleccionEliminarAlrededor(beforeLength, afterLength);
		return r;
	}

	@Override
	public boolean finishComposingText(){
		/*
		Have the text editor finish whatever composing text is currently active.
		This simply leaves the text as-is, removing any special composing styling or other state that was around it.
		The cursor position remains unchanged.
		*/
		boolean r = super.finishComposingText();
		//Log.w("AUInput","InputConn: finishComposingText <== "+r+".");
		_appNative.entradaRangoDesmarcar();
		return r;
	}
	
	@Override
	public int getCursorCapsMode(int reqModes){
		int r = super.getCursorCapsMode(reqModes);
		//Log.w("AUInput","InputConn: getCursorCapsMode("+reqModes+") <== "+r+".");
		return r;
	}
	 
	@Override 
	public ExtractedText getExtractedText(ExtractedTextRequest request, int flags){
		/*
		Retrieve the current text in the input connection's editor,
		and monitor for any changes to it.
		Editor authors: as a general rule,
		try to comply with the fields in request for how many chars to return,
		but if performance or convenience dictates otherwise,
		please feel free to do what is most appropriate for your case.
		Also, if the GET_EXTRACTED_TEXT_MONITOR flag is set,
		you should be calling updateExtractedText(View, int, ExtractedText)
		whenever you call updateSelection(View, int, int, int, int).
		*/
		//FLAG_SELECTING = when "shift" is holded.
		//FLAG_SINGLE_LINE = the text being edited can only be on a single line.
		ExtractedText r  = new ExtractedText();
		r.flags = 0;
		r.partialStartOffset = 0;
		r.partialEndOffset = 0;
		r.startOffset = 0;
		r.selectionStart = 0; //The real selection start position is at startOffset+selectionStart.
		r.selectionEnd = 0; //The real selection end position is at startOffset+selectionEnd.
		r.text = ""; 
		//
		String entradaContenido = _appNative.entradaContenido();
		int[] rangoSel = _appNative.entradaRangoSeleccion();
		if(entradaContenido != null && rangoSel != null){
			r.text = entradaContenido;
			r.selectionStart = rangoSel[0];
			r.selectionEnd = rangoSel[0] + rangoSel[1] - 1;
		}
		//ExtractedText r = super.getExtractedText(request, flags); 
		//Log.w("AUInput","InputConn: getExtractedText("+request+", "+flags+") <== '"+r+"'.");
		return r;
	}
	
	@Override
	public CharSequence getSelectedText(int flags){
		String r = _appNative.entradaContenidoSeleccion();
		//CharSequence r = super.getSelectedText(flags); 
		//Log.w("AUInput","InputConn: getSelectedText("+flags+") <== '"+r+"'.");
		return r;
	}
	
	@Override
	public CharSequence getTextAfterCursor(int length, int flags){
		String r = _appNative.entradaContenidoDespuesSeleccion(length);
		//CharSequence r = super.getTextAfterCursor(length, flags);
		//Log.w("AUInput","InputConn: getTextAfterCursor(len:"+length+", flg:"+flags+") <== '"+r+"'.");
		return r;
	}
	
	@Override
	public CharSequence getTextBeforeCursor(int length, int flags){
		String r = _appNative.entradaContenidoAntesSeleccion(length);
		//CharSequence r = super.getTextBeforeCursor(length, flags);
		//Log.w("AUInput","InputConn: getTextBeforeCursor(len:"+length+", flg:"+flags+") <== '"+r+"'.");
		return r;
	}
	
	@Override
	public boolean performContextMenuAction(int id){
		/*
		Perform a context menu action on the field.
		The given id may be one of:
		selectAll, startSelectingText, stopSelectingText,
		cut, copy, paste, copyUrl, or switchInputMethod.
		*/
		boolean r = super.performContextMenuAction(id);
		//Log.w("AUInput","InputConn: performContextMenuAction("+id+") <== "+r+".");
		return r;
	}
	
	@Override
	public boolean performEditorAction(int actionCode){
		/*
		Have the editor perform an action it has said it can do.
		This is typically used by IMEs when the user presses the key associated with the action.
		Example: actionCode = EditorInfo.EDITOR_ACTION_GO:
		*/
		boolean r = super.performEditorAction(actionCode);
		//Log.w("AUInput","InputConn: performEditorAction("+actionCode+") <== "+r+".");
		return r;
	}
	 
	@Override
	public boolean performPrivateCommand(String action, Bundle data){
		/*
		API to send private commands from an input method to its connected editor.
		This can be used to provide domain-specific features that are only
		known between certain input methods and their clients. 
		*/
		boolean r = super.performPrivateCommand(action, data);
		//Log.w("AUInput","InputConn: performPrivateCommand("+action+", "+data+") <== "+r+".");
		return r;
	}
	
	@Override
	public boolean reportFullscreenMode(boolean enabled){
		/*
		Called by the IME to tell the client when it switches between fullscreen and normal modes.
		*/
		boolean r = super.reportFullscreenMode(enabled);
		//Log.w("AUInput","InputConn: reportFullscreenMode("+enabled+") <== "+r+".");
		return r;
	}
	
	/*@Override
	public boolean requestCursorUpdates (int cursorUpdateMode){
		/ *
		Called by the input method to ask the editor
		for calling back updateCursorAnchorInfo(android.view.View, CursorAnchorInfo)
		to notify cursor/anchor locations.
		CURSOR_UPDATE_IMMEDIATE and/or CURSOR_UPDATE_MONITOR. Pass 0 to disable the effect of CURSOR_UPDATE_MONITOR.
		//
		Returns true if the request is scheduled.
		false to indicate that when the application
		will not call updateCursorAnchorInfo(android.view.View, CursorAnchorInfo).
		* /
		boolean r = super.requestCursorUpdates(cursorUpdateMode);
		//Log.w("AUInput","InputConn: requestCursorUpdates("+cursorUpdateMode+") <== "+r+".");
		return r;
	}*/
	
	@Override
	public boolean sendKeyEvent(KeyEvent event){
		//boolean consumido = false;
		int keyAction = event.getAction();
		if(keyAction == KeyEvent.ACTION_DOWN){
			switch(event.getKeyCode()){
				/*
				NOTA RELEVANTE: En algunos teclados virtuales de ANDROID
				el metodo deleteSurroundingText(beforeLength = 1, afterLength = 0)
				es llamado en lugar de "onKeyDown"+"onKeyUp" para "KEYCODE_DEL".
				*/
				case KeyEvent.KEYCODE_DEL:
					_appNative.teclaBackspace();
					//consumido = true; //Log.w("AUInput","InputConn: KEYCODE_DEL(DOWN).");
					break;
				//case KeyEvent.KEYCODE_FORWARD_DEL: break;
			}
		}
		//if(!consumido){
		//	//Log.w("AUInput","InputConn: sendKeyEvent("+event+").");
		//}
		return super.sendKeyEvent(event);
	}
	
	@Override
	public boolean setComposingRegion(int start, int end){
		boolean r = true;
		if(start > end){ int tmp = start; start = end; end = tmp; }
		_appNative.entradaRangoMarcadoEstablecer(start, (end - start + 1));
		//boolean r = super.setComposingRegion(start, end);
		//Log.w("AUInput","InputConn: setComposingRegion("+start+", "+end+") <== "+r+".");
		return r;
	}
	
	@Override
	public boolean setComposingText(CharSequence text, int newCursorPosition){
		boolean r = true;
		_appNative.entradaContenidoMarcadoReemplazar(text.toString(), ENTextRangeSet_Current);
		//boolean r = super.setComposingText(text, newCursorPosition);
		//Log.w("AUInput","InputConn: setComposingText('"+text+"', "+newCursorPosition+") <== "+r+".");
		return r;
	}
	
	@Override
	public boolean setSelection(int start, int end){
		boolean r = true;
		if(start > end){ int tmp = start; start = end; end = tmp; }
		_appNative.entradaRangoSeleccionEstablecer(start, (end - start + 1));
		//boolean r = super.setSelection(start, end);
		//Log.w("AUInput","InputConn: setSelection("+start+", "+end+") <== "+r+".");
		return r;
	}
	
}
