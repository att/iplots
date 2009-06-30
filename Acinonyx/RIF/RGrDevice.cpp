/*
 *  RRAcinonyxDevice.cpp - Acinonyx-based R graphics device
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef USE_RINTERNALS
#define USE_RINTERNALS 1
#endif
#include "RObject.h"
#include "RCalls.h"
#include "AVisual.h"

#ifndef R_USE_PROTOTYPES
#define R_USE_PROTOTYPES 1
#endif
#include <R_ext/GraphicsEngine.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#undef _
#define _(String) dgettext ("grDevices", String)
#else
#define _(String) (String)
#endif

#ifdef SUPPORT_MBCS
#include <wchar.h>
#endif

extern "C" {
	SEXP RAcinonyxDevice(SEXP args);
}

/* device name passed down to R */
static const char *device_name = "Acinonyx";

class ARGraphicsDevice;

typedef struct DeviceSpecific_s {
    pDevDesc      dev;             /* device structure holding this one */
	ARGraphicsDevice *agd;         /* associated Acinonyx visual */
    double        ps;              /* point size */
    double        width, height;   /* size (in device coordinates) */
	double        dpix, dpiy;      /* mapping from device coords to real-word coords */
    int           bg;              /* background color */
    int           canvas;          /* background color */
    int           flags;           /* additional flags */
    int           redraw;          /* redraw flag is set when replaying
		                              and inhibits syncs on Mode */

} DeviceSpecific_t;

class ARGraphicsDevice : public AVisual {
protected:
	DeviceSpecific_t *dss;
public:
	ARGraphicsDevice(AContainer *parent, DeviceSpecific_t *_dss, ARect frame, unsigned int flags) : AVisual(parent, frame, flags), dss(_dss) {
		OCLASS(ARGraphicsDevice)
	}
	
	virtual void draw() {
		/* int _dirty = qd->dirty; */
		if (dss && dss->dev) {
			pGEDevDesc gdd = desc2GEDesc(dss->dev);
			dss->redraw = 1;
			if (gdd->displayList != R_NilValue)
				GEplayDisplayList(gdd);
			dss->redraw = 0;
		}
		// qd->dirty = _dirty; /* we do NOT change the dirty flag */
	}
	
	virtual void moveAndResize(ARect frame) {
		if (!ARectsAreEqual(frame, _frame)) {
			dss->dev->right = dss->width = frame.width;
			dss->dev->bottom = dss->height = frame.height;
			AVisual::moveAndResize(frame);
			redraw();
		} else
			AVisual::moveAndResize(frame);
	}

	void close() {
	}
	
	void activate() {
	}
	
	void deactivate() {
	}
	
	void mode(int mode) {
	}
	
