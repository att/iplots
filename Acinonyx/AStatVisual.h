/*
 *  AStatVisual.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/8/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_STAT_VISUAL_H
#define A_STAT_VISUAL_H

#include "AMarker.h"
#include "AVisualPrimitive.h"

#define ANoGroup (-1)

typedef int group_t;

static char query_buffer[512];

class AStatVisual : public AVisualPrimitive {
protected:
	// currently we support two types of indexing:
	// - a list of case IDs (_group = ANoGroup, ids = int[represented cases])
	// - indexing by group (_group = value which will be checked against ids = int[all cases])
	vsize_t *ids, n; // FIXME: we may want to abstract this out / generalize later ...
	group_t _group;
	AMarker *mark;
	// cached values - those are updated by update()
	vsize_t selected, hidden, visible; 
	mark_t minMark, maxMark;

public:
	AStatVisual(AMarker *m, vsize_t *i, vsize_t len, group_t group=ANoGroup, bool copy=true) : mark(m), n(len), _group(group) {
		if (mark) {
			mark->retain();
			mark->add(this);
		}
		ids = copy ? ((vsize_t *) memdup(i, len)) : i;
		this->update();
		OCLASS(AStatVisual)
	}

	virtual ~AStatVisual() {
		if (ids) free(ids);
		if (mark) {
			mark->remove(this);
			mark->release();
		}
		DCLASS(AStatVisual)
	}
	
	// this method is called upon highlighting change
	// and it calculates selected, hidden and min/max marks
	virtual void update() {
		selected = 0;
		hidden = 0;
		if (_group == ANoGroup) { // direct indexing
			if (n) minMark = maxMark = mark->value(ids[0]);
			else minMark = maxMark = 0;
			for (vsize_t i = 0; i < n; i++) {
				if (mark->isHidden(ids[i])) hidden++;
				else {
					if (mark->isSelected(ids[i])) selected++;
					mark_t v =  mark->value(ids[i]);
					if (v > maxMark) maxMark = v; else if (v < minMark) minMark = v;
				}
			}
			visible = n - hidden;
		} else { // group indexing
			visible = 0;
			for (vsize_t i = 0; i < n; i++)
				if ((group_t)ids[i] == _group) {
					mark_t v =  mark->value(i);
					if (mark->isHidden(i)) hidden++; else {
						if (visible == 0)
							minMark = maxMark = v;
						else {
							if (v > maxMark) maxMark = v; else if (v < minMark) minMark = v;
						}
						visible++;
						if (mark->isSelected(i)) selected++;
					}
				}
		}
	}
	
	virtual bool select(AMarker *marker, int type) {
		if (_group == ANoGroup) { // direct indexing
			if (n) minMark = maxMark = mark->value(ids[0]);
			else minMark = maxMark = 0;
			if (type == SEL_XOR) {
				for (vsize_t i = 0; i < n; i++)
					marker->selectXOR(ids[i]);
			} else if (type == SEL_NOT) {
				for (vsize_t i = 0; i < n; i++)
					marker->deselect(ids[i]);
			} else if (type == SEL_AND) {
				for (vsize_t i = 0; i < n; i++)
					marker->deselect(ids[i]);
			} else {
				for (vsize_t i = 0; i < n; i++)
					marker->select(ids[i]);
			}
		} else { // group indexing
			if (type == SEL_XOR) {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group)
						marker->selectXOR(i);
			} else if (type == SEL_NOT) {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group)
						marker->deselect(i);
			} else if (type == SEL_AND) {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group)
						marker->deselect(i);
			} else {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group)
						marker->select(i);
			}
		}
		return true;
	}
	
	virtual void notification(AObject *source, notifid_t nid) {
		ALog("%s: notification() -> update()", describe());
		update();
	}
	
	virtual char *query(int level) {
		if (visible)
			snprintf(query_buffer, sizeof(query_buffer), "%d/%d (%g%%)", selected, visible, ((double) selected) / ((double) visible));
		else
			snprintf(query_buffer, sizeof(query_buffer), "no cases are visible");
		return query_buffer;
	}

};

typedef enum { Up = 1, Down = 2, fromLeft = 3, fromRight = 4 } direction_t;

class ABarStatVisual : public AStatVisual {
protected:
	ARect _r;
	direction_t fillingDirection;
public:
	ABarStatVisual(ARect r, direction_t fillDir, AMarker *m, vsize_t *ids, vsize_t len, group_t group, bool copy=true) : AStatVisual(m, ids, len, group, copy), _r(r), fillingDirection(fillDir) {
		f = barColor;
		c = pointColor;
		OCLASS(ABarStatVisual);
	}
	
	void setRect(ARect r) {
		_r = r;
	}
	
	ARect rect() {
		return _r;
	}
	
	virtual void draw(ARenderer &renderer) {
		ALog("%s: draw (visible=%d, selected=%d, hidden=%d) [%g,%g %g,%g]", describe(), visible, selected, hidden,
			   _r.x, _r.y, _r.width, _r.height);
		if (f.a) {
			renderer.color(f);
			renderer.rect(_r);
		}
		if (visible) {
			if (selected) {
				renderer.color(hiliteColor);
				AFloat prop = ((AFloat) selected) /  ((AFloat) visible);
				switch (fillingDirection) {
					case Up: renderer.rect(_r.x, _r.y, _r.x + _r.width, _r.y + prop * _r.height); break;
					case Down: renderer.rect(_r.x, _r.y + (1.0 - prop) * _r.height, _r.x + _r.width, _r.y + _r.height); break;
					case fromLeft: renderer.rect(_r.x, _r.y, _r.x + prop * _r.width, _r.y + _r.height); break;
					case fromRight: renderer.rect(_r.x + (1.0 - prop) * _r.width, _r.y, _r.x + _r.width, _r.y + _r.height); break;
				}
			}
		}
		if (c.a) {
			renderer.color(c);
			renderer.rectO(_r);
		}
		
	}
	
	virtual bool intersects(ARect rect) {
#ifdef ODEBUG
		bool a = ARectsIntersect(rect, _r);
		ALog("%s intersects(%g,%g %g,%g) [%g,%g %g,%g]: %s", describe(), ARect4(rect), ARect4(_r), a?"YES":"NO");
		return a;
#else
		return ARectsIntersect(rect, _r);
#endif
	}
	
	virtual bool containsPoint(APoint pt) {
		return ARectContains(_r, pt);
	}
};

#endif
