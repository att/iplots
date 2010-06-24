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
#include "ATimeSeriesPlot.h"
#include "ARVector.h"
#include "AColorMap.h"

// R API to Acinonyx
extern "C" {
	SEXP A_Init();
	SEXP A_ReleaseObject(SEXP o);
	SEXP A_Describe(SEXP o);

	SEXP A_CONS(SEXP head, SEXP tail);
	
	SEXP A_MarkerCreate(SEXP len);
	SEXP A_MarkerAdd(SEXP m, SEXP o);
	SEXP A_MarkerSelected(SEXP m);
	SEXP A_MarkerValues(SEXP m);
	SEXP A_MarkerSelect(SEXP m, SEXP sel);
	SEXP A_MarkerSetValues(SEXP sM, SEXP sel);
	SEXP A_MarkerDependentCreate(SEXP sM, SEXP fun);

	SEXP A_WindowCreate(SEXP w, SEXP pos);
	SEXP A_WindowMoveAndResize(SEXP w, SEXP pos, SEXP size);
	
	SEXP A_ContainerCreate(SEXP sParent /* can be NULL */, SEXP sRect, SEXP sFlags);
	SEXP A_ContainerAdd(SEXP sCont, SEXP sVis);
	
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
	SEXP A_VPSetHidden(SEXP vp, SEXP hf);
	SEXP A_VPGetHidden(SEXP vp);
	SEXP A_VPSetQuery(SEXP vp, SEXP txt);
	SEXP A_VPGetQuery(SEXP vp);
	
	SEXP A_PolygonSetPoints(SEXP vp, SEXP xp, SEXP yp);

	SEXP A_VisualSetFrame(SEXP sPlot, SEXP sFrame);
	SEXP A_VisualGetFrame(SEXP sPlot);
	
	SEXP A_PlotPrimitives(SEXP sPlot);
	SEXP A_PlotAddPrimitive(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotAddPrimitives(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotRemovePrimitive(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotRemovePrimitives(SEXP sPlot, SEXP sPrim);
	SEXP A_PlotRemoveAllPrimitives(SEXP sPlot);
	SEXP A_PlotScale(SEXP sPlot, SEXP sSNR);
	SEXP A_PlotScales(SEXP sPlot);
	SEXP A_PlotCaption(SEXP sPlot);
	SEXP A_PlotRedraw(SEXP sPlot, SEXP sRoot);
	SEXP A_PlotValue(SEXP sPlot);
	SEXP A_PlotSetValue(SEXP sPlot, SEXP sValue);
	SEXP A_PlotDoubleProperty(SEXP sPlot, SEXP pName);
	SEXP A_PlotSetDoubleProperty(SEXP sPlot, SEXP pName, SEXP sValue);
	SEXP A_PlotPrimaryMarker(SEXP sPlot);
	
	SEXP A_ScatterPlot(SEXP x, SEXP y, SEXP rect, SEXP flags);
	SEXP A_TimePlot(SEXP x, SEXP y, SEXP rect, SEXP flags);
	SEXP A_BarPlot(SEXP x, SEXP rect, SEXP flags);
	SEXP A_HistPlot(SEXP x, SEXP rect, SEXP flags);
	SEXP A_PCPPlot(SEXP vl, SEXP rect, SEXP flags);
}


/*------------- internal functions --------------*/

static void AObjFinalizer(SEXP ref) {
	if (TYPEOF(ref) == EXTPTRSXP) {
		AObject *o = (AObject*) R_ExternalPtrAddr(ref);
		if (o) o->release();
	}
}

SEXP A2SEXP(AObject *o) {
	SEXP xp = R_MakeExternalPtr(o, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(xp, AObjFinalizer, TRUE);
	return xp;
}

static SEXP PTR2SEXP(void *ptr) {
	return R_MakeExternalPtr(ptr, R_NilValue, R_NilValue);
}

AObject *SEXP2A(SEXP o) {
	if (TYPEOF(o) != EXTPTRSXP)
		Rf_error("invalid object");
	return (AObject*) R_ExternalPtrAddr(o);
}


/*------------- initialization --------------*/

#ifdef HAVE_AQUA
/* we use Quartz to make sure the event loop is running */
#undef DEBUG
#include <R_ext/QuartzDevice.h>

#define QCF_SET_PEPTR  1  /* set ProcessEvents function pointer */
#define QCF_SET_FRONT  2  /* set application mode to front */

extern "C" {
	void QuartzCocoa_SetupEventLoop(int flags, unsigned long latency); /* from qdCocoa */
	void ACocoa_Init(); /* from CocoaApp */
}

#endif

static bool inside_Rapp = false;

static AColorMap *defaultColorMap;

SEXP A_Init() {
#ifdef HAVE_AQUA
	const char* rapp=getenv("R_GUI_APP_VERSION");
	if (!rapp || !rapp[0]) { /* if we're not inside R.app, use Quartz to start the event loop */
		QuartzFunctions_t *qf = getQuartzFunctions();
		if (qf) {
			/* check embedding parameters to see if Rapp (or other Cocoa app) didn't do the work for us */
			int eflags = 0;
			int *p_eflags = (int*) qf->GetParameter(NULL, QuartzParam_EmbeddingFlags);
			if (p_eflags) eflags = p_eflags[0];
			if ((eflags & QP_Flags_CFLoop) && (eflags & QP_Flags_Cocoa) && (eflags & QP_Flags_Front)) {
				return R_NilValue; /* ok, all is set already */
			}
			ACocoa_Init();
#if 0 // it's inside grDevices.so - so we cannot reach it ...
			if ((eflags & QP_Flags_CFLoop) == 0) /* no event loop? start one ... */
				QuartzCocoa_SetupEventLoop(QCF_SET_PEPTR|QCF_SET_FRONT, 100);
#endif
		}
	} else /* we're in R.app, all should be set */
		inside_Rapp = true;
#endif
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

/** A_CONS is simply CONS - strangely enough R has completely hidden this useful feature at the R level AFAICT
 *  @param head the CAR part of the entry
 *  @param tail the CDR part of the entry (must be a pairlist or NULL)
 *  @return pairlist with head prepended */
SEXP A_CONS(SEXP head, SEXP tail) {
	return CONS(head, tail);
}

#undef cons
#undef lcons

/*------------- variables (ADataVector) --------------*/

SEXP A_VarRegister(SEXP v, SEXP mark, SEXP sName)
{
	AObject *vo = NULL;
	AMarker *m = (mark == R_NilValue) ? NULL : (AMarker*) SEXP2A(mark);
	if (TYPEOF(v) == REALSXP) {
		if (Rf_inherits(v, "POSIXct"))
			vo = new ARTimeVector(m, v);
		else
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
	if (!defaultColorMap)
		defaultColorMap = new ADefaultColorMap();
	// Note: we never release the default color map. That is ok since it's supposed to live forever, but ...
	m->setColorMap(defaultColorMap);
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

SEXP A_MarkerValues(SEXP sM)
{
	AMarker *m = (AMarker*) SEXP2A(sM);
	if (!m) Rf_error("invalid marker (NULL)");
	vsize_t n = m->length();
	SEXP res = Rf_allocVector(INTSXP, n);
	int *l = INTEGER(res);
	for (vsize_t i = 0; i < n; i++)
		l[i] = m->value(i);
	return res;
}

SEXP A_MarkerSetValues(SEXP sM, SEXP sel)
{
	AMarker *m = (AMarker*) SEXP2A(sM);
	if (!m) Rf_error("invalid marker (NULL)");
	vsize_t n = m->length();
	if (TYPEOF(sel) == INTSXP) {
		m->begin();
		int *l = INTEGER(sel);
		vsize_t ll = LENGTH(sel);
		for (vsize_t i = 0; i < ll && i < n; i++)
			m->setValue(i, l[i]);
		m->end();
	} else Rf_error("value must be an integer vector");
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

static int visual_flags(SEXP sFlags) {
	int flags = 0;
	if (TYPEOF(sFlags) == INTSXP) {
		vsize_t n = LENGTH(sFlags);
		int *fi = INTEGER(sFlags);
		for (vsize_t i = 0; i < n; i++)
			flags |= fi[i];
	} else if (sFlags != R_NilValue)
		Rf_error("invalid flags");
	return flags;
}

static ARect visual_frame(SEXP sFrame) {
	if (TYPEOF(sFrame) != REALSXP || LENGTH(sFrame) != 4)
		Rf_error("invalid frame size specification");
	double *d = REAL(sFrame);
	return AMkRect(d[0], d[1], d[2], d[3]);
}

SEXP A_ContainerCreate(SEXP sParent /* can be NULL */, SEXP sRect, SEXP sFlags)
{
	AContainer *co = (AContainer*) ((sParent == R_NilValue) ? NULL : SEXP2A(sParent));
	AContainer *nc = new AContainer(co, visual_frame(sRect),  visual_flags(sFlags));
	if (!nc) Rf_error("failed to create a container");
	if (co) co->add(*nc);
	return A2SEXP(nc);
}

SEXP A_ContainerAdd(SEXP sCont, SEXP sVis) {
	AContainer *co = (AContainer*) SEXP2A(sCont);
	AVisual *vi = (AVisual*) SEXP2A(sVis);
	if (co && vi)
		co->add(*vi);
	return sCont;
}

/*------------- AVisual ------------*/

SEXP A_VisualSetFrame(SEXP sPlot, SEXP sFrame)
{
	AVisual *pl = (AVisual*) SEXP2A(sPlot);
	if (pl) pl->setFrame(visual_frame(sFrame));
	return sPlot;
}

SEXP A_VisualGetFrame(SEXP sPlot)
{
	AVisual *pl = (AVisual*) SEXP2A(sPlot);
	if (!pl) Rf_error("invalid visual object (NULL)");
	ARect r = pl->frame();
	SEXP res = Rf_allocVector(REALSXP, 4);
	double *d = REAL(res);
	d[0] = r.x; d[1] = r.y; d[2] = r.width; d[3] = r.height;
	return res;
}


/*------------- APlot --------------*/

SEXP A_PlotAddPrimitive(SEXP sPlot, SEXP sPrim) {
	APlot *pl = (APlot*) SEXP2A(sPlot);
	AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(sPrim);
	if (pl && vp) pl->addPrimitive(vp);
	return sPlot;
}

SEXP A_PlotAddPrimitives(SEXP sPlot, SEXP sPrim) {
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (!pl) return sPlot;
	if (TYPEOF(sPrim) == LISTSXP) {
		while (sPrim != R_NilValue) {
			AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(CAR(sPrim));
			if (vp)
				pl->addPrimitive(vp);
			sPrim = CDR(sPrim);
		}
	} else if (TYPEOF(sPrim) == VECSXP) {
		vsize_t i = 0, n = LENGTH(sPrim);
		for (; i < n; i++) {
			AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(VECTOR_ELT(sPrim, i));
			if (vp)
				pl->addPrimitive(vp);
		}
	} else Rf_error("invalid primitive object");
	return sPlot;
}

SEXP A_PlotRemovePrimitive(SEXP sPlot, SEXP sPrim)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(sPrim);
	if (pl && vp) pl->removePrimitive(vp);
	return sPlot;
}

SEXP A_PlotRemovePrimitives(SEXP sPlot, SEXP sPrim) {
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (!pl) return sPlot;
	if (TYPEOF(sPrim) == LISTSXP) {
		while (sPrim != R_NilValue) {
			AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(CAR(sPrim));
			if (vp)
				pl->removePrimitive(vp);
			sPrim = CDR(sPrim);
		}
	} else if (TYPEOF(sPrim) == VECSXP) {
		vsize_t i = 0, n = LENGTH(sPrim);
		for (; i < n; i++) {
			AVisualPrimitive *vp = (AVisualPrimitive*) SEXP2A(VECTOR_ELT(sPrim, i));
			if (vp)
				pl->removePrimitive(vp);
		}
	} else Rf_error("invalid primitive object");
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

SEXP A_PlotCaption(SEXP sPlot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (!pl) return Rf_mkString("<NULL>");
	const char *cap = pl->caption();
	return cap ? Rf_mkString(cap) : A_Describe(sPlot);
}

SEXP A_PlotRedraw(SEXP sPlot, SEXP sRoot)
{
	APlot *pl = (APlot*) SEXP2A(sPlot);
	if (pl) {
		if (sRoot && TYPEOF(sRoot) == LGLSXP && LENGTH(sRoot) > 0 && LOGICAL(sRoot)[0])
			pl->setRedrawLayer(LAYER_ROOT);
		pl->redraw();
	}
	return sPlot;
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
	if ((TYPEOF(sValue) == LGLSXP || TYPEOF(sValue) == REALSXP || TYPEOF(sValue) == INTSXP) && LENGTH(sValue) > 0) {
		double value = 0.0;
		if (TYPEOF(sValue) == LGLSXP)
			value = (double) (LOGICAL(sValue)[0]);
		else if (TYPEOF(sValue) == INTSXP)
			value = (double) (INTEGER(sValue)[0]);
		else
			value = REAL(sValue)[0];
		return Rf_ScalarLogical(pl ? pl->setDoubleProperty(CHAR(STRING_ELT(pName, 0)), value) : FALSE);
	} else
		return Rf_ScalarLogical(FALSE);
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
	if (p && p->plot()) { // FIXME: we do this due to issues with layers ...!
		p->plot()->setRedrawLayer(LAYER_ROOT);
		p->plot()->redraw();
	}
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

SEXP A_VPSetQuery(SEXP vp, SEXP txt) {
	AScaledPrimitive *p = (AScaledPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid object (NULL)");
	if (txt == R_NilValue || (TYPEOF(txt) == STRSXP && LENGTH(txt) == 0)) {
		p->setQueryText(NULL, 0);
		p->setQueryText(NULL, 1);
	} else if (TYPEOF(txt) == STRSXP) {
		p->setQueryText(CHAR(STRING_ELT(txt, 0)), 0);
		if (LENGTH(txt) > 1)
			p->setQueryText(CHAR(STRING_ELT(txt, 1)), 1);
	} else Rf_error("invalid query string");
	return vp;
}

SEXP A_VPGetQuery(SEXP vp) {
	AScaledPrimitive *p = (AScaledPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid object (NULL)");
	const char *q0 = p->queryText(0), *q1 = p->queryText(1);
	SEXP r = PROTECT(Rf_allocVector(STRSXP, q0 ? (q1 ? 2 : 1) : 0));
	if (q0) SET_STRING_ELT(r, 0, Rf_mkChar(q0));
	if (q0 && q1) SET_STRING_ELT(r, 1, Rf_mkChar(q1));
	UNPROTECT(1);
	return r;
}

SEXP A_VPSetHidden(SEXP vp, SEXP hf)
{
	if (LENGTH(hf) < 1 && TYPEOF(hf) != LGLSXP)
		Rf_error("invalid value for hidden flag");
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (p) p->setHidden(LOGICAL(hf)[0]);
	return vp;
}

SEXP A_VPGetHidden(SEXP vp)
{
	AVisualPrimitive *p = (AVisualPrimitive*) SEXP2A(vp);
	if (!p) Rf_error("invalid visual primitive object (NULL)");
	return Rf_ScalarLogical(p->hidden());
}

SEXP A_PolygonSetPoints(SEXP vp, SEXP xp, SEXP yp)
{
	APolygonPrimitive *p = (APolygonPrimitive*) SEXP2A(vp);
	if (p) p->setPoints(REAL(xp), REAL(yp), LENGTH(xp));
	return vp;
}

/*------------- AWindow --------------*/

#ifndef GLUT
#if __APPLE__
extern "C" { void *ACocoa_CreateWindow(AVisual *visual, APoint position);  }

SEXP A_WindowCreate(SEXP sVis, SEXP sPos)
{
	AVisual *vis = (AVisual*) SEXP2A(sVis);
	double *pos = REAL(sPos);
	void *win = ACocoa_CreateWindow(vis, AMkPoint(pos[0], pos[1]));
	return PTR2SEXP(win);
}
#else
#ifdef WIN32
extern "C" { void *AWin32_CreateWindow(AVisual *visual, APoint position);  }

SEXP A_WindowCreate(SEXP sVis, SEXP sPos)
{
	AVisual *vis = (AVisual*) SEXP2A(sVis);
	double *pos = REAL(sPos);
	void *win = AWin32_CreateWindow(vis, AMkPoint(pos[0], pos[1]));
	return PTR2SEXP(win);
}
#endif
#endif
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

SEXP A_WindowMoveAndResize(SEXP w, SEXP pos, SEXP size)
{
	AWindow *win = (AWindow*) SEXP2A(w);
	if (win) {
		ARect frame = win->frame();
		if (pos != R_NilValue) {
			if (TYPEOF(pos) != REALSXP || LENGTH(pos) != 2)
				Rf_error("invalid position specification");
			frame.x = REAL(pos)[0];
			frame.y = REAL(pos)[1];
		}
		if (size != R_NilValue) {
			if (TYPEOF(size) != REALSXP || LENGTH(size) != 2)
				Rf_error("invalid size specification");
			frame.width = REAL(size)[0];
			frame.height = REAL(size)[1];
		}
		win->moveAndResize(frame);
	}
	return w;
}

/*------------- plot implementations --------------*/

SEXP A_ScatterPlot(SEXP x, SEXP y, SEXP rect, SEXP flags)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	ADataVector *yv = (ADataVector*) SEXP2A(y);
	AScatterPlot *sp = new AScatterPlot(NULL, visual_frame(rect), visual_flags(flags), xv, yv);
	return A2SEXP(sp);
}

SEXP A_TimePlot(SEXP x, SEXP y, SEXP rect, SEXP flags)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	ADataVector *yv = (ADataVector*) SEXP2A(y);
	ATimeSeriesPlot *sp = new ATimeSeriesPlot(NULL, visual_frame(rect), visual_flags(flags), xv, yv);
	return A2SEXP(sp);
}

SEXP A_BarPlot(SEXP x, SEXP rect, SEXP flags)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	if (!xv->isFactor()) Rf_error("x must be a factor");
	ABarChart *sp = new ABarChart(NULL, visual_frame(rect), visual_flags(flags), (AFactorVector*) xv);
	return A2SEXP(sp);
}

SEXP A_HistPlot(SEXP x, SEXP rect, SEXP flags)
{
	ADataVector *xv = (ADataVector*) SEXP2A(x);
	AHistogram *sp = new AHistogram(NULL, visual_frame(rect), visual_flags(flags), xv);
	return A2SEXP(sp);
}

SEXP A_PCPPlot(SEXP vl, SEXP rect, SEXP flags)
{
	vsize_t n = LENGTH(vl);
	ADataVector **dv = (ADataVector**) malloc(sizeof(ADataVector*) * n);
	for (vsize_t i = 0; i < n; i++)
		dv[i] = (ADataVector*) SEXP2A(VECTOR_ELT(vl, i));
	AParallelCoordPlot *pcp = new AParallelCoordPlot(NULL, visual_frame(rect), visual_flags(flags), n, dv);
	free(dv);
	return A2SEXP(pcp);
}