	void new_page() {
		clipOff();
		glClearColor(R_RED(dss->canvas), R_GREEN(dss->canvas), R_BLUE(dss->canvas), 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	bool prepare_color(int col) {
		if (R_TRANSPARENT(col)) return false;
		color(AMkColor(R_RED(col), R_GREEN(col), R_BLUE(col), R_ALPHA(col)));
		return true;
	}							
};

#if 0
void RAcinonyxDevice_ReplayDisplayList(DeviceSpecific_t *desc)
{
    DeviceSpecific_t *qd = (DeviceSpecific_t*) desc;
    int _dirty = qd->dirty;
    pGEDevDesc gdd = desc2GEDesc(qd->dev);
    qd->redraw = 1;
    /* CHECK this */
    if(gdd->displayList != R_NilValue) GEplayDisplayList(gdd);
    qd->redraw = 0;
    qd->dirty = _dirty; /* we do NOT change the dirty flag */
}

void* RAcinonyxDevice_GetSnapshot(DeviceSpecific_t *desc, int last)
{
    DeviceSpecific_t *qd = (DeviceSpecific_t*) desc;
    pGEDevDesc gd  = GEgetDevice(ndevNumber(qd->dev));
    SEXP snap;
    if (last)
	snap = desc2GEDesc(qd->dev)->savedSnapshot;
    else
	snap = GEcreateSnapshot(gd);
    if (R_NilValue == VECTOR_ELT(snap, 0))
	snap = 0;
    return (snap == R_NilValue) ? 0 : snap;
}

void RAcinonyxDevice_RestoreSnapshot(DeviceSpecific_t *desc, void* snap)
{
    DeviceSpecific_t *qd = (DeviceSpecific_t*) desc;
    pGEDevDesc gd  = GEgetDevice(ndevNumber(qd->dev));
    if(NULL == snap) return; /*Aw, hell no!*/
    PROTECT((SEXP)snap);
    if(R_NilValue == VECTOR_ELT(snap,0))
        warning("Tried to restore an empty snapshot?");
    qd->redraw = 1;
    GEplaySnapshot((SEXP)snap, gd);
    qd->redraw = 0;
    qd->dirty = 0; /* we reset the dirty flag */
    UNPROTECT(1);
}
/*------------------------------------------------------------ end of unnecessary stuff */
#endif


#pragma mark RGD API Function Prototypes

static void     RAcinonyxDevice_Close(pDevDesc);
static void     RAcinonyxDevice_Activate(pDevDesc);
static void     RAcinonyxDevice_Deactivate(pDevDesc);
static void     RAcinonyxDevice_Size(double*, double*, double*, double*, pDevDesc);
static void     RAcinonyxDevice_NewPage(const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Clip(double, double, double, double, pDevDesc);
static double   RAcinonyxDevice_StrWidth(const char*, const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Text(double, double, const char*, double, double, const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Rect(double, double, double, double, const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Circle(double, double, double, const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Line(double, double, double, double, const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Polyline(int, double*, double*, const pGEcontext, pDevDesc);
static void     RAcinonyxDevice_Polygon(int, double*, double*, const pGEcontext, pDevDesc);
static Rboolean RAcinonyxDevice_Locator(double*, double*, pDevDesc);
static void     RAcinonyxDevice_Mode(int mode, pDevDesc);
static void     RAcinonyxDevice_MetricInfo(int, const pGEcontext , double*, double*, double*, pDevDesc);

#pragma mark Device implementation

static DeviceSpecific_t* RAcinonyxDevice_Create(pDevDesc dev, double width, double height, double ps, int bg, int canvas, double dpix, double dpiy, int flags)
{
	/* just a brief DPI sanity check -- NaN / NA / 0.0 can be used as auto-detect */
	if (ISNAN(dpix) || dpix < 0.01) dpix = 72.0;
	if (ISNAN(dpix) || dpiy < 0.01) dpiy = 72.0;
	
    dev->startfill = bg;
    dev->startcol  = R_RGB(0, 0, 0);
    dev->startps   = ps;
    dev->startfont = 1;
    dev->startlty  = LTY_SOLID;
    dev->startgamma= 1.8;

    dev->close        = RAcinonyxDevice_Close;
    dev->activate     = RAcinonyxDevice_Activate;
    dev->deactivate   = RAcinonyxDevice_Deactivate;
    dev->size         = RAcinonyxDevice_Size;
    dev->newPage      = RAcinonyxDevice_NewPage;
    dev->clip         = RAcinonyxDevice_Clip;
    dev->strWidth     = RAcinonyxDevice_StrWidth;
    dev->text         = RAcinonyxDevice_Text;
    dev->rect         = RAcinonyxDevice_Rect;
    dev->circle       = RAcinonyxDevice_Circle;
    dev->line         = RAcinonyxDevice_Line;
    dev->polyline     = RAcinonyxDevice_Polyline;
    dev->polygon      = RAcinonyxDevice_Polygon;
    dev->locator      = RAcinonyxDevice_Locator;
    dev->mode         = RAcinonyxDevice_Mode;
    dev->metricInfo   = RAcinonyxDevice_MetricInfo;
    dev->hasTextUTF8  = TRUE;
    dev->textUTF8     = RAcinonyxDevice_Text;
    dev->strWidthUTF8 = RAcinonyxDevice_StrWidth;

    dev->left = 0;
    dev->top  = 0;

    dev->xCharOffset = 0.4900;
    dev->yCharOffset = 0.3333;
    dev->yLineBias   = 0.20; /* This is .2 for PS/PDF devices... */

	dev->cra[0] = 0.9 * ps;
    dev->cra[1] = 1.2 * ps;
    dev->ipr[0] = 1.0 / dpix;
    dev->ipr[1] = 1.0 / dpiy;
	
    dev->canClip       = TRUE;
    dev->canHAdj       = 2;
    dev->canChangeGamma= FALSE;
    dev->displayListOn = TRUE;

    DeviceSpecific_t *qd = (DeviceSpecific_t*) calloc(1, sizeof(DeviceSpecific_t));
	if (!qd)
		Rf_error(_("unable to allocate memory for a graphics device"));

    qd->width      = width;
    qd->height     = height;
    qd->ps         = ps;
    qd->bg         = bg;
    qd->canvas     = canvas;
	qd->dpix       = dpix;
	qd->dpiy       = dpiy;
	qd->flags      = flags;

    dev->deviceSpecific = qd;
    qd->dev             = dev;

	dev->right = width;
	dev->bottom= height;

	qd->agd = new ARGraphicsDevice(NULL, qd, AMkRect(100.0, 100.0, width, height), AVF_XSPRING | AVF_YSPRING);

    return qd;
}

static void RAcinonyxDevice_Close(pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) xd->agd->close();
}

static void RAcinonyxDevice_Activate(pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) xd->agd->activate();
}

static void RAcinonyxDevice_Deactivate(pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) xd->agd->deactivate();
}

static void RAcinonyxDevice_Size(double *left, double *right, double *bottom, double *top, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
    *left = *top = 0;
    *right  = xd->width;
    *bottom = xd->height;
}

static void RAcinonyxDevice_NewPage(const pGEcontext gc, pDevDesc dd)
{
	DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) xd->agd->new_page();
}

static void RAcinonyxDevice_Clip(double x0, double x1, double y0, double y1, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	//if (xd->agd) xd->agd->clip(AMkRect(x0, y0, x1 - x0, y1 - y0));
}

static double RAcinonyxDevice_StrWidth(const char *text, const pGEcontext gc, pDevDesc dd)
{
	DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	return strlen(text) * 8.0 * xd->ps;
}

static void RAcinonyxDevice_Text(double x, double y, const char *text, double rot, double hadj, const pGEcontext gc, pDevDesc dd)
{
	DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd && xd->agd->prepare_color(gc->col))
		xd->agd->text(AMkPoint(x,y), text, AMkPoint(0, hadj), rot);
}

static void RAcinonyxDevice_Rect(double x0, double y0, double x1, double y1, const pGEcontext gc, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) {
		if (xd->agd->prepare_color(gc->fill))
			xd->agd->rectO(x0, y0, x1, y1);
		if (xd->agd->prepare_color(gc->col))
			xd->agd->rect(x0, y0, x1, y1);
	}
}

static void RAcinonyxDevice_Circle(double x, double y, double r, const pGEcontext gc, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) {
		if (xd->agd->prepare_color(gc->fill))
			xd->agd->circle(x, y, r);
		if (xd->agd->prepare_color(gc->col))
			xd->agd->circleO(x, y, r);
	}		
}

static void RAcinonyxDevice_Line(double x1, double y1, double x2, double y2, const pGEcontext gc, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd && xd->agd->prepare_color(gc->col)) xd->agd->line(x1, y1, x2, y2);
}

