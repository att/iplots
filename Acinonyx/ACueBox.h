/*
 *  ACueBox.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/1/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_CUE_BOX_H
#define A_CUE_BOX_H

#include "ACueWidget.h"

// a simple box, mainly to be used for grouping of other widgets
class ACueBox : public ACueWidget {
protected:
	AFloat roundX, roundY;
	bool draw_border, draw_hover, draw_background;
public:
	
#pragma mark --- constructor ---
	ACueBox(AContainer *parent, ARect frame, unsigned int flags) : ACueWidget(parent, frame, flags, true), roundX(0.0), roundY(0.0), draw_border(true), draw_hover(false), draw_background(true) { OCLASS(ACueBox) }
	
	void setRoundCorners(AFloat cornerX = 8.0, AFloat cornerY = -1.0) {
		if (cornerY < 0.0) cornerY = cornerX;
		roundX = cornerX;
		roundY = cornerY;
	}
	
	void setDrawBorder(bool what) {
		draw_border = what;
	}

	void setDrawHover(bool what) {
		draw_hover = what;
	}

	void setDrawBackground(bool what) {
		draw_background = what;
	}
	
	virtual bool drawWidget() {
		if (ACueWidget::drawWidget()) {
			if (draw_background) {
				color((hovering && draw_hover) ? widgetHoverColor : widgetColor);
				roundRect(_frame, roundX, roundY);
			}
			if (draw_border) {
				color(0,0,0,0.5);
				roundRectO(_frame, roundX, roundY);			
			}
			return true;
		}
		return false;
	}
};

#endif
