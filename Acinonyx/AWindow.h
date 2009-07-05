/*
 *  AWindow.h - abtract Acinonyx window class
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 *  lang: C++
 */

#ifndef A_WINDOW_H
#define A_WINDOW_H

#include "ATypes.h"
#include "AObject.h"

#ifdef DEBUG
#define GLC(X) { X; int ec = glGetError(); if (ec != GL_NO_ERROR) ALog("*** %s: GL error: %s\n", describe(), gluErrorString(ec)); }
#else
#define GLC(X) X
#endif

/* #define TEXTURE_RECTANGLE_ARB 0x84F5  (name "GL_ARB_texture_rectangle") -- it is equivalent to GL_TEXTURE_RECTANGLE_EXT (name "GL_EXT_texture_rectangle") on OS X */

#ifdef GL_TEXTURE_RECTANGLE_EXT
#define A_TEXTURE_TYPE GL_TEXTURE_RECTANGLE_EXT
#define A_EXACT_TEXTURE 1
#elif defined GL_TEXTURE_RECTANGLE_ARB
#define A_TEXTURE_TYPE GL_TEXTURE_RECTANGLE_ARB
#define A_EXACT_TEXTURE 1
#else
#define A_TEXTURE_TYPE GL_TEXTURE_2D
#define A_EXACT_TEXTURE 0
#endif

typedef GLuint texture_t;

static int first_gl = 1;

// this is static for now -- 0 = plot, 1 = objects, 2 = interaction (not saved as layer)
#define _max_layers LAYER_TRANS

// IDEA: use window-global query object
class AWindow : public AObject
{
protected:
	AObject *modalOwner;
	AObject *_rootVisual;
	ARect _frame;
	
	unsigned int _layers, _redraw_layer;
	texture_t _layer[_max_layers];
	bool _layers_initialized;
	AColor text_color;
	vsize_t dirtyFlagLayer; ///< if the dirty flag triggers a redraw this variable specifies the redraw layer to use
public:
	int *dirtyFlag;

	AWindow(ARect frame) : modalOwner(0), _rootVisual(0), _frame(frame), dirtyFlag(0) {
		_redraw_layer = _layers = 0;
		dirtyFlagLayer = LAYER_TRANS; /* by default dirty flag triggers the transient layer only - use setDirtyFlagLayer() to change this */
		text_color = AMkColor(0.0, 0.0, 0.0, 1.0);
		OCLASS(AWindow)
	};

	virtual ~AWindow() {
		if (_rootVisual) _rootVisual->release();
		DCLASS(AWindow)
	}

	// this function is called before the first OpenGL call is made but after at least one context has been created
	// it can detect extensions and setup the behavior ...
	void first_gl_call() {
		ALog("OpenGL version: %s", glGetString(GL_VERSION));
		ALog("OpenGL vendor: %s", glGetString(GL_VENDOR));
		ALog("OpenGL renderer: %s", glGetString(GL_RENDERER));
		ALog("OpenGL extensions: %s", glGetString(GL_EXTENSIONS));
		first_gl = 0;
	}
	
