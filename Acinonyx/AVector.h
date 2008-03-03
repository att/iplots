/*
 *  AVector.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#include "AObject.h"
#include "ATools.h"
#include "AValue.h"

// FIXME: throw an exception or something more reasonable
#define RNS { return NULL; }

class AVector : public AObject {
protected:
	vsize_t _len;
public:
	AVector(vsize_t length) : _len(length) {}
	virtual ~AVector() {}
	
	vsize_t length() const { return _len; }
	
	virtual const double* asDoubles() RNS;
	virtual const float* asFloats() RNS;
	virtual const int* asInts() RNS;
	virtual const char** asStrings() RNS;
	virtual AObject **asObjects() RNS;
	
	AObject *objectAt(vsize_t i) { if (i >= _len) return NULL; AObject **x = asObjects(); return x?x[i]:NULL; }
	const char *stringAt(vsize_t i) { if (i >= _len) return NULL; const char **x = asStrings(); return x?x[i]:NULL; }
	double doubleAt(vsize_t i) { if (i >= _len) return NA_double; const double *x = asDoubles(); return x?x[i]:NA_double; }
	
};

#undef RNS

class AFloatVector : public AVector {
protected:
	float *_data;
	double *d_data;
	int *i_data;
public:
	AFloatVector(float *data, vsize_t len, bool copy) : AVector(len), d_data(0), i_data(0) {
		_data = (float*) (copy?memdup(data, len * sizeof(float)):data);
	}
	AFloatVector(float *data, vsize_t len) : AVector(len), d_data(0), i_data(0) {
		_data = (float*) memdup(data, len * sizeof(float));
	}
	
	virtual ~AFloatVector() {
		free(_data);
		if (d_data) free(d_data);
		if (i_data) free(i_data);
	}
	
	virtual const float *asFloats() const { return _data; }		
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
};

class ADoubleVector : public AVector {
protected:
	double *_data;
	float *f_data;
	int *i_data;
public:
	ADoubleVector(double *data, vsize_t len, bool copy) : AVector(len), f_data(0), i_data(0) {
		_data = copy?(double*)memdup(data, len * sizeof(double)):data;
	}
	ADoubleVector(double *data, vsize_t len) : AVector(len), f_data(0), i_data(0) {
		_data = (double*)memdup(data, len * sizeof(double));
	}	
	virtual ~ADoubleVector() {
		free(_data);
		if (f_data) free(f_data);
		if (i_data) free(i_data);
	}
	
	virtual const double *asDouble() const { return _data; }		
	virtual const float *asFloats() {
		if (!f_data) {
			f_data = (float*) malloc(_len * sizeof(float));
			for (int i=0; i<_len; i++) f_data[i] = (float)_data[i];
		}
		return f_data;
	}
	virtual const int *asInts() {
		if (!i_data) {
			i_data = (int*) malloc(_len * sizeof(int));
			for (int i=0; i<_len; i++) i_data[i] = (int)_data[i];
		}
		return i_data;
	}
};

class AIntVector : public AVector {
protected:
	int *_data;
	double *d_data;
	float *f_data;
public:
	AIntVector(const int *data, vsize_t len, bool copy) : AVector(len), f_data(0), d_data(0) {
		_data = (int*)(copy?memdup(data, len * sizeof(int)):data);
	}
	AIntVector(const int *data, vsize_t len) : AVector(len), f_data(0), d_data(0) {
		_data = (int*)memdup(data, len * sizeof(int));
	}
	
	virtual ~AIntVector() {
		free(_data);
		if (d_data) free(d_data);
		if (f_data) free(f_data);
	}
	
	virtual const int *asInts() const { return _data; }
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
		_names = (char**) names;
	}
	virtual ~AFactorVector() {
		for (int i = 0; i < _levels; i++) if (_names[i]) free(_names[i]);
		free(_names);
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

