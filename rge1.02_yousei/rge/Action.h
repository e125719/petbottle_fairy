/*
 *  Action.h
 *  VrGame
 *
 *  Created by Yuhei Akamine on 08/07/17.
 *  Copyright 2008 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once 

#include "Types.h"
#include "Utils.h"
#include "Bone.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include <boost/algorithm/string.hpp>


using std::string;
using std::vector;
using std::map;

namespace rge {
	
class BezierTriple
{
public:
	BezierTriple() {}
	BezierTriple(rgeVector2 c1_, rgeVector2 p_, rgeVector2 c2_): c1(c1_), p(p_), c2(c2_) {}
	
	void CreateFromRge(std::istream& is) {
		is >> c1.x >> c1.y >> p.x >> p.y >> c2.x >> c2.y;
	}
	
	rgeVector2 c1, p, c2;

private:

};

typedef vector<BezierTriple> BezierTriples;
//typedef map<float, int> float2int;


	
class BezierCurve 
{
public:

	
	enum METHOD {
		CONTROL_POINTS,
		SAMPLED_VALUE,
		SAMPLED_MATRICES
	};
	
	BezierCurve(METHOD m = CONTROL_POINTS) : mMethod(m)  {}
	/*
	TARGET createFromRge(std::istream& is) {
		static 	struct {
			const char* ipo_name;
			TARGET ipo_type;
		} name2type[] = {
			{"LOCX", LOCX},
			{"LOCY", LOCY},
			{"LOCZ", LOCZ},
			{"ROTX", ROTX},
			{"ROTY", ROTY},
			{"ROTZ", ROTZ},
			{"SCAX", SCAX},
			{"SCAY", SCAY},
			{"SCAZ", SCAZ},
		};
		
		string ipu, type;
		int num;
		
		is >> ipu;
		if(ipu != "IPU") {
			unread(is, ipu.c_str());
			return NONE;
		}
		is >> type >> num;
		
		mTarget = NONE;
		
		int i;
		for(i=0; i<num; ++i) {
			BezierTriple bt;
			bt.CreateFromRge(is);
			curve.push_back(bt);
		}
		
		if(num <= 1) {
			curve.clear();
			return NONE;
		}
		
		sampleAll();
		 
		for(i=0; i<END; ++i) {
			if(type == name2type[i].ipo_name) {
				mTarget = name2type[i].ipo_type;
				return name2type[i].ipo_type;
			
			}
		}
		return NONE;
	}*/
	
	void createFromSampledValue(int offset_to_start, const rgeScalars& sample)
	{
		mMethod = SAMPLED_VALUE;
		offset = offset_to_start;
		sampled = sample;
	}
	
	void createEmptySampledValue(int start, int end) 
	{
		mMethod = SAMPLED_VALUE;
		offset = start;
		sampled.resize(end-start);
	}
	
	void setSampledValue(int time, rgeScalar v)
	{
		sampled[time-offset] = v;
	}
	
	void addBezierTriple(const BezierTriple& pt) {
		curve.push_back(pt);
		//timeToIndex.insert(int2int::value_type(pt.p.x, curve.size()-1));
	}
	
	inline rgeScalar bezierFunc(rgeScalar t, rgeScalar x1, rgeScalar x2, rgeScalar x3, rgeScalar x4)
	{
		return  (1-t)*(1-t)*(1-t)*x1 + 3*(1-t)*(1-t)*t*x2 + 3*(1-t)*t*t*x3 + t*t*t*x4;	
	}
	
	void sampleAll(int off)
	{
		offset = off;
//		int s = curve.size(); 
		if(curve.size() <= 1) {
			sampled.clear();
			sampled.push_back(curve.front().p.y);
			return;
		}
		
		sampled.resize((int)curve.back().p.x-offset+1);
		
#if 1
		rgeVector2 prev_pt = curve.front().p;
		
		u_int i;
		for(i=0; i<curve.size()-1; ++i) {
			rgeScalar x1 = curve[i  ].p.x;
			rgeScalar x2 = curve[i  ].c2.x;
			rgeScalar x3 = curve[i+1].c1.x;
			rgeScalar x4 = curve[i+1].p.x;

			rgeScalar y1 = curve[i  ].p.y;
			rgeScalar y2 = curve[i  ].c2.y;
			rgeScalar y3 = curve[i+1].c1.y;
			rgeScalar y4 = curve[i+1].p.y;
			
			for( rgeScalar t=0; t<=1.1f; t+= 0.02 ) {
				rgeScalar x = bezierFunc(t,x1,x2,x3,x4);
				rgeScalar y = bezierFunc(t,y1,y2,y3,y4);
				
				if( (int)x - (int)prev_pt.x > 1) {
					rgeScalar dy = (y - prev_pt.y) / (x - prev_pt.x);
					
					for(int j=(int)prev_pt.x; j<=(int)x; ++j) {
                                            if(j-offset < (int)sampled.size())
						sampled[j-offset] = prev_pt.y + dy*(j-prev_pt.x);
					}
				}
				
                                if(x-offset < sampled.size())
                                    sampled[x-offset] = y;
				
				prev_pt = rgeVector2(x,y);
			}
		}
		
#else

		
		rgeVector2s dpt;
		u_int i;
		for(i=0; i<curve.size()-1; ++i) {
			rgeVector2s pts;
			pts.push_back(curve[i].p);
			pts.push_back(curve[i].c2);
			pts.push_back(curve[i+1].c1);
			pts.push_back(curve[i+1].p);
			pts = divideCurve(pts, 20);
		//	pts.pop_back();
			
			std::copy(pts.begin(), pts.end(), back_inserter(dpt));
		}

		for(i=0; i<sampled.size(); ++i) {
			bool bfound = false;
			for(u_int j=0; j<dpt.size()-1; ++j) {
				if(dpt[j].x <= i && i < dpt[j+1].x) {
					rgeVector2 d = dpt[j+1] - dpt[j];
					rgeScalar dtan = d.y / d.x;
					sampled[i] = dpt[j].y + (i-dpt[j].x)*dtan;
					
					bfound = true;
					break;
				}
			}
			if(!bfound) {
				if(i<=dpt.front().x)
					sampled[i] = dpt.front().y;
				else if(i>= dpt.back().x)
					sampled[i] = dpt.back().y;
			}
		}
#endif
	}
		
	rgeVector2s divideCurve(const rgeVector2s& pts, int level)
	{
		if(level < 0)
			return pts;
		
		rgeVector2s nps;
		
		nps.push_back(pts.front());
		
		u_int i;
		rgeScalar maxdx = 0;
		for(i=0; i<pts.size()-1; ++i) {
			nps.push_back( (pts[i] + pts[i+1])/2.0 );
			maxdx = std::max(maxdx, (rgeScalar)fabs(pts[i+1].x - pts[i].x));
		}
		if(maxdx < 1.0)
			return pts;
		
		nps.push_back(pts.back());
		
		return divideCurve(nps, level -1);
	}
	
	
	rgeScalar getValue(int time, bool bLooping = false) const { 
		if(sampled.empty())
			return 0;
		else if(time - offset < 0)
			return sampled.front(); // constant extrapolation
		else if(time - offset >= (int)sampled.size())
			if(bLooping)
				return sampled[(time-offset) % sampled.size()];
			else
				return sampled.back();
		else
			return sampled[time-offset]; 
	}
	int size() const { return sampled.size(); }
	
	rgeVector2i range() const { return rgeVector2i(offset, offset+size()); }
	
	static void multiply(rgeScalar& v, rgeScalar factor) { v *= factor; }
	
	void scale(rgeScalar s) {
		for_each(sampled.begin(), sampled.end(), bind(multiply, std::placeholders::_1, s));
	}
