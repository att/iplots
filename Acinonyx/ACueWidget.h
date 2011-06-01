/*
 *  ACueWidget.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/31/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_CUE_WIDGET_H
#define A_CUE_WIDGET_H

#include "AWidget.h"

// A cue widget is a widget that is normally invisible unless the mouse it moved over the widget
class ACueWidget : public AWidget {
protected:
	int on_screen; // if != 0 then it was brought to scree by moving a mouse over it
	bool cue_link_children;  // determines whether children's cue behavior is linked to this parent
	bool cue_link_parent;    // determines whether this widget shoudl respect the parent before ending cue
public:
#pragma mark --- contructor ---
	ACueWidget(AContainer *parent, ARect frame, unsigned int flags, bool link_children=false) : AWidget(parent, frame, flags), on_screen(0), cue_link_children(link_children), cue_link_parent(false) {
		OCLASS(ACueWidget)
	}

	virtual bool cueOn() {
		on_screen = 1;
		if (cue_link_children) {
			chList_t *c = chRoot;
			while (c) {
				ACueWidget *child = static_cast <ACueWidget*> (c->o);
				if (child) {
					child->cue_link_parent = true;
					child->cueOn();
				}
				c = c->next;
			}
		}
		// FIXME: no redraw is performend, we are relying on the redraw in mouseEnter/Leave!
		return true;
	}

	virtual bool cueOff(bool all = false) {
		if (!all && cue_link_parent && _parent && (static_cast <ACueWidget*> (_parent))->on_screen) return false;
		on_screen = 0;
		if (all || cue_link_children) {
			chList_t *c = chRoot;
			while (c) {
				(static_cast <ACueWidget*> (c->o))->cueOff(all);
				c = c->next;
			}
		}
		return true;
	}
	
	virtual bool mouseEnter(AEvent event) {
		ALog("ACueWidget %s: mouseEnter", describe());
		AWidget::mouseEnter(event);
		cueOn();
		redrawWidget();
		return false; // FIXME: we want to allow everyone to access move since it may be pertinent even to visuals that are not in the region - but maybe we should shift this back down to mouseMove
	}
	
	virtual bool mouseLeave(AEvent event) {
		ALog("ACueWidget %s: mouseLeave", describe());
		AWidget::mouseLeave(event);
		if (!mouse_locked) {
			cueOff();
			redrawWidget();
		}
		return false; // see comment in mouseEnter
	}

	virtual bool drawWidget() {
		return on_screen ? true : false; // subclasses won't draw if not on screen
	}
};

#endif
