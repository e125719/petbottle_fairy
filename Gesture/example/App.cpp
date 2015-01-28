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

#include "OpenGLTexture.h"

#include <opencv2/opencv.hpp>
using namespace cv;

static VideoCapture video;


#if 0
#include "glsl.h"
class GlslMaterial : public Material
{
    GLuint vertShader;
    GLuint fragShader;
    GLuint gl2Program;
public:
    GlslMaterial() {
    }
    
    virtual void create() {
        /* シェーダプログラムのコンパイル／リンク結果を得る変数 */
        GLint compiled, linked;
        
        /* GLSL の初期化 (windowsのみ必要　*/
        if (glslInit()) exit(1);
        
        /* シェーダオブジェクトの作成 */
        vertShader = glCreateShader(GL_VERTEX_SHADER);
        fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        
        /* シェーダのソースプログラムの読み込み */
        if (readShaderSource(vertShader, "simple.vert")) exit(1);
        if (readShaderSource(fragShader, "simple.frag")) exit(1);
        
        /* バーテックスシェーダのソースプログラムのコンパイル */
        glCompileShader(vertShader);
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
        printShaderInfoLog(vertShader);
        if (compiled == GL_FALSE) {
            fprintf(stdout, "Compile error in vertex shader.\n");
            exit(1);
        }
        
        /* フラグメントシェーダのソースプログラムのコンパイル */
        glCompileShader(fragShader);
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
        printShaderInfoLog(fragShader);
        if (compiled == GL_FALSE) {
            fprintf(stdout, "Compile error in fragment shader.\n");
            exit(1);
        }
        
        /* プログラムオブジェクトの作成 */
        gl2Program = glCreateProgram();
        
        /* シェーダオブジェクトのシェーダプログラムへの登録 */
        glAttachShader(gl2Program, vertShader);
        glAttachShader(gl2Program, fragShader);
        
        /* シェーダオブジェクトの削除 */
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        
        
        /* シェーダプログラムのリンク */
        glLinkProgram(gl2Program);
        glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
        printProgramInfoLog(gl2Program);
        if (linked == GL_FALSE) {
            fprintf(stdout, "Link error.\n");
            exit(1);
        }
    }
    
    virtual void issue()
    {
        glUseProgram(gl2Program);
    }
};
#endif

App::App()
{
}

void App::readTextures()
{
    vector<string> tex_names = {
        "sampleModels/sabi.jpg",
        "sampleModels/tex2.jpg",
        "sampleModels/rh1.jpg"
    };
    
    for(auto& name : tex_names) {
        TextureRef tex = TextureRef(new ImageTexture);
        
        tex->createFromFile(name);
        
        textures_.push_back(tex);
    }
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
    
    readTextures();

    //妖精描画
    robotbase = RGE::getInstance()->findFrame("body");
    
    
    //テクスチャ
    liveTexture_ = TextureRef(new ImageTexture);
    liveTexture_->createFromFile("sampleModels/pet.jpg");
    RGE::getInstance()->findMaterial("dress1")->setTexture(liveTexture_);
    
    //video.open(0);
    
    dummyData(0,0,0);
    
//    MaterialRef mat = MaterialRef(new GlslMaterial);
//    mat->create();
//    mat->setId("glsl");
//
//    RGE::getInstance()->registerMaterial(mat);
//    RGE::getInstance()->updateMaterial();
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
    Mat img;
    video >> img;
    
    //imshow("video", img);
    //waitKey(1);
    
    liveTexture_->updateImage(img.size().width, img.size().height, GL_BGR, img.ptr());
    //RGE::getInstance()->findMesh("Cylinder.000")->clearCache();
    
    fairy_.update();
    
    rgeVector3 position = fairy_.getPosition();
    double direction = fairy_.getDirection();
    robotbase->setTranslation(position);
    robotbase->setRotation(0,0,direction+180);
    

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
    
    
    //texture_.update(img, GL_RGB);//GL_BGR
    //texture_.drawBackground();


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


