/*
 *  AX11Window.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/1/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_X11_WINDOW_H__
#define A_X11_WINDOW_H__

#include <X11/Xlib.h>
#include "AOpenGL.h"

#include "AWindow.h"
#include "AVisual.h"

#include "RObject.h" /* for debugging with Rprintf */

#ifdef DEBUG
#define fdebug Rprintf
#else
#define fdebug(X, ...) {}
#endif

#include "AFreeType.h"
extern AFreeType *sharedFT;

#include <R_ext/eventloop.h>

extern Display   *xdisplay;

class AX11Window;

static void AX11_rhand(void*);

class AX11Display {
protected:
	Display *xdisplay;
	AX11Window *win_list[512];
	int wins;
	InputHandler *rhand;
public:
	
	/* name can be NULL */
	AX11Display(const char *name) {
		wins = 0;
		rhand = 0;
		xdisplay = XOpenDisplay(name);
		if (xdisplay) {
			rhand = addInputHandler(R_InputHandlers, ConnectionNumber(xdisplay), AX11_rhand, XActivity);
			if (rhand)
				rhand->userData = this;
		}
	}
	
	~AX11Display() {
		if (xdisplay)
			XCloseDisplay(xdisplay);
	}
	
	bool isValid() {
		return xdisplay ? true : false;
	}
	
	Display *display() {
		return xdisplay;
	}
	
	/* we do NOT retain to avoid cycles - we assume that the window will remove itself in the destructor */
	void addWindow(AX11Window *w) {
		int i = 0;
		while (i < wins && win_list[i]) i++;
		win_list[i] = w;
		if (i == wins)
			wins++;
	}
	
	void removeWindow(AX11Window *w) {
		if (!wins) return;
		int i = 0;
		while (i < wins) {
			if (win_list[i] == w)
				break;
			i++;
		}
		win_list[i] = 0;
		if (i == wins - 1)
			wins--;
	}
	
	void processEvents() {
		AX11_rhand(this);
	}
	
	AX11Window *findWindow(Window w);
};

class AX11Window : public AWindow {
protected:
	XVisualInfo *xvinfo;
	Window       xwindow;
	GLXContext   glxctx;
	Display     *xdisplay;
	AX11Display *display;
	
	char *font_name;
	double font_size;
	bool _active;
	AFreeType *ft;

public:
	// FIXME: window coordinates in Windows are flipped so placement of windows has to be re-calculated
	AX11Window(ARect frame, AX11Display *display_) : AWindow(frame), display(display_) {
		_active = true;
		font_size = 10.0;
		font_name = strdup("Arial");
	
		if (sharedFT)
			ft = sharedFT;
		else {
#ifdef __APPLE__
			const char *fpath = "/Library/Fonts/Arial.ttf";
#else
			const char *fpath = "/usr/share/fonts/truetype/ttf-liberation/LiberationSans-Regular.ttf";
#endif
			ft = new AFreeType();
			ft->setFont(fpath);
			ft->setFontSize(font_size);
			sharedFT = ft;
		}

		if (!display || !display->display())
			Rf_error("Unable to connect to X11 display");

		xdisplay = display->display();
		
		if (!glXQueryExtension(xdisplay, 0, 0)) {
			XCloseDisplay(xdisplay);
			xdisplay = 0;
			Rf_error("Specified X11 display does not support GLX extension");
		}

		{
			int attr[] = {
				GLX_RGBA,
				GLX_DOUBLEBUFFER,
				GLX_RED_SIZE, 4,
				GLX_GREEN_SIZE, 4,
				GLX_BLUE_SIZE, 4,
				GLX_DEPTH_SIZE, 1,
				None
			};
		
			xvinfo = glXChooseVisual(xdisplay, DefaultScreen(xdisplay), attr);
		}
		if (!xvinfo) {
			int attr[] = { GLX_RGBA, None };
			xvinfo = glXChooseVisual(xdisplay, DefaultScreen(xdisplay), attr);
			if (!xvinfo)
				Rf_error("Unable to retrieve suitable GLX visual");
		}
		
		XSetWindowAttributes attrib;
		Window xparent = RootWindow(xdisplay, DefaultScreen(xdisplay));
		
		attrib.colormap = XCreateColormap(xdisplay, xparent, xvinfo->visual, AllocNone);
		attrib.border_pixel = 0;
		attrib.event_mask = ButtonMotionMask
		| PointerMotionHintMask
		| VisibilityChangeMask 
		| ExposureMask
		| StructureNotifyMask 
		| ButtonPressMask 
		| KeyPressMask
		| KeyReleaseMask
		| ButtonReleaseMask;
		
		xwindow = XCreateWindow(xdisplay, xparent,
					0, 0, frame.width, frame.height, 0, 
					xvinfo->depth, InputOutput,
					xvinfo->visual,	CWEventMask | CWColormap | CWBorderPixel,
					&attrib);
		if (!xwindow)
			Rf_error("Unable to create window");

		display->addWindow(this);

		XSync(xdisplay, False);
		
		glxctx = glXCreateContext(xdisplay, xvinfo, NULL, True);
		
		// CreateThread(NULL, 0, AWin32Heartbeat, this, 0, NULL);
	}
	
