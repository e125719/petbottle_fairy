/*
 *  Physics.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/29.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */

#pragma once 

#ifdef NO_PHYSICS 


#else

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
using namespace std;


#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "LinearMath/btAlignedObjectArray.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

	
#include "Frame.h"

namespace rge {

class PhysicsEngine;


inline btVector3 toBt(rgeVector3 v) {
	return btVector3(v.x, v.y, v.z);
}

inline rgeVector3 toRge(btVector3 v) {
	return rgeVector3(v.x(), v.y(), v.z());
}

class Frame;
//!物理エンジンにおけるRigitBody(固いオブジェクト）を管理する
/*!
	BulletのbtRigitBodyの派生クラスであるので，詳細はBulletのドキュメントを参照
 */
class PhBody : public btRigidBody, public btMotionState
{
public:
	PhBody(const btRigidBodyConstructionInfo &constructionInfo) : btRigidBody(constructionInfo) {
	}
	
	void bindFrame(Frame* f) {
		setUserPointer(f);
		setMotionState(this);
	}
	
	Frame* getBindedFrame() {
		return (Frame*)getUserPointer();
	}
	
	const Frame* getBindedFrame() const {
		return (Frame*)getUserPointer();
	}	

	void getWorldTransform(btTransform& trans) const;
	void setWorldTransform(const btTransform& trans);
	
	//! ローカル座標系を用いてトルクを指定する(bulletにはないので）
	virtual void applyLocalTorque(rgeVector3 ltq) {
		btVector3 tq =  localToWorld(toBt(ltq)) ; 

		activate(true);
		applyTorque(tq);			
	}

	//! ローカル座標系を用いてトルクインパルス（トルクx時間)を指定する(bulletにはないので）
	virtual void applyLocalTorqueImpulse(rgeVector3 ltq) {
		btVector3 tq =  localToWorld(toBt(ltq)) ; 
		
		activate(true);
		applyTorqueImpulse(tq);
	}
	
	//! ローカル座標をワールド座標へ変換
	btVector3 localToWorld(btVector3 lp) {
		btTransform t;
		
		t = getCenterOfMassTransform();
		t.setOrigin(btVector3(0,0,0));
		return  t * lp ; 
	}
};

//!物理エンジンクラス
/*!
	衝突オブジェクトとは，衝突判定にのみ利用される単純形状を表現するオブジェクトである．描画されるRenderableオブジェクトとは関係ない
 */
class PhysicsEngine : public RGEObject
{
	///this is the most important class
	btDynamicsWorld*		m_dynamicsWorld;
	
	//keep the collision shapes, for deletion/cleanup
	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;
	btBroadphaseInterface*	m_broadphase;
	btCollisionDispatcher*	m_dispatcher;
	btConstraintSolver*	m_solver;
	btDefaultCollisionConfiguration* m_collisionConfiguration;
	
public:
	PhysicsEngine() : RGEObject()
	{
		///collision configuration contains default setup for memory, collision setup
		m_collisionConfiguration = new btDefaultCollisionConfiguration();

		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration);

		m_broadphase = new btDbvtBroadphase();

		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
		m_solver = sol;

		m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);

