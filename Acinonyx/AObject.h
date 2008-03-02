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
#include <stdio.h>
#include <stdlib.h>

class AObject {
	int arpp;
	static int arpe;
	static AObject *arp[1024];
	int refcount;
	
public:
	AObject() : refcount(1) {
		printf("new AObject<%p>\n", this);
	}
	
	virtual ~AObject() {
		printf("AObject<%p> dealloc\n", this);
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