static void RAcinonyxDevice_Polyline(int n, double *x, double *y, const pGEcontext gc, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd && xd->agd->prepare_color(gc->col))
		xd->agd->polyline(x, y, n); // FIXME: this will only work as long as AFloat is double
}

static void RAcinonyxDevice_Polygon(int n, double *x, double *y, const pGEcontext gc, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;

    if (n < 2) return;
	if (xd->agd) {
		if (xd->agd->prepare_color(gc->fill))
			xd->agd->polygon(x, y, n);
		if (xd->agd->prepare_color(gc->col))
			xd->agd->polygonO(x, y, n);
	}
}

static void RAcinonyxDevice_Mode(int mode, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	if (xd->agd) xd->agd->mode(mode);
#if 0 /* jsut an idea ... */
    /* don't do anything in redraw as we can signal the end */
    if (xd->redraw) return;
    /* mode=0 -> drawing complete, signal sync */
    if (mode == 0)
		xd->sync(xd, xd->userInfo);
#endif
}

static void RAcinonyxDevice_MetricInfo(int c, const pGEcontext gc, double *ascent, double *descent, double *width, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;
	*ascent = 10.0;
	*descent = 2.0;
	*width = 8.0;
	
	/* if (c >= 0 && c <= ((mbcslocale && gc->fontface != 5) ? 127 : 255)) {
	} else {
	    single = (UniChar) ((c < 0) ? -c : c);
	    buffer = &single;
	    len = 1;
	} */
}

