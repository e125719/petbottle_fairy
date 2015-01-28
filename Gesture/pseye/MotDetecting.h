//
//  MotDetecting.h
//  pseye_
//
//  Created by e125719 on 2015/01/28.
//  Copyright (c) 2015年 Yuhei Akamine. All rights reserved.
//

#ifndef pseye__MotDetecting_h
#define pseye__MotDetecting_h

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


class MotionDetector {
    int camWidth_, camHeight_;
    int objectWid_, objectHei_;
    float objectMom_;
    int dtcShape_;
    bool judgeDraw_;
    vector<Point3f> tracking_;
    
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
            
            detect.returnSize(objectWid_, objectHei_, objectMom_);
            
            eye >> img_;
            
            if (!img_.empty()) {
                tracker.track(img_);
                detect.detect(tracker);
                
                detect.draw(img_);
                imshow("preview", img_);
                
                judgeDraw_ = detect.drawJudging();
                
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
    
    std::vector<Point3f> getTrack() {
        //cout << "track" << track_ << endl;
        
        std::vector<Point3f> track1_(tracking_.size());
        
        for (int i=0; i<tracking_.size(); i++) {
            if (tracking_[i].x > camWidth_/2 - objWidth_/2 && tracking_[i].x < camWidth_/2 + objWidth_/2) {
                if (tracking_[i].y < camHeight_/2) {
                    track1_[i].x = -100.0;
                    track1_[i].y = -100.0;
                    track1_[i].z = 0.0;
                }
            }else{
                track1_[i].x = tracking_[i].x;
                track1_[i].y = tracking_[i].y;
                track1_[i].z = (objHeight_ / tracking_.size()) * (i+1);
            }
        }
        
        /*
        for (int i=0; i<track1_.size(); i++) {
            cv::circle(imgPoint_, Point(track1_[i].x, track1_[i].z), 3, Scalar::all(255), -1);
            imshow("Point", imgPoint_);
            waitKey(1);
        }
        
        cout << "track1_ = " << track1_ << endl;
        cout << "track_ = " << track_ << endl;
        */
         
        return track1_;
    }
};

#endif
