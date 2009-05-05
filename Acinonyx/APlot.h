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
#include "ADataVector.h"

#define SEL_REPLACE 0
#define SEL_OR      1
#define SEL_XOR     2
#define SEL_AND     3
#define SEL_NOT     4

class APlot : public AContainer {
protected:
	AScale **scales;
	int nScales;
	
	AMutableObjectVector *vps;
	
public:
	APlot(AContainer *parent, ARect frame, int flags) :  AContainer(parent, frame, flags), nScales(0), scales(NULL), vps(new AMutableObjectVector(0)) { OCLASS(APlot) }

	APlot(AContainer *parent, ARect frame, int flags, AScale *xScale, AScale *yScale) : AContainer(parent, frame, flags), nScales(2), vps(new AMutableObjectVector(0)) {
		scales = (AScale**) malloc(sizeof(AScale*)*2);
		scales[0] = xScale;
		scales[1] = yScale;
		OCLASS(APlot)
	}

	virtual ~APlot() {
		if (scales) {
			for (int i = 0; i < nScales; i++) scales[i]->release();
			free(scales);
		}
		if (vps) vps->release();
		DCLASS(APlot)
	}
	//virtual void drawLayer(int l) = 0;
	
	virtual void update() { }; // this is called when gemotry update is requested (e.g. on resize)
	
	virtual void draw() {
		// draw all primitives
		if (vps) {
			vsize_t i = 0, n = vps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
				o->draw(*this);
			}
		}

		// draw interactive stuff
		if (inSelection || inZoom) {
			ARect r = AMkRect(dragStartPoint.x, dragStartPoint.y, dragEndPoint.x - dragStartPoint.x, dragEndPoint.y - dragStartPoint.y);
			if (r.width < 0) { r.x += r.width; r.width = -r.width; }
			if (r.height < 0) { r.y += r.height; r.height = -r.height; }
			if (inSelection)
				color(AMkColor(1.0,0.0,0.0,0.3));
			else if (inZoom)
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
	
	virtual void notification(AObject *source, notifid_t nid)
	{
		if (source != this) {
			if (nid == N_MarkerChanged)
				redraw();
			AContainer::notification(source, nid);
		}
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
				// FIXME: performZoom
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
	
	virtual bool performSelection(ARect where, int type) {
		printf("%s: perform selection: (%g,%g - %g,%g)\n", describe(), where.x, where.y, where.width, where.height);
		return false;
	}

	virtual void moveAndResize(ARect frame) {
		if (!ARectsAreEqual(frame, _frame)) {
			AContainer::moveAndResize(frame);
			update();
		}
	}
};

#endif
