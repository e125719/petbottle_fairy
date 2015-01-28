
#ifndef __GLUT_WINDOW_H__
#define __GLUT_WINDOW_H__

class GlutMaster;

class GlutWindow{
	friend class GlutMaster;
public:
	void create(int width, int height,
				int xposition, int yposition,
				const char * title);
	
	virtual void onCreate(void);
	virtual void onDisplay(void);
	virtual void onIdle(void);
	virtual void onKeyPress(unsigned char key, int x, int y);
	virtual void onMouseMotion(int x, int y);
	virtual void onMouseClick(int button, int state, int x, int y);
	virtual void onPassiveMotion(int x, int y);
	virtual void onReshape(int w, int h);   
	virtual void onSpecialKeyPress(int key, int x, int y);   
	virtual void onVisibilityChanged(int visible);
	
	void    redraw() { glutPostRedisplay(); }
	int     GetWindowID(void);	

protected:
	GlutWindow();
	virtual ~GlutWindow();

private:
	void    SetWindowID(int newWindowID);
	
	int windowID;
};

#endif







