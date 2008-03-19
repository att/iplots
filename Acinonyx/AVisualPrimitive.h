/*
 *  AVisualPrimitive.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ARenderer.h"

class AVisualPrimitive : public AObject {
protected:
	AColor c, f;
public:
	AVisualPrimitive() { c = AMkColor(0.0, 0.0, 0.0, 1.0); f = AMkColor(0.0, 0.0, 0.0, 0.0); OCLASS(AVisualPrimitive) }
	void drawColor(AFloat r, AFloat g, AFloat b, AFloat a) { c = AMkColor(r,g,b,a); }
	void fillColor(AFloat r, AFloat g, AFloat b, AFloat a) { f = AMkColor(r,g,b,a); }
	
	virtual void draw(ARenderer &renderer) = 0;
};

class ALinePrimitive : public AVisualPrimitive {
	APoint _p1, _p2;
public:
	ALinePrimitive(APoint p1, APoint p2) : _p1(p1), _p2(p2) {}

	virtual void draw(ARenderer &renderer) {
		if (c.a > 0.0f) renderer.color((AFloat*)(&c));
		renderer.line(_p1.x, _p1.y, _p2.x, _p2.y);
	}
};

