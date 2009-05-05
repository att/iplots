/*
 *  AScatterPlot.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_SCATTERPLOT_H_
#define A_SCATTERPLOT_H_

#include "APlot.h"
#include "AAxis.h"
#include "AMarker.h"

class AScatterPlot : public APlot {
	AMarker *marker;
	AXAxis *xa;
	AYAxis *ya;
	AFloat mLeft, mTop, mBottom, mRight, ptSize, ptAlpha;

public:
	AScatterPlot(AContainer *parent, ARect frame, int flags, ADataVector *x, ADataVector *y) : APlot(parent, frame, flags) {
		mLeft = 30.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 10.0f;
		ptSize = 15.0;
		ptAlpha = 0.6;
		// printf("AScatterPlot frame = (%f,%f - %f x %f)\n", _frame.x, _frame.y, _frame.width, _frame.height);
		nScales = 2;
		marker = x->marker();
		if (!marker) marker = y->marker();
		if (marker) marker->retain();
		scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		scales[0] = new AScale(x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range());
		scales[1] = new AScale(y, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), y->range());
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, scales[1]);
		add(*ya);
		OCLASS(AScatterPlot)
	}
	
	virtual ~AScatterPlot() {
		xa->release();
		ya->release();
		DCLASS(AScatterPlot)
	}

	// this is subtle and holefully we'll get rid of this, but the constructor may be called with NULL parent so it has no windows and axes are not registered in the hierarchy ...
	virtual void setWindow(AWindow *win) {
		APlot::setWindow(win);
		xa->setWindow(win);
		ya->setWindow(win);
	}
	
	void update() {
		scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));
	}
	
	virtual bool performSelection(ARect where, int type) {
		if (!marker) return false;
		AFloat *lx = scales[0]->locations();
		AFloat *ly = scales[1]->locations();
		vsize_t nPts = scales[0]->data()->length();
		marker->begin();
		if (type == SEL_REPLACE)
			marker->deselectAll();
		if (type == SEL_XOR) {
			for (vsize_t i = 0; i < nPts; i++)
				if (ARectContains(where, AMkPoint(lx[i], ly[i])))
					marker->selectXOR(i);
		} else if (type == SEL_NOT) {
			for (vsize_t i = 0; i < nPts; i++)
				if (ARectContains(where, AMkPoint(lx[i], ly[i])))
					marker->deselect(i);
		} else if (type == SEL_AND) {
			for (vsize_t i = 0; i < nPts; i++)
				if (!ARectContains(where, AMkPoint(lx[i], ly[i])))
					marker->deselect(i);
		} else {
			for (vsize_t i = 0; i < nPts; i++)
				if (ARectContains(where, AMkPoint(lx[i], ly[i])))
					marker->select(i);
		}
		marker->end();
		redraw();
		return true;
	}

	virtual void draw() {
		printf("%s: draw\n", describe());
		xa->draw();
		ya->draw();
		
		glPointSize(ptSize);
		color(AMkColor(0.0,0.0,0.0,ptAlpha));
		AFloat *lx = scales[0]->locations();
		AFloat *ly = scales[1]->locations();
		points(lx, ly, scales[0]->data()->length());
		if (marker) {
			mark_t *ms = marker->rawMarks();
			vsize_t n = marker->length();
			color(AMkColor(1.0,0.0,0.0,1.0));
			for (vsize_t i = 0; i < n; i++)
				if (M_TRANS(ms[i]))
					point(lx[i], ly[i]);
		}
		color(AMkColor(0.0,1.0,0.0,0.5));
		rect(0.0,0.0,10.0,10.0);
		
		APlot::draw();
	}
};

#endif
