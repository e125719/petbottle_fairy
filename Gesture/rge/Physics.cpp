/*
 *  Physics.cpp
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 09/07/28.
 *  Copyright 2009 Yuhei Akamine. All rights reserved.
 *
 */
#ifndef NO_PHYSICS
#include <boost/shared_ptr.hpp>

#include "Physics.h"
#include "ConvexDecomposition/ConvexBuilder.h"

#include "BulletCollision/CollisionShapes/btShapeHull.h"

#include "Frame.h"

using namespace rge;

void PhBody::getWorldTransform(btTransform& trans) const 
{
	const Frame* f = getBindedFrame();
	rgeMatrix4x4 m = f->matrixWorld();
	trans.setFromOpenGLMatrix(m.getOpenGLMatrix());
}

void PhBody::setWorldTransform(const btTransform& trans)
{
	rgeScalar mat[16]; 
	trans.getOpenGLMatrix(mat);
	
	getBindedFrame()->setMatrix(rgeMatrix4x4(mat));
	getBindedFrame()->setTranslation(0,0,0);
	getBindedFrame()->setRotation(0,0,0);
	getBindedFrame()->setScale(rgeVector3(1,1,1));
	
	getBindedFrame()->setCoordSpace(Frame::WORLD_SPACE);
}

static void addMeshFromConvexShape(TriMeshObject* mesh, btConvexHullShape* convex, rgeVector3 origin)
{
	btShapeHull hull(convex);
	
	btScalar margin= convex->getMargin();
	hull.buildHull(margin);
	
	int voffset = mesh->numPositions();
	
	int i;
	for(i=0; i<hull.numVertices(); ++i) {
		const btVector3* v = hull.getVertexPointer()+i;
		mesh->addPosition(rgeVector3(v->x(), v->y(), v->z())+ origin);
	}
	
	int iMesh = mesh->makeTriangleMesh(Material::getTemplate(Material::COLORNAME_GREEN));
									   
	const unsigned int* idx = hull.getIndexPointer();
	const btVector3* vtx = hull.getVertexPointer();
	int index = 0;
	for(i=0; i<hull.numTriangles(); ++i) {
		int iv1 = idx[index++] ;
		int iv2 = idx[index++] ;
		int iv3 = idx[index++] ;
		
		btVector3 v1 = vtx[iv1];
		btVector3 v2 = vtx[iv2];
		btVector3 v3 = vtx[iv3];
		btVector3 n = (v3-v1).cross(v2-v1);
		n.normalize();
		
		rgeVector3 normal(n.x(), n.y(), n.z());
		
		mesh->addTriangle(iMesh, 
						  iv1+voffset, normal, color4(), rgeVector2(), 
						  iv2+voffset, normal, color4(), rgeVector2(), 
						  iv3+voffset, normal, color4(), rgeVector2());
	}
}

