/*
 *  GI.h
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 09/08/10.
 *  Copyright 2009 Yuhei Akamine. All rights reserved.
 *
 */
#include <limits>
#include <vector>
#include <map>

#include <boost/random.hpp>

#include <OpenGL/gl.h>

#include "Types.h"
#include "Utils.h"

using namespace std;

namespace rge {
	
typedef double rgeGIScalar ;
typedef vector3<rgeGIScalar> rgeGIVector3;
typedef vector4<rgeGIScalar> rgeGIVector4;
typedef quaternion<rgeGIScalar> rgeGIQuat;
	
typedef vector<rgeGIVector3> rgeGIVector3s;
typedef vector<rgeGIVector4> rgeGIVector4s;

const rgeGIScalar ZERO_THRE = 0.01;
	
inline bool isZero(rgeGIScalar a) { return fabs(a) < ZERO_THRE; }
inline int signOf(rgeGIScalar a) { return (a>0) - (a<0); }	
	
const rgeGIScalar  INVALID_SCALAR = std::numeric_limits<rgeGIScalar>::infinity();
const rgeGIVector3 INVALID_VECTOR3(INVALID_SCALAR,INVALID_SCALAR,INVALID_SCALAR);
	
inline void rgeGIGlVertex(const rgeGIVector3& v) {
	glVertex3d(v.x, v.y, v.z);
}	
	
	
class Ray {
public:
	Ray(rgeGIVector3 org, rgeGIVector3 dir) : org(org), dir(dir) {}
	Ray() {}

	rgeGIVector3 org;
	rgeGIVector3 dir;
};
	
class Photon : public Ray {
public:
	Photon(rgeGIVector3 org, rgeGIVector3 dir, rgeGIScalar e) : Ray(org, dir), energy(e) {}
	Photon(): energy() {}
	rgeGIScalar energy;
};
	
class Plane : public rgeGIVector4 {
public:
	Plane(rgeGIVector3 normal, rgeGIScalar D) {
		x = normal.x;
		y = normal.y;
		z = normal.z;
		w = D;
	}
	
	bool checkIntersectWith(const Ray& r) {
		return (calcIntersectWith(r) != INVALID_VECTOR3);			
	}
	 
	rgeGIVector3 calcIntersectWith(const Ray& r) {
		rgeGIScalar tb = dotProduct(*this, rgeGIVector4(r.dir, 0));
		
		if(isZero(tb))
			return INVALID_VECTOR3;
		
		rgeGIScalar t = -dotProduct(*this, rgeGIVector4(r.org, 1)) / tb;
		
		if(0 <= t)
			return r.org + r.dir*t;
		else
			return INVALID_VECTOR3;
	}
};

class Triangle {
public:
	Triangle(const rgeGIVector3& a, const rgeGIVector3& b, const rgeGIVector3& c)
	:v1(a), v2(b), v3(c) 
	{}
	
	rgeGIVector3 normal() {
		return crossProduct(v2-v1, v3-v1).normalized();
	}
	
	Plane plane() {
		rgeGIVector3 n = normal();
		rgeGIScalar  D = -dotProduct(n, v1);
		
		return Plane(n,D);
	}
	
	bool checkIntersectWithLine(const Ray& r) {
		rgeGIVector3 PA = v1 - r.org;
		rgeGIVector3 PB = v2 - r.org;
		rgeGIVector3 PC = v3 - r.org;
		const rgeGIVector3& v  = r.dir;
		
		int sab = signOf(v.dot(PA.cross(PB)));
		int sbc = signOf(v.dot(PB.cross(PC)));
		int sca = signOf(v.dot(PC.cross(PA)));
		return ( (sab == sbc) && (sbc == sca) );		
	}
	
	bool checkIntersectWithSegment(const Ray& r) {
		return (checkIntersectWithLine(r) && 
				plane().checkIntersectWith(r)) ;// && (sab != 0 && sbc != 0 && sca != 0);
	}
	
	rgeGIVector3 calcIntersectWithSegment(const Ray& r) {
		if(checkIntersectWithLine(r)) {
			return plane().calcIntersectWith(r);
		}else {
			return INVALID_VECTOR3;
		}

	}
	
	rgeGIVector3 v1,v2,v3;
};
	
inline bool checkCross(const Ray& ray, const Plane& plane)
{
	return isZero(dotProduct(plane, rgeGIVector4(ray.dir, 0)));
}

class PhotonTriangle 
{
public:
	PhotonTriangle(int v1, int v2, int v3, rgeGIScalar _energy=0)
	:energy(_energy) { v[0] = v1; v[1] = v2; v[2]= v3; }
	
	PhotonTriangle() : energy() {}
	
	~PhotonTriangle() {}
	
	int v[3];
	rgeGIScalar energy;
};
	
typedef std::vector<PhotonTriangle> PhotonTriangles;
	
struct PhotonVertex : public rgeGIVector3 {
	PhotonVertex(const rgeGIVector3& _v, rgeGIScalar energy=0): rgeGIVector3(_v), e(energy) {}
	rgeGIScalar  e;
};

typedef std::vector<PhotonVertex> PhotonVertices;
/*
class LightMap
{
public:
	LightMap() {}
	
	GLuint mTexWidth;
	GLuint mTexHeight;
	
	GLuint mColorTex;
	GLuint mFrameBuf;	
};
*/	
class PhotonMap
{
public:
	PhotonMap() {}
	void create(const rgeVector3s& triangles) {
		mTriangles.reserve(triangles.size()/3);

		
		typedef std::map<rgeGIVector3, int> Vmap;
		Vmap vmap;
		
		size_t i,j;
		for(i=0; i<triangles.size(); i+=3) {
			PhotonTriangle tri;
			
			for(j=0; j<3; ++j) {
				rgeGIVector3 v = triangles[i+j].cast<rgeGIScalar>();
				Vmap::iterator m = vmap.find(v);
				
				if(m == vmap.end()) {
					m = vmap.insert(Vmap::value_type(v, (int)mVertices.size())).first;
					mVertices.push_back(v);
				}
				int n = m->second;
				tri.v[j] = (n);
			}
			
			mTriangles.push_back(tri);
		}
	}
	~PhotonMap() {}

