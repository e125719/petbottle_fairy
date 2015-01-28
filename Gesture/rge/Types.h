/*
 *  Types.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/02.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once
#include <cmath>
#include <iostream>
#include <vector>


using std::istream;

namespace rge {
	
inline int max(int a, int b) { return a > b ? a : b; }

typedef unsigned int u_int;


template<class T>
struct vector2 {
	vector2(): x(), y(){}
	vector2(double _x, double _y): x(_x), y(_y) {}
	
	vector2 operator - (const vector2& rhs) const { return vector2(x-rhs.x, y-rhs.y); }
	vector2 operator + (const vector2& rhs) const { return vector2(x+rhs.x, y+rhs.y); }
	vector2 operator * (double rhs) const { return vector2(x*rhs, y*rhs); }
	vector2 operator / (double rhs) const { return vector2(x/rhs, y/rhs); }
	
	vector2 operator +=(const vector2& rhs) { x+=rhs.x; y+=rhs.y; return *this; }
	vector2 operator -=(const vector2& rhs) { x-=rhs.x; y-=rhs.y; return *this; }
	
	vector2 normalized() const { T l = length(); return l==0?vector2():vector2(x/l, y/l); }

	bool operator ==(const vector2& rhs) const { return x==rhs.x&&y==rhs.y;}
	bool operator !=(const vector2& rhs) const { return x!=rhs.x||y!=rhs.y;}

        T mag() const { return length(); }
	T length() const { return (T)sqrt(sq_length()); }
	T sq_length() const { return (T)(x*x+y*y); }
	vector2 rotate(T r) {
		T s = std::sin(r);
		T c = std::cos(r);

		double x_ = x * c - y * s;
		double y_ = x * s + y * c;
		//x =_x; y = _y;
		return vector2(x_,y_);
	}
	
        bool equals(const vector2<T>& rhs)
        {
            return operator==(rhs);
        }
        
        T cross(const vector2<T>& rhs) const
        {
            return x*rhs.y - y*rhs.x;
        }
        
        static T cross2d(const vector2<T>& a, const vector2<T>& b)
        {
            return a.x*b.y - a.y*b.x;
        }
        
        void normalize()
        {
            operator/(length());
        }
        
	T x;
	T y;
};

template<class T>
vector2<T> operator*(T a, const vector2<T>& b )
{
    return b * a;
}

template<class T>
struct vector3 {
	typedef T value_type;
	
	vector3(): x(), y(), z() {}
	vector3(double _x, double _y, double _z): x(_x), y(_y), z(_z) {}
	
	template<typename U>
	vector3<U> cast() const { return vector3<U>((U)x, (U)y, (U)z); }
	
	vector3 operator - () const { return vector3(-x, -y, -z); }
	
	vector3 operator - (const vector3& rhs) const { return vector3(x-rhs.x, y-rhs.y, z-rhs.z); }
	vector3 operator + (const vector3& rhs) const { return vector3(x+rhs.x, y+rhs.y, z+rhs.z); }
	vector3 operator * (double rhs) const { return vector3(x*rhs, y*rhs, z*rhs); }
	vector3 operator / (double rhs) const { return vector3(x/rhs, y/rhs, z/rhs); }
	
	vector3 operator +=(const vector3& rhs) { x+=rhs.x; y+=rhs.y; z+=rhs.z; return *this; }
	vector3 operator -=(const vector3& rhs) { x-=rhs.x; y-=rhs.y; z-=rhs.z; return *this; }
	
	vector3 normalized() const { T l = length(); return l==0?vector3():vector3(x/l, y/l, z/l); }
	vector3 divideEachBy(const vector3& rhs) { return vector3(x/rhs.x, y/rhs.y, z/rhs.z);}
	bool operator ==(const vector3& rhs) const { return x==rhs.x&&y==rhs.y&&z==rhs.z;}
	bool operator !=(const vector3& rhs) const { return x!=rhs.x||y!=rhs.y||z!=rhs.z;}
	
	bool operator <(const vector3& rhs) const { 
		if(x == rhs.x) {
			if(y == rhs.y)
				return z<rhs.z;
			else
				return y<rhs.y;
		}else{
			return x<rhs.x;
		}
	}
	
	T length() const { return (T)sqrt(sq_length()); }
	T sq_length() const { return (T)(x*x+y*y+z*z); }
	
	T dot(const vector3 b) const { return (x*b.x+ y*b.y+ z*b.z); }
	vector3 cross(const vector3 b) const { return vector3<T>(y*b.z-z*b.y, z*b.x-x*b.z, x*b.y-y*b.x); }
	T x;
	T y;
	T z;
    
};

template<class T>
struct vector4 {
	vector4(vector3<T> d, T _w = (T)1) : x(d.x), y(d.y), z(d.z), w(_w) {}
	vector4(): x(), y(), z(), w(1.0) {}
	vector4(T _x, T _y, T _z, T _w): x(_x), y(_y), z(_z), w(_w) {}
	
	T x;
	T y;
	T z;
	T w;
    
};


template<class T>
vector3<T> crossProduct(const vector3<T>& a, const vector3<T>& b) {
//	return vector3<T>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
	return a.cross(b);
}

template<class T>
T dotProduct(const vector3<T>& a, const vector3<T>& b){
	return a.dot(b);
}


template<class T>
T dotProduct(const vector4<T>& a, const vector4<T>& b){
	return (a.x*b.x+ a.y*b.y+ a.z*b.z + a.w*b.w);
}


struct color3 {
	color3(): r(), g(), b() {}
	color3(float _r, float _g, float _b):r(_r), g(_g), b(_b) {}
	
	color3 operator / ( float rhs ) { return color3(r/rhs, g/rhs, b/rhs); } 
	color3 operator * ( float rhs ) { return color3(r*rhs, g*rhs, b*rhs); } 
	float r;
	float g;
	float b;
    

};

struct color4 : public color3 {
	color4(): color3(), a(1.0) {}
	color4(const color3& c): color3(c), a(1.0) {}
	color4(float _r, float _g, float _b, float _a):color3(_r, _g, _b), a(_a) {}
	
	color4 operator*(float rhs) {
		return color4(r*rhs, g*rhs, b*rhs, a);
	}

	static color4 black() { return color4(0,0,0,1); }
	static color4 white() { return color4(1,1,1,1); }
		
	float a;
    
};

typedef vector2<double> double2;
typedef vector3<double> double3;
typedef vector4<double> double4;
typedef std::vector<double> doubles;
typedef std::vector<double2> double2s;
typedef std::vector<double3> double3s;
typedef std::vector<double4> double4s;

typedef vector2<float> float2;
typedef vector3<float> float3;
typedef vector4<float> float4;
typedef std::vector<float> floats;
typedef std::vector<float2> float2s;
typedef std::vector<float3> float3s;
typedef std::vector<float4> float4s;

typedef std::vector<color3> color3s;
typedef std::vector<color4> color4s;


typedef vector2<int> int2;
typedef vector3<int> int3;
typedef vector4<int> int4;
typedef std::vector<int> ints;


template <class T>
struct matrix4x4 {
	matrix4x4() { identify(); }
	
	static const matrix4x4& identity() { static matrix4x4 i; return i; }
	void identify() {
		T ind[16] = {	1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1 };
		for(unsigned int i=0; i<sizeof(m)/sizeof(T); ++i)
			m[i] = ind[i];
	}
	template<typename U>
	matrix4x4(U* p) { fromArray(p); }

	template<typename U>
	void fromArray(U* p) { for(int i=0; i!=16; ++i) m[i] = (T)p[i]; }
	const T* get() const { return m; }
	const T* getOpenGLMatrix() const { return m; }
	T* get() { return m; }
	T* getOpenGLMatrix()  { return m; }
	T& operator [] (int i)  { return m[i]; }
	T operator [] (int i) const { return m[i]; }
	
	void resetTranslation() { m[12]=m[13]=m[14]=0.0;}
	void setTranslation(vector3<T> t) { m[12]=t.x; m[13]=t.y; m[14]=t.z; }
	vector3<T> translation() { return vector3<T>(m[12], m[13], m[14]); }
	void applyRotationOnly(matrix4x4 r) {
		m[0] = r[0];
		m[1] = r[1];
		m[2] = r[2];

		m[4] = r[4];
		m[5] = r[5];
		m[6] = r[6];

		m[8] = r[8];
		m[9] = r[9];
		m[10] = r[10];
	}
	
	void transpose() {
		matrix4x4 om = *this;
		for(int i=0; i<4; ++i) {
			for(int j=0; j<4; ++j) {
				m[i*4+j] = om[j*4+i];
			}
		}
	}
	
	matrix4x4 inversed() const {
		matrix4x4 inv;
		matrixDelta(&inv, this, &matrix4x4::identity());
		return inv;
	}
	
	static int matrixDelta(matrix4x4 *pOut, const matrix4x4 *pM1, const matrix4x4 *pM2)
	{
		matrix4x4 mat;
		int i, j, loop;
		T fDat, fDat2;
		T mat_8x4[4][8];
		int flag;
		const T *pF;
		T *pD, *pG;
		
		//8 x 4行列に値を入れる
		for(i = 0; i < 4; i++) {
			pF = &pM1->m[i*4];
			for(j = 0; j < 4; j++, pF++) mat_8x4[i][j] = (T)(*pF);
			pD  = mat_8x4[i] + 4;
			for(j = 0; j < 4; j++) {
				/*            if(i == j)   *pD = 1.0;
				 else         *pD = 0.0;*/
				*pD = pM2->m[i*4+j];
				pD++;
			}
		}
		
		flag = 1;
		for(loop = 0; loop < 4; loop++) {
			fDat = mat_8x4[loop][loop];
			if(fDat != 1.0) {
				if(fDat == 0.0) {
					for(i = loop + 1; i < 4; i++) {
						fDat = mat_8x4[i][loop];
						if(fDat != 0.0) break;
					}
					if(i >= 4) {
						flag = 0;
						break;
					}
					//行を入れ替える
					for(j = 0; j < 8; j++) {
						fDat = mat_8x4[i][j];
						mat_8x4[i][j] = mat_8x4[loop][j];
						mat_8x4[loop][j] = fDat;
					}
					fDat = mat_8x4[loop][loop];
				}
				
				for(i = 0; i < 8; i++) mat_8x4[loop][i] /= fDat;
			}
			for(i = 0; i < 4; i++) {
				if(i != loop) {
					fDat = mat_8x4[i][loop];
					if(fDat != 0.0f) {
						//mat[i][loop]をmat[loop]の行にかけて
						//(mat[j] - mat[loop] * fDat)を計算
						for(j = 0; j < 8; j++) {
							fDat2 = mat_8x4[loop][j] * fDat;
							mat_8x4[i][j] -= fDat2;
						}
					}
				}
			}
		}
		
		if(flag){
			for(i = 0; i < 4; i++) {
				pG = &mat.m[i*4];
				pD = mat_8x4[i] + 4;
				for(j = 0; j < 4; j++) {
					*pG = (float)(*pD);
					pG++;
					pD++;
				}
			}
		} else {
			//単位行列を求める
			mat.identify();
		}
		
		*pOut = mat;
		
		if(flag) return 1;
		return 0;
	}

	T m[16];
    
};

