/*
 *  APermutation.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/2/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#include "ASort.h"
#include "AObject.h"

class APermutation : public AObject {
protected:
	vsize_t n;
	vsize_t *perm;
	
	void initializePermutations() {
		if (!perm) {
			perm = (vsize_t*) AAlloc(sizeof(vsize_t) * n);
			AMEM(perm);
		}
		for (vsize_t i = 0; i < n; i++) perm[i] = i;
	}
	
public:
	APermutation(vsize_t len) : n(len), perm(NULL) {
		OCLASS(APermutation)
	}
	
	virtual ~APermutation() {
		if (perm) AFree(perm);
	}
	
	vsize_t size() {
		return n;
	}
	
	vsize_t permutationOf(vsize_t index) {
		if (!perm) return index;
		return (index >= n) ? index : perm[index];
	}
	
	vsize_t permutationAt(vsize_t index) {
		if (!perm) return index;
		for(vsize_t i = 0; i < n; i++)
			if (perm[i] == index)
				return i;
		return index;
	}
	
	const vsize_t *permutations() { return (const vsize_t*) perm; }
	
	void swap(vsize_t a, vsize_t b) {
		if (!perm) initializePermutations();
		vsize_t h = perm[a]; perm[a] = perm[b]; perm[b] = h;
	}
	
	void moveToIndex(vsize_t a, vsize_t ix) {
		if (!perm) initializePermutations();
		if (ix >= n) ix = n - 1;
		vsize_t prev = perm[a];
		if (prev == ix) return;
		if (ix < prev) {
			for (vsize_t i = 0; i < n; i++)
				if (perm[i] >= ix && perm[i] < prev)
					perm[i]++;
		} else { /* prev < ix */
			for (vsize_t i = 0; i < n; i++)
				if (perm[i] > prev && perm[i] <= ix)
					perm[i]--;
		}
		perm[a] = ix;
	}
	
	void reset() {
		if (!perm) return;
		AFree(perm);
		perm = NULL;
	}
	
	void orderAccordingToVSizes(const vsize_t *array) {
		if (!perm) initializePermutations();
		vsize_t *iperm = (vsize_t*) AAlloc(sizeof(vsize_t) * n);
		for (vsize_t i = 0; i < n; i++) iperm[perm[i]] = i;
		quicksortVSizesPerm(array, perm, iperm, 0, n);
		for (vsize_t i = 0; i < n; i++) perm[iperm[i]] = i;
		AFree(iperm);
	}
	
	void orderLexicographically(const char **array) {
		if (!perm) initializePermutations();
		vsize_t *iperm = (vsize_t*) AAlloc(sizeof(vsize_t) * n);
		for (vsize_t i = 0; i < n; i++) iperm[perm[i]] = i;
		quicksortStringsPerm(array, perm, iperm, 0, n);
		for (vsize_t i = 0; i < n; i++) perm[iperm[i]] = i;
		AFree(iperm);
	}

	void orderNumerically(const char **array) {
		if (!perm) initializePermutations();
		vsize_t *iperm = (vsize_t*) AAlloc(sizeof(vsize_t) * n);
		for (vsize_t i = 0; i < n; i++) iperm[perm[i]] = i;
		quicksortNumericStringsPerm(array, perm, iperm, 0, n);
		for (vsize_t i = 0; i < n; i++) perm[iperm[i]] = i;
		AFree(iperm);
	}
};
