/*
 *  RGE.cpp
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 08/11/12.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "RGE.h"

#include <map>
#include <string>
#include <iostream>
using namespace std;

#include <memory>
#include <algorithm>

#include "Types.h"
#include "Exceptions.h"
#include "Material.h"
#include "MeshObj.h"
#include "Frame.h"
#include "Texture.h"
#include "Physics.h"
#include "ShadowBuffer.h"

#define RGE_HEADER_NAME "#Blender RGE exporter 1.3"

using namespace rge;

RGE* RGE::theInstance = 0;


RGE::RGE() : 
	mpPhysicsEngine(0), 
	mShadowMapper(),
	mAspectRatio(1.0), 
	mAmbientColor(0.1,0.1,0.1), 
	mMaxLights(8), 
	mLightNum(0), 
	mLightEnable(true) ,
	mMaterialEnable(true),
	mbClearMatrix(true),
	mbUseCallList(true),
	mActiveLayer(1),
	mZMaskLayer(LAYER_INVISIBLE)
{
}

RGE::~RGE() 
{
#ifndef NO_PHYSICS
	delete mpPhysicsEngine;
#endif
	delete mShadowMapper;
}

void RGE::deleteInstance()
{
    delete theInstance;
}

RGE* RGE::getInstance()
{
    if(!theInstance)
        theInstance = new RGE;
    return theInstance;
    
}

RGE* RGEObject::getRGE()
{
    return RGE::getInstance();
}

void RGE::init()
{
	mShadowMapper = new ShadowMapping;
	mShadowMapper->initShadowBuffer();	

	Material::createTemplates();
	
	setVisibleLayerMask(LAYER_MASK_SHOW_ALL);
	
	mRootFrame = newFrame(0);

	//register __nomat
	MaterialRef def = newMaterial();
	*def = *Material::getTemplate(Material::COLORNAME_WHITE);
	
	Material::setDefault(def);
	mMaterials.insert(MaterialRefMap::value_type("__nomat", Material::defaultMaterial()));
	
}
#ifndef NO_PHYSICS
void RGE::createPE()
{
	mpPhysicsEngine = new PhysicsEngine();
}

PhysicsEngine* RGE::getPE() { return mpPhysicsEngine; }
#endif
//MeshObjectRef RGE::createMeshObj() { return MeshObjectRef(new MeshObject(this)); }

bool RGE::createSceneFromRgm(istream* pis) {
	istream& is = *pis;
	char line[1024];
	string token;
	int count;
	
	if(!pis->good()) {
		(logger() << "The provided file can't be accessed at createSceneFromRgm. It may not be found.") << "\n";
		return false;
	}
	
	is.getline( line, sizeof(line) );
	if(strcmp(line, RGE_HEADER_NAME) != 0) {
		logger() << "Not RGE file!" << "\n";
		return false;
	}
	
	int i;
	//material
	is >> token >> count;
	if(token != "[TextureSection]") {
		logger() << "RGE:Bad texture section: parsing at " << token << "\n";
		return false;
	}
	for(i=0; i<count; ++i) {
		ImageTextureRef thisTexture = newImageTexture();
		thisTexture->createFromRgm(is);
		registerTexture(thisTexture);
		logger() << "read texture:" << thisTexture->getId() << "\n";
	}
	
	is >> token >> count;
	if(token != "[MaterialSection]") {
		logger() << "RGE:Bad material section: parsing at " << token << "\n";
		return false;
	}
	for(i=0; i<count; ++i) {
		MaterialRef thisMaterial = newMaterial();
		thisMaterial->createFromRgm(is);			
		registerMaterial(thisMaterial);
		logger() << "read material:" << thisMaterial->getId() << "\n";			
	}
	
	//register _nomat_
	mMaterials.insert(MaterialRefMap::value_type("_nomat_", Material::getTemplate(Material::COLORNAME_WHITE)));
	
	is >> token >> count;
	if(token != "[MeshSection]") {
		logger() << "RGE:Bad mesh section: parsing at " << token << "\n";
		return false;
	}
	for(i=0; i<count; ++i) {
		MeshObjectRef thisMesh = newTriMeshObject();
		thisMesh->createFromRgm(is);			
		mMeshObjs.insert(MeshObjRefMap::value_type(thisMesh->getId(), thisMesh));
		logger() << "read mesh:" << thisMesh->getId() << "\n";		
	}
	
	is >> token >> count;
	if(token != "[CameraSection]") {
		//			logger() << "RGE:Bad camera section: parsing at " << token << "\n";
		//			return false;
		logger() << "No camera section" << "\n";
		goto _OBJECT_SECTION;
	}
	for(i=0; i<count; ++i) {
		CameraRef thisCam = CameraRef(new Camera());
		thisCam->createFromRgm(is);
		registerCamera(thisCam);
		logger() << "read camera:" << thisCam->getId() << "\n";
	}
	
	is >> token >> count;
	if(token != "[LightSection]") {
		logger() << "No light section" << "\n";
		goto _OBJECT_SECTION;
	}
	for(i=0; i<count; ++i) {
		string item, name, type;
		color3 col;
		rgeScalar e, cutoff, exp;
		is >> item >> name; 
		is >> item >> type;
		is >> item >> col;
		is >> item >> e;
		is >> item >> cutoff;
		is >> item >> exp;
		
		LightRef light;
		if(type == "Point") {
			light = LightRef(new PointLight( col, e));
		}else if(type == "Directional") {
			light = LightRef(new DirectionalLight( col, e));
		}else if(type == "Spot") {
			light = LightRef(new SpotLight( col, e, cutoff, exp));
		}else {
			light = LightRef(new PointLight( col, e));
			logger() << "RGE: unknown light type: " << type << ":assume Point light" << "\n";
		}
		
		light->setId(name);
		registerLight(light);
//		mLights.insert(LightRefMap::value_type(light->getId(), light));
		
		logger() << "read light:" << light->getId() << "\n";
	}
_OBJECT_SECTION:
	is >> token;
	if(token != "[ObjectSection]") {
		logger() << "RGE:Bad object section: parsing at " << token << "\n";
		return false;
	}
	mRootFrame->createChildrenFromRgm(is);
	
	registerChildFrames(mRootFrame);
	
	return true;
}
#ifndef NO_PHYSICS
void RGE::applyPhysics() {
	if(!mpPhysicsEngine)
		createPE();

	if(mRootFrame.get()) {
		rootFrame()->worldIssue();
		rootFrame()->applyPhysics(); 
	}
}
#endif

FrameRef RGE::rootFrame() { return mRootFrame; }

void RGE::setRootFrame(FrameRef newRoot)
{
	mRootFrame = newRoot;
}

rgeScalar RGE::update(rgeScalar delta) {
	if(delta == -1)
		delta = mTimer.elapsedTime();
	
	if(delta == 0.0)
		delta = 0.0000001;
	
//	printf("delta = %f\n", delta);
#ifndef NO_PHYSICS
	if(getPE())
		getPE()->step(delta);
#endif
	if(mRootFrame)
		mRootFrame->update(delta);
		
	return delta;
}

void RGE::setViewport(int x, int y, int width, int height)
{
	mViewport[0] = x;
	mViewport[1] = y;
	mViewport[2] = width;
	mViewport[3] = height;
	
	glViewport(x, y, width, height);
}

void RGE::render() {
	//		std::for_each(mRenderables.begin(), mRenderables.end(), boost::mem_fn(&Renderable::render) );
	
	if(mRootFrame) {
		setStatus();
		resetLightNum();
		
/*		glMatrixMode(GL_MODELVIEW);
		
		if(mbClearMatrix)
			glLoadIdentity();
		
		mRootFrame->worldIssue(); //主にカメラの位置を決定
*/
		calcModelToWorldMatrix();