typedef matrix4x4<double> double4x4;
typedef matrix4x4<float> float4x4;


template<class T>
struct quaternion {
	quaternion(T w_, T x_, T y_, T z_): w(w_), x(x_), y(y_), z(z_) {}
	quaternion(): w(1.0), x(), y(), z() {}
	
	static quaternion rotation(T t, vector3<T> axis) {
		T hrad;
		T s;
		
		hrad = 0.5 * t;
		s = sin(hrad);
		
		quaternion q;
		q.w = cos(hrad);
		q.x = s * axis.x;
		q.y = s * axis.y;
		q.z = s * axis.z;
		
		return q;
	}
	
	static quaternion rotationBySinCos(T sin_half_theta, T cos_half_theta, vector3<T> axis) {
		//T hrad;
		T s;
		
		//hrad = 0.5 * t;
		s = sin_half_theta;
		
		quaternion q;
		q.w = cos_half_theta;
		q.x = s * axis.x;
		q.y = s * axis.y;
		q.z = s * axis.z;
		
		return q;
	}
	
	static quaternion rotationBetweenAxis(vector3<T> axisA, vector3<T> axisB)
	{
		vector3<T> A = axisA.normalized();
		vector3<T> B = axisB.normalized();
		
		vector3<T> xyz = crossProduct(B, A) / crossProduct(B, A).length();
		T costheta = dotProduct(B, A) / (B.length() * A.length());
		T costheta2= sqrt(0.5*(1+costheta));
		T sintheta2= sqrt(0.5*(1-costheta));
		
		quaternion q;
		q.w = costheta2;
		q.x = sintheta2 * xyz.x;
		q.y = sintheta2 * xyz.y;
		q.z = sintheta2 * xyz.z;
		
		if(A == B)
			return quaternion();
		else
			return q;		
	}
	
