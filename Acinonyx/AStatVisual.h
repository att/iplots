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
#include "AQuery.h"

#define ANoGroup (-1)

typedef int group_t;

static char query_buffer[512];

#define ASVF_HIDDEN 0x0001

class AStatVisual : public AVisualPrimitive {
protected:
	// currently we support two types of indexing:
	// - a list of case IDs (_group = ANoGroup, ids = int[represented cases])
	// - indexing by group (_group = value which will be checked against ids = int[all cases])
	vsize_t *ids, n; // FIXME: we may want to abstract this out / generalize later ...
	group_t _group;
	// group name - used for queries, optional
	const char *_group_name;
	AMarker *mark;
	// cached values - those are updated by update()
	vsize_t selected, hidden, visible; 
	mark_t minMark, maxMark;
	// the subclasses can allocate a value table that will be updated automatiacally (if set the visual is assumed to be the owner). The inisital size can be arbitrary as it will be re-allocated to accomadate larger mark sets. It is NULL by default in which case it is ignored.
	AUnivarTable *value_table;
	int flags;
	bool release_ids, showHidden;

public:
	AStatVisual(APlot *plot, AMarker *m, vsize_t *i, vsize_t len, group_t group=ANoGroup, bool copy=true, bool releaseIDs=true, AUnivarTable *initValTable=0) : AVisualPrimitive(plot), mark(m), n(len), _group(group), _group_name(NULL), flags(0), value_table(initValTable), showHidden(false) {
		if (mark) {
			mark->retain();
			mark->add(this);
		}
		if (value_table) value_table->retain();
		ids = copy ? ((vsize_t *) memdup(i, len)) : i;
		release_ids = releaseIDs;
		this->markerChanged();
		this->update();
		OCLASS(AStatVisual)
	}

	virtual ~AStatVisual() {
		if (release_ids && ids) free(ids);
		if (value_table) value_table->release();
		if (mark) {
			mark->remove(this);
			mark->release();
		}
		DCLASS(AStatVisual)
	}
	
	bool isHidden() { return (flags & ASVF_HIDDEN) ? true : false; }

	void setHidden(bool h) { flags &= ~ASVF_HIDDEN; if (h) flags |= ASVF_HIDDEN; }
	
	void setGroupName(const char *group_name) {
		_group_name = group_name;
	}
	
