
#include "rge/Types.h"
#include "rge/RGE.h"

#include "trackball.h"

using namespace rge;

static float2 lastPos;
static rgeQuat objRot;
static int2 region;

void trackballInit(void)
{
}

void trackballRegion(int w, int h)
{
    region = int2(w,h);
}

void trackballStart(int x, int y)
{
    lastPos = float2(x,y);
}

void trackballMotion(int x, int y)
{
    float2 pos(x, y);
    float2 dp = lastPos - pos;

    rgeQuat& q = objRot;
    
    dp = dp * (M_PI / 180.0);
    
    if (0) {
        q = q * rgeQuat::rotation(dp.y, rgeVector3(0, 0, 1));
    } else {
        q = q * rgeQuat::rotation(dp.y, rgeVector3(1, 0, 0));
    }
    q = q * rgeQuat::rotation(dp.x, rgeVector3(0, 1, 0));
    
    lastPos = pos;
    
}

void trackballStop(int x, int y)
{
}

void trackballRotation(const char* camera)
{
    FrameRef cam = RGE::getInstance()->findFrame(camera);
    cam->useQuatanion(true);
    cam->setQuatanion(objRot);
}

rgeQuat trackballQuat()
{
    return objRot;
}