/*		if(mCurrentCamera.get())
			mCurrentCamera->viewTransform(mAspectRatio);
*/		
		applyCameraTransform();

	
		lighting();
		
		int i;
		if(mLightEnable) {
			glEnable(GL_LIGHTING);
			for(i=0; i<mLightNum; ++i)
				glEnable(GL_LIGHT0+i);
		}
		
/*		setRenderBlend(true);
		mRootFrame->render();*/

		drawZOnly();

		drawScene();

		castShadaw();

	}
}

void RGE::setStatus()
{
	glViewport(mViewport[0], mViewport[1], mViewport[2], mViewport[3]);
 
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND); 
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	glEnable(GL_NORMALIZE);
	
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glClearColor(mBackgroundColor.r,mBackgroundColor.g,mBackgroundColor.b,1);
}

void RGE::castShadaw()
{
	if(!currentCamera())
		return;
	
	setRenderBlend(false);

	const rgeMatrix4x4& sceneProj = currentCamera()->projectionMatrix();
	const rgeMatrix4x4& sceneModelView = currentCamera()->cameraMatrix();
	
	for(vector<ShadowMapDesc>::iterator i=mShadowMapList.begin(); 
		i != mShadowMapList.end(); ++i) {
//		rgeMatrix4x4 lightModelView = findFrame("Lamp")->matrixWorld().inversed();
		mShadowMapper->drawShadow(this,
								 sceneModelView, 
								 sceneProj, 
								 i->lightMatrix, 
								 i->fov,
								 i->range,
								 i->intencity
								 );
	}
	clearShadowMapLishts();
}

