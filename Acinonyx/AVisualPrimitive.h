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
	void drawColor(AColor aColor) { c = aColor; }
	void fillColor(AFloat r, AFloat g, AFloat b, AFloat a) { f = AMkColor(r,g,b,a); }
	void fillColor(AColor aColor) { f = aColor; }
	
	virtual void draw(ARenderer &renderer) = 0;
	
	virtual char *query(int level) { return NULL; }
	virtual bool containsPoint(APoint pt) { return false; }
	virtual bool intersects(ARect rect) { return false; }
};

class ALinePrimitive : public AVisualPrimitive {
protected:
	APoint _p1, _p2;
public:
	ALinePrimitive(APoint p1, APoint p2) : _p1(p1), _p2(p2) { OCLASS(ALinePrimitive) }

	virtual void draw(ARenderer &renderer) {
		if (c.a > 0.0f) {
			renderer.color((AFloat*)(&c));
			renderer.line(_p1.x, _p1.y, _p2.x, _p2.y);
		}
	}
	
	virtual bool intersects(ARect rect) {
		// if the bounding box of the line doesn't intersect the rectangle then it cannot intersect the line
		if (rect.x > AMAX(_p1.x, _p2.x) || rect.y > AMAX(_p1.y, _p2.y) ||
			rect.x + rect.width < AMIN(_p1.x, _p2.x) || rect.y + rect.height < AMIN(_p1.y, _p2.y)) return false;
		// since the bounding box intersects, for straight line this implies intersection with the line
		if (_p1.x == _p2.x || _p1.y == _p2.y) return true;
		// ok, not a straight line, so we can calculate the distance of the rectangle points from the line along y
		AFloat m = (_p2.x - _p1.x) / (_p2.y - _p1.y);
		AFloat d1 = m * (rect.x - _p1.x) + _p1.y;
		if (rect.y <= d1 && rect.y + rect.height >= d1) return true;
		AFloat d2 = m * (rect.x + rect.width - _p1.x) + _p1.y;
		if (rect.y <= d2 && rect.y + rect.height >= d2) return true;
		// if neither side was hit then the only way it can be an intersection is if each virtual intersection point is on the other side
		return ((d1 < rect.y && d2 > rect.y + rect.height) || (d1 > rect.y && d2 < rect.y + rect.height));
	}
};

class ABarPrimitive : public AVisualPrimitive {
protected:
	ARect _r;
public:
	ABarPrimitive(ARect rect) : _r(rect) { OCLASS(ABarPrimitive) }
	
	virtual void draw(ARenderer &renderer) {
		if (f.a) {
			renderer.color((AFloat*)(&f));
			renderer.rect(_r);
		}
		if (c.a) {
			renderer.color((AFloat*)(&c));
			renderer.rectO(_r);
		}
	}
	
	virtual bool intersects(ARect rect) {
		return ARectsIntersect(rect, _r);
	}
	
	virtual bool containsPoint(APoint pt) {
		return ARectContains(_r, pt);
	}
};
