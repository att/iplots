/*
 *  AQuery.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/2/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_QUERY_H__
#define A_QUERY_H__

#include "AVisual.h"
#include "AContainer.h"

class AQuery : public AVisual {
protected:
	char *q_text;
	ASize text_bbox;
	AFloat margins;
public:
	AQuery(AContainer *parent) : AVisual(parent, AMkRect(0,0,0,0), 0), q_text(0), margins(5.0) {
		OCLASS(AQuery)
	}
	
	void setText(const char *txt) {
		if (q_text) free(q_text);
		if (txt) {
			q_text = strdup(txt);
			text_bbox = bbox(txt);
			_frame.width = text_bbox.width + 2.0 * margins;
			_frame.height = text_bbox.height + 2.0 * margins;
			redraw();
		} else {
			q_text = 0;
			setHidden(true);
		}
	}
	
	void reset() {
		setHidden(true);
		if (q_text) { free(q_text); q_text = 0; }
	}
	
	void move(APoint pt) {
		_frame.x = pt.x;
		_frame.y = pt.y;
		redraw();
	}
	
	const char *text() {
		return q_text;
	}
	
	virtual void draw(vsize_t layer) {
		if (layer != LAYER_TRANS) return;
		color(AMkColor(0,0,0,0.3));
		ARect r = _frame; r.x += 5.0; r.y -= 5.0; // shadow
		rect(r);
		color(backgroundColor);
		rect(_frame);
		color(AMkColor(0,0,0,1));
		rectO(_frame);
		ARenderer::text(AMkPoint(_frame.x + margins, _frame.y + margins), q_text, AMkPoint(0, 0), 0.0);
	}
};

#endif
