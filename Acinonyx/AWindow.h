/*
 *  AWindow.h - abtract Acinonyx window class
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 *  lang: C++
 */

#ifndef A_WINDOW_H
#define A_WINDOW_H

#include "ATypes.h"
#include "AObject.h"

// IDEA: use window-global query object
class AWindow : public AObject
{
	AObject *modalOwner;
	AObject *_rootVisual;
public:
	int *dirtyFlag;

	AWindow(ARect frame) : modalOwner(0), _rootVisual(0), dirtyFlag(0) { OCLASS(AWindow) };
	virtual ~AWindow() {
		if (_rootVisual) _rootVisual->release();
		DCLASS(AWindow)
	}

	AObject *rootVisual() { return _rootVisual; }
	
	void setRootVisual(AObject *rv) {
		if (_rootVisual) _rootVisual->release();
		_rootVisual = rv;
		if (rv) rv->retain();
	}
	
	// enter modal mode with the given owner. returns false if owner is NULL or if already in modal mode by other owner, true otherwise
	virtual bool enterModal(AObject *owner) {
		if (owner && owner == modalOwner) return true;
		if (modalOwner || !owner) return false;
		modalOwner = owner;
		owner->retain();
		return true;
	}
	
	virtual bool leaveModal(AObject *owner) {
		if (modalOwner && modalOwner == owner) {
			modalOwner->release();
			modalOwner = NULL;
			return true;
		}
		return false;
	}
	
	virtual void redraw() {};
	
	/* the following are rendering methods that are off-loaded to the window implementation since they are not part of OpenGL */
	virtual void glstring(APoint pt, APoint adj, AFloat rot, const char *txt) {}; //  we need some implementation help here since gl cannot draw text
	virtual void glfont(const char *name, AFloat size) {}
	
	void setDirtyFlag(int *newDF) { dirtyFlag = newDF; };
	bool isDirty() { return (dirtyFlag && dirtyFlag[0] != 0); }

	virtual bool canClose() { return true; }
	virtual void close() {};

	virtual void setVisible(bool flag) {};
	virtual bool visible() { return false; }
};

#endif
