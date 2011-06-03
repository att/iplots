/*
 *  AContainer.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_CONTAINER_H
#define A_CONTAINER_H

#include "ATypes.h"
#include "AVisual.h"
#include "AQuery.h"

#include <stdlib.h>

typedef struct chList {
	AVisual *o;
	struct chList *next;
} chList_t;

class AContainer : public AVisual {
protected:
	chList_t *chRoot, *chTail;
	bool passToAll;
public:
#pragma mark --- contructor ---
	AContainer(AContainer *parent, ARect frame, unsigned int flags = AVF_DEFAULT) : AVisual(parent, frame, flags|AVF_CONTAINER), chRoot(0), chTail(0), passToAll(true) { OCLASS(AContainer) }

	virtual ~AContainer() {
		removeAll();
		DCLASS(AContainer);
	}
	
#pragma mark --- child list manipulation ---
	virtual void add(AVisual &obj) {
		chList_t *n = (chList_t*) AAlloc(sizeof(chList_t));
		AMEM(n);
		obj.retain();
		n->o = &obj;
		if (_window) {
			obj.setWindow(_window);
			_window->setRedrawLayer(LAYER_ROOT); // the window has to redraw form scratch
		}
		
		n->next = 0;
		if (chRoot)
			chTail = chTail->next = n;
		else
			chTail = chRoot = n;
	}
	
	void remove(AVisual &obj) {
		chList_t *c = chRoot, *cp = 0;
		while (c && c->o != &obj) { cp = c; c = c->next; }
		if (c->o) {
			if (cp) {
				cp->next = c->next;
				if (chTail == c) chTail = cp;
				AFree(c);
			} else {
				chRoot = c->next;
				if (chTail == c) chTail = 0;
			}
			c->o->release();
		}
	}
	
	void removeAll() {
		chList_t *c = chRoot;
		while (c) {
			chList_t *n = c->next;
			c->o->release();
			AFree(c);
			c = n;
		}
		chRoot = chTail = 0;
	}
	
	bool isChild(AVisual *obj) {
		chList_t *c = chRoot;
		while (c && c->o != obj) c = c->next;
		return c?true:false;
	}
	
	AVisual *childByTag(int tag) {
		chList_t *c = chRoot;
		while (c) {
			if (c->o && c->o->tag_ == tag)
				return c->o;
			c = c->next;
		}
		return NULL;		
	}
	
	// similar to isChild but checks recursively in container children
	bool contains(AVisual *obj) {
		chList_t *c = chRoot;
		while (c && c->o != obj && !(c->o->isContainer() && ((AContainer*)c->o)->contains(obj))) c = c->next;
		return c?true:false;
		
	}
	
	virtual void setWindow(AWindow *win) {
		AVisual::setWindow(win); // set our window and then all children
		chList_t *c = chRoot;
		while (c) { if (c->o) c->o->setWindow(win); c = c->next; }
	}
	
	virtual void draw(vsize_t layer) {
		chList_t *c = chRoot;
		while (c) {
			if (!c->o->isHidden()) {
				// clip(c->o->frame()); // bad things happen if you do that ... (e.g. axes don't like to be clipped)
				c->o->draw(layer);
			}
			c = c->next;
		}
	}

	virtual void query(AQuery *query, int level) {
		chList_t *c = chRoot;
		while (c) {
			if (!c->o->isHidden())
				c->o->query(query, level);
			c = c->next;
		}
	}

	virtual void notification(AObject *source, notifid_t nid) {
		AVisual::notification(source, nid);
		chList_t *c = chRoot;
		while (c) {
			if (c->o != source) c->o->notification(source, nid);
			c = c->next;
		}
	}

	virtual bool event(AEvent event) {
		chList_t *c = chRoot;
		if (passToAll) { // event propagtion is either pass-to-all or stop after one child has acknowledged processing it
			bool hitAny = false;
			while (c) {
				if (c->o->event(event)) hitAny = true;
				c = c->next;
			}
			if (hitAny) return true;
		} else
			while (c) {
				if (c->o->event(event)) return true;
				c = c->next;
			}
		// if no children handled this, resort to local processing (dispatching to virtual methods)
		return AVisual::event(event);
	}
	
	virtual void move(APoint where) {
		ALog("%s.move: (%f, %f) %f x %f -> (%f, %f)", describe(),
		     _frame.x, _frame.y, _frame.width, _frame.height,
		     where.x, where.y);
		ARect f = _frame;
		AFloat movX = where.x - _frame.x;
		AFloat movY = where.y - _frame.y;
		f.x = where.x;
		f.y = where.y;
		setFrame(f);

		chList_t *c = chRoot;
		while (c) {
			ARect cr = c->o->frame();
			c->o->move(AMkPoint(cr.x + movX, cr.y + movY));
			c = c->next;
		}
	}
	
	virtual void moveAndResize(ARect frame) {
		if (ARectsAreEqual(frame, _frame)) return; // it's a no-op if the size match already
		if (ASizesAreEqual(frame, _frame)) { move(AMkPoint(frame.x, frame.y)); return; } // delegate to move() if the size didn not change
		ARect previousFrame = _frame;
		AFloat deltaW = frame.width - _frame.width;
		AFloat deltaH = frame.height - _frame.height;
		AFloat movX = frame.x - _frame.x;
		AFloat movY = frame.y - _frame.y;
		
		ALog("%s.moveAndResize: (%f, %f) %f x %f -> (%f, %f) %f x %f", describe(),
			 previousFrame.x, previousFrame.y, previousFrame.width, previousFrame.height,
			 frame.x, frame.y, frame.width, frame.height);
		
		setFrame(frame);
		
		chList_t *c = chRoot;
		while (c) {
			unsigned int cf = c->o->flags();
			ARect cr = c->o->frame();
			ARect pr = cr;
			ALog("child frame %s: (%f, %f) (%f x %f) flags: %04x", c->o->describe(), cr.x, cr.y, cr.width, cr.height, cf);
			cr.x += movX;
			cr.y += movY;

			if (cf & AVF_FIX_LEFT)
				/* nothing to do */;
			else if (cf & AVF_FIX_WIDTH) {
				if (cf & AVF_FIX_RIGHT)
					cr.x += deltaW; // full share in the left part
				else // this is a rather pathological case: fixed width and no fixed side, so we interpret it at "fix the center"
					// (x + w/2)*alpha - w/2 - x = (alpha - 1) * (x + w/2)
					cr.x += (frame.width / previousFrame.width  - 1) * (pr.x + pr.width / 2.0);
			} else /* no flags = move proportionally */
				cr.x = movX + pr.x / previousFrame.width * frame.width;

			if (cf & AVF_FIX_BOTTOM)
				/* nothing to do */;
			else if (cf & AVF_FIX_HEIGHT) {
				if (cf & AVF_FIX_TOP)
					cr.y += deltaH;
				else
					cr.y += (frame.height / previousFrame.height  - 1) * (pr.y + pr.height / 2.0);
			} else
				cr.y = movY + pr.y / previousFrame.height * frame.height;

			ALog("  step 1: %f, %f (from %f, %f)", cr.x, cr.y, pr.x, pr.y);
			if ((cf & AVF_FIX_WIDTH) == 0) { // change width only if it's not fixed
				if ((cf & AVF_FIX_RIGHT) && (cf & AVF_FIX_LEFT)) /* both fixed = all in the width */
					cr.width += deltaW; 
				else if (cf & AVF_FIX_LEFT) // otherwise proportional, but we have to take out the fixed amount (if fixed)
					cr.width *= (frame.width - pr.x) / (previousFrame.width - pr.x);
				else if (cf & AVF_FIX_RIGHT) {
					AFloat fixed_space = previousFrame.width - pr.x - pr.width;
					cr.width *= (frame.width - fixed_space) / (previousFrame.width - fixed_space);
				} else
					cr.width *= frame.width / previousFrame.width;
			}
			
			if ((cf & AVF_FIX_HEIGHT) == 0) { // change height only if it's not fixed
				if ((cf & AVF_FIX_TOP) && (cf & AVF_FIX_BOTTOM))
					cr.height += deltaH;
				else if (cf & AVF_FIX_BOTTOM)
					cr.height *= (frame.height - pr.y) / (previousFrame.height - pr.y);
				else if (cf & AVF_FIX_TOP) {
					AFloat fixed_space = previousFrame.height - pr.y - pr.height;
					cr.height *= (frame.height - fixed_space) / (previousFrame.height - fixed_space);
				} else
					cr.height *= frame.height / previousFrame.height;
			}
			ALog("  step 2: %f x %f (from %f x %f)", cr.width, cr.height, pr.width, pr.height);
			if (!ARectsAreEqual(cr, pr))
				c->o->moveAndResize(cr);
			c = c->next;
		}		
	}
	
	virtual const char *caption() {
		return "Container";
	}
};

#endif
