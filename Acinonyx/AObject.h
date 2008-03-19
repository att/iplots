/*
 *  AObject.h - a general-purpose object implementing reference counting
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_OBJECT_H
#define A_OBJECT_H

#include "ATypes.h"
#include "ATools.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef ODEBUG
typedef unsigned int object_serial_t;
extern object_serial_t _globalObjectSerial;
#define OCLASS(X) { _className = # X ; _classSize = sizeof(X); printf("+ %08x %06x %s [%d]\n", (int) this, _objectSerial, _className, _classSize); }
#define DCLASS(X) { printf("- %08x %06x %s/%s [%d]\n", this, _objectSerial, _className, # X, _classSize); }
#else
#define OCLASS(X)
#define DCLASS(X)
#endif

class AObject {
	int arpp;
	static int arpe;
	static AObject *arp[1024];
	int refcount;
#ifdef ODEBUG
public:
	const char *_className;
	unsigned int _classSize;
	object_serial_t _objectSerial;
#endif

public:
	AObject() : refcount(1) {
#ifdef ODEBUG
		_objectSerial = _globalObjectSerial++; // the only class handling serials is AObject to make sure that it is called only once
#endif
		OCLASS(AObject)
	}
	
	virtual ~AObject() {
		DCLASS(AObject)
	}
	
	void retain() { refcount++; }

	void release() {
		if (--refcount < 1 && arpp == 0)
			delete this;
	}
	
	AObject *autorelease() {
		arpp = arp_add(this);
		refcount--;
		return this;
	}

private:
	static int arp_add(AObject *o) {
		int i = 0;
		while (arp[i] && i<1024) i++;
		if (i == 1024) {
			fprintf(stderr, "FATAL: ARP overflow!\n");
			exit(1);
		}
		AObject::arp[i] = o;
		if (i > arpe) arpe = i;
		return i;
	}
	
public:
	static void ARPFlush() {
		int i = 0;
		while (i <= arpe) {
			if (arp[i] && arp[i]->refcount < 1) {
				delete arp[i];
				arp[i] = 0;
			}
			i++;
		}
		while (arpe > 0 && !arp[arpe]) arpe--;
	}
	
};

#endif
