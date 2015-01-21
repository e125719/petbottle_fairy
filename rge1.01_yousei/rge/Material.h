/*
 *  Material.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/02.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once  
#include <iostream>
#include <vector>
#include <memory>

#include "Types.h"
#include "Renderable.h"
#include "RgeObject.h"
namespace rge {
	
class Texture;

typedef int COLORNAME;

//! マテリアルを管理する
class Material : public RGEObject, public Issuable {
	friend class RGE;
//	typedef domProfile_COMMON::domTechnique::domPhong domPhong;
public:
	Material() : RGEObject(), mTexture(), mIsInvalid(false) {}
	Material(color4 diff, double shine=0) : RGEObject(), mTexture(), mIsInvalid(false) {
		create(diff, shine);
	}
	
	void create(color4 diff, double shine=0); //!< 拡散反射色をdiffとしてmaterialを生成
	void createFromRgm(std::istream& is) ;        //!< rgeファイルから読み込み
//	void createFromDomMaterial(domMaterial* material) ;
	
	virtual void issue() ;
	OBJ_TYPE objType() { return MATERIAL; }
	
	void setDiffuse(color4 c) { mDiffuse = c; } //!< 拡散反射色をcに変更する
	void setEmission(color4 e){ mEmission = e; }
	void setAmbient(color4 a) { mAmbient = a; }
	void setSpecular(color4 s){ mSpecular = s; }
	void setShininess(rgeScalar s) { mShininess = s;}
	void setReflective(color4 c) { mReflective = c; }
	void setReflectivity(rgeScalar s) { mReflectivity = s; }
	void setTransparent(color4 c) { mTransparent = c; }
	void setTransparency(rgeScalar s) { mTransparency = s; }
	
	void setTexture(std::shared_ptr<Texture> t) { mTexture = t; }
	
	color4 diffuse() { return mDiffuse; }
	color4 emission() { return mEmission; }
	color4 ambient() { return mAmbient; }
	color4 specular() { return mSpecular; }
	rgeScalar shininess() { return mShininess; }
	rgeScalar transparency() { return mTransparency; }
	
	bool useBlending() { return mDiffuse.a != 1.0; } //!< アルファブレンンドが利用されているか
	
	bool isInvalid() { return mIsInvalid; }
	void invalidate(bool b = true) { mIsInvalid = b; }
	
	static std::shared_ptr<Material> getTemplate(COLORNAME cn);
	static const COLORNAME COLORNAME_RED   ;
	static const COLORNAME COLORNAME_GREEN ;
	static const COLORNAME COLORNAME_BLUE  ;
	static const COLORNAME COLORNAME_WHITE ;
	static const COLORNAME COLORNAME_BLACK ;	
	
	static void setDefault(std::shared_ptr<Material> m) { sDefault = m; }
	static std::shared_ptr<Material> defaultMaterial() { return sDefault; } 
private:
	static void createTemplates();
//	void fromPhong(domPhong* phong) ;
	
private:
	std::shared_ptr<Texture> mTexture;
	
	color4 mEmission;
	color4 mAmbient;
	color4 mDiffuse;
	color4 mSpecular;
	rgeScalar mShininess;
	color4 mReflective;
	rgeScalar mReflectivity;
	color4 mTransparent;
	rgeScalar mTransparency;	
	
	bool mIsInvalid;
	
	static vector<std::shared_ptr<Material> > sTemplates;
	static std::shared_ptr<Material> sDefault;
};

typedef std::shared_ptr<Material> MaterialRef;
inline MaterialRef newMaterial() { return MaterialRef(new Material); }

}