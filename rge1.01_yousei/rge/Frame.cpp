/*
 *  Frame.cpp
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/08.
 *  Copyright 2007 Y. Akamine. All rights reserved.
 *
 */

#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"

#include "RGE.h"
#include "Physics.h"
#include "Frame.h"
#include "Bone.h"
#include "Action.h"

using namespace rge;

void Camera::lookUp(rgeScalar ex, rgeScalar ey, rgeScalar ez,
					rgeScalar rx, rgeScalar ry, rgeScalar rz,
					rgeScalar ux, rgeScalar uy, rgeScalar uz) 
{
	mEye =		rgeVector3(ex, ey, ez);
	mReference =rgeVector3(rx, ry, rz);
	mUp =		rgeVector3(ux, uy, uz);
}

void Camera::createFromRgm(istream& is)
{
	string item, name;
	rgeScalar near, far;
	rgeScalar lens;
	
	is >> item >> name; setId(name);
	is >> item >> near >> far;
	is >> item >> lens;
	
	mNearClip = near;
	mFarClip = far;
	mFov = degree(atan2(35.f/2.f, lens))*2;
	
	mReference = rgeVector3(0,0,-1);
	mUp = rgeVector3(0,1,0);
}


void Camera::issue()
{
	static const rgeVector3 eye(0,0,0);
	static const rgeVector3 ref(0,0,-1);
	static const rgeVector3 up (0,1,0);
	
	if (mbBindFrame) {
		rgeScalar mat[16];
		
		rgeGlGetVector(GL_MODELVIEW_MATRIX, mat);
		
		rgeMatrix4x4 m(mat);
		
		mEye = eye*m;
		mReference = ref*m;
		
		m.resetTranslation();
		mUp = (up)*m;
	}
}

void Camera::setProjectionMatrix(rgeScalar m[16])
{
	for(int i=0; i<16; ++i)
		mProjectionMatrix[i] = m[i];
}

void Camera::viewTransform(rgeScalar aspect)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if(mbUseCustomProj)
		rgeGlMultMatrix(mProjectionMatrix.getOpenGLMatrix());
	else {
        glm::mat4x4 m;
        m = glm::perspective(mFov, aspect, mNearClip, mFarClip);
//		gluPerspective(mFov, aspect, mNearClip, mFarClip);
        glMultMatrixf(&m[0][0]);
		rgeGlGetVector(GL_PROJECTION_MATRIX, mProjectionMatrix.get());
	}
	//	glRotated(mDistortion.x,0,1,0);
	//	glRotated(mDistortion.y,1,0,0);
	
	/*	GLdouble mp[16];
	 
	 rgeGlGetVector(GL_PROJECTION_MATRIX, mp);
	 GLdouble m[16] = {
	 1, 0, 0, 0,
	 0, 1, 0, 0,
	 0, 0, 1, 0,
	 0, 0, 0, 1
	 };
	 //	for(int i=0; i<16; ++i) {
	 //		mp[i] += m[i];
	 //	}
	 //	glMultMatrixd(m);
	 */	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	
	if(mbUseCameraMatrix) 
		rgeGlLoadMatrix(mCameraMatrix.getOpenGLMatrix());
	else {
        glm::mat4x4 m =
            glm::lookAt(
                        glm::vec3(mEye.x,mEye.y,mEye.z),
                        glm::vec3(mReference.x, mReference.y, mReference.z),
                        glm::vec3(mUp.x, mUp.y, mUp.z)
                        );
        glMultMatrixf(&m[0][0]);
		rgeGlGetVector(GL_MODELVIEW, mCameraMatrix.get());
	}
	
	//	printf("look %f %f %f\n", mEye.x, mEye.y, mEye.z);
	
}


