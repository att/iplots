/*
 *  ANotfier.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/4/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_NOTIFIER_H
#define A_NOTIFIER_H

#include "AVector.h"

class ANotifierInterface {
protected:
	AMutableObjectVector *_dependents;

public:
	ANotifierInterface(bool retain = false) : // do not retain dependents by default - we assume that they are registering themselves and thus retaning would prevent them from re-registering on dealloc
	_dependents(new AMutableObjectVector(16, retain)) {	}

	virtual ~ANotifierInterface() {
		_dependents->release();
	}
	
	virtual void add(AObject *obj) {
		if (!_dependents->contains(obj))
			_dependents->addObject(obj);
	}
	
	virtual void remove(AObject *obj) {
		_dependents->removeObject(obj);
	}
	
	virtual void sendNotification(AObject *source, notifid_t nid) {
		vsize_t i = 0, n = _dependents->length();
		while (i < n)
			_dependents->objectAt(i++)->notification(source, nid);
	}
};
	
#endif
