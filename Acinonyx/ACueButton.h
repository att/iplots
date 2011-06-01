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
	bool centered_label;    // true if the label is to be placed in the center, otherwise it is left-aligned

#pragma mark --- contructor ---
	ACueButton(AContainer *parent, ARect frame, unsigned int flags, const char *label) : ACueWidget(parent, frame, flags), label_(strdup(label ? label : "")), centered_label(true) { OCLASS(ACueButton) }
	
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
			roundRect(_frame, 8.0);
			color(0,0,0,0.5);
			roundRectO(_frame, 8.0);			
			if (*label_) {
				AColor orig_text = txtcolor();
				if (hovering)
					txtcolor(0.0, 0.0, 0.0, 1.0);
				else
					txtcolor(1.0, 1.0, 1.0, 1.0);
				text(AMkPoint(_frame.x + (centered_label ? (_frame.width / 2) : 8.0), _frame.y + _frame.height / 2 + 1.5), label_, AMkPoint(centered_label ? 0.5: 0.0, 0.5));
				txtcolor(orig_text);
			}			
			return true;
		}
		return false;
	}
	
	virtual char *describe() {
		char *q = ACueWidget::describe();
		if (strlen(q) + strlen(label_) + 5 >= sizeof(desc_buf)) return q;
		q[strlen(q) - 1] = 0;
		strcat(q, " '");
		strcat(q, label_);
		strcat(q, "'>");
		return q;
	}
};

#endif
