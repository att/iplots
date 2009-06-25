/*
 *  ADataVector.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/4/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

/******** THIS IS CURRENTLY UNUSED - ADataVector is defined in AVector.h *********/

#ifndef A_DATAVECTOR_H
#define A_DATAVECTOR_H

#include "AVector.h"
#include "AMarker.h"

#if 0
class AObjectWithMarker {
protected:
	AMarker *_marker;
	
public:
	AObjectWithMarker(AMarker *m) : _marker(m ? ((AMarker*)m->retain()) : m) {}
	virtual ~AObjectWithMarker() { if (_marker) _marker->release(); }
	
	AMarker *marker() { return _marker; }
};


/*
class ADoubleDataVector : public ADoubleVector, AObjectWithMarker {
	ADoubleDataVector(AMarker *m, double *data, vsize_t len, bool copy) : AObjectWithMarker(m), ADoubleVector(data, len, copy) {
		OCLASS(ADoubleDataVector);
	}
	virtual ~ADoubleDataVector() {
		DCLASS(ADoubleDataVector);
	}
};
 */

class ADoubleDataVector : public ADoubleVector, public ADataVector {
	ADoubleDataVector(AMarker *m, double *data, vsize_t len, bool copy) : ADataVector(m, len), ADoubleVector(data, len, copy) {
		OCLASS(ADoubleDataVector);
	}
	virtual ~ADoubleDataVector() {
		DCLASS(ADoubleDataVector);
	}
};

#endif

#endif
