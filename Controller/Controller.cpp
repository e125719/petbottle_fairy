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
        
        if(objDetector_->detectObject()){
            state_ = MOTION_DETECT;
        }
        
    }else if (state_==MOTION_DETECT){
        
        if(motDetector_->motiondetecting){
            state_ = ANIMATION;
        }
    }else{ //state_ == ANIMATION
        fairy_->setTrack(motDetector_->getTrack());
        fairy_->setTexture(objDetector_->getTexture());
        
    }
    
}

void Controller::draw(){
    // appから呼ぶ
    return;
}

void Controller::init(ObjDetect *object,MotionDetect *motion, Fairy *fairy) {
    objDetector_ = object;
    motDetector_ = motion;
    fairy_ = fairy;
}