void PointLight::issue()
{
	color3 amb = getRGE()->getAmbientColor();
	GLfloat glamb[] = {amb.r, amb.g, amb.b, 1 };
	GLfloat gldif[] = {mColor.r*mEnergy, mColor.g*mEnergy, mColor.b*mEnergy, 1};
	GLfloat glpos[] = { 0.0, 0.0, 0.0, 1.0};
	
	int ln = getRGE()->usedLight();
	if(ln != -1) {
		glLightfv(GL_LIGHT0+ln, GL_AMBIENT, glamb);
		glLightfv(GL_LIGHT0+ln, GL_DIFFUSE, gldif);
		glLightfv(GL_LIGHT0+ln, GL_POSITION, glpos);
		glLightf(GL_LIGHT0+ln, GL_SPOT_CUTOFF, 180.0);
		glLightf(GL_LIGHT0+ln, GL_CONSTANT_ATTENUATION, 1.0);
		glLightf(GL_LIGHT0+ln, GL_LINEAR_ATTENUATION, 0.0);
		glLightf(GL_LIGHT0+ln, GL_QUADRATIC_ATTENUATION, 0);
	}
}

void DirectionalLight::issue()
{
	color3 amb = getRGE()->getAmbientColor();
	GLfloat glamb[] = {amb.r, amb.g, amb.b, 1 };
	GLfloat gldif[] = {mColor.r*mEnergy, mColor.g*mEnergy, mColor.b*mEnergy, 1};
	GLfloat glpos[] = { 0,0,-1.0, 0.0, 0.0};
	
	int ln = getRGE()->usedLight();
	if(ln != -1) {
		glLightfv(GL_LIGHT0+ln, GL_AMBIENT, glamb);
		glLightfv(GL_LIGHT0+ln, GL_DIFFUSE, gldif);
		glLightfv(GL_LIGHT0+ln, GL_POSITION, glpos);
		glLightf(GL_LIGHT0+ln, GL_SPOT_CUTOFF, 180.0);
		glLightf(GL_LIGHT0+ln, GL_CONSTANT_ATTENUATION, 1.0);
		glLightf(GL_LIGHT0+ln, GL_LINEAR_ATTENUATION, 0.0);
		glLightf(GL_LIGHT0+ln, GL_QUADRATIC_ATTENUATION, 0);
	}
}

void SpotLight::issue()
{
	color3 amb = getRGE()->getAmbientColor();
	GLfloat glamb[] = {amb.r, amb.g, amb.b, 1 };
	GLfloat gldif[] = {mColor.r*mEnergy, mColor.g*mEnergy, mColor.b*mEnergy, 1};
	GLfloat glpos[] = { 0.0, 0.0, 0.0, 1.0};
	
	rgeVector3 dir(0,0,-1);
	
	/* rgeMatrix4x4 m;
	 rgeGlGetVector(GL_MODELVIEW_MATRIX, (GLdouble*)m.get());
	 m.resetTranslation();
	 dir = dir*m; */
	
	GLfloat gldir[] = { dir.x, dir.y, dir.z };
	
	int ln = getRGE()->usedLight();
	if(ln != -1) {
		glLightfv(GL_LIGHT0+ln, GL_AMBIENT, glamb);
		glLightfv(GL_LIGHT0+ln, GL_DIFFUSE, gldif);
		glLightfv(GL_LIGHT0+ln, GL_POSITION, glpos);
		glLightfv(GL_LIGHT0+ln, GL_SPOT_DIRECTION, gldir);
		glLightf(GL_LIGHT0+ln, GL_SPOT_CUTOFF, mSpotCutoff/2.0);
		glLightf(GL_LIGHT0+ln, GL_SPOT_EXPONENT, mSpotExponent);
		glLightf(GL_LIGHT0+ln, GL_CONSTANT_ATTENUATION, 1.0);
		glLightf(GL_LIGHT0+ln, GL_LINEAR_ATTENUATION, 0.0);
		glLightf(GL_LIGHT0+ln, GL_QUADRATIC_ATTENUATION, 0);
	}
}
void Frame::createFromRgm(istream& is) 
{
	cout << "createFromRgm has never implemented." << endl;
}

void Frame::createChildrenFromRgm(istream& is)
{
	string item;
	int count;
	is >> item >> count;
	if(item != "Children:" && item != "Root:")
		throw "Bad Object at children";
	
	int i;
	for(i=0; i<count; ++i) {
		FrameRef child = createChild();
		child->createFromRgm(is);
	}
}

