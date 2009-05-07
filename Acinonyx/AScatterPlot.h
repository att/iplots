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
		ptSize = 5.0;
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
		// add home zoom entry
		AZoomEntryBiVar *ze = new AZoomEntryBiVar(scales[0]->dataRange(), scales[1]->dataRange());
		zoomStack->push(ze);
		ze->release();
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
	
	virtual bool performZoom(ARect where) {
		// printf("%s: perform selection: (%g,%g - %g,%g)\n", describe(), where.x, where.y, where.width, where.height);
		if (where.width < 3.0 && where.height < 3.0) { // consider this a single click = zoom out
			bool homeZoom = zoomStack->isLast();
			AZoomEntryBiVar *ze = (AZoomEntryBiVar*) ( homeZoom ? zoomStack->peek() : zoomStack->pop());
			if (!ze) return false;
			scales[0]->setDataRange(ze->range(0));
			scales[1]->setDataRange(ze->range(1));
			if (!homeZoom) ze->release();
			redraw();
			return true;
		} else {
			// convert where to data ranges
			AZoomEntryBiVar *ze = new AZoomEntryBiVar(scales[0]->toDataRange(AMkRange(where.x,where.width)), scales[1]->toDataRange(AMkRange(where.y,where.height)));
			zoomStack->push(ze);
			scales[0]->setDataRange(ze->range(0));
			scales[1]->setDataRange(ze->range(1));
			ze->release();
			redraw();
			return true;
		}
		return false;
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

	virtual bool keyDown(AEvent e) {
		switch (e.key) {
			case KEY_DOWN: if (ptSize > 1.0) { ptSize -= 1.0; redraw(); }; break;
			case KEY_UP: ptSize += 1.0; redraw(); break;
			case KEY_LEFT: if (ptAlpha > 0.02) { ptAlpha -= (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha < 0.02) ptAlpha = 0.02; redraw(); }; break;
			case KEY_RIGHT: if (ptAlpha < 0.99) { ptAlpha += (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha > 1.0) ptAlpha = 1.0; redraw(); } break;
			default:
				return false;
		}
		return true;
	}
	
	virtual void draw() {
		printf("%s: draw\n", describe());
		//xa->draw();
		//ya->draw();
		
		//clip(AMkRect(_frame.x + mLeft, _frame.y + mBottom, _frame.width - mLeft - mRight, _frame.height - mTop - mBottom));
		clip(_frame);
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
		clipOff();
		color(AMkColor(backgroundColor.r,backgroundColor.g,backgroundColor.b,0.5));
		rect(0.0,0.0,mLeft,mBottom);
		// draw children - in our case axes etc.
		APlot::draw();
	}

	virtual char *describe() {
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d]>", this, refcount, _objectSerial, _className, _classSize);
#else
		snprintf(desc_buf, 512, "<AScatterPlot %p>", this);
#endif
		return desc_buf;
	}
	
};

#endif
