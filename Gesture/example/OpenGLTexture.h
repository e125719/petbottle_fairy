//
//  OpenGLTexture.h
//  PoseEstimater
//
//  Created by Yuhei Akamine on 2014/12/17.
//  Copyright (c) 2014年 Yuhei Akamine. All rights reserved.
//

#ifndef PoseEstimater_OpenGLTexture_h
#define PoseEstimater_OpenGLTexture_h

#include <OpenGL/gl.h>
#include <opencv2/opencv.hpp>


class OpenGLTexture
{
    GLuint texId_ = 0;
    
public:
    OpenGLTexture() {
    }
    
    void update(const cv::Mat& img, int format = GL_BGR) {
        if(texId_ == 0)
            glGenTextures(1, &texId_);
        
        glBindTexture(GL_TEXTURE_2D, texId_);
        
        /* テクスチャ画像はバイト単位に詰め込まれている */
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.size().width, img.size().height, 0,format, GL_UNSIGNED_BYTE, img.ptr<void>());
    }
    
    void drawBackground() {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glBindTexture(GL_TEXTURE_2D, texId_);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1, 1, 0, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glColor3f(1,1,1);
        glBegin(GL_QUADS);
        
        glTexCoord2f(0,0);
        glVertex2d(0,0);
        
        glTexCoord2f(0,1);
        glVertex2d(0,1);
        
        glTexCoord2f(1,1);
        glVertex2d(1,1);
        
        glTexCoord2f(1,0);
        glVertex2d(1,0);
        
        glEnd();
        
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
    }
};
#endif
