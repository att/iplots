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
protected:
	AXAxis *xa;
	AYAxis *ya;
	ADataVector *datax, *datay;
public:
	AScatterPlot(AContainer *parent, ARect frame, int flags, ADataVector *x, ADataVector *y) : APlot(parent, frame, flags) {
		mLeft = 20.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 10.0f;
		ptSize = 5.0;
		ptAlpha = 0.6;
		 printf("AScatterPlot frame = (%f,%f - %f x %f)\n", _frame.x, _frame.y, _frame.width, _frame.height);
		nScales = 2;
		marker = x->marker();
		if (!marker) marker = y->marker();
		if (marker) {
			marker->retain();
			marker->add(this);
		}
		_scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		AMEM(_scales);
		_scales[0] = new AScale(datax = x, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), x->range());
		_scales[1] = new AScale(datay = y, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), y->range());
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, _scales[0]);
		add(*xa);
		ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, _scales[1]);
		add(*ya);
		// add home zoom entry
		AZoomEntryBiVar *ze = new AZoomEntryBiVar(_scales[0]->dataRange(), _scales[1]->dataRange());
		zoomStack->push(ze);
		ze->release();
		OCLASS(AScatterPlot)
	}
	
	virtual ~AScatterPlot() {
		xa->release();
		ya->release();
		DCLASS(AScatterPlot)
	}
	
	void update() {
		_scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		_scales[1]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));

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
 			redraw();
			return true;
		}
		return false;
	}
	
	virtual bool performSelection(ARect where, int type, bool batch = false) {
		bool ps = APlot::performSelection(where, type, batch);
		if (currentContext != SEL_CONTEXT_DEFAULT)
			return ps;
		if (!marker) return ps;
		AFloat *lx = _scales[0]->locations();
		AFloat *ly = _scales[1]->locations();
		vsize_t nPts = _scales[0]->data()->length();
		if (!batch) marker->begin();
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
		if (!batch) marker->end();
		return true;
	}

	virtual bool keyDown(AEvent e) {
		if (APlot::keyDown(e)) return true;
		switch (e.key) {
			case KEY_DOWN: if (ptSize > 1.0) { ptSize -= 1.0; setRedrawLayer(LAYER_ROOT); redraw(); }; break;
			case KEY_UP: ptSize += 1.0; setRedrawLayer(LAYER_ROOT); redraw(); break;
			case KEY_LEFT: if (ptAlpha > 0.02) { ptAlpha -= (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha < 0.02) ptAlpha = 0.02; setRedrawLayer(LAYER_ROOT); redraw(); }; break;
			case KEY_RIGHT: if (ptAlpha < 0.99) { ptAlpha += (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha > 1.0) ptAlpha = 1.0; setRedrawLayer(LAYER_ROOT); redraw(); } break;
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

		AFloat *lx = _scales[0]->locations();
		AFloat *ly = _scales[1]->locations();

		if (layer == LAYER_ROOT) {
			clip(_frame);
			glPointSize(ptSize);
			AColor baseColor = AMkColor(0.0,0.0,0.0,ptAlpha);
			color(baseColor);

			if (marker && marker->maxValue()) {
#ifndef PFA
				glNewList(5, GL_COMPILE);
				circle(0, 0, ptSize / 2.0);
				glEndList();
				glPushMatrix();
#endif
				AFloat *lx = _scales[0]->locations();
				AFloat *ly = _scales[1]->locations();
				vsize_t n = _scales[0]->data()->length();
				for (vsize_t i = 0; i < n; i++) if (!marker->isHidden(i)) {
					color(marker->color(i, ptAlpha));
#ifndef PFA
					glTranslated(lx[i], ly[i], 0.0);
					glCallList(5);
					glTranslated(-lx[i], -ly[i], 0.0);
#else
					point(lx[i], ly[i]);
#endif
				}
			} else {
#ifndef PFA
				glNewList(5, GL_COMPILE);
				circle(0, 0, ptSize / 2.0);
				glEndList();
				glPushMatrix();
#endif
				vsize_t n = _scales[0]->data()->length();
#ifdef PFA
				for (vsize_t i = 0; i < n; i++)
					if (!marker->isHidden(i))
						point(lx[i], ly[i]);
#else
				for (vsize_t i = 0; i < n; i++) if (!marker->isHidden(i)) {
					glTranslated(lx[i], ly[i], 0.0);
					glCallList(5);
					glTranslated(-lx[i], -ly[i], 0.0);
				}
#endif
			}
			
#ifndef PFA
			glPopMatrix();
#endif
			clipOff();
			color(AMkColor(backgroundColor.r,backgroundColor.g,backgroundColor.b,0.5));
			rect(0.0,0.0,mLeft,mBottom);			
		}
		if (layer == LAYER_HILITE) {
			//points(lx, ly, _scales[0]->data()->length());
			if (marker) {
				const mark_t *ms = marker->rawMarks();
				vsize_t n = marker->length();
				color(AMkColor(1.0,0.0,0.0,1.0));
				for (vsize_t i = 0; i < n; i++)
					if (!marker->isHidden(i) && M_TRANS(ms[i])) {
#ifdef PFA
						point(lx[i], ly[i]);
#else
						glTranslated(lx[i], ly[i], 0.0);
						glCallList(5);
						glTranslated(-lx[i], -ly[i], 0.0);
#endif
					}
			}
		}

		// draw children - in our case axes etc.
		APlot::draw(layer);
	}

	virtual char *describe() {
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d]>", this, refcount, _objectSerial, _className, _classSize);
#else
		snprintf(desc_buf, 512, "<AScatterPlot %p>", this);
#endif
		return desc_buf;
	}
	
	virtual const char *caption() {
		if (_caption)
			return _caption;
		return value_printf("Scatterplot %s vs %s",
					 (datay->name()) ? datay->name() : "<tmp>",
					 (datax->name()) ? datax->name() : "<tmp>");
	}
};

#endif
