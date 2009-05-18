/*
 *  ATools.c
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#include "ATools.h"

// common color scheme
AColor backgroundColor = AMkColor(1.0, 1.0, 0.7, 1.0);
AColor pointColor      = AMkColor(0.0, 0.0, 0.0, 1.0);
AColor hiliteColor     = AMkColor(1.0, 0.0, 0.0, 1.0);
AColor barColor        = AMkColor(0.8, 0.8, 0.8, 1.0);

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

#include <stdio.h>

#ifdef DEBUG
void ALog(char *fmt, ...) {
#ifdef PROFILE
	long npt = time_ms();
	if (startupTime == 0L) startupTime = npt;
	fprintf(stderr, "[%4ld.%03ld] ", (npt - startupTime) / 1000, (npt - startupTime) % 1000);
#endif
	va_list v;
	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);
	fprintf(stderr, "\n");
}
#endif

#ifdef PROFILE
#include <sys/time.h>
#include <stdio.h>

long time_ms() {
#ifdef Win32
	return 0; /* in Win32 we have no gettimeofday :( */
#else
	struct timeval tv;
	gettimeofday(&tv,0);
	return (tv.tv_usec/1000)+(tv.tv_sec*1000);
#endif
}

long profilerTime, startupTime;

void profReport(char *fmt, ...) {
	long npt = time_ms();
	if (startupTime == 0L) startupTime = profilerTime;
	fprintf(stderr, "[%4ld.%03ld] ", (npt - startupTime) / 1000, (npt - startupTime) % 1000);
	va_list v;
	va_start(v, fmt);
	vfprintf(stderr, fmt, v);
	va_end(v);
	fprintf(stderr, " %ld ms\n", npt - profilerTime);
	profilerTime = npt;
}
#endif
