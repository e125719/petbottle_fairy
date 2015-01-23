//
//  App.h
//  rge_sample
//
//  Created by Yuhei Akamine on 2014/10/28.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef __rge_sample__App__
#define __rge_sample__App__

#include <vector>
#include <SDL2/SDL.h>
#include <math.h>
//#include "MotionDetector.h"
//#include "objDetector.h"
#include "RGE.h"
#include "Frame.h"
#include "OpenGLTexture.h"

//妖精の軌跡の表示
class Fairy
{
    std::vector<rge::rgeVector3> track_;
    
    int step_;
    rge::rgeVector3 pos_;
    bool visible_;
    double direction_;
    
public:
    Fairy() {
        step_ = 0;
        visible_ = false;
        direction_ = 0;
    }
    
    rge::rgeVector3 getPosition() {
        return pos_;
    }
    
    double getDirection(){
        return direction_;
    }
    
    //モデルの表示確認
    bool isVisible(){
        return visible_;
    }
    
    void setTrack(const std::vector<rge::rgeVector3>& track) {
        track_ = track;
        step_ = 0;
        visible_ = true;
    }
    
    void update() {
        if (visible_ == true){
            rge::rgeVector3 pos = track_[step_];
            
            rge::rgeVector3 next_point = track_[step_+1];
            
            direction_ = atan2(next_point.y - pos.y, next_point.x - pos.x)/M_PI * 180;
            
            step_++;
            pos.z = sin(M_PI/30 * step_);
            //妖精の位置にゆらぎを加えるsin波
            pos_ = pos;
        }
        
        //移動し終えたか？
        if(step_+1 == track_.size() ) {
            
            //移動終了の処理
            //消える処理
            visible_ = false;
            return;
            
        }
    }
    
};


class App
{
    std::vector<rge::TextureRef> textures_;
    rge::TextureRef liveTexture_;
    int selectedTexture_ = 0;
    float poseRatio_ = 0;
public:
    App();
    void init();
    void applyWindowSize(int w, int h);
    void update();
    void display();
    
    void mouseLeftPressed(int x, int y);
    void mouseLeftReleased(int x, int y);
    void mouseLeftDragged(int x, int y);
    void dummyData(float x, float y ,float rot);
    
private:
    rge::FrameRef robotbase;
    Fairy fairy_;
//    Controller controller_;
//    MotionDetector objDetector;
//    ObjectDetector motDetector;
    void readTextures();
    
        OpenGLTexture texture_;
    
    
};


#endif /* defined(__rge_sample__App__) */