	bool freezeLayer(vsize_t layer) {
		if (layer >= _max_layers) return false;
		GLC(glBindTexture(A_TEXTURE_TYPE, _layer[layer]));
		ALog(" -> freeze layer %d (texture %d)", layer, _layer[layer]);
		int width = _frame.width, height = _frame.height;
/*
		int dbuf, rbuf;
		glGetIntegerv(GL_DRAW_BUFFER, &dbuf);
		glGetIntegerv(GL_READ_BUFFER, &rbuf);
		printf("draw buffer: %d, read buffer: %d (back=%d, front=%d)\n", dbuf, rbuf, GL_FRONT, GL_BACK);
 */
		
#if ! A_EXACT_TEXTURE
		int rw = width, rh = height;
		width = height = 32;
		while (width < rw) width <<= 2;
		while (height < rh) height <<= 2;
		ALog("%s: creating POTS texture %d x %d", describe(), width, height);
#endif

		GLC(glCopyTexImage2D(A_TEXTURE_TYPE, 0, GL_RGB, 0, 0, width, height, 0));
		
#ifdef DEBUG
		unsigned char *foo = (unsigned char*) malloc(width * height * 4);
		GLC(glGetTexImage(A_TEXTURE_TYPE, 0, GL_RGBA, GL_UNSIGNED_BYTE, foo));
		printf("texture [");
		for (vsize_t i = 0; i < 32; i++) printf("%02x%s", (int) foo[i + width * 40 + 40], ((i & 3) == 3) ? " " : "-");
		printf("]\n");
		free(foo);
#endif
#if ! A_EXACT_TEXTURE
		glTexParameteri(A_TEXTURE_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(A_TEXTURE_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
		_redraw_layer = layer;
		if (layer >= _layers) _layers = layer + 1;
		return true;
	}
	
	bool recallLayer(vsize_t layer) {
		if (_layers == 0) return false;
		if (layer >= _layers) layer = _layers - 1; // recall the topmost layer if layer is too high
		ALog(" <- recall layer %d (texture %d)", layer, _layer[layer]);
		
		glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
		glDisable(GL_BLEND); // make sure all gets replaced, no blending
		glDisable(GL_DEPTH_TEST);
		glEnable(A_TEXTURE_TYPE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		GLC(glBindTexture(A_TEXTURE_TYPE, _layer[layer]));
		glBegin(GL_QUADS);
		int width = _frame.width - 1, height = _frame.height - 1;

#if ! A_EXACT_TEXTURE
		int tw = 32, th = 32;
		while (tw < width) tw <<= 2;
		while (th < height) th <<= 2;
		double tpw = (double) width / (double) tw;
		double tph = (double) height / (double) th;
		ALog("%s: recalling POTS texture %d x %d", describe(), tw, th);
#endif
		
#if A_EXACT_TEXTURE
		glTexCoord2i(0, 0);
		glVertex3f(-0.5, -0.5, 0);
		glTexCoord2i(0, height);
		glVertex3f(-0.5, height - 0.5, 0);
		glTexCoord2i(width, height);
		glVertex3f(width - 0.5, height - 0.5, 0);
		glTexCoord2i(width, 0);
		glVertex3f(width - 0.5, -0.5, 0);
#else
		glTexCoord2d(0.0, 0.0);
		glVertex3f(-0.5, -0.5, 0);
		glTexCoord2d(0.0, tph);
		glVertex3f(-0.5, height - 0.5, 0);
		glTexCoord2d(tpw, tph);
		glVertex3f(width - 0.5, height - 0.5, 0);
		glTexCoord2d(tpw, 0.0);
		glVertex3f(width - 0.5, -0.5, 0);
#endif
		glEnd();
		glPopAttrib();		
		
		_redraw_layer = layer + 1; // no need to redraw ourself anymore
		return true;
	}
	
	// Note that redraw layer specifies the bottom-most layer to be redrawn, i.e. 0 means that everything needs to be cleared, 1 means that layer 0 can be restored etc.
	void setRedrawLayer(vsize_t layer) {
		ALog("%s: setRedrawLayer(%d)", describe(), layer);
		if (_redraw_layer > layer)
			_redraw_layer = layer;
	}
	
	void setTextColor(AColor col) {
		text_color = col;
	}
	
	AColor textColor() { return text_color; }
	
	void begin() {
		profStart()
		if (_layers == 0) {
			if (first_gl)
				first_gl_call();
			GLC(glGenTextures(_max_layers, _layer));
			_layers = _max_layers;
		}
		ALog("%s: begin, redraw layer = %d", describe(), _redraw_layer);
		glViewport(0.0f, 0.0f, _frame.width, _frame.height);
		//glClearColor(0.0, 0.0, 0.0, 0);
		glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
		if (_redraw_layer == 0)
			glClear(GL_COLOR_BUFFER_BIT);

#ifndef WIN32
		glEnable(GL_MULTISAMPLE);
#endif
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glPushMatrix();
		// change coordinates to graphics coords
		glTranslatef(-1, -1, 0);
		glScalef( 2.0 / _frame.width, 2.0 / _frame.height, 1);
		// anti-aliasing trick
		glTranslatef(0.5, 0.5, 0.0); // subpixel shift to make sure that lines don't wash over two integration regions. However, filled areas are probably still in trouble

		if (_redraw_layer > 0)
			recallLayer(_redraw_layer - 1);

		// glColor4f(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0);
		_prof(profReport("$window.begin"))
	}

	void draw();
	
	void end() {
		_prof(profReport("^window.end"))
		glPopMatrix();
		glFlush();
		_prof(profReport("$window.end"))
	}
	
	vsize_t redrawLayer() { return _redraw_layer; }

	AObject *rootVisual() { return _rootVisual; }
	
	ARect frame() { return _frame; }

	void setFrame(ARect frame);
	
	void setRootVisual(AObject *rv) {
		if (_rootVisual) _rootVisual->release();
		_rootVisual = rv;
		if (rv) rv->retain();
	}
	
	// enter modal mode with the given owner. returns false if owner is NULL or if already in modal mode by other owner, true otherwise
	virtual bool enterModal(AObject *owner) {
		if (owner && owner == modalOwner) return true;
		if (modalOwner || !owner) return false;
		modalOwner = owner;
		owner->retain();
		return true;
	}
	
	virtual bool leaveModal(AObject *owner) {
		if (modalOwner && modalOwner == owner) {
			modalOwner->release();
			modalOwner = NULL;
			return true;
		}
		return false;
	}
	
	virtual void redraw() {};
	
	/* the following are rendering methods that are off-loaded to the window implementation since they are not part of OpenGL */
	virtual void glstring(APoint pt, APoint adj, AFloat rot, const char *txt) {}; //  we need some implementation help here since gl cannot draw text
	virtual void glfont(const char *name, AFloat size) {}
	virtual ASize glbbox(const char *txt) { return AMkSize(strlen(txt) * 5.6, 10.0); } // jsut a very crude fallback
	
	void setDirtyFlag(int *newDF) { dirtyFlag = newDF; };
	void setDirtyFlagLayer(vsize_t layer) { dirtyFlagLayer = layer; }
	bool isDirty() { return (dirtyFlag && dirtyFlag[0] != 0); }

	bool event(AEvent event);

	virtual bool canClose() { return true; }
	virtual void close() {};

	virtual void setVisible(bool flag) {};
	virtual bool visible() { return false; }
};

#endif
