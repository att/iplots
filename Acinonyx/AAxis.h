/*
 *  AAxis.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "AVisual.h"

class AAxis : public AVisual {
protected:
	AScale *_scale;
public:
	AAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AVisual(parent, frame, flags), _scale(scale) { scale->retain(); OCLASS(AAxis) }
	virtual ~AAxis() {
		if (_scale) _scale->release();
		DCLASS(AAxis)
	}
};

class AXAxis : public AAxis {
public:
	AXAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		rect(_frame);
		color(0.0, 0.0, 1.0, 0.3);
		rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x, _frame.y + _frame.height / 2, _frame.x, _frame.y + _frame.height);
		line(_frame.x + _frame.width, _frame.y + _frame.height / 2, _frame.x + _frame.width, _frame.y + _frame.height);
		
		// show outer labels
		char buf[64];
		ADataRange dr = _scale->dataRange();
		snprintf(buf, 64, "%g", dr.begin);
		text(AMkPoint(_frame.x, _frame.y + _frame.height / 2), AMkPoint(0.0, 0.5), buf);
		snprintf(buf, 64, "%g", dr.begin + dr.length);
		text(AMkPoint(_frame.x + _frame.width, _frame.y + _frame.height / 2), AMkPoint(1.0, 0.5), buf);
	}
};

class ADiscreteXAxis : public AAxis {
public:
	ADiscreteXAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		rect(_frame);
		color(0.0, 0.0, 1.0, 0.3);
		rectO(_frame);

		ADataRange r = _scale->dataRange();
		vsize_t n = (vsize_t) r.length;
		vsize_t i = 0;
		APoint centerAdj = AMkPoint(0.5, 0.5);
		color(pointColor);
		while (i <= n) {
			AFloat c = _scale->discreteCenter(i++);
			line(c, _frame.y + _frame.height, c, _frame.y + _frame.height * 0.8);
			text(AMkPoint(c, _frame.y + _frame.height * 0.4), centerAdj, "X");
		}
		/*
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x, _frame.y + _frame.height / 2, _frame.x, _frame.y + _frame.height);
		line(_frame.x + _frame.width, _frame.y + _frame.height / 2, _frame.x + _frame.width, _frame.y + _frame.height);
		
		// show outer labels
		char buf[64];
		ADataRange dr = _scale->dataRange();
		snprintf(buf, 64, "%g", dr.begin);
		text(AMkPoint(_frame.x, _frame.y + _frame.height / 2), AMkPoint(0.0, 0.5), buf);
		snprintf(buf, 64, "%g", dr.begin + dr.length);
		text(AMkPoint(_frame.x + _frame.width, _frame.y + _frame.height / 2), AMkPoint(1.0, 0.5), buf);
		 */
	}
};


class AYAxis : public AAxis {
public:
	AYAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		rect(_frame);
		color(1.0, 0.0, 0.0, 0.3);
		rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x + _frame.width, _frame.y, _frame.x + _frame.width, _frame.y + _frame.height);
	}
};