static Rboolean RAcinonyxDevice_Locator(double *x, double *y, pDevDesc dd)
{
    DeviceSpecific_t *xd = (DeviceSpecific_t*) dd->deviceSpecific;

    return FALSE;
}

#pragma mark -
#pragma mark R Interface

#define ARG(HOW,WHAT) HOW(CAR(WHAT));WHAT = CDR(WHAT)

/* ARGS: width, height, ps, bg, canvas, dpi */
SEXP RAcinonyxDevice(SEXP args)
{
    SEXP tmps;
    double   width, height, ps, dpix = 72.0, dpiy = 72.0;
    int      bg = -1, canvas = -1, flags = 0;
    DeviceSpecific_t *qd = NULL;

    char    *vmax = (char*) vmaxget();

    args = CDR(args); /* Skip the call */
    width     = ARG(Rf_asReal,args);
    height    = ARG(Rf_asReal,args);
    ps        = ARG(Rf_asReal,args);

    tmps      = CAR(args); args = CDR(args);
    bg        = RGBpar(tmps, 0);
	if (args != R_NilValue) {
		tmps      = CAR(args); args = CDR(args);
		canvas    = RGBpar(tmps, 0) | 0xff000000; /* force opaque */
		if (args != R_NilValue) {
			tmps      = CAR(args); args = CDR(args);
			if (!isNull(tmps)) {
				tmps = Rf_coerceVector(tmps, REALSXP);
				if (LENGTH(tmps) == 1) {
					dpix = dpiy = REAL(tmps)[0];
				} else if (LENGTH(tmps) > 1) {
					dpix = REAL(tmps)[0];
					dpiy = REAL(tmps)[0];
				}
			}
			if (args != R_NilValue) {
				flags = ARG(Rf_asInteger, args);
			}
		}
	}
    if (ISNAN(width) || ISNAN(height) || width <= 0.0 || height <= 0.0)
        Rf_error(_("invalid device size"));

    R_GE_checkVersionOrDie(R_GE_version);
    R_CheckDeviceAvailable();
    BEGIN_SUSPEND_INTERRUPTS {
		pDevDesc dev = (pDevDesc) calloc(1, sizeof(DevDesc));

		if (!dev)
			Rf_error(_("Unable to create device description."));

		qd = RAcinonyxDevice_Create(dev, width, height, ps, bg, canvas, dpix, dpiy, 0);

		if (qd == NULL) {
			vmaxset(vmax);
			free(dev);
			Rf_error(_("Unable to create graphics device."));
		}

		Rf_gsetVar(Rf_install(".Device"), Rf_mkString(device_name), R_BaseEnv); /* R_DeviceSymbol is only present in 2.10+ */
		pGEDevDesc dd = (pGEDevDesc) GEcreateDevDesc(dev);
		GEaddDevice(dd);
		GEinitDisplayList(dd);
    } END_SUSPEND_INTERRUPTS;

    vmaxset(vmax);
    return A2SEXP(qd->agd);
}
