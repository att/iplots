/*
 *  AScale.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "AVector.h"

class AScale : public AObject {
	AVector *_data;
	ARange gr;
	ADataRange dr;
	int n;
public:
	AScale(AVector *data, ARange graphicsRange, ADataRange dataRange) : _data(data), gr(graphicsRange),
	dr(dataRange), n(dataRange.length) {}
	
	ARange range() { return gr; }
	ADataRange dataRange() { return dr; }
	void setRange(ARange r) { gr = g; /* FIXME: notify ? */ }
	void setDataRange(ADataRange r) { dr = d; }

	double value(AFloat pos) { return ((double)((pos - gr.begin) / gr.length)) * dr.length + dr.begin; }
	AFloat position(double value) { return ((value - dr.begin) / dr.length) * gr.length + gr.begin; }
	
	AVector *data() { return _data; }
	
	// FIXME: discrete layout?
	int discreteValue(AFloat pos) { return -1; }
	AFloat discretePosition(int value) { return g_o; }
}
