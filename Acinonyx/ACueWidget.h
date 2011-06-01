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

// A cue widget is a widget that is normally invisible unless the mouse it moved over the widget.
// Pretty much all widgets implemented in Acinonyx are cue widgets, because we don't use any static widgets
class ACueWidget : public AWidget {
protected:
	int on_screen; // if != 0 then it was brought to screen, otherwise is it not visible (yet not hidden!) FIXME: on_screen is int because the plan was to support animation...
	bool cue_link_children;  // determines whether this widget should also send cueOn message to its children when displaying
	bool cue_link_parent;    // determines whether this widget should respect the parent before ending its visibility status - if true then the parent must be a cue widget as well
	bool manual_cue;         // special case for a cue widget that is not activated by mouse (most commonly activated by its parent instead - see, e.g., ACueHotButton)

public:
#pragma mark --- contructor ---
	ACueWidget(AContainer *parent, ARect frame, unsigned int flags, bool link_children=false) : AWidget(parent, frame, flags), on_screen(0), cue_link_children(link_children), cue_link_parent(false), manual_cue(false) {
		OCLASS(ACueWidget)
	}

	void setManualCue(bool what) { manual_cue = what; }
	
	virtual bool mouseDown(AEvent event) {
		if (!on_screen)
			return false;
		return AWidget::mouseDown(event);
	}

	virtual bool mouseUp(AEvent event) {
		if (!on_screen)
			return false;
		return AWidget::mouseUp(event);
	}
	
	
	virtual void add(ACueWidget &obj) {
		ALog("ACueWidget: add %s (link=%s)", obj.describe(), cue_link_children ? "yes" : "no");
		if (cue_link_children)
			obj.cue_link_parent = true;
 		AWidget::add(obj);
	}

	virtual bool cueOn() {
		on_screen = 1;
		if (cue_link_children) {
			chList_t *c = chRoot;
			while (c) {
				ACueWidget *child = static_cast <ACueWidget*> (c->o);
				if (child)
					child->cueOn();
				c = c->next;
			}
		}
		if (cue_link_parent && _parent && !(static_cast <ACueWidget*> (_parent))->on_screen)
			(static_cast <ACueWidget*> (_parent))->cueOn();
		// FIXME: no redraw is performend, we are relying on the redraw in mouseEnter/Leave!
		return true;
	}

	virtual bool cueOff(bool all = false, bool descent = false, bool excludeSelf = false) {
		//if (!all && cue_link_parent && _parent && (static_cast <ACueWidget*> (_parent))->on_screen) {
		//	ALog("%s: cueOff denied", describe());
		//	return false;
		//}
		if (descent) {
			if (cue_link_parent && _parent)
				return (static_cast <ACueWidget*> (_parent))->cueOff(all, true);
			ALog("%s: cueOff descent reached root", describe());
		}
		if (!all && inHoveringMode(true)) {
			ALog("%s: cueOff rejected, at least one widget is hovering", describe());
			return false;
		}
		ALog("%s: cueOff allowed", describe());
		if (!excludeSelf)
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
		if (!manual_cue) cueOn();
		redrawWidget();
		return false; // FIXME: we want to allow everyone to access move since it may be pertinent even to visuals that are not in the region - but maybe we should shift this back down to mouseMove
	}
	
	virtual bool mouseLeave(AEvent event) {
		ALog("ACueWidget %s: mouseLeave", describe());
		AWidget::mouseLeave(event);
		if (!mouse_locked) {
			cueOff(false, cue_link_parent);
			redrawWidget();
		}
		return false; // see comment in mouseEnter
	}

	virtual bool drawWidget() {
		return on_screen ? true : false; // subclasses won't draw if not on screen
	}
};

#endif