void Frame::applyPhysics()
{
#ifndef NO_PHYSICS
	if (mPhysics.enabled) {
		if(mVisuals.size() >= 1) {
			PhBody* b;
			Renderable* vis = mVisuals.front().get();
			if(mPhysics.type == STATIC)
				mPhysics.mass = 0.f;
			if(mPhysics.boundBox == TRIMESH ||  mPhysics.boundBox == CONVEX) {
				if(vis->geomType() == Renderable::MESH_OBJ || vis->geomType() == Renderable::TRI_MESH_OBJ ) {
					if( mPhysics.boundBox == CONVEX )
						b = getRGE()->getPE()->createConvexHullBody(mPhysics.mass, (MeshObject*)vis);
					else if(mPhysics.type == STATIC)	
						b = getRGE()->getPE()->createTriMeshBody(mPhysics.mass, (MeshObject*)vis);
					else 
						b = getRGE()->getPE()->createConvexDecompositionBody(mPhysics.mass, (TriMeshObject*)vis);
				}else
					b = 0; 
			}else if(mPhysics.boundBox == CYLINDER) 
				b = getRGE()->getPE()->createBodyWithBoundingCylinder(mPhysics.mass, vis);
			else if(mPhysics.boundBox == SPHERE)
				b = getRGE()->getPE()->createBodyWithBoundingSphere(mPhysics.mass , vis);
			else //other geometry is treated as a BOX
				b = getRGE()->getPE()->createBodyWithBoundingBox(mPhysics.mass ,vis);
			
			if(b)
				bindByBody(b);
			//b->bindFrame(this);
			
			//getRGE()->logger() << "bind " << getId() << " = " << b->getGeom()->id() << "\n";
		}
	}
	for_each(mChildren.begin(), mChildren.end(), bind(&Frame::applyPhysics, _1));
#else
        std::cout << "The physical simulation funcion is disabled" <<endl;
        assert(0);
#endif
}
/*
 void Frame::transformBody()
 {
 PhBody* b = getPhBody();
 rgeMatrix4x4 mat = matrixWorld();
 btTransform btmat;
 btmat.setFromOpenGLMatrix(mat.getOpenGLMatrix());
 
 //	btMotionState* ms = new btDefaultMotionState(btmat);
 //	b->setMotionState(ms);
 b->getMotionState()->setWorldTransform(btmat);
 //	b->setCenterOfMassTransform(btmat);
 //	b->proceedToTransform(btmat);
 }*/

void Frame::bindByBody(PhBody* b) 
{	
#ifndef NO_PHYSICS
	mPhBody = b;
	b->bindFrame(this);
	//	transformBody();
#endif
}

void Frame::applyTransformationToMatrix()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	rgeGlLoadMatrix(mTransform.get());
	
	glTranslated(mPosition.x,mPosition.y,mPosition.z);
	rgeVector3 r = mRotations;// - mStartRotations;
	glRotated(r.z ,0.0,0.0,1.0);
	glRotated(r.y ,0.0,1.0,0.0);
	glRotated(r.x ,1.0,0.0,0.0);	
	
	glScaled(mScales.x,mScales.y,mScales.z);
	
	rgeGlGetVector(GL_MODELVIEW_MATRIX, mTransform.get());
	
	glPopMatrix();
	
	mPosition = rgeVector3();
	mRotations = rgeVector3();
	mScales = rgeVector3(1,1,1);
	
}

FrameRef Frame::createChild() {
	FrameRef child(new Frame( this));
	child->setLayer(getRGE()->activeLayer());
	mChildren.push_back(child);
	return child;
}

template<class T>
struct comp_ptr {
	T* ptr;
	
	comp_ptr(T* f) : ptr(f) {}
	bool operator() (shared_ptr<T> rhs) {
		return rhs.get() == ptr;
	}
};

void Frame::removeChild(FrameRef removing_frame)
{
	//	int s = mChildren.size();
	comp_ptr<Frame> pred(removing_frame.get());	
	mChildren.erase(remove_if(mChildren.begin(), mChildren.end(), pred), mChildren.end());
	//	printf("%d %d\n", s, mChildren.size());
	//	assert(s != mChildren.size());
}