private:
	rgeVector2s makePoints()
	{
		rgeVector2s pts;
		for(BezierTriples::iterator i=curve.begin(); i!=curve.end(); ++i) {
			if(i!=curve.begin())
				pts.push_back(i->c1);
			pts.push_back(i->p);
			if(i+1 != curve.end())
				pts.push_back(i->c2);
		}
		return pts;
	}
	METHOD mMethod;
	
	BezierTriples curve;
	//int2int timeToIndex;
	
	int offset;

	rgeScalars sampled;
    
private:
  
};

typedef vector<BezierCurve> BezierCurves;

class ValueDriver 
{
public:
	ValueDriver(const BezierCurve& c, const string& obj, const string& member, int index = 0) : 
		curve(c), targetObject(obj), targetMember(member), arrayIndex(index) {}
	
	
	void apply(Frame* f, int time, bool loop, rgeScalar blendFunc(rgeScalar, rgeScalar), rgeScalar factor) const {

		rgeScalar* dst_val =0;
		
		vector<string> toks;
		
		boost::algorithm::split( toks, targetObject, boost::algorithm::is_any_of("/"));
		
		if(toks.size() > 1 && toks[0] == "pose") {
			BoneRef b = f->findBone(toks[1]);
			
			if(!b) {
				cout  << "Bone " << toks[1] << " is not found in " << f->getId() << endl;
				return;
			}
			
			f = static_cast<Frame*>(b.get());
		}else if(targetObject != "object") {
			cout << "Unknown target object " << endl;
			return ;
		}
																										
		if(targetMember == "ROTQ") {
			static rgeScalar rgeQuat::*indexToMember[] = {
				&rgeQuat::w,
				&rgeQuat::x,
				&rgeQuat::y,
				&rgeQuat::z
			};
			
			dst_val = &(f->mQuatanion.*indexToMember[arrayIndex]);
			
		}else {
			static rgeScalar rgeVector3::*indexToMember[] = {
				&rgeVector3::x,
				&rgeVector3::y,
				&rgeVector3::z
			};
			
			rgeVector3 Frame::*target = 0;
			
			if(targetMember == "LOC") {
				target = &Frame::mPosition;
			}else if(targetMember == "ROT") {
				target = &Frame::mRotations;
			}
			
            if(target)
                dst_val = &(f->*target.*indexToMember[arrayIndex]);
		}
		if(dst_val)
			*dst_val = blendFunc(*dst_val, getValue(time, loop)  * factor );
	}
	
