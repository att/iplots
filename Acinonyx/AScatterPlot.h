/*
 *  AScatterPlot.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_SCATTERPLOT_H_
#define A_SCATTERPLOT_H_

#include "APlot.h"
#include "AAxis.h"

class AScatterPlot : public APlot {
	AXAxis *xa;
	AYAxis *ya;
	APoint *pts;
	int nPts;
public:
	AScatterPlot(AContainer *parent, ARect frame, int flags, AVector *x, AVector *y) : APlot(parent, frame, flags) {
		AFloat mLeft = 30.0f, mTop = 10.0f, mBottom = 20.0f, mRight = 10.0f;
		// printf("AScatterPlot frame = (%f,%f - %f x %f)\n", _frame.x, _frame.y, _frame.width, _frame.height);
		nScales = 2;
		scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		scales[0] = new AScale(x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range());
		scales[1] = new AScale(y, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), y->range());
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, scales[1]);
		add(*ya);
		nPts = x->length();
		if (nPts < y->length()) nPts = y->length();
		pts = (APoint*) malloc(sizeof(APoint) * nPts);
		AMEM(pts);
		OCLASS(AScatterPlot)
	}
	
	virtual ~AScatterPlot() {
		xa->release();
		ya->release();
		free(pts);
		DCLASS(AScatterPlot)
	}

	void update() {
		//const double *xv = scales[0]->data()->asDoubles();
		//const double *yv = scales[1]->data()->asDoubles();
	}
	
	virtual void draw() {
		xa->draw();
		ya->draw();
		
		glPointSize(5);
		color(AMkColor(0.0,0.0,0.0,0.6));
		points(scales[0]->locations(), scales[1]->locations(), scales[0]->data()->length());
		color(AMkColor(1.0,0.0,0.0,0.5));
		rect(0.0,0.0,10.0,10.0);
		
	}
};

#endif
