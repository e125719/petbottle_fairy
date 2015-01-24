//
//  PSEyeCapture.h
//  pseye_
//
//  Created by Yuhei Akamine on 2014/12/28.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef pseye__PSEyeCapture_h
#define pseye__PSEyeCapture_h

#include <opencv2/opencv.hpp>

#include "ps3eyedriver.h"
#include "ps3eye.h"


class PSEyeCapture
{
    int width_;
    int height_;
    
    ps3eye_t* pseye_;
public:
    PSEyeCapture() {}
    
    void open(int w, int h, int num = 0) {
        width_ = w;
        height_ = h;
        
        ps3eye_init();
        pseye_ = ps3eye_open(num, w, h, 30);
    }
    
    ps3eye::PS3EYECam* driver() { return pseye_->eye.get(); }
    
    cv::Size size() { return cv::Size(width_, height_); }
    
    //! capture one frame as YUV422(YUV_YUNV)
    cv::Mat captureRaw() {
        int stride;
        uchar* frame;
        
        frame = ps3eye_grab_frame(pseye_, &stride);
        if(frame == NULL) {
            return Mat();
        }
        
        Mat raw(Size(width_, height_), CV_8UC2, frame, stride);
        
        return raw;
    }
    
    cv::Mat captureBGR() {
        cv::Mat raw = captureRaw();
        cv::Mat bgr;
        
        if(raw.empty())
            return Mat();
        
        cvtColor(raw, bgr, COLOR_YUV2BGR_YUNV);
        return bgr;
    }
};

inline bool operator >> (PSEyeCapture& cap, cv::Mat& m)
{
    m = cap.captureBGR();
    
    return !m.empty();
}
#endif
