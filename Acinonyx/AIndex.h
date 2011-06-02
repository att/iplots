/*
 *  AIndex.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/9/10.
 *  Copyright 2010 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_INDEX_H
#define A_INDEX_H

#include "AObject.h"

typedef int bmsrc_t;

class ADenseBitmap : public AObject {
protected:
	byte_t *bits;
	vsize_t vlen;
public:
	ADenseBitmap(bmsrc_t *src, vsize_t n, bmsrc_t mask = 1) : vlen(n) {
		vsize_t bf_len = n >> 3, i = 0;
		if (n & 7) bf_len++;
		bits = (byte_t*) AAlloc(bf_len);
		AMEM(bits);
		byte_t cv = 0, *here = bits;
		for (; i < n; i++) {
			cv <<= 1;
			if (src[i] & mask)
				cv |= 1;
			if ((i & 7) == 7) {
				*(here++) = cv;
				cv = 0;
			}
		}
		if (i & 7) {
			while (i++ & 7)
				cv <<= 1;
			*here = cv;
		}
		OCLASS(ADenseBitmap)
	}
	
	virtual ~ADenseBitmap() {
		AFree(bits);
		DCLASS(ADenseBitmap)
	}
	
	vsize_t length() {
		return vlen;
	}
	
	bool at(vsize_t index) {
		if (index >= vlen) return false;
		vsize_t pos = index >> 3;
		byte_t bit = 1 << (index & 7);
		return ((bits[pos] & bit) != 0);
	}
	
	void restore(bmsrc_t *dst, bmsrc_t mask = 1) {
		vsize_t n = vlen;
		byte_t *here = bits, cv = 0x80;
		while (n--) {
			if (here[0] & cv)
				*(dst++) |= mask;
			else
				*(dst++) &= ~mask;
			cv >>= 1;
			if (!cv) {
				cv = 0x80;
				here++;
			}	
		}
	}
};

#endif
