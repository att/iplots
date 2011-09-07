/*
 *  AX11Window.cpp
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/1/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifdef USE_X11

#include "AX11Window.h"

static int lastButtonState = 0;
static APoint lastMousePos;

AFreeType *sharedFT;
AX11Display *mainDisplay;
char *core_font_path;

extern "C" {
	AX11Window *AX11_CreateWindow(AVisual *visual, APoint position);
	SEXP A_SetCoreFontPath(SEXP sPath);
}

AX11Window *AX11_CreateWindow(AVisual *visual, APoint position)
{
	ARect frame = AMkRect(position.x, position.y, visual->frame().width, visual->frame().height);
	if (!mainDisplay)
		mainDisplay = new AX11Display(0);
	AX11Window *win = new AX11Window(frame, mainDisplay);
	win->setRootVisual(visual);
	visual->setWindow(win);
	return win;
}

void AX11Window::heartbeat() {
	if (dirtyFlag && dirtyFlag[0]) {
		dirtyFlag[0]++;
		if (dirtyFlag[0] > 2)
			redraw();
	}
}

AX11Window* AX11Display::findWindow(Window w) {
	int i = 0;
	while (i < wins) {
		if (win_list[i] && win_list[i]->isXWindow(w))
			return win_list[i];
		i++;
	}
	return 0;
}

#if 0	
static DWORD WINAPI AWin32Heartbeat( LPVOID lpParam ) {
	AWin32Window *win = (AWin32Window*) lpParam;
	win->retain();
	while (win->active()) {
		Sleep(200);
		win->heartbeat();
	}
}

#endif

/* -- this part must be last -- disable memory allocation tracking -- */
#ifdef strdup
#undef strdup
#endif
#ifdef free
#undef free
#endif

SEXP A_SetCoreFontPath(SEXP sPath) {
	if (TYPEOF(sPath) != STRSXP || LENGTH(sPath) < 1)
		Rf_error("invalid font path");
	if (core_font_path)
		free(core_font_path);
	core_font_path = strdup(CHAR(STRING_ELT(sPath, 0)));
	return R_NilValue;
}

#endif
