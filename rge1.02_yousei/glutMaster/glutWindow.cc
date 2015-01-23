////////////////////////////////////////////////////////////////
//                                                            //
// glutMaster.h                                               //
// 
// Created by Yuhei Akamine on 2009/07/29.
// These codes are based on 
// George Stetten and Korin Crawford's code                   //
//                                                            //
////////////////////////////////////////////////////////////////

#include <GLUT/glut.h>

#include "glutWindow.h"
#include "glutMaster.h"

GlutWindow::GlutWindow()
{

}

GlutWindow::~GlutWindow(){
	glutDestroyWindow(windowID);
}

void GlutWindow::create(int setWidth, int setHeight,
			int setInitPositionX, int setInitPositionY,
			const char * title)
{
	onCreate();
}

void GlutWindow::onCreate(void) {
	//dummy
}

void GlutWindow::onDisplay(void){

                             //dummy function
}

void GlutWindow::onIdle(void){

                             //dummy function
}

void GlutWindow::onKeyPress(unsigned char key, int x, int y){

   key; x; y;                //dummy function
}

void GlutWindow::onMouseMotion(int x, int y){

   x; y;                     //dummy function
}

void GlutWindow::onMouseClick(int button, int state, int x, int y){

   button; state; x; y;      //dummy function
}

void GlutWindow::onPassiveMotion(int x, int y){

   x; y;                     //dummy function
}

void  GlutWindow::onReshape(int w, int h){

   w; h;                     //dummy function
}
   
void GlutWindow::onSpecialKeyPress(int key, int x, int y){

   key; x; y;
}   

void GlutWindow::onVisibilityChanged(int visible){

   visible;                  //dummy function
}

void GlutWindow::SetWindowID(int newWindowID){

   windowID = newWindowID;
}

int GlutWindow::GetWindowID(void){

   return( windowID );
}





