/*
 *  AScale.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_SCALE_H_
#define A_SCALE_H_

#include "AVector.h"

typedef enum { XScale = 1, YScale } scale_t; // designated scale types

class AScale : public AObject {
protected:
	AVector *_data;
	ARange gr;
	ADataRange dr;
	AFloat *_locations;
	int nLoc;
	bool cacheDirty;
	vsize_t n;
	vsize_t *perm;
	AScale *_prev, *_next; // shared axes linked list
	bool shareDataRange, shareRange;
	
	// the following functions are internal and can only be called on the root scale
	void _removeShared(AScale *scale) {
		if (_next == scale) {
			scale->_prev = NULL;
			_next = scale->_next;
			scale->_next = NULL; // we take the ownership, hence no retain/release
			scale->release();
		} else if (_next) _next->_removeShared(scale);
	}
	
	void __setRange(ARange r) { gr = r; cacheDirty = true; }
	void __setDataRange(ADataRange r) { dr = r; cacheDirty = true; }
	void _setRange(ARange r) { if (shareRange) __setRange(r); if (_next) _next->_setRange(r); }
	void _setDataRange(ADataRange r) { if (shareDataRange) __setDataRange(r); if (_next) _next->_setDataRange(r); }

public:
	AScale(AVector *data, ARange graphicsRange, ADataRange dataRange) : _data(data), gr(graphicsRange),
	dr(dataRange), n(dataRange.length), _locations(NULL), nLoc(0), perm(NULL), shareRange(false), shareDataRange(false), _prev(NULL), _next(NULL) {
		if (data) data->retain();
		OCLASS(AScale)
	}
	
	AScale(AVector *data, ARange graphicsRange, vsize_t entries) : _data(data), gr(graphicsRange), dr(AMkDataRange(0.0, entries - 1)), n(entries), _locations(NULL), nLoc(0), perm(NULL), shareRange(false), shareDataRange(false), _prev(NULL), _next(NULL) {
		if (data) data->retain();
		OCLASS(AScale)
	}
	
	virtual ~AScale() {
		if (_data) _data->release();
		if (_prev && _next) _prev->_next = _next; // this should never really happen since _prev has retained us ...
		else if (_next) {
			_next->_prev = NULL;
			_next->release();
		}
		DCLASS(AScale)
	}
	
	AScale *getSharedRoot() {
		return _prev ? _prev->getSharedRoot() : this;
	}
	
	void addShared(AScale *scale)
	{
		if (!_next) {
			if (scale->_prev) {
				fprintf(stderr, "ERROR: %s->addShared(", describe());
				fprintf(stderr, "%s) but the scale is already registered elsewhere (", scale->describe());
				fprintf(stderr, "%s) - ignoring\n", scale->_prev->describe());
				return;
			}
			_next = scale;
			scale->_prev = this;
			scale->retain();
		} else _next->addShared(scale);
	}
	
	void removeShared(AScale *scale)
	{
		getSharedRoot()->_removeShared(scale);
	}
	
	bool isShared() {
		return (_prev || _next);
	}
	
	ARange range() { return gr; }
	ADataRange dataRange() { return dr; }

	void setRange(ARange r) { if (shareRange) getSharedRoot()->_setRange(r); else __setRange(r); }
	void setDataRange(ADataRange r) { if (shareDataRange) getSharedRoot()->_setDataRange(r); else __setDataRange(r); }

	double value(AFloat pos) { return ((double)((pos - gr.begin) / gr.length)) * dr.length + dr.begin; }
	ADataRange toDataRange(ARange r) { ADataRange d = AMkDataRange(value(r.begin), value(r.begin+r.length)); d.length -= d.begin; return d; }

	AFloat position(double value) { return ((value - dr.begin) / dr.length) * gr.length + gr.begin; }
	ARange toRange(ADataRange r) { ARange v = AMkRange(position(r.begin), position(r.begin + r.length)); v.length -= v.begin; return v; }
	
	vsize_t permutationOf(vsize_t index) {
		if (!perm) return index;
		return (index >= n) ? index : perm[index];
	}
	
	vsize_t permutationAt(vsize_t index) {
		if (!perm) return index;
		for(vsize_t i = 0; i < n; i++)
			if (perm[i] == index)
				return i;
		return index;
	}
	
	vsize_t *permutations() { return perm; }
	
	AVector *data() { return _data; }
	AFloat *locations() {
		if (!_data) return NULL;
		if (nLoc < _data->length()) {
			free(_locations); _locations = NULL;
		}
		if (!_locations) {
			nLoc = _data->length();
			_locations = (AFloat*) malloc(sizeof(AFloat) * nLoc);
			cacheDirty = true;
		}
		if (cacheDirty) {
			float a = gr.length / dataRange().length;
			float b = gr.begin - a * dataRange().begin;
			_data->transformToFloats(_locations, a, b);
			cacheDirty = false;
		}
		return _locations;
	}
		
	void initializePermutations() {
		if (!perm)
			perm = (vsize_t*) malloc(sizeof(vsize_t) * n);
		for (vsize_t i = 0; i < n; i++) perm[i] = i;
	}
	
	void swap(vsize_t a, vsize_t b) {
		if (!perm) initializePermutations();
		vsize_t h = perm[a]; perm[a] = perm[b]; perm[b] = h;
	}
	
	void moveToIndex(vsize_t a, vsize_t ix) {
		if (!perm) initializePermutations();
		if (ix >= n) ix = n - 1;
		vsize_t prev = perm[a];
		if (prev == ix) return;
		if (ix < prev) {
			for (vsize_t i = 0; i < n; i++)
				if (perm[i] >= ix && perm[i] < prev)
					perm[i]++;
		} else { /* prev < ix */
			for (vsize_t i = 0; i < n; i++)
				if (perm[i] > prev && perm[i] <= ix)
					perm[i]--;
		}
		perm[a] = ix;
	}
	
	vsize_t discreteValue(AFloat pos) {
		if (gr.length == 0.0) return ANotFound;
		if (n == 1) return (gr.begin >= pos && gr.begin + gr.length <= pos) ? 0 : ANotFound;
		AFloat width = gr.length / ((AFloat) (n));
		int i = (int) ((pos - gr.begin) / width);
		if (i < 0 || i >= n) return ANotFound;
		return (vsize_t) permutationAt(i);
	}

	vsize_t discreteIndex(AFloat pos) {
		if (gr.length == 0.0) return ANotFound;
		if (n == 1) return (gr.begin >= pos && gr.begin + gr.length <= pos) ? 0 : ANotFound;
		AFloat width = gr.length / ((AFloat) (n));
		int i = (int) ((pos - gr.begin) / width);
		if (i < 0 || i >= n) return ANotFound;
		return (vsize_t) i;
	}
	
	AFloat discreteCenter(vsize_t value) {
		vsize_t i = permutationOf(value);
		if (i >= n) return ANotFound;
		AFloat width = gr.length / ((AFloat) (n));
		return gr.begin + (0.5 + (AFloat) i) * width;
	}
	
	ARange discreteRange(vsize_t value) {
		vsize_t i = permutationOf(value);
		if (i >= n) return AUndefRange;
		AFloat width = gr.length / ((AFloat) (n));
		AFloat left = gr.begin + ((AFloat) i) * width;
		return AMkRange(left, width);
	}

	AFloat discreteWidth(vsize_t value) { 
		return gr.length / ((AFloat) (n));
	}
};

#endif

