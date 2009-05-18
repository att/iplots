/*
 *  GLUTWindow.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/18/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "AWindow.h"

#include <GLUT/glut.h>

class AGLUTWindow : public AWindow {
protected:
	int win_id;
public:
	AGLUTWindow(ARect frame);
	
	virtual void redraw() {
		glutSetWindow(win_id);
		glutPostRedisplay();
	}
	
	virtual void glstring(APoint pt, APoint adj, const char *txt) {
		int len, i;
		// FIXME: implement adj, color?
		glDisable(GL_LIGHTING);
		glRasterPos2f(pt.x, pt.y);
		len = (int) strlen(txt);
		for (i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, txt[i]);
		}
	}
	
};
