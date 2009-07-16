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

/** RValueHolder is just a simple class/interface that allows its subclasses to hold a R-side value. It manages the lifetime of the associate object to match the lifetime of the C++ object. */
class RValueHolder {
protected:
	/** R object/value held in this object */
	SEXP _value;
public:
	/** create a new object that holds an R object/value - the value is protected from garbage collection until replaced or the value holder is freed.
	 @param value value to hold (SEXP) */
	RValueHolder(SEXP value = R_NilValue) : _value(value) {
		R_PreserveObject(_value);
	}
	
	virtual ~RValueHolder() {
		R_ReleaseObject(_value);
	}
	
	/** returns the SEXP value held by this object
	 @return R object held by this object */
	SEXP value() { return _value; }

	/** replaces the R value held by this object
	 @param newValue value to replace the current value, the previously held value is released. */
	void setValue(SEXP newValue) {
		R_PreserveObject(newValue);
		R_ReleaseObject(_value);
		_value = newValue;
	}
};

#endif