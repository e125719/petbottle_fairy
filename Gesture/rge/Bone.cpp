/*
 *  Bone.cpp
 *  rgeSample
 *
 *  Created by Yuhei Akamine on 10/11/19.
 *  Copyright 2010 Yuhei Akamine. All rights reserved.
 *
 */
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

#include "Bone.h"

namespace rge {
	void Bone::renderMe()
	{		
		if(!mVisible) 
			return;
			
		glPushMatrix();
		
		//glutWireSphere(mLength/10, 8, 8);
		
		glRotatef(-90, 1, 0, 0);
		//glutWireCone(mLength/10.f, mLength, 4, 1);
		glPopMatrix();
        
        cout << "glutWireSphere is not implemented!\n";
	}
}