//
//  AUApp.cpp
//  AUAppNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#include "AUAppNucleoPrecompilado.h"
#include "AUAppGlueIOSKeyInput.h"
//
#include "NBRango.h"
#include "NBGestorPilaLlamadas.h"
#include "NBGestorTeclas.h"
//

//General notes about obj-c files (*.m, *.mm):
// @autoreleasepool {...} are important or releasing objetcs and memory.

//-----------------
// <UITextInput> pos and range definitions
//-----------------

@interface AUTextPosition : UITextPosition {
	SI32 _pos;
}
-(id) initWithPos:(SI32) pos;
-(void) dealloc;
-(SI32) pos;
@end

@interface AUTextRange : UITextRange {
	AUTextPosition* _start;
	AUTextPosition* _end;
	BOOL _empty;
}
-(id) initWithRange:(NBRangoI)rango;
-(id) initWithAUTextPositions:(AUTextPosition*)posStart posEnd:(AUTextPosition*)posEnd;
-(void) dealloc;
-(AUTextPosition*) start;
-(AUTextPosition*) end;
-(BOOL) isEmpty;
@end

@implementation AUTextPosition
-(id) initWithPos:(SI32) pos {
	_pos = pos;
	return self;
}
-(void) dealloc {
	[super dealloc];
}
-(SI32) pos { return _pos; }
@end

@implementation AUTextRange
-(id) initWithRange:(NBRangoI)rango {
	_empty	= (rango.tamano <= 0 ? YES : NO);
	_start	= [[AUTextPosition alloc] initWithPos:rango.inicio];
	_end	= [[AUTextPosition alloc] initWithPos:(_empty ? rango.inicio : (rango.inicio + rango.tamano - 1))];
	return self;
}
-(id) initWithAUTextPositions:(AUTextPosition*)posStart posEnd:(AUTextPosition*)posEnd {
	_empty	= (posStart.pos > posEnd.pos ? YES : NO);
	_start	= [[AUTextPosition alloc] initWithPos:posStart.pos];
	_end	= [[AUTextPosition alloc] initWithPos:(_empty ? posStart.pos : posEnd.pos)];
	return self;
}
-(void) dealloc {
	if(_start != nil) [_start release]; _start = nil;
	if(_end != nil) [_end release]; _end = nil;
	[super dealloc];
}
-(AUTextPosition*) start { return _start; }
-(AUTextPosition*) end { return _end; }
-(BOOL) isEmpty { return _empty; }
@end

//----------------
// Keyboard input
//----------------

@implementation AUAppGlueIOSKeyInput

- (id) initWithVisualView:(UIView*)visualView {
	@autoreleasepool {
		const CGRect bounds	= [visualView bounds];
		[super initWithFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.height)];
		self.autoresizingMask		= (UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleBottomMargin);
		self.multipleTouchEnabled	= visualView.multipleTouchEnabled;
		self.contentScaleFactor		= visualView.contentScaleFactor;
		_visualView			= visualView; if(visualView != nil) [visualView retain];
		_inputDelegate		= nil;
		_inputWriteDir		= UITextWritingDirectionLeftToRight;
		_markedTextStyle	= nil;
		_tokenizer			= [[UITextInputStringTokenizer alloc] initWithTextInput:self];
	}
	return self;
}

- (void) dealloc {
	AU_GESTOR_PILA_LLAMADAS_PUSH_3("AUAppGlueIOSKeyInput::dealloc")
	PRINTF_INFO("AUAppGlueIOSKeyInput::dealloc.\n");
	@autoreleasepool {
		if(_inputDelegate != nil) [_inputDelegate release]; _inputDelegate = nil;
		if(_visualView != nil) [_visualView release]; _visualView = nil;
		if(_markedTextStyle != nil) [_markedTextStyle release]; _markedTextStyle = nil;
		if(_tokenizer != nil) [_tokenizer release]; _tokenizer = nil;
	}
	//
	[super dealloc];
	AU_GESTOR_PILA_LLAMADAS_POP_3
}

-(void) notifyChangePre:(BOOL)textChanged selChanged:(BOOL)selChanged {
	PRINTF_INFO("<<<AUAppGlueIOSKeyInput>>> notifyChangePre(%s%s).\n", (textChanged ? " textChanged" : ""), (selChanged ? " selChanged" : ""));
	if(_inputDelegate != nil){
		if(textChanged){
			[_inputDelegate textWillChange:self];
		}
		if(selChanged){
			[_inputDelegate selectionWillChange:self];
		}
	}
}

-(void) notifyChangePost:(BOOL)textChanged selChanged:(BOOL)selChanged {
	PRINTF_INFO("<<<AUAppGlueIOSKeyInput>>> notifyChangePost(%s%s).\n", (textChanged ? " textChanged" : ""), (selChanged ? " selChanged" : ""));
	if(_inputDelegate != nil){
		if(textChanged){
			[_inputDelegate textDidChange:self];
		}
		if(selChanged){
			[_inputDelegate selectionDidChange:self];
		}
	}
}


//-------------
//<UIResponder>: Managing the Responder Chain
//-------------

//@property(nonatomic, readonly) UIResponder *nextResponder;
//@property(nonatomic, readonly) BOOL isFirstResponder;
//@property(nonatomic, readonly) BOOL canBecomeFirstResponder;

-(BOOL)canBecomeFirstResponder {
	return (NBGestorTeclas::keyboardShouldBeVisible() ? YES : NO);
}

//@property(nonatomic, readonly) BOOL canResignFirstResponder;
-(BOOL)canResignFirstResponder {
	return YES;
}

//-(BOOL)becomeFirstResponder;
//-(BOOL)resignFirstResponder;

//-------------
//<UIResponder>: Responding to Touch Events
//-------------
//- (void) touchesBegan:(NSSet *) touches withEvent: (UIEvent *) event;
//- (void) touchesMoved:(NSSet *) touches withEvent: (UIEvent *) event;
//- (void) touchesEnded:(NSSet *) touches withEvent: (UIEvent *) event;
//- (void) touchesCancelled:(NSSet *) touches withEvent: (UIEvent *) event;
//- (void)touchesEstimatedPropertiesUpdated:(NSSet<UITouch *> *)touches;

