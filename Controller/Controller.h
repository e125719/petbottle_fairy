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
#include "App.h"

class Controller{
public:
    Controller(){state_ = OBJECT_DETECT;}
    void update();
    void draw();
    void init();
private:
    
    enum STATE {
      OBJECT_DETECT,
      MOTION_DETECT,
      ANIMATION
    };
    
    STATE state_;
    ObjectDetector *objDetector_;
    MotionDetector *motDetector_;
    
};

#endif /* defined(__level7__Controler__) */

