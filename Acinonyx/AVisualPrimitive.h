/*
 *  AVisualPrimitive.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

class AVisualPrimitive : public AVisual {
	AFloat c[4], f[4];
public:
	AVisualPrimitive() { c = {0.0, 0.0, 0.0, 1.0}; f = {0.0, 0.0, 0.0, 0.0}; }
	void drawColor(AFloat r, AFloat g, AFloat g, AFloat a) { c[0]=r; c[1]=g; c[2]=b; c[3]=a; }
	void fillColor(AFloat r, AFloat g, AFloat g, AFloat a) { f[0]=r; f[1]=g; f[2]=b; f[3]=a; }
	
	virtual void draw() = 0;
}

class ALinePrimitive : public AVisualPrimitive {
	APoint _p1, _p2;
public:
	ALinePrimitive(APoint p1, APoint p2) : _p1(p1), _p2(p2) {}

	virtual void draw() {
		if (c[3] > 0.0f) color(c);
		line(_p1.x, _p1.y, _p2.x, _p2.y);
	}
}

