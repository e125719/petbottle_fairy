#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <cv.h>
#include <highgui.h>

using namespace std;

class objDetect{
    
    cv::Mat backImg;
    cv::Mat frame;
    cv::Mat grayFrame;
    cv::Mat diff;
    cv::Mat texture;
    float area1 = 0;
    float area = 0;
    cv::Mat brect;
    int roiCnt = 0;
    int i = 0;
    
    float cx;
    float cy;
    
public:
    
    objDetect::objDetect(void) {
        // Constructer
    }
    
    int getBackImg(){
        
        cout << "please press key" << endl;
        
        getchar();
        
        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            cerr << "camera cannot open" << endl;
            return -1;
        }
        cap >> backImg;
        
        cv::cvtColor(backImg, backImg, CV_BGR2GRAY);
        
        return 0;
        
    }
    
    
    int detectObject(){
        cv::VideoCapture cap1(0);
        if (!cap1.isOpened())
        {
            cerr << "camera cannot open" << endl;
            return -1;
        }
        cap1 >> frame;
        
        cv::cvtColor(frame, grayFrame, CV_BGR2GRAY);
        cv::absdiff(backImg,grayFrame,diff);
        cv::threshold(diff,diff,20,255,CV_THRESH_BINARY);
        
        cv::erode(diff,diff,getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
        cv::dilate(diff,diff,getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
        cv::dilate(diff,diff,getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
        cv::erode(diff,diff,getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5)));
        
        vector< vector< cv::Point > > contours;
        cv::findContours(diff, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        
        for (auto contour = contours.begin(); contour != contours.end(); contour++){
            vector< cv::Point > approx;
            cv::approxPolyDP(cv::Mat(*contour), approx, 0.01 * cv::arcLength(*contour, true), true);
            area1 = cv::contourArea(approx);
        }
        
        if(area1 > 1000)
            return 1;
        else
            return 0;
        
    }
    
    
    void makeTexture(){
        vector< vector< cv::Point > > contours;
        vector< cv::Point > approx;
        cv::findContours(diff, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        
        cv::Moments mom = cv::moments(contours[i]);
        cx = mom.m10/mom.m00;
        cy = mom.m01/mom.m00;
        
        cout << "重心座標（"<< cx << "," << cy << "）" << endl;
        
        for (auto contour = contours.begin(); contour != contours.end(); contour++){
            cv::convexHull(*contour, approx);
            area = cv::contourArea(approx);
            if (area > 1000.0){
                cv::Rect brect = cv::boundingRect(cv::Mat(approx).reshape(2));
                texture = cv::Mat(frame, brect);
                
                
                roiCnt++;
                if (roiCnt == 99){
                    break;
                }
            }
            i++;
        }
    }
    
    cv::Mat getTexture(){
        
        return texture;
        
    }
    
    float getPosition(){
    
        return cx;
    
    }
    
    
    
};