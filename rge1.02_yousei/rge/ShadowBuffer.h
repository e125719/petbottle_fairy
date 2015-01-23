/*
 *  ShadowBuffer.h
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 09/07/15.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "RGeObject.h"

namespace rge {
	
static const int SHADOW_MAP_SIZE= 512;

class ShadowMapDrawer {
public:
	ShadowMapDrawer() {}
	virtual ~ShadowMapDrawer() {}
	
	virtual void drawLightSceneCallback() {}
};

class ShadowMapping {
public:
	ShadowMapping(int w = SHADOW_MAP_SIZE, int h = SHADOW_MAP_SIZE) :
		mTexWidth(w), mTexHeight(h),
		mColorTex(), mDepthTex(), mFrameBuf()
	{}
	virtual ~ShadowMapping() ;
	
	void initShadowBuffer();
	void drawShadow(ShadowMapDrawer* castShadow, 
					const rgeMatrix4x4& sceneModelView, 
					const rgeMatrix4x4& sceneProj, 
					const rgeMatrix4x4& lightModelView, 
					rgeScalar lightFov, 
					rgeScalar range,
					rgeScalar intencity);
	void drawLightMap(rgeScalar x, rgeScalar y, rgeScalar w, rgeScalar h);
private:
	GLuint mTexWidth;
	GLuint mTexHeight;
	
	GLuint mColorTex;
	GLuint mDepthTex;
	GLuint mFrameBuf;
};

}
//void initShadowBuffer(void);
//void drawWithShadow(void(*sceneFunc)(void), void(*shadowMapFunc)(void), void(*cameraToLightTransform)(void));
//void drawWithShadow(void(*sceneFunc)(void), const rgeMatrix4x4& sceneModelView, const rgeMatrix4x4& sceneProj, const rgeMatrix4x4& lightModelView, rgeScalar lightFov);