	quaternion slerpTo(quaternion r, T t) {
		T qr = w * r.w + x * r.x + y * r.y+ z * r.z;
		T ss = 1.0 - qr * qr;
		
		if (ss == 0.0) {
			return *this;
		}
		else {
			T sp = sqrt(ss);
			T ph = acos(qr);
			T pt = ph * t;
			T t1 = sin(pt) / sp;
			T t0 = sin(ph - pt) / sp;
			
			quaternion p;
			p.w = w * t0 + r.w * t1;
			p.x = x * t0 + r.x * t1;
			p.y = y * t0 + r.y * t1;
			p.z = z * t0 + r.z * t1;
			return p;
		}
	}
	
	vector3<T> apply(vector3<T> p) const {
		quaternion R(w, -x, -y ,-z);
		quaternion P(0, p.x, p.y, p.z);
		
		quaternion res = (R*P)* *this;
		return vector3<T>(res.x, res.y, res.z);
	}
	
	quaternion operator*(const quaternion& rhs) const  {
		double pw, px, py, pz;
		double qw, qx, qy, qz;
		
		pw = w; px = x; py = y; pz = z;
		qw = rhs.w; qx = rhs.x; qy = rhs.y; qz = rhs.z;
		
		return quaternion(
						  pw * qw - px * qx - py * qy - pz * qz,
						  pw * qx + px * qw + py * qz - pz * qy,
						  pw * qy - px * qz + py * qw + pz * qx,
						  pw * qz + px * qy - py * qx + pz * qw );
	}
	
