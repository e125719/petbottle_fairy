/*
 *  RgeReader.h
 *  rgeSample
 *
 *  Created by 赤嶺 有平 on 09/11/27.
 *  Copyright 2009 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include <boost/algorithm/string.hpp>

#include "RGE.h"

namespace rge {
	
	using std::string;
	using std::stringstream;
	
	typedef vector<string> strings;
	
	inline void operator >> (const YAML::Node& node, rgeVector4& v)
	{
		node[0] >> v.x;
		node[1] >> v.y;
		node[2] >> v.z;
		node[3] >> v.w;
	}
	
	inline void operator >> (const YAML::Node& node, rgeQuat& v)
	{
		node[0] >> v.w;
		node[1] >> v.x;
		node[2] >> v.y;
		node[3] >> v.z;
	}
	
	inline void operator >> (const YAML::Node& node, rgeVector3& v)
	{
		node[0] >> v.x;
		node[1] >> v.y;
		node[2] >> v.z;
	}
	
	inline void operator >> (const YAML::Node& node, rgeVector2& v)
	{
		node[0] >> v.x;
		node[1] >> v.y;
	}
	
	inline void operator >> (const YAML::Node& node, color4& v)
	{
		node[0] >> v.r;
		node[1] >> v.g;
		node[2] >> v.b;
		node[3] >> v.a;
	}
	
	inline void operator >> (const YAML::Node& node, color3& v)
	{
		node[0] >> v.r;
		node[1] >> v.g;
		node[2] >> v.b;
	}
	
	inline void operator >> (const YAML::Node& node, rgeMatrix4x4& m)
	{
		rgeVector4 row1,row2,row3,row4;
		
		node[0] >> row1;
		node[1] >> row2;
		node[2] >> row3;
		node[3] >> row4;
		
		rgeScalar a[16] = {
			row1.x, row1.y, row1.z, row1.w,
			row2.x, row2.y, row2.z, row2.w,
			row3.x, row3.y, row3.z, row3.w,
			row4.x, row4.y, row4.z, row4.w}; 
		
		m.fromArray(a);
		//	m.transpose();
	}
	/*
	 inline istream& operator >> (istream& is, rgeVector3& v)
	 {
	 return is >> v.x >> v.y >> v.z;
	 }
	 
	 inline istream& operator >> (istream& is, rgeVector2& v)
	 {
	 return is >> v.x >> v.y;
	 }
	 
	 inline istream& operator >> (istream& is, color3& c)
	 {
	 return is >> c.r >> c.g >> c.b;
	 }
	 */
	
	class RgeReader
	{
		static string get_path(const char* file_path)
		{
			string path;
			
			const char* p = file_path+strlen(file_path)-1;

#ifdef __WIN32__
			while(*p != '\\' && p > file_path)
#else
			while(*p != '/' && p > file_path)
#endif
				--p;

			while(file_path != p) {
				path += *file_path++;
			}
			return path;
		}
	public:
		RgeReader()   {
            mRge = RGE::getInstance();
        }
		
		
		//	std::ostream& logger() { return mRge->logger(); }
		bool read(const char* file_path) {
			std::ifstream is(file_path);
			
			if(!is) {
				std::cout << file_path << "is not found." << std::endl;
				return false;
			}
			
//			std::cout << get_path(file_path) << endl;
			mRge->setBaseDir(get_path(file_path));
			return read(is);
		}
		
		bool read(istream& fin) {
			//	try{
			YAML::Parser parser(fin);  
			YAML::Node doc;
			parser.GetNextDocument(doc);
			
			const YAML::Node* texs = doc.FindValue("Textures");
			for(size_t i=0; texs && i<texs->size(); ++i)
				readTexture((*texs)[i]);
			
			const YAML::Node* mats = doc.FindValue("Materials");
			for(size_t i=0; mats && i<mats->size(); ++i)
				readMaterial((*mats)[i]);
			
			const YAML::Node* meshes = doc.FindValue("Meshes");
			for(size_t i=0; meshes && i<meshes->size(); ++i)
				readMesh((*meshes)[i]);
			
			const YAML::Node* cams = doc.FindValue("Cameras");
			for(size_t i=0; cams && i<cams->size(); ++i)
				readCamera((*cams)[i]);
			
			const YAML::Node* lits = doc.FindValue("Lights");
			for(size_t i=0; lits && i<lits->size(); ++i)
				readLight((*lits)[i]);
			
			const YAML::Node* acts = doc.FindValue("Actions");
			for(size_t i=0; acts && i<acts->size(); ++i)
				readActions((*acts)[i]);
			
			
			const YAML::Node* objs = doc.FindValue("Objects");
			for(size_t i=0; objs && i<objs->size(); ++i)
				readObject((*objs)[i], mRge->rootFrame());
			//	}catch (YAML::ParserException& e ) {
			//		cout << e.what() << endl;
			//	}
			
			mRge->registerChildFrames(mRge->rootFrame());
			mRge->rootFrame()->resolveParentBone();
			
			int fps = 30;
			if(doc.FindValue("Fps"))
				doc["Fps"] >> fps;
			
			mRge->setFps(fps);
			
			return true;
		}
		
	private:
		void readTexture(const YAML::Node& node) {
			string name;
			string typestr;
			string file;
			
			node["Name"] >> name;
			node["Type"] >> typestr;
			
			boost::algorithm::to_upper(typestr);
			
			if(typestr == "IMAGE") {
				node["File"] >> file;
				
				TextureRef t = TextureRef(new ImageTexture);
				
				t->createFromFile(mRge->baseDir() + file);
				t->setId(name);
				
				mRge->registerTexture(t);
			}	
			mRge->logger() << "Texture: \"" << name<< "\"\n";
		}
		
		void readMaterial(const YAML::Node& node) {
			string name, texname;
			color3 diff, spec;
			rgeScalar specDeg,alpha, emit;
			
			node["Name"] >> name;
			node["Diffuse"] >> diff;
			node["Specular"] >> spec;
			node["SpecDeg"] >> specDeg;
			node["Alpha"] >> alpha;
			node["Emit"] >> emit;
			
			const YAML::Node* tex = node.FindValue("Texture");
			
			MaterialRef m = MaterialRef(new Material);
			
			m->setId(name);
			m->setAmbient(diff/4);
			m->setDiffuse(diff);
			m->setSpecular(spec);
			m->setShininess(specDeg);
			m->setTransparency(alpha);
			m->setTransparent(diff);
			m->setEmission(diff*emit);
			
			if(tex) {
				*tex >> texname;
				
				m->setTexture(mRge->findTexture(texname));
			}
			
			mRge->registerMaterial(m);
			
			mRge->logger() << "Matrial: " << name<< "\n";
		}
		
		void readVertex(const YAML::Node& n,  TriMeshObject::SOURCE_TYPE source, int& ipos, rgeVector3& normal, color3& color, rgeVector2& uv)
		{
			n["v"] >> ipos;
			
			if(source & TriMeshObject::TYPE_NORMAL)
				n["n"] >> normal;
			
			if(source & TriMeshObject::TYPE_COLOR)
				n["c"] >> color;
			
			if(source & TriMeshObject::TYPE_TEXCOORD)
				n["u"] >> uv;		
		}
		
		void readVertex(stringstream& s, TriMeshObject::SOURCE_TYPE source, int& ipos, rgeVector3& normal, color3& color, rgeVector2& uv)
		{
			s >> ipos;
			
			if(source & TriMeshObject::TYPE_NORMAL)
				s >> normal;
			
			if(source & TriMeshObject::TYPE_COLOR)
				s >> color;
			
			if(source & TriMeshObject::TYPE_TEXCOORD)
				s >> uv;
		}
		
		void readMesh(const YAML::Node& node) {
			typedef std::map<string, int> str2int;
			
			string name, mat, items;
			rgeVector3 v, n;
			
			TriMeshObjectRef m = newTriMeshObject();
			
			node["Name"] >> name; 
			m->setId(name);
			
			const YAML::Node& verts = node["Positions"];
			
			if(verts.size() == 0) { // is this short format?
				verts >> items;
				
				stringstream s(items);
				
				while(true) {
					s >> v.x >> v.y >> v.z;
					if(!s)
						break;
					
					m->addPosition(v);
				}
				
			}else {
				for(size_t i=0; i<verts.size(); ++i) {
					verts[i] >> v;
					m->addPosition(v);
				}
			}
			
			str2int mat2imesh;
			const YAML::Node& faces = node["Faces"];
			
			//	faces
			if(faces.size() == 0) { // is this short form?
				faces >> items;
				
				stringstream s(items);
				string types;
				
				s >> types;
				
				TriMeshObject::SOURCE_TYPE source;
				
				source = (TriMeshObject::TYPE_POSITION)*( types.find("v") != string::npos)
				|(TriMeshObject::TYPE_NORMAL  )*( types.find("n") != string::npos)
				|(TriMeshObject::TYPE_COLOR   )*( types.find("c") != string::npos)
				|(TriMeshObject::TYPE_TEXCOORD)*( types.find("u") != string::npos);
				m->setSourceType(source);
				
				while(true) {
					string mat;
					int nverts;
					int i1,i2,i3,i4;
					rgeVector3 n1,n2,n3,n4;
					color3 c1,c2,c3,c4;
					rgeVector2 u1,u2,u3,u4;
					
					s >> mat;
					
					if(!s)
						break;
					
					str2int::iterator imi;
					int imesh;
					if((imi = mat2imesh.find(mat)) == mat2imesh.end()) {
						imesh = m->makeTriangleMesh(mat);
						mat2imesh.insert(str2int::value_type(mat, imesh));
					}else{
						imesh = imi->second; 
					}				
					
					s >> nverts;
					
					if(nverts == 3) {
						
						readVertex(s, source, i1, n1, c1, u1);
						readVertex(s, source, i2, n2, c2, u2);
						readVertex(s, source, i3, n3, c3, u3);
						
						m->addTriangle(imesh,
									   i1,n1,c1,u1,
									   i2,n2,c2,u2,
									   i3,n3,c3,u3);
					}else{
						
						readVertex(s, source, i1, n1, c1, u1);
						readVertex(s, source, i2, n2, c2, u2);
						readVertex(s, source, i3, n3, c3, u3);
						readVertex(s, source, i4, n4, c4, u4);
						
						m->addQuad(imesh,
								   i1,n1,c1,u1,
								   i2,n2,c2,u2,
								   i3,n3,c3,u3,
								   i4,n4,c4,u4);
					}				
				}
			}else{
				
				TriMeshObject::SOURCE_TYPE source = TriMeshObject::TYPE_POSITION | TriMeshObject::TYPE_NORMAL;
				if(faces.size() > 0) {
					source |= faces[0]["V"][0].FindValue("c") ? TriMeshObject::TYPE_COLOR : 0;
					source |= faces[0]["V"][0].FindValue("u") ? TriMeshObject::TYPE_TEXCOORD : 0;
				}
				m->setSourceType(source);
				
				for(size_t i=0; i<faces.size(); ++i) {
					const YAML::Node& face = faces[i];
					
					face["Mat"] >> mat;
					const YAML::Node* nor = face.FindValue("Normal");
					if(nor) {
						(*nor) >> n;
					}
					
					str2int::iterator imi;
					int imesh;
					if((imi = mat2imesh.find(mat)) == mat2imesh.end()) {
						imesh = m->makeTriangleMesh(mat);
						mat2imesh.insert(str2int::value_type(mat, imesh));
					}else{
						imesh = imi->second; 
					}
					
					const YAML::Node& vertices = face["V"];
					
					int i1,i2,i3,i4;
					rgeVector3 n1,n2,n3,n4;
					color3 c1,c2,c3,c4;
					rgeVector2 u1,u2,u3,u4;
					
					source &= TriMeshObject::TYPE_ALL - TriMeshObject::TYPE_NORMAL;
					source |= nor ? 0 : TriMeshObject::TYPE_NORMAL;
					
					if(nor) {
						n1=n2=n3=n4= n;
					}
					if(vertices.size() == 3) {
						
						readVertex(vertices[0], source, i1, n1, c1, u1);
						readVertex(vertices[1], source, i2, n2, c2, u2);
						readVertex(vertices[2], source, i3, n3, c3, u3);
						
						m->addTriangle(imesh,
									   i1,n1,c1,u1,
									   i2,n2,c2,u2,
									   i3,n3,c3,u3);
					}else{
						
						readVertex(vertices[0], source, i1, n1, c1, u1);
						readVertex(vertices[1], source, i2, n2, c2, u2);
						readVertex(vertices[2], source, i3, n3, c3, u3);
						readVertex(vertices[3], source, i4, n4, c4, u4);
						
						m->addQuad(imesh,
								   i1,n1,c1,u1,
								   i2,n2,c2,u2,
								   i3,n3,c3,u3,
								   i4,n4,c4,u4);
					}
				}
			}
			m->resolveMaterials();
			mRge->registerMesh(m);
			
			mRge->logger() << "Mesh: " << name << "\n";
		}
		
		void readCamera(const YAML::Node& node) 
		{
			string name;
			rgeVector2 clip;
			rgeScalar lens;
			
			node["Name"] >> name;
			node["Clip"] >> clip;
			node["Lens"] >> lens;
			
			CameraRef cam = CameraRef(new Camera());
			
			cam->setId(name);
			cam->setClip(clip.x, clip.y);
			cam->setFov(degree(atan2(35.f/2.f, lens))*2);
			
			mRge->registerCamera(cam);
			
			mRge->logger() << "Camera: " << name << "\n";
		}
		
		void readLight(const YAML::Node& node) 
		{
			string name, type;
			color3 col;
			rgeScalar e, cutoff, exp;
			rgeScalar dist =0; //optional values
			
			node["Name"] >> name;
			node["Type"] >> type;
			node["Color"] >> col;
			node["Energy"] >> e;
			node["SpotSize"] >> cutoff;
			node["SpotSoft"] >> exp;
			
			if(node.FindValue("Distance"))
				node["Distance"] >> dist;
			
			LightRef light;
			if(type == "Point") {
				light = LightRef(new PointLight( col, e));
			}else if(type == "Directional") {
				light = LightRef(new DirectionalLight( col, e));
			}else if(type == "Spot") {
				light = LightRef(new SpotLight( col, e, cutoff, exp));
			}else {
				light = LightRef(new PointLight( col, e));
				mRge->logger() << "RGE: unknown light type: " << type << ":assume Point light" << "\n";
			}
			light->setId(name);
			light->setRange(dist);
			
			mRge->registerLight(light);
			
			mRge->logger() << "Light: " << name << "\n";
		}
		
		void readActions(const YAML::Node& node)
		{			
			ActionRef new_action = ActionRef(new Action);

			string name;
			node["Name"] >> name;
			
			new_action->setId(name);
			
			const YAML::Node* methodn = node.FindValue("Method");
			
			BezierCurve::METHOD method = BezierCurve::CONTROL_POINTS;
			
			if(methodn) {
				string m;
				*methodn >> m;
				
				if(m == "SAMPLED_VALUE") {
					method = BezierCurve::SAMPLED_VALUE;
				}else if(m == "SAMPLED_MATRIX") {
					method = BezierCurve::SAMPLED_MATRICES;
				}else {
					method = BezierCurve::CONTROL_POINTS;
				}
			}
			
			const YAML::Node* rangen = node.FindValue("Range");
			
			int startf= INT_MAX, endf=0;
			if(rangen) {
				(*rangen)[0] >> startf;
				(*rangen)[1] >> endf;
			}
			
			const YAML::Node& curvesn = node["Curves"];
			
			if(curvesn.size() == 0) {
				mRge->logger() << "Action "<<name<< " has no curves. Skip this...\n";
				return;
			}
			
			for(size_t i=0; i<curvesn.size(); ++i) {
				const YAML::Node& curve = curvesn[i];
				
				BezierCurve ic(method);
				
				string items;
				curve >> items;
				stringstream ss(items);
				
				string target_obj, target_member;
				int array_index;
				rgeScalar start, end;
				
				ss >> target_obj >> target_member >> array_index;
				ss >> start >> end;
				
				new_action->setRange(start, end);
				
				rgeScalars sample;
				sample.reserve(end-start);
				
				if(method == BezierCurve::CONTROL_POINTS) {
					while(true) {
						BezierTriple pt;
						
						ss >> pt.c1;
						ss >> pt.p;
						ss >> pt.c2;
						
						if(!ss) 
							break;
						
						ic.addBezierTriple(pt);
					}
					ic.sampleAll(start);
				}else if(method == BezierCurve::SAMPLED_VALUE) {
					for(int j=start; j<end; ++j) {
						rgeScalar v;
						
						ss >> v;
						sample.push_back(v);
					}							
					ic.createFromSampledValue(start, sample);
				}
				if(target_member == "ROT") {
					ic.scale(180.0 / 3.1415926);
				}
				
				new_action->setBezierCurve(target_obj, target_member, array_index, ic);	
				
			}
			mRge->registerAction(new_action);
			
			mRge->logger() << "Action: " << name << "\n";
		}
		
		void readRawTransform(const YAML::Node& rowtransn, Frame* f)
		{
			int start, end;
			
			rowtransn["Range"][0] >> start;
			rowtransn["Range"][1] >> end;
			
			const YAML::Node& bonesn = rowtransn["Bones"];
			
			strings names;
			for(YAML::Iterator i=bonesn.begin(); i!=bonesn.end(); ++i) {
				string bone_name;
				*i >> bone_name;
				
				names.push_back(bone_name);
			}
			
			string trans_stream;
			rowtransn["Trans"] >> trans_stream;
			stringstream ss(trans_stream);
			
			vector<BezierCurves> bcs(names.size());
		
			for(size_t i=0; i<names.size(); ++i) {
				bcs[i].resize(3+4);
				for(int j=0; j<3+4 /* rgeVector3 + rgeQuat */ ; ++j) {
					bcs[i][j].createEmptySampledValue(start, end);
				}
			}
			
			for(int time=start; time<end; ++time) {
				rgeVector3 t;
				rgeQuat q;
				
				for(size_t j=0; j<names.size(); ++j) {
					ss >> t;
					ss >> q;
					
					bcs[j][0].setSampledValue(time, t.x);
					bcs[j][1].setSampledValue(time, t.y);
					bcs[j][2].setSampledValue(time, t.z);
					
					bcs[j][3].setSampledValue(time, q.w);
					bcs[j][4].setSampledValue(time, q.x);
					bcs[j][5].setSampledValue(time, q.y);
					bcs[j][6].setSampledValue(time, q.z);
				}
			}
			
			ActionRef act(new Action);
			
			for(size_t i=0; i<names.size(); ++i) {
				act->setBezierCurve(string("pose/") + names[i], "LOC", 0, bcs[i][0]);	
				act->setBezierCurve(string("pose/") + names[i], "LOC", 1, bcs[i][1]);	
				act->setBezierCurve(string("pose/") + names[i], "LOC", 2, bcs[i][2]);	

				act->setBezierCurve(string("pose/") + names[i], "ROTQ", 0, bcs[i][3]);	
				act->setBezierCurve(string("pose/") + names[i], "ROTQ", 1, bcs[i][4]);	
				act->setBezierCurve(string("pose/") + names[i], "ROTQ", 2, bcs[i][5]);	
				act->setBezierCurve(string("pose/") + names[i], "ROTQ", 3, bcs[i][6]);	
			}
			
			f->anim()->setAction(act);
			
			act->setRange(start, end);
			//act->print(cout , "test");
			
			f->flattenBoneTree(f);
		}
		
		void readBones(const YAML::Node& bonesn, Frame* base, Bone* parent, int depth)
		{				
			for(size_t i=0; i<bonesn.size(); ++i) {
				const YAML::Node& bonen = bonesn[i];
				
				string name;
				rgeMatrix4x4 matrix;
				rgeVector3 rot;
				rgeQuat quat;
				rgeScalar len;
				
				bonen["Name"] >> name;
				bonen["Length"] >> len;				
				bonen["Matrix"] >> matrix;

				BoneRef b;
				if(parent)
					b = parent->createChildBone();
				else
					b = base->createBone();
				
				b->setId(name);
				b->setMatrix(matrix);
				b->setLength(len);
				
				if(bonen.FindValue("Quat")) {
					bonen["Quat"] >> quat;
					b->setQuatanion(quat);
                    b->useEulerRotation(false);
					b->useQuatanion(true);
				}else if(bonen.FindValue("Rot")) {
					bonen["Rot"] >> rot;
					b->setRotation(rot);
                    b->useEulerRotation(true);
					b->useQuatanion(false);
				}
				
				
				for(int j=0; j<depth; ++j)
					mRge->logger() << "  ";
				mRge->logger() << "Bone: " << name << "\n";
				
				readBones(bonen["Bones"], base, b.get(), depth + 1);
			}
			
		}
		
		void readObject(const YAML::Node& node, FrameRef parent, int depth = 0)
		{
			string name,type,data;
			rgeMatrix4x4 matrix;
			rgeVector3 pos, rot, sca;
			u_int layerBit;
			
			node["Name"] >> name;
			node["Matrix"] >> matrix;
			node["Pos"] >> pos;
			node["Rot"] >> rot;
			node["Sca"] >> sca;
			node["Type"] >> type;
			if(node.FindValue("DataName"))
				node["DataName"] >> data;
			node["Layers"] >> layerBit;
			const YAML::Node* physics  = node.FindValue("Physics");
			const YAML::Node* children = node.FindValue("Children");
			const YAML::Node* action   = node.FindValue("Action");
			const YAML::Node* rowtrans = node.FindValue("RawTrans");
			const YAML::Node* pose     = node.FindValue("Pose");
			const YAML::Node* parent_type = node.FindValue("ParentType");
			const YAML::Node* parent_bone = node.FindValue("ParentBone");
			
			FrameRef f = parent->createChild();
			
			f->setId(name);
			f->setMatrix(matrix);
			f->setTranslation(pos);
			f->setRotation(rot * (180/3.1415926535) );
			f->setScale(sca);
			f->setLayerBit(layerBit);
			
			if(type == "Mesh") {
				RenderableRef vis = mRge->findMesh(data);
				if(vis)
					f->addRenderable(vis);
				else
					mRge->logger() << "Frame: Can't resolve mesh: " << data;
			}else  if(type == "Camera") {
				CameraRef cam = mRge->findCamera(data);
				if(cam) {
					mRge->setCurrentCamera(cam);
					f->addCamera(cam);
				}else {
					mRge->logger() << "Frame: can't resolve camera: " << data << "\n";
				}
			}else if(type == "Lamp") {
				LightRef lit = mRge->findLight(data);
				if(lit) {
					f->addLight(lit);
				}else {
					mRge->logger() << "Frame: can't resolve light:" << data << "\n";
				}
			}else if(type == "Armature" || type == "Empty") {
				
			}else {
				mRge->logger() << "Frame: unknown type:" << type << " name:" << data << "\n";
			}
			
			if(physics && physics->size() > 0) {

				Frame::Physics ph;
				const YAML::Node* ptype= physics->FindValue("Type");
				const YAML::Node* bounds = physics->FindValue("Bounds");
				const YAML::Node* mass   = physics->FindValue("Mass");

				ph.enabled = true;
				
				//NO_COLLISION’, ‘STATIC’, ‘DYNAMIC’, ‘RIGID_BODY’, ‘SOFT_BODY’, ‘OCCLUDE’, ‘SENSOR’]
				if(ptype) {
					*ptype >> type;
					if(type == "NO_COLLISION") {
						ph.enabled = false;
						ph.type = Frame::NO_COLLISION;
					}else if(type == "STATIC") {
						ph.type = Frame::STATIC;
					}else if(type == "DYNAMIC") {
						ph.type = Frame::DYNAMIC;
					}else if(type == "RIGID_BODY") {
						ph.type = Frame::RIGID_BODY;
					}else {
						mRge->logger() << "Frame: unknown physics type:" << type << " name:" << data << "\n";
					}
				}
				
				if(bounds) {
					*bounds >> name;
					if(name == "Sphere")
						ph.boundBox = Frame::SPHERE;
					else if(name == "Box")
						ph.boundBox = Frame::BOX;
					else if(name == "Cylinder")
						ph.boundBox = Frame::CYLINDER;
					else if(name == "Cone")
						ph.boundBox = Frame::CONE;
					else if(name == "Triangle_mesh")
						ph.boundBox = Frame::TRIMESH;
					else if(name == "convex")
						ph.boundBox = Frame::CONVEX;
					else
						mRge->logger() << "Frame:Unknown bounding box type\n";
				}
				if(mass) {
					*mass >> ph.mass;
				}
				f->setPhysics(ph);
			}
			
			if(action) {
				*action >> name;
				
				ActionRef a = mRge->findAction(name);
				
				if(a) {
					f->anim()->setAction(a);
				}else {
					mRge->logger() << "Frame: can't resolve action:" << name << "\n";
				}
			}
			

			
			if(pose) {
				readBones((*pose)["Bones"], f.get(), (Bone*)0, depth+1);
			}

			if(rowtrans) {
				readRawTransform(*rowtrans, f.get());
			}
			
									
			if(parent_type ) {
				*parent_type >> type;
				
				if(type == "BONE" && parent_bone) {
					*parent_bone >> name;
					
					f->setParentType(Frame::BONE_NAME);
					f->setParentBoneName(name);
				}
			}
			
			for(int i=0; i<depth; ++i)
				mRge->logger() << "  ";
			mRge->logger() << "Object: " << f->getId() << "\n";
			
			if(children) {
				for(size_t i=0; i<children->size(); ++i) {
					readObject((*children)[i], f, depth + 1);
				}		
			}
		}
		
		RGE* mRge;
	};
	
}