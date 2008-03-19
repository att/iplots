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

#include "AScale.h"
#include "AVisualPrimitive.h"

class APlot {
protected:
	AScale **scales;
	int nScales;
	
	AVisualPrimitive **vps;
	int nVps;
	
public:
	APlot(AScale *xScale, AScale *yScale) : nScales(2), nVps(0), vps(NULL), scales({xScale, yScale}) {}
	
	virtual void drawLayer(int l) = 0;
}
