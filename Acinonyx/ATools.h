/*
 *  ATools.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_TOOLS_H
#define A_TOOLS_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ATypes.h"

extern AColor backgroundColor, pointColor, hiliteColor, barColor;

extern long profilerTime, startupTime;

#ifdef __cplusplus
extern "C" {
#endif

void *memdup(const void *buf, unsigned int len);

	/*-- profiling support --*/
#ifdef PROFILE
#define profStart() profilerTime=time_ms();
#define _prof(X) X;
	long time_ms();
	void profReport(const char *fmt, ...);
#else
#define profStart()
#define _prof(X)
#endif

#ifdef DEBUG
	void ALog(const char *fmt, ...);
#else
#define ALog(X, ...)
#endif
	
#ifdef __cplusplus
}
#endif

#endif /* A_TOOLS_H */