btCollisionShape* PhysicsEngine::createTriangleMeshByConvexDecomposition(TriMeshObject* mesh)
{
	class MyConvexDecomposition : public ConvexDecomposition::ConvexDecompInterface
	{		
	public:
		btAlignedObjectArray<btConvexHullShape*> m_convexShapes;
		btAlignedObjectArray<btVector3> m_convexCentroids;
		
		MyConvexDecomposition (PhysicsEngine* pe)
		: mPE(pe)
		{
		}
		
		virtual void ConvexDecompResult(ConvexDecomposition::ConvexResult &result)
		{
//			btTriangleMesh* trimesh = new btTriangleMesh();
//			m_convexDemo->m_trimeshes.push_back(trimesh);
			
			btVector3 localScaling(1.f,1.f,1.f);
			
			//calc centroid, to shift vertices around center of mass
			btVector3 centroid;
			centroid.setValue(0,0,0);
				
			btAlignedObjectArray<btVector3> vertices;
			if ( 1) 
			{
				//const unsigned int *src = result.mHullIndices;
				for (unsigned int i=0; i<result.mHullVcount; i++)
				{
					btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
					vertex *= localScaling;
					centroid += vertex;
					
				}
			}
			
			centroid *= 1.f/(float(result.mHullVcount) );
			
			if ( 1 )
			{
				//const unsigned int *src = result.mHullIndices;
				for (unsigned int i=0; i<result.mHullVcount; i++)
				{
					btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
					vertex *= localScaling;
					vertex -= centroid ;
					vertices.push_back(vertex);
				}
			}
				
			
/*			
			if ( 1 )
			{
				const unsigned int *src = result.mHullIndices;
				for (unsigned int i=0; i<result.mHullTcount; i++)
				{
					unsigned int index0 = *src++;
					unsigned int index1 = *src++;
					unsigned int index2 = *src++;
					
					
					btVector3 vertex0(result.mHullVertices[index0*3], result.mHullVertices[index0*3+1],result.mHullVertices[index0*3+2]);
					btVector3 vertex1(result.mHullVertices[index1*3], result.mHullVertices[index1*3+1],result.mHullVertices[index1*3+2]);
					btVector3 vertex2(result.mHullVertices[index2*3], result.mHullVertices[index2*3+1],result.mHullVertices[index2*3+2]);
					vertex0 *= localScaling;
					vertex1 *= localScaling;
					vertex2 *= localScaling;
					
					vertex0 -= centroid;
					vertex1 -= centroid;
					vertex2 -= centroid;
					
					
					trimesh->addTriangle(vertex0,vertex1,vertex2);
					
					index0+=mBaseCount;
					index1+=mBaseCount;
					index2+=mBaseCount;					
				}
			}*/
				
	//		float mass = 1.f;
				//float collisionMargin = 0.01f;
				
				//this is a tools issue: due to collision margin, convex objects overlap, compensate for it here:
				//#define SHRINK_OBJECT_INWARDS 1
#ifdef SHRINK_OBJECT_INWARDS
			std::vector<btVector3> planeEquations;
			btGeometryUtil::getPlaneEquationsFromVertices(vertices,planeEquations);
			
			std::vector<btVector3> shiftedPlaneEquations;
			for (int p=0;p<planeEquations.size();p++)
			{
				btVector3 plane = planeEquations[p];
				plane[3] += 5*collisionMargin;
				shiftedPlaneEquations.push_back(plane);
			}
			std::vector<btVector3> shiftedVertices;
			btGeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations,shiftedVertices);
			
			
			btConvexHullShape* convexShape = new btConvexHullShape(&(shiftedVertices[0].getX()),shiftedVertices.size());
			
#else //SHRINK_OBJECT_INWARDS
			
			btConvexHullShape* convexShape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
#endif 
			
			convexShape->setMargin(0.1);
			m_convexShapes.push_back(convexShape);
			m_convexCentroids.push_back(centroid);
			mPE->m_collisionShapes.push_back(convexShape);
//			mBaseCount+=result.mHullVcount; // advance the 'base index' counter.
		}
		
		PhysicsEngine* mPE;
//		int   	mBaseCount;
//		int		mHullCount;		
	};
	
	floats vertices;
	ints indices;
	
	rgeVector3s v;
	mesh->getTriangles(v);
	
	vertices.reserve(v.size()*3);
	
	int i;
	for(i=0; i<v.size(); ++i) {
		vertices.push_back(v[i].x);
		vertices.push_back(v[i].y);
		vertices.push_back(v[i].z);
		
		indices.push_back(i);
	}
	
	ConvexDecomposition::DecompDesc desc;
	desc.mVcount      = v.size();
	desc.mVertices    = &(vertices[0]);
	desc.mTcount      = indices.size()/3;
	desc.mIndices     = (unsigned int*)&(indices[0]);
	desc.mDepth       = 5;
	desc.mCpercent    = 5;
	desc.mPpercent    = 15;
	desc.mMaxVertices = 16;
	desc.mSkinWidth   = 0.0;
	
	MyConvexDecomposition ConvexDecomp(this);
	desc.mCallback = &ConvexDecomp;
	
	ConvexBuilder cb(&ConvexDecomp);
	cb.process(desc);
	
	
	btCompoundShape* compound = new btCompoundShape();
	m_collisionShapes.push_back (compound);
	
	btTransform trans;
	trans.setIdentity();
	for (int i=0;i<ConvexDecomp.m_convexShapes.size();i++)
	{
		
		btVector3 centroid = ConvexDecomp.m_convexCentroids[i];
		trans.setOrigin(centroid);
		btConvexHullShape* convexShape = ConvexDecomp.m_convexShapes[i];
		compound->addChildShape(trans,convexShape);
	}
	//draw collision shape for debug
#if 0
//	btConvexHullShape* h = (btConvexHullShape*)createConvexHull(mesh);
	mesh->clear();
	for (int i=0;i<ConvexDecomp.m_convexShapes.size();i++)
	{
		btVector3 centroid = ConvexDecomp.m_convexCentroids[i];
		btConvexHullShape* convexShape = ConvexDecomp.m_convexShapes[i];
		addMeshFromConvexShape(mesh, convexShape, toRge(centroid));
	}	
//	addMeshFromConvexShape(mesh, h, rgeVector3());
#endif
	return compound;
}


#endif
