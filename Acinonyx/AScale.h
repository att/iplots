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
	AVector *_data;
	ARange gr;
	ADataRange dr;
	int n;
public:
	AScale(AVector *data, ARange graphicsRange, ADataRange dataRange) : _data(data), gr(graphicsRange),
	dr(dataRange), n(dataRange.length) { data->retain(); OCLASS(AScale) }
	virtual ~AScale() {
		if (_data) _data->release();
		DCLASS(AScale)
	}
	
	ARange range() { return gr; }
	ADataRange dataRange() { return dr; }
	void setRange(ARange r) { gr = r; /* FIXME: notify ? */ }
	void setDataRange(ADataRange r) { dr = r; }

	double value(AFloat pos) { return ((double)((pos - gr.begin) / gr.length)) * dr.length + dr.begin; }
	AFloat position(double value) { return ((value - dr.begin) / dr.length) * gr.length + gr.begin; }
	
	AVector *data() { return _data; }
	
	// FIXME: discrete layout?
	int discreteValue(AFloat pos) { return -1; }
	AFloat discretePosition(int value) { return gr.begin; }
};

#endif

