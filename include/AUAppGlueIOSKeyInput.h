//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef AUAppGlueIOSKeyInput_H
#define AUAppGlueIOSKeyInput_H

#import <UIKit/UIKit.h> //for UIView, UITextInputStringTokenizer
#include "AUAppI.h"

//Keyboard input
@interface AUAppGlueIOSKeyInput : UIView<UIKeyInput, UITextInput, UITextInputTraits> /*UIResponder<UIKeyInput, UITextInput, UITextInputTraits>*/ {
	UIView*					_visualView;	//View that provides a coordinate system for all geometric values.
	id<UITextInputDelegate>	_inputDelegate;
	UITextWritingDirection	_inputWriteDir;
	NSDictionary<NSAttributedStringKey, id>* _markedTextStyle;
	UITextInputStringTokenizer* _tokenizer;
    //floatingCursor
    struct {
        BOOL        isActive;
        //pos
        struct {
            //os
            struct {
                CGPoint start;
                CGPoint cur;
            } os;
            //os
            struct {
                STNBPoint start;
                STNBPoint cur;
                //cursro
                struct {
                    STNBPoint start;
                } cursor;
            } clt; //framework
            
        } pos;
    } floatingCursor;
}
//Alloc
-(id) initWithVisualView:(UIView*)visualView;
-(void) dealloc;

-(void) notifyChangePre:(BOOL)textChanged selChanged:(BOOL)selChanged;
-(void) notifyChangePost:(BOOL)textChanged selChanged:(BOOL)selChanged;

//<UIResponder>: Managing the Responder Chain
//@property(nonatomic, readonly) UIResponder *nextResponder;
//@property(nonatomic, readonly) BOOL isFirstResponder;
-(BOOL)canBecomeFirstResponder; //@property(nonatomic, readonly) BOOL canBecomeFirstResponder;
-(BOOL)canResignFirstResponder; //@property(nonatomic, readonly) BOOL canResignFirstResponder;
//-(BOOL)becomeFirstResponder;
//-(BOOL)resignFirstResponder;

//<UIResponder>: Responding to Touch Events
//- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
//- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
//- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
//- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event;
//- (void)touchesEstimatedPropertiesUpdated:(NSSet<UITouch *> *)touches;

//<UIResponder>: Responding to Motion Events
//- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event;
//- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event;
//- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event;

//<UIResponder>: Responding to Press Events
//- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
//- (void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
//- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;
//- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event;

//<UIResponder>: Responding to Remote-Control Events
//- (void)remoteControlReceivedWithEvent:(UIEvent *)event;

//<UIResponder>: Managing Input Views
//@property(nonatomic, readonly, strong) __kindof UIView *inputView;
//@property(nonatomic, readonly, strong) UIInputViewController *inputViewController;
//@property(nonatomic, readonly, strong) __kindof UIView *inputAccessoryView;
//@property(nonatomic, readonly, strong) UIInputViewController *inputAccessoryViewController;
//- (void)reloadInputViews;

//<UIResponder>: Getting the Undo Manager
//@property(nonatomic, readonly) NSUndoManager *undoManager;

//<UIResponder>: Building and Validating Commands
//- (void)buildCommandsWithBuilder:(id<UICommandBuilder>)builder; (beta)
//- (void)validateCommand:(UIMutableCommand *)command; (beta)
//- (UICommandValidation *)validationForCommand:(UICommand *)command; (deprecated)
//- (BOOL)canPerformAction:(SEL)action withSender:(id)sender;
//- (id)targetForAction:(SEL)action withSender:(id)sender;

//<UIResponder>: Accessing the Available Key Commands
//@property(nonatomic, readonly) NSArray<UIKeyCommand *> *keyCommands;

//<UIResponder>: Managing the Text Input Mode
//@property(nonatomic, readonly, strong) UITextInputMode *textInputMode;
//@property(nonatomic, readonly, strong) NSString *textInputContextIdentifier;
//+ (void)clearTextInputContextIdentifier:(NSString *)identifier;
//@property(nonatomic, readonly, strong) UITextInputAssistantItem *inputAssistantItem;

//<UIResponder>: Supporting User Activities
//@property(nonatomic, strong) NSUserActivity *userActivity;
//- (void)restoreUserActivityState:(NSUserActivity *)activity;
//- (void)updateUserActivityState:(NSUserActivity *)activity;

@end

#endif
