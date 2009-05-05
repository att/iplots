/*
 *  ATools.c
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#include "ATools.h"

AColor backgroundColor = AMkColor(1.0, 1.0, 0.7, 0.0);

void *memdup(const void *buf, unsigned int len) {
	void *v = malloc(len);
	AMEM(v);
	memcpy(v, buf, len);
	return v;
}

/* FIXME: intitallize NA_xx values */
/* declared in ATypes.h */
double NA_double;
float NA_float;