void Frame::removeCamera(CameraRef cam)
{
	comp_ptr<Issuable> pred(cam.get());	
	mWorldIssue.erase(remove_if(mWorldIssue.begin(), mWorldIssue.end(), pred), mWorldIssue.end());	
}

void Frame::printChildren(int depth ) const  {
	cout <<  string(depth, ' ') << getId() << "\n";
	for (size_t i=0; i < mChildren.size(); ++i) 
		mChildren[i]->printChildren(depth+1);
	
	for (size_t i=0; i < mBones.size(); ++i) 
		mBones[i]->printChildren(depth+1);
}



rgeVector3 Frame::worldToLocal(rgeVector3 p) const
{
	rgeMatrix4x4 inv;
	rgeMatrix4x4::matrixDelta(&inv, &mLocalToWorld, &rgeMatrix4x4::identity());
	return p*inv;
}


void Frame::lookAt(rgeVector3 p) 
{// mLookAt=(Frame*)-1; mReference = p; }
 //	rgeVector3 dir = (getParent()->worldToLocal(p)-pos).normalized();
 //	rgeVector3 up= getParent()->worldToLocal(getParent()->getWorldPos() + rgeVector3(0,0,1));
	rgeVector3 pos = getWorldPos();
	rgeVector3 dir = (p - pos).normalized();
	rgeVector3 up = rgeVector3(0,0,1);
	
	rgeVector3 side = crossProduct(dir, up);
	up = crossProduct(side, dir);
	rgeScalar m[16] = {
		side.x, side.y, side.z, 0,
		up.x, up.y, up.z, 0,
		-dir.x, -dir.y, -dir.z, 0,
		0,0,0, 1 };
	
	rgeMatrix4x4 delta;
	rgeMatrix4x4 dst((rgeScalar*)m);
	rgeMatrix4x4 src = matrixWorld();
	
	src.resetTranslation();
	dst.resetTranslation();
	//	swap(src,dst);
	int res = rgeMatrix4x4::matrixDelta(&delta, &src, &dst);
	if(res) {
		//		glMultMatrixd((GLdouble*)delta.get());
		//		mTransform = delta;
		/*		mPosition = mPosition*mTransform;
		 mTransform.setTranslation(mPosition);
		 mPosition = rgeVector3();
		 
		 mTransform.applyRotationOnly(delta);
		 setRotation(rgeVector3());*/
		
		//		mExtraTransform = delta;
		//		mbUseExtraTransform = true;
		
		mCoordSpace = WORLD_SPACE;
		mTransform = dst;
		mTransform.setTranslation(pos);
		setRotation(0,0,0);
		setTranslation(0,0,0);
	}
	
	//	mTransform.fromArray(m);
	//	setTranslation(pos);
}

void Frame::moveLocally(rgeVector3 p)
{
    p = p * mLocalToWorld;
    
    if(getParent())
        p = p * getParent()->mLocalToWorld.inversed();
    
    moveTo(p);
}

void Frame::doTransform() {
	
	
	rgeGlMultMatrix(mTransform.get());
	
	
	glTranslated(mPosition.x,mPosition.y,mPosition.z);
	
    if(mbUseEulerRotation) {
		rgeVector3 r = mRotations;// - mStartRotations;
		glRotated(r.z ,0.0,0.0,1.0);
		glRotated(r.y ,0.0,1.0,0.0);
		glRotated(r.x ,1.0,0.0,0.0);
	}
	if(mbUseQuat) {
		//		glRotatef(mAngle, mRotations.x, mRotations.y, mRotations.z);
		rgeMatrix4x4 m = mQuatanion.rotationMatrix();
		rgeGlMultMatrix(m.get());
	}
	
	glScaled(mScales.x,mScales.y,mScales.z);
	
	
	return;
	
}

void Frame::doExtraTransform()
{
	if(mbUseExtraTransform) {
		rgeGlMultMatrix(mExtraTransform.get());
	}
}

