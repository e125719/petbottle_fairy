//
//  getPosition.h
//  rge_sample
//
//  Created by 稲福也美 on 2015/01/20.
//  Copyright (c) 2015年 Yuhei Akamine. All rights reserved.
//

#ifndef rge_sample_getPosition_h_
#define rge_sample_getPosition_h_

#include <vector>
#include <SDL2/SDL.h>
#include <math.h>
#include "RGE.h"
#include "Frame.h"
using namespace rge;

class getPosition
{
    
    double radian;
    int r;
    //rot = 0;    //円の角度(0~360)

public:
    void rotation(float x, float y, float rot) {
        r = 3;     //円の半径
        
        radian = M_PI/180 * rot;
        x = x + cos(radian) * r;
        y = y + sin(radian) * r;
        
        //妖精を動かす
        RGE::getInstance()->setLayerVisibility(LAYER_2, false);
        robotbase = RGE::getInstance()->findFrame("body");
        
        robotbase->setTranslation(x,y,0);
        //robotbase->rotate(1,0,0);
    }
private:
    rge::FrameRef robotbase;
    
};



#endif
