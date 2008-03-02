/*
 *  ARenderer.h - class mapping rendering API to OpenGL
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

// coords: world -> screen -> window -> container

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

	void rectO(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
		glBegin(GL_LINE_LOOP);
		rectV(x1, y1, x2, y2);
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