	float frand(float f=1) { 
//		return (float)rand() / RAND_MAX *f; 
//		static boost::minstd_rand gen(42);
		static boost::mt19937 gen(42);
		static boost::uniform_real<> dst(0,1);
		static boost::variate_generator<boost::mt19937&, boost::uniform_real<> > rand(gen, dst);
		return rand() *f;
	}
	
	void emit(Photon p, PhotonVertices& trace) {
	
		//trace.push_back(PhotonVertex(p.org, p.energy));
		
		p.dir = randDir(p.dir, 0.05);
		
		int i;
		for(i=0; i<6; ++i) {
			Photon p2;
			reflectRay(p, p2);
			
//			if(i==2) {
//				trace.push_back(PhotonVertex(p.org, p.energy));
				trace.push_back(PhotonVertex(p2.org, p.energy));
//			}
			
			p = p2;
		}
	}
	
	rgeGIVector3 randDir(const rgeGIVector3& axis, rgeGIScalar scatt) {
		const double PI = 3.1415926535;
		rgeGIScalar theta = acos((frand(scatt)+(1.0-scatt)));
		rgeGIScalar phi   = 2.f*PI*(frand(1));
		
		rgeGIVector3 r(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
		//r = rgeGIVector3(0,0,1);
		
		rgeGIVector3 va;
		if(fabs(axis.x) < fabs(axis.y)) {
			if(fabs(axis.x) < fabs(axis.z))
				va.x = 1;
			else
				va.z = 1;
		}else{
			if(fabs(axis.y) < fabs(axis.z))
				va.y = 1;
			else
				va.z = 1;
		}
		
		rgeGIVector3 bz = axis;//.normalized();
		rgeGIVector3 bx = axis.cross(va).normalized();
		rgeGIVector3 by = bz.cross(bx).normalized();
		
		rgeGIVector3 v(
					 bx.x*r.x + by.x*r.y + bz.x * r.z,
					 bx.y*r.x + by.y*r.y + bz.y * r.z,
					 bx.z*r.x + by.z*r.y + bz.z * r.z);
					 
		return v;
	}
	
	void reflectRay(const Photon& in_ray, Photon& out_ray)
	{
//		int refc=0;
		size_t i; 
		for(i=0; i<mTriangles.size(); ++i) {
			PhotonTriangle& ptri = mTriangles[i];
			
			Triangle tri(mVertices[ptri.v[0]],mVertices[ptri.v[1]],mVertices[ptri.v[2]]);
			
			rgeGIVector3 ref_point = tri.calcIntersectWithSegment(in_ray);
			rgeGIVector3 n = tri.normal();

			if(ref_point != INVALID_VECTOR3 && (in_ray.org - ref_point).length() > 0.001
				&& n.dot(in_ray.dir)<0 ) {
				
				//rgeGIVector3 ref = rgeGIQuat::rotationBySinCos(1, 0, n).apply(-in_ray.dir).normalized();
				
//				out_ray.dir = (ref*0.99 + v.normalized()*0.01)*100;
				out_ray.dir = randDir(n, 1);
				out_ray.org = ref_point;
				out_ray.energy = in_ray.energy/4*2;
				
				rgeGIScalar absorbed_e = in_ray.energy/4*2;
				ptri.energy += absorbed_e;
//				ptri.energy = 1;
				
				rgeGIScalar sum_d=0, d[3] = {0,0,0};
				for(int j=0; j<3; ++j) {
					sum_d += (d[j] = (rgeGIScalar)1/((mVertices[ptri.v[j]]-ref_point).length()+0.0000001));
				}
				for(int j=0; j<3; ++j) {
					rgeGIScalar e = absorbed_e * (d[j]/sum_d);
					mVertices[ptri.v[j]].e += e;
//					mVertices[ptri.v[j]].e = absorbed_e;
				}
//				refc ++;
			}
		}
//		if(refc >= 2)
//			printf(">2");
		
	}	
	
	void draw()
	{
		rgeGIScalar max_e=0.00001;
		for(size_t i=0; i<mVertices.size(); ++i) {
			max_e = std::max( mVertices[i].e, max_e);
		}
	//	printf("%f\n", max_e);
		glBegin(GL_TRIANGLES);
		
		for(size_t i=0; i<mTriangles.size(); ++i) {
			color3 c(1,1,1);
			
			const PhotonTriangle& tri = mTriangles[i];

			//rgeGlColor(c*tri.energy);
			for(int j=0; j<3; ++j) {
				rgeGlColor (c * (mVertices[tri.v[j]].e / max_e) );
				rgeGIGlVertex(mVertices[tri.v[j]]);
			}
		}
		
		glEnd();
	}
	
private:
	PhotonVertices  mVertices;
	PhotonTriangles mTriangles;
};
	
	
}