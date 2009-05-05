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

#include <R.h>
#include <Rinternals.h>

class RObject : public AObject {
	SEXP ptr;
public:	
	RObject(SEXP rObj) : ptr(rObj) {
		R_PreserveObject(rObj);
#ifdef ODEBUG
		printf("R %08x <- %08x\n", (int) this, (int) rObj);
#endif
		OCLASS(RObject);
	}
	
	virtual ~RObject() {
		R_ReleaseObject(rObj);
#ifdef ODEBUG
		printf("R %08x -> %08x\n", (int) this, (int) rObj);
#endif
		DCLASS(RObject)
	}

	SEXP value() { return ptr; }
};

#endif
