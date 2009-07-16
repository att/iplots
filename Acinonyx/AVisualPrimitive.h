/*
 *  AVisualPrimitive.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_VISUAL_PRIMITIVE_H
#define A_VISUAL_PRIMITIVE_H

#include "ARenderer.h"
#include "AMarker.h"
#include "AQuery.h"

class APlot;

#define FVP_HIDDEN 0x01

class AVisualPrimitive : public AObject {
protected:
	AColor c, f;
	APlot *_plot;
	unsigned int flags;
public:
	AVisualPrimitive(APlot *plot) : _plot(plot), flags(0) { c = AMkColor(0.0, 0.0, 0.0, 1.0); f = AMkColor(0.0, 0.0, 0.0, 0.0); OCLASS(AVisualPrimitive) }
	void drawColor(AFloat r, AFloat g, AFloat b, AFloat a) { c = AMkColor(r,g,b,a); }
	void drawColor(AColor aColor) { c = aColor; }
	AColor drawColor() { return c; }
	void fillColor(AFloat r, AFloat g, AFloat b, AFloat a) { f = AMkColor(r,g,b,a); }
	void fillColor(AColor aColor) { f = aColor; }
	AColor fillColor() { return f; }
	void setAlpha(AFloat a) {
		if (a < 0.01) a = 0.01;
		if (c.a) c.a = a;
		if (f.a) f.a = a;
	}
	
	void setHidden(bool hf) { if (hf) flags |= FVP_HIDDEN; else flags &= ~FVP_HIDDEN; }
	bool hidden() { return (flags & FVP_HIDDEN) ? true : false; }
	
	APlot *plot() { return _plot; }
	void setPlot(APlot *aPlot) { _plot = aPlot; } // to be use inside APlot only!
	
	virtual void draw(ARenderer &renderer) = 0;
	
	virtual void update() { };
	virtual void query(AQuery *query, int level) { }
	virtual bool containsPoint(APoint pt) { return false; }
	virtual bool intersects(ARect rect) { return false; }
	virtual bool select(AMarker *marker, int type) { return false; }	
};

#endif
