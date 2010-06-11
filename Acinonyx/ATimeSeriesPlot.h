/*
 *  TimeSeries.h
 *  Acinonyx
 *
 *  Created by Anushka Anand on 6/8/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_TIMESERIESPLOT_H_
#define A_TIMESERIESPLOT_H_

#include "APlot.h"
#include "AAxis.h"
#include "AMarker.h"
#include "AStatVisual.h"

class ATimeSeriesPlot : public APlot {
protected:
	AXAxis *xa;
	AYAxis *ya;
	vsize_t lines;
	ADataVector *datax, *datay;
	AIntVector *dataTS;
public:
	ATimeSeriesPlot(AContainer *parent, ARect frame, int flags, ADataVector *x, ADataVector *y) : APlot(parent, frame, flags) {
		mLeft = 20.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 10.0f;
		ptSize = 2.0;
		ptAlpha = 0.7;
		printf("ATimeSeriesPlot frame = (%f,%f - %f x %f)\n", _frame.x, _frame.y, _frame.width, _frame.height);
		printf("ATimeSeriesPlot xrange = (%f,%f) & yrange=(%f,%f) \n", x->range().begin,x->range().length,y->range().begin,y->range().length);
		nScales = 2;
		
		marker = x->marker();
		if (!marker) marker = y->marker();
		if (marker) {
			marker->retain();
			marker->add(this);
		}

		vsize_t i = 0, m = 0, npts = x->length();
		int *membership = (int*)malloc(npts * sizeof(int));
		double prev = x->doubleAt(i);
		while(i < npts){
			if (x->doubleAt(i) < prev)
				m++;
			membership[i] =  m;

			prev = x->doubleAt(i);
			i++;
		}
		lines = m + 1;
		
		dataTS = new AIntVector(membership ,npts);
		free(membership); membership = NULL;
		
		_scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		_scales[0] = new AScale(datax = x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range());
		_scales[1] = new AScale(datay = y, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), y->range());
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, _scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, _scales[1]);
		add(*ya);

		createPrimitives();
		// add home zoom entry
		AZoomEntryBiVar *ze = new AZoomEntryBiVar(_scales[0]->dataRange(), _scales[1]->dataRange());
		zoomStack->push(ze);
		ze->release();
		OCLASS(ATimeSeriesPlot)
	}
	
	
	virtual ~ATimeSeriesPlot() {
		xa->release();
		ya->release();
		DCLASS(ATimeSeriesPlot)
	}
	
	APointVector *createPolyLine(AFloat *lx, AFloat *ly, group_t group, vsize_t len){
		vsize_t ng = 0;
		APoint *pts =  (APoint*)malloc(len * sizeof(APoint));
		const int* ms = dataTS->asInts();
		for(int i = 0; i < len; i++){
			if (ms[i] == (int)group){
				pts[ng] = AMkPoint(lx[i], ly[i]);
				ng++;
			}
		}
		APointVector *pvector = new APointVector(pts, ng);
		free(pts); pts = NULL;
		return pvector;
	}
	
	void createPrimitives() {
		AFloat *lx = _scales[0]->locations();
		AFloat *ly = _scales[1]->locations();
		vsize_t len = _scales[0]->data()->length();
		if (pps && pps->length() != lines) {
			pps->release();
			pps = NULL;
		}
		if (!pps)
			pps = new ASettableObjectVector(lines);
		for(vsize_t i = 0; i < lines; i++) {
			group_t group = (group_t) i;
			APointVector *pts = createPolyLine(lx, ly, (group_t)i, len);
			APolyLineStatVisual *pl = new APolyLineStatVisual(this, pts, marker, 
															  (vsize_t*)dataTS->asInts(), len, group, false, false);
			pl->setDrawAttributes(ptSize, ptAlpha);
			pts->release();	//we passed ownership to polylinestatvisual
			((ASettableObjectVector*)pps)->replaceObjectAt(i, pl);
			pl->release(); // we passed the ownership to pps
		}
	}
	
	void update() {
		_scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		_scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));
		
		if (!pps)
			createPrimitives();
		else {
			AFloat *lx = _scales[0]->locations();
			AFloat *ly = _scales[1]->locations();
			vsize_t i = 0, lines = pps->length();
			while (i < lines) {
				APolyLineStatVisual *pl = (APolyLineStatVisual*) pps->objectAt(i);
				APointVector* pts = createPolyLine(lx, ly, (group_t) i, _scales[0]->data()->length());
				pl->setPolyLine(pts);
				pl->setDrawAttributes(ptSize, ptAlpha);
				pts->release();
				i++;
			}
		}
		APlot::update();
	}
	
	virtual bool performZoom(ARect where) {
		// printf("%s: perform selection: (%g,%g - %g,%g)\n", describe(), where.x, where.y, where.width, where.height);
		if (where.width < 3.0 && where.height < 3.0) { // consider this a single click = zoom out
			bool homeZoom = zoomStack->isLast();
			AZoomEntryBiVar *ze = (AZoomEntryBiVar*) ( homeZoom ? zoomStack->peek() : zoomStack->pop());
			if (!ze) return false;
			_scales[0]->setDataRange(ze->range(0));
			_scales[1]->setDataRange(ze->range(1));
			if (!homeZoom) ze->release();
			setRedrawLayer(LAYER_ROOT); 
			update();
			redraw();
			return true;
		} else {
			// convert where to data ranges
			AZoomEntryBiVar *ze = new AZoomEntryBiVar(_scales[0]->toDataRange(AMkRange(where.x,where.width)), _scales[1]->toDataRange(AMkRange(where.y,where.height)));
			zoomStack->push(ze);
			_scales[0]->setDataRange(ze->range(0));
			_scales[1]->setDataRange(ze->range(1));
			ze->release();
			setRedrawLayer(LAYER_ROOT);
			update();
 			redraw();
			return true;
		}
		return false;
	}
	
	virtual bool keyDown(AEvent e) {
		switch (e.key) {
			case KEY_DOWN: if (ptSize > 1.0) { ptSize -= 1.0; setRedrawLayer(LAYER_ROOT); update(); redraw(); }; break;
			case KEY_UP: ptSize += 1.0; setRedrawLayer(LAYER_ROOT); update();redraw(); break;
			case KEY_LEFT: if (ptAlpha > 0.02) { ptAlpha -= (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha < 0.02) ptAlpha = 0.02; setRedrawLayer(LAYER_ROOT); update();redraw(); }; break;
			case KEY_RIGHT: if (ptAlpha < 0.99) { ptAlpha += (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha > 1.0) ptAlpha = 1.0; setRedrawLayer(LAYER_ROOT); update(); redraw(); } break;
			default:
				return false;
		}
		return true;
	}
	
	virtual void queryAt(APoint pt, int level) {
		APlot::queryAt(pt, level);
		if (level == 0) inQuery = true; // force inQuery since we'll use the orientation query that we plot ourself
	}
	
	virtual void queryOff() {
		if (inQuery && _query->isHidden())
			redraw();
		APlot::queryOff();
	}
	

	virtual char *describe() {
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d]>", this, refcount, _objectSerial, _className, _classSize);
#else
		snprintf(desc_buf, 512, "<ATimeSeriesPlot %p>", this);
#endif
		return desc_buf;
	}
	
	virtual const char *caption() {
		return value_printf("TimeSeries Plot of %s",
							(datay->name()) ? datay->name() : "<tmp>");
	}
};

#endif
