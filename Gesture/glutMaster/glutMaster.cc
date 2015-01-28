#include <GLUT/glut.h>

#include "glutMaster.h"
#include "glutWindow.h"

GlutMaster* GlutMaster::mInstance = 0;

GlutMaster::GlutMaster(){

   // Create dummy variables 

   static const char * dummy_argv[1];
   dummy_argv[0] = "run";
   int dummy_argc = 1;

   // Initialize GLUT

   glutInit(&dummy_argc, const_cast<char**>(dummy_argv));
}

GlutMaster::~GlutMaster(){
	for(IDtoGlutWindowPtr::iterator i=mWindows.begin(); i!=mWindows.end(); ++i)
		delete i->second;
}

GlutMaster* GlutMaster::getMaster() {
	if(!mInstance)
		mInstance = new GlutMaster();
	
	return mInstance;
}

void GlutMaster::deleteMaster() {
	if(mInstance)
		delete mInstance;
	mInstance = 0;
}

GlutWindow* GlutMaster::createWindow(GlutWindow* win, int w, int h,
									 int xpos, int ypos,
									 const char * title)
{
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	int windowID = glutCreateWindow(title);
	win->SetWindowID(windowID);
	
	mWindows.insert(IDtoGlutWindowPtr::value_type( windowID , win)) ;

	
	// Hand address of universal static callback functions to Glut.
	// This must be for each new window, even though the address are constant.
	
	glutDisplayFunc(CallBackDisplayFunc);
	glutIdleFunc(CallBackIdleFunc); 
	glutKeyboardFunc(CallBackKeyboardFunc);
	glutSpecialFunc(CallBackSpecialFunc);
	glutMouseFunc(CallBackMouseFunc);
	glutMotionFunc(CallBackMotionFunc);
//	glutPassiveMotionFunc(CallBackPassiveMotionFunc);
	glutReshapeFunc(CallBackReshapeFunc); 
//	glutVisibilityFunc(CallBackVisibilityFunc);

	win->create(w, h, xpos, ypos, title);
	//win->onReshape(w, h);
	
	glutReshapeWindow(w, h);
	glutPositionWindow(xpos, ypos);	

	return win;
}
 
void GlutMaster::CallBackDisplayFunc(void){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onDisplay();
}

void GlutMaster::CallBackIdleFunc(void){

	GlutMaster* m = GlutMaster::getMaster();
	for(IDtoGlutWindowPtr::iterator i=m->mWindows.begin();
		i != m->mWindows.end();
		++i) {
		i->second->onIdle();
   }
}
 
void GlutMaster::CallBackKeyboardFunc(unsigned char key, int x, int y){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onKeyPress(key, x, y);
}

void GlutMaster::CallBackMotionFunc(int x, int y){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onMouseMotion(x, y);
}

void GlutMaster::CallBackMouseFunc(int button, int state, int x, int y){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onMouseClick(button, state, x, y);
}

void GlutMaster::CallBackPassiveMotionFunc(int x, int y){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onPassiveMotion(x, y);
}

void GlutMaster::CallBackReshapeFunc(int w, int h){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onReshape(w, h);
}

void GlutMaster::CallBackSpecialFunc(int key, int x, int y){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onSpecialKeyPress(key, x, y);
}   

void GlutMaster::CallBackVisibilityFunc(int visible){

   int windowID = glutGetWindow();
   GlutMaster::getMaster()->getWindow(windowID)->onVisibilityChanged(visible);
}
void GlutMaster::enterMainLoop(void){

   glutMainLoop();
}
 


