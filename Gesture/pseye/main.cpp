//
//  main.cpp
//  pseye
//
//  Created by Yuhei Akamine on 2014/12/27.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

//#include <iostream>
//#include <unistd.h>
//using namespace std;

//#include <opencv2/opencv.hpp>
//using namespace cv;

//#include "psmove_config.h"
//#include "PSEyeCapture.h"
//#include "PSMoveTracker.h"
//#include "MotionDetector.h"

//#include "Timer.h"

#include "MotDetecting.h"


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
 */

int main(int argc, const char * argv[]) {
/*
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
    */
    
    // hogehoge
    
    return 0;
}


// class
