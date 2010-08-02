/*
 *  AMarkerValuesPlot.h
 *  Acinonyx
 *
 *  Created by Anushka Anand on [July 20].
 *  Copyright 2010 University of Illinois at Chicago. All rights reserved.
 *
 */

#ifndef MARKER_VALUES_PLOT_H
#define MARKER_VALUES_PLOT_H

#include "AStatVisual.h"
#include "APlot.h"
#include <ext/hash_map>

using namespace __gnu_cxx;

class AMarkerValuesPlot : public APlot {
protected:
	ADiscreteXAxis *xa;
//	AFactorVector *data;
	AYAxis *ya;
	vsize_t bars;
	bool spines;
	vsize_t *data;	
	vsize_t movingBar, movingBarTarget;
	AFloat movingBarX, movingBarDelta;
	
public:
	AMarkerValuesPlot(AContainer *parent, ARect frame, int flags, AMarker* m) : APlot(parent, frame, flags), movingBar(ANotFound), spines(false) {
		mLeft = 20.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 10.0f;
		marker = m;
		if (marker) {
			marker->retain();
			marker->add(this);
		}
		data = (vsize_t*) calloc(marker->length(), sizeof(vsize_t));
		
		AUnivarTable* tab = setData();
		nScales = 2;
		_scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		AMEM(_scales);
		_scales[0] = new AScale(NULL, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), bars);
		_scales[1] = new AScale(NULL, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), tab->maxCount() + 1);
		
		xa = new ADiscreteXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, _scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, _scales[1]);
		add(*ya);
		
		createPrimitives(tab);
		OCLASS(AMarkerValuesPlot)
	}
	
	virtual ~AMarkerValuesPlot() {
		xa->release();
		ya->release();
		free(data); data = NULL;
		DCLASS(AMarkerValuesPlot);
	}
	
	AUnivarTable* setData(){
		//set the data - marker values, hidden
		vsize_t levels = 1;		//starts at 1 because it includes hidden
		hash_map<int, int> uniqueValues;
		for(vsize_t i = 0; i < marker->length(); i++){
			if (!marker->isHidden(i)){
				hash_map <int, int> :: const_iterator ui = uniqueValues.find((marker->value(i)+1));
				if (ui == uniqueValues.end())	{
					uniqueValues[(marker->value(i)+1)] = levels;
					levels++;
				}
			}
		}
		//set the data - marker values, hidden
		AUnivarTable *tab = new AUnivarTable(levels);
		if (tab){
			tab->setName(0,"hidden");
			int vi=1;
			for (hash_map <int, int> :: const_iterator ui = uniqueValues.begin(), e = uniqueValues.end(); ui != e; ++ui){
				char * mvalue = new char[50];
				sprintf(mvalue, "%d", ui->first);						
				uniqueValues[ui->first] = vi;
				tab->setName(vi, (const char *)mvalue);
				vi++;
			}
			for (vsize_t i = 0; i < marker->length(); i++){
				if(marker->isHidden(i)){
					tab->add(0);
					data[i] =(vsize_t)  0; 
				}
				else{
					tab->add((vsize_t) uniqueValues[(marker->value(i) + 1)]);
					data[i] =(vsize_t)  uniqueValues[(marker->value(i) + 1)]; 
				}
			}
		}
		
		bars = levels;
		return tab;
	}

	void setScales(AUnivarTable* tab){
		_scales[0]->setPermutation(new APermutation(bars));
		_scales[0]->setDataRange(AMkDataRange(0.0, bars - 1));
		_scales[0]->setN(bars);
		_scales[1]->setPermutation(new APermutation(tab->maxCount() + 1));
		_scales[1]->setDataRange(AMkDataRange(0.0, tab->maxCount()));
		_scales[1]->setN(tab->maxCount() + 1);
	}
	
	void createPrimitives(AUnivarTable *tab) {
		if (pps && pps->length() != bars) {
			pps->release();
			pps = NULL;
		}
		if (!pps)
			pps = new ASettableObjectVector(bars);
		for(vsize_t i = 0; i < bars; i++) {
			bool showHidden = (i==0) ? true : false;
			group_t group = (group_t) i;
			ABarStatVisual *b = new ABarStatVisual(this, rectForBar(tab, group), Up, marker, data, marker->length(), group, false, false, showHidden);
			b->setGroupName(tab->name(i));
			ALog("%s: i=%d, name=%s", describe(), i, tab->name(i));
			((ASettableObjectVector*)pps)->replaceObjectAt(i, b);
			b->release(); // we passed the ownership to pps			
		}
	}
	
	virtual void notification(AObject *source, notifid_t nid) {
		ALog("%s: notification() -> update()", describe());
		if (nid == N_PermanentMarkerChanged) update();
		APlot::notification(source, nid);
	}
	
	virtual void update() {
		_scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		_scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));
		ya->setHidden(spines);
		
		AUnivarTable* tab = setData();
		setScales(tab);
		createPrimitives(tab);
		APlot::update();
	}	
	
	ARect rectForBar(AUnivarTable *tab, group_t group) {
		ARange xr = _scales[0]->discreteRange((int)group);
		if (!spines && xr.length > 40.0) { xr.begin = xr.begin + xr.length / 2 - 20.0; xr.length = 40.0; }
		if (xr.length > 5.0) { xr.begin += xr.length * 0.1; xr.length *= 0.8; }
		if (spines) {
			double prop = tab->maxCount() ? (((double) tab->count(group)) / ((double) tab->maxCount())) : 0;
			xr.begin += (1.0 - prop) / 2 * xr.length;
			xr.length *= prop;
			ARange gr = _scales[1]->range();
			return AMkRect(xr.begin, gr.begin, xr.length, gr.length);
		} else
			return AMkRect(xr.begin, _scales[1]->position(0), xr.length, _scales[1]->position(tab->count(group)) - _scales[1]->position(0));
	}
	
	virtual bool mouseDown(AEvent e) {
		// handle bar dragging
		if ((e.flags & AEF_ALT) && (e.flags & AEF_BUTTON1) && pps) {
			vsize_t i = 0, bars = pps->length();
			while (i < bars) {
				ABarStatVisual *b = (ABarStatVisual*) pps->objectAt(i);
				if (b->containsPoint(e.location)) {
					ALog("moving bar %d - start", i);
					movingBar = i;
					ABarStatVisual *b = (ABarStatVisual*) pps->objectAt(i);
					movingBarX = b->rect().x;
					movingBarDelta = movingBarX - e.location.x;
					movingBarTarget = scale(0)->discreteValue(e.location.x);
					if (b) b->setAlpha(0.5);
					update(); redraw();
					return true;
				}
				i++;
			}
		}
		return APlot::mouseDown(e);
	}
	
	virtual bool event(AEvent e) {
		if (movingBar != ANotFound && e.event == AE_MOUSE_MOVE) {
			movingBarX = e.location.x + movingBarDelta;
			movingBarTarget = scale(0)->discreteIndex(e.location.x);
			vsize_t currentTarget = scale(0)->permutationOf(movingBar);
			ALog("moving bar target = %d (current = %d)", movingBarTarget, currentTarget);
			if (movingBarTarget != currentTarget)
				scale(0)->moveToIndex(movingBar, movingBarTarget);
			ALog("after move: %d", scale(0)->permutationOf(movingBar));
			update();
			redraw();
			return true;
		} else return APlot::event(e);
	}
	
	virtual bool mouseUp(AEvent e) {
		if (movingBar != ANotFound) {
			ABarStatVisual *b = (ABarStatVisual*) pps->objectAt(movingBar);
			if (b) b->setAlpha(1.0);
			movingBar = ANotFound;
			update();
			redraw();
			return true;
		}
		return APlot::mouseUp(e);
	}
	
	virtual double doubleProperty(const char *name) {
		if (!strcmp(name, "spines")) return spines ? 1.0 : 0.0;
		return APlot::doubleProperty(name);
	}
	
	virtual bool setDoubleProperty(const char *name, double value) {
		if (!strcmp(name, "spines")) { bool desired = (value > 0.5); if (spines != desired) { spines=desired; update(); redraw(); return true; } };
		return APlot::setDoubleProperty(name, value);
	}
	
	virtual const char *caption() {
		if (_caption)
			return _caption;
		return value_printf("Barchart of Marker Values");
	}

	virtual bool keyDown(AEvent e) {
		if (APlot::keyDown(e)) return true;
		switch (e.key) {
			case KEY_S: spines = !spines; update(); redraw(); break;
			default:
				return false;
		}
		return true;
	}
	
};

#endif