	// this method is called upon highlighting change
	// and it calculates selected, hidden, min/max marks and the value table (if set)
	void markerChanged() {
		selected = 0;
		hidden = 0;
		if (value_table) {
			if (value_table->size() < mark->maxValue()) {
				value_table->release();
				value_table = new AUnivarTable(mark->maxValue(), false);
			} else
				value_table->reset();
		}
		if (_group == ANoGroup) { // direct indexing
			if (n) minMark = maxMark = mark->value(ids[0]);
			else minMark = maxMark = 0;
			for (vsize_t i = 0; i < n; i++) {
				if (mark->isHidden(ids[i]) != showHidden) hidden++;
				else{
					mark_t v =  mark->value(ids[i]);
					if (mark->isSelected(ids[i]))
						selected++;
					else if (v && value_table) value_table->add(v);
					if (v > maxMark) maxMark = v; else if (v < minMark) minMark = v;
				}
			}
			visible = n - hidden;
		} else { // group indexing
			visible = 0;
			for (vsize_t i = 0; i < n; i++)
				if ((group_t)ids[i] == _group) {
					if (mark->isHidden(i) != showHidden) hidden++;
					else {
						mark_t v =  mark->value(i);
						if (visible == 0)
							minMark = maxMark = v;
						else {
							if (v > maxMark) maxMark = v; else if (v < minMark) minMark = v;
						}
						visible++;
						if (mark->isSelected(i)) selected++;
						else if (v && value_table) value_table->add(v);
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
					if (mark->isHidden(i) == showHidden)
						marker->selectXOR(ids[i]);
			} else if (type == SEL_NOT) {
				for (vsize_t i = 0; i < n; i++)
					if (mark->isHidden(i) == showHidden)
						marker->deselect(ids[i]);
			} else if (type == SEL_AND) {
				for (vsize_t i = 0; i < n; i++)
					if (mark->isHidden(i) == showHidden)
						marker->deselect(ids[i]);
			} else {
				for (vsize_t i = 0; i < n; i++)
					if (mark->isHidden(i) == showHidden)
						marker->select(ids[i]);
			}
		} else { // group indexing
			if (type == SEL_XOR) {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group && mark->isHidden(i) == showHidden)
						marker->selectXOR(i);
			} else if (type == SEL_NOT) {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group && mark->isHidden(i) == showHidden)
						marker->deselect(i);
			} else if (type == SEL_AND) {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group && mark->isHidden(i) == showHidden)
						marker->deselect(i);
			} else {
				for (vsize_t i = 0; i < n; i++)
					if ((group_t)ids[i] == _group && mark->isHidden(i) == showHidden)
						marker->select(i);
			}
		}
		return true;
	}
	
	virtual void notification(AObject *source, notifid_t nid) {
		ALog("%s: notification() -> update()", describe());
		markerChanged();
	}
	
	virtual void query(AQuery *query, int level) {
		ALog("%s: query, level=%d", describe(), level);
		vsize_t aux = 0;
		char *qb = query_buffer;
		if (_group_name) {
			strcpy(query_buffer, _group_name);
			strcat(query_buffer, (strlen(_group_name) < 12) ? ": " : "\n");
			qb = query_buffer + (aux = strlen(query_buffer));
		}
		if (visible) {
			if (selected)
				snprintf(qb, sizeof(query_buffer) - aux, "%d / %d (%.2f%%)", selected, visible, ((double) selected) / ((double) visible) * 100.0);
			else
				snprintf(qb, sizeof(query_buffer) - aux, "%d cases", visible);
		} else
			snprintf(query_buffer, sizeof(query_buffer) - aux, "no cases are visible");
		ALog(" - set %s to '%s'", query->describe(), query_buffer);
		query->setText(query_buffer);
	}

};

typedef enum { Up = 1, Down = 2, fromLeft = 3, fromRight = 4 } direction_t;