		m_dynamicsWorld->setGravity(btVector3(0,0,-10));
		
//		btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher *>(m_dynamicsWorld ->getDispatcher());
//		btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
//		btConcaveConcaveCollisionAlgorithm::registerAlgorithm(dispatcher);
	}
	
	~PhysicsEngine() {
		//cleanup in the reverse order of creation/initialization
		
		//remove the rigidbodies from the dynamics world and delete them
		int i;
		for (i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
		{
			btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
/*			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}*/
			m_dynamicsWorld->removeCollisionObject( obj );
			delete obj;
		}
		
		//delete collision shapes
		for (int j=0;j<m_collisionShapes.size();j++)
		{
			btCollisionShape* shape = m_collisionShapes[j];
			delete shape;
		}

	
		delete m_dynamicsWorld;
		delete m_solver;
		delete m_broadphase;
		delete m_dispatcher;
		delete m_collisionConfiguration;		
	}
	//!< 重力加速度を設定
	void setGravity(rgeScalar x, rgeScalar y, rgeScalar z) { 
		m_dynamicsWorld->setGravity(btVector3(x,y,z));
	}
	
	//!< Boxの衝突オブジェクト
	btCollisionShape* createBox(rgeScalar lx, rgeScalar ly, rgeScalar lz) {
		btCollisionShape* s = new btBoxShape(btVector3(btScalar(lx/2),btScalar(ly/2),btScalar(lz/2)));
		m_collisionShapes.push_back(s);
		return s;
	}
	
	btCollisionShape* createSphere(rgeScalar r) {
		btCollisionShape* s = new btSphereShape(btScalar(r)); 
		m_collisionShapes.push_back(s);
		return s;
	}
	
	btCollisionShape* createCylinder(rgeScalar r, rgeScalar halfExtent)
	{
		btCollisionShape* s = new btCylinderShapeZ(btVector3(r,r, halfExtent));
		m_collisionShapes.push_back(s);
		return s;												
	}
	
	btCollisionShape* createConvexHull(MeshObject* mesh) {
		/*
		btConvexHullShape* shape = new btConvexHullShape();
		
		rgeVector3s v;
		mesh->getTriangles(v);
		
		for(rgeVector3s::iterator i=v.begin(); i!=v.end(); ++i) {
			btVector3 p(i->x, i->y, i->z); 
			shape->addPoint(p);
		}
				*/
		
		btTriangleMesh* tmpTrimesh = new btTriangleMesh();
		
		rgeVector3s v;
		mesh->getTriangles(v);
		
		for(rgeVector3s::iterator i=v.begin(); i!=v.end(); ) {
			btVector3 v1(i->x, i->y, i->z); ++i;
			btVector3 v2(i->x, i->y, i->z); ++i;
			btVector3 v3(i->x, i->y, i->z); ++i;
			tmpTrimesh->addTriangle(v1, v2, v3);
		}
		
		btConvexShape* tmpConvexShape = new btConvexTriangleMeshShape(tmpTrimesh);
				
		//create a hull approximation
		btShapeHull* hull = new btShapeHull(tmpConvexShape);
		btScalar margin = tmpConvexShape->getMargin();
		hull->buildHull(margin);
		tmpConvexShape->setUserPointer(hull);
		
		btConvexHullShape* convexShape = new btConvexHullShape();
		for (int i=0;i<hull->numVertices();i++)
		{
			convexShape->addPoint(hull->getVertexPointer()[i]);	
		}	
		
		m_collisionShapes.push_back(convexShape);		
		return convexShape;
	}	
	
	btCollisionShape* createTriangleMesh(MeshObject* mesh) {
		btTriangleMesh* trimesh = new btTriangleMesh();
		
		rgeVector3s v;
		mesh->getTriangles(v);
		
		for(rgeVector3s::iterator i=v.begin(); i!=v.end(); ) {
			btVector3 v1(i->x, i->y, i->z); ++i;
			btVector3 v2(i->x, i->y, i->z); ++i;
			btVector3 v3(i->x, i->y, i->z); ++i;
			trimesh->addTriangle(v1, v2, v3);
		}
/*		
		rgeVector3s vts;
		ints index;
		
		mesh->getTriangleIndexVertices(vts, index);
		
		btTriangleIndexVertexArray* trimesh = new btTriangleIndexVertexArray(
			index.size()/3, &(index[0]), 3*sizeof(int), vts.size(), (btScalar*)&vts[0].x,
			sizeof(rgeVector3));
		*/
		btBvhTriangleMeshShape* s = new btBvhTriangleMeshShape(trimesh, true);
//		btConvexTriangleMeshShape* s = new btConvexTriangleMeshShape(trimesh);
//		btGImpactMeshShape * s = new btGImpactMeshShape(trimesh);
//		s->setLocalScaling(btVector3(4.f,4.f,4.f));
//		s->updateBound();

/*		btGImpactConvexDecompositionShape* s = new btGImpactConvexDecompositionShape(
			trimesh, btVector3(1,1,1), btScalar(0.01));
		s->updateBound();
*/
		m_collisionShapes.push_back(s);		
		return s;
	}
	
	btCollisionShape* createTriangleMeshByConvexDecomposition(TriMeshObject* mesh);
	
	
	btCollisionShape* createPlane(rgeVector3 normal, rgeScalar offset) {
		btCollisionShape* s = new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), offset);
		m_collisionShapes.push_back(s);
		return s;
	}
	
	
	PhBody* createRigitBody(btCollisionShape* shape, rgeScalar mass, rgeMatrix4x4 trans) {
		btTransform mat;
		mat.setFromOpenGLMatrix(trans.getOpenGLMatrix());
		
		btVector3 localInertia(0,0,0);
		if(mass != rgeScalar(0)) {
			shape->calculateLocalInertia(mass,localInertia);
		}
		
//		btDefaultMotionState* myMotionState = new btDefaultMotionState(mat);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,0,shape,localInertia);
		PhBody* body = new PhBody(rbInfo);

		//add the body to the dynamics world
		m_dynamicsWorld->addRigidBody(body);
		
		return body;
	}
	
	PhBody* createBoxBody(rgeScalar mass, rgeScalar lx, rgeScalar ly, rgeScalar lz) {
		btCollisionShape* shape = createBox(lx,ly,lz);
		
		PhBody* b = createRigitBody(shape, mass, rgeMatrix4x4());
		return b;		
	}
	
	PhBody* createSphereBody(rgeScalar mass, rgeScalar r) {
		btCollisionShape* shape = createSphere(r);
		PhBody* b = createRigitBody(shape, mass, rgeMatrix4x4());
		return b;
	}
	

	
	PhBody* createConvexHullBody(rgeScalar m, MeshObject* mesh) {
		btCollisionShape* shape = createConvexHull(mesh);
		shape->setMargin(0.001);
		PhBody* b = createRigitBody(shape, m, rgeMatrix4x4());
		return b;
	}	
	
	PhBody* createTriMeshBody(rgeScalar m, MeshObject* mesh) {
		btCollisionShape* shape = createTriangleMesh(mesh);
		shape->setMargin(0.01);
		PhBody* b = createRigitBody(shape, m, rgeMatrix4x4());
		b->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
		
		return b;
	}	
	
	PhBody* createConvexDecompositionBody(rgeScalar m, TriMeshObject* mesh) {
		btCollisionShape* shape = createTriangleMeshByConvexDecomposition(mesh);
		shape->setMargin(0.1);
		PhBody* b = createRigitBody(shape, m, rgeMatrix4x4());
//		b->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
		
		return b;
	}		
	
	PhBody* createBodyWithBoundingBox(rgeScalar m, Renderable* mesh) {
		rgeVector3 maxv, minv;
		
		mesh->getBoundingBox(maxv, minv);
		
		rgeVector3 size = maxv -minv;
		
		return createBoxBody(m,size.x,size.y,size.z);
	}

	PhBody* createBodyWithBoundingSphere(rgeScalar m, Renderable* mesh) {
		rgeVector3 maxv, minv;
		
		mesh->getBoundingBox(maxv, minv);
		
		rgeVector3 size = maxv -minv;
		rgeScalar r = (size.x+size.y+size.z) / 3 / 2 ;
		
		return createSphereBody(m, r);
		
	}
	
	PhBody* createBodyWithBoundingCylinder(rgeScalar mass, Renderable* rend) {
		rgeVector3 maxv, minv;
		rend->getBoundingBox(maxv, minv);
		rgeVector3 size = maxv -minv;	
		
		btCollisionShape* shape = createCylinder(//(rgeScalar)sqrt(size.x*size.x+size.y*size.y)/2.f
												 (rgeScalar)size.x/2.f,
												 size.z/2.f);
		PhBody* b = createRigitBody(shape, mass, rgeMatrix4x4());
		
		return b;
	}	
	
	void setPlane(rgeVector3 norm, rgeScalar offset) {
		 createRigitBody(
					createPlane(norm,offset),
					rgeScalar(0), // zero mass means static object
				   rgeMatrix4x4());
	}
	
	void createHingeZ(FrameRef fa, FrameRef fb) {
		PhBody *a = fa->getPhBody();
		PhBody *b = fb->getPhBody();
		btTransform m1, m2;
//		btHingeConstraint* ct;
		
		//t0のz軸方向に拘束される．拘束位置は，t0の原点となる
		m1.setIdentity();
		m2 = b->btCollisionObject::getWorldTransform().inverse() * a->btCollisionObject::getWorldTransform();	//bdからjointへの変換
		world()->addConstraint( new btHingeConstraint(*a,*b,
													  m1, m2 ));		
	}
	
	void step(rgeScalar s) {
		if(m_dynamicsWorld) {
			m_dynamicsWorld->stepSimulation(s /*/ 1000000.f*/, 20/*max sub steps*/);
		}
	}
	
	btDynamicsWorld* world() { return m_dynamicsWorld; }
};

}

#endif