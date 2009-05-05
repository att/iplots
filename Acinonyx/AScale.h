/*
 *  AScale.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_SCALE_H_
#define A_SCALE_H_

#include "AVector.h"

class AScale : public AObject {
protected:
	AVector *_data;
	ARange gr;
	ADataRange dr;
	AFloat *_locations;
	int nLoc;
	bool cacheDirty;
	int n;
	
public:
	AScale(AVector *data, ARange graphicsRange, ADataRange dataRange) : _data(data), gr(graphicsRange),
	dr(dataRange), n(dataRange.length), _locations(NULL), nLoc(0) { if (data) data->retain(); OCLASS(AScale) }
	virtual ~AScale() {
		if (_data) _data->release();
		DCLASS(AScale)
	}
	
	ARange range() { return gr; }
	ADataRange dataRange() { return dr; }
	void setRange(ARange r) { gr = r; cacheDirty = true; /* FIXME: notify ? */ }
	void setDataRange(ADataRange r) { dr = r; cacheDirty = true;}

	double value(AFloat pos) { return ((double)((pos - gr.begin) / gr.length)) * dr.length + dr.begin; }
	ADataRange toDataRange(ARange r) { ADataRange d = AMkDataRange(value(r.begin), value(r.begin+r.length)); d.length -= d.begin; return d; }

	AFloat position(double value) { return ((value - dr.begin) / dr.length) * gr.length + gr.begin; }
	ARange toRange(ADataRange r) { ARange v = AMkRange(position(r.begin), position(r.begin + r.length)); v.length -= v.begin; return v; }
	
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
		
	
	// FIXME: discrete layout?
	int discreteValue(AFloat pos) { return -1; }
	AFloat discretePosition(int value) { return gr.begin; }
};

#endif

