/*
 *  APlot.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

// option 1) purely added functionality, the APlotVisual = AVisual + APlot
// option 2) subclass AVisual

#ifndef A_PLOT_H_
#define A_PLOT_H_

#include "AScale.h"
#include "AVisualPrimitive.h"
#include "AContainer.h"
#include "AStack.h"
#include "ADataVector.h"

#include "RValueHolder.h" // for variables stored in the plot object

#define SEL_REPLACE 0
#define SEL_OR      1
#define SEL_XOR     2
#define SEL_AND     3
#define SEL_NOT     4

class AZoomEntry : public AObject {
public:
	virtual ADataRange range(vsize_t coord) { return AUndefDataRange; };
};

class APlot : public AContainer, public RValueHolder {
protected:
	AScale **_scales;
	int nScales;
	
	AMutableObjectVector *vps; /* visual plot primitives (managed externally) */
	AObjectVector *pps;        /* plot primitives (used by the plot internally - usually statistical visuals) */
	AStack *zoomStack;
	AMarker *marker; // subclasses should use this marker if they want primitives to handle selection automatically. It is not used by the APlot class itself (except as pass-through to visual primitives)

	// those may not be needed, but since most plots use them it makes sense to keep them here
	AFloat mLeft, mTop, mBottom, mRight, ptSize, ptAlpha;

