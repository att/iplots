/*
 *  RCalls.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/18/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_RCALLS_H__
#define A_RCALLS_H__

#include "AObject.h"
#include "RObject.h"

extern "C" {
	void call_with_object(SEXP fun, AObject *o, const char *clazz);
	void call_notification(SEXP fun, AObject *dep, AObject *src, int nid);

	SEXP A2SEXP(AObject *o);
	AObject *SEXP2A(SEXP o);
}

#endif
