//
//  Timer.h
//  libmv_xcode
//
//  Created by Yuhei Akamine on 2014/10/03.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef libmv_xcode_Timer_h
#define libmv_xcode_Timer_h

#include <chrono>
class Timer
{
    std::chrono::system_clock::time_point start_;
    
public:
    Timer() {
        start_ = std::chrono::system_clock::now();
    }
    
    double duration_sec() {
        auto now = std::chrono::system_clock::now();
        auto dur = now - start_;
        start_ = now;
        return (double)std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() / 1000.0;
    }
    
    std::chrono::system_clock::time_point getStart() {
        return start_;
    }
    
};

#endif