void Frame::worldIssue() {
	glPushMatrix();
	
	doTransform();
	rgeGlGetVector(GL_MODELVIEW_MATRIX,mLocalToWorld.get());
	
	doExtraTransform();
	
	for_each(mWorldIssue.begin(), mWorldIssue.end(), bind(&Issuable::issue, std::placeholders::_1));
	for_each(mChildren.begin(), mChildren.end(), bind(&Frame::worldIssue, std::placeholders::_1));
	glPopMatrix();
	
}

void Frame::viewIssue() {
	glPushMatrix();
  	
	doTransform();
	doExtraTransform();
	
	for(IssuableRefs::iterator i=mViewIssue.begin(); i!=mViewIssue.end(); ++i) {
		//		(mViewIssue.begin(), mViewIssue.end(), bind(&Issuable::issue, _1));
		(*i)->issue();
		Light* l = Light::upcast(*i);
		if(l && l->isCastShadow()) {
			rgeMatrix4x4 m = matrixWorld();
			getRGE()->addShadowMap(m.inversed(), l->shadowMapAngle(), l->range(), l->shadowIntencity());
		}
	}
	
	for_each(mChildren.begin(), mChildren.end(), bind(&Frame::viewIssue, std::placeholders::_1));
	glPopMatrix();
	
}

void Frame::render() {
	
	glPushMatrix();
	
	if(mCoordSpace == WORLD_SPACE)
		getRGE()->applyCameraMatrix();	//カメラマトリクスを適用することで，親フレームでの変換が無視される.
	doTransform();
	doExtraTransform();
	
	
	for_each(mLocalIssue.begin(), mLocalIssue.end(), bind(&Issuable::issue, std::placeholders::_1));
	if(getRGE()->getLayerVisibility(mLayer) )
		for_each(mVisuals.begin(), mVisuals.end(), bind(&Renderable::render, std::placeholders::_1));

	renderMe();
	
	for_each(mChildren.begin(), mChildren.end(), bind(&Frame::render, std::placeholders::_1));
	for_each(mBones.begin(), mBones.end(), bind(&Bone::render, std::placeholders::_1));
	
	
	//	glutWireCube(2);
	
	glPopMatrix();
	
}

void Frame::update(rgeScalar delta) {
	anim()->update(delta, getRGE()->fps());
	
	setTranslation(getTranslation() + mVelocity * delta);
	mRotations += mAnglerVelo * delta;
	
	if(mLookAt) {
		lookAt(mLookAt->getWorldPos());
	}
	
	for_each(mChildren.begin(), mChildren.end(), bind(&Frame::update, std::placeholders::_1, delta));
}

void Frame::copyAction(FrameRef f, bool bShared ) { 
	if(bShared) {
		anim()->setAction(f->anim()->action());
		//cout << getId() << " " << f->getId() << endl;
	}else{
		ActionRef a = ActionRef( new Action );
		*(a.get()) = *(f->anim()->action().get());
		anim()->setAction(a);
	}
	
	if(f->numChildren() == numChildren()) {
		for(int i=0; i<numChildren(); ++i) {
			mChildren[i]->copyAction(f->childByIndex(i), bShared);
		}
	}else{
		dprintf("different number of children at Frame::copyActions %d %d\n", f->numChildren() , numChildren());
	}
}

void Frame::shareActionWith(FrameRef f) { copyAction(f, true); }


void Frame::setAnimationBlending(FrameRef blendingFrame, float blendingFactor) { 
	anim()->setBlendingAction(blendingFrame->anim()->action());
	anim()->setBlendingFactor(blendingFactor);
	
	if(blendingFrame->numChildren() == numChildren()) {
		for(int i=0; i<numChildren(); ++i) {
			childByIndex(i)->setAnimationBlending(blendingFrame->childByIndex(i), blendingFactor);
		}
	}else{
		dprintf("different number of children at Frame::copyActions %s %s\n", getId().c_str() , blendingFrame->getId().c_str());
	}
}

string Frame::getActionsDump( bool bApplyChildren )
{
	string buf;
	stringstream ss(buf);
	
	if(anim()->action())
		anim()->action()->print(ss, getId());
	
	string dump = ss.str();
	
	if(bApplyChildren) {
		for(FrameRefs::iterator i=mChildren.begin(); i != mChildren.end(); ++i) {
			dump += (*i)->getActionsDump(bApplyChildren);
		}
		
	}
	return dump;
}

