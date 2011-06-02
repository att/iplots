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
	AIntVector *groupIDs;
	vsize_t **groupMems;
	vsize_t *groupCounts;
	AFactorVector *groupNames;
	
public:
	ATimeSeriesPlot(AContainer *parent, ARect frame, int flags, ADataVector *x, ADataVector *y, AFactorVector *names) : APlot(parent, frame, flags) {
		groupNames = names;
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

		_scales = (AScale**) AAlloc(sizeof(AScale*) * nScales);
		AMEM(_scales);
		_scales[0] = new AScale(datax = x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range());
		_scales[1] = new AScale(datay = y, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), y->range());
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_DEFAULT|AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_XSPRING, _scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_DEFAULT|AVF_FIX_LEFT|AVF_FIX_WIDTH|AVF_YSPRING, _scales[1]);
		add(*ya);

		setGroups();
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
		AFree(groupCounts); groupCounts = NULL;
		AFree(groupMems); groupMems = NULL;
		DCLASS(ATimeSeriesPlot)
	}
	
	void setGroups(){
		vsize_t i = 0, m = 0, npts = datax->length();
		int *membership = (int*)AAlloc(npts * sizeof(int));
		AMEM(membership);
		double prev = datax->doubleAt(i);
		while(i < npts){
			if (datax->doubleAt(i) < prev){
				m++;
			}
			membership[i] =  m;
			prev = datax->doubleAt(i);
			i++;
		}
		lines = m + 1;
		groupCounts = (vsize_t*)AZAlloc(lines, sizeof(vsize_t));
		AMEM(groupCounts);
		vsize_t* groupCounters = (vsize_t*)AZAlloc(lines, sizeof(vsize_t));
		AMEM(groupCounters);
		for(i = 0, m = 0; i < npts; i++)
			groupCounts[membership[i]]++;
		groupMems = (vsize_t**)AAlloc(lines * sizeof(vsize_t*));
		AMEM(groupMems);
		for(i = 0; i < lines; i++) {
			groupMems[i] = (vsize_t*)AAlloc(groupCounts[i] * sizeof(vsize_t));
			AMEM(groupMems[i]);
		}
		for(i = 0; i < npts; i++)
			groupMems[membership[i]][groupCounters[membership[i]]++] = i;
		AFree(groupCounters); groupCounters = NULL;
		groupIDs = new AIntVector(membership ,npts);
		AFree(membership); membership = NULL;
	}
	
	void createPrimitives() {
		vsize_t len = _scales[0]->data()->length();
		if (pps && pps->length() != lines) {
			pps->release();
			pps = NULL;
		}
		if (!pps)
			pps = new ASettableObjectVector(lines);
		for(vsize_t i = 0; i < lines; i++) {
			group_t group = (group_t) i;
			vsize_t index=0;
			for(vsize_t gi = 0; gi < group; gi++)
				index += groupCounts[gi];
			const char* groupName = groupNames->stringAt(index);
			APolyLineStatVisual *pl = new APolyLineStatVisual(this, _scales[0]->locations(), _scales[1]->locations(), 
															  groupMems[(vsize_t)group], groupCounts[(vsize_t)group],
															  marker, (vsize_t*)groupIDs->asInts(), len, group, groupName, false, false);
			pl->setDrawAttributes(ptSize, ptAlpha);
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
			vsize_t i = 0, lines = pps->length();
			while (i < lines) {
				APolyLineStatVisual *pl = (APolyLineStatVisual*) pps->objectAt(i);
				pl->setPoints(_scales[0]->locations(), _scales[1]->locations());
				pl->setDrawAttributes(ptSize, ptAlpha);
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
		if (APlot::keyDown(e)) return true;
		switch (e.key) {
			case KEY_DOWN: if (ptSize > 1.0) { ptSize -= 1.0; setRedrawLayer(LAYER_ROOT); update(); redraw(); }; break;
			case KEY_UP: ptSize += 1.0; setRedrawLayer(LAYER_ROOT); update();redraw(); break;
			case KEY_LEFT: if (ptAlpha > 0.02) { ptAlpha -= (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha < 0.02) ptAlpha = 0.02; setRedrawLayer(LAYER_ROOT); update();redraw(); }; break;
			case KEY_RIGHT: if (ptAlpha < 0.99) { ptAlpha += (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha > 1.0) ptAlpha = 1.0; setRedrawLayer(LAYER_ROOT); update(); redraw(); } break;
			//toggle keys
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
	
	virtual void draw(vsize_t layer) {
		if (layer == LAYER_TRANS) { // draw the orientation query if needed
			if (_query->isHidden() && inQuery) {
				clip(AMkRect(mLeft, mBottom, _frame.width - mRight - mLeft, _frame.height - mTop - mBottom));
				APoint ql = AMkPoint(_query->frame().x, _query->frame().y);
				char buf[64];
				const char *v2;
				strcpy(buf, _scales[0]->stringForDoubleValue(_scales[0]->value(ql.x)));
				v2 = _scales[1]->stringForDoubleValue(_scales[1]->value(ql.y));
				color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.6);
				ASize ts1 = bbox(buf), ts2 = bbox(v2);
				if (ts1.width < ts2.width) ts1.width = ts2.width;
				rect(AMkRect(ql.x, ql.y - 4.0, ts1.width + 6.0, ts2.height + ts1.height + 8.0));
				color(0,0,0,0.7);
				line(0.0, ql.y, _frame.width, ql.y);
				line(ql.x, 0.0, ql.x, _frame.height);
				color(1,1,1,0.7);
				line(0.0+1, ql.y+1, _frame.width+1, ql.y+1);
				line(ql.x+1, 0.0+1, ql.x+1, _frame.height+1);
				//txtcolor(0,0,0.4);
				AFloat xcoord = ql.x + 4.0;
				AFloat ycoord = ql.y + 2.0;
				if ((ql.x + 4.0 + ts1.width) > _frame.width)
					xcoord = ql.x - 4.0 - ts1.width;
				if ((ql.y + 2.0 + ts1.height + ts2.height) > _frame.height)
					ycoord = ql.y - 2.0 - ts1.height - ts2.height;
				text(xcoord, ycoord, buf, 0.0);
				text(xcoord, ycoord + ts1.height, v2, 0.0);
				clip(_frame);
			}
		}
		//draw children - polyines!
		APlot::draw(layer);
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
		if (_caption)
			return _caption;
		return value_printf("TimeSeries Plot of %s",
							(datay->name()) ? datay->name() : "<tmp>");
	}
};

#endif
