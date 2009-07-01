/*
 *  AWin32Window.cpp
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/1/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#include "AWin32Window.h"

#ifdef WIN32

extern "C" {
	AWin32Window *AWin32_CreateWindow(AVisual *visual, APoint position);
}

AWin32Window *AWin32_CreateWindow(AVisual *visual, APoint position)
{
	ARect frame = AMkRect(position.x, position.y, visual->frame().width, visual->frame().height);
	AWin32Window *win = new AWin32Window(frame);
	win->setRootVisual(visual);
	visual->setWindow(win);
	return win;
}

static void HelpClose(window w)
{
	AWin32Window *win = (AWin32Window*) getdata(w);
	if (win) win->close();
}

static void HelpExpose(window w, rect r)
{
	AWin32Window *win = (AWin32Window*) getdata(w);
	if (win) win->expose();
#if 0
	Rcairo_backend *be = (Rcairo_backend *) getdata(w);
	Rcairo_w32_data *xd = (Rcairo_w32_data *) be->backendSpecific;
	HWND hwnd = (HWND) ((ga_object*)(xd->gawin))->handle;
	RECT cr;
	
	HDC wdc=GetDC(hwnd);
	GetClientRect( hwnd, &cr );
	
	/*
	 if (cr.right != xd->width || cr.bottom != xd->height) {
	 ReleaseDC(hwnd, wdc);
	 Rprintf("HelpExpose: size mismatch, calling resize\n");
	 Rcairo_backend_resize(be, cr.right, cr.bottom);
	 return;
	 }*/
	
	BitBlt(wdc, 0, 0, cr.right, cr.bottom, xd->cdc, 0, 0, SRCCOPY);
	ValidateRect(hwnd, NULL);
	ReleaseDC(hwnd, wdc);
#endif
}

static void HelpResize(window w, rect r)
{
	AWin32Window *win = (AWin32Window*) getdata(w);
	if (win) win->resize(AMkRect(0.0, 0.0, r.width, r.height));
}

static void SetupPixelFormat(HDC hDC)
{
	int nPixelFormat;
	
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,                                     //32 bit color mode
		0, 0, 0, 0, 0, 0,                       //ignore color bits
		0,                                      //no alpha buffer
		0,                                      //ignore shift bit
		0,                                      //no accumulation buffer
		0, 0, 0, 0,                             //ignore accumulation bits
		16,                                     //16 bit z-buffer size
		0,                                      //no stencil buffer
		0,                                      //no aux buffer
		PFD_MAIN_PLANE,                         //main drawing plane
		0,                                      //reserved
		0, 0, 0 
	};                              //layer masks ignored
	
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, nPixelFormat, &pfd);
}

#endif
