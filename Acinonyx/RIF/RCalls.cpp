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
#include "AHistogram.h"
#include "ARVector.h"

// R API to Acinonyx
extern "C" {
	SEXP A_Init();
	SEXP A_ReleaseObject(SEXP o);
	SEXP A_Describe(SEXP o);

	SEXP A_MarkerCreate(SEXP len);
	SEXP A_MarkerAdd(SEXP m, SEXP o);
	SEXP A_MarkerSelected(SEXP m);
	SEXP A_MarkerSelect(SEXP m, SEXP sel);
	SEXP A_MarkerDependentCreate(SEXP sM, SEXP fun);

	SEXP A_WindowCreate(SEXP w, SEXP pos);

	SEXP A_VarMark(SEXP v);
	SEXP A_VarRegister(SEXP v, SEXP mark, SEXP sName);
	SEXP A_VarName(SEXP v);
	
	SEXP A_ScaleValue(SEXP sScale, SEXP sPos);
	SEXP A_ScalePosition(SEXP sScale, SEXP sPos);

	SEXP A_LineCreate(SEXP pos);
	SEXP A_BarCreate(SEXP pos);
	SEXP A_PolygonCreate(SEXP sx, SEXP sy);
	SEXP A_TextCreate(SEXP pos, SEXP text);

	SEXP A_VPSetFill(SEXP vp, SEXP col);
	SEXP A_VPSetColor(SEXP vp, SEXP col);
	SEXP A_VPGetFill(SEXP vp);
	SEXP A_VPGetColor(SEXP vp);
	SEXP A_VPRedraw(SEXP vp);
	SEXP A_VPPlot(SEXP vp);
	SEXP A_VPSetCallback(SEXP vp, SEXP cb);
	SEXP A_VPGetCallback(SEXP vp);

	SEXP A_PolygonSetPoints(SEXP vp, SEXP xp, SEXP yp);

	SEXP A_PlotPrimitives(SEXP sPlot);
	SEXP A_PlotAddPrimitive(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotRemovePrimitive(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotRemoveAllPrimitives(SEXP sPlot);
	SEXP A_PlotScale(SEXP sPlot, SEXP sSNR);
	SEXP A_PlotScales(SEXP sPlot);
	SEXP A_PlotRedraw(SEXP sPlot);
	SEXP A_PlotValue(SEXP sPlot);
	SEXP A_PlotSetValue(SEXP sPlot, SEXP sValue);
	SEXP A_PlotDoubleProperty(SEXP sPlot, SEXP pName);
	SEXP A_PlotSetDoubleProperty(SEXP sPlot, SEXP pName, SEXP sValue);
	SEXP A_PlotPrimaryMarker(SEXP sPlot);
	
	SEXP A_ScatterPlot(SEXP x, SEXP y, SEXP rect);
	SEXP A_BarPlot(SEXP x, SEXP rect);
	SEXP A_HistPlot(SEXP x, SEXP rect);
	SEXP A_PCPPlot(SEXP vl, SEXP rect);
}


/*------------- internal functions --------------*/

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


/*------------- initialization --------------*/

SEXP A_Init() {
	return R_NilValue;
}

#ifndef cons
#define cons Rf_cons
#define lcons Rf_lcons
#endif

void call_with_object(SEXP fun, AObject *o, const char *clazz) {
	SEXP sSelf = A2SEXP(o);
	o->retain();
	PROTECT(sSelf);
	Rf_setAttrib(sSelf, R_ClassSymbol, Rf_mkString(clazz));
	SEXP sCall = LCONS(fun, CONS(sSelf, R_NilValue));
	PROTECT(sCall);
	Rf_applyClosure(sCall, fun, CDR(sCall), R_GlobalEnv, R_BaseEnv);
	UNPROTECT(2);
}

#undef cons
#undef lcons

/*------------- variables (ADataVector) --------------*/

SEXP A_VarRegister(SEXP v, SEXP mark, SEXP sName)
{
	AObject *vo = NULL;
	AMarker *m = (mark == R_NilValue) ? NULL : (AMarker*) SEXP2A(mark);
	if (TYPEOF(v) == REALSXP) {
		vo = new ARDoubleVector(m, v);
	} else if (TYPEOF(v) == INTSXP) {
		if (Rf_inherits(v, "factor"))
			vo = new ARFactorVector(m, v);
		else 
			vo = new ARIntVector(m, v);
	} else Rf_error("unsupported data type");
	if (vo && TYPEOF(sName) == STRSXP && LENGTH(sName) > 0)
		((ADataVector*)vo)->setName(CHAR(STRING_ELT(sName, 0)));
	return A2SEXP(vo);	
}

SEXP A_VarMark(SEXP v) {
	ADataVector *dv = (ADataVector*) SEXP2A(v);
	if (!dv) Rf_error("invalid variable (NULL)");
	AMarker *mark = dv->marker();
	return mark ? A2SEXP(mark) : R_NilValue;
}

SEXP A_VarName(SEXP v) {
	ADataVector *dv = (ADataVector*) SEXP2A(v);
	if (!dv) Rf_error("invalid variable (NULL)");
	return dv->name() ? Rf_mkString(dv->name()) : R_NilValue;
}

/*------------- AObject --------------*/

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


/*------------- AMarker --------------*/

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

SEXP A_MarkerSelected(SEXP sM)
{
	AMarker *m = (AMarker*) SEXP2A(sM);
	if (!m) Rf_error("invalid marker (NULL)");
	vsize_t n = m->length();
	SEXP res = Rf_allocVector(LGLSXP, n);
	int *l = LOGICAL(res);
	for (vsize_t i = 0; i < n; i++)
		l[i] = m->isSelected(i) ? 1 : 0;
	return res;
}

SEXP A_MarkerSelect(SEXP sM, SEXP sel)
{
	AMarker *m = (AMarker*) SEXP2A(sM);
	if (!m) Rf_error("invalid marker (NULL)");
	vsize_t n = m->length();
	if (TYPEOF(sel) == LGLSXP) { /* logical */
		if (LENGTH(sel) != n)
			Rf_error("length mismatch");
		m->begin();
		int *l = LOGICAL(sel);
		for (vsize_t i = 0; i < n; i++)
			if (l[i] == 1)
				m->select(i);
			else if (l[i] == 0)
				m->deselect(i);
		m->end();
	} else if (TYPEOF(sel) == INTSXP) {
		m->begin();
		int *l = INTEGER(sel);
		vsize_t ll = LENGTH(sel);
		for (vsize_t i = 0; i < ll; i++)
			if (l[i] > 0)
				m->select(l[i] - 1);
			else if (l[i] < 0)
				m->deselect(-l[i]);
		m->end();
	} else Rf_error("invalid selection specification (must be integer or logical vector)");
	return sM;
}

SEXP A_MarkerDependentCreate(SEXP sM, SEXP fun)
{
	AMarker *m = (AMarker*) SEXP2A(sM);
	if (!m) Rf_error("invalid marker (NULL)");
	ARCallbackDependent *dep = new ARCallbackDependent(fun);
	m->add(dep);
	// dep->release(); // markers don't retain ...
	return sM;
}


/*------------- APlot --------------*/

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

SEXP A_PlotRemoveAllPrimitives(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (pl) pl->removeAllPrimitives();
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

SEXP A_PlotRedraw(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (pl) pl->redraw();
	return R_NilValue;
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

SEXP A_PlotValue(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	return pl ? pl->value() : R_NilValue;
}
	
SEXP A_PlotSetValue(SEXP sPlot, SEXP sValue)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (pl) pl->setValue(sValue);
	return sPlot;
}

SEXP A_PlotDoubleProperty(SEXP sPlot, SEXP pName)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	return pl ? Rf_ScalarReal(pl->doubleProperty(CHAR(STRING_ELT(pName, 0)))) : R_NilValue;
}

SEXP A_PlotSetDoubleProperty(SEXP sPlot, SEXP pName, SEXP sValue)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	return Rf_ScalarLogical(pl ? pl->setDoubleProperty(CHAR(STRING_ELT(pName, 0)), REAL(sValue)[0]) : FALSE);
}

SEXP A_PlotPrimaryMarker(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	AMarker *m = pl ? pl->primaryMarker() : 0;
	return m ? A2SEXP(m) : R_NilValue;
}


/*------------- AScale --------------*/

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


/*------------- AVisualPrimitive --------------*/

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
	vsize_t n = LENGTH(sx);
	for(vsize_t i = 0; i < n; i++)
		pt[i] = AMkPoint(x[i], y[i]);
	APolygonPrimitive *p = new APolygonPrimitive(NULL, pt, n, false);
	return A2SEXP(p);
}

