//
//  main.cpp
//  pseye
//
//  Created by Yuhei Akamine on 2014/12/27.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#include <iostream>
#include <unistd.h>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "psmove_config.h"
#include "PSEyeCapture.h"
#include "PSMoveTracker.h"
#include "MotionDetector.h"

#include "Timer.h"


#define DO_PAIRING 1
/*
void setupEye(PSEyeCapture& eye)
{
     eye.driver()->setGain(10);
     eye.driver()->setExposure(20);
     eye.driver()->setAutoWhiteBalance(0);
     eye.driver()->setBlueBalance(120);
     eye.driver()->setRedBalance(120);
}

int main(int argc, const char * argv[]) {

#if DO_PAIRING
    PSMoveTracker::pair();
#endif
    
    PSEyeCapture eye;
    eye.open(320*2,240*2);
    setupEye(eye);
    
    PSMoveTracker tracker;
    tracker.init();
    tracker.setThreshold(50);
    MotionDetect detect;
    
    tracker.calibrate(eye);
    
    int camWidth = eye.size().width;
    int camHeight = eye.size().height;
    
    Timer timer;
    
    detect.init(camWidth, camHeight);
    
    detect.makeTeacher(camWidth, camHeight);
    
    while(1) {
        
        Mat img, imgDraw = Mat::zeros(camHeight, camWidth, CV_8UC3);
        
        eye >> img;
        
        if(!img.empty()) {
            
            tracker.track(img);
            detect.detect(tracker);
            
            detect.draw(img);

            imshow("preview", img);

            bool draw = detect.drawJudging(camWidth, camHeight);
            
            if (draw) {
                detect.draw(imgDraw);
            }

           int dtcShape = detect.matching(imgDraw);
                
           if (dtcShape == 1) {
               // Gesture is CIRCLE
               cout << "Circle" << endl;
           }else if (dtcShape == 2) {
               // Gesture is TRIANGLE
               cout << "Triangle" << endl;
           }else if (dtcShape == 3) {
               // Gesture is SQUARE
               cout << "Square" << endl;
           }
            
            imshow("draw", imgDraw);
        }
        waitKey(1);
    }
    
    return 0;
}
 */


// class
class MotionDetector {
    int camWidth_, camHeight_;
    int objectWid_, objectHei_;
    int dtcShape_;
    bool judgeDraw_;
    
    PSEyeCapture eye;
    PSMoveTracker tracker;
    MotionDetect detect;
    Timer timer;
    
public:
    MotionDetector() {
        #if DO_PAIRING
        PSMoveTracker::pair();
        #endif
        
        eye.open(320*2,240*2);
        setupEye(eye);
        
        camWidth_ = eye.size().width;
        camHeight_ = eye.size().height;
        
        tracker.init();
        tracker.setThreshold(50);
        tracker.calibrate(eye);
    }
    
    void setupEye(PSEyeCapture& eye)
    {
        eye.driver()->setGain(10);
        eye.driver()->setExposure(20);
        eye.driver()->setAutoWhiteBalance(0);
        eye.driver()->setBlueBalance(120);
        eye.driver()->setRedBalance(120);
    }
    
    bool MotionDetecting() {
        while(1) {
            Mat img_;
            Mat imgDraw_ = Mat::zeros(camWidth_, camHeight_, CV_8UC3);
            
            detect.returnSize(objectWid_, objectHei_);
            
            eye >> img_;
            
            if (!img_.empty()) {
                tracker.track(img_);
                detect.detect(tracker);
                
                detect.draw(img_);
                imshow("preview", img_);
                
                judgeDraw_ = detect.drawJudging(camWidth_, camHeight_);
                
                /*
                if (judgeDraw_) {
                    detect.draw(imgDraw_);
                }
                 */
                
                dtcShape_ = detect.matching(img_);
                
                if (dtcShape_ == 1) {
                    cout << "Circle" << endl;
                    break;
                }else if (dtcShape_ == 2) {
                    cout << "Triangle" << endl;
                    break;
                }else if (dtcShape_ == 3) {
                    cout << "Square" << endl;
                    break;
                }
                
                //imshow("draw", imgDraw_);
            }
            
            waitKey(1);
        }
        
        return true;
    }
};
