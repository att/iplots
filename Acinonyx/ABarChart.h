/*
 *  ABarChart.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/12/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_BAR_CHART_H
#define A_BAR_CHART_H

#include "AStatVisual.h"
#include "APlot.h"

class ABarChart : public APlot {
protected:
	ADiscreteXAxis *xa;
	AYAxis *ya;
	AFloat mLeft, mTop, mBottom, mRight, ptSize, ptAlpha;
	vsize_t bars;
	bool spines;
	
public:
	ABarChart(AContainer *parent, ARect frame, int flags, AFactorVector *x) : APlot(parent, frame, flags) {
		mLeft = 30.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 10.0f;

		nScales = 2;
		marker = x->marker();
		if (marker) {
			marker->retain();
			marker->add(this);
		}
		bars = 0;
		scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		AUnivarTable *tab = x->table();
		scales[0] = new AScale(x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), bars = x->levels());
		scales[1] = new AScale(NULL, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), tab->maxCount());
		xa = new ADiscreteXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, scales[1]);
		add(*ya);
		createPrimitives();
		OCLASS(ABarChart)
	}
	
	virtual ~ABarChart() {
		xa->release();
		ya->release();
		DCLASS(ABarChart);
	}

	ARect rectForBar(AUnivarTable *tab, group_t group) {
		ARange xr = scales[0]->discreteRange((int)group - 1);
		if (!spines && xr.length > 40.0) { xr.begin = xr.begin + xr.length / 2 - 20.0; xr.length = 40.0; }
		if (xr.length > 5.0) { xr.begin += xr.length * 0.1; xr.length *= 0.8; }
		if (spines) {
			double prop = tab->maxCount() ? (((double) tab->count(group)) / ((double) tab->maxCount())) : 0;
			xr.begin += (1.0 - prop) / 2 * xr.length;
			xr.length *= prop;
			ARange gr = scales[1]->range();
			return AMkRect(xr.begin, gr.begin, xr.length, gr.length);
		} else
			return AMkRect(xr.begin, scales[1]->position(0), xr.length, scales[1]->position(tab->count(group)) - scales[1]->position(0));
	}
	
	void createPrimitives() {
		AFactorVector *data = (AFactorVector*) scales[0]->data();
		AUnivarTable *tab = data->table();
		vps->removeAll();
		for(vsize_t i = 0; i < bars; i++) {
			group_t group = (group_t) i + 1;
			ABarStatVisual *b = new ABarStatVisual(rectForBar(tab, group), Up, marker, (vsize_t*) data->asInts(), data->length(), group, false);
			vps->addObject(b);
			b->release(); // we passed the ownership to vps
		}
	}
	
	void update() {
		scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));

		AFactorVector *data = (AFactorVector*) scales[0]->data();
		AUnivarTable *tab = data->table();
		vsize_t i = 0, bars = vps->length();
		while (i < bars) {
			ABarStatVisual *b = (ABarStatVisual*) vps->objectAt(i++);
			b->setRect(rectForBar(tab, (group_t) i));			
		}
	}
	
	virtual bool keyDown(AEvent e) {
		switch (e.key) {
			case 1: spines = !spines; update(); redraw(); break;
			default:
				return false;
		}
		return true;
	}
};

#endif
