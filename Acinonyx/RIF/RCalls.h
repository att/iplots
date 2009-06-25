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

extern "C" void call_with_object(SEXP fun, AObject *o, const char *clazz);

#endif
