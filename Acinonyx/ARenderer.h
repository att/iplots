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

#ifndef A_RENDERER_H_
#define A_RENDERER_H_

#include "AObject.h"
#include <OpenGL/gl.h>
#include "AWindow.h"

class ARenderer : public AObject {
protected:
	ARect _frame; // in window coords
	AWindow *_window;
public:

	ARenderer(AWindow *aWindow, ARect frame) : _window(aWindow), _frame(frame) { OCLASS(ARenderer) }

	void setFrame(ARect frame) { _frame = frame; }
	ARect frame() { return _frame; }

	AWindow *window() { return _window; }
	virtual void setWindow(AWindow *win) { _window = win;  }
	
	bool containsPoint(APoint pt) { return (!((pt.x < _frame.x) || (pt.y < _frame.y) || (pt.x > _frame.x + _frame.width) || (pt.y > _frame.y + _frame.height))); }
	bool intersectsRect(ARect r) { return (!((r.x > _frame.x + _frame.width) || (r.x + r.width < _frame.x) || (r.y > _frame.y + _frame.height) || (r.y + r.height < _frame.y))); }
	
	void redraw() { if (_window) _window->redraw(); }
	
	// drawing methods
	
	void begin() {
		profStart()
		glViewport(0.0f, 0.0f, _frame.width, _frame.height);
		//glClearColor(0.0, 0.0, 0.0, 0);
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPushMatrix();
		// change coordinates to graphics coords
		glTranslatef(-1, -1, 0);
		glScalef( 2.0 / _frame.width, 2.0 / _frame.height, 1);
		// anti-aliasing trick
		glTranslatef(0.5, 0.5, 0.0); // subpixel shift to make sure that lines don't wash over two integration regions. However, filled areas are probably still in trouble
		_prof(profReport("$renderer.begin"))
	}
	
	void end() {
		_prof(profReport("^renderer.end"))
		glPopMatrix();
		glFlush();
		_prof(profReport("$renderer.end"))
	}
	
	void color(AColor c) {
		glColor4f(c.r, c.g, c.b, c.a);
	}

	void color(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		glColor4f(r, g, b, a);
	}

	void color(const AFloat col[4]) {
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

	void rect(ARect aRect) { rect(aRect.x, aRect.y, aRect.x + aRect.width, aRect.y + aRect.height); }

	void rectO(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
		glBegin(GL_LINE_LOOP);
		rectV(x1, y1, x2, y2);
		glEnd();
	}

	void rectO(ARect aRect) { rectO(aRect.x, aRect.y, aRect.x + aRect.width, aRect.y + aRect.height); }

	void point(AFloat x, AFloat y) {
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();
	}
	
	void point(APoint p) { point(p.x, p.y); }
	
	void points(const APoint *p, int n) {
		int i = 0;
		glBegin(GL_POINTS);
		while (i < n) {
			glVertex2f(p[i].x, p[i].y);
			i++;
		}
		glEnd();
	}
	
	void text(const APoint pt, const char *txt) {
		if (_window) _window->glstring(pt, AMkPoint(0,0), txt);
	}

	void text(const APoint pt, const APoint adj, const char *txt) {
		if (_window) _window->glstring(pt, adj, txt);
	}
	
	void text(AFloat x, AFloat y, const char *txt) {
		if (_window) _window->glstring(AMkPoint(x,y), AMkPoint(0,0), txt);
	}
	
	void clip(ARect where) {
		glScissor(where.x, where.y, where.width, where.height);
		ALog("clip to (%g,%g - %g,%g)", where.x, where.y, where.width, where.height);
		glEnable(GL_SCISSOR_TEST);
	}
	
	void clipOff() {
		ALog("disable clipping");
		glDisable(GL_SCISSOR_TEST);
	}
	
	void points(const AFloat *x, const AFloat *y, int n) {
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
	
	void lineFrom(AFloat x, AFloat y) {
		glBegin(GL_LINE_STRIP);
		glVertex2f(x, y);
	}
	
	void lineBegin() {
		glBegin(GL_LINE_STRIP);
	}
		
	void lineTo(AFloat x, AFloat y) {
		glVertex2f(x, y);
	}
	
	void lineEnd() {
		glEnd();
	}
	
	void polyV(const AFloat *x, const AFloat *y, int n) {
		int i = 0;
		while (i < n) {
			glVertex2f(x[i], y[i]);
			i++;
		}
	}
	
	void polyP(const APoint *pt, int n) {
		int i = 0;
		while (i < n) {
			glVertex2f(pt[i].x, pt[i].y);
			i++;
		}
	}

	void polygon(const APoint *pt, int n) {
		glBegin(GL_POLYGON);
		polyP(pt, n);
		glEnd();
	}
	
	void polygon(const AFloat *x, const AFloat *y, int n) {
		glBegin(GL_POLYGON);
		polyV(x, y, n);
		glEnd();
	}

	void polygonO(const APoint *pt, int n) {
		glBegin(GL_LINE_LOOP);
		polyP(pt, n);
		glEnd();
	}
	
	void polygonO(const AFloat *x, const AFloat *y, int n) {
		glBegin(GL_LINE_LOOP);
		polyV(x, y, n);
		glEnd();
	}

	void polyline(const APoint *pt, int n) {
		glBegin(GL_LINE_STRIP);
		polyP(pt, n);
		glEnd();		
	}
	
	void polyline(const AFloat *x, const AFloat *y, int n) {
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

	void tri(const AFloat *x, const AFloat *y, int n) {
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
	
	void trimesh(const AFloat *x, const AFloat *y, int n) {
		glBegin(GL_TRIANGLE_STRIP);
		polyV(x, y, n);
		glEnd();
	}

	void quadmesh(const AFloat *x, const AFloat *y, int n) {
		glBegin(GL_QUAD_STRIP);
		polyV(x, y, n);
		glEnd();
	}
};

#endif
