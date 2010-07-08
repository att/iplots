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
	ADiscreteXAxis *xa; // x-axis (virtual, coords)
	AYAxis *ca, **ya;
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
		
		_scales = (AScale**) malloc(sizeof(AScale*) * nScales);
		AMEM(_scales);
		_scales[0] = new AScale(NULL, AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight), coords);
		_scales[1] = new AScale(NULL, AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), computeCommonScaleRange());
		i = 0;
		while (i < coords) {
			_scales[i + 2] = new AScale(data[i], AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop), data[i]->range());
			i++;
		}

		ptGrid = (AFloat**) malloc(sizeof(AFloat*) * coords);
		AMEM(ptGrid);
		
		xa = new ADiscreteXAxis(this, AMkRect(_frame.x + mLeft, _frame.y, _frame.width - mLeft - mRight, mBottom), AVF_FIX_BOTTOM|AVF_FIX_HEIGHT|AVF_FIX_LEFT, _scales[0]);
		xa->setNames(NULL, coords);
		for (vsize_t i = 0; i < coords; i++)
			xa->setName(i, _data[i]->name());
		add(*xa);
		/* ya = new AYAxis(this, AMkRect(_frame.x, _frame.y + mBottom, mLeft, _frame.height - mBottom - mTop), AVF_FIX_LEFT|AVF_FIX_WIDTH, scales[1]);
		add(*ya); 
		// add home zoom entry
		AZoomEntryBiVar *ze = new AZoomEntryBiVar(scales[0]->dataRange(), scales[1]->dataRange());
		zoomStack->push(ze);
		ze->release();
		 */
		update();
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
		
	void update() {
		_scales[0]->setRange(AMkRange(_frame.x + mLeft, _frame.width - mLeft - mRight));
		for(vsize_t i = 1; i < nScales; i++)
			_scales[i]->setRange(AMkRange(_frame.y + mBottom, _frame.height - mBottom - mTop));

		vsize_t i = 0;
		while (i < coords) {
			ptGrid[i] = 0;
			vsize_t si = _scales[0]->permutationAt(i);
			if (commonScale) { /* FIXME: this is currently a hack -- we need to check whether we could use the real scale sharing instead ... */
				ARange gr = _scales[1]->range();
				ADataRange dr = _scales[1]->dataRange();
				float a = gr.length / dr.length;
				float b = gr.begin - a * dr.begin;
				ptGrid[i] = _scales[si + 2]->locations(); /* we use this only to maintain separate memory locations */
				_data[i]->transformToFloats(ptGrid[i], a, b);
			} else if (si != ANotFound)
				ptGrid[i] = _scales[si + 2]->locations();
			i++;
		}

		APlot::update();
	}
	
	virtual void draw(vsize_t layer) {
		ALog("%s: draw", describe());
		//xa->draw();
		//ya->draw();
		
		if (layer == LAYER_ROOT) {
			//clip(AMkRect(_frame.x + mLeft, _frame.y + mBottom, _frame.width - mLeft - mRight, _frame.height - mTop - mBottom));
			clip(_frame);
			// glPointSize(ptSize);
		
			// draw axes and calculate locations
			color(AMkColor(0.0, 0.0, 0.5, 0.4));
			vsize_t i = 0;
			while (i < coords) {
				vsize_t si = _scales[0]->permutationAt(i);
				if (si != ANotFound) {
					AFloat xpos = _scales[0]->discreteCenter(i);
					ARange gr = _scales[si]->range();
					line(xpos, gr.begin, xpos, gr.length);
				}
				i++;
			}
		
			// draw lines
			AColor baseColor = AMkColor(0.0,0.0,0.0,ptAlpha);
			AColorMap *cMap = marker->colorMap();
			color(baseColor);
			bool lastWasBrushed = false;
			vsize_t j = 0, n = _data[0]->length();
			while (j < n) {
				if (!marker->isHidden(j)) {
					mark_t mv = marker->value(j);
					if (mv) {
						AColor c = cMap->color(mv);
						c.a = ptAlpha;
						color(c);
						lastWasBrushed = true;
					} else if (lastWasBrushed) {
						color(baseColor);
						lastWasBrushed = false;
					}
					lineBegin();
					for (vsize_t ci = 0; ci < coords; ci++) {
						AFloat xpos = _scales[0]->discreteCenter(ci);
						if (ptGrid[ci])
							lineTo(xpos, ptGrid[ci][j]);
					}
					lineEnd();
				}
				j++;
			}
		
			// draw selection
			color(AMkColor(1.0, 0.0, 0.0, 1.0));
			j = 0, n = _data[0]->length();
			while (j < n) {
				if (!marker->isHidden(j) && (marker->isSelected(j))) {
					lineBegin();
					for (vsize_t ci = 0; ci < coords; ci++) {
						AFloat xpos = _scales[0]->discreteCenter(ci);
						if (ptGrid[ci])
							lineTo(xpos, ptGrid[ci][j]);
					}
					lineEnd();
				}
				j++;
			}
		}

		APlot::draw(layer);
	}
	
	virtual bool performSelection(ARect where, int type, bool batch = false) {
		if (!marker) return false;
		vsize_t n = _data[0]->length();
		if (!batch) marker->begin();
		if (type == SEL_REPLACE)
			marker->deselectAll();
		if (type == SEL_XOR) {
			for (vsize_t i = 0; i < coords; i++)
				if (ptGrid[i]) {
					AFloat xpos = _scales[0]->discreteCenter(i);
					if ((xpos >= where.x) && (xpos <= where.x + where.width))
						for(vsize_t j = 0; j < n; j++)
							if (where.y <= ptGrid[i][j] && where.y + where.height >= ptGrid[i][j])
								marker->selectXOR(j);
				}
		} else if (type == SEL_NOT) {
			for (vsize_t i = 0; i < coords; i++)
				if (ptGrid[i]) {
					AFloat xpos = _scales[0]->discreteCenter(i);
					if ((xpos >= where.x) && (xpos <= where.x + where.width))
						for(vsize_t j = 0; j < n; j++)
							if (where.y <= ptGrid[i][j] && where.y + where.height >= ptGrid[i][j])
								marker->deselect(j);
				}
		} else if (type == SEL_AND) {
			// FIXME: check whether this is what we want if more than one axis is selected
			for (vsize_t i = 0; i < coords; i++)
				if (ptGrid[i]) {
					AFloat xpos = _scales[0]->discreteCenter(i);
					if ((xpos >= where.x) && (xpos <= where.x + where.width))
						for(vsize_t j = 0; j < n; j++)
							if (!(where.y <= ptGrid[i][j] && where.y + where.height >= ptGrid[i][j]))
								marker->deselect(j);
				}
		} else {
			for (vsize_t i = 0; i < coords; i++)
				if (ptGrid[i]) {
					AFloat xpos = _scales[0]->discreteCenter(i);
					if ((xpos >= where.x) && (xpos <= where.x + where.width))
						for(vsize_t j = 0; j < n; j++)
							if (where.y <= ptGrid[i][j] && where.y + where.height >= ptGrid[i][j])
								marker->select(j);
				}
		}
		if (!batch) marker->end();
		return true;
	}
	
	virtual bool keyDown(AEvent e) {
	  ALog("PCP keyDown: %d\n", e.key);
		switch (e.key) {
			case KEY_DOWN: if (ptSize > 1.0) { ptSize -= 1.0; setRedrawLayer(LAYER_ROOT); redraw(); }; break;
			case KEY_UP: ptSize += 1.0; setRedrawLayer(LAYER_ROOT); redraw(); break;
			case KEY_LEFT: if (ptAlpha > 0.02) { ptAlpha -= (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha < 0.02) ptAlpha = 0.01; setRedrawLayer(LAYER_ROOT); redraw(); }; break;
			case KEY_RIGHT: if (ptAlpha < 0.99) { if (ptAlpha < 0.02) ptAlpha = 0.02; else ptAlpha += (ptAlpha < 0.2) ? 0.02 : 0.1; if (ptAlpha > 1.0) ptAlpha = 1.0; setRedrawLayer(LAYER_ROOT); redraw(); } break;
		  	case KEY_C: commonScale = !commonScale; setRedrawLayer(LAYER_ROOT); update(); redraw(); break;
			default:
				return false;
		}
		return true;
	}

	virtual const char *caption() {
		if (_caption)
			return _caption;
		return "Parallel Coordinates Plot";
	}	
};