	rgeScalar getValue(int time, bool loop) const { return curve.getValue(time, loop); }
	const BezierCurve& getCurve() const { return curve; }
	
	BezierCurve curve;
	string targetObject;
	string targetMember;
	int arrayIndex;
	
private:

};

typedef map<string, ValueDriver> StrToValueDriver;

class Action : public IdentifiedObj
{
public:
	typedef enum {
		ADD,
		REPLACE
	} BLEND_FUNC;
	
	Action() : mbLooping()//: //blendingAction( (Action*)(0) ), blendingFactor()
	 {}
	
	void setRange(int start, int end) {
		range = rgeVector2i(start, end);
	}
	
	void setBezierCurve(const string& obj, const string& member, int array_index, const BezierCurve& c) {		
		if(member != "NONE") {
			char buf[1024];
			sprintf(buf, "%s%d", (obj + member).c_str(), array_index);
			drivers.insert(StrToValueDriver::value_type(string(buf), 
														ValueDriver(c, obj, member, array_index)));
		}
	}	

	bool hasValue(string data_path) {
		return drivers.find(data_path) != drivers.end() ; 
	}
	void setLooping(bool bloop) { mbLooping = bloop; }
/*	void setBlend(std::shared_ptr<Action> blendingAction_, float blendingFactor_) {
		blendingAction = blendingAction_;
		blendingFactor = blendingFactor_;
	}
*/		
	
	static rgeScalar addFunc(rgeScalar a, rgeScalar b)
	{
		return a + b;
	}

	static rgeScalar replaceFunc(rgeScalar a, rgeScalar b)
	{
		return b;
	}
	
	
	void apply(Frame* f, int time, rgeScalar factor = 1.f, BLEND_FUNC func = REPLACE)
	{
		if(mbLooping && range.x != range.y) {
			time = ((time-range.x) % (range.y - range.x)) + range.x;
		}
	
		for(StrToValueDriver::const_iterator i=drivers.begin(); i!=drivers.end(); ++i) {
			const ValueDriver& d = i->second;
			
			if(func == ADD)	
				d.apply(f, time, false, addFunc, factor);
			else {
				d.apply(f, time, false, replaceFunc, factor);
			}
		}
	}
	
	void print(std::ostream& os, const string& subject) const {
		
		for(StrToValueDriver::const_iterator i=drivers.begin(); i!=drivers.end(); ++i) {
			const ValueDriver& d = i->second;
			
			os << subject << "," << d.targetObject << "," << d.targetMember << "," << d.arrayIndex;

			for(int t=0; t< d.getCurve().size(); ++t) {
				os << "," << d.getValue(t, false) ;
			}
			os << std::endl;
		}
			
	}
private:
	
	StrToValueDriver drivers;	
	bool mbLooping;
	rgeVector2i range;
	
	

};

typedef std::shared_ptr<Action> ActionRef;

}
