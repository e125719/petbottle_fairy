////////////////////////////////////////////////////////////////
//                                                            //
// glutMaster.h                                               //
// 
// Created by Yuhei Akamine on 2009/07/29.
// These codes are based on 
// George Stetten and Korin Crawford's code                   //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef __GLUT_MASTER_H__
#define __GLUT_MASTER_H__
#include <map>

#include "glutWindow.h"

typedef std::map<int, GlutWindow*> IDtoGlutWindowPtr;

class GlutMaster{
	
private:
	
	static void CallBackDisplayFunc(void);
	static void CallBackIdleFunc(void); 
	static void CallBackKeyboardFunc(unsigned char key, int x, int y);
	static void CallBackMotionFunc(int x, int y);
	static void CallBackMouseFunc(int button, int state, int x, int y);
	static void CallBackPassiveMotionFunc(int x, int y);
	static void CallBackReshapeFunc(int w, int h); 
	static void CallBackSpecialFunc(int key, int x, int y);   
	static void CallBackVisibilityFunc(int visible);
	
	static GlutMaster* mInstance;
	GlutMaster();
	GlutMaster(const GlutMaster& g) {}
	~GlutMaster();
	
	IDtoGlutWindowPtr mWindows;
public:
	static GlutMaster* getMaster();
	static void deleteMaster();
	
	GlutWindow* getWindow(int i) { return mWindows[i]; }
	
	template<class GLUTWINDOWCLASS>
	GlutWindow* createWindow(int setWidth, int setHeight,
							 int setInitPositionX, int setInitPositionY,
							 const char * title) {
		return createWindow(new GLUTWINDOWCLASS, setWidth, setHeight,
					 setInitPositionX, setInitPositionY,
					 title);
	}
	
	GlutWindow* createWindow(GlutWindow* w, int setWidth, int setHeight,
							 int setInitPositionX, int setInitPositionY,
							 const char * title);
	
	void  enterMainLoop(void);
	
};

#endif



