/*
 *  ARVector.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/26/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_R_VECTOR_H__
#define A_R_VECTOR_H__

#include "AVector.h"
#include "RObject.h"
#include "ANotfier.h"

class AContainsRObject {
protected:
	RObject *ro;
public:
	AContainsRObject(SEXP data) {
		ro = new RObject(data);
	}

	AContainsRObject(RObject *robj) {
		ro = robj;
		robj->retain();
	}
	
	virtual ~AContainsRObject() {
		if (ro) ro->release();
	}
	
	RObject *object() { return ro; }
	SEXP value() { return ro ? ro->value() : R_NilValue; }
};

class ARDoubleVector : public ADoubleVector, public AContainsRObject {
public:
	ARDoubleVector(AMarker *m, SEXP data) : ADoubleVector(m, REAL(data), LENGTH(data), false), AContainsRObject(data) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARDoubleVector)
	}

	ARDoubleVector(AMarker *m, RObject *o) : ADoubleVector(m, REAL(o->value()), LENGTH(o->value()), false), AContainsRObject(o) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARDoubleVector)
	}
};

class ARTimeVector : public ATimeVector, public AContainsRObject {
public:
	ARTimeVector(AMarker *m, SEXP data) : ATimeVector(m, REAL(data), LENGTH(data), false), AContainsRObject(data) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARTimeVector)
	}
	
	ARTimeVector(AMarker *m, RObject *o) : ATimeVector(m, REAL(o->value()), LENGTH(o->value()), false), AContainsRObject(o) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARTimeVector)
	}
};

class ARIntVector : public AIntVector, public AContainsRObject {
public:
	ARIntVector(AMarker *m, SEXP data) : AIntVector(m, INTEGER(data), LENGTH(data), false), AContainsRObject(data) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARIntVector)
	}

	ARIntVector(AMarker *m, RObject *o) : AIntVector(m, INTEGER(o->value()), LENGTH(o->value()), false), AContainsRObject(o) {
		owned = false; // do not free the pointer since it's owned by R
		OCLASS(ARIntVector)
	}
};

class ARFactorVector : public AFactorVector, public AContainsRObject {
public:
	// we need to copy the data, because R factors are 1-based, but we are 0-based *sigh* - may need to rethink this ...
	ARFactorVector(AMarker *m, SEXP data) : AFactorVector(m, INTEGER(data), LENGTH(data), NULL, 0, true), AContainsRObject(data) {
		// owned = false; // do not free the pointer since it's owned by R
		for (vsize_t i = 0; i < _len; i++) _data[i]--; // decrement all ...
		SEXP sl = Rf_getAttrib(data, R_LevelsSymbol);
		if (TYPEOF(sl) == STRSXP) {
			_levels = LENGTH(sl);
			_names = (char**) malloc(sizeof(char*) * _levels);
			AMEM(_names);
			for (vsize_t i = 0; i < _levels; i++)
				_names[i] = (char*) CHAR(STRING_ELT(sl, i));
		}
		OCLASS(ARFactorVector)
	}
	
	// we need a special descructor because we did not copy the names so we have to get rid of them before the super-destructor
	virtual ~ARFactorVector() {
		if (_names)
			memset(_names, 0, sizeof(*_names) * _levels);
	}
};

#endif
