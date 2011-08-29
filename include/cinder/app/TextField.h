//
//  TextField.h
//
//  Created by Tom Carden on 8/9/11.
//  Copyright 2011 Bloom Studio, Inc. All rights reserved.
//

#pragma once

#include <string>
#include "cinder/Function.h"

class TextFieldImpl;

class TextField {
    
public:
    
    TextField(): textFieldImpl(NULL) {};
    ~TextField();
    
    //void init(float x, float y, float length, float height);
    void setup();
    
    template<typename T>
    ci::CallbackId registerTextChanged( T *obj, bool (T::*callback)(TextField*) )
	{
		return mCbTextChanged.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}
    
    template<typename T>
    ci::CallbackId registerTextDone( T *obj, bool (T::*callback)(TextField*) )
	{
		return mCbTextDone.registerCb(std::bind1st(std::mem_fun(callback), obj));
	}
    
    std::string getText();
    void setPosition(float x, float y, float length, float height);
    void hideTextField(bool decision);
    
protected:
    
    friend class TextFieldImpl; // to save too much indirection firing callbacks
    
    TextFieldImpl *textFieldImpl; // to save from setting everything that touches this header as an objective-c++ file
    
    ci::CallbackMgr<bool(TextField*)> mCbTextChanged, mCbTextDone;
    
};