class ABarStatVisual : public AStatVisual {
protected:
	ARect _r;
	direction_t fillingDirection;
public:
	ABarStatVisual(APlot *plot, ARect r, direction_t fillDir, AMarker *m, vsize_t *ids, vsize_t len, group_t group, bool copy=true,
				   bool releaseIDs=true, bool sHidden) : AStatVisual(plot, m, ids, len, group, copy, releaseIDs, new AUnivarTable(32, false)), _r(r), fillingDirection(fillDir) {
		showHidden = sHidden;
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
	
	virtual void draw(ARenderer &renderer, vsize_t layer) {
		if (isHidden() && !showHidden) return;
		ALog("%s: draw (visible=%d, selected=%d, hidden=%d, showHidden=%d, group=%d) [%g,%g %g,%g]", describe(), visible, selected, hidden, showHidden, _group, _r.x, _r.y, _r.width, _r.height);
		if (visible) {
			ARect r = _r;
			r.height *= ((AFloat) visible) / ((AFloat) (visible + hidden));
			
			if (f.a && layer == LAYER_ROOT) {
				renderer.color(f);
				renderer.rect(r);
			}

			if (layer == LAYER_HILITE && mark->maxValue() > 0) { /* color brushing */
				AColorMap *cmap = mark->colorMap();
				AFloat progress = r.height * ((AFloat) selected) /  ((AFloat) visible), base = r.height / ((AFloat) visible);
				
				if (fillingDirection == fromLeft || fillingDirection == fromRight)
					base = r.width / ((AFloat) visible);
				
				vsize_t n = value_table->size();
				for (mark_t i = 1; i < n; i++) {
					AFloat ch = base * (AFloat) value_table->count(i);
					if (ch > 0.0) {	
						renderer.color(cmap->color(i));
						switch (fillingDirection) {
							case Up: renderer.rect(r.x, r.y + progress, r.x + r.width, r.y + progress + ch); break;
							case Down: renderer.rect(r.x, r.y + r.height - progress - ch, r.x + r.width, r.y + r.height - progress); break;
							case fromLeft: renderer.rect(r.x + progress, r.y, r.x + progress + ch, r.y + r.height); break;
							case fromRight: renderer.rect(r.x + r.width - progress - ch, r.y, r.x + r.width - progress, r.y + r.height); break;
						}
						progress += ch;
					}
				}
			}
			if (layer == LAYER_HILITE && selected) {
				renderer.color(hiliteColor);
				AFloat prop = ((AFloat) selected) /  ((AFloat) visible);
				switch (fillingDirection) {
					case Up: renderer.rect(r.x, r.y, r.x + r.width, r.y + prop * r.height); break;
					case Down: renderer.rect(r.x, r.y + (1.0 - prop) * r.height, r.x + r.width, r.y + r.height); break;
					case fromLeft: renderer.rect(r.x, r.y, r.x + prop * r.width, r.y + r.height); break;
					case fromRight: renderer.rect(r.x + (1.0 - prop) * r.width, r.y, r.x + r.width, r.y + r.height); break;
				}
			}
			if (layer == LAYER_HILITE && c.a) {
				renderer.color(c);
				renderer.rectO(r);
			}
		}
	}
		
	virtual bool intersects(ARect rect) {
		if (!visible && !showHidden) return false;
#ifdef ODEBUG
		ARect r = AMkRect(_r.x, _r.y, _r.width, _r.height);
		r.height *= ((AFloat) visible) / ((AFloat) (visible + hidden));
		bool a = ARectsIntersect(rect, r);
		ALog("%s intersects(%g,%g %g,%g) [%g,%g %g,%g]: %s", describe(), ARect4(rect), ARect4(_r), a?"YES":"NO");
		return a;
#else
		return ARectsIntersect(rect, _r);
#endif
	}
	
	virtual bool containsPoint(APoint pt) {
		if (!visible && !showHidden) return false;
		ARect r = AMkRect(_r.x, _r.y, _r.width, _r.height);
		r.height *= ((AFloat) visible) / ((AFloat) (visible + hidden));
		return ARectContains(r, pt);
	}
	
	virtual void query(AQuery *query, int level) {
		ALog("%s: query, level=%d", describe(), level);
		vsize_t aux = 0;
		char *qb = query_buffer;
		if (_group_name) {
			strcpy(query_buffer, _group_name);
			strcat(query_buffer, (strlen(_group_name) < 12) ? ": " : "\n");
			qb = query_buffer + (aux = strlen(query_buffer));
		}
		if (visible) {
			if (selected)
				snprintf(qb, sizeof(query_buffer) - aux, "%d / %d (%.2f%%)", selected, visible, ((double) selected) / ((double) visible) * 100.0);
			else if (hidden)
				snprintf(qb, sizeof(query_buffer) - aux, "%d / %d (%.2f%%)", visible, (visible+hidden), ((double) visible) / ((double) hidden + visible) * 100.0);
				else
					snprintf(qb, sizeof(query_buffer) - aux, "%d cases", visible);
		} else{
			if (showHidden)
				snprintf(query_buffer, sizeof(query_buffer) - aux, "%d cases", hidden);
			else
				snprintf(query_buffer, sizeof(query_buffer) - aux, "no cases visible");
		}
		ALog(" - set %s to '%s'", query->describe(), query_buffer);
		query->setText(query_buffer);
	}
	
};

class APolyLineStatVisual : public AStatVisual {
protected:
	AFloat *xs, *ys;
	AFloat _ptSize;
	vsize_t *groupMems;
	vsize_t groupCount;
	APoint *pts;
	
public:
	APolyLineStatVisual(APlot *plot, AFloat *x, AFloat *y, 	vsize_t *gMems, vsize_t gCount, AMarker *m, vsize_t *ids, vsize_t len, 
						group_t group, const char* name, bool copy=true, bool releaseIDs=true) : AStatVisual(plot, m, ids, len, group, copy, releaseIDs) {
		c = pointColor;
		groupCount = gCount;
		groupMems = gMems;
		_group_name = name;
		pts = (APoint*) malloc(sizeof(APoint) * groupCount);
		setPoints(x, y);
		OCLASS(APolyLineStatVisual);
	}
	
