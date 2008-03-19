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
	AScale *_scale;
public:
	AAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AVisual(parent, frame, flags), _scale(scale) { }
};

class AXAxis : public AAxis {
public:
	AXAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(1.0, 0.0, 0.0, 0.3);
		rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x + _frame.width, _frame.y, _frame.x + _frame.width, _frame.y + _frame.height);
	}
};

class AYAxis : public AAxis {
public:
	AYAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(1.0, 0.0, 0.0, 0.3);
		rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
	}
};
