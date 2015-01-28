/*
 *  Texture.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/26.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */

#pragma once  
#include <memory>
#include <string>
#include <iostream>
using std::istream;

#include <OpenGL/gl.h>
#include "Types.h"
#include "Exceptions.h"
#include "RgeObject.h"
#include "Image2D.h"
namespace  rge {

//!テクスチャーの基底クラス
class Texture : public IdentifiedObj, public Issuable
{
    GLuint mTexName;
public:
    Texture() : mTexName() {}
	virtual ~Texture() {}
	
    void create() {
 		glGenTextures(1, &mTexName);
    }
    
    void bind() {
        glBindTexture(GL_TEXTURE_2D, mTexName);
    }
    
	virtual void createFromFile(const std::string& filename) {}
    virtual void updateImage(int w, int h, int format, void* image) {
        bind();
        
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,
                     w,h, 0,
                     format,GL_UNSIGNED_BYTE, image);
    }
//	virtual void order() = 0;
};

//! 画像ファイルからテクスチャーを作成するクラス(現在はjpegのみ）
class ImageTexture : public Texture
{
public:
	ImageTexture() : hasImage(false) { }

	void createFromRgm(istream& is) 
	{
		string item,param;
		is >> item >> param; setId(param);
		is >> item >> param;
		
		if(item != "None:")
			createFromFile(param);
	}
	
	//!jpegファイルからテクスチャを生成
	virtual void createFromFile(const std::string& filename)
	{
        create();

		JpegImage jpeg;
//		cout << filename;
		if(!jpeg.read(filename.c_str())) {
			CheckerImage jpeg;
			jpeg.create(256,256);
			
            bind();
			glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,
					 jpeg.getSizx(), jpeg.getSizy(), 0,
					 GL_RGBA,GL_UNSIGNED_BYTE, jpeg.getRawData());
		}else {
			bind();
            
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
            glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,
                         jpeg.getSizx(), jpeg.getSizy(), 0,
                         GL_RGBA,GL_UNSIGNED_BYTE, jpeg.getRawData());
                
//                gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
//                 jpeg.getSizx(), jpeg.getSizy(), GL_RGBA, GL_UNSIGNED_BYTE, jpeg.getRawData()); 

//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); //
//            
//                mipmap_level = 1;
//                Image2D img = jpeg.normalized();
//                for(int i=0; i<mipmap_level; ++i) {
//
//                    glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,
//                                 img.getSizx(), img.getSizy(), 0,
//                                 GL_RGBA,GL_UNSIGNED_BYTE, img.getRawData());
//                    
//                    img = img.halfImage();
//                    if(img.getSizx() == 0 || img.getSizy() == 0)
//                        break;
//                }
		}
		
		hasImage = true;
//		delete[] bits;
	}
    

	
	virtual void issue() {
		if(hasImage) { 
//			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glEnable(GL_TEXTURE_2D);
			bind();
			/* テクスチャマップの方法を設定 */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); //GL_NEAREST_MIPMAP_LINEAR
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);			

		}
	}
	
	virtual OBJ_TYPE objType() { return MATERIAL; }
	
private:

	bool hasImage;
};

typedef std::shared_ptr<Texture> TextureRef;
typedef std::shared_ptr<ImageTexture> ImageTextureRef;
inline ImageTextureRef newImageTexture() { return ImageTextureRef(new ImageTexture()); }

}