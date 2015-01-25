//
//  MotionDetector.h
//  pseye_
//
//  Created by Yuhei Akamine on 2014/12/28.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef pseye__MotionDetector_h
#define pseye__MotionDetector_h

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "PSMoveTracker.h"
#include "psmove.h"


class MotionDetect
{
    std::vector<cv::Point2f> motion_;
    
    float thre_ = 3;
    int i = 1;
    cv::Point2f prevPos_;
    
    int objWidth_, objHeight_;
    int winWidth_, winHeight_;
    
    double vmin_;
    double distX_, distY_, v_, vmax_;
    double alpha_ = 0.2;
    double duration1_, duration2_;
    
    std::chrono::system_clock::time_point start1_;
    std::chrono::system_clock::time_point start2_;
    
    double match2Cir_;  double match2Tri_;  double match2Squa_;
    bool drawJudge_;
    
    std::vector<Point2f> pos_;
    std::vector<Point3f> track_;
    
    cv::Mat imgLines_;
    cv::Mat grayCircle_, grayTriangle_, graySquare_;
    cv::Mat imgCircle_;
    cv::Mat imgTriangle_;
    cv::Mat imgSquare_;
    
    PSEyeCapture eye;
    
    
public:
    MotionDetect() {
        objWidth_ = 60; objHeight_ = 200;
        
        winWidth_ = eye.size().width;   winHeight_ = eye.size().height;
        
        vmin_ = 1.0;    v_ = 0.0;   vmax_ = 0.0;
        distX_ = 0.0;   distY_ = 0.0;
        
        start1_ = std::chrono::system_clock::now();
        duration1_ = 0.0;   duration2_ = 0.0;

        match2Cir_ = 0.0;   match2Tri_ = 0.0;   match2Squa_ = 0.0;
        
        drawJudge_ = false;
        
        imgLines_ = cv::Mat::zeros(winHeight_, winWidth_, CV_8UC3);
        imgCircle_ = cv::Mat::zeros(winHeight_, winWidth_, CV_8UC3);
        imgTriangle_ = cv::Mat::zeros(winHeight_, winWidth_, CV_8UC3);
        imgSquare_ = cv::Mat::zeros(winHeight_, winWidth_, CV_8UC3);
        MotionDetect::makeTeacher(winWidth_, winHeight_);
    }
    
    void init() {
        objWidth_ = 60; objHeight_ = 200;
        
        winWidth_ = eye.size().width;   winHeight_ = eye.size().height;
        
        vmin_ = 1.0;    v_ = 0.0;   vmax_ = 0.0;
        distX_ = 0.0;   distY_ = 0.0;
        
        start1_ = std::chrono::system_clock::now();
        duration1_ = 0.0;   duration2_ = 0.0;
        
        match2Cir_ = 0.0;   match2Tri_ = 0.0;   match2Squa_ = 0.0;
        
        drawJudge_ = false;
    }
    
    void setThreshold(float t) { thre_ = t; }
    
    bool detect(MotionTracker& tracker)
    {
        Point2f pos = tracker.pos();
        
        if(norm(pos-prevPos_) > thre_) {
            motion_.push_back(pos);
        }else {
            motion_.clear();
        }
        
        prevPos_ = pos;
        return true;
    }
    
    void draw(cv::Mat& img) {
        for(int i=0; i<(int)motion_.size()-1; ++i) {
            cv::line(img, motion_[i], motion_[i+1], Scalar::all(255));
        }
    }
    
    bool drawJudging() {
        if ((int)motion_.size() > 1) {
            distX_ = motion_[(int)motion_.size()-1].x - motion_[(int)motion_.size()-2].x;
            distY_ = motion_[(int)motion_.size()-1].y - motion_[(int)motion_.size()-2].y;
            v_ = sqrt(distX_*distX_ + distY_*distY_);
            //cout << "Speed = " << v << endl;
        }else{
            distX_ = 0.0;  distY_ = 0.0;  v_ = 0.0;
        }
        
        if (vmin_ > v_) {
            std::chrono::system_clock::time_point end1_ = chrono::system_clock::now();
            std::chrono::duration<double> sec1_ = end1_ - start1_;
            duration1_ = sec1_.count();
            cout << "Duration = " << duration1_ << endl;
        }else{
            start1_ = std::chrono::system_clock::now();
        }
        
        if (duration1_ > 2.0 && vmin_ < v_) {
            cout << "DRAWING" << endl;
            drawJudge_ = true;
            start2_ = std::chrono::system_clock::now();
            
            if (vmax_ < v_) {
                vmax_ = v_;
                //cout << "Vmax = " << vmax << endl;
            }
        }
        
        if (vmax_*alpha_ > v_) {
            std::chrono::system_clock::time_point end2_ = std::chrono::system_clock::now();
            std::chrono::duration<double> sec2_ = end2_ - start2_;
            duration2_ = sec2_.count();
            cout << "Stop = " << duration2_ << endl;
        }else{
            start2_ = chrono::system_clock::now();
        }
        
        if (duration2_ > 1.0) {
            cout << "FINISH" << endl;
            drawJudge_ = false;
            start2_ = chrono::system_clock::now();
            duration2_ = 0.0;
        }
        
        return drawJudge_;
    }
    
    int matching(cv::Mat& drawImage) {
        cvtColor(drawImage, drawImage, COLOR_RGB2GRAY);
        
        match2Cir_ = matchShapes(drawImage, grayCircle_, 1, 0);
        match2Tri_ = matchShapes(drawImage, grayTriangle_, 1, 0);
        match2Squa_ = matchShapes(drawImage, graySquare_, 1, 0);
        
        if (match2Cir_ < match2Tri_ && match2Cir_ < match2Squa_) {
            return 1;
        }else if (match2Tri_ < match2Cir_ && match2Tri_ < match2Squa_) {
            return 2;
        }else if (match2Squa_ < match2Cir_ && match2Squa_ < match2Tri_) {
            return 3;
        }
        
        return 0;
    }
    
    void makeTeacher(int width, int height) {
        circle(imgCircle_, Point(width/2,height/2), 200, Scalar::all(255));
        
        cv::Point pt[3];
        pt[0] = cv::Point(320,100);  pt[1] = cv::Point(100,380);  pt[2] = cv::Point(540,380);

        line(imgTriangle_, pt[0], pt[1], Scalar::all(255));
        line(imgTriangle_, pt[1], pt[2], Scalar::all(255));
        line(imgTriangle_, pt[2], pt[0], Scalar::all(255));
        
        rectangle(imgSquare_, Point(100,100), Point(540,380), Scalar::all(255));
        
        cvtColor(imgCircle_, grayCircle_, COLOR_RGB2GRAY);
        cvtColor(imgTriangle_, grayTriangle_, COLOR_RGB2GRAY);
        cvtColor(imgSquare_, graySquare_, COLOR_RGB2GRAY);
    }
    
    
    std::vector<Point3f> getTrack() {
        //cout << "motion" << motion_ << endl;
        for (int i=0; i<motion_.size(); i++) {
            if (motion_[i].y < winHeight_/2) {
                if (motion_[i].x > winWidth_/2 - objWidth_/2 && motion_[i].x < winWidth_/2 + objWidth_/2) {
                    track_[i].x = -100.0;
                    track_[i].y = -100.0;
                    track_[i].z = 0.0;
                }
            }else{
                track_[i].x = motion_[i].x;
                track_[i].y = motion_[i].y;
                track_[i].z = (objHeight_ / motion_.size()) * (i+1);
            }
        }

        //cout << "track" << track_ << endl;
        return track_;
    }
};

#endif