	void x11_destroy() {
		if (xdisplay) {
			if (glxctx) {
				glXMakeCurrent(xdisplay, None, NULL);
				glXDestroyContext(xdisplay, glxctx);
				glxctx = NULL;
			}
			
			XSync(xdisplay, False);
			if (display)
				display->processEvents();
		}
		
		if (xvinfo) {
			XFree(xvinfo);
			xvinfo = 0;
		}
		
		display->removeWindow(this);
	}
	
	virtual ~AX11Window() {
		if (font_name) AFree(font_name);
		x11_destroy();
	}
	
	bool isXWindow(Window w) {
		return (w == xwindow) ? true : false;
	}
	
	bool active() { return _active; }
	
	/* FIXME: destroy ...
	{
		if (xd->cdc) {
			DeleteDC(xd->cdc); xd->cdc=0;
			DeleteObject(xd->cb); xd->cb=0;
		}
		if (xd->gawin) {
			del(xd->gawin);
			doevent();
			xd->gawin=0;
		} else if (xd->wh) {
			DestroyWindow(xd->wh);
		}
	} */
	
	void resize(ARect newSize) {
		AVisual * v = (AVisual*) rootVisual();
		if (v) {
			_frame.width = newSize.width;
			_frame.height = newSize.height;
			ARect vf = AMkRect(0.0, 0.0, _frame.width, _frame.height); // force 0.0/0.0 - root visual is defined to fill the whole window in all cases
#ifdef DEBUG
			Rprintf("%s: resizing visual to %g x %g\n", describe(), vf.width, vf.height);
#endif
			v->moveAndResize(vf);
		}
	}

	virtual void redraw() {
		/* GA calls don't always work - we have to use Win's even loop to make sure it is handled properly */
		//if (gawin) GA_draw(gawin);
		//if (gawin) GA_redraw(gawin); /* redraw = clear+draw so it's bad anyway */
		//if (wh) InvalidateRect(wh, NULL, FALSE);
	}

	void expose() {
		AVisual * v = (AVisual*) rootVisual();
#ifdef DEBUG
		Rprintf("%s: expose, visual=%p\n", describe(), v);
#endif
		if (v) {
			if (dirtyFlag) dirtyFlag[0] = 0;
			glXMakeCurrent(xdisplay, xwindow, glxctx);
			begin();
			draw();
			end();
			glXSwapBuffers(xdisplay, xwindow);
			if (dirtyFlag) dirtyFlag[0] = 0;
		} else { /* I'm not sure this branching is realistic or even necessary ... */
			if (dirtyFlag) dirtyFlag[0] = 0;
			glXMakeCurrent(xdisplay, xwindow, glxctx);
			begin();
			end();
			glXSwapBuffers(xdisplay, xwindow);
		}
	}
	
	virtual void close() {
		XDestroyWindow(xdisplay, xwindow);
		x11_destroy();
		_active = FALSE;
	}
	
	// FIXME: this implementation lacks some hearbeat with dirtly flag observation
	// FIXME: we are not passing any key/mouse events
	
	virtual ASize glbbox(const char *txt) {
		return ft->bbox(txt);
	}
	
	virtual void glstring(APoint pt, APoint adj, AFloat rot, const char *txt) {
		ft->generateTexture(txt);
		ft->drawTexture(pt, adj, rot, text_color);
	}
	
	virtual void glfont(const char *name, AFloat size) {
		// FIXME: we ignore font name until we know how to locate TTF files
		ft->setFontSize(size);
	}

	void processX11Event(XEvent ev) {
		switch (ev.xany.type) {
			case Expose:
				if (ev.xexpose.count == 0)
					expose();
				break;
			case ConfigureNotify:
				setFrame(AMkRect(_frame.x, _frame.y, ev.xconfigure.width, ev.xconfigure.height ));
				break;
			case DestroyNotify:
				close();
				break;
		}
	}
	
	void heartbeat();
};

static void AX11_rhand(void* userData) {
	AX11Display *disp = (AX11Display*) userData;
	if (disp) {
		Display *xdisplay = disp->display();
		int n;
		while ((n = XEventsQueued(xdisplay, QueuedAfterReading)))
			while (n--) {
				XEvent ev;
				XNextEvent(xdisplay, &ev);
				AX11Window *win = disp->findWindow(ev.xany.window);
				if (win)
					win->processX11Event(ev);
			}
	}
}

#endif
