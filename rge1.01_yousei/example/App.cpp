//
//  App.cpp
//  rge_sample
//
//  Created by Yuhei Akamine on 2014/10/28.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#include "App.h"

#include <fstream>
using namespace std;

#include <rge/RGE.h>
#include <math.h>
#include "trackball.h"
#include "rge/RgeReader.h"
using namespace rge;


App::App()
{
}


void App::init()
{
    
    //背景クリア色
    glClearColor(0.0, 0.0, 1.0, 1.0);
			 
    RGE::getInstance()->init();
    
    RgeReader reader;
    reader.read("sampleModels/youseimodel4.rge");
    
    if(RGE::getInstance()->findFrame("Armature"))
        RGE::getInstance()->findFrame("Armature")->setBonesVisibility(false);
    
    ofstream os("/tmp/ipo.csv");
    os << RGE::getInstance()->rootFrame()->getActionsDump() << endl;
    
    RGE::getInstance()->rootFrame()->printChildren();
    
    RGE::getInstance()->setBackgroundColor(color3(0,0,1));
    //RGE::getInstance()->rootFrame()->stopAnimation();
    RGE::getInstance()->rootFrame()->anim()->setLooping(true, true);
    
    //LAYER_2に属するオブジェクトを描画しない
    RGE::getInstance()->setLayerVisibility(LAYER_2, false);
    
    //ロボット描画
    robotbase = RGE::getInstance()->findFrame("body");
    
    
    //テクスチャ
    liveTexture_ = TextureRef(new ImageTexture);
    liveTexture_->createFromFile("sampleModels/pet.jpg");
    RGE::getInstance()->findMaterial("dress1")->setTexture(liveTexture_);
    
    dummyData(0, 0, 0);
}


void App::applyWindowSize(int w, int h)
{
    /* トラックボールする範囲 */
    trackballRegion(w, h);
    
    /* ウィンドウ全体をビューポートにする */
    //	glViewport(0, 0, w, h);
    RGE::getInstance()->setViewport(0, 0, w, h);
    
    //ウインドウのアスペクト比を設定
    RGE::getInstance()->setAspectRatio((double)w/h);
}

void App::update()
{
    RGE::getInstance()->update();
    //robotbase->setVelocity(5, 4, 0);
    //robotbase->rotate(0,0,0);
    //robotbase->setTranslation(0,0,0);
    fairy_.update();
    rgeVector3 position = fairy_.getPosition();
    robotbase->setTranslation(position);
    
    //setRotateだと絶対的な位置を指定できる！！
    robotbase->rotate(0,0,0);
}

void App::display()
{
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    /* モデルビュー変換行列の初期化 */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* トラックボール処理による回転(デバッグ用) */
    trackballRotation("Camera");
    
    //RGE(グラフィックエンジン）による描画
    glDisable(GL_BLEND);
    
    if(RGE::getInstance()->findLight("Spot"))
        SpotLight::upcast(RGE::getInstance()->findLight("Spot"))->setCastShadow(true);
    
    
    
    RGE::getInstance()->render();
}

void App::mouseLeftPressed(int x, int y)
{
    trackballStart(x, y);
}

void App::mouseLeftReleased(int x, int y)
{
    trackballStop(x,y);
}

void App::mouseLeftDragged(int x, int y)
{
    trackballMotion(x, y);
}



//妖精を回転させる
void App::dummyData(float x, float y ,float rot)
{
    double radian;
    int r;
    float z;
    
    r = 4;     //円の半径
    std::vector<rge::rgeVector3> dummy_track;
    
    for(int i=0; i<180;i++){
        radian = M_PI/180 * i;
        x = cos(radian) * r;
        y = sin(radian) * r;
        z = 0;
//      z = sin(radian * 4);
        
        dummy_track.push_back(rge::rgeVector3(x,y,z));
    }
    
    fairy_.setTrack(dummy_track);
    //妖精を動かす
//    robotbase->setTranslation(x,y,z);
//    robotbase->rotate(0,0,0);
    
    
}

