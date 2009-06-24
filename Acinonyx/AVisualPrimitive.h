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

class APlot;

class AVisualPrimitive : public AObject {
protected:
	AColor c, f;
	APlot *_plot;
public:
	AVisualPrimitive(APlot *plot) : _plot(plot) { c = AMkColor(0.0, 0.0, 0.0, 1.0); f = AMkColor(0.0, 0.0, 0.0, 0.0); OCLASS(AVisualPrimitive) }
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
	
	APlot *plot() { return _plot; }
	void setPlot(APlot *aPlot) { _plot = aPlot; } // to be use inside APlot only!
	
	virtual void draw(ARenderer &renderer) = 0;
	
	virtual void update() { };
	virtual char *query(int level) { return NULL; }
	virtual bool containsPoint(APoint pt) { return false; }
	virtual bool intersects(ARect rect) { return false; }
	virtual bool select(AMarker *marker, int type) { return false; }	
};

#endif
