//
//  Controler.h
//  level7
//
//  Created by Yume OOHAMA on 2015/01/20.
//  Copyright (c) 2015年 Yume OOHAMA. All rights reserved.
//

#ifndef __level7__Controler__
#define __level7__Controler__

#include <stdio.h>
#include "ObjDetect.h"
#include "MotionDetector.h"
#include "App.h"

class Controller{
public:
    Controller(){state_ = OBJECT_DETECT;}
    void update();
    void draw();
    void init(ObjDetect *object,MotionDetect *motion,Fairy *fairy); //初期化
    
private:
    
    enum STATE {
        OBJECT_DETECT,
        MOTION_DETECT,
        ANIMATION
    };
    
    STATE state_;
    ObjDetect *objDetector_;
    MotionDetect *motDetector_;
    Fairy *fairy_;
    
};

#endif /* defined(__level7__Controler__) */

