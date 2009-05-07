/*
 *  AParallelCoordPlot.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/6/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

class AParallelCoordPlot : public APlot {
protected:
	AMarker *marker;
	AXAxis *xa; // x-axis (virtual, coords)
	AYAxis *ca, **ya;
	AFloat mLeft, mTop, mBottom, mRight, ptSize, ptAlpha;
	vsize_t coords;
	ADataVector **_data;

	AFloat **ptGrid;

	bool commonScale;
	
	ADataRange computeCommonScaleRange() {
		vsize_t i = 1;
		ADataRange gdr = _data[0]->range();
		while (i < coords) {
			ADataRange r = _data[i]->range();
			if (!ADataRangesAreEqual(r, AUndefDataRange)) {
				if (r.begin < gdr.begin) { gdr.length += gdr.begin - r.begin; gdr.begin = r.begin; }
				if (r.begin + r.length > gdr.begin + gdr.length) gdr.length += r.begin + r.length - gdr.begin - gdr.length;
			}
			i++;
		}
		return gdr;
	}

public:
	AParallelCoordPlot(AContainer *parent, ARect frame, int flags, vsize_t coordinates, ADataVector **data) : APlot(parent, frame, flags), coords(coordinates), commonScale(false) {
		mLeft = 0.0f; mTop = 10.0f; mBottom = 20.0f; mRight = 0.0f;
		ptSize = 5.0;
		ptAlpha = 0.6;
		// printf("AScatterPlot frame = (%f,%f - %f x %f)\n", _frame.x, _frame.y, _frame.width, _frame.height);
		nScales = coords + 2; // coordinates, common and x-axis
		marker = 0;
		vsize_t i = 0;
		while (!marker && i < coords)
			marker = data[i++]->marker();
		if (marker) {
			marker->add(this);
			marker->retain();
		}

		_data = (ADataVector**) memdup(data, sizeof(ADataVector*) * coords);
		for (vsize_t ii = 0; ii < coordinates; ii++)
			_data[ii]->retain();
		
		scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		AMEM(scales);
		scales[0] = new AScale(NULL, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), coords);
		scales[1] = new AScale(NULL, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), computeCommonScaleRange());
		i = 0;
		while (i < coords) {
			scales[i + 2] = new AScale(data[i], AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), data[i]->range());
			i++;
		}

		ptGrid = (AFloat**) malloc(sizeof(AFloat*) * coords);
		AMEM(ptGrid);
		
		xa = new AXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, scales[0]);
		add(*xa);
		/* ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, scales[1]);
		add(*ya); 
		// add home zoom entry
		AZoomEntryBiVar *ze = new AZoomEntryBiVar(scales[0]->dataRange(), scales[1]->dataRange());
		zoomStack->push(ze);
		ze->release();
		 */
		OCLASS(AParallelCoordPlot)
	}
	
	virtual ~AParallelCoordPlot() {
		marker->remove(this);
		marker->release();
		xa->release();
		for (vsize_t i = 0; i < coords; i++)
			_data[i]->release();
		free(_data);
		free(ptGrid);

		// ya->release();
		// scales get released by APlot destructor
		DCLASS(AScatterPlot)
	}
	
	// this is subtle and holefully we'll get rid of this, but the constructor may be called with NULL parent so it has no windows and axes are not registered in the hierarchy ...
	virtual void setWindow(AWindow *win) {
		APlot::setWindow(win);
		xa->setWindow(win);
		//ya->setWindow(win);
	}
	
	void update() {
		scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		for(vsize_t i = 1; i < nScales; i++)
			scales[i]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));
	}
	
	virtual void draw() {
		printf("%s: draw\n", describe());
		//xa->draw();
		//ya->draw();
		
		//clip(AMkRect(_frame.x + mLeft, _frame.y + mBottom, _frame.width - mLeft - mRight, _frame.height - mTop - mBottom));
		clip(_frame);
		// glPointSize(ptSize);
		
		// draw axes and calculate locations
		color(AMkColor(0.0, 0.0, 0.5, 0.4));
		vsize_t i = 0;
		while (i < coords) {
			ptGrid[i] = 0;
			vsize_t si = scales[0]->permutationAt(i);
			if (si != ANotFound) {
				AFloat xpos = scales[0]->discreteCenter(i);
				ARange gr = scales[si]->range();
				line(xpos, gr.begin, xpos, gr.length);
				if (!commonScale)
					ptGrid[i] = scales[si + 2]->locations();
			}
			i++;
		}
		
		// draw lines
		color(AMkColor(0.0,0.0,0.0,ptAlpha));
		vsize_t j = 0, n = _data[0]->length();
		while (j < n) {
			lineBegin();
			for (vsize_t ci = 0; ci < coords; ci++) {
				AFloat xpos = scales[0]->discreteCenter(ci);
				if (ptGrid[ci])
					lineTo(xpos, ptGrid[ci][j]);
			}
			lineEnd();
			j++;
		}
		
		// draw selection
		color(AMkColor(1.0, 0.0, 0.0, 1.0));
		j = 0, n = _data[0]->length();
		while (j < n) {
			if (marker->isSelected(j)) {
				lineBegin();
				for (vsize_t ci = 0; ci < coords; ci++) {
					AFloat xpos = scales[0]->discreteCenter(ci);
					if (ptGrid[ci])
						lineTo(xpos, ptGrid[ci][j]);
				}
				lineEnd();
			}
			j++;
		}
		
		
		APlot::draw();
	}		
};
