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

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include "AWindow.h"
#include "AVisual.h"

#include "RObject.h" /* for debugging with Rprintf */

/* --- GraphApp support --- we use GA from R headers --- */
#ifndef NATIVE_UI
extern "C" {
#undef __cplusplus
#include <ga.h>
#define __cplusplus
}
#undef resize
#undef draw
#undef redraw

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
static void HelpMouseClick(window w, int button, point pt);
static void HelpMouseMove(window w, int button, point pt);
static void HelpMouseUp(window w, int button, point pt);
static void HelpMouseDown(window w, int button, point pt);
static void HelpKeyDown(control w, int key);
static void HelpKeyAction(control w, int key);

static void SetupPixelFormat(HDC hDC);

static DWORD WINAPI AWin32Heartbeat( LPVOID lpParam );

#ifdef DEBUG
static void PrintLastError(const char *fname, BOOL result) 
{ 
	if (result) return; // return on success
    // Retrieve the system error message for the last-error code
	
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 
	
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				  FORMAT_MESSAGE_FROM_SYSTEM |
				  FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,
				  dw,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPTSTR) &lpMsgBuf,
				  0, NULL );
    Rprintf("%s failed with error %d: %s\n", fname, dw, (const char*) lpMsgBuf);
    LocalFree(lpMsgBuf);
}
#else
#define PrintLastError(FN, X) X
#endif

// FIXME: implement check for anti-aliasing and review how to enable it on Win32

#ifndef WGL_FONTS

// FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#define GL_TEXTURE_RECTANGLE_EXT GL_TEXTURE_2D

class AFreeType {
protected:
	FT_Library library;
	FT_Face face;
	int width, height, bsize;
	int adx[255];
	int ghs[255];
	char *buf;
	GLuint texName;
public:
	AFloat dpi_x, dpi_y;
	
	AFreeType() {
		FT_Init_FreeType(&library);
		dpi_x = 100;
		dpi_y = 100;
		texName = 0;
		memset(adx, 0, sizeof(adx));
		memset(ghs, 0, sizeof(ghs));
		buf = (char*) malloc(bsize = 256*1024);
	}
	
	bool setFont(const char *fn) {
		if (FT_New_Face(library, fn, 0, &face )) return false;
		return true;
	}
	
	bool setFontSize(AFloat size) {
		if (FT_Set_Char_Size(face, 0, size * 64.0, dpi_x, dpi_y)) return false;
		FT_GlyphSlot  slot = face->glyph;
		//FILE *f = fopen("c:/tmp/debug.txt","a");
		//if (f) fprintf(f, "NewFace '%s' ok\n", fn);
		for (int i = 32; i < 128; i++)
			if (!FT_Load_Char(face, i, FT_LOAD_DEFAULT)) {
				adx[i] = slot->advance.x;
				ghs[i] = slot->metrics.height;
				//		fprintf(f," %c[%d/%d]", (char) i, (int) adx[i], ghs[i]);
			}
		//if (f) fclose(f);
		return true;
	}
	
	ASize bbox(const char *txt) {
		FILE *f = fopen("c:/tmp/debug.txt","a");
		if (f) fprintf(f, "bbox('%s')\n", txt);
		ASize box = AMkSize(0, 0);
		while (*txt) {
			box.width += adx[*txt] / 64.0;
			if (ghs[*txt] > box.height)
				box.height = ghs[*txt];
			txt++;
		}
		box.height /= 64.0;
		if (f) fprintf(f, " -> %g, %g\n", box.width, box.height);
		if (f) fclose(f);
		return box;
	}
	
