/*
 *  RCalls.cpp
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/18/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#include "RCalls.h"

#include "REngine.h"
#include "AMarker.h"
#include "AVisual.h"

extern "C" {
	SEXP A_Init();
	SEXP A_RegisterVar(SEXP v);
	SEXP A_ReleaseObject(SEXP o);
	SEXP A_MarkerCreate(SEXP len);
	SEXP A_MarkerAdd(SEXP m);
	SEXP A_WindowCreate(SEXP);
}

static void AObjFinalizer(SEXP ref) {
	if (TYPEOF(ref) == EXTPTRSXP) {
		AObject *o = (AObject*) R_ExternalPtrAddr(ref);
		if (o) o->release();
	}
}

static SEXP A2SEXP(AObject *o) {
	SEXP xp = R_MakeExternalPtr(o, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(xp, AObjFinalizer, TRUE);
	return xp;
}

static SEXP PTR2SEXP(void *ptr) {
	return R_MakeExternalPtr(ptr, R_NilValue, R_NilValue);
}

static AObject *SEXP2A(SEXP o) {
	if (TYPEOF(o) != EXTPTRSXP)
		Rf_error("invalid object");
	return (AObject*) R_ExternalPtrAddr(o);
}

SEXP A_Init() {
	return R_NilValue;
}

SEXP A_RegisterVar(SEXP v, SEXP mark) {
	AObject *vo = NULL;
	AMarker *m = (mark == R_NilValue) ? NULL : (AMarker*) SEXP2A(mark);
	if (TYPEOF(v) == REALSXP) {
		// FIXME: we should createa a subclass of A...Vector that contains the R object so we don't have to worry about mm
		vo = new ADoubleVector(m, REAL(v), LENGTH(v), true);
	} else if (TYPEOF(v) == INTSXP) {
		if (Rf_inherits(v, "factor")) {
			SEXP sls = Rf_getAttrib(v, R_LevelsSymbol);
			int ls = LENGTH(sls);
			char **lstr = (char**) malloc(ls * sizeof(char*));
			for (vsize_t i = 0; i < ls; i++) lstr[i] = strdup(CHAR(STRING_ELT(sls, i)));
			vo = new AFactorVector(m, INTEGER(v), LENGTH(v), (const char**) lstr, ls, true);
			free(lstr);
		} else 
			vo = new AIntVector(m, INTEGER(v), LENGTH(v), true);
	} else Rf_error("unsupported data type");
	return A2SEXP(vo);	
}

SEXP A_ReleaseObject(SEXP v) {
	AObject *o = SEXP2A(v);
	if (o) o->release();
	return R_NilValue;
}

SEXP A_MarkerCreate(SEXP len) {
	AMarker *m = new AMarker(Rf_asInteger(len));
	return A2SEXP(m);
}

SEXP A_MarkerAdd(SEXP sM, SEXP sO) {
	AMarker *m = (AMarker*) SEXP2A(sM);
	AObject *o = SEXP2A(sO);
	m->add(o);
	return sM;
}

void *ACocoa_CreateWindow(AVisual *visual, APoint position);

SEXP AWindowCreate(SEXP sVis, SEXP sPos)
{
	AVisual *vis = (AVisual*) SEXP2A(sVis);
	void *win = ACocoa_CreateWindow(vis, AMkPoint(REAL(sPos)[0], REAL(sPos)[1]));
	return PTR2SEXP(win);
}
