/*
 *  Utils.h
 *  VrGame
 *
 *  Created by Yuhei Akamine on 08/07/17.
 *  Copyright 2008 Yuhei Akamine. All rights reserved.
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include "Types.h"
#include <OpenGL/gl.h>

namespace rge {
	
template <class IS>
void unread(IS& is, const char* s) {
	for(size_t i=0; i<strlen(s); ++i) 
		is.unget();
}
 
inline void rgeGlVertex(const rgeVector3& v) {
#ifdef RGE_USE_DOUBLE
	glVertex3d(v.x, v.y, v.z);
#else
	glVertex3f(v.x, v.y, v.z);
#endif
}

inline void rgeGlColor(const color3& c) {
	glColor4f(c.r, c.g, c.b, 1);
}

inline void rgeGlColor(const color4& c) {
	glColor4f(c.r, c.g, c.b, c.a);
}

inline int dprintf(const char* format, ...)
{
#ifdef __RGE_VERBOSE
	va_list arg;
	va_start(arg, format);
	int r = vprintf(format, arg);
	va_end(arg);
	return r;
#else
	return 0;
#endif
}

}
#endif

	

