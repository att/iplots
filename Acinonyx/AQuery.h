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
		if (q_text) AFree(q_text);
		if (txt) {
			q_text = strdup(txt);
			text_bbox = bbox(txt);
			_frame.width = text_bbox.width + 2.0 * margins;
			_frame.height = text_bbox.height + 2.0 * margins;
			redraw();
		} else {
			q_text = 0;
			_frame.width = 0;
			_frame.height = 0;
			setHidden(true);
		}
	}
	
	/** resets the query by removing all content. This method should be called when building up a new query. Implicitly it sets the status of the query to hidden, but it doesn't cause a redraw. */
	void reset() {
		setHidden(true);
		if (q_text) { AFree(q_text); q_text = 0; }
	}
	
	/** move the query to a given point. The point is adjusted based on its size if it would not fit in the window. This method causes a redraw.
	 *  @param pt point at which the query shall be displayed (if visible) */
	void move(APoint pt) {
		_frame.x = pt.x;
		_frame.y = pt.y;
		if (window()) { // if there is a window, let's see if we fit in and adjust the position if we don't
			ARect wf = window()->frame();
			if (_frame.x + _frame.width > wf.width - 2.0) _frame.x = wf.width - _frame.width - 2.0;
			if (_frame.y + _frame.height > wf.height - 2.0) _frame.y = wf.height - _frame.height - 2.0;
		}
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
