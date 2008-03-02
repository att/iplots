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

#include <stdlib.h>

typedef struct chList {
	AVisual *o;
	struct chList *next;
} chList_t;

class AContainer : public AVisual {
	chList_t *chRoot, *chTail;
public:
#pragma mark --- contructor ---
	AContainer(AContainer *parent, ARect frame, unsigned int flags) : AVisual(parent, frame, flags|AVF_CONTAINER), chRoot(0), chTail(0) {
	}

	~AContainer() {
		removeAll();
	}
	
#pragma mark --- child list manipulation ---
	void add(AVisual &obj) {
		chList_t *n = (chList_t*) malloc(sizeof(chList_t));
		// FIXME: assert(n!=0)
		obj.retain();
		n->o = &obj;
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
				free(c);
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
			free(c);
			c = n;
		}
		chRoot = chTail = 0;
	}
	
	bool isChild(AVisual *obj) {
		chList_t *c = chRoot;
		while (c && c->o != obj) c = c->next;
		return c?true:false;
	}
	
	// similar to isChild but checks recursively in container children
	bool contains(AVisual &obj) {
		chList_t *c = chRoot;
		while (c && c->o != &obj && !(c->o->isContainer() && ((AContainer*)c->o)->contains(obj))) c = c->next;
		return c?true:false;
		
	}
	
	virtual void draw() {
		chList_t *c = chRoot;
		while (c) {
			c->o->draw();
			c = c->next;
		}		
	}

	virtual bool event(AEvent event) {
		chList_t *c = chRoot;
		while (c) {
			if (c->o->event(event)) return true;
			c = c->next;
		}
		return false;
	}
	
	virtual void resize(ARect frame) {
		// resize children ...
		_frame = frame;
	}
};

#endif
