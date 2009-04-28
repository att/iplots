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

class APlot : public AContainer {
protected:
	AScale **scales;
	int nScales;
	
	AVisualPrimitive **vps;
	int nVps;
	
public:
	APlot(AContainer *parent, ARect frame, int flags) :  AContainer(parent, frame, flags), nScales(0), scales(NULL), nVps(0), vps(NULL) { OCLASS(APlot) }
	APlot(AContainer *parent, ARect frame, int flags, AScale *xScale, AScale *yScale) : AContainer(parent, frame, flags), nScales(2), nVps(0), vps(NULL) { scales = (AScale**) malloc(sizeof(AScale*)*2); scales[0] = xScale; scales[1] = yScale; OCLASS(APlot) }
	
	virtual ~APlot() {
		if (scales) {
			for (int i = 0; i < nScales; i++) scales[i]->release();
			free(scales);
		}
		if (vps) {
			for (int i = 0; i < nVps; i++) vps[i]->release();
			free(vps);
		}
		DCLASS(APlot)
	}
	//virtual void drawLayer(int l) = 0;
	
	// we should move this to another class - this is here for now ...
	
	bool inSelection;
	APoint selectionStartPoint, selectionEndPoint;

	virtual bool mouseDown(AEvent e) {
		if (!containsPoint(e.location)) return false;
		if (e.flags & AEF_BUTTON1) {
			inSelection = true;
			selectionStartPoint = e.location;
			return true;
		}
		return false;
	}
	
	virtual bool mouseUp(AEvent e) {
		if ((e.flags & AEF_BUTTON1) && inSelection) {
			inSelection = false;
			selectionEndPoint = e.location;
			ARect r = AMkRect(selectionStartPoint.x, selectionStartPoint.y, selectionEndPoint.x - selectionStartPoint.x, selectionEndPoint.y - selectionStartPoint.y);
			if (r.width < 0) { r.x += r.width; r.width = -r.width; }
			if (r.height < 0) { r.y += r.height; r.height = -r.height; }
			redraw();
			performSelection(r);
			return true;
		}
		return false;
	}
	
	virtual bool mouseMove(AEvent e) {
		if (inSelection) {
			selectionEndPoint = e.location;
			redraw();
			return true;
		}
		return false;
	}
	
	virtual bool performSelection(ARect where) {
		printf("%s: perform selection: (%g,%g - %g,%g)\n", describe(), where.x, where.y, where.width, where.height);
		return false;
	}
};

#endif
