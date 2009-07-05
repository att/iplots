/*
 *  AVector.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_VECTOR_H_
#define A_VECTOR_H_

#include "AObject.h"
#include "ATools.h"
#include "AValue.h"

// FIXME: throw an exception or something more reasonable
#define RNS { return NULL; }

class APermutation : public AObject {
protected:
	vsize_t n;
	vsize_t *perm;
	
	void initializePermutations() {
		if (!perm)
			perm = (vsize_t*) malloc(sizeof(vsize_t) * n);
		for (vsize_t i = 0; i < n; i++) perm[i] = i;
	}
	
public:
	APermutation(vsize_t len) : n(len), perm(NULL) {
		OCLASS(APermutation)
	}
	
	virtual ~APermutation() {
		if (perm) free(perm);
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
	
	vsize_t *permutations() { return perm; }
	
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
};

class AVector : public AObject {
protected:
	vsize_t _len;
	bool owned;
public:
	AVector(vsize_t length) : _len(length), owned(true) { OCLASS(AVector) }
	virtual ~AVector() { DCLASS(AVector) }
	
	vsize_t length() const { return _len; }
	
	virtual const double* asDoubles() RNS;
	virtual const float* asFloats() RNS;
	virtual const int* asInts() RNS;
	virtual const char** asStrings() RNS;
	virtual AObject **asObjects() RNS;
	virtual ADataRange range() { return AUndefDataRange; }
	
	virtual bool isFactor() { return false; }
	virtual bool isTime() { return false; }
	virtual APermutation* permutation() { return NULL; }
	
	AObject *objectAt(vsize_t i) { if (i >= _len) return NULL; AObject **x = asObjects(); return x?x[i]:NULL; }
	const char *stringAt(vsize_t i) { if (i >= _len) return NULL; const char **x = asStrings(); return x?x[i]:NULL; }
	double doubleAt(vsize_t i) { if (i >= _len) return NA_double; const double *x = asDoubles(); return x?x[i]:NA_double; }
	
	virtual void transformToFloats(AFloat *f, float a, float b) { } // a * data + b
	virtual void transformToDoubles(double *f, double a, double b) { } // a * data + b
};

#undef RNS

class AObjectVector : public AVector {
protected:
	AObject **_data;
	bool retainContents;
public:
	AObjectVector(AObject **data, vsize_t len, bool copy = true, bool retain = true) : AVector(len), retainContents(retain) {
		_data = (AObject**) (copy ? memdup(data, len * sizeof(AObject*)) : data);
		if (retainContents && _data) for (vsize_t i = 0; i < len; i++) if (_data[i]) _data[i]->retain();		
		OCLASS(AObjectVector);
	}
	
	AObjectVector(AObject *first, ...) : AVector(0) {
		if (first) {
			va_list (varg);
			va_start (varg, first);
			unsigned int objects = 0;
			for (AObject *obj = first; obj; obj = va_arg(varg, AObject*))
				objects++;
			va_end (varg);
			_len = objects;
			_data = (AObject**) malloc(_len * sizeof(AObject*));
			AMEM(_data);
			va_start (varg, first);
			unsigned int i = 0;
			for (AObject *obj = first; obj; obj = va_arg(varg, AObject*))
				_data[i++] = retainContents ? obj->retain() : obj;
			va_end (varg);
		}
		OCLASS(AObjectVector);
	}

	virtual ~AObjectVector() {
		if (_data) {
			if (retainContents) for (vsize_t i = 0; i < _len; i++) if (_data[i]) _data[i]->release();
			free(_data);
		}
		DCLASS(AObjectVector);
	}
	
	virtual vsize_t indexOf(AObject *obj) {
		for (vsize_t i = 0; i < _len; i++)
			if (_data[i] == obj) return i;
		return ANotFound;
	}	

	virtual bool contains(AObject *obj) {
		return indexOf(obj) != ANotFound;
	}
	
	virtual AObject **asObjects() { return _data; };
};

class ASettableObjectVector : public AObjectVector {
public:
	ASettableObjectVector(vsize_t size, bool retain=true) : AObjectVector(0, size, false, retain) {
		_data = (AObject**) calloc(sizeof(AObject*), size);
		AMEM(_data);
		OCLASS(ASettableObjectVector)
	}
	
	virtual void replaceObjectAt(vsize_t ix, AObject *o) {
		if (ix >= _len || o == _data[ix]) return;
		if (_data[ix] && retainContents) _data[ix]->release();
		if (o && retainContents) o->retain();
		_data[ix] = o;
	}
};

// NOTE: mutable vectors are not thread-safe!
// class AMutableObjectVector : public ASettableObjectVector {
class AMutableObjectVector : public AObjectVector {
protected:
	vsize_t _alloc;
public:
/*	AMutableObjectVector(vsize_t initSize = 16, bool retain=true) : ASettableObjectVector((initSize < 16) ? 16 : initSize, retain) {
		_alloc = _len;
		_len = 0;
		OCLASS(AMutableObjectVector);
	} */

	AMutableObjectVector(vsize_t initSize = 16, bool retain=true) : AObjectVector(0, 0, false, retain) {
		_alloc = (initSize < 16) ? 16 : initSize;
		_data =  (AObject**) malloc(sizeof(AObject*) * _alloc);
		ALog("%s: init(_alloc=%d, _data=%p)", describe(), _alloc, _data);
		AMEM(_data);
		OCLASS(AMutableObjectVector);
	}
	
	virtual ~AMutableObjectVector() {
		/* removeAll(); -- actually the superclass already does all this
		 free(_data); */
		DCLASS(AMutableObjectVector);
	}
	
	// if newSize is smaller than the current length, the vector is truncated
	virtual void resize(vsize_t newSize) {
		ALog("%s: resize(%d), _alloc= %d, _len = %d, _data=%p", describe(), newSize, _alloc, _len, _data);
		if (newSize < _len) {
			if (retainContents) for(vsize_t i = newSize; i < _len; i++) if (_data[i]) _data[i]->release();
			_len = newSize;
		}
		_alloc = newSize;
		if (_alloc < 16) _alloc = 16; // 16 is the minimal size - we never use anything smaller
		AObject **new_mem = (AObject**) realloc(_data, _alloc * sizeof(AObject*));
		AMEM(new_mem);
		_data = new_mem;
		ALog("%s: realloc into %p (_alloc = %d, _len = %d)", describe(), _data, _alloc, _len);
	}
	
	virtual vsize_t addObject(AObject *obj) {
		ALog("%s: addObject(), _alloc= %d, _len = %d, _data=%p", describe(), _alloc, _len, _data);
		if (_alloc <= _len)
			resize(_alloc + (_alloc >> 1));
		_data[_len++] = (retainContents && obj) ? obj->retain() : obj;
		return _len - 1;
	}
	
	virtual void setObject(vsize_t index, AObject *obj) {
		if (index >= _alloc)
			resize(index + 64); // FIXME: this is rather arbitrary - we may think more about it
		if (_len > index && _data[index]) _data[index]->release(); // replace previously held object
		while (_len < index) _data[_len++] = 0;
		_data[index] = (retainContents && obj) ? obj->retain() : obj;
	}
	
	virtual void remove(vsize_t index) {
		if (index >= _len) return;
		if (retainContents && _data[index]) _data[index]->release();
		_len--;
		if (index == _len) return;
		memmove(_data + index, _data + index + 1, (_len - index) * sizeof(AObject*));
	}

	// FIXME: currently it only removes the *first* instance of the object!
	virtual void removeObject(AObject *obj) {
		vsize_t i = indexOf(obj);
		if (i != ANotFound)
			remove(i);
	}
	
	virtual void removeAll() {
		if (retainContents)
			for (vsize_t i = 0; i < _len; i++)
				if (_data[i]) _data[i]->release();
		_len = 0;
	}
};

