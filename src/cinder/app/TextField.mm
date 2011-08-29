//
//  TextField.mm
//
//  Created by Tom Carden on 8/9/11.
//  Copyright 2011 Bloom Studio, Inc. All rights reserved.
//

#include "TextField.h"
#import <UIKit/UIKit.h>

class TextFieldImpl {
public:
    
    UITextField *uiTextField; // cocoa touch
    TextField *mTextField; // cinder 
    float       angle;
    CGAffineTransform prevTransform;
    
    TextFieldImpl( TextField *textField );
    void textChanged();
    void textDone();
    
    std::string getText();
    void setPosition(float x, float y, float length, float height);
    void hideTextField(bool decision);
};

@interface TFDelegate : NSObject <UITextFieldDelegate> {
    @public
    UITextField *activeTextField;
    TextFieldImpl *mTextFieldImpl;
}
@end

@implementation TFDelegate

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    activeTextField = textField;
    return YES;
}

- (BOOL) textField: (UITextField *) textField shouldChangeCharactersInRange: (NSRange) range replacementString: (NSString *) string
{
    mTextFieldImpl->textChanged(); // FIXME, will be missing the new character until we return YES ... use notification instead?
    return YES;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
    mTextFieldImpl->textDone();
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    activeTextField = nil;
    [textField resignFirstResponder];
    return YES;
}

@end

TextFieldImpl::TextFieldImpl( TextField *textField )
{
    mTextField = textField;
    angle=0;
    prevTransform=CGAffineTransformIdentity;
    TFDelegate *tfDelegate = [[TFDelegate alloc] init];
    tfDelegate->mTextFieldImpl = this; // to send textChanged, textDone
    
    UIWindow *window = [[UIApplication sharedApplication] keyWindow];
    // TODO: getter/setters for all the basic attributes of UITextField, position, etc.
    uiTextField = [[UITextField alloc] initWithFrame:CGRectMake(25, 35, 100, 31)];	
    uiTextField.clearButtonMode=UITextFieldViewModeWhileEditing;
    uiTextField.borderStyle = UITextBorderStyleRoundedRect; // or maybe UISearchBar instead?
    uiTextField.delegate = tfDelegate;
    uiTextField.keyboardType = UIKeyboardTypeDefault;
    uiTextField.returnKeyType = UIReturnKeyDone; 
    uiTextField.textAlignment = UITextAlignmentLeft;	
//    uiTextField.placeholder = @"Username\n";
    uiTextField.autocorrectionType = UITextAutocorrectionTypeNo;
    uiTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    //uiTextField.frame=CGRectMake(25, 200, 100, 31);
    
    
    [window addSubview: uiTextField];        
}

std::string TextFieldImpl::getText()
{
    return std::string([uiTextField.text UTF8String]);
}

void TextFieldImpl::setPosition(float x, float y, float length, float height)
{
    //uiTextField.frame=CGRectMake(x, y, length, height);
    NSString *tmp=uiTextField.text;
    TFDelegate *tfDelegate = [[TFDelegate alloc] init];
    tfDelegate->mTextFieldImpl = this;
    UIWindow *window = [[UIApplication sharedApplication] keyWindow];
    [uiTextField removeFromSuperview];
    [uiTextField release];
    uiTextField=nil;
    
    uiTextField = [[UITextField alloc] initWithFrame:CGRectMake(x, y, length, height)];	
    uiTextField.clearButtonMode=UITextFieldViewModeWhileEditing;
    uiTextField.borderStyle = UITextBorderStyleRoundedRect; // or maybe UISearchBar instead?
    uiTextField.delegate = tfDelegate;
    uiTextField.keyboardType = UIKeyboardTypeDefault;
    uiTextField.returnKeyType = UIReturnKeyDone; 
    uiTextField.textAlignment = UITextAlignmentLeft;
	
    //    uiTextField.placeholder = @"Username\n";
    uiTextField.autocorrectionType = UITextAutocorrectionTypeNo;
    uiTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    
    uiTextField.text=tmp;
    [window addSubview: uiTextField];
    switch ([[UIDevice currentDevice] orientation]) {
        case UIDeviceOrientationPortrait:
            [UIApplication sharedApplication].statusBarOrientation = UIInterfaceOrientationPortrait;
            uiTextField.transform = CGAffineTransformMakeRotation(0);
            prevTransform=CGAffineTransformIdentity;
            break;
        case UIDeviceOrientationLandscapeLeft:
            [UIApplication sharedApplication].statusBarOrientation = UIInterfaceOrientationLandscapeRight;
            uiTextField.transform = CGAffineTransformMakeRotation(M_PI * (90.0 / 180.0));
            prevTransform=CGAffineTransformMakeRotation(-M_PI * (90.0 / 180.0));
            break;
        case UIDeviceOrientationLandscapeRight:
            [UIApplication sharedApplication].statusBarOrientation = UIInterfaceOrientationLandscapeLeft;
            uiTextField.transform = CGAffineTransformMakeRotation(-M_PI * (90.0 / 180.0));
            prevTransform=CGAffineTransformMakeRotation(M_PI * (90.0 / 180.0));
            break;
        case UIDeviceOrientationPortraitUpsideDown:
            [UIApplication sharedApplication].statusBarOrientation = UIInterfaceOrientationPortraitUpsideDown;
            uiTextField.transform = CGAffineTransformMakeRotation(M_PI * (90.0 / 180.0)*2);
            prevTransform=CGAffineTransformMakeRotation(M_PI * (90.0 / 180.0)*2);
            break;
        default:
            break;
    }
    //uiTextField.transform = CGAffineTransformMakeRotation(M_PI * (90.0 / 180.0));
    
}

void TextFieldImpl::hideTextField(bool decision)
{
    uiTextField.hidden=decision;
    [uiTextField resignFirstResponder];
}

void TextFieldImpl::textChanged()
{
    mTextField->mCbTextChanged.call( mTextField );
}

void TextFieldImpl::textDone()
{
    mTextField->mCbTextDone.call( mTextField );
}

TextField::~TextField()
{
    delete textFieldImpl;
}

void TextField::setup()
{
    textFieldImpl = new TextFieldImpl( this );
}

std::string TextField::getText()
{
    return textFieldImpl->getText();
}

void TextField::setPosition(float x, float y, float length, float height)
{
    textFieldImpl->setPosition(x, y, length, height);
}

void TextField::hideTextField(bool decision)
{
    textFieldImpl->hideTextField(decision);
    
}
