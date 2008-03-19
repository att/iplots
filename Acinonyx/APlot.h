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
};

#endif
