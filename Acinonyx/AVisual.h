/*
 *  AVisual.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_VISUAL_H_
#define A_VISUAL_H_

#include "AObject.h"
#include "ARenderer.h"

class AContainer;

#define AVF_CONTAINER 0x0001

/* flags related to resizing in containers */
#define AVF_FIX_TOP     0x0100
#define AVF_FIX_LEFT    0x0200
#define AVF_FIX_BOTTOM  0x0400
#define AVF_FIX_RIGHT   0x0800
#define AVF_FIX_WIDTH   0x1000
#define AVF_FIX_HEIGHT  0x2000

#define AVF_XSPRING (AVF_FIX_RIGHT|AVF_FIX_LEFT)
#define AVF_YSPRING (AVF_FIX_TOP|AVF_FIX_BOTTOM)

/* clipping */
#define AVF_CLIPPED    0x10000

/* events (not clear how to handle children's request?) */
#define AVFE_MOUSE_MOVE 0x100000
#define AVFE_MOUSE_DRAG 0x200000

#define AVF_DEFAULT (AVF_CLIPPED)

class AVisual : public ARenderer {
protected:
	AContainer *_parent;
	ASize _min_size, _max_size;
	unsigned int _flags;

	void setParent(AContainer *parent) { _parent = parent; }
public:
	AVisual(AContainer *parent, ARect frame, unsigned int flags) : ARenderer(parent?((ARenderer*)parent)->window():NULL, frame), _parent(parent), _flags(flags),
	_min_size(AUndefSize), _max_size(AUndefSize) { OCLASS(AVisual) };
	
	AContainer *parent() { return _parent; }
	unsigned int flags() { return _flags; }
		
	bool isContainer() { return (_flags&AVF_CONTAINER)?true:false; }
	
	virtual void draw() { }

	virtual bool event(AEvent event) {
#ifdef EDEBUG
		ELog("%s: event(%x,%x,%d,(%g,%g))", describe(), event.event, event.flags, event.key, event.location.x, event.location.y);
#endif
		switch (event.event) { // dispatch to virtual event methods
			case AE_MOUSE_UP: return mouseUp(event);
			case AE_MOUSE_DOWN: return mouseDown(event);
			case AE_MOUSE_MOVE: return mouseMove(event);
			case AE_KEY_UP: return keyUp(event);
			case AE_KEY_DOWN: return keyDown(event);
		}
		return false;
	}
	
	// we split up events for convenience when overriding - this may go to another class or not
	virtual bool mouseDown(AEvent event) { return false; }
	virtual bool mouseUp(AEvent event) { return false; }
	virtual bool mouseMove(AEvent event) { return false; }
	virtual bool keyDown(AEvent event) { return false; }
	virtual bool keyUp(AEvent event) { return false; }
	
	// NOTE: setFrame inherited from ARenderer is non-virtual, defined in window coordinates and specific for rendering
	virtual void moveAndResize(ARect frame) { setFrame(frame); }
};

#endif