//-------------
//<UIResponder>: Responding to Motion Events
//-------------
//- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event;
//- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event;
//- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event;

//-------------
//<UIResponder>: Responding to Press Events
//-------------
//- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
//- (void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
//- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
//- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;

//-------------
//<UIResponder>: Responding to Remote-Control Events
//-------------
//- (void)remoteControlReceivedWithEvent:(UIEvent *)event;

//-------------
//<UIResponder>: Managing Input Views
//-------------
//@property(nonatomic, readonly, strong) __kindof UIView *inputView;
//@property(nonatomic, readonly, strong) UIInputViewController *inputViewController;
//@property(nonatomic, readonly, strong) __kindof UIView *inputAccessoryView;
//@property(nonatomic, readonly, strong) UIInputViewController *inputAccessoryViewController;
//- (void)reloadInputViews;

//-------------
//<UIResponder>: Getting the Undo Manager
//-------------
//@property(nonatomic, readonly) NSUndoManager *undoManager;

//-------------
//<UIResponder>: Building and Validating Commands
//-------------
//- (void)buildCommandsWithBuilder:(id<UICommandBuilder>)builder; (beta)
//- (void)validateCommand:(UIMutableCommand *)command; (beta)
//- (UICommandValidation *)validationForCommand:(UICommand *)command; (deprecated)
//- (BOOL)canPerformAction:(SEL)action withSender:(id)sender;
//- (id)targetForAction:(SEL)action withSender:(id)sender;

//-------------
//<UIResponder>: Accessing the Available Key Commands
//-------------
//@property(nonatomic, readonly) NSArray<UIKeyCommand *> *keyCommands;

//-------------
//<UIResponder>: Managing the Text Input Mode
//-------------
//@property(nonatomic, readonly, strong) UITextInputMode *textInputMode;
//@property(nonatomic, readonly, strong) NSString *textInputContextIdentifier;
//+ (void)clearTextInputContextIdentifier:(NSString *)identifier;
//@property(nonatomic, readonly, strong) UITextInputAssistantItem *inputAssistantItem;

//-------------
//<UIResponder>: Supporting User Activities
//-------------
//@property(nonatomic, strong) NSUserActivity *userActivity;
//- (void)restoreUserActivityState:(NSUserActivity *)activity;
//- (void)updateUserActivityState:(NSUserActivity *)activity;


//-------------
//<UIKeyInput>
//-------------