public:
	APlot(AContainer *parent, ARect frame, int flags) : AContainer(parent, frame, flags), RValueHolder(Rf_allocVector(VECSXP, 0)), nScales(0), pps(NULL), _scales(NULL), vps(new AMutableObjectVector()), zoomStack(new AStack()), marker(0), inSelection(false), inZoom(false), mLeft(20.0), mTop(10.0), mBottom(20.0), mRight(10.0), ptSize(5.0), ptAlpha(0.6) {
		OCLASS(APlot)
	}

	APlot(AContainer *parent, ARect frame, int flags, AScale *xScale, AScale *yScale) : AContainer(parent, frame, flags), RValueHolder(Rf_allocVector(VECSXP, 0)), nScales(2), pps(NULL), vps(new AMutableObjectVector()), inSelection(false), inZoom(false), mLeft(20.0), mTop(10.0), mBottom(20.0), mRight(10.0), ptSize(1.0), ptAlpha(1.0) {
		_scales = (AScale**) malloc(sizeof(AScale*)*2);
		_scales[0] = xScale;
		_scales[1] = yScale;
		OCLASS(APlot)
	}

	virtual ~APlot() {
		if (_scales) {
			for (int i = 0; i < nScales; i++) _scales[i]->release();
			free(_scales);
		}
		if (zoomStack) zoomStack->release();
		if (vps) vps->release();
		if (pps) pps->release();
		DCLASS(APlot)
	}
	//virtual void drawLayer(int l) = 0;
	
	vsize_t scales() { return nScales; }
	
	AScale *scale(vsize_t index) { return (index < nScales) ? _scales[index] : NULL; }
	
	// by default 0 is X scale and 1 is Y scale - other plots can override this
	virtual AScale *designatedScale(scale_t type) {
		if (type == XScale && nScales > 0) return _scales[0];
		if (type == YScale && nScales > 1) return _scales[1];
		return NULL;
	}
	
	AMarker *primaryMarker() { return marker; }

	virtual void home() { // scale back to the "home" scale
	}
	
	virtual void update() { // this is called when plot properties change (including plot is resize)
		// update plot primitives
		if (pps) {
			vsize_t i = 0, n = pps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i++);
				if (o) o->update();
			}
		}
		
		// update visual primitives
		if (vps) {
			vsize_t i = 0, n = vps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
				if (o) o->update();
			}
		}		
	}

	virtual double doubleProperty(const char *name) {
		AScale *s;
		if (!strcmp(name, "xlim.low") && (s = designatedScale(XScale))) return s->dataRange().begin;
		if (!strcmp(name, "ylim.low") && (s = designatedScale(YScale))) return s->dataRange().begin;
		if (!strcmp(name, "xlim.hi") && (s = designatedScale(XScale))) return s->dataRange().begin + s->dataRange().length;
		if (!strcmp(name, "ylim.hi") && (s = designatedScale(YScale))) return s->dataRange().begin + s->dataRange().length;
		if (!strcmp(name, "num.scales")) return nScales;
		return NA_REAL;
	}
	
	virtual bool setDoubleProperty(const char *name, double value) {
		AScale *s;
		if (!strcmp(name, "xlim.low") && (s = designatedScale(XScale))) { ADataRange r = s->dataRange(); r.begin = value; s->setDataRange(r); update(); redraw(); return true; }
		if (!strcmp(name, "ylim.low") && (s = designatedScale(YScale))) { ADataRange r = s->dataRange(); r.begin = value; s->setDataRange(r); update(); redraw(); return true; }
		if (!strcmp(name, "xlim.hi") && (s = designatedScale(XScale))) { ADataRange r = s->dataRange(); r.length = value - r.begin; s->setDataRange(r); update(); redraw(); return true; }
		if (!strcmp(name, "ylim.hi") && (s = designatedScale(YScale))) { ADataRange r = s->dataRange(); r.length = value - r.begin; s->setDataRange(r); update(); redraw(); return true; }
		return false;
	}
	
	virtual void draw() {
		// draw plot primitives
		if (pps) {
			vsize_t i = 0, n = pps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i++);
				// ALog("%s: draw", o->describe());
				o->draw(*this);
			}
		}

		// draw visual primitives
		if (vps) {
			vsize_t i = 0, n = vps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
				ALog("%s: draw", o->describe());
				o->draw(*this);
			}
		}

		// draw interactive stuff
		if (inSelection || inZoom) {
			ARect r = AMkRect(dragStartPoint.x, dragStartPoint.y, dragEndPoint.x - dragStartPoint.x, dragEndPoint.y - dragStartPoint.y);
			if (r.width < 0) { r.x += r.width; r.width = -r.width; }
			if (r.height < 0) { r.y += r.height; r.height = -r.height; }
			if (inSelection) {
				color(AMkColor(1.0,0.4,0.4,0.5));
			} else if (inZoom)
				color(AMkColor(0.0,0.0,1.0,0.3));				
			rect(r);
			if (inSelection)
				color(AMkColor(1.0,0.0,0.0,1.0));
			else if (inZoom)
				color(AMkColor(0.0,0.0,1.0,1.0));
			rectO(r);			
		}
		
		// draw all children
		AContainer::draw();
	}

	// in batch mode the marker's batch mode won't be chenged such that multiple selections can be performed (e.g. selection sequence). The correct usage then is: marker->begin(); performSelection(,,true); ...; marker->end();
	// this implementation uses stat primitives
	virtual bool performSelection(ARect where, int type, bool batch = false) {
		if (!marker) return false;
		if (!vps->length() && !pps && !pps->length()) return false;
		_prof(profReport("^performSelection %s", describe()))
		if (!batch) marker->begin();
		if (type == SEL_REPLACE)
			marker->deselectAll();
		bool pointSelection = (!where.width && !where.height);
		APoint point = AMkPoint(where.x, where.y);
		ALog("%s performSelection[%g,%g %g,%g] (point selection: %s)", describe(), ARect4(where), pointSelection?"YES":"NO");
		vsize_t i = 0, n;
		if (pps) { // plot primitives
			n = pps->length();
			while (i < n) {
				AVisualPrimitive *vp = (AVisualPrimitive*) pps->objectAt(i++);
				if ((!pointSelection && vp->intersects(where)) || (pointSelection && vp->containsPoint(point)))
					vp->select(marker, type);
			}
		}
		// visual primitives
		i = 0; n = vps->length();
		while (i < n) {
			AVisualPrimitive *vp = (AVisualPrimitive*) vps->objectAt(i++);
			if ((!pointSelection && vp->intersects(where)) || (pointSelection && vp->containsPoint(point)))
				vp->select(marker, type);
		}
		if (!batch) marker->end();
		_prof(profReport("$performSelection %s", describe()))
		return true;
	}
	
	virtual void notification(AObject *source, notifid_t nid)
	{
		if (source != this) {
			// notify plot primitives
			if (pps) {
				vsize_t i = 0, n = pps->length();
				while (i < n) {
					AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i++);
					if (o) o->notification(source, nid);
				}
			}
			
			// notify visual primitives
			if (vps) {
				vsize_t i = 0, n = vps->length();
				while (i < n) {
					AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
					if (o) o->notification(source, nid);
				}
			}		

			if (nid == N_MarkerChanged)
				redraw();
			AContainer::notification(source, nid);
		}
	}

	void addPrimitive(AVisualPrimitive *vp) {
		vp->setPlot(this);
		vps->addObject(vp);
	}
	
	void removePrimitive(AVisualPrimitive *vp) {
		if (vp->plot() == this) vp->setPlot(NULL);
		vps->removeObject(vp);
	}
	
	void removeAllPrimitives() {
		vsize_t n = vps->length();
		for (vsize_t i = 0; i < n; i++) {
			AVisualPrimitive *vp = (AVisualPrimitive*) vps->objectAt(i);
			if (vp) vp->setPlot(NULL);
		}
		vps->removeAll();
	}
	
	AObjectVector *primitives() {
		return vps;
	}
	
	// we should move this to another class - this is here for now ...
	
	bool inSelection, inZoom;
	APoint dragStartPoint, dragEndPoint;
	int selType;

	virtual bool mouseDown(AEvent e) {
		if (!containsPoint(e.location)) return false;
		if (e.flags & AEF_BUTTON1) {
			if (e.flags & AEF_META)
				inZoom = true;
			else {
				inSelection = true;
				selType = SEL_REPLACE;
				if (e.flags & AEF_SHIFT) {
					selType = SEL_XOR;
					if (e.flags & AEF_ALT)
						selType = SEL_AND;
				}
			}
			dragStartPoint = e.location;
			return true;
		}
		return false;
	}
	
	virtual bool mouseUp(AEvent e) {
		if ((e.flags & AEF_BUTTON1) && (inSelection || inZoom)) {
			dragEndPoint = e.location;
			ARect r = AMkRect(dragStartPoint.x, dragStartPoint.y, dragEndPoint.x - dragStartPoint.x, dragEndPoint.y - dragStartPoint.y);
			if (r.width < 0) { r.x += r.width; r.width = -r.width; }
			if (r.height < 0) { r.y += r.height; r.height = -r.height; }
			if (inSelection) {
				inSelection = false;
				inZoom = false;
				redraw();
				performSelection(r, selType);
			} else if (inZoom) {
				inZoom = false;
				inSelection = false;
				performZoom(r);
				redraw();				
			}
			return true;
		}
		return false;
	}
	
	virtual bool mouseMove(AEvent e) {
		if (inSelection || inZoom) {
			dragEndPoint = e.location;
			redraw();
			return true;
		}
		return false;
	}
	
	virtual bool performZoom(ARect where) {
		// printf("%s: perform selection: (%g,%g - %g,%g)\n", describe(), where.x, where.y, where.width, where.height);
		return false;
	}
	
	virtual void moveAndResize(ARect frame) {
		if (!ARectsAreEqual(frame, _frame)) {
			AContainer::moveAndResize(frame);
			update();
		}
	}
};

class AZoomEntryBiVar : public AZoomEntry {
protected:
	ADataRange r[2];
public:
	AZoomEntryBiVar(ADataRange r1, ADataRange r2) {
		r[0] = r1;
		r[1] = r2;
		OCLASS(AZoomEntryBiVar)
	}

	virtual ADataRange range(vsize_t coord) {
		if (coord == 0) return r[0];
		if (coord == 1) return r[1];
		return AUndefDataRange;
	}
};

#endif
