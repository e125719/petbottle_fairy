/*
 *  ShadowBuffer.cpp
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 09/07/15.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <glut/glut.h>

#include <memory.h>
#include "ShadowBuffer.h"
#include "Types.h"

using namespace rge;

ShadowMapping::~ShadowMapping()
{
	glDeleteFramebuffersEXT(1, &mFrameBuf);
	glDeleteTextures(1, &mColorTex);
	glDeleteTextures(1, &mDepthTex);
}

void ShadowMapping::initShadowBuffer(void)
{
//	glActiveTexture(GL_TEXTURE1);
	
	glEnable(GL_TEXTURE_2D);
	
	unsigned int *tex = new unsigned int[mTexWidth*mTexHeight];
	memset(tex, 0, sizeof(tex));
	
//	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT, mTexWidth, mTexHeight, 0,GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE, tex);
	
	//http://angra.blog31.fc2.com/blog-entry-11.html
	// フレームバッファオブジェクト、デプスとカラー用のテクスチャを作成
	glGenFramebuffersEXT( 1, &mFrameBuf );
	glGenTextures( 1, &mColorTex );
	glGenTextures( 1, &mDepthTex );

	// 深度バッファだけではうまくいかないのでとりあえずカラーバッファも作成？
	glBindTexture( GL_TEXTURE_2D, mColorTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex );

	// 深度バッファを作成
	glBindTexture( GL_TEXTURE_2D, mDepthTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, mTexWidth, mTexHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, tex );
	
	// テクスチャをフレームバッファオブジェクトにバインド
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mFrameBuf );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mColorTex, 0 );
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, mDepthTex, 0 );
	
	// デフォルトのレンダーターゲットに戻す
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT ,0);
	
//	glActiveTexture(GL_TEXTURE0);

	delete[] tex;
}

void ShadowMapping::drawShadow(ShadowMapDrawer* castShadow, 
							   const rgeMatrix4x4& sceneModelView, 
							   const rgeMatrix4x4& sceneProj, 
							   const rgeMatrix4x4& lightModelView, 
							   rgeScalar lightFov,
							   rgeScalar range,
							   rgeScalar intencity
							   )
{
//glActiveTexture(GL_TEXTURE1);
	GLint matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
	
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mFrameBuf );	
	 	
	/* ビューポートをテクスチャのサイズに設定する */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, mTexWidth, mTexHeight);
 
	glEnable(GL_CULL_FACE); 
//	glCullFace(GL_BACK);
	glCullFace(GL_FRONT);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

//	glClearColor(1, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	
	
	//to avoid z-fighting
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 1.8, 8);
	
	rgeScalar lightProj[16];
	/* ライトからの視点での描画 */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(lightFov, 1, 1, range);
	
	//std::cout << lightFov << ",";
	
	rgeGlGetVector(GL_PROJECTION_MATRIX, lightProj);
	
	glMatrixMode(GL_MODELVIEW);
	rgeGlLoadMatrix(lightModelView.get());
	
//	sceneFunc();
//	glEnable(GL_LIGHTING);
//	glColor4f(1, 0, 0, 1);
	castShadow->drawLightSceneCallback();

	glDisable( GL_POLYGON_OFFSET_FILL );

	
//  陰範囲の境界を丸くする処理	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-(double)mTexWidth, (double)mTexWidth, -(double)mTexHeight, (double)mTexHeight, 0, 10);
//	gluPerspective(1, 1, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDepthFunc(GL_ALWAYS);

	glBegin(GL_TRIANGLE_STRIP);
	for(int i=0; i<=360; i+=10) {
		double t = radian((double)i);
		glVertex3f(cos(t)*(mTexWidth-1), sin(t)*(mTexHeight-1), 0);
		glVertex3f(cos(t)*mTexWidth*2, sin(t)*mTexHeight*2, 0);
	}
	glEnd();
	glDepthFunc(GL_LESS);
	   
	
	//シャドウマップ生成ここまで
	