- (void)deleteBackward {
	NBGestorTeclas::entradaLockForBatch();
	{
		PRINTF_INFO("<<<UIKeyInput>>> deleteBackward.\n");
		NBGestorTeclas::entradaBackspace(true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (BOOL)hasText {
	BOOL r = FALSE;
	NBGestorTeclas::entradaLockForBatch();
	{
		r = (NBGestorTeclas::entradaTieneTexto() ? YES : NO);
		PRINTF_INFO("<<<UIKeyInput>>> hasText('%s').\n", (r ? "YES" : "NO"));
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (void)insertText:(NSString *)text {
	PRINTF_INFO("<<<UIKeyInput>>> insertText('%s').\n", [text UTF8String]);
	NBGestorTeclas::entradaLockForBatch();
	{
		NBGestorTeclas::entradaIntroducirTexto([text UTF8String], true);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

//-------------
//<UITextInput>: Replacing and Returning Text
//-------------
- (NSString *)textInRange:(UITextRange *)range {
	NSString* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 ini = ((AUTextPosition*)range.start).pos;
		const SI32 fin = ((AUTextPosition*)range.end).pos;
		NBASSERT(ini <= fin)
		{
			if(range.empty){
				r = @"";
			} else {
				AUCadenaMutable8* strContenido = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				NBGestorTeclas::entradaTextoEnCharDefsContenido(ini, (fin - ini) + 1, strContenido);
				r = [NSString stringWithUTF8String:strContenido->str()];
				strContenido->liberar(NB_RETENEDOR_NULL);
			}
		}
		PRINTF_INFO("<<<UITextInput>>> textInRange(%d, %d%s) <== %s%s%s.\n", ini, fin, (range.empty? ", empty":""), (r == nil ? "" : "'"), (r == nil ? "nil" : [r UTF8String]), (r == nil ? "" : "'"));
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (void)replaceRange:(UITextRange *)range withText:(NSString *)text {
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 ini = ((AUTextPosition*)range.start).pos;
		const SI32 fin = ((AUTextPosition*)range.end).pos;
		{
			if(range.empty){
				PRINTF_INFO("<<<UITextInput>>> replaceRange(%d, %d%s, '%s').\n", ini, fin, (range.empty? ", empty":""), [text UTF8String]);
			} else {
				AUCadenaMutable8* strContenido = new(ENMemoriaTipo_Temporal) AUCadenaMutable8();
				NBGestorTeclas::entradaTextoEnCharDefsContenido(ini, (fin - ini) + 1, strContenido);
				PRINTF_INFO("<<<UITextInput>>> replaceRange(%d, %d%s, '%s' ==> '%s').\n", ini, fin, (range.empty? ", empty":""), strContenido->str(), [text UTF8String]);
				strContenido->liberar(NB_RETENEDOR_NULL);
			}
		}
		NBASSERT(ini <= fin)
		{
			if(range.empty){
				NBGestorTeclas::entradaTextoEnCharDefsReemplazar(ini, 0, [text UTF8String], ENTextRangeSet_Word);
			} else {
				NBGestorTeclas::entradaTextoEnCharDefsReemplazar(ini, (fin - ini) + 1, [text UTF8String], ENTextRangeSet_Word);
			}
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

- (BOOL)shouldChangeTextInRange:(UITextRange *)range replacementText:(NSString *)text {
	const SI32 ini = ((AUTextPosition*)range.start).pos;
	const SI32 fin = ((AUTextPosition*)range.end).pos;
	PRINTF_INFO("<<<UITextInput>>> shouldChangeTextInRange(%d, %d%s, '%s') <== YES.\n", ini, fin, (range.empty? ", empty":""), [text UTF8String]);
	//NBASSERT(ini <= fin)
	return YES;
}

//-------------
//<UITextInput>: Working with Marked and Selected Text
//-------------
//@property(readwrite, copy) UITextRange *selectedTextRange;
-(UITextRange*) selectedTextRange {
	AUTextRange* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const NBRangoI rSel = NBGestorTeclas::entradaRangoSeleccionAbs();
		r = [[AUTextRange alloc] initWithRange:rSel];
		[r autorelease];
		PRINTF_INFO("<<<UITextInput>>> selectedTextRange <= (%d, %d%s).\n", rSel.inicio, (rSel.inicio + rSel.tamano - 1), (r.empty? ", empty":""));
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setSelectedTextRange:(UITextRange*) range {
	NBGestorTeclas::entradaLockForBatch();
	{
		const SI32 ini = ((AUTextPosition*)range.start).pos;
		const SI32 fin = ((AUTextPosition*)range.end).pos;
		PRINTF_INFO("<<<UITextInput>>>+ setSelectedTextRange(%d, %d%s).\n", ini, fin, (range.empty? ", empty":""));
		NBASSERT(ini <= fin)
		{
			if(range.empty){
				NBGestorTeclas::entradaRangoSeleccionEstablecer(ini, 0);
			} else {
				NBGestorTeclas::entradaRangoSeleccionEstablecer(ini, (fin - ini) + 1);
			}
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
}

//@property(nonatomic, readonly) UITextRange *markedTextRange;
-(UITextRange*) markedTextRange {
	AUTextRange* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			const NBRangoI rSel = NBGestorTeclas::entradaRangoMarcadoAbs();
			if(rSel.tamano > 0){
				r = [[AUTextRange alloc] initWithRange:rSel];
				[r autorelease];
			}
		}
		if(r == nil){
			PRINTF_INFO("<<<UITextInput>>> selectedTextRange <= nil.\n");
		} else {
			PRINTF_INFO("<<<UITextInput>>> markedTextRange <= (%d, %d%s).\n", ((AUTextPosition*)[r start]).pos, ((AUTextPosition*)[r end]).pos, ([r isEmpty]? ", empty":""));
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (void)setMarkedText:(NSString *)markedText selectedRange:(NSRange)selectedRange {
	PRINTF_INFO("<<<UITextInput>>>+ setMarkedText(%d, +%d ==> %s).\n", (UI32)selectedRange.location, (SI32)selectedRange.length, [markedText UTF8String]);
}

- (void)unmarkText {
	PRINTF_INFO("<<<UITextInput>>> unmarkText.\n");
	//NBGestorTeclas::entradaLockForBatch();
	//{
	//	NBGestorTeclas::entradaRangoDesmarcar();
	//}
	//NBGestorTeclas::entradaUnlockFromBatch();
}

//@property(nonatomic, copy) NSDictionary *markedTextStyle;
- (NSDictionary*) markedTextStyle {
	PRINTF_INFO("<<<UITextInput>>> markedTextStyle.\n");
	return _markedTextStyle;
}

-(void) setMarkedTextStyle:(NSDictionary*) style {
	PRINTF_INFO("<<<UITextInput>>> setMarkedTextStyle.\n");
	NSDictionary* copy = nil;
	if(style != nil){
		copy = [style copy];
	}
	if(_markedTextStyle != nil){
		[_markedTextStyle release];
		_markedTextStyle = nil;
	}
	_markedTextStyle = copy;
}

//NSString *const UITextInputTextBackgroundColorKey (UIColor); //Deprecated in iOS 8.0.
//NSString *const UITextInputTextColorKey (UIColor); //Deprecated in iOS 8.0.
//NSString *const UITextInputTextFontKey (UIFont); //Deprecated in iOS 8.0.
//@property(nonatomic) UITextStorageDirection selectionAffinity;
/*
 In the default implementation,
 if the selection is not at the end of the line, or if the selection is at the start of a paragraph for an empty line,
 a forward direction is assumed (UITextStorageDirectionForward);
 otherwise,
 a backward direction UITextStorageDirectionBackward is assumed.
 */

//-------------
//<UITextInput>: Computing Text Ranges and Text Positions
//-------------
- (UITextRange *)textRangeFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition {
	AUTextRange* r = [[AUTextRange alloc] initWithAUTextPositions:(AUTextPosition*)fromPosition posEnd:(AUTextPosition*)toPosition];
	[r autorelease];
	PRINTF_INFO("<<<UITextInput>>> textRangeFromPosition(%d, %d) <== AUTextRange.\n", ((AUTextPosition*)fromPosition).pos, ((AUTextPosition*)toPosition).pos);
	return r;
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)position offset:(NSInteger)offset {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			const SI32 iPrimer = NBGestorTeclas::entradaIndiceCharDefPrimero();
			const SI32 iUltimo = NBGestorTeclas::entradaIndiceCharDefUltimo();
			const SI32 posResultado = (((AUTextPosition*)position).pos + (SI32)offset);
			if(posResultado >= iPrimer && posResultado <= iUltimo){ //posResultado <= (iUltimo + 1)
				r = [[AUTextPosition alloc] initWithPos:posResultado];
				[r autorelease];
			}
		}
		if(r == nil){
			PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d) <== nil.\n", ((AUTextPosition*)position).pos, (offset >=0 ? "+" : ""), (SI32)offset);
		} else {
			PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d) <== (%d).\n", ((AUTextPosition*)position).pos, (offset >=0 ? "+" : ""), (SI32)offset, r.pos);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (UITextPosition *)positionFromPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction offset:(NSInteger)offset {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		{
			const SI32 iPrimer = NBGestorTeclas::entradaIndiceCharDefPrimero();
			const SI32 iUltimo = NBGestorTeclas::entradaIndiceCharDefUltimo();
			SI32 posResultado = 0;
			switch(direction){
				case UITextLayoutDirectionRight:
					posResultado = (((AUTextPosition*)position).pos + (SI32)offset);
					break;
				case UITextLayoutDirectionLeft:
					posResultado = (((AUTextPosition*)position).pos - (SI32)offset);
					break;
				case UITextLayoutDirectionUp:
					NBASSERT(false)
					break;
				case UITextLayoutDirectionDown:
					NBASSERT(false)
					break;
				default:
					NBASSERT(false)
					break;
			}
			if(posResultado >= iPrimer && posResultado <= iUltimo){ //posResultado <= (iUltimo + 1)
				r = [[AUTextPosition alloc] initWithPos:posResultado];
				[r autorelease];
			}
		}
		if(r == nil){
			PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d, %s) <== nil .\n", ((AUTextPosition*)position).pos, (offset >= 0 ? "+" : ""), (SI32)offset, (direction == UITextLayoutDirectionRight ? "toRight" : direction == UITextLayoutDirectionLeft ? "toLeft" : direction == UITextLayoutDirectionUp ? "toUp" : direction == UITextLayoutDirectionDown ? "toDown" : "to????"));
		} else {
			PRINTF_INFO("<<<UITextInput>>> positionFromPosition(%d, %s%d, %s) <== (%d) .\n", ((AUTextPosition*)position).pos, (offset >= 0 ? "+" : ""), (SI32)offset, (direction == UITextLayoutDirectionRight ? "toRight" : direction == UITextLayoutDirectionLeft ? "toLeft" : direction == UITextLayoutDirectionUp ? "toUp" : direction == UITextLayoutDirectionDown ? "toDown" : "to????"), r.pos);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

//@property(nonatomic, readonly) UITextPosition *beginningOfDocument;
-(UITextPosition*) beginningOfDocument {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const UI32 rPos = NBGestorTeclas::entradaIndiceCharDefPrimero();
		r = [[AUTextPosition alloc] initWithPos:rPos];
		[r autorelease];
		PRINTF_INFO("<<<UITextInput>>> beginningOfDocument <== (%d).\n", rPos);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

//@property(nonatomic, readonly) UITextPosition *endOfDocument;
-(UITextPosition*) endOfDocument {
	AUTextPosition* r = nil;
	NBGestorTeclas::entradaLockForBatch();
	{
		const UI32 rPos = NBGestorTeclas::entradaIndiceCharDefUltimo();
		r = [[AUTextPosition alloc] initWithPos:rPos];
		[r autorelease];
		PRINTF_INFO("<<<UITextInput>>> endOfDocument <== (%d).\n", rPos);
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

//-------------
//<UITextInput>: Evaluating Text Positions
//-------------
- (NSComparisonResult)comparePosition:(UITextPosition *)position toPosition:(UITextPosition *)other {
	NSComparisonResult r = NSOrderedSame;
	const SI32 ini = ((AUTextPosition*)position).pos;
	const SI32 fin = ((AUTextPosition*)other).pos;
	if(ini < fin){
		r = NSOrderedAscending;
	} else if(ini > fin){
		r = NSOrderedDescending;
	} else {
		NBASSERT(r == NSOrderedSame)
	}
	PRINTF_INFO("<<<UITextInput>>> comparePosition(%d, %d) <== %s.\n", ini, fin, (r == NSOrderedAscending ? "Ascending" : r == NSOrderedDescending ? "Descending" : r == NSOrderedSame ? "Same" : "????"));
	return r;
}

- (NSInteger)offsetFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition {
	const SI32 ini = ((AUTextPosition*)fromPosition).pos;
	const SI32 fin = ((AUTextPosition*)toPosition).pos;
	PRINTF_INFO("<<<UITextInput>>> offsetFromPosition(%d, %d) <== %d.\n", ini, fin, (fin - ini));
	return (fin - ini);
}

//-------------
//<UITextInput>: Determining Layout and Writing Direction
//-------------
- (nullable UITextPosition *)positionWithinRange:(UITextRange *)range farthestInDirection:(UITextLayoutDirection)direction {
	PRINTF_INFO("<<<UITextInput>>> positionWithinRange.\n");
	return nil;
}

- (nullable UITextRange *)characterRangeByExtendingPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction {
	PRINTF_INFO("<<<UITextInput>>> characterRangeByExtendingPosition.\n");
	return nil;
}

- (UITextWritingDirection)baseWritingDirectionForPosition:(UITextPosition *)position inDirection:(UITextStorageDirection)direction {
	//UITextStorageDirectionForward = 0
	//UITextStorageDirectionBackward
	//
	//UITextWritingDirectionNatural = -1,
	//UITextWritingDirectionLeftToRight = 0,
	//UITextWritingDirectionRightToLeft,
	PRINTF_INFO("<<<UITextInput>>> baseWritingDirectionForPosition(%d, %s) <== %s.\n", ((AUTextPosition *)position).pos, (direction == UITextStorageDirectionForward ? "Forward" : direction == UITextStorageDirectionBackward ? "Backward" : "???"), (_inputWriteDir == UITextWritingDirectionNatural ? "Natural" : _inputWriteDir == UITextWritingDirectionLeftToRight ? "LeftToRight" : _inputWriteDir == UITextWritingDirectionRightToLeft ? "RightToLeft" : "???"));
	return _inputWriteDir;
}

- (void)setBaseWritingDirection:(UITextWritingDirection)writingDirection forRange:(UITextRange *)range {
	PRINTF_INFO("<<<UITextInput>>> setBaseWritingDirection(%s, %d, %d).\n", (writingDirection == UITextWritingDirectionNatural ? "Natural" : writingDirection == UITextWritingDirectionLeftToRight ? "LeftToRight" : writingDirection == UITextWritingDirectionRightToLeft ? "RightToLeft" : "???"), ((AUTextPosition*)range.start).pos, ((AUTextPosition*)range.end).pos);
	_inputWriteDir = writingDirection;
}

//-------------
//<UITextInput>: Geometry and Hit-Testing Methods
//-------------
- (CGRect)firstRectForRange:(UITextRange *)range {
	CGRect r; r.origin.x = 0; r.origin.y = 0; r.size.width = 0; r.size.height = 0;
	NBGestorTeclas::entradaLockForBatch();
	{
		//AUIOSMonoViewData* data = (AUIOSMonoViewData*)_opaqueData;
		if(_visualView != nil){
			AUTextRange* rango = (AUTextRange*)range;
			if(![rango isEmpty]){
				const float scale = [_visualView contentScaleFactor];
				//
				const SI32 ini = [rango start].pos;
				const SI32 fin = [rango end].pos;
				NBASSERT(ini <= fin)
				const NBRectangulo rect = NBGestorTeclas::entradaPrimerRectanguloParaCharDefs(ini, (fin - ini + 1));
				r.origin.x = (float)rect.x / scale;
				r.origin.y = (float)rect.y / scale;
				r.size.width = (float)rect.ancho / scale;
				r.size.height = (float)rect.alto / scale;
			}
		}
		if(r.size.width == 0 && r.size.height == 0){
			PRINTF_INFO("<<<UITextInput>>> firstRectForRange(%d, %d) <== empty.\n", ((AUTextPosition*)range.start).pos, ((AUTextPosition*)range.end).pos);
		} else {
			PRINTF_INFO("<<<UITextInput>>> firstRectForRange(%d, %d) <== (%f, %f)-(+%f, +%f).\n", ((AUTextPosition*)range.start).pos, ((AUTextPosition*)range.end).pos, r.origin.x, r.origin.y, r.size.width, r.size.height);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (CGRect)caretRectForPosition:(UITextPosition *)position {
	CGRect r; r.origin.x = 0; r.origin.y = 0; r.size.width = 0; r.size.height = 0;
	NBGestorTeclas::entradaLockForBatch();
	{
		if(_visualView != nil){
			const float scale = [_visualView contentScaleFactor];
			//
			const SI32 pos = ((AUTextPosition*) position).pos;
			const NBRectangulo rect = NBGestorTeclas::entradaRectanguloParaCursor(pos);
			r.origin.x = (float)rect.x / scale;
			r.origin.y = (float)rect.y / scale;
			r.size.width = (float)rect.ancho / scale;
			r.size.height = (float)rect.alto / scale;
		}
		if(r.size.width == 0 && r.size.height == 0){
			PRINTF_INFO("<<<UITextInput>>> caretRectForPosition(%d) <== empty.\n", ((AUTextPosition*)position).pos);
		} else {
			PRINTF_INFO("<<<UITextInput>>> caretRectForPosition(%d) <== (%f, %f)-(+%f, +%f).\n", ((AUTextPosition*)position).pos, r.origin.x, r.origin.y, r.size.width, r.size.height);
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

- (nullable UITextPosition *)closestPositionToPoint:(CGPoint)point {
	PRINTF_INFO("<<<UITextInput>>> closestPositionToPoint(%f, %f).\n", point.x, point.y);
	return nil;
}

- (nullable UITextPosition *)closestPositionToPoint:(CGPoint)point withinRange:(UITextRange *)range {
	PRINTF_INFO("<<<UITextInput>>> closestPositionToPoint(%f, %f) withinRange(...).\n", point.x, point.y);
	return nil;
}

- (nonnull NSArray<UITextSelectionRect *> *)selectionRectsForRange:(UITextRange *)range {
	PRINTF_INFO("<<<UITextInput>>> selectionRectsForRange.\n");
	//An array of UITextSelectionRect objects that encompass the selection.
	NSArray<UITextSelectionRect*>* r = [[NSArray<UITextSelectionRect*> alloc] init];
	[r autorelease];
	return r;
}

- (nullable UITextRange *)characterRangeAtPoint:(CGPoint)point {
	PRINTF_INFO("<<<UITextInput>>> characterRangeAtPoint.\n");
	return nil;
}

//-------------
//<UITextInput>: Text Input Delegate and Text Input Tokenizer
//-------------
//@property(nonatomic, assign) id< UITextInputDelegate > inputDelegate;
- (id< UITextInputDelegate >) inputDelegate {
	PRINTF_INFO("<<<UITextInput>>> inputDelegate.\n");
	return _inputDelegate;
}

- (void) setInputDelegate: (id< UITextInputDelegate >) delegate {
	PRINTF_INFO("<<<UITextInput>>> setInputDelegate.\n");
	if(delegate != nil) [delegate retain];
	if(_inputDelegate != nil) [_inputDelegate release];
	_inputDelegate = delegate;
}

//@property(nonatomic, readonly) id< UITextInputTokenizer > tokenizer;
- (id< UITextInputTokenizer >) tokenizer {
	PRINTF_INFO("<<<UITextInput>>> tokenizer.\n");
	return _tokenizer;
}

//-------------
//<UITextInput>: Managing the Floating Cursor
//-------------

- (void)beginFloatingCursorAtPoint:(CGPoint)point {
    NBGestorTeclas::entradaLockForBatch();
    if(!self->floatingCursor.isActive){
        STNBPoint pos;
        NBMemory_setZeroSt(pos, STNBPoint);
        pos.x = point.x;
        pos.y = point.y;
        if(_visualView != nil){
            const float scale = [_visualView contentScaleFactor];
            pos.x *= scale;
            pos.y *= scale;
        }
        self->floatingCursor.isActive = TRUE;
        self->floatingCursor.pos.os.start = self->floatingCursor.pos.os.cur = point;
        self->floatingCursor.pos.clt.start = self->floatingCursor.pos.clt.cur = pos;
        {
            const NBRangoI sel = NBGestorTeclas::entradaRangoSeleccion();
            const NBRectangulo rect = NBGestorTeclas::entradaRectanguloParaCursor(sel.inicio + sel.tamano);
            self->floatingCursor.pos.clt.cursor.start.x = rect.x + (rect.ancho / 2.0f);
            self->floatingCursor.pos.clt.cursor.start.y = rect.y + (rect.alto / 2.0f);
            PRINTF_INFO("<<<UITextInput>>> beginFloatingCursorAtPoint osPos(%f, %f) cltPos(%f, %f) cursorPos(%f, %f) rngSel(%d, %s%d) rectSel(%f, %f)-(+%f, +%f).\n", (float)point.x, (float)point.y, pos.x, pos.y, self->floatingCursor.pos.clt.cursor.start.x, self->floatingCursor.pos.clt.cursor.start.y, sel.inicio, (sel.tamano >= 0 ? "+" : ""), sel.tamano, rect.x, rect.y);
        }
    }
    NBGestorTeclas::entradaUnlockFromBatch();
}

- (void)updateFloatingCursorAtPoint:(CGPoint)point {
    NBGestorTeclas::entradaLockForBatch();
    if(self->floatingCursor.isActive){
        STNBPoint pos;
        NBMemory_setZeroSt(pos, STNBPoint);
        pos.x = point.x;
        pos.y = point.y;
        if(_visualView != nil){
            const float scale = [_visualView contentScaleFactor];
            pos.x *= scale;
            pos.y *= scale;
        }
        self->floatingCursor.pos.os.cur = point;
        self->floatingCursor.pos.clt.cur = pos;
        {
            BOOL isMultiline = FALSE;
            /*const ENTextEditorType type = */ NBGestorTeclas::entradaEditorType(&isMultiline);
            STNBPoint delta;
            NBMemory_setZeroSt(delta, STNBPoint);
            delta.x = self->floatingCursor.pos.clt.cur.x - self->floatingCursor.pos.clt.start.x;
            if(isMultiline){
                delta.y = self->floatingCursor.pos.clt.cur.y - self->floatingCursor.pos.clt.start.y;
            }
            //calculate
            {
                const UI32 charSel = NBGestorTeclas::entradaIndiceCharDefMasCercano(self->floatingCursor.pos.clt.cursor.start.x + delta.x, self->floatingCursor.pos.clt.cursor.start.y + delta.y);
                NBGestorTeclas::entradaRangoSeleccionEstablecer((SI32)charSel, 0);
                PRINTF_INFO("<<<UITextInput>>> updateFloatingCursorAtPoint osPos(%f, %f) cltPos(%f, %f) cursorPos(%f, %f) charSel(%u).\n", (float)point.x, (float)point.y, pos.x, pos.y, self->floatingCursor.pos.clt.cursor.start.x + delta.x, self->floatingCursor.pos.clt.cursor.start.y + delta.y, charSel);
            }
        }
    }
    NBGestorTeclas::entradaUnlockFromBatch();
}

- (void)endFloatingCursor {
    NBGestorTeclas::entradaLockForBatch();
    if(self->floatingCursor.isActive){
        self->floatingCursor.isActive = FALSE;
        PRINTF_INFO("<<<UITextInput>>> endFloatingCursor.\n");
    }
    NBGestorTeclas::entradaUnlockFromBatch();
}

//-------------
//<UITextInput>: Using Dictation
//-------------

/*- (void)dictationRecordingDidEnd {
 //
 }*/

/*- (void)dictationRecognitionFailed {
 //
 }*/

/*- (void)insertDictationResult:(NSArray<UIDictationPhrase *> *)dictationResult {
 //
 }*/

//@property(nonatomic, readonly) id insertDictationResultPlaceholder;
/*- (id) insertDictationResultPlaceholder {
 //
 }*/

/*- (CGRect)frameForDictationResultPlaceholder:(id)placeholder {
 //
 }*/

/*- (void)removeDictationResultPlaceholder:(id)placeholder willInsertResult:(BOOL)willInsertResult {
 //
 }*/

//-------------
//<UITextInput>: Returning Text Styling Information
//-------------

/*- (NSDictionary<NSAttributedStringKey, id> *)textStylingAtPosition:(UITextPosition *)position inDirection:(UITextStorageDirection)direction {
 //
 }*/

//-------------
//<UITextInput>: Reconciling Text Position and Character Offset
//-------------

/*- (UITextPosition *)positionWithinRange:(UITextRange *)range atCharacterOffset:(NSInteger)offset {
 //
 }*/

/*- (NSInteger)characterOffsetOfPosition:(UITextPosition *)position withinRange:(UITextRange *)range {
 //
 }*/

//-------------
//<UITextInput>: Returning the Text Input View
//-------------
//@property(nonatomic, readonly) UIView *textInputView
- (UIView*)textInputView {
	PRINTF_INFO("<<<UITextInput>>> textInputView.\n");
	return _visualView;
}

//
//
//

//-------------
//<UITextInputTraits>: Returning the Text Input View
//-------------

//@property(nonatomic) UIKeyboardType keyboardType;
-(UIKeyboardType) keyboardType {
	PRINTF_INFO("<<<UITextInputTraits>>> keyboardType.\n");
	UIKeyboardType r = UIKeyboardTypeASCIICapable;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UIKeyboardTypeASCIICapable; //UIKeyboardTypeDefault includes emojis
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UIKeyboardTypeASCIICapable; //UIKeyboardTypeDefault includes emojis
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UIKeyboardTypeASCIICapable; //UIKeyboardTypeDefault includes emojis
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UIKeyboardTypeEmailAddress;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UIKeyboardTypePhonePad;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UIKeyboardTypeNumberPad;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UIKeyboardTypeDecimalPad;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UIKeyboardTypeNamePhonePad;
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setKeyboardType:(UIKeyboardType)keyboardType {
	PRINTF_INFO("<<<UITextInputTraits>>> setKeyboardType.\n");
}

//@property(nonatomic) UIKeyboardAppearance keyboardAppearance;
-(UIKeyboardAppearance) keyboardAppearance {
	PRINTF_INFO("<<<UITextInputTraits>>> keyboardAppearance.\n");
	return UIKeyboardAppearanceDefault;
}

-(void) setKeyboardAppearance:(UIKeyboardAppearance)keyboardAppearance {
	PRINTF_INFO("<<<UITextInputTraits>>> setKeyboardAppearance.\n");
}

//@property(nonatomic) UIReturnKeyType returnKeyType;
-(UIReturnKeyType) returnKeyType {
	PRINTF_INFO("<<<UITextInputTraits>>> returnKeyType.\n");
	BOOL isMultiline = FALSE;
	{
		NBGestorTeclas::entradaLockForBatch();
		/*const ENTextEditorType nbType =*/ NBGestorTeclas::entradaEditorType(&isMultiline);
		NBGestorTeclas::entradaUnlockFromBatch();
	}
	/*UIReturnKeyDefault, Specifies that the visible title of the Return key is “return”.
	 UIReturnKeyGo, Specifies that the visible title of the Return key is “Go”.
	 UIReturnKeyGoogle, Specifies that the visible title of the Return key is “Google”.
	 UIReturnKeyJoin, Specifies that the visible title of the Return key is “Join”.
	 UIReturnKeyNext, Specifies that the visible title of the Return key is “Next”.
	 UIReturnKeyRoute, Specifies that the visible title of the Return key is “Route”.
	 UIReturnKeySearch, Specifies that the visible title of the Return key is “Search”.
	 UIReturnKeySend, Specifies that the visible title of the Return key is “Send”.
	 UIReturnKeyYahoo, Specifies that the visible title of the Return key is “Yahoo”.
	 UIReturnKeyDone, Specifies that the visible title of the Return key is “Done”.
	 UIReturnKeyEmergencyCall, Specifies that the visible title of the Return key is “Emergency Call”.
	 UIReturnKeyContinue, Specifies that the visible title of the Return key is “Continue”.*/
	return (isMultiline ? UIReturnKeyDefault : UIReturnKeyDone);
}

-(void) setReturnKeyType:(UIReturnKeyType)returnKeyType {
	PRINTF_INFO("<<<UITextInputTraits>>> setReturnKeyType.\n");
}

//@property(nonatomic, copy) UITextContentType textContentType;
-(UITextContentType) textContentType {
	PRINTF_INFO("<<<UITextInputTraits>>> textContentType.\n");
	return @""; //UITextContentTypeName;
}

-(void) setTextContentType:(UITextContentType)textContentType {
	PRINTF_INFO("<<<UITextInputTraits>>> setTextContentType.\n");
}

//-------------
//<UITextInputTraits>: Managing the Keyboard Behavior
//-------------

//@property(nonatomic, getter=isSecureTextEntry) BOOL secureTextEntry;
-(BOOL) secureTextEntry {
	BOOL r = NO;
	PRINTF_INFO("<<<UITextInputTraits>>> secureTextEntry.\n");
	//Note 2019-01-19: when you enable this, the OS retains the value/behavior; is like for security reasons the OS wont allow to disable it.
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = YES;
				break;
			default:
				r = NO;
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
	
}

-(BOOL) isSecureTextEntry {
	PRINTF_INFO("<<<UITextInputTraits>>> isSecureTextEntry.\n");
	return [self secureTextEntry];
}

-(void) setSecureTextEntry:(BOOL)secureTextEntry {
	PRINTF_INFO("<<<UITextInputTraits>>> setSecureTextEntry.\n");
}

-(void) setIsSecureTextEntry:(BOOL)secureTextEntry {
	PRINTF_INFO("<<<UITextInputTraits>>> setIsSecureTextEntry.\n");
	[self setSecureTextEntry: secureTextEntry];
}

//@property(nonatomic) BOOL enablesReturnKeyAutomatically;
-(BOOL) enablesReturnKeyAutomatically {
	PRINTF_INFO("<<<UITextInputTraits>>> enablesReturnKeyAutomatically.\n");
	//FALSE: the "Return" key is always visible.
	//TRUE: the "Return" is hidden untill th user enters text
	return FALSE;
}

-(void) setEnablesReturnKeyAutomatically:(BOOL)enablesReturnKeyAutomatically {
	PRINTF_INFO("<<<UITextInputTraits>>> setEnablesReturnKeyAutomatically.\n");
}

//-------------
//<UITextInputTraits>: Managing Spelling and Autocorrection
//-------------

//@property(nonatomic) UITextAutocapitalizationType autocapitalizationType;
-(UITextAutocapitalizationType) autocapitalizationType {
	PRINTF_INFO("<<<UITextInputTraits>>> autocapitalizationType.\n");
	UITextAutocapitalizationType r = UITextAutocapitalizationTypeSentences;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UITextAutocapitalizationTypeNone;
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UITextAutocapitalizationTypeNone;
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UITextAutocapitalizationTypeSentences;
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UITextAutocapitalizationTypeNone;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UITextAutocapitalizationTypeNone;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UITextAutocapitalizationTypeNone;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UITextAutocapitalizationTypeNone;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UITextAutocapitalizationTypeWords;
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setAutocapitalizationType:(UITextAutocapitalizationType)autocapitalizationType {
	PRINTF_INFO("<<<UITextInputTraits>>> setAutocapitalizationType.\n");
}

//@property(nonatomic) UITextAutocorrectionType autocorrectionType;
-(UITextAutocorrectionType) autocorrectionType {
	PRINTF_INFO("<<<UITextInputTraits>>> autocorrectionType.\n");
	UITextAutocorrectionType r = UITextAutocorrectionTypeDefault;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UITextAutocorrectionTypeNo;
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UITextAutocorrectionTypeNo;
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UITextAutocorrectionTypeYes; //UITextAutocorrectionTypeYes; //ToDo: enable after autoselect-word returns
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UITextAutocorrectionTypeNo;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UITextAutocorrectionTypeNo;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UITextAutocorrectionTypeNo;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UITextAutocorrectionTypeNo;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UITextAutocorrectionTypeYes; //UITextAutocorrectionTypeYes; //ToDo: enable after autoselect-word returns
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setAutocorrectionType:(UITextAutocorrectionType)autocorrectionType {
	PRINTF_INFO("<<<UITextInputTraits>>> setAutocorrectionType.\n");
}

//@property(nonatomic) UITextSpellCheckingType spellCheckingType;
-(UITextSpellCheckingType) spellCheckingType {
	PRINTF_INFO("<<<UITextInputTraits>>> spellCheckingType.\n");
	UITextSpellCheckingType r = UITextSpellCheckingTypeDefault;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UITextSpellCheckingTypeNo;
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UITextSpellCheckingTypeNo;
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UITextSpellCheckingTypeYes; //UITextSpellCheckingTypeYes; //ToDo: enable after autoselect-word returns
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UITextSpellCheckingTypeNo;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UITextSpellCheckingTypeNo;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UITextSpellCheckingTypeNo;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UITextSpellCheckingTypeNo;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UITextSpellCheckingTypeYes; //UITextSpellCheckingTypeYes; //ToDo: enable after autoselect-word returns
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setSpellCheckingType:(UITextSpellCheckingType)spellCheckingType {
	PRINTF_INFO("<<<UITextInputTraits>>> setSpellCheckingType.\n");
}

//-------------
//<UITextInputTraits>: Configuring the Auto-Formatting Behaviors
//-------------

//@property(nonatomic) UITextSmartQuotesType smartQuotesType;
-(UITextSmartQuotesType) smartQuotesType API_AVAILABLE(ios(11)) {
	PRINTF_INFO("<<<UITextInputTraits>>> smartQuotesType.\n");
	//UIKit replaces straight apostrophes and quotation marks with region-specific glyphs.
	UITextSmartQuotesType r = UITextSmartQuotesTypeDefault;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UITextSmartQuotesTypeNo;
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UITextSmartQuotesTypeNo;
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UITextSmartQuotesTypeYes; //UITextSmartQuotesTypeYes; //ToDo: enable after autoselect-word returns
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UITextSmartQuotesTypeNo;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UITextSmartQuotesTypeNo;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UITextSmartQuotesTypeNo;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UITextSmartQuotesTypeNo;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UITextSmartQuotesTypeNo;
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setSmartQuotesType:(UITextSmartQuotesType)smartQuotesType API_AVAILABLE(ios(11)) {
	PRINTF_INFO("<<<UITextInputTraits>>> setSmartQuotesType.\n");
}

//@property(nonatomic) UITextSmartDashesType smartDashesType;
-(UITextSmartDashesType) smartDashesType API_AVAILABLE(ios(11)) {
	PRINTF_INFO("<<<UITextInputTraits>>> smartDashesType.\n");
	//UIKit converts two hyphens into an en-dash and three hyphens into an em-dash automatically.
	UITextSmartDashesType r = UITextSmartDashesTypeDefault;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UITextSmartDashesTypeNo;
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UITextSmartDashesTypeNo;
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UITextSmartDashesTypeYes; //UITextSmartDashesTypeYes; //ToDo: enable after autoselect-word returns
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UITextSmartDashesTypeNo;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UITextSmartDashesTypeNo;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UITextSmartDashesTypeNo;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UITextSmartDashesTypeNo;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UITextSmartDashesTypeNo;
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setSmartDashesType:(UITextSmartDashesType)smartDashesType API_AVAILABLE(ios(11)) {
	PRINTF_INFO("<<<UITextInputTraits>>> setSmartDashesType.\n");
}

//@property(nonatomic) UITextSmartInsertDeleteType smartInsertDeleteType;
-(UITextSmartInsertDeleteType) smartInsertDeleteType API_AVAILABLE(ios(11)) {
	PRINTF_INFO("<<<UITextInputTraits>>> smartInsertDeleteType.\n");
	//UIKit may insert an extra space after a paste operation or delete one or two spaces after a cut or delete operation.
	UITextSmartInsertDeleteType r = UITextSmartInsertDeleteTypeDefault;
	NBGestorTeclas::entradaLockForBatch();
	{
		const ENTextEditorType nbType = NBGestorTeclas::entradaEditorType(NULL);
		switch (nbType) {
			case ENTextEditorType_Literal:	//Text will be literal
				r = UITextSmartInsertDeleteTypeNo;
				break;
			case ENTextEditorType_Password:	//Text will be literal (and special behavior; passchar is optional)
				r = UITextSmartInsertDeleteTypeNo;
				break;
			case ENTextEditorType_Compose:	//Text will be autocompleted
				r = UITextSmartInsertDeleteTypeYes; //UITextSmartInsertDeleteTypeYes; //ToDo: enable after autoselect-word returns
				break;
			case ENTextEditorType_Email:	//Text will be literal (and special keyboard distribution)
				r = UITextSmartInsertDeleteTypeNo;
				break;
			case ENTextEditorType_PhoneNumber: //Text will be numeric (and special keyboard distribution)
				r = UITextSmartInsertDeleteTypeNo;
				break;
			case ENTextEditorType_Integer:	//Integer number
				r = UITextSmartInsertDeleteTypeNo;
				break;
			case ENTextEditorType_Decimal:	//Decimal number
				r = UITextSmartInsertDeleteTypeNo;
				break;
			case ENTextEditorType_Name:		//Names (first letter capitalized)
				r = UITextSmartInsertDeleteTypeNo;
				break;
			default:
				break;
		}
	}
	NBGestorTeclas::entradaUnlockFromBatch();
	return r;
}

-(void) setSmartInsertDeleteType:(UITextSmartInsertDeleteType)smartInsertDeleteType API_AVAILABLE(ios(11)) {
	PRINTF_INFO("<<<UITextInputTraits>>> setSmartInsertDeleteType.\n");
}

@end

