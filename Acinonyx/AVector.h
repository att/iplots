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

class AVector : public AObject {
protected:
	vsize_t _len;
public:
	AVector(vsize_t length) : _len(length) { OCLASS(AVector) }
	virtual ~AVector() { DCLASS(AVector) }
	
	vsize_t length() const { return _len; }
	
	virtual const double* asDoubles() RNS;
	virtual const float* asFloats() RNS;
	virtual const int* asInts() RNS;
	virtual const char** asStrings() RNS;
	virtual AObject **asObjects() RNS;
	virtual ADataRange range() { return AUndefDataRange; }
	
	AObject *objectAt(vsize_t i) { if (i >= _len) return NULL; AObject **x = asObjects(); return x?x[i]:NULL; }
	const char *stringAt(vsize_t i) { if (i >= _len) return NULL; const char **x = asStrings(); return x?x[i]:NULL; }
	double doubleAt(vsize_t i) { if (i >= _len) return NA_double; const double *x = asDoubles(); return x?x[i]:NA_double; }
	
	virtual void transformToFloats(AFloat *f, float a, float b) { } // a * data + b
	virtual void transformToDoubles(double *f, double a, double b) { } // a * data + b
};

#undef RNS

class AMarker;

class AObjectWithMarker {
protected:
	AMarker *_marker;
	
public:
	AObjectWithMarker(AMarker *m) : _marker(m ? ((AMarker*)((AObject*)m)->retain()) : m) {}
	virtual ~AObjectWithMarker() { if (_marker) ((AObject*)_marker)->release(); }
	
	AMarker *marker() { return _marker; }
};

class ADataVector : public AVector, public AObjectWithMarker {
public:
	ADataVector(AMarker *mark, vsize_t len) : AVector(len), AObjectWithMarker(mark) { OCLASS(ADataVector); };
	virtual ~ADataVector() {
		DCLASS(ADataVector);
	}
};

