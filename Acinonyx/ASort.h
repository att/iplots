/*
 *  ASort.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/2/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_SORT_H
#define A_SORT_H

#include "ATypes.h"

//--- sorting tools

static void swapVSize(vsize_t *array, vsize_t a, vsize_t b) {
	vsize_t h = array[a];
	array[a] = array[b];
	array[b] = h;
}

/** Quicksort on a vsize_t array using permutation array to break ties (essentially secondary vsize_t array)
 @param array  values to sort
 @param perm   permutation array (if two array elements have the same value, the order is defined by perm values)
 @param iperm  array of positions, will be perturbed accordingly to the sort order (must be initialized to some order 1:n)
 @param iFrom  start index of the subarray to sort
 @param len    length of the subarray to sort */
static void quicksortVSizesPerm(const vsize_t *array, const vsize_t *perm, vsize_t *iperm, vsize_t iFrom, vsize_t len) {
	if (len < 2) return;
	if (len == 2) {
		if (array[iperm[iFrom]] > array[iperm[iFrom + 1]] ||
		    (array[iperm[iFrom]] == array[iperm[iFrom + 1]] && perm[iperm[iFrom]] > perm[iperm[iFrom + 1]]))
			swapVSize(iperm, iFrom, iFrom + 1);
		return;
	}
	vsize_t pivot = iFrom + len / 2, ipivot = iperm[pivot];
	vsize_t val = array[ipivot];
	vsize_t pos = perm[ipivot];
	vsize_t first = iFrom;
	swapVSize(iperm, iFrom + len - 1, pivot);
	for (vsize_t i = iFrom; i < iFrom + len - 1; i++)
		if (array[iperm[i]] < val || (array[iperm[i]] == val && perm[iperm[i]] < pos))
			swapVSize(iperm, i, first++);
	swapVSize(iperm, first, iFrom + len - 1);
	quicksortVSizesPerm(array, perm, iperm, iFrom, first - iFrom);
	quicksortVSizesPerm(array, perm, iperm, first + 1, len + iFrom - first - 1);
}

#endif