	/* FIXME: simplified rendering - we are advancing only in whole pixels */
	bool generateTexture(const char *txt) {
		// compute texture size
		if (!txt) return false;
		ASize bb = bbox(txt);
		int txw = 8, txh = 8;
		while (txw < bb.width) txw <<= 1;
		while (txh < bb.height) txh <<= 1;
		if (txw * txh > bsize) // texture bigger than the buffer - bail out for sanity
			return false;
		memset(buf, 0, txw* txh);
		
		FT_GlyphSlot  slot = face->glyph;
		FT_UInt       glyph_index;
		int           x = 0, y = 0;
		while (*txt) {
			if (FT_Load_Char(face, *txt, FT_LOAD_RENDER)) continue;
			FT_Bitmap bitmap = slot->bitmap;
			for (int i = 0; i < bitmap.rows; i++)
				memcpy(buf + i * txw + x, bitmap.buffer + i * bitmap.pitch, bitmap.width);
			x += slot->advance.x >> 6;
			txt++;
		}
		
		glPushAttrib(GL_TEXTURE_BIT);
		if (texName == 0) glGenTextures (1, &texName);
		glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texName);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, txw, txh, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
		glPopAttrib();
		width = txw;
		height = txh;
		return true;
	}
	
	bool drawTexture(APoint point, APoint adj, AFloat rot) {
		if (texName) {
			APoint ll, lr, ul, ur;
			
			ll = point;
			double th = rot * 3.14159265 / 180.0; // theta
			double cth = cos(th), sth = sin(th); // cos(theta), sin(theta)
			// base point in x (width) and y (height) direction (delta from point of text origin)
			lr.x = width * cth;
			lr.y = width * sth;
			ul.x = - height * sth;
			ul.y = height * cth;
			// diagonal point
			ur.x = lr.x + ul.x;
			ur.y = lr.y + ul.y;
			// multiply adj by the diagonal
			//adj.x *= - ur.x;
			//adj.y *= - ur.y;
			// adjust the origin
			
#ifdef DEBUG
			glBegin(GL_LINE_STRIP);
			glColor4f(0.0, 0.0, 1.0, 0.5);
			glVertex2f(ul.x + ll.x, ul.y + ll.y);
			glVertex2f(ll.x, ll.y);
			glColor4f(0.0, 1.0, 0.0, 0.5);
			glVertex2f(lr.x + ll.x, lr.y + ll.y);
			glEnd();
#endif
			
			ll.x += - adj.x * lr.x - adj.y * ul.x;
			ll.y += - adj.y * ul.y - adj.x * lr.y;
			// make sure the texture is pixel-aligned
			ll.x = round(ll.x) - 0.5;
			ll.y = round(ll.y) - 0.5;
			// adjust all other points according to the text origin
			lr.x += ll.x;
			lr.y += ll.y;
			ul.x += ll.x;
			ul.y += ll.y;
			ur.x += ll.x;
			ur.y += ll.y;
			
#ifdef DEBUG
			glBegin(GL_LINE_STRIP);
			glColor4f(1.0, 0.0, 0.0, 0.5);
			glVertex2f(ul.x, ul.y);
			glVertex2f(ll.x, ll.y);
			glColor4f(0.0, 1.0, 0.0, 0.5);
			glVertex2f(lr.x, lr.y);
			glEnd();
#endif
			
			// glColor4f([textColor redComponent], [textColor greenComponent], [textColor blueComponent], [textColor alphaComponent]);
			// ALog("points: A(%g,%g) B(%g,%g), C(%g,%g), D(%g,%g) (th=%g, cth=%g, sth=%g)\n", ll.x, ll.y, lr.x, lr.y, ur.x, ur.y, ul.x, ul.y, th, cth, sth);
			
			glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT for glBlendFunc, GL_ENABLE_BIT for glEnable / glDisable
			
			glDisable (GL_DEPTH_TEST); // ensure text is not remove by depth buffer test.
			glEnable (GL_BLEND); // for text fading
			glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // ditto
			glEnable (GL_TEXTURE_RECTANGLE_EXT);  
			
			glBindTexture (GL_TEXTURE_RECTANGLE_EXT, texName);
			glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); // draw upper left in world coordinates
			glVertex2f (ul.x, ul.y);
			
			glTexCoord2f (0.0f, height); // draw lower left in world coordinates
			glVertex2f (ll.x, ll.y);
			
			glTexCoord2f (width, height); // draw lower right in world coordinates
			glVertex2f (lr.x, lr.y);
			
			glTexCoord2f (width, 0.0f); // draw upper right in world coordinates
			glVertex2f (ur.x, ur.y);
			glEnd ();
			
			glPopAttrib();
		}		
	}
};
	
	
extern AFreeType *sharedFT;

#endif
	
class AWin32Window : public AWindow {
protected:
	window gawin;
	HWND wh;
	HDC hDC;
	HGLRC hRC;
	HFONT hFont;
	char *font_name;
	double font_size;
	bool _active;
#ifndef WGL_FONTS
	AFreeType *ft;
#endif
public:
	// FIXME: window coordinates in Windows are flipped so placement of windows has to be re-calculated
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

		_active = true;
		font_size = 10.0;
		font_name = strdup("Arial");
	
#ifndef WGL_FONTS
		if (sharedFT)
			ft = sharedFT;
		else {
			ft = new AFreeType();
			ft->setFont("C:/Windows/Fonts/arial.ttf");
			ft->setFontSize(font_size);
			sharedFT = ft;
		}
#endif
		
