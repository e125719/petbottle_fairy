/*
 *  RgeObject.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/02.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once
#include <string>
#include <OpenGL/gl.h> 
#include "Types.h"

namespace rge {

//!名前で識別可能なオブジェクト(抽象クラス)
class IdentifiedObj
{
public:
	const std::string& getId() const { return mId;}
	
	void setId(std::string id) { mId = id; }
//	void idFromDae(domElement* ele) { mId = ele->getID(); }
protected:
	IdentifiedObj() {}
	virtual ~IdentifiedObj() {}
	std::string mId;
};

class RGE;
//!rgeで管理される全てのオブジェクトの基底クラス
class RGEObject : public IdentifiedObj
{
public:
	RGE* getRGE();
        
protected:
	RGEObject() {}
	virtual ~RGEObject() {}
	
//	RGE* mRge;
};

#ifdef RGE_USE_DOUBLE
inline void rgeGlLoadMatrix(const rgeScalar* m) { glLoadMatrixd(m); }
#else
inline void rgeGlLoadMatrix(const rgeScalar* m) { glLoadMatrixf(m); }
inline void rgeGlMultMatrix(const rgeScalar* m) { glMultMatrixf(m); }
inline void rgeGlGetVector(u_int type, rgeScalar* v) { glGetFloatv(type, v); }
#endif

}