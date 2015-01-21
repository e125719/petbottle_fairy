//
//  MotionTracker.h
//  pseye_
//
//  Created by Yuhei Akamine on 2014/12/28.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef pseye__MotionTracker_h
#define pseye__MotionTracker_h

#include <opencv2/opencv.hpp>

class MotionTracker
{
public:
    MotionTracker() {}

    virtual Point2f pos() = 0;
};
#endif
