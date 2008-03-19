/*
 *  ARenderer.h - class mapping rendering API to OpenGL
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

// coords: world -> screen -> window -> container

/* Notes:
  - layers could be implemented using display lists
 
 //	creating a display list
 GLuint listname = glGenLists( 1 );
 glNewList( listname, GL_COMPILE or GL_COMPILE_AND_EXECUTE );
 // glBegin/glEnd ...  
 glEndList();
 
 //	drawing with a display list
 glCallList( listname );
 
 //	disposing of a display list
 glDeleteLists( listname, 1 );
 
 http://developer.apple.com/graphicsimaging/opengl/optimizingdata.html */

#include "AObject.h"
#include <OpenGL/gl.h>

class ARenderer : public AObject {
protected:
	ARect _frame; // in window coords
public:
	ARenderer(ARect frame) : _frame(frame) {}

	void setFrame(ARect frame) { _frame = frame; }
	ARect frame() { return _frame; }

	void begin() {
		glViewport(0.0f, 0.0f, _frame.width, _frame.height);
		glClearColor(0.0, 0.0, 0.0, 0);
		//glClearColor(1, 1, 0.7, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPushMatrix();
	}
	
	void end() {
		glPopMatrix();
		glFlush();
	}
	
	void color(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		glColor4f(r, g, b, a);
	}

	void color(AFloat col[4]) {
		glColor4fv(col);
	}
	
	void color(GLfloat r, GLfloat g, GLfloat b) {
		glColor3f(r, g, b);
	}
	
	void rectV(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
	}

	void rect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
		glBegin(GL_QUADS);
		rectV(x1, y1, x2, y2);
		glEnd();
	}

	void rect(ARect aRect) { rect(aRect.x, aRect.y, aRect.width, aRect.height); }

	void rectO(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
		glBegin(GL_LINE_LOOP);
		rectV(x1, y1, x2, y2);
		glEnd();
	}

	void rectO(ARect aRect) { rectO(aRect.x, aRect.y, aRect.width, aRect.height); }

	void point(AFloat x, AFloat y) {
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();
	}
	
	void point(APoint p) { point(p.x, p.y); }
	
	void points(APoint *p, int n) {
		int i = 0;
		glBegin(GL_POINTS);
		while (i < n) {
			glVertex2f(p[i].x, p[i].y);
			i++;
		}
		glEnd();
	}
	
	void points(AFloat *x, AFloat *y, int n) {
		int i = 0;
		glBegin(GL_POINTS);
		while (i < n) {
			glVertex2f(x[i], y[i]);
			i++;
		}
		glEnd();
	}

	void line(AFloat x1, AFloat y1, AFloat x2, AFloat y2) {
		glBegin(GL_LINE_STRIP);
		glVertex2f(x1, y1);
		glVertex2f(x2, y2);
		glEnd();
	}
	
	void polyV(AFloat *x, AFloat *y, int n) {
		int i = 0;
		while (i < n) {
			glVertex2f(x[i], y[i]);
			i++;
		}
	}
	
	void polygon(AFloat *x, AFloat *y, int n) {
		glBegin(GL_POLYGON);
		polyV(x, y, n);
		glEnd();
	}

	void polygonO(AFloat *x, AFloat *y, int n) {
		glBegin(GL_LINE_LOOP);
		polyV(x, y, n);
		glEnd();
	}
	
	void polyline(AFloat *x, AFloat *y, int n) {
		glBegin(GL_LINE_STRIP);
		polyV(x, y, n);
		glEnd();		
	}
	
	void fan(AFloat centerx, AFloat centery, AFloat *x, AFloat *y, int n) {
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(centerx, centery);
		polyV(x, y, n);
		glEnd();
	}

	void tri(AFloat *x, AFloat *y, int n) {
		glBegin(GL_TRIANGLES);
		polyV(x, y, n);
		glEnd();
	}
	
	void pointPV(APoint p) {
		glVertex2f(p.x, p.y);
	}
	
	void triP(APoint p1, APoint p2, APoint p3) {
		glBegin(GL_TRIANGLES);
		pointPV(p1);
		pointPV(p2);
		pointPV(p3);
		glEnd();
	}
	
	void trimesh(AFloat *x, AFloat *y, int n) {
		glBegin(GL_TRIANGLE_STRIP);
		polyV(x, y, n);
		glEnd();
	}

	void quadmesh(AFloat *x, AFloat *y, int n) {
		glBegin(GL_QUAD_STRIP);
		polyV(x, y, n);
		glEnd();
	}
};