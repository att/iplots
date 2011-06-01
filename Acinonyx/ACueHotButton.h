/*
 *  ACueHotButton.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/1/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_CUE_HOTBUTTON_H
#define A_CUE_HOTBUTTON_H

#include "ACueButton.h"

// A button that links to its cue children; its main purpose is to act as the cue surrogate for the children based on its hovering status. All its children must be cue widgets and they will be place in manual cue mode
class ACueHotButton : public ACueButton {
protected:
	virtual void setHovering(bool what) {
		if (hovering != what) {
			hovering = what;
			if (what)
				cueOn();
			else
				cueOff(false, false, true);
		}
		ACueButton::setHovering(what);
	}
	
public:
#pragma mark --- constructor ---
	ACueHotButton(AContainer *parent, ARect frame, unsigned int flags, const char *label) : ACueButton(parent, frame, flags, label) {
		cue_link_children = true;
		OCLASS(ACueButton)
	}
	
	virtual void add(ACueWidget &obj) {
		obj.setManualCue(true);
 		ACueButton::add(obj);
	}
	
	virtual bool cueOn() {
		if (!hovering) { // if not hovering, cue only this widget, no children
			cue_link_children = false;
			bool res = ACueButton::cueOn();
			cue_link_children = true;
			return res;
		}
		return ACueButton::cueOn();
	}
};

#endif
