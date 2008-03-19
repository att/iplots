/*
 *  AScatterPlot.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "APlot.h"
#include "AAxis.h"

class AScatterPlot : public AVisual, public APlot {
	AXAxis *xa;
	AYAxis *ya;
	APoint *pts;
	int nPts;
public:
	AScatterPlot(AContainer *parent, ARect frame, int flags, AVector *x, AVector *y) : AVisual(parent, frame, flags) {
		AFloat mLeft = 20.0f, mTop = 10.0f, mBottom = 20.0f, mRight = 10.0f;		
		APlot(
			  new AScale(x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range()),
			  new AScale(y, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), y->range()));
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT, scales[0]);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, scales[1]);
		nPts = x->length();
		if (nPts < y->length()) nPts = y->length();
		pts = (APoint*) malloc(sizeof(APoint) * nPts);
		AMEM(pts);
	}
	
	void update() {
		double *xv = scale[0]->data()->asDoubles();
		double *yv = scale[1]->data()->asDoubles();
	}
	
	virtual void draw() {
		xa->draw();
		ya->draw();
		
		glPointSize(2.5);
		points(scale[0]->data()->asFloats()
	}
}