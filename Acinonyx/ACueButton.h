/*
 *  ACueButton.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/31/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_CUE_BUTTON_H
#define A_CUE_BUTTON_H

#include "ACueWidget.h"

// A click-button
class ACueButton : public ACueWidget {
protected:
	char *label_;
public:
#pragma mark --- contructor ---
	ACueButton(AContainer *parent, ARect frame, unsigned int flags, const char *label) : ACueWidget(parent, frame, flags), label_(strdup(label ? label : "")) { OCLASS(ACueButton) }
	
	virtual ~ACueButton() {
		free(label_);
		DCLASS(ACueButton);
	}
	
	const char *label() { return label_; }

	void setLabel(const char *newLabel) {
		free(label_);
		label_ = strdup(newLabel ? newLabel : "");
		redrawWidget();
	}
	
	virtual bool drawWidget() {
		if (ACueWidget::drawWidget()) {
			color(hovering ? widgetHoverColor : widgetColor);
			roundRect(_frame, 10.0);
			color(0,0,0,0.5);
			roundRectO(_frame, 10.0);			
			if (*label_) {
				AColor orig_text = txtcolor();
				if (hovering)
					txtcolor(0.0, 0.0, 0.0, 1.0);
				else
					txtcolor(1.0, 1.0, 1.0, 1.0);
				text(AMkPoint(_frame.x + _frame.width / 2, _frame.y + _frame.height / 2 + 1.5), label_, AMkPoint(0.5, 0.5));
				txtcolor(orig_text);
			}			
			return true;
		}
		return false;
	}
};

#endif