void RGE::drawLightSceneCallback()
{
	bool bm = isMaterialEnable();
	bool bl = isLightEnable();
	enableLight(true);
	enableMaterial(false);
	setLayerVisibility(LAYER_SHADOW_ONLY, true);
	drawScene();
	setLayerVisibility(LAYER_SHADOW_ONLY, false);
	enableLight(bl);
	enableMaterial(bm);
}

void RGE::calcModelToWorldMatrix() 
{
	glMatrixMode(GL_MODELVIEW);
	
	if(mbClearMatrix)
		glLoadIdentity();
	
	mRootFrame->worldIssue(); //主にカメラの位置を決定	
}

void RGE::applyCameraTransform()
{
	if(mCurrentCamera)
		mCurrentCamera->viewTransform(mAspectRatio);	
}

void RGE::lighting()
{
	mRootFrame->viewIssue(); //主にライトの位置を決定	
}

void RGE::drawScene()
{
//	glMatrixMode(GL_MODELVIEW);
	setRenderBlend(false);
	rgeGlGetVector(GL_MODELVIEW_MATRIX, mCameraMatrix.get());
	mRootFrame->render();
	
}

void RGE::drawZOnly()
{
	u_int zmask = zmaskLayer();
	u_int lmask = visibleLayer();
	setZMaskLayer(0);
	
	setVisibleLayerMask(0);
	setLayerVisibility(zmask, true);
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	
	drawScene();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	setVisibleLayerMask(lmask);
	setZMaskLayer(zmask);
}

void RGE::registerChildFrames(FrameRef f)
{
	for(int i=0; i<f->numChildren(); ++i) {
		FrameRef cf = f->childByIndex(i);
		registerFrame(cf->getId(), cf);
		
		registerChildFrames(cf);
	}
}

void RGE::registerFrame(const string& name, FrameRef f) { mFrames.insert(FrameRefMap::value_type(name, f) ); }

MaterialRef RGE::findMaterial(const string& name) {
	MaterialRefMap::iterator m = mMaterials.find(name);
	
	if(m != mMaterials.end())
		return m->second;
	else {
		logger() << "RGE: Material " << name << " is not found" << "\n";
		return MaterialRef((Material*)0);
	}
}

TextureRef RGE::findTexture(const string& name) {
	TextureRefMap::iterator t = mTextures.find(name);
	
	if(t != mTextures.end())
		return t->second;
	else {
		logger() << "RGE: texture " << name << " is not found" << "\n";
		return TextureRef((Texture*)0);
	}
}

