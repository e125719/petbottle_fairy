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
#include "ImageFile.h"

namespace  rge {

//!テクスチャーの基底クラス
class Texture : public IdentifiedObj, public Issuable
{
public:
	Texture() {}
	virtual ~Texture() {}
	
	virtual void createFromFile(const std::string& filename) {}
//	virtual void order() = 0;
};

//! 画像ファイルからテクスチャーを作成するクラス(現在はjpegのみ）
class ImageTexture : public Texture
{
public:
	ImageTexture() : mTexName(), hasImage(false) { }

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
		
/*		unsigned int *bits = new unsigned int[256*256];
		for(int y=0; y!=256; ++y) {
			for(int x=0; x!=256; ++x) {
				bits[y*256+x] = (x/16 + y/16) % 2 == 0 ? -1 : 0;
			}
		}*/
		glGenTextures(1, &mTexName);

		JpegFileReader jpeg;
//		cout << filename;
		if(!jpeg.read(filename.c_str())) {
			CheckerImage jpeg;
			jpeg.create(256,256);
			
			glBindTexture(GL_TEXTURE_2D, mTexName);
			glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,
					 jpeg.getSizx(), jpeg.getSizy(), 0,
					 GL_RGBA,GL_UNSIGNED_BYTE, jpeg.getRawData());
		}else {
			glBindTexture(GL_TEXTURE_2D, mTexName);
			glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,
						 jpeg.getSizx(), jpeg.getSizy(), 0,
						 GL_RGBA,GL_UNSIGNED_BYTE, jpeg.getRawData());
	/*		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
			jpeg.getSizx(), jpeg.getSizy(), GL_RGBA, GL_UNSIGNED_BYTE, jpeg.getRawData()); */
		}
		
		hasImage = true;
//		delete[] bits;
	}
	
	virtual void issue() {
		if(hasImage) { 
//			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, mTexName);
			/* テクスチャマップの方法を設定 */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);			

		}
	}
	
	virtual OBJ_TYPE objType() { return MATERIAL; }
	
private:
	GLuint mTexName;
	bool hasImage;
};

typedef std::shared_ptr<Texture> TextureRef;
typedef std::shared_ptr<ImageTexture> ImageTextureRef;
inline ImageTextureRef newImageTexture() { return ImageTextureRef(new ImageTexture()); }

}