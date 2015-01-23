/*
 *  Bone.h
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 10/11/19.
 *  Copyright 2010 Yuhei Akamine. All rights reserved.
 *
 */

#pragma once

#include "Frame.h"

namespace rge {
class Bone : public Frame
{
public:
	Bone( Frame* base_frame) : 
		Frame((Frame*)0), 
		mParentBone(0), 
		mBaseFrame(base_frame),
		mLength(0),
		mVisible(true) 
			{}

	BoneRef createChildBone() {
		BoneRef b =  createBone(mBaseFrame);
		b->mParentBone = this;
		return b;
	}
	
	void addChildBone(BoneRef b) {
		b->mParentBone = this;
		b->mBaseFrame = mBaseFrame;
		
		mBones.push_back(b);
	}
	
	void setBaseFrame(Frame* base) { mBaseFrame = base; }
	void setParentBone(Bone* parent) { mParentBone = parent; }
	
	void setLength(rgeScalar l) { mLength = l; }
	
	virtual void renderMe() ;
	
	void setVisibility(bool b, bool applyChildren= true) { 
		mVisible = b; 
		
		for_each(mBones.begin(), mBones.end(), bind(&Bone::setVisibility, std::placeholders::_1, b, applyChildren));
	}
private:
	Bone* mParentBone;
	Frame* mBaseFrame;
	
	rgeScalar mLength;
	
	bool mVisible;
};

}