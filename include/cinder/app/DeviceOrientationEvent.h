//
//  DeviceOrientationEvent.h
//  cinder
//
//  Created by  on 10/08/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/Event.h"

typedef enum{
    DeviceOrientationUnknown,
    DeviceOrientationPortrait, 
    DeviceOrientationPortraitUpsideDown,  
    DeviceOrientationLandscapeLeft,       
    DeviceOrientationLandscapeRight,      
    DeviceOrientationFaceUp,              
    DeviceOrientationFaceDown  
}Orientation;

namespace cinder { namespace app {
    
    class DeviceOrientationEvent : public Event {
    public:
        DeviceOrientationEvent(){};
        DeviceOrientationEvent(const Orientation &orientation):Event(), mOrientation(orientation){};
        
        Orientation getDeviceOrientation(){return mOrientation;}
        std::string      getDeviceOrientationString()
        {
            switch (mOrientation) {
                case DeviceOrientationUnknown:
                    return "DeviceOrientationUnknown";
                    break;
                case DeviceOrientationPortrait:
                    return "DeviceOrientationPortrait";
                    break; 
                case DeviceOrientationPortraitUpsideDown:
                    return "DeviceOrientationPortraitUpsideDown";
                    break;
                case DeviceOrientationLandscapeLeft:
                    return "DeviceOrientationLandscapeLeft";
                    break;
                case DeviceOrientationLandscapeRight:
                    return "DeviceOrientationLandscapeRight";
                    break;
                case DeviceOrientationFaceUp:
                    return "DeviceOrientationFaceUp";
                    break;
                case DeviceOrientationFaceDown:
                    return "DeviceOrientationFaceDown";
                    break;
                default:
                    break;
            }
            return "DeviceOrientationUnknown";
        }
    private:
        
        Orientation mOrientation;   
    };  
}}
