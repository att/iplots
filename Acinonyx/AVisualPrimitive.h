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
	
	virtual void update() { };
	virtual char *query(int level) { return NULL; }
	virtual bool containsPoint(APoint pt) { return false; }
	virtual bool intersects(ARect rect) { return false; }
	virtual bool select(AMarker *marker, int type) { return false; }
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

static int isLeft( APoint P0, APoint P1, APoint P2 )
{
    return ( (P1.x - P0.x) * (P2.y - P0.y)
            - (P2.x - P0.x) * (P1.y - P0.y) );
}

class APolygonPrimitive : public AVisualPrimitive {
protected:
	APoint *_pt;
	vsize_t _pts;
public:
	APolygonPrimitive(APoint *p, vsize_t pts, bool copy=true) : _pts(pts) {
		_pt = copy ? (APoint*) memdup(p, pts * sizeof(APoint)) : p;
		AMEM(_pt);
		OCLASS(APolygonPrimitive)
	}
	
	virtual ~APolygonPrimitive() {
		free(_pt);
		DCLASS(APolygonPrimitive)
	}
	
	virtual void draw(ARenderer &renderer) {
		if (f.a) {
			renderer.color(f);
			renderer.polygon(_pt, _pts);
		}
		if (c.a) {
			renderer.color(c);
			renderer.polyline(_pt, _pts);
		}
	}
	
//	virtual bool intersects(ARect rect) {
//	}
		
	virtual bool containsPoint(APoint P) {
		if (_pts < 2) return false;
		// use the winding rule to determine whether the point is contained
		int wn = 0;
		for (vsize_t i = 0; i < _pts - 1; i++) {
			if (_pt[i].y <= P.y) {
				if (_pt[i + 1].y > P.y)
					if (isLeft(_pt[i], _pt[i + 1], P) > 0)
						++wn;
			} else {
				if (_pt[i + 1].y <= P.y)
					if (isLeft(_pt[i], _pt[i + 1], P) < 0)
						--wn;
			}
		}
		// finish off with the closing line point[n - 1], point[0]
		vsize_t i = _pts - 1;
		if (_pt[i].y <= P.y) {
			if (_pt[0].y > P.y)
				if (isLeft(_pt[i], _pt[0], P) > 0)
					++wn;
		} else {
			if (_pt[0].y <= P.y)
				if (isLeft(_pt[i], _pt[0], P) < 0)
					--wn;
		}
		return (wn != 0);
		/*
		bool c = 0;
		for (vsize_t i = 0, j = _pts - 1; i < _pts; j = i++)
			if ((((_pt[i].y <= pt.y) && (pt.y < _pt[j].y)) ||
				 ((_pt[j].y <= pt.y) && (pt.y < _pt[i].y))) &&
				(pt.x < (_pt[j].x - _pt[i].x) * (pt.y - _pt[i].y) / (_pt[j].y - _pt[i].y) + _pt[i].x))
				c = !c;
		return c;
		 */
	}
	
#undef PNEXT
	
};

class ATextPrimitive : public AVisualPrimitive {
protected:
	APoint _pt, _adj;
	char *_text;
public:
	ATextPrimitive(APoint pt, const char *text, bool copy=true) : _pt(pt) {
		_text = copy ? strdup(text) : (char*) text;
		_adj = AMkPoint(0.5, 0.5);
		OCLASS(ATextPrimitive)
	}
	
	virtual ~ATextPrimitive() {
		free(_text);
		DCLASS(ATextPrimitive)
	}
	
	virtual void draw(ARenderer &renderer) {
		if (c.a) {
			renderer.color(c);
			renderer.text(_pt, _adj, _text);
		}
	}
	
	/*
	virtual bool intersects(ARect rect) {
		return ARectsIntersect(rect, _r);
	}
	
	virtual bool containsPoint(APoint pt) {
		return ARectContains(_r, pt);
	}*/
};

#endif
