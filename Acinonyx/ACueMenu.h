/*
 *  ACueMenu.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/1/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_CUE_MENU_H
#define A_CUE_MENU_H

#include "ACueBox.h"
#include "ACueButton.h"

// a box that will atuomatically create items to act as a menu
class ACueMenu : public ACueBox {
protected:
	AFloat next_line, line_height;
public:
	
#pragma mark --- constructor ---
	ACueMenu(AContainer *parent, ARect frame, unsigned int flags) : ACueBox(parent, frame, flags), next_line(0.0), line_height(14.0) {
		draw_background = false;
		OCLASS(ACueBox)
	}
	
	ACueButton *addItem(const char *name, const char *action) {
		_frame.y -= line_height;
		_frame.height += line_height;
		ARect f = _frame;
		ACueButton *b = new ACueButton(this, AMkRect(f.x, f.y + f.height - next_line - line_height, f.width, line_height - 1.0), AVF_DEFAULT|AVF_FIX_TOP|AVF_FIX_HEIGHT|AVF_XSPRING, name);
		b->click_action = action;
		b->centered_label = false;
		// b->cue_link_parent = true;
		b->setDelegate(delegate());
		b->setManualCue(true);
		add(*b);
		b->release();
		next_line += line_height;
		return b;
	}
	
};

#endif
