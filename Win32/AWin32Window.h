/*
 *  AWin32Window.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/1/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifdef WIN32
#ifndef A_WIN32_WINDOW_H__
#define A_WIN32_WINDOW_H__

#include "AWindow.h"
#include "AVisual.h"

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

/* --- GraphApp support --- we use GA from R headers --- */
#ifndef NATIVE_UI
extern "C" {
#undef __cplusplus
#include <ga.h>
#define __cplusplus
}
#undef resize
#undef draw

/* this is an excerpt from internal.h objinfo structure
 we need it to get the window handle for native GDI */
typedef struct ga_objinfo {
	int kind;
	int refcount;
	HANDLE handle;
} ga_object;
#endif

static void HelpClose(window w);
static void HelpExpose(window w, rect r);
static void HelpResize(window w, rect r);
static void SetupPixelFormat(HDC hDC);

class AWin32Window : public AWindow {
	window gawin;
	HWND wh;
	HDC hDC;
	HGLRC hRC;
public:
	AWin32Window(ARect frame) : AWindow(frame) {
#if 0
		if (be->dpix<=0) { /* try to find out the DPI setting */
			HWND dw = GetDesktopWindow();
			if (dw) {
				HDC  dc = GetDC(dw);
				if (dc) {
					int dpix = GetDeviceCaps(dc, LOGPIXELSX);
					int dpiy = GetDeviceCaps(dc, LOGPIXELSY);
					ReleaseDC(dw, dc);
					if (dpix>0) be->dpix=(double)dpix;
					if (dpiy>0) be->dpiy=(double)dpiy;
				}
			}
		}
#endif

		hDC = 0;
		hRC = 0;
		wh = 0;

#ifdef NATIVE_UI
		if (!inited_w32) {
			WNDCLASS wc;
			
			get_R_window();
			
			{ FILE *f=fopen("c:\\debug.txt","w"); fprintf(f,"init W32; isMDI=%d, parent=%x\n", isMDI, (unsigned int) parent); fclose(f); }
			
			wc.style=0; wc.lpfnWndProc=WindowProc;
			wc.cbClsExtra=0; wc.cbWndExtra=0;
			wc.hInstance=instance;
			wc.hIcon=0;
			wc.hCursor=LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground=GetSysColorBrush(COLOR_WINDOW);
			wc.lpszMenuName=NULL;
			wc.lpszClassName="RCairoWindow";
			
			RegisterClass(&wc);
			
			wc.cbWndExtra=/* CBWNDEXTRA */ 1024;
			wc.lpszClassName="RCairoDoc";
			
			RegisterClass(&wc);
			
			inited_w32 = 1;
		}
		
		while (l->be && l->next) l=l->next;
		if (l->be) { l->next = (w32chain_t*) calloc(1,sizeof(w32chain_t)); l = l->next; }
		
		l->be = be;	
#else
		{
			gawin = newwindow("Acinonyx", rect(frame.x, frame.y, frame.width ,frame.height),
							  Document|StandardWindow);
			wh = (HWND) ((ga_object*)gawin)->handle;

			/* setup DC and wgl context */
			hDC = GetDC(wh);
			SetupPixelFormat(hDC);
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			setdata(gawin, (void *) this);
			setresize(gawin, HelpResize);
			setredraw(gawin, HelpExpose);
			setclose(gawin, HelpClose);
			
#if 0
			addto(gawin);
			gsetcursor(gawin, ArrowCursor);
			if (ismdi()) {
				int btsize = 24;
				control tb;
				
				tb = newtoolbar(btsize + 4);
				gsetcursor(tb, ArrowCursor);
				addto(tb);
			}
#endif
			addto(gawin);
			newmenubar(NULL);
			newmdimenu();
			show(gawin); /* twice, for a Windows bug */
			show(gawin);
			//BringToTop(gawin, 0);
		}
#endif
			
#ifdef NATIVE_UI
		if (isMDI) {
			HWND mdiClient;
			CLIENTCREATESTRUCT ccs;
			HMENU rm = GetMenu(parent);
			HMENU wm = GetSubMenu(rm, 4);
			ccs.hWindowMenu = wm;
			ccs.idFirstChild = 512+0xE000; /* from graphapp/internal.h -> MinDocID */
			Rprintf("Menu = %x, Windows = %x, ID = %x\n", rm, wm, ccs.idFirstChild);
			
			mdiClient = CreateWindow("MDICLIENT","Cairo",WS_CHILD|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,width,height,
									 parent,NULL,instance,&ccs);
			ShowWindow(mdiClient, SW_SHOW);
			
			Rprintf("mdiClient: %x\n", mdiClient);
			{
				MDICREATESTRUCT mcs;
				mcs.szTitle = "Cairo device";
				mcs.szClass = "RCairoDoc";
				mcs.hOwner  = instance;
				mcs.x = CW_USEDEFAULT; mcs.cx = width;
				mcs.y = CW_USEDEFAULT; mcs.cy = height;
				mcs.style = WS_MAXIMIZE|WS_MINIMIZE|WS_CHILD|WS_VISIBLE;
				xd->wh = l->w = (HWND) SendMessage (mdiClient, WM_MDICREATE, 0, 
													(LONG) (LPMDICREATESTRUCT) &mcs);
				Rprintf("MDICREATE result: %x\n", l->w);
			}
			{ FILE *f=fopen("c:\\debug.txt","a"); fprintf(f,"parent: %x\nMDIclient: %x\ndoc: %x\n", parent, mdiClient, l->w); fclose(f); }
		} else {
			l->w = xd->wh = CreateWindow("RCairoWindow","Cairo",WS_OVERLAPPEDWINDOW,
										 100,100,width,height,
										 parent,NULL,instance,NULL);
		}
		
		ShowWindow(xd->wh, SW_SHOWNORMAL);
		
		w32_resize(be, width, height);
		
		UpdateWindow(xd->wh);
#endif		
	}
	
	void resize(ARect newSize) {
		AVisual * v = (AVisual*) rootVisual();
		if (v) {
			ARect vf = v->frame();
			vf.width = _frame.width = newSize.width;
			vf.height = _frame.height = newSize.height;
			v->moveAndResize(vf);
		}
	}
	
	void expose() {
		AVisual * v = (AVisual*) rootVisual();
		if (v) {
			v->begin();
			v->draw();
			v->end();
		}
	}
	
	virtual void close() {
		if (hDC) wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);
		
		/*      Send quit message to queue*/
		PostQuitMessage(0);
	}
};



#endif
#endif
