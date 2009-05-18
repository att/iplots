/*
 *  RObject.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 4/30/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_ROBJECT_H
#define A_ROBJECT_H

#include "AObject.h"

#define R_NO_REMAP 1
#define USE_RINTERNALS 1

/*-- we need to re-map our DEBUG --*/
#ifdef DEBUG
#define A_DEBUG 1
#undef DEBUG
#endif

#include <R.h>
#include <Rinternals.h>

#ifdef DEBUG
#undef DEBUG
#ifdef A_DEBUG
#define DEBUG
#endif
#endif

class RObject : public AObject {
	SEXP ptr;
public:	
	RObject(SEXP rObj) : ptr(rObj) {
		R_PreserveObject(rObj);
#ifdef ODEBUG
		printf("R %08x <- %08x (%d)\n", (int) this, (int) rObj, TYPEOF(rObj));
#endif
		OCLASS(RObject);
	}
	
	virtual ~RObject() {
		R_ReleaseObject(ptr);
#ifdef ODEBUG
		printf("R %08x -> %08x\n", (int) this, (int) ptr);
#endif
		DCLASS(RObject)
	}

	vsize_t length() { return LENGTH(ptr); }
	int type() { return TYPEOF(ptr); }
	
	bool isNULL() { return (ptr == R_NilValue) || (ptr == NULL); }
	
	int *integers() { return INTEGER(ptr); }
	double *doubles() { return REAL(ptr); }

	SEXP value() { return ptr; }
};

#endif