class APlainIntVector : public AVector {
protected:
	int *_data;
	double *d_data;
	float *f_data;
public:
	APlainIntVector(const int *data, vsize_t len, bool copy=false) : AVector(len), f_data(0), d_data(0) {
		_data = (int*)(copy?memdup(data, len * sizeof(int)):data); OCLASS(APlainIntVector)
	}
	
	virtual ~APlainIntVector() {
		if (owned) free(_data);
		if (d_data) free(d_data);
		if (f_data) free(f_data);
		DCLASS(APlainIntVector)
	}
	
	virtual const int *asInts() { return _data; }
	virtual const double *asDoubles() {
		if (!d_data) {
			d_data = (double*) malloc(_len * sizeof(double));
			for (int i=0; i<_len; i++) d_data[i] = (double)_data[i];
		}
		return d_data;
	}
	virtual const float *asFloats() {
		if (!f_data) {
			f_data = (float*) malloc(_len * sizeof(float));
			for (int i=0; i<_len; i++) f_data[i] = (float)_data[i];
		}
		return f_data;
	}
	
	virtual ADataRange range() {
		ADataRange r = AUndefDataRange;
		if (length()) {
			double e = r.begin = _data[0];
			for (int i = 0; i < length(); i++)
				if (_data[i] < r.begin) r.begin = _data[i]; else if (_data[i] > e) e = _data[i];
			r.length = e - r.begin;
		}
		return r;
	}
	
	virtual void transformToFloats(AFloat *f, float a, float b) { // a * data + b
		for (int i = 0; i < length(); i++)
			f[i] = ((float)_data[i]) * a + b;
	}
	
	virtual void transformToDoubles(double *f, double a, double b) { // a * data + b
		for (int i = 0; i < length(); i++)
			f[i] = ((double)_data[i]) * a + b;
	}	
};


#endif