		hDC = 0;
		hRC = 0;
		wh = 0;
		hFont = 0;

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
		if (l->be) { l->next = (w32chain_t*) AZAlloc(1,sizeof(w32chain_t)); l = l->next; }
		
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
#ifdef DEBUG
			Rprintf("wglCreateContext(%x): %x\n", (int) hDC, (int) hRC);
#endif
			wglMakeCurrent(hDC, hRC);

			setdata(gawin, (void *) this);
			setresize(gawin, HelpResize);
			setredraw(gawin, HelpExpose);
			setclose(gawin, HelpClose);
			setmousedown(gawin, HelpMouseClick);
			setmousemove(gawin, HelpMouseMove);
			setmousedrag(gawin, HelpMouseMove);
			setmouseup(gawin, HelpMouseUp);
			setmousedown(gawin, HelpMouseDown);
			setkeydown(gawin, HelpKeyDown);
			setkeyaction(gawin, HelpKeyAction);
			
			addto(gawin);
			gsetcursor(gawin, ArrowCursor);
			if (ismdi()) {
				int btsize = 24;
				control tb;
				
				tb = newtoolbar(btsize + 4);
				gsetcursor(tb, ArrowCursor);
				addto(tb);
			}

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
		
		CreateThread(NULL, 0, AWin32Heartbeat, this, 0, NULL);
	}
	
	virtual ~AWin32Window() {
		if (font_name) AFree(font_name);
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
		//if (gawin) GA_redraw(gawin);
		if (wh) InvalidateRect(wh, NULL, FALSE);
	}

	void expose() {
		AVisual * v = (AVisual*) rootVisual();
#ifdef DEBUG
		Rprintf("%s: expose, visual=%p\n", describe(), v);
#endif
		if (v) {
			if (dirtyFlag) dirtyFlag[0] = 0;
			wglMakeCurrent(hDC, hRC);
			begin();
			draw();
			end();
			if (dirtyFlag) dirtyFlag[0] = 0;
		}
#ifndef USE_GDI
		SwapBuffers(hDC);
#endif
	}
	
	virtual void close() {
#ifdef DEBUG
		Rprintf("%s: close\n", describe());
#endif
		if (hDC) wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hRC);
		
		DeleteDC(hDC);
		if (gawin) {
			del(gawin);
			doevent();
			gawin=0;
		}
		_active = FALSE;
		/*      Send quit message to queue*/
		/* PostQuitMessage(0); */
	}
	
	// FIXME: this implementation lacks some hearbeat with dirtly flag observation
	// FIXME: we are not passing any key/mouse events
	
#ifndef WGL_FONTS
	virtual ASize glbbox(const char *txt) {
		return ft->bbox(txt);
	}
	
	virtual void glstring(APoint pt, APoint adj, AFloat rot, const char *txt) {
		ft->generateTexture(txt);
		ft->drawTexture(pt, adj, rot);
	}
	
	virtual void glfont(const char *name, AFloat size) {
		// FIXME: we ignore font name until we know how to locate TTF files
		ft->setFontSize(size);
	}

#else