MeshObjectRef RGE::findMesh(const string& name) {
	MeshObjRefMap::iterator m = mMeshObjs.find(name);
	
	if(m != mMeshObjs.end())
		return m->second;
	else {
		logger() << "RGE: mesh " << name << " is not found" << "\n";
		return MeshObjectRef((MeshObject*)0);
	}
}

CameraRef RGE::findCamera(const string& name) {
	CameraRefMap::iterator m = mCameras.find(name);
	
	if(m != mCameras.end())
		return m->second;
	else {
		logger() << "RGE: Camera " << name << " is not found" << "\n";
		return CameraRef((Camera*)0);
	}
}

LightRef RGE::findLight(const string& name) {
	LightRefMap::iterator m = mLights.find(name);
	
	if(m != mLights.end())
		return m->second;
	else {
		logger() << "RGE: Light " << name << " is not found" << "\n";
		return LightRef((Light*)0);
	}
}

ActionRef RGE::findAction(const string& name) {
	ActionRefMap::iterator m = mActions.find(name);
	
	if(m != mActions.end())
		return m->second;
	else {
		logger() << "RGE: Action " << name << " is not found" << "\n";
		return ActionRef((Action*)0);
	}
}

FrameRef RGE::findFrame(const string& name) {
	FrameRefMap::iterator f = mFrames.find(name);
	
	if(f != mFrames.end())
		return f->second;
	else {
		logger() << "RGE: Frame " << name << " is not found" << "\n";
		return FrameRef((Frame*)0);
	}
}

MaterialRef RGE::makeMaterial(const string& name) { 
	MaterialRef ref = newMaterial();
	ref->setId(name);
	mMaterials.insert(MaterialRefMap::value_type(ref->getId(), ref));
	logger() << "create material " << name << "\n";
	
	return ref;
}
#ifndef NO_PHYSICS

FrameRef RGE::createSphereBody(const rgeVector3& pos, rgeScalar radius, rgeScalar mass, FrameRef parent)
{
	if(!parent)
		parent = rootFrame(); //parentがないなら、rootFrameにする。
		
	FrameRef f = parent->createChild();
	
	Renderable* r;
	f->addRenderable(r = new SphereMesh(radius));//
	f->addMaterial(Material::getTemplate(Material::COLORNAME_WHITE));
	f->setTranslation(pos);
	
	glPushMatrix();
	glLoadIdentity();
	rootFrame()->worldIssue();
	glPopMatrix();
	
	f->setStatic(false);
	f->enablePhysics(true);
	
	f->bindByBody( getPE()->createBodyWithBoundingSphere(mass, r));
	
	return f;
}

FrameRef RGE::createBoxBody(const rgeVector3& pos, rgeScalar size, rgeScalar mass, FrameRef parent)
{
	if(!parent)
		parent = rootFrame();
	
	FrameRef f = parent->createChild();
	
	Renderable* r;
	f->addRenderable(r = new CubeMesh(size));
	f->addMaterial(Material::getTemplate(Material::COLORNAME_WHITE));
	f->setTranslation(pos);
	
	glPushMatrix();
	glLoadIdentity();
	rootFrame()->worldIssue();
	glPopMatrix();
	
	f->setStatic(false);
	f->enablePhysics(true);
	
	f->bindByBody( getPE()->createBodyWithBoundingBox(mass, r));
	
	return f;
}

FrameRef RGE::createConvexHullBody(const rgeVector3& pos, MeshObjectRef mesh, rgeScalar mass, FrameRef parent)
{
	if(!parent)
		parent = rootFrame();
	
	FrameRef f = parent->createChild();
	
	f->addRenderable(mesh);
	f->addMaterial(Material::getTemplate(Material::COLORNAME_WHITE));
	f->setTranslation(pos);
	
	glPushMatrix();
	glLoadIdentity();
	rootFrame()->worldIssue();
	glPopMatrix();
	
	f->setStatic(false);
	f->enablePhysics(true);
	
	f->bindByBody( getPE()->createConvexHullBody(mass, mesh.get()));
	
	return f;
}

#endif