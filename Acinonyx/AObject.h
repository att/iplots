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
#if __LP64__
#define PTR2INT(X) ((int) (long) (X))
#else
#define PTR2INT(X) ((int) (X))
#endif
#define OCLASS(X) { _className = # X ; _classSize = sizeof(X); printf("+ %08x %06x %s [%d]\n", PTR2INT(this), _objectSerial, _className, _classSize); }
#define DCLASS(X) { printf("- %08x %06x %s/%s [%d]\n", PTR2INT(this), _objectSerial, _className, # X, _classSize); }
#else
#define OCLASS(X)
#define DCLASS(X)
#endif

class AObject {
protected:
	int refcount;
#ifdef AUTORELEASE_SUPPORT
	int arpp;
	static int arpe;
	static AObject *arp[1024];
#endif
	static char desc_buf[512];
#ifdef ODEBUG
public:
	const char *_className;
	unsigned int _classSize;
	object_serial_t _objectSerial;
#endif

public:
	int tag_;

	AObject() : refcount(1), tag_(0) 
#ifdef AUTORELEASE_SUPPORT
	, arpp(0)
#endif
	{
#ifdef ODEBUG
		_objectSerial = _globalObjectSerial++; // the only class handling serials is AObject to make sure that it is called only once
#endif
		OCLASS(AObject)
	}
	
	virtual ~AObject() {
		DCLASS(AObject)
	}
	
	AObject* retain() {
#ifdef ODEBUG
		printf("> %08x %06x %s [%d] %d++\n", PTR2INT(this), _objectSerial, _className, _classSize, refcount);
#endif
		refcount++; return this;
	}

	void release() {
#ifdef ODEBUG
		printf("< %08x %06x %s [%d] %d--\n", PTR2INT(this), _objectSerial, _className, _classSize, refcount);
#endif
#ifdef AUTORELEASE_SUPPORT
		if (--refcount < 1 && arpp == 0)
			delete this;
#else
		if (--refcount < 1)
			delete this;
#endif
	}
	
#ifdef AUTORELEASE_SUPPORT
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
#endif
	
	virtual void notification(AObject *source, notifid_t nid) { };

	virtual char *describe() {
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d]>", this, refcount, _objectSerial, _className, _classSize);
#else
		snprintf(desc_buf, 512, "<AObject %p>", this);
#endif
		return desc_buf;
	}

	char *_ptr_describe() {
#ifdef ODEBUG
		snprintf(desc_buf, 512, "<%p/%d %04x %s [%d]>", this, refcount, _objectSerial, _className, _classSize);
#else
		snprintf(desc_buf, 512, "<AObject %p>", this);
#endif
		return desc_buf;
	}
};


extern "C" void* A_memdup(const void *ptr, vsize_t len, AObject *owner);
extern "C" char* A_strdup(const char *str, AObject *owner);
#ifdef ODEBUG
extern "C" void* A_alloc(vsize_t size, vsize_t elt_size, AObject *owner);
extern "C" void* A_calloc(vsize_t size, vsize_t elt_size, AObject *owner);
extern "C" void* A_realloc(void *ptr, vsize_t size, AObject *owner);
extern "C" void A_free(void *ptr);
extern "C" void A_transfer(void *ptr, AObject *ptr);
#else
#define A_alloc(X, Y, O) malloc(((vsize_t)(X)) * ((vsize_t)(Y)))
#define A_calloc(X, Y, O) calloc(X, Y)
#define A_free(X) free(X)
#define A_transfer(X, O)
#define A_realloc(X, S, O) realloc(X, S)
#endif

#define AAlloc(X) A_alloc(X, 1, this)
#define AZAlloc(X, Y) A_calloc(X, Y, this)
#define AFree(X) A_free(X)
#define AMemTransferOwnership(X, O) A_transfer(X, O)
#define ARealloc(X, Y) A_realloc(X, Y, this)
#define memdup(X, L) A_memdup(X, L, this)
#define strdup(X) A_strdup(X, this)

// Sentinel is the only class outside the AObject hierarchy and its sole purpose is
// to allow automatic cleanup of locally created objects. It assumes ownership of an object
// and releases that object as soon as the sentinel itself is released.

class AObjectSentinel {
public:
	AObject *o;

	AObjectSentinel(AObject *obj) {
		o = obj;
	}
	
	~AObjectSentinel() {
		if (o) o->release();
	}
};

// Use LOCAL as follows:
// AObject *foo = new AObject( ... ); 
// LOCAL(foo);

#ifndef SWIG
extern "C" {
	void AObject_retain(void *o);
	void AObject_release(void *o);
}
#endif

#define LOCAL(X) AObjectSentinel local_os_ ## X = AObjectSentinel(X)

#endif
