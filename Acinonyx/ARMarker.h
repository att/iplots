/*
 *  ARMarker.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/16/10.
 *  Copyright 2010 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_R_MARKER
#define A_R_MARKER

#include "AMarker.h"

/*** ARMarker is a marker with an additional list of R callbacks. Although those callbacks are also registered as notifiers the extra list is needed to filter out callbacks such that it is possible to remove them all without affecting other depenendents, find existing callbacks etc. */
class ARMarker : public AMarker {
protected:
	AMutableObjectVector *callbacks_;

public:
	ARMarker(vsize_t len) : AMarker(len) {
		callbacks_ = new AMutableObjectVector(8, true);
		OCLASS(ARMarker)
	}
	
	virtual ~ARMarker() {
		callbacks_->release();
	}
	
	void addCallback(ARCallbackDependent *callback) {
		// add it to the dependents list
		add(callback);
		// but also tothe callbacks
		callbacks_->addObject(callback);
	}
	
	void removeCallback(ARCallbackDependent *callback) {
		if (callbacks_->contains(callback)) {
			callbacks_->removeObject(callback);
			remove(callback);
		}
	}
	
	AObjectVector *callbacks() {
		return (AObjectVector*) callbacks_;
	}
	
	void removeAllCallbacks() {
		vsize_t n = callbacks_->length();
		if (n) {
			for (vsize_t i = 0; i < n; i++)
				remove(callbacks_->objectAt(i));
			callbacks_->removeAll();
		}
	}
};

#endif
