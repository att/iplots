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

#include <time.h>

#include "ATools.h"
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
	APermutation *perm;
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
		if (data) {
			data->retain();
			perm = data->permutation();
			if (perm) perm->retain();
		}
		OCLASS(AScale)
	}
	
	AScale(AVector *data, ARange graphicsRange, vsize_t entries) : _data(data), gr(graphicsRange), dr(AMkDataRange(0.0, entries)), n(entries), _locations(NULL), nLoc(0), perm(new APermutation(entries)), shareRange(false), shareDataRange(false), _prev(NULL), _next(NULL) {
		if (data) {
			data->retain();
			perm = data->permutation();
			if (perm) perm->retain();
		}
		OCLASS(AScale)
	}
	
	virtual ~AScale() {
		if (_data) _data->release();
		if (perm) perm->release();
		if (_prev && _next) _prev->_next = _next; // this should never really happen since _prev has retained us ...
		else if (_next) {
			_next->_prev = NULL;
			_next->release();
		}
		DCLASS(AScale)
	}
	
	///FIX ME: added by Anushka -- to set n for markervaluesglyph
	void setN(vsize_t np){
		n=np;
	}
	
	void setPermutation(APermutation *p) {
		if (perm) perm->release();
		perm = p;
		perm->retain();
	}
	
	APermutation *permutation() { return perm; }
	
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
		
	AVector *data() { return _data; }
	AFloat *locations() {
		if (!_data) return NULL;
		if (nLoc < _data->length()) {
			if (_locations) AFree(_locations); _locations = NULL;
		}
		if (!_locations) {
			nLoc = _data->length();
			_locations = (AFloat*) AAlloc(sizeof(AFloat) * nLoc);
			AMEM(_locations);
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
	
	vsize_t permutationAt(vsize_t index) { return perm ? perm->permutationAt(index) : index; }
	vsize_t permutationOf(vsize_t index) { return perm ? perm->permutationOf(index) : index; }
	void swap(vsize_t a, vsize_t b) { if (!perm) perm = new APermutation(n); perm->swap(a,b); }
	void moveToIndex(vsize_t a, vsize_t ix) { if (!perm) perm = new APermutation(n); perm->moveToIndex(a, ix); }
		
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
        return position(0.5 + (double) i);
	}
    	
	ARange discreteRange(vsize_t value) {
		vsize_t i = permutationOf(value);
		if (i >= n) return AUndefRange;
        AFloat left = position(i);
		return AMkRange(left, position(i + 1) - left);
	}

	AFloat discreteWidth(vsize_t value) {
        return position(1) - position(0);
	}
	
	const char *stringForDoubleValue(double val) {
		if (_data && _data->isTime()) {
			time_t t = (time_t) val;
			struct tm *ts = gmtime(&t);
			return value_printf("%u/%02u/%02u %02u:%02u:%02u", ts->tm_year + 1900, ts->tm_mon + 1,
								ts->tm_mday, ts->tm_hour, ts->tm_min, ts->tm_sec);
			
		}
		return value_printf("%g", val);
	}
};

#endif

