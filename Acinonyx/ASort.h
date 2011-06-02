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
#include <stdlib.h>
#include <string.h>

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

#define str_compare(A,B) ::strcmp((A)?(A):"", (B)?(B):"")

static void quicksortStringsPerm(const char **array, const vsize_t *perm, vsize_t *iperm, vsize_t iFrom, vsize_t len) {
	if (len < 2) return;
	if (len == 2) {
		int res = str_compare(array[iperm[iFrom]], array[iperm[iFrom + 1]]);
		if (res > 0 || (res == 0 && perm[iperm[iFrom]] > perm[iperm[iFrom + 1]]))
			swapVSize(iperm, iFrom, iFrom + 1);
		return;
	}
	vsize_t pivot = iFrom + len / 2, ipivot = iperm[pivot];
	const char* val = array[ipivot];
	vsize_t pos = perm[ipivot];
	vsize_t first = iFrom;
	swapVSize(iperm, iFrom + len - 1, pivot);
	for (vsize_t i = iFrom; i < iFrom + len - 1; i++) {
		int res = str_compare(array[iperm[i]], val);
		if (res < 0 || (res == 0 && perm[iperm[i]] < pos))
			swapVSize(iperm, i, first++);
	}
	swapVSize(iperm, first, iFrom + len - 1);
	quicksortStringsPerm(array, perm, iperm, iFrom, first - iFrom);
	quicksortStringsPerm(array, perm, iperm, first + 1, len + iFrom - first - 1);
}

// compare strings numerically by using the first part as a fp number and in equality compare the rest of the strings.
static int nstr_compare(const char *A, const char *B) {
	char *endA, *endB;
	double a = ::strtod(A, &endA);
	double b = ::strtod(B, &endB);
	if (a < b) return -1;
	if (a > b) return 1;
	return ::strcmp(endA, endB);
}

static void quicksortNumericStringsPerm(const char **array, const vsize_t *perm, vsize_t *iperm, vsize_t iFrom, vsize_t len) {
	if (len < 2) return;
	if (len == 2) {
		int res = nstr_compare(array[iperm[iFrom]], array[iperm[iFrom + 1]]);
		if (res > 0 || (res == 0 && perm[iperm[iFrom]] > perm[iperm[iFrom + 1]]))
			swapVSize(iperm, iFrom, iFrom + 1);
		return;
	}
	vsize_t pivot = iFrom + len / 2, ipivot = iperm[pivot];
	const char* val = array[ipivot];
	vsize_t pos = perm[ipivot];
	vsize_t first = iFrom;
	swapVSize(iperm, iFrom + len - 1, pivot);
	for (vsize_t i = iFrom; i < iFrom + len - 1; i++) {
		int res = nstr_compare(array[iperm[i]], val);
		if (res < 0 || (res == 0 && perm[iperm[i]] < pos))
			swapVSize(iperm, i, first++);
	}
	swapVSize(iperm, first, iFrom + len - 1);
	quicksortNumericStringsPerm(array, perm, iperm, iFrom, first - iFrom);
	quicksortNumericStringsPerm(array, perm, iperm, first + 1, len + iFrom - first - 1);
}

#endif