SEXP A_TextCreate(SEXP pos, SEXP text) {
	double *pp = REAL(pos);
	ATextPrimitive* p = new ATextPrimitive(NULL, AMkPoint(pp[0], pp[1]), CHAR(STRING_ELT(text,0)), true);
	return A2SEXP(p);
}

SEXP A_TextSetRotation(SEXP tp, SEXP rot) {
	ATextPrimitive *p = (ATextPrimitive*) SEXP2A(tp);
	if (p) p->setRotation(REAL(rot)[0]);
	return tp;
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

SEXP A_VPGetFill(SEXP vp) {
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid object (NULL)");
	AColor c = p->fillColor();
	SEXP rc = Rf_allocVector(REALSXP, 4);
	double *rcp = REAL(rc);
	rcp[0] = c.r; rcp[1] = c.g; rcp[2] = c.b; rcp[3] = c.a;
	return rc;
}

SEXP A_VPGetColor(SEXP vp) {
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid object (NULL)");
	AColor c = p->drawColor();
	SEXP rc = Rf_allocVector(REALSXP, 4);
	double *rcp = REAL(rc);
	rcp[0] = c.r; rcp[1] = c.g; rcp[2] = c.b; rcp[3] = c.a;
	return rc;
}

SEXP A_VPPlot(SEXP vp)
{
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	return (p && p->plot()) ? A2SEXP(p->plot()) : R_NilValue;
}

SEXP A_VPRedraw(SEXP vp) {
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (p && p->plot()) p->plot()->redraw();
	return vp;
}

SEXP A_VPSetCallback(SEXP vp, SEXP cb)
{
	ARCallbackPrimitive *p = (ARCallbackPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid object (NULL)");
	p->setValue(cb);
	return vp;
}

SEXP A_VPGetCallback(SEXP vp)
{
	ARCallbackPrimitive *p = (ARCallbackPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid object (NULL)");
	return p->value();
}

SEXP A_PolygonSetPoints(SEXP vp, SEXP xp, SEXP yp)
{
	APolygonPrimitive *p = (APolygonPrimitive*) SEXP2A(vp);
	if (p) p->setPoints(REAL(xp), REAL(yp), LENGTH(xp));
	return vp;
}

/*------------- AWindow --------------*/

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


/*------------- plot implementations --------------*/

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

SEXP A_HistPlot(SEXP x, SEXP rect)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	double *rv = REAL(rect);
	AHistogram *sp = new AHistogram(NULL, AMkRect(rv[0], rv[1], rv[2], rv[3]), 0, xv);
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