#define FONT_LIST_ID 100
#define FONT_LIST_START 0
#define FONT_LIST_CHARS 255
	
	virtual void glstring(APoint pt, APoint adj, AFloat rot, const char *txt) {
#ifdef USE_GDI
		LOGFONT lf = { 0 };
		strncpy(lf.lfFaceName, font_name, LF_FACESIZE);
		lf.lfWeight = FW_NORMAL;
		
		int nOldBkMode = SetBkMode(hDC, TRANSPARENT);

		lf.lfOrientation = lf.lfEscapement = rot * 10.0;
		
		//int nOldGMode = SetGraphicsMode( hDC, GM_COMPATIBLE );
		
		lf.lfHeight = (font_size * (double) GetDeviceCaps(hDC, LOGPIXELSY)) / 72.0;
		
		HFONT hFont = (HFONT) CreateFontIndirect(&lf);
		HGDIOBJ hPrevFont = SelectObject(hDC, hFont);
		
		SIZE bbox;
		if (GetTextExtentPoint32(hDC, txt, strlen(txt), &bbox)) {
			ASize ts = AMkSize((double) bbox.cx, (double) bbox.cy);
			APoint ll, lr, ul;			
			ll = pt;
			double th = rot * PI / 180.0; // theta
			double cth = cos(th), sth = sin(th); // cos(theta), sin(theta)
			
			// base point in x (width) and y (height) direction (delta from point of text origin)
			lr.x = ts.width * cth;
			lr.y = ts.width * sth;
			ul.x = - ts.height * sth;
			ul.y = ts.height * cth;
			// adjust the origin by the two orthogonal vectors
			ll.x += - adj.x * lr.x - adj.y * ul.x;
			ll.y += - adj.y * ul.y - adj.x * lr.y;
			/* make sure the texture is pixel-aligned
			ll.x = round(ll.x) - 0.5;
			ll.y = round(ll.y) - 0.5; */
			TextOut(hDC, ll.x, _frame.height - ll.y, txt, strlen(txt));			
		} else // if getting the size fails, just spit it out anyway
			TextOut(hDC, pt.x, _frame.height - pt.y, txt, strlen(txt));

		SelectObject(hDC, hPrevFont);
		DeleteObject(hFont);
		
		SetBkMode( hDC, nOldBkMode );
		//SetGraphicsMode( hDC, nOldGMode );
#else
		glPushMatrix();
		SIZE bbox;
		glTranslated(pt.x, pt.y, 0.0);
		
		ASize ts;
		if (GetTextExtentPoint32(hDC, txt, strlen(txt), &bbox)) {
			ts = AMkSize((double) bbox.cx, (double) bbox.cy);
			APoint lr, ul;			

			double th = rot * PI / 180.0; // theta
			double cth = cos(th), sth = sin(th); // cos(theta), sin(theta)
			
			// base point in x (width) and y (height) direction (delta from point of text origin)
			lr.x = ts.width * cth;
			lr.y = ts.width * sth;
			ul.x = - ts.height * sth;
			ul.y = ts.height * cth;
			// adjust the origin by the two orthogonal vectors
			glTranslated(- adj.x * lr.x - adj.y * ul.x, - adj.y * ul.y - adj.x * lr.y, -1.0);
			if (rot != 0.0) glRotated(rot, 0.0, 0.0, 1.0);
		} else // very rough guess
			ts = AMkSize(5.6 / 12.0 * font_size * (double) strlen(txt), 0.85 * font_size);

#ifdef DEBUG
		glBegin(GL_LINE_STRIP);
		glColor4f(0.0, 0.0, 1.0, 0.5);
		glVertex2f(0.0, ts.height);
		glVertex2f(0.0, 0.0);
		glColor4f(0.0, 1.0, 0.0, 0.5);
		glVertex2f(ts.width, 0.0);
		glEnd();
#endif
		glColor4f(0.0, 0.0, 0.0, 1.0);
		glRasterPos2d(0.0, 0.0);
		glListBase(FONT_LIST_ID - FONT_LIST_START);
		glCallLists(strlen(txt), GL_UNSIGNED_BYTE, txt);
		glPopMatrix();
#endif
	}

	virtual void glfont(const char *name, AFloat size) {
		bool changed = false;
		if (!*name) name = "Arial"; /* default font is Arial */
		if (size > 0.0 && size != font_size) {
			changed = true;
			font_size = size;
		}
		if (name && strcmp(name, font_name)) {
			AFree(font_name);
			font_name = strdup(name);
			changed = true;
		}
		if (changed) {
			if (hFont)
				glDeleteLists(FONT_LIST_ID, FONT_LIST_CHARS);
			LOGFONT lf = { 0 };
			strncpy(lf.lfFaceName, font_name, LF_FACESIZE);
			lf.lfWeight = FW_NORMAL;
			lf.lfOrientation = lf.lfEscapement = 0.0;
			lf.lfHeight = (font_size * (double) GetDeviceCaps(hDC, LOGPIXELSY)) / 72.0;
#ifdef USE_OUTLINES
			lf.lfOutPrecision = OUT_TT_PRECIS;
#endif
			HFONT newFont = (HFONT) CreateFontIndirect(&lf);
			if (SelectObject(hDC, newFont) == hFont && hFont)
				DeleteObject(hFont);
			hFont = newFont;
#ifdef DEBUG
			Rprintf("create wgl outlines from font '%s', size %g\n", font_name, font_size);
#endif
#ifdef USE_OUTLINES
			GLYPHMETRICSFLOAT agmf[FONT_LIST_CHARS];
			PrintLastError("wglUseFontOutlines",
						   wglUseFontOutlines(hDC, FONT_LIST_START, FONT_LIST_CHARS, FONT_LIST_ID, 0.0, 0.1, WGL_FONT_POLYGONS, (GLYPHMETRICSFLOAT*) &agmf));
#else
			PrintLastError("wglUseFontBitmaps",
						   wglUseFontBitmaps(hDC, FONT_LIST_START, FONT_LIST_CHARS, FONT_LIST_ID));
#endif
		}
	}
	
#endif /* WGL_FONTS */
	//

	void heartbeat();
};



#endif
#endif
