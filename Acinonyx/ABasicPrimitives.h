/*
 *  ABasicPrimitives.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/25/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_BASIC_PRIMITIVES_H
#define A_BASIC_PRIMITIVES_H

#include "AVisualPrimitive.h"
#include "APlot.h"
#include "REngine.h"
#include "RValueHolder.h"

class ARCallbackPrimitive : public AVisualPrimitive, public RValueHolder {
public:
	ARCallbackPrimitive(APlot *plot) : AVisualPrimitive(plot), RValueHolder(R_NilValue) { OCLASS(ARCallbackPrimitive) }
	
	virtual void update() {
		if (_value != R_NilValue) 
			call_with_object(_value, this, "primitive");// call _value(self) in R
		AVisualPrimitive::update();
	}
};

// scaled primitives allows the primitive to scale itself according to the scale used in the plod
class AScaledPrimitive : public ARCallbackPrimitive {
public:
	AScaledPrimitive(APlot *plot) : ARCallbackPrimitive(plot) { OCLASS(AScaledPrimitive) }
	
	APoint transformPoint(APoint pt) {
		if (!_plot || _plot->scales() == 0) return pt;
		APoint p;
		AScale *xs = _plot->designatedScale(XScale);
		AScale *ys = _plot->designatedScale(YScale);
		p.x = xs ? xs->position(pt.x) : pt.x;
		p.y = ys ? ys->position(pt.y) : pt.y;
		return p;
	}
	
	void transformPoints(APoint *dst, APoint *src, vsize_t n) {
		if (!_plot || _plot->scales() == 0) {
			memcpy(dst, src, n * sizeof(APoint));
			return;
		}
		AScale *xs = _plot->designatedScale(XScale);
		AScale *ys = _plot->designatedScale(YScale);
		for (vsize_t i = 0; i < n; i++) {
			dst[i].x = xs ? xs->position(src[i].x) : src[i].x; 
			dst[i].y = ys ? ys->position(src[i].y) : src[i].y;
		}
	}
};

class ALinePrimitive : public AScaledPrimitive {
protected:
	APoint _p1, _p2;
public:
	ALinePrimitive(APlot *plot, APoint p1, APoint p2) : AScaledPrimitive(plot), _p1(p1), _p2(p2) { OCLASS(ALinePrimitive) }

	virtual void draw(ARenderer &renderer) {
		if (c.a > 0.0f) {
			renderer.color(c);
			APoint _s1 = transformPoint(_p1), _s2 = transformPoint(_p2);
			ALog("%s: render %g,%g %g,%g ---> %g,%g %g,%g", describe(), _p1.x, _p1.y, _p2.x, _p2.y, _s1.x, _s1.y, _s2.x, _s2.y);
			renderer.line(_s1.x, _s1.y, _s2.x, _s2.y);
		}
	}
	
	virtual bool intersects(ARect rect) {
		APoint _s1 = transformPoint(_p1), _s2 = transformPoint(_p2);
		// if the bounding box of the line doesn't intersect the rectangle then it cannot intersect the line
		if (rect.x > AMAX(_s1.x, _s2.x) || rect.y > AMAX(_s1.y, _s2.y) ||
			rect.x + rect.width < AMIN(_s1.x, _s2.x) || rect.y + rect.height < AMIN(_s1.y, _s2.y)) return false;
		// since the bounding box intersects, for straight line this implies intersection with the line
		if (_s1.x == _s2.x || _s1.y == _s2.y) return true;
		// ok, not a straight line, so we can calculate the distance of the rectangle points from the line along y
		AFloat m = (_s2.x - _s1.x) / (_s2.y - _s1.y);
		AFloat d1 = m * (rect.x - _s1.x) + _s1.y;
		if (rect.y <= d1 && rect.y + rect.height >= d1) return true;
		AFloat d2 = m * (rect.x + rect.width - _s1.x) + _s1.y;
		if (rect.y <= d2 && rect.y + rect.height >= d2) return true;
		// if neither side was hit then the only way it can be an intersection is if each virtual intersection point is on the other side
		return ((d1 < rect.y && d2 > rect.y + rect.height) || (d1 > rect.y && d2 < rect.y + rect.height));
	}
};

class ABarPrimitive : public AScaledPrimitive {
protected:
	ARect _r;
public:
	ABarPrimitive(APlot *plot, ARect rect) : AScaledPrimitive(plot), _r(rect) { OCLASS(ABarPrimitive) }
	
	virtual void draw(ARenderer &renderer) {
		ARect _s;
		APoint pt = transformPoint(AMkPoint(_r.x, _r.y));
		_s.x = pt.x; _s.y = pt.y;
		pt = transformPoint(AMkPoint(_r.x + _r.width, _r.y + _r.height));
		_s.width = pt.x - _s.x;
		_s.height = pt.y - _s.y;
		if (f.a) {
			renderer.color(f);
			renderer.rect(_r);
		}
		if (c.a) {
			renderer.color(c);
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

class APolygonPrimitive : public AScaledPrimitive {
protected:
	APoint *_pt, *_op;
	vsize_t _pts;
public:
	APolygonPrimitive(APlot *plot, APoint *p, vsize_t pts, bool copy=true) : AScaledPrimitive(plot), _pts(pts) {
		_op = copy ? (APoint*) memdup(p, pts * sizeof(APoint)) : p;
		AMEM(_op);
		_pt = (APoint*) malloc(pts * sizeof(APoint));
		AMEM(_pt);
		OCLASS(APolygonPrimitive)
	}
	
	virtual ~APolygonPrimitive() {
		free(_pt);
		free(_op);
		DCLASS(APolygonPrimitive)
	}
	
	virtual void draw(ARenderer &renderer) {
		transformPoints(_pt, _op, _pts);
		if (f.a) {
			renderer.color(f);
			renderer.polygon(_pt, _pts);
		}
		if (c.a) {
			renderer.color(c);
			renderer.polyline(_pt, _pts);
		}
	}
	
	void setPoints(double *x, double *y, vsize_t n) {
		if (n > _pts) {
			free(_pt); free(_op);
			_op = (APoint*) malloc(sizeof (APoint) * n);
			AMEM(_op);
			_pt = (APoint*) malloc(sizeof (APoint) * n);
			AMEM(_pt);
		}
		_pts = n;
		for (vsize_t i = 0; i < n; i++)
			{ _op[i].x = x[i]; _op[i].y = y[i]; }
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

class ATextPrimitive : public AScaledPrimitive {
protected:
	APoint _pt, _adj;
	AFloat _rot;
	char *_text;
public:
	ATextPrimitive(APlot *plot, APoint pt, const char *text, bool copy=true) : AScaledPrimitive(plot), _pt(pt), _rot(0.0) {
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
			APoint _st = transformPoint(_pt);
			renderer.color(c);
			renderer.text(_st, _text, _adj, _rot);
		}
	}
	
	void setRotation(AFloat rot) {
		_rot = rot;
	}
	
	void setAdjustment(APoint adj) {
		_adj = adj;
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

