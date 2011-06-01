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
#include "ACueButton.h"

class ABarChart : public APlot {
protected:
	ADiscreteXAxis *xa;
	AFactorVector *data;
	AYAxis *ya;
	vsize_t bars;
	bool spines;
	
	vsize_t movingBar, movingBarTarget;
	AFloat movingBarX, movingBarDelta;
	
	ACueButton *buttonSpine, *buttonBrush;
	
public:
	ABarChart(AContainer *parent, ARect frame, int flags, AFactorVector *x) : APlot(parent, frame, flags), movingBar(ANotFound), spines(false){
		mLeft = 20.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 10.0f;

		nScales = 2;
		marker = x->marker();
		if (marker) {
			marker->retain();
			marker->add(this);
		}
		bars = 0;
		_scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		AMEM(_scales);
		AUnivarTable *tab = x->table();
		_scales[0] = new AScale(data = x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), bars = x->levels());
		_scales[1] = new AScale(NULL, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), tab->maxCount() + 1);
		xa = new ADiscreteXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_DEFAULT|AVF_FIX_BOTTOM|AVF_XSPRING|AVF_FIX_HEIGHT, _scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_DEFAULT|AVF_FIX_LEFT|AVF_FIX_WIDTH|AVF_YSPRING, _scales[1]);
		add(*ya);
		createPrimitives();
		
		ACueWidget *cb = new ACueWidget(this, AMkRect(frame.x, frame.y + frame.height - 17, frame.width, 17), AVF_DEFAULT | AVF_FIX_TOP | AVF_FIX_HEIGHT | AVF_XSPRING, true);
		add(*cb);
		cb->release();
		buttonSpine = new ACueButton(cb, AMkRect(frame.x + 3, frame.y + frame.height - 3 - 14, 50, 14), AVF_DEFAULT | AVF_FIX_LEFT | AVF_FIX_TOP | AVF_FIX_HEIGHT | AVF_FIX_WIDTH, "Foo");
		buttonSpine->setDelegate(this);
		buttonSpine->setLabel(spines ? ">Bars" : ">Spines");
		cb->add(*buttonSpine);
		buttonSpine->release();

		ARect f = buttonSpine->frame();
		f.x += f.width + 10;
		buttonBrush = new ACueButton(cb, f, AVF_DEFAULT | AVF_FIX_LEFT | AVF_FIX_TOP | AVF_FIX_HEIGHT | AVF_FIX_WIDTH, "Brush");
		buttonBrush->setDelegate(this);
		buttonBrush->click_action = "brush.by.group";
		cb->add(*buttonBrush);
		buttonBrush->release();

		OCLASS(ABarChart)
	}
	
	virtual ~ABarChart() {
		xa->release();
		ya->release();
		DCLASS(ABarChart);
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
	
	void createPrimitives() {
		AFactorVector *data = (AFactorVector*) _scales[0]->data();
		AUnivarTable *tab = data->table();
		if (pps && pps->length() != bars) {
			pps->release();
			pps = NULL;
		}
		if (!pps)
			pps = new ASettableObjectVector(bars);
		for(vsize_t i = 0; i < bars; i++) {
			group_t group = (group_t) i;
			ABarStatVisual *b = new ABarStatVisual(this, rectForBar(tab, group), Up, marker, (vsize_t*) data->asInts(), data->length(), group, false, false, false);
			b->setGroupName(tab->name(i));
			((ASettableObjectVector*)pps)->replaceObjectAt(i, b);
			b->release(); // we passed the ownership to pps
		}
	}
	
	virtual void delegateAction(AWidget *source, const char *action, AObject *aux) {
		if (source == buttonSpine) {
			spines = !spines;
			update();
			redraw();
		} else if (action && !strcmp(action, "brush.by.group")) {
			brushByGroup();
		} else if (action && !strcmp(action, "brush.clear")) {
			buttonBrush->click_action = "brush.by.group";
			buttonBrush->setLabel("Brush");
			marker->clearValues();
		}
	}
	
	void update() {
		_scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		_scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));

		ya->setHidden(spines);
		buttonSpine->setLabel(spines ? ">Bars" : ">Spines");
		
		if (!pps)
			createPrimitives();
		else {
			AFactorVector *data = (AFactorVector*) _scales[0]->data();
			AUnivarTable *tab = data->table();
			vsize_t i = 0, bars = pps->length();
			while (i < bars) {
				ABarStatVisual *b = (ABarStatVisual*) pps->objectAt(i);
				ARect br = rectForBar(tab, (group_t) i);
				if (i == movingBar)
					br.x = movingBarX;
				b->setRect(br);
				i++;
			}
		}
		APlot::update();
	}
	
	void brushByGroup() {
		AFactorVector *data = (AFactorVector*) _scales[0]->data();
		vsize_t l = data->levels();
		AUnivarTable *tab = new AUnivarTable(l);
		vsize_t n = data->length();
		const int *bi = data->asInts();
		for (vsize_t i = 0; i < n; i++)
			if (!marker->isHidden(i))
				tab->add((vsize_t) bi[i]);
		APermutation *perm = _scales[0]->permutation();
		vsize_t nonzero = 0;
		vsize_t *b_map = (vsize_t*)calloc(sizeof(vsize_t), l);
		AMEM(b_map);
		for (vsize_t i = 0; i < l; i++) {
			vsize_t level = perm->permutationAt(i); 
			if (tab->count(level) > 0) {
				nonzero++;
				b_map[level] = nonzero;
			}
		}
		vsize_t color_base = COL_CB1;
		if (nonzero > 10) {
			color_base = COL_HCL;
			for (vsize_t i = 0; i < l; i++)
				b_map[i] = b_map[i] * COL_NHCL / (l + 1);
		}
		marker->begin();
		for (vsize_t i = 0; i < n; i++)
			if (!marker->isHidden(i)) // FIXME: whould be alse re-set invisibles?
				marker->setValue(i, color_base + b_map[bi[i]]);
		marker->end();
		free(b_map);
		tab->release();

		buttonBrush->click_action = "brush.clear";
		buttonBrush->setLabel("Clear");

		update(); redraw();
	}

	virtual bool keyDown(AEvent e) {
		if (APlot::keyDown(e)) return true;
		switch (e.key) {
			case KEY_S: spines = !spines; update(); redraw(); break;
			case KEY_C: brushByGroup(); break;
			default:
				return false;
		}
		return true;
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
		return value_printf("Barchart of %s", (data && data->name()) ? data->name() : "<tmp>");
	}
	
};

#endif
