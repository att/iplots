/*
 *  AContainer.cpp
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#include "AContainer.h"

#include "AVisual.h"
#include "AWindow.h"

void AWindow::draw() {
	AVisual *rv = (AVisual*) _rootVisual;
	if (rv) {
		// default font ("" stands for user/default font)
		// FIXME: 10.0 font size works well on OS X - check other platforms...
		rv->font("", 10.0);

		vsize_t layer = _redraw_layer;
		while (layer < _max_layers) {
			rv->clipOff();
			printf("  - draw layer %d\n", layer);
			rv->draw(layer);
			freezeLayer(layer++);
		}
		printf("  - draw layer %d (transient)\n", LAYER_TRANS);
		rv->draw(LAYER_TRANS); // the transient layer is not stored
		_redraw_layer = LAYER_TRANS;
	}
}

void AWindow::setFrame(ARect frame) {
	_frame = frame;
	AVisual *rv = (AVisual*) _rootVisual;
	if (rv)
		rv->moveAndResize(AMkRect(0.0, 0.0, frame.width, frame.height));
}
