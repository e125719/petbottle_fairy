//
//  MotionTracker.h
//  pseye_
//
//  Created by Yuhei Akamine on 2014/12/28.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef pseye__PSMoveTracker_h
#define pseye__PSMoveTracker_h

#include <opencv2/opencv.hpp>

#include "psmove.h"
#include "Labeling.h"
typedef Labeling<uchar, uchar> Labeling8U;

#include "MotionTracker.h"

class PSMoveTracker : public MotionTracker
{
    PSMove* move;
    
    struct {
        int r,g,b;
    } ledColor, detectionColor;
    
    struct BGR {
        uchar b,g,r;
    };
    
    int thre;
    cv::Point2f pos_;
public:
    PSMoveTracker() {
        thre = 150;
        
        ledColor.r = 0;
        ledColor.g = 255;
        ledColor.b = 255;
        
        detectionColor = ledColor;
    }
    
    void init()
    {
        if (!psmove_init(PSMOVE_CURRENT_VERSION)) {
            fprintf(stderr, "PS Move API init failed (wrong version?)\n");
            exit(1);
        }
        
        int i = psmove_count_connected();
        printf("Connected controllers: %d\n", i);
        
        move = psmove_connect();
        if (move == NULL) {
            printf("Could not connect to default Move controller.\n"
                   "Please connect one via USB or Bluetooth.\n");
            exit(1);
        }
        
        char *serial = psmove_get_serial(move);
        printf("Serial: %s\n", serial);
        free(serial);

        
        enum PSMove_Connection_Type  ctype = psmove_connection_type(move);
        switch (ctype) {
            case Conn_USB:
                printf("Connected via USB.\n");
                break;
            case Conn_Bluetooth:
                printf("Connected via Bluetooth.\n");
                break;
            case Conn_Unknown:
                printf("Unknown connection type.\n");
                break;
        }
    }
    
    void setSphereColor(int r, int g, int b)
    {
        ledColor.r = r;
        ledColor.g = g;
        ledColor.b = b;
    }
    
    void setThreshold(int t)
    {
        thre = t;
    }
    
    template <class CAP>
    void calibrate(CAP& cap)
    {
        printf("Spot your move to center of screen. then press any key.");

        Mat m;
        while(1) {
            psmove_set_leds(move, ledColor.r, ledColor.g, ledColor.b);
            //psmove_set_rumble(move, 255*(i%2));
            psmove_update_leds(move);
            
            cap >> m;
            cv::imshow("calibration", m);
            
            int k = cv::waitKey(1);
            if(k >= 0)
                break;
        }
        
        BGR col = m.at<BGR>(Point2f(m.size().width/2, m.size().height/2));
        
        detectionColor.r = col.r;
        detectionColor.g = col.g;
        detectionColor.b = col.b;
        
        cv::rectangle(m, cv::Rect(Point(), m.size()), Scalar(col.b, col.g, col.r), FILLED);
        imshow("calibration", m);
        waitKey(1);
    }
    
    void track(cv::Mat& bgr_image)
    {
        Labeling8U labeling;
        
        cv::Mat mask(bgr_image.size(), CV_8U);
        cv::Mat bgr = bgr_image;
        
        psmove_set_leds(move, ledColor.r, ledColor.g, ledColor.b);
        // psmove_set_rumble(move, 255*(i%2));
        psmove_update_leds(move);
        
#if 1
        for(int i=0; i<bgr.size().area(); ++i) {
            BGR& p = bgr.at<BGR>(i);
            
            if(abs(p.r-detectionColor.r) + abs(p.g-detectionColor.g) + abs(p.b-detectionColor.b) < thre)
            {
                mask.at<uchar>(i) = 255;
            }else {
                mask.at<uchar>(i) = 0;
            }
        }
        
        imshow("mask", mask);
#else
        {
            vector<Mat> chn;
            split(bgr, chn);
            
            mask = abs(chn[0]-Scalar(0))+abs(chn[1]-Scalar(255))+abs(chn[2]-Scalar(0));
            mask = 255-mask*3;
            //imshow("dif", mask);
        }
#endif
        
        /*            
         GaussianBlur( mask, mask, Size(9, 9), 2, 2 );
         
         vector<Vec3f> circles;
         HoughCircles(mask, circles, HOUGH_GRADIENT, 2, 120, 200, 100);
         
         for(auto& c : circles) {
         circle(bgr, Point(c[0], c[1]), c[2], Scalar::all(255));
         }*/
        
        cv::Mat labeled(mask.size(), CV_8U);
        
        labeling.Exec(mask.ptr(), labeled.ptr(), mask.size().width, mask.size().height, false, 10);
        /*
        for(int i=0; i<labeling.GetNumOfResultRegions(); ++i) {
            Labeling8U::RegionInfo* ri = labeling.GetResultRegionInfo(i);
            
            float x,y;
            ri->GetCenterOfGravity(x, y);
            
            int sx, sy;
            ri->GetSize(sx, sy);
            
            circle(bgr, Point(x,y), (sx+sy)/4, Scalar::all(255));
        }
        */
        
        if(labeling.GetNumOfResultRegions() > 0) {
            Labeling8U::RegionInfo* ri = labeling.GetResultRegionInfo(0);
            
            ri->GetCenter(pos_.x, pos_.y);
        }
    }
    
    static
    int pair(const char *custom_addr = 0)
    {
        int count = psmove_count_connected();
        int i;
        PSMove *move;
        int result = 0;
        
        printf("Connected controllers: %d\n", count);
        
        for (i=0; i<count; i++) {
            move = psmove_connect_by_id(i);
            
            if (move == NULL) {
                printf("Error connecting to PSMove #%d\n", i+1);
                result = 1;
                continue;
            }
            
            if (psmove_connection_type(move) != Conn_Bluetooth) {
                printf("PSMove #%d connected via USB.\n", i+1);
                int result = 0;
                
                if (custom_addr != NULL) {
                    result = psmove_pair_custom(move, custom_addr);
                } else {
                    result = psmove_pair(move);
                }
                
                if (result) {
                    printf("Pairing of #%d succeeded!\n", i+1);
                    char *serial = psmove_get_serial(move);
                    printf("Controller address: %s\n", serial);
                    free(serial);
                } else {
                    printf("Pairing of #%d failed.\n", i+1);
                }
                
                if (psmove_has_calibration(move)) {
                    printf("Calibration data available and saved.\n");
                } else {
                    printf("Error reading/loading calibration data.\n");
                }
            } else {
                printf("Ignoring non-USB PSMove #%d\n", i+1);
            }
            
            psmove_disconnect(move);
        }
        
        return result;
    }

    Point2f virtual pos() { return pos_; }
};

#endif
