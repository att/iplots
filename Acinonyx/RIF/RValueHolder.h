/*
 *  RValueHolder.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/24/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_RVALUEHOLDER_H__
#define A_RVALUEHOLDER_H__

#include "RObject.h"

// this is just a simple class/interface that allows its subclasses to hold a R-side value
class RValueHolder {
protected:
	SEXP _value;
public:
	RValueHolder(SEXP value) : _value(value) {
		R_PreserveObject(_value);
	}
	
	virtual ~RValueHolder() {
		R_ReleaseObject(_value);
	}
	
	SEXP value() { return _value; }

	void setValue(SEXP newValue) {
		R_PreserveObject(newValue);
		R_ReleaseObject(_value);
		_value = newValue;
	}
};

#endif