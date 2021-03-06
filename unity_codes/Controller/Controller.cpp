//
//  Controler.cpp
//  level7
//
//  Created by Yume OOHAMA on 2015/01/20.
//  Copyright (c) 2015年 Yume OOHAMA. All rights reserved.
//

#include "Controller.h"

void Controller::update(){
    
    
    if (state_==OBJECT_DETECT) {
        objDetect *detect;
        detect = new objDetect();
        detect -> objDetect::detectObject();
        // detectObjectの返り値がtrueならstate_をMOTION_DETECTにかえる
        state_ = MOTION_DETECT;
        
    }else if (state_==MOTION_DETECT){
        MotionDetector *motion;
        motion = new MotionDetector();
        motion -> MotionDetector::detect();
        // MotionDetector::detectの返り値がtrueならstate_をANIMATIONにかえる
        state_ = ANIMATION;
        
    }else{ //state_ == ANIMATION
        
        
    }
    
}

void Controller::draw(){
    // appから呼ぶ
    return;
}

void Controller::init(ObjectDetecter *object, MotionDetector *motion) {
    objDetector_ = object;
    motDetector_ = motion;
}