	~APolyLineStatVisual() {
		free(pts);
		DCLASS(APolyLineStatVisual)
	}
	
	void setPoints(AFloat *x, AFloat *y){
		xs = x;
		ys = y;
		for (vsize_t i = 0; i < groupCount; i++) {
			pts[i].x = x[groupMems[i]];
			pts[i].y = y[groupMems[i]];
		}
	}
	
	void setDrawAttributes(AFloat ptSize, AFloat ptAlpha){
		c.a = ptAlpha;
		_ptSize = ptSize; 
	}
	
	virtual void draw(ARenderer &renderer, vsize_t layer) {
		if (isHidden()) return;
		ALog("%s: draw (visible=%d, selected=%d, hidden=%d) PolyLine!", describe(), visible, selected, hidden);
		if (visible == 0) return;
		glPointSize(_ptSize);
		if (c.a && layer == LAYER_ROOT) {
			if (hidden == 0 && minMark == maxMark) {
				renderer.color(mark->color(groupMems[0], c.a));
				renderer.polyline(pts, groupCount);
				renderer.points(pts, groupCount);
			} else {
				//draw lines
				for (vsize_t i=0; i<(groupCount-1); i++){
					renderer.color(mark->color(groupMems[i], c.a));
					if (!mark->isHidden(groupMems[i]) && !mark->isHidden(groupMems[i+1]))
						renderer.line(xs[groupMems[i]], ys[groupMems[i]], xs[groupMems[i+1]], ys[groupMems[i+1]]);		
				}
				//draw points
				for (vsize_t i=0; i<groupCount; i++){
					renderer.color(mark->color(groupMems[i], c.a));
					if (!mark->isHidden(groupMems[i]))
						renderer.point(xs[groupMems[i]], ys[groupMems[i]]);				
				}
			}
		}
		if (visible){
			if (layer == LAYER_HILITE && selected){
				renderer.color(hiliteColor);
				//draw lines
				for (vsize_t i=0; i<(groupCount-1); i++){
					if (!mark->isHidden(groupMems[i]) && !mark->isHidden(groupMems[i+1]) &&
						mark->isSelected(groupMems[i]) && mark->isSelected(groupMems[i+1]))
						renderer.line(xs[groupMems[i]], ys[groupMems[i]], xs[groupMems[i+1]], ys[groupMems[i+1]]);			
				}
				//draw points
				for (vsize_t i=0; i<groupCount; i++){
					if (!mark->isHidden(groupMems[i]) && mark->isSelected(groupMems[i]))
						renderer.point(xs[groupMems[i]], ys[groupMems[i]]);				
				}
			}
		}
	}

	//checks if the selection rectangle contains a point on this polyline
	virtual bool intersects(ARect rect) {
		if (!visible) return false;
		for (vsize_t i = 0; i < groupCount; i++)
			if (!mark->isHidden(groupMems[i]) && ARectContains(rect, pts[i]))
				return true;
		return false;
	}
	
	//checks if this polyline contains the input point 
	virtual bool containsPoint(APoint pt) {
		if (!visible) return false;
		for (int i = 0; i < groupCount; i++){
			if (!mark->isHidden(groupMems[i]) && isNear(pt, pts[i])) {
				return true;
			}
		}
		return false;
	}
	bool isNear(APoint a, APoint b){
		double dist = (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
		if (dist < 10) 
			return true;
		return false;
	}
		
};

#endif