class AObjectVector : public AVector {
protected:
	AObject **_data;
public:
	AObjectVector(AObject **data, vsize_t len, bool copy = true) : AVector(len) {
		data = (AObject**) (copy?memdup(data, len * sizeof(AObject*)):data);
		for (vsize_t i = 0; i < len; i++) if (_data[i]) _data[i]->retain();		
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
				_data[i++] = obj->retain();
			va_end (varg);
		}
		OCLASS(AObjectVector);
	}

	virtual ~AObjectVector() {
		for (vsize_t i = 0; i < _len; i++) if (_data[i]) _data[i]->release();
		free(_data);
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

// NOTE: mutable vectors are not thread-safe!
class AMutableObjectVector : public AObjectVector {
protected:
	vsize_t _alloc;
public:
	AMutableObjectVector(vsize_t initSize = 16) : AObjectVector(0, 0, false), _alloc(initSize) {
		if (_alloc < 16) _alloc = 16; // 16 is the minimal size - we never use anything smaller
		_data = (AObject**) malloc(sizeof(AObject*) * _alloc);
		AMEM(_data);
		OCLASS(AMutableObjectVector);
	}
	
	virtual ~AMutableObjectVector() {
		removeAll();
		free(_data);
		DCLASS(AMutableObjectVector);
	}
	
	// if newSize is smaller than the current length, the vector is truncated
	virtual void resize(vsize_t newSize) {
		if (newSize < _len) {
			for(vsize_t i = newSize; i < _len; i++) if (_data[i]) _data[i]->release();
			_len = newSize;
		}
		_alloc = newSize;
		if (_alloc < 16) _alloc = 16; // 16 is the minimal size - we never use anything smaller
		AMEM(_data = realloc(_data, _alloc * sizeof(AObject*)));
	}
	
	virtual vsize_t addObject(AObject *obj) {
		if (_alloc <= _len)
			resize(_alloc + (_alloc >> 1));
		_data[_len++] = obj ? obj->retain() : obj;
		return _len - 1;
	}
	
	virtual void setObject(vsize_t index, AObject *obj) {
		if (index >= _alloc)
			resize(index + 64); // FIXME: this is rather arbitrary - we may think more about it
		if (_len > index && _data[index]) _data[index]->release(); // replace previously held object
		while (_len < index) _data[_len++] = 0;
		_data[index] = obj ? obj->retain() : obj;
	}
	
	virtual void remove(vsize_t index) {
		if (index >= _len) return;
		if (_data[index]) _data[index]->release();
		_len--;
		if (index == _len) return;
		memmove(_data + index + 1, _data + index, _len - index);
	}

	// FIXME: currently it only removes the *first* instance of the object!
	virtual void removeObject(AObject *obj) {
		vsize_t i = indexOf(obj);
		if (i != ANotFound)
			remove(i);
	}
	
	virtual void removeAll() {
		for (vsize_t i = 0; i < _len; i++)
			if (_data[i]) _data[i]->release();
		_len = 0;
	}
};

class AFloatVector : public ADataVector {
protected:
	float *_data;
	double *d_data;
	int *i_data;
public:
	AFloatVector(AMarker *m, float *data, vsize_t len, bool copy) : ADataVector(m, len), d_data(0), i_data(0) {
		_data = (float*) (copy?memdup(data, len * sizeof(float)):data); OCLASS(AFloatVector)
	}
	AFloatVector(float *data, vsize_t len, bool copy) : ADataVector(0, len), d_data(0), i_data(0) {
		_data = (float*) (copy?memdup(data, len * sizeof(float)):data); OCLASS(AFloatVector)
	}
	AFloatVector(float *data, vsize_t len) : ADataVector(0, len), d_data(0), i_data(0) {
		_data = (float*) memdup(data, len * sizeof(float)); OCLASS(AFloatVector)
	}
	
	virtual ~AFloatVector() {
		free(_data);
		if (d_data) free(d_data);
		if (i_data) free(i_data);
		DCLASS(AFloatVector)
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
	
	virtual const float *asFloats() { return _data; }
	virtual const double *asDoubles() {
		if (!d_data) {
			d_data = (double*) malloc(_len * sizeof(double));
			for (int i=0; i<_len; i++) d_data[i] = (double)_data[i];
		}
		return d_data;
	}
	virtual const int *asInts() {
		if (!i_data) {
			i_data = (int*) malloc(_len * sizeof(int));
			for (int i=0; i<_len; i++) i_data[i] = (int)_data[i];
		}
		return i_data;
	}

	virtual void transformToFloats(AFloat *f, float a, float b) { // a * data + b
		for (int i = 0; i < length(); i++)
			f[i] = _data[i] * a + b;
	}

	virtual void transformToDoubles(double *f, double a, double b) { // a * data + b
		for (int i = 0; i < length(); i++)
			f[i] = _data[i] * a + b;
	}
};

class ADoubleVector : public ADataVector {
protected:
	double *_data;
	float *f_data;
	int *i_data;
public:
	ADoubleVector(AMarker *m, double *data, vsize_t len, bool copy) : ADataVector(m, len), f_data(0), i_data(0) {
		_data = copy?(double*)memdup(data, len * sizeof(double)):data; OCLASS(ADoubleVector)
	}
	ADoubleVector(double *data, vsize_t len, bool copy) : ADataVector(0, len), f_data(0), i_data(0) {
		_data = copy?(double*)memdup(data, len * sizeof(double)):data; OCLASS(ADoubleVector)
	}
	ADoubleVector(const double *data, vsize_t len) : ADataVector(0, len), f_data(0), i_data(0) {
		_data = (double*)memdup(data, len * sizeof(double)); OCLASS(ADoubleVector)
	}	
	virtual ~ADoubleVector() {
		free(_data);
		if (f_data) free(f_data);
		if (i_data) free(i_data);
		DCLASS(ADoubleVector)
	}
	
	virtual const double *asDouble() { return _data; }		

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
	
	virtual const float *asFloats() {
		if (!f_data) {
			f_data = (float*) malloc(_len * sizeof(float));
			AMEM(f_data);
			for (vsize_t i = 0; i < _len; i++)
				f_data[i] = (float) _data[i];
		}
		return f_data;
	}

	virtual const int *asInts() {
		if (!i_data) {
			i_data = (int*) malloc(_len * sizeof(int));
			AMEM(i_data);
			for (vsize_t i = 0; i < _len; i++)
				i_data[i] = (int)_data[i];
		}
		return i_data;
	}
	
	virtual void transformToFloats(AFloat *f, float a, float b) { // a * data + b
		for (int i = 0; i < length(); i++)
			f[i] = _data[i] * a + b;
	}
	
	virtual void transformToDoubles(double *f, double a, double b) { // a * data + b
		for (int i = 0; i < length(); i++)
			f[i] = _data[i] * a + b;
	}
};

class AIntVector : public ADataVector {
protected:
	int *_data;
	double *d_data;
	float *f_data;
public:
	AIntVector(AMarker *m, const int *data, vsize_t len, bool copy) : ADataVector(m, len), f_data(0), d_data(0) {
		_data = (int*)(copy?memdup(data, len * sizeof(int)):data); OCLASS(AIntVector)
	}
	AIntVector(const int *data, vsize_t len, bool copy) : ADataVector(0, len), f_data(0), d_data(0) {
		_data = (int*)(copy?memdup(data, len * sizeof(int)):data); OCLASS(AIntVector)
	}
	AIntVector(const int *data, vsize_t len) : ADataVector(0, len), f_data(0), d_data(0) {
		_data = (int*)memdup(data, len * sizeof(int)); OCLASS(AIntVector)
	}
	
	virtual ~AIntVector() {
		free(_data);
		if (d_data) free(d_data);
		if (f_data) free(f_data);
		DCLASS(AIntVector)
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
};

class AFactorVector : public AIntVector {
protected:
	char **_names;
	int _levels;
	char **s_data;
public:
	AFactorVector(const int *data, int len, const char **names, int n_len) : AIntVector(data, len, false), _levels(n_len) {
		_names = (char**) names; OCLASS(AFactorVector)
	}
	virtual ~AFactorVector() {
		for (int i = 0; i < _levels; i++) if (_names[i]) free(_names[i]);
		free(_names);
		DCLASS(AFactorVector)
	}
	virtual const char **asStrings() {
		if (!s_data) {
			s_data = (char**) malloc(_len * sizeof(char*));
			for (int i = 0; i < _len; i++) s_data[i] = (_data[i] < 0 || _data[i] >= _levels)?NULL:_names[_data[i]];
		}
		return (const char**) s_data;
	}
};

// TODO: mutable vectors ( + notification?)

#endif