	matrix4x4<T> rotationMatrix() const {
		float qw, qx, qy, qz;
		float x2, y2, z2;
		float xy, yz, zx;
		float wx, wy, wz;
		
		qw = w; qx = x; qy = y; qz = z;
		
		x2 = 2.0 * qx * qx;
		y2 = 2.0 * qy * qy;
		z2 = 2.0 * qz * qz;
		
		xy = 2.0 * qx * qy;
		yz = 2.0 * qy * qz;
		zx = 2.0 * qz * qx;
        
		wx = 2.0 * qw * qx;
		wy = 2.0 * qw * qy;
		wz = 2.0 * qw * qz;
		
		T m00 = 1.0 - y2 - z2;
		T m01 = xy - wz;
		T m02 = zx + wy;
		T m03 = 0.0;
		
		T m10 = xy + wz;
		T m11 = 1.0 - z2 - x2;
		T m12 = yz - wx;
		T m13 = 0.0;
		
		T m20 = zx - wy;
		T m21 = yz + wx;
		T m22 = 1.0 - x2 - y2;
		T m23 = 0.0;
		
		T m30 = 0.0;
		T m31 = 0.0;
		T m32 = 0.0;
		T m33 = 1.0;
		
		T m[16] = {
			m00, m10, m20, m30,
			m01, m11, m21, m31,
			m02, m12, m22, m32,
			m03, m13, m23, m33
		};
		return matrix4x4<T>(m);
	}
	
