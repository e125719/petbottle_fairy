/*
 *  Material.cpp
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/26.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */
#include <iostream>
#include <string>
#include <memory>
using namespace std;

#include <OpenGL/gl.h>

#include "Utils.h"
#include "RGE.h"
#include "Material.h"
#include "Exceptions.h"
#include "Texture.h"

using namespace rge;



void Material::create(color4 diff, double shine) //!< 拡散反射色をdiffとしてmaterialを生成
{
	mDiffuse = diff;
	mAmbient = diff/4;
//	mSpecular= color4::white();
	mShininess = shine;
}

void Material::createFromRgm(std::istream& is)
{
	string item, name;
	color3 col;
	double v;
	
	is >> item >> name; setId(name);
	is >> item >> col;  mDiffuse = col;
	is >> item >> col;  mSpecular = col;
	is >> item >> v;    mShininess= v*128;
	is >> item >> v;    mTransparency = v;
	is >> item >> v;	mEmission = mDiffuse * v;
	
	mDiffuse.a = mTransparency;
	mSpecular.a = mTransparency;
	
	while(is.good()) {
		is >> item;
		
		if(item != "Texture:") {
			unread(is, item.c_str());
			break;
		}
		
		is >> name;
		
		if(name != "None") {
			if((mTexture = getRGE()->findTexture(name))) 
				cout << "Resolve texture:"<<name<<endl;
			else
				cout << "Can't resolve texture name:"<<name<<endl;
		}
	}
		
}

void Material::issue() {
	if(isInvalid() || !getRGE()->isMaterialEnable()) 
		return;
	
	GLfloat em[] = { mEmission.r, mEmission.g, mEmission.b, mEmission.a };
	GLfloat am[] = { mAmbient.r, mAmbient.g, mAmbient.b, mAmbient.a };
	GLfloat di[] = { mDiffuse.r, mDiffuse.g, mDiffuse.b, mDiffuse.a };
	GLfloat sp[] = { mSpecular.r, mSpecular.g, mSpecular.b, mSpecular.a };
	glMaterialfv(GL_FRONT, GL_EMISSION, em);
	glMaterialfv(GL_FRONT, GL_AMBIENT, am);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, di);
	glMaterialfv(GL_FRONT, GL_SPECULAR, sp);
	glMaterialf(GL_FRONT, GL_SHININESS, mShininess);
	
	if(mTexture) {
		GLfloat di[] = { 1.0, 1.0, 1.0, 1.0};
		glMaterialfv(GL_FRONT, GL_DIFFUSE, di);
		mTexture->issue();
	}else{
		glDisable(GL_TEXTURE_2D);
	}
}

/////class methods
vector<shared_ptr<Material> > Material::sTemplates;

const COLORNAME Material::COLORNAME_RED   = 0;
const COLORNAME Material::COLORNAME_GREEN = 1;
const COLORNAME Material::COLORNAME_BLUE  = 2;
const COLORNAME Material::COLORNAME_WHITE = 3;
const COLORNAME Material::COLORNAME_BLACK = 4;	

MaterialRef Material::sDefault;

void Material::createTemplates()
{
	sTemplates.push_back(MaterialRef( new Material( color4(1,0,0,1))));
	sTemplates.push_back(MaterialRef( new Material( color4(0,1,0,1))));
	sTemplates.push_back(MaterialRef( new Material( color4(0,0,1,1))));
	sTemplates.push_back(MaterialRef( new Material( color4(1,1,1,1))));
	sTemplates.push_back(MaterialRef( new Material( color4(0,0,0,1))));	

}

MaterialRef Material::getTemplate(COLORNAME cn)
{
	if(sTemplates.empty())
		dprintf("call RGE::init() first!\n");
	assert((size_t)cn < sTemplates.size());
	return sTemplates[cn];
}


//
// private methods
//
/*
void Material::fromPhong(domPhong* phong) {
	domColorToColor4(phong->getEmission(), mEmission);
	domColorToColor4(phong->getAmbient(), mAmbient);
	
	if(phong->getDiffuse()->getTexture()) {
		string name = phong->getDiffuse()->getTexture()->getTexture();
		TextureRef tex = getRGE()->findTexture(name);
		if(tex.get()) {
			cout << "resolve texture: " << name << endl;
		}else{
			cout << "Unresolvable texture: " << name << endl;
		}
		
		mTexture = tex;
	}
	domColorToColor4(phong->getDiffuse(), mDiffuse);
	domColorToColor4(phong->getSpecular(), mSpecular);
	domParamToDouble(phong->getShininess(), mShininess);
	domParamToDouble(phong->getTransparency(), mTransparency);
	
	mReflectivity = 0;
}*/
