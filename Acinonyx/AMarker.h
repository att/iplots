/*
 *  AMarker.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/4/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_MARKER_H
#define A_MARKER_H

#include "ANotfier.h"
#include "AVector.h"
#include "AColorMap.h"
#include "ATable.h"
#include "AStack.h"
#include "AIndex.h"

// NOTE: if this is changed, then also the supercalss may have to be changed!
#ifdef __cplusplus
extern "C" {
#endif
typedef unsigned int mark_t;
#ifdef __cplusplus
}
#endif
	
#define M_MARK_BIT ((mark_t)1)
#define M_OUT_BIT  ((mark_t)2)
#define M_BITMASK  ((mark_t)3)
#define M_MKMARK(X) (((mark_t)(X)) << 2)

#define M_TRANS(X) ((((mark_t)(X)) & 1) == 1)
#define M_OUT(X) ((((mark_t)(X)) & 2) == 2)
#define M_MARK(X) (((mark_t)(X)) >> 2)

#define N_TransientMarkerChanged 0x11
#define N_PermanentMarkerChanged 0x10

#define TAG_UNDO_TRANS  1
#define TAG_UNDO_OUT    2
#define TAG_UNDO_VALUE  3

class AMarker : public ANotifierInterface, public APlainIntVector {
protected:
	bool _batch;
	bool _changed, _perm_changed;
	AColorMap *color_map;
	mark_t max_value;
	AUnivarTable *value_table;
	AForgetfulStack *undo_;
	ADenseBitmap *lastTrans, *lastPerm;
	
	// checks whether anything changed and performs caching and notification if the batch mode is off
	void weChanged() {
		if (_changed && !_batch) {
			ALog("%s: weChanged: %s", describe(), _perm_changed ? "permanent" : "transient");
			// re-compute the maximum value
			if (_perm_changed) {
				max_value = 0;
				for (vsize_t i = 0; i < _len; i++)
					if ((mark_t) _data[i] > max_value)
						max_value = (mark_t) _data[i];
				if (value_table) { // invalidate the value table
					value_table->release();
					value_table = 0;
				}
				// currently we can't tell whether it's visibility or value - but we store only visibility
				if (undo_) {
					undo_->push(lastPerm);
					lastPerm->release();
					lastPerm  = new ADenseBitmap(_data, length(), M_OUT_BIT);
					lastPerm->tag_  = TAG_UNDO_OUT;
				}
				// FIXME: we don't know whether transient selection changed as well or not, so we assume it did not .. not good .. We should maybe store the whole set instead ..				
			} else
				if (undo_) {
					undo_->push(lastTrans);
					lastTrans->release();
					lastTrans = new ADenseBitmap(_data, length(), M_MARK_BIT);
					lastTrans->tag_ = TAG_UNDO_TRANS;
				}
			// notify all dependents
			sendNotification(this, _perm_changed ? N_PermanentMarkerChanged : N_TransientMarkerChanged);
			// re-set changed flag
			_perm_changed = _changed = false;
		}
	}

public:
	AMarker(vsize_t len) : APlainIntVector(0, len, false), ANotifierInterface(false), value_table(0), max_value(0), undo_(0) {
		_len = len;
		_perm_changed = _changed = false;
		_data = (int*) calloc(sizeof(len), len);
		color_map = 0;
		AMEM(_data);
		OCLASS(AMarker)
	};
	
	virtual ~AMarker() {
		if (_data) free(_data);
		if (color_map) color_map->release();
		if (value_table) value_table->release();
		if (undo_) {
			lastTrans->release();
			lastPerm->release();
			undo_->popAll();
			undo_->release();
		}
		DCLASS(AMarker)
	};
	
	void enableUndo(vsize_t maxEntries = 8) {
		if (undo_) return;
		lastTrans = new ADenseBitmap(_data, length(), M_MARK_BIT);
		lastTrans->tag_ = TAG_UNDO_TRANS;
		lastPerm  = new ADenseBitmap(_data, length(), M_OUT_BIT);
		lastPerm->tag_  = TAG_UNDO_OUT;
		undo_ = new AForgetfulStack(maxEntries);
		ALog("%s: enabled undo", describe());
	}
	
	void undo() {
		ALog("Marker undo: %s", undo_ ? undo_->describe() : "<undo is NULL>");
		if (undo_ && !undo_->isEmpty()) {
			ADenseBitmap *bm = (ADenseBitmap*) undo_->pop();
			if (bm) {
				if (bm->tag_ == TAG_UNDO_TRANS) { /* selection undo */
					lastTrans->release();
					lastTrans = bm;
					bm->restore(_data, M_MARK_BIT);
					sendNotification(this, N_TransientMarkerChanged);
				} else if (bm->tag_ == TAG_UNDO_OUT) { /* visibility undo */
					lastPerm->release();
					lastPerm = bm;
					bm->restore(_data, M_OUT_BIT);
					sendNotification(this, N_PermanentMarkerChanged);
				} else bm->release(); /* don't know what to do - just remove */
			}
		}
	}
	
	void setColorMap(AColorMap *cm) {
		if (color_map)
			color_map->release();
		if (cm)
			cm->retain();
		color_map = cm;
	};
	
	AColorMap *colorMap() {
		return color_map;
	};
	
	// begin batch transactions - those will be grouped for notification
	void begin() {
		_batch = true;
	};
	
	// end batch transactions
	void end() {
		_batch = false;
		weChanged();
	};
	
	bool isSelected(vsize_t index) {
		return (index < _len) ? M_TRANS(_data[index]) : false;
	};
	
	bool isHidden(vsize_t index) {
		return (index < _len) ? M_OUT(_data[index]) : false;
	};

	mark_t value(vsize_t index) {
		return (index < _len) ? M_MARK(_data[index]) : 0;
	};

	AColor color(vsize_t index) {
		return (color_map) ? color_map->color(value(index)) : NilColor;
	};
		
	AColor color(vsize_t index, AFloat alpha) {
		AColor c = color(index);
		if (alpha >= 1.0)
			return c;
		if (c.a > 0.0)
			c.a = alpha;
		return c;
	};
	
	void select(vsize_t index) {
		if (index < _len && M_TRANS(_data[index]) == 0) {
			_data[index] |= M_MARK_BIT;
			_changed = true;
		}
		weChanged();
	};

	void hide(vsize_t index) {
		if (index < _len && M_OUT(_data[index]) == 0) {
			_data[index] |= M_OUT_BIT;
			_perm_changed = _changed = true;
		}
		weChanged();
	};
	
	void show(vsize_t index) {
		if (index < _len && M_OUT(_data[index])) {
			_data[index] &= ~M_OUT_BIT;
			_perm_changed = _changed = true;
		}
		weChanged();
	};

	void selectXOR(vsize_t index) {
		if (index < _len)
			_data[index] ^= M_MARK_BIT;
		_changed = true;
		weChanged();
	};

	void deselect(vsize_t index) {
		if (index < _len && M_TRANS(_data[index])) {
			_data[index] &= ~M_MARK_BIT;
			_changed = true;
		}
		weChanged();
	};

	void invertSelection() {
		for (vsize_t i = 0; i < _len; i++)
			_data[i] ^= M_MARK_BIT;
		_changed = true;
		weChanged();
	};

	void selectAll() {
		for (vsize_t i = 0; i < _len; i++)
			if (M_TRANS(_data[i]) == 0) {
				_changed = true;
				_data[i] |= M_MARK_BIT;
			}				
		weChanged();
	};

	void deselectAll() {
		for (vsize_t i = 0; i < _len; i++)
			if (M_TRANS(_data[i])) {
				_changed = true;
				_data[i] &= ~M_MARK_BIT;
			}
		weChanged();
	};
	
	void setValue(vsize_t index, mark_t value) {
		if (index < _len) {
			mark_t nm = M_MKMARK(value) | (_data[index] & M_BITMASK);
			if (_data[index] != nm) {
				_data[index] = nm;
				_perm_changed = _changed = true;
			}
		}
		weChanged();
	};
	
	mark_t maxValue() {
		return max_value;
	};
	
	AUnivarTable *valueTable() {
		// we compute the table only on-demand and cache it
		if (!value_table) {
			value_table = new AUnivarTable(max_value + 1, false);
			for (vsize_t i = 0; i < _len; i++)
				value_table->add(M_MARK(_data[i]));
		}
		return value_table;
	};
	
	// NOTE: we expose this so that fast iterators can be written. We may think about some solution that is fast but doesn't expose as much ...
	const mark_t *rawMarks() {
		return (const mark_t*) _data;
	};
};

#endif