BoneRef Frame::createBone(Frame* base_frame)
{
	if(base_frame == 0)
		base_frame = this;
	
	BoneRef b(new Bone( base_frame));
	
	mBones.push_back(b);
	
	return b;
}

BoneRef Frame::findBone(const string& name) 
{
	for(BoneRefs::iterator i=mBones.begin(); i!=mBones.end(); ++i) {
		if((*i)->getId() == name) {
			return *i;
		}
		
		BoneRef b = (*i)->findBone(name);
		if(b)
			return b;
	}
	return BoneRef((Bone*)0);
}

void Frame::resolveParentBone()
{
	for(FrameRefs::iterator i = mChildren.begin(); i!=mChildren.end(); ) {
		FrameRef f = *i;
		
		if(f->mParentType == BONE_NAME) {
			BoneRef b = findBone(f->mParentBoneName);
			
			if(b) {
				i = mChildren.erase(i);
				b->addChild(f);
				continue;
			}
		}else {
			f->resolveParentBone();
		}
		++i;
	}
}

void Frame::flattenBoneTree(Frame* base_frame)
{
	for_each(mBones.begin(), mBones.end(), bind(&Frame::flattenBoneTree, std::placeholders::_1, base_frame));
	
	for(BoneRefs::iterator i=mBones.begin(); i!=mBones.end(); ++i) {
		if(base_frame != this) {
			base_frame->mBones.push_back(*i);
			(*i)->setParentBone((Bone*)0);
		}
		(*i)->setMatrix(rgeMatrix4x4());
	}
	if(base_frame != this) {
		mBones.clear();
	}
}


void Frame::setBonesVisibility(bool b)
{
	for_each(mBones.begin(), mBones.end(), bind(&Bone::setVisibility, std::placeholders::_1, b, true));
}

void Frame::setLayer(LAYER_NUM nlayer, bool bApplyChildren ) {
    mLayer = 1 << (nlayer-1);
    
    if(bApplyChildren) {
        for_each(mChildren.begin(), mChildren.end(), bind(&Frame::setLayer, std::placeholders::_1, nlayer, bApplyChildren));
        for_each(mBones.begin(), mBones.end(), bind(&Bone::setLayer, std::placeholders::_1, nlayer, bApplyChildren));
    }
}

void Animation::update(rgeScalar deltaTime, rgeScalar fps)
{
	if(mIsAnimated) {
		mTime += deltaTime*fps*mAnimSpeed; 
	}
	
	if(mAction) {
		
		if(mBlendingAction) {
			mAction->apply(mTargetFrame, mTime, 1.f - mBlendingFactor, Action::REPLACE);
			mBlendingAction->apply(mTargetFrame, mTime, mBlendingFactor, Action::ADD);
		}else {
			mAction->apply(mTargetFrame, mTime, 1.f);
		}	
		
	}
}

void Animation::stop(bool bApplyChildren) { //!< アニメーションを止める
	mIsAnimated = false; 
	if(bApplyChildren)
		mTargetFrame->forEachAnimOfChildren(&Animation::stop, bApplyChildren);	
} 
void Animation::start(bool bApplyChildren ) { //!< アニメーションを開始する
	mIsAnimated = true; 
	if(bApplyChildren)
		mTargetFrame->forEachAnimOfChildren(&Animation::start, bApplyChildren);	
} 

void Animation::setTime(int t, bool bApplyChildren  ) { //!< アニメーションの再生時刻を設定(t), 子フレームに適用するか（bAppluChildren)
	mTime = t; 
	if(bApplyChildren)
		mTargetFrame->forEachAnimOfChildren(&Animation::setTime, t, bApplyChildren);		
}

void Animation::setLooping(bool bloop, bool bApplyChildren ) { //!< アニメーションをループ再生するか(bloop), 子フレームにも適用するか(bAppluChildren)
	if(mAction)
		mAction->setLooping(bloop);
	
	if(bApplyChildren)
		mTargetFrame->forEachAnimOfChildren(&Animation::setLooping, bloop, bApplyChildren);		
}

