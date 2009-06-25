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

#define N_MarkerChanged 0x10

class AMarker : public ANotifierInterface, public AIntVector {
protected:
	bool _batch;
	bool _changed;
	
	void weChanged() {
		if (_changed && !_batch) {
			sendNotification(this, N_MarkerChanged);
			_changed = false;
		}
	}

public:
	AMarker(vsize_t len) : AIntVector(0, len, false), ANotifierInterface(false) {
		_len = len;
		_changed = false;
		_data = (int*) calloc(sizeof(len), len);
		AMEM(_data);
		OCLASS(AMarker)
	};
	
	virtual ~AMarker() {
		DCLASS(AMarker)
	};
	
	// begin batch transactions - those will be grouped for notification
	void begin() {
		_batch = true;
	};
	
	// end batch transactions
	// FIXME: currently we issue a change notification even if nothing changed - review if it's worth adding a flag to truly track the changes
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
	
	void select(vsize_t index) {
		if (index < _len && M_TRANS(_data[index]) == 0) {
			_data[index] |= M_MARK_BIT;
			_changed = true;
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
		if (index < _len)
			_data[index] = M_MKMARK(value) | (_data[index] & M_BITMASK);
		weChanged();
	};
	
	// NOTE: we expose this so that fast iterators can be written. We may think about some solution that is fast but doesn't expose as much ...
	mark_t *rawMarks() {
		return (mark_t*) _data;
	};
};

#endif