	void normalize() {
		T l = sqrt(w*w+x*x+y*y+z*z);
		w /= l; x /= l; y /= l; z /= l;
	}
	
	T w,x,y,z;
    
};

typedef quaternion<double> d_quat;


template<class T>
istream& operator >> (istream& is, vector2<T>& v) { return is >> v.x >> v.y; }
template<class T>
istream& operator >> (istream& is, vector3<T>& v) { return is >> v.x >> v.y >> v.z; }
template<class T>
istream& operator >> (istream& is, quaternion<T>& v) { return is >> v.w >> v.x >> v.y >> v.z; }
template<class T>
istream& operator >> (istream& is, double4& v) { return is >> v.x >> v.y >> v.z >> v.w; }

inline istream& operator >> (istream& is, color3& c) { return is >> c.r>>c.g>>c.b; }
inline istream& operator >> (istream& is, color4& c) { return is >> c.r>>c.g>>c.b>>c.a; }

template <class T>
inline istream& operator >> (istream& is, matrix4x4<T>& m) {for(int i=0; i<16; ++i) is >> m.m[i]; return is;}

template <class T>
vector3<T> operator * (const vector3<T> d, const matrix4x4<T> m) {
	vector3<T> r;
	
	r.x = d.x*m[0] + d.y*m[4] + d.z*m[8] + m[12];
	r.y = d.x*m[1] + d.y*m[5] + d.z*m[9] + m[13];
	r.z = d.x*m[2] + d.y*m[6] + d.z*m[10]+ m[14];
	
	return r;
} 

template <class  T>
double pp_distance(const T& a, const T& b)
{
	return (a-b).length();
}

inline double radian(double r) { return r/180.0*3.141592653589; }
inline double degree(double r) { return r*180.0/3.141592653589; }
inline float radian(float r) { return r/180.0*3.141592653589; }
inline float degree(float r) { return r*180.0/3.141592653589; }


#ifdef RGE_USE_DOUBLE
typedef double rgeScalar;
#else
typedef float rgeScalar;
#endif
typedef vector2<rgeScalar> rgeVector2;
typedef vector3<rgeScalar> rgeVector3;
typedef vector4<rgeScalar> rgeVector4; 
typedef matrix4x4<rgeScalar> rgeMatrix4x4;

typedef std::vector<rgeVector2> rgeVector2s;
typedef std::vector<rgeVector3> rgeVector3s;
typedef std::vector<rgeVector4> rgeVector4s;

typedef quaternion<rgeScalar> rgeQuat;

typedef std::vector<rgeScalar> rgeScalars;

typedef vector2<int> rgeVector2i;

typedef int LAYER_NUM;
const LAYER_NUM LAYER_INVISIBLE = 0;
const LAYER_NUM LAYER_1 = 1;
const LAYER_NUM LAYER_2 = 2;
const LAYER_NUM LAYER_3 = 3;
const LAYER_NUM LAYER_4 = 4;
const LAYER_NUM LAYER_5 = 5;
const LAYER_NUM LAYER_6 = 6;
const LAYER_NUM LAYER_7 = 7;
const LAYER_NUM LAYER_8 = 8;
const LAYER_NUM LAYER_9 = 9;
const LAYER_NUM LAYER_10 = 10;
const LAYER_NUM LAYER_11 = 11;
const LAYER_NUM LAYER_12 = 12;
const LAYER_NUM LAYER_13 = 13;
const LAYER_NUM LAYER_14 = 14;
const LAYER_NUM LAYER_15 = 15;
const LAYER_NUM LAYER_16 = 16;
const LAYER_NUM LAYER_17 = 17;
const LAYER_NUM LAYER_18 = 18;
const LAYER_NUM LAYER_19 = 19;
const LAYER_NUM LAYER_SHADOW_ONLY = 20;

const u_int LAYER_MASK_SHOW_ALL = (1<<(LAYER_SHADOW_ONLY-1))-1;

}