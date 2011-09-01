/*
 *  AFreeType.h
 *  Acinonyx
 *  Class providing text rendering using OpenGL textures, based on FreeType rendering of text into bitmaps
 *
 *  Created by Simon Urbanek on 9/1/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_FREE_TYPE_H
#define A_FREE_TYPE_H

/* FreeType Headers */
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#include "AOpenGL.h"

#ifdef DEBUG
#include "RObject.h"
#define GLC(U,X) { X; int ec = glGetError(); if (ec != GL_NO_ERROR) REprintf("*** GL error (%s): %s\n", U, gluErrorString(ec)); }
#else
#define GLC(U,X) X
#endif

class AFreeType {
protected:
	FT_Library library;
	FT_Face face;
	int twidth, theight;  /* texture width, height */
	int bsize;            /* texture buffer size */
	char *buf;            /* texture buffer */
	ASize box;            /* bounding box of the text in the last generated texture */
	
	/* we currently support only 7-bit characters - we don't bother decoding anything (yet) */
	int adx[255]; /* advances for all glyphs */
	int ghs[255]; /* heights (ascent + descent) for all glyphs */
	int gbs[255]; /* bearings (ascent) for all glyphs */
	
	GLuint texName;    /* texture "name" - i.e. OpenGL identifier */

public:
	AFloat dpi_x, dpi_y;
	
	AFreeType() {
		FT_Init_FreeType(&library);
		dpi_x = 70; // FIXME: this is a trick to get narrow version of the font
		dpi_y = 85;
		texName = 0;
		twidth = theight = 0;
		face = 0;
		memset(adx, 0, sizeof(adx));
		memset(ghs, 0, sizeof(ghs));
		memset(gbs, 0, sizeof(gbs));
		buf = (char*) malloc(bsize = 512*1024);
	}
	
	bool setFont(const char *fn) {
		if (FT_New_Face(library, fn, 0, &face )) return false;
		return true;
	}
	
	bool setFontSize(AFloat size) {
		if (FT_Set_Char_Size(face, 0, size * 64.0, dpi_x, dpi_y)) return false;
		FT_GlyphSlot  slot = face->glyph;
		for (int i = 32; i < 128; i++)
			if (!FT_Load_Char(face, i, FT_LOAD_DEFAULT)) {
				adx[i] = slot->advance.x;
				ghs[i] = slot->metrics.height;
				gbs[i] = slot->metrics.horiBearingY;
			}
		return true;
	}
	
	ASize bbox(const char *txt) {
		/* fdebug("bbox('%s')", txt); */
		ASize box = AMkSize(0, 0);
#ifdef PRECISE_BBOX /* this would be the actual bbox, but that's not what we actually use ... */
		int xasc = 0, xdsc = 0;
		while (*txt) {
			int txo = (unsigned char) *txt;
			int asc = gbs[txo], dsc = ghs[txo] - asc;
			box.width += adx[txo] / 64.0;
			if (asc > xasc) xasc = asc;
			if (dsc > xdsc) xdsc = dsc;
			txt++;
		}
		box.height = xasc + xdsc;
#else /* for imprecise boxes we use fixed height to ensure all of them are on the same baseline and integer advance (which is how we render anyway) */
		while (*txt) {
			int txo = (unsigned char) *txt;
			box.width += (int) (adx[txo] / 64.0);
			txt++;
		}
		box.height = ghs[(int)'M'] + ghs[(int)'y'] - ghs[(int)'a']; /* ascent of M, descent of y */
#endif
		
		box.height /= 64.0;
		/* fdebug(" -> %g, %g (%g + %g)\n", box.width, box.height, (double) xasc / 64.0, (double) xdsc / 64.0); */
		return box;
	}
	
	/* Win32 doesn't support 1-component textures, so we can't use GL_LUMINANCE/GL_INTENSITY, unfortunately */
#ifdef WIN32
#define TX_PLANES 4
#define TX_FORMAT GL_RGBA
#else
#define TX_PLANES 1
#define TX_FORMAT GL_INTENSITY
#endif
	
	bool generateTexture(const char *txt) {
		// compute texture size
		if (!txt) return false;
		box = bbox(txt);
		int txw = 8, txh = 8;
		while (txw < box.width + 0.5) txw <<= 1;
		while (txh < box.height + 1.1) txh <<= 1;
		if (txw * txh * TX_PLANES > bsize) // texture bigger than the buffer - bail out for sanity
			return false;
		memset(buf, 0, txw * txh * TX_PLANES);
		
		FT_GlyphSlot  slot = face->glyph;
		FT_UInt       glyph_index;
		int           x = 0, y = 0, bline = ghs[(int)'M'] >> 6;
		while (*txt) {
			if (FT_Load_Char(face, *txt, FT_LOAD_RENDER)) continue;
			int left = slot->bitmap_left, shift = bline - slot->bitmap_top;
			FT_Bitmap bitmap = slot->bitmap;
			if (shift + bitmap.rows > txh) shift = txh - bitmap.rows;
			if (shift < 0) shift = 0;
			for (int i = 0; i < bitmap.rows; i++)
#if (TX_PLANES == 1)
				memcpy(buf + (i + shift) * txw + x + left + j, bitmap.buffer + i * bitmap.pitch, bitmap.width);
#else
				for (int j = 0; j < bitmap.width; j++) {
					char *bp = buf + TX_PLANES * ((i + shift) * txw + x + left + j);
					bp[0] = bp[1] = bp[2] = bp[3] = bitmap.buffer[i * bitmap.pitch + j];
				}
#endif
			x += slot->advance.x >> 6;
			txt++;
		}
		glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
		if (texName == 0) glGenTextures (1, &texName);
		GLC("glBiglBindTexture", glBindTexture (A_TEXTURE_TYPE, texName));
		glDisable(GL_DEPTH_TEST);
		if (1 /* txw != twidth || txh != theight */) {
			GLC("glTExEnv",glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));
			glTexParameterf(A_TEXTURE_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(A_TEXTURE_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			/* those are actually irrelevant since we make sure the texture doesn't exceed the area */
			glTexParameterf(A_TEXTURE_TYPE, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(A_TEXTURE_TYPE, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLC("glTexImage2D",glTexImage2D(A_TEXTURE_TYPE, 0, TX_FORMAT, txw, txh, 0, TX_FORMAT, GL_UNSIGNED_BYTE, buf));
		} else
			GLC("glTexSubImage2D",glTexSubImage2D(A_TEXTURE_TYPE, 0, 0, 0, txw, txh, TX_FORMAT, GL_UNSIGNED_BYTE, buf));
		glPopAttrib();
		twidth = txw;
		theight = txh;
		return true;
	}
	
	bool drawTexture(APoint point, APoint adj, AFloat rot, AColor text_color) {
		if (texName) {
			APoint ll, lr, ul, ur;
			
			ll = point;
			double width = box.width, height = box.height;
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
			
			// baseline shift - we use y-a
			double bls = ghs[(int)'y'] - ghs[(int)'a']; bls /= 64.0;
			ll.x += bls * sth;
			ll.y -= bls * cth;
			
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
			glEnable (A_TEXTURE_TYPE);  
			
			GLC("glBindTexture", glBindTexture (A_TEXTURE_TYPE, texName));
			glColor4f(text_color.r, text_color.g, text_color.b, text_color.a); // set text color
			glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); // draw upper left in world coordinates
			glVertex2f (ul.x, ul.y);
			
			ASize trs = box; trs.width /= twidth; trs.height /= theight;
			glTexCoord2f (0.0f, trs.height); // draw lower left in world coordinates
			glVertex2f (ll.x, ll.y);
			
			glTexCoord2f (trs.width, trs.height); // draw lower right in world coordinates
			glVertex2f (lr.x, lr.y);
			
			glTexCoord2f (trs.width, 0.0f); // draw upper right in world coordinates
			glVertex2f (ur.x, ur.y);
			glEnd ();
			
			glPopAttrib();
		}		
	}
};

#endif