//シャドウ生成
	
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
//	printf("bindbuf %x\n", glGetError());
	
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	

	////テクスチャの設定
	glBindTexture( GL_TEXTURE_2D, mDepthTex );

	/* テクスチャマッピングとテクスチャ座標の自動生成を有効にする */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);
	
	/* テクスチャを拡大・縮小する方法の指定 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* テクスチャの繰り返し方法の指定 */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	
	
    /* テクスチャ座標に視点座標系における物体の座標値を用いる */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);	
	
	/* 生成したテクスチャ座標をそのまま (S, T, R, Q) に使う */
	static const GLdouble genfunc[][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 }, 
		{ 0.0, 0.0, 0.0, 1.0 },
	};
	glTexGendv(GL_S, GL_EYE_PLANE, genfunc[0]); 
	glTexGendv(GL_T, GL_EYE_PLANE, genfunc[1]);
	glTexGendv(GL_R, GL_EYE_PLANE, genfunc[2]);
	glTexGendv(GL_Q, GL_EYE_PLANE, genfunc[3]);	
	
	/* 透視変換行列の指定 */	
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	
	/* テクスチャ座標の [-1,1] の範囲を [0,1] の範囲に収める */
	glTranslated(0.5, 0.5, 0.5);
	glScaled(0.5, 0.5, 0.5);
	
	//カメラ座標系をシャドウマップ生成時の座標系へ変換
//	cameraToLightTransform();
	{
		rgeGlMultMatrix(lightProj);
		rgeGlMultMatrix(lightModelView.get());
		rgeGlMultMatrix(sceneModelView.inversed().get());
	}
	
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	if(0){ //フォグ機能を使って強制的に全ての色を灰色にする

		glEnable(GL_FOG);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, -1);
		glFogf(GL_FOG_END, 1);
		const GLfloat shadow_density = 0.1;
		GLfloat fogcol[] = {shadow_density,shadow_density,shadow_density,0};
		glFogfv(GL_FOG_COLOR, fogcol);
	}
	
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset(-1.4, -8);
	
//	glActiveTexture(GL_TEXTURE0);
	/* 書き込むポリゴンのテクスチャ座標値のＲとテクスチャとの比較を行うようにする */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	
	/* もしＲの値がテクスチャの値以下なら真（つまり日向） */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
	
    /* 比較の結果を輝度値として得る */
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);
	
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.01f);
	
	glEnable(GL_BLEND);
        //glDisable(GL_BLEND);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
#if 1
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
	glColor4f(intencity,intencity,intencity,1);
	//glColor3f(0,0,intencity);
#else
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	glColor4f(1,1,1,1);
#endif
	
	glDisable(GL_LIGHTING);

//	glBindTexture( GL_TEXTURE_2D, mDepthTex );
	
	glDepthFunc(GL_LEQUAL);
	
	{
		glMatrixMode(GL_PROJECTION);
		rgeGlLoadMatrix(sceneProj.get());
		glMatrixMode(GL_MODELVIEW);
		rgeGlLoadMatrix(sceneModelView.get());
		castShadow->drawLightSceneCallback();
	}
	glDepthFunc(GL_LESS);

	glDisable( GL_POLYGON_OFFSET_FILL );
	
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);
	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	
	
//	glActiveTexture(GL_TEXTURE0);

	//glDisable( GL_FOG);
	
	glMatrixMode(matrix_mode);
}

void ShadowMapping::drawLightMap(rgeScalar x, rgeScalar y, rgeScalar w, rgeScalar h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 1, 0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mDepthTex);
	
	glDisable(GL_BLEND);

	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	
	
	glColor4f(1, 1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 0);
	glVertex2f(x, y);

	glTexCoord2d(0, 1);
	glVertex2f(x, y+h);

	glTexCoord2d(1, 1);
	glVertex2f(x+w, y+h);

	glTexCoord2d(1, 0);
	glVertex2f(x+w, y);
	glEnd();
}
