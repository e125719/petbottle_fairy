#pragma once

extern void trackballInit(void);
extern void trackballRegion(int w, int h);
extern void trackballStart(int x, int y);
extern void trackballMotion(int x, int y);
extern void trackballStop(int x, int y);
extern void trackballRotation(const char*);

#include "rge/Types.h"
extern rge::rgeQuat trackballQuat();
