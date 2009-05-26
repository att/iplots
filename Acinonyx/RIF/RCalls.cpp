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

#include "ABasicPrimitives.h"
#include "AScatterPlot.h"
#include "AParallelCoordPlot.h"
#include "ABarChart.h"

// R API to Acinonyx
extern "C" {
	SEXP A_Init();
	SEXP A_ReleaseObject(SEXP o);
	SEXP A_Describe(SEXP o);

	SEXP A_MarkerCreate(SEXP len);
	SEXP A_MarkerAdd(SEXP m, SEXP o);

	SEXP A_WindowCreate(SEXP w, SEXP pos);

	SEXP A_VarMark(SEXP v);
	SEXP A_VarRegister(SEXP v, SEXP mark);
	
	SEXP A_ScaleValue(SEXP sScale, SEXP sPos);
	SEXP A_ScalePosition(SEXP sScale, SEXP sPos);

	SEXP A_LineCreate(SEXP pos);
	SEXP A_BarCreate(SEXP pos);
	SEXP A_PolygonCreate(SEXP sx, SEXP sy);

	SEXP A_VPSetFill(SEXP vp, SEXP col);
	SEXP A_VPSetColor(SEXP vp, SEXP col);

	SEXP A_PlotPrimitives(SEXP sPlot);
	SEXP A_PlotAddPrimitive(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotRemovePrimitive(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotScale(SEXP sPlot, SEXP sSNR);
	SEXP A_PlotScales(SEXP sPlot);

	SEXP A_ScatterPlot(SEXP x, SEXP y, SEXP rect);
	SEXP A_BarPlot(SEXP x, SEXP rect);
	SEXP A_PCPPlot(SEXP vl, SEXP rect);
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


SEXP A_VarRegister(SEXP v, SEXP mark) {
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

SEXP A_VarMark(SEXP v) {
	ADataVector *dv = (ADataVector*) SEXP2A(v);
	AMarker *mark = dv->marker();
	return mark ? A2SEXP(mark) : R_NilValue;
}

SEXP A_Describe(SEXP o) {
	AObject *oo = SEXP2A(o);
	if (oo) return Rf_mkString(oo->describe());
	return R_NilValue;
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

SEXP A_PlotAddPrimitive(SEXP sPlot, SEXP sPrim) {
	APlot *pl = (APlot*) SEXP2A(sPlot);
	AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(sPrim);
	if (pl && vp) pl->addPrimitive(vp);
	return sPlot;
}

SEXP A_PlotRemovePrimitive(SEXP sPlot, SEXP sPrim)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(sPrim);
	if (pl && vp) pl->removePrimitive(vp);
	return sPlot;
}

SEXP A_PlotScale(SEXP sPlot, SEXP sSNR)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (pl) {
		AScale *s = pl->scale(Rf_asInteger(sSNR));
		if (s) return A2SEXP(s);
	}
	return R_NilValue;
}

SEXP A_PlotScales(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	return Rf_ScalarInteger(pl ? pl->scales() : 0);
}

SEXP A_PlotPrimitives(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (pl) {
		AObjectVector *p = pl->primitives();
		vsize_t n = p->length();
		SEXP res = Rf_protect(Rf_allocVector(VECSXP, n));
		for (vsize_t i = 0; i < n; i++)
			SET_VECTOR_ELT(res, i, A2SEXP(p->objectAt(i)));
		Rf_unprotect(1);
		return res;
	}
	return R_NilValue;
}

SEXP A_ScalePosition(SEXP sScale, SEXP sPos) {
	AScale *s = (AScale*) SEXP2A(sScale);
	vsize_t n = LENGTH(sPos);
	SEXP res = Rf_allocVector(REALSXP, n);
	double *d = REAL(res);
	double *p = REAL(sPos);
	if (!s) {
		memcpy(d, p, sizeof(double) * n);
		return res;
	}
	for(vsize_t i = 0; i < n; i++)
		d[i] = s->position(p[i]);
	return res;
}

SEXP A_ScaleValue(SEXP sScale, SEXP sPos) {
	AScale *s = (AScale*) SEXP2A(sScale);
	vsize_t n = LENGTH(sPos);
	SEXP res = Rf_allocVector(REALSXP, n);
	double *d = REAL(res);
	double *p = REAL(sPos);
	if (!s) {
		memcpy(d, p, sizeof(double) * n);
		return res;
	}
	for(vsize_t i = 0; i < n; i++)
		d[i] = s->value(p[i]);
	return res;
}

SEXP A_LineCreate(SEXP pos) {
	double *pp = REAL(pos);
	ALinePrimitive* p = new ALinePrimitive(NULL, AMkPoint(pp[0], pp[1]), AMkPoint(pp[2], pp[3]));
	return A2SEXP(p);
}

SEXP A_BarCreate(SEXP pos) {
	double *pp = REAL(pos);
	ABarPrimitive* p = new ABarPrimitive(NULL, AMkRect(pp[0], pp[1], pp[2], pp[3]));
	return A2SEXP(p);
}

SEXP A_PolygonCreate(SEXP sx, SEXP sy) {
	double *x = REAL(sx),  *y = REAL(sy);
	APoint *pt = (APoint*) malloc(sizeof(APoint) * LENGTH(sx));
	vsize_t n = LENGTH(x);
	for(vsize_t i = 0; i < n; i++)
		pt[i] = AMkPoint(x[i], y[i]);
	APolygonPrimitive *p = new APolygonPrimitive(NULL, pt, n, false);
	return A2SEXP(p);
}

SEXP A_VPSetColor(SEXP vp, SEXP col) {
	double *c = REAL(col);
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (p) 
		p->drawColor(c[0], c[1], c[2], c[3]);
	return R_NilValue;
}

SEXP A_VPSetFill(SEXP vp, SEXP col) {
	double *c = REAL(col);
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (p) 
		p->fillColor(c[0], c[1], c[2], c[3]);
	return R_NilValue;
}

#ifndef GLUT
extern "C" { void *ACocoa_CreateWindow(AVisual *visual, APoint position);  }

SEXP A_WindowCreate(SEXP sVis, SEXP sPos)
{
	AVisual *vis = (AVisual*) SEXP2A(sVis);
	double *pos = REAL(sPos);
	void *win = ACocoa_CreateWindow(vis, AMkPoint(pos[0], pos[1]));
	return PTR2SEXP(win);
}
#else
#include "AGLUTWindow.h"

SEXP A_WindowCreate(SEXP sVis, SEXP sPos)
{
	AVisual *vis = (AVisual*) SEXP2A(sVis);
	double *pos = REAL(sPos);
	ARect frame = vis->frame();
	AGLUTWindow *win = new AGLUTWindow(AMkRect(pos[0], pos[1], frame.width, frame.height));
	return PTR2SEXP(win);
}
#endif

SEXP A_ScatterPlot(SEXP x, SEXP y, SEXP rect)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	ADataVector *yv = (ADataVector*) SEXP2A(y);
	double *rv = REAL(rect);
	AScatterPlot *sp = new AScatterPlot(NULL, AMkRect(rv[0], rv[1], rv[2], rv[3]), 0, xv, yv);
	return A2SEXP(sp);
}

SEXP A_BarPlot(SEXP x, SEXP rect)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	if (!xv->isFactor()) Rf_error("x must be a factor");
	double *rv = REAL(rect);
	ABarChart *sp = new ABarChart(NULL, AMkRect(rv[0], rv[1], rv[2], rv[3]), 0, (AFactorVector*) xv);
	return A2SEXP(sp);
}

SEXP A_PCPPlot(SEXP vl, SEXP rect)
{
	vsize_t n = LENGTH(vl);
	double *rv = REAL(rect);
	ADataVector **dv = (ADataVector**) malloc(sizeof(ADataVector*) * n);
	for (vsize_t i = 0; i < n; i++)
		dv[i] = (ADataVector*) SEXP2A(VECTOR_ELT(vl, i));
	AParallelCoordPlot *pcp = new AParallelCoordPlot(NULL, AMkRect(rv[0], rv[1], rv[2], rv[3]), 0, n, dv);
	free(dv);
	return A2SEXP(pcp);
}
