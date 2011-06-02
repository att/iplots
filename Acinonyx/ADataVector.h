/*
 *  ADataVector.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/4/09.
 *  Copyright 2009 Simon Urbanek. All rights reserved.
 *
 */

/******** THIS IS CURRENTLY UNUSED - ADataVector is defined in AVector.h *********/

#ifndef A_DATAVECTOR_H
#define A_DATAVECTOR_H

#include "AVector.h"
#include "ANotfier.h"
#include "AMarker.h"
#include "ATable.h"

/*
class AObjectWithMarker {
protected:
	AMarker *_marker;
 
public:
	AObjectWithMarker(AMarker *m) : _marker(NULL) { if (m) { _marker = m; ((AObject*)_marker)->retain(); } }
	virtual ~AObjectWithMarker() { if (_marker) ((AObject*)_marker)->release(); }
 
	AMarker *marker() { return _marker; }
};

class ADataVector : public AVector, public AObjectWithMarker {
 */

class ADataVector : public AVector, public ANotifierInterface {
protected:
	AMarker *_marker;
	char *_name;
public:
	ADataVector(AMarker *mark, vsize_t len, const char *name = NULL) : AVector(len), ANotifierInterface(false), _marker(mark) {
		_name = name ? strdup(name) : NULL;
		// FIXME: we have a problem here with retaining the marker - for now we require that markers must be retained separately from their users ..
		//if (_marker) AObject_retain(_marker);
		if (_marker) _marker->retain();
		OCLASS(ADataVector);
	};
	virtual ~ADataVector() {
		if (_name) AFree(_name);
		//if (_marker) AObject_release(_marker);
		if (_marker) _marker->release();
		DCLASS(ADataVector);
	}
	
	AMarker *marker() { return _marker; }
	const char *name() { return _name; }
	void setName(const char *newName) { if (_name) AFree(_name); _name = strdup(newName); }
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
		if (owned) AFree(_data);
		if (d_data) AFree(d_data);
		if (i_data) AFree(i_data);
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
			d_data = (double*) AAlloc(_len * sizeof(double));
			AMEM(d_data);
			for (int i=0; i<_len; i++) d_data[i] = (double)_data[i];
		}
		return d_data;
	}
	virtual const int *asInts() {
		if (!i_data) {
			i_data = (int*) AAlloc(_len * sizeof(int));
			AMEM(i_data);
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
	ADoubleVector(AMarker *m, double *data, vsize_t len, bool copy=true) : ADataVector(m, len), f_data(0), i_data(0) {
		_data = copy?(double*)memdup(data, len * sizeof(double)):data; OCLASS(ADoubleVector)
	}
	ADoubleVector(double *data, vsize_t len, bool copy=true) : ADataVector(0, len), f_data(0), i_data(0) {
		_data = copy?(double*)memdup(data, len * sizeof(double)):data; OCLASS(ADoubleVector)
	}
	virtual ~ADoubleVector() {
		if (owned) AFree(_data);
		if (f_data) AFree(f_data);
		if (i_data) AFree(i_data);
		DCLASS(ADoubleVector)
	}
	
	virtual const double *asDoubles() { return _data; }		
	
	virtual ADataRange range() {
		ADataRange r = AUndefDataRange;
		if (length()) {
			double e = r.begin = _data[0];
			for (int i = 0; i < length(); i++)
				if (AisNA(e)) e = r.begin = _data[i]; else
					if (!AisNA(_data[i])) { if(_data[i] < r.begin) r.begin = _data[i]; else if (_data[i] > e) e = _data[i]; }
			r.length = e - r.begin;
		}
		return r;
	}
	
	virtual const float *asFloats() {
		if (!f_data) {
			f_data = (float*) AAlloc(_len * sizeof(float));
			AMEM(f_data);
			for (vsize_t i = 0; i < _len; i++)
				f_data[i] = (float) _data[i];
		}
		return f_data;
	}
	
	virtual const int *asInts() {
		if (!i_data) {
			i_data = (int*) AAlloc(_len * sizeof(int));
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

class ATimeVector : public ADoubleVector {
protected:
	double _tzOffset;
public:
	ATimeVector(AMarker *m, double *data, vsize_t len, bool copy=true) : ADoubleVector(m, data, len, copy), _tzOffset(0.0) {
		OCLASS(ATimeVector)
	}
	ATimeVector(double *data, vsize_t len, bool copy=true) : ADoubleVector(data, len, copy), _tzOffset(0.0) {
		OCLASS(ATimeVector)
	}
	
	virtual bool isTime() { return true; }
	double tzOffset() { return _tzOffset; }
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
		if (owned) AFree(_data);
		if (d_data) AFree(d_data);
		if (f_data) AFree(f_data);
		DCLASS(AIntVector)
	}
	
	virtual const int *asInts() { return _data; }
	virtual const double *asDoubles() {
		if (!d_data) {
			d_data = (double*) AAlloc(_len * sizeof(double));
			AMEM(d_data);
			for (int i=0; i<_len; i++) d_data[i] = (double)_data[i];
		}
		return d_data;
	}
	virtual const float *asFloats() {
		if (!f_data) {
			f_data = (float*) AAlloc(_len * sizeof(float));
			AMEM(f_data);
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

class AFactorVector : public AIntVector {
protected:
	char **_names;
	int _levels;
	char **s_data;
	AUnivarTable *_tab;
	APermutation *perm;
public:
	AFactorVector(AMarker *mark, const int *data, int len, const char **names, int n_len, bool copy=true) : AIntVector(mark, data, len, copy), _levels(n_len), _tab(0), perm(NULL) {
		_names = (char**) (copy ? memdup(names, n_len * sizeof(char*)) : names); OCLASS(AFactorVector)
	}
	virtual ~AFactorVector() {
		if (owned) {
			for (int i = 0; i < _levels; i++) if (_names[i]) AFree(_names[i]);
			AFree(_names);
		}
		if (perm) perm->release();
		if (_tab) _tab->release();
		DCLASS(AFactorVector)
	}
	virtual const char **asStrings() {
		if (!s_data) {
			s_data = (char**) AAlloc(_len * sizeof(char*));
			AMEM(s_data);
			for (vsize_t i = 0; i < _len; i++)
				s_data[i] = (_data[i] < 0 || _data[i] >= _levels)?NULL:_names[_data[i]];
		}
		return (const char**) s_data;
	}
	
	virtual bool isFactor() { return true; }
	
	virtual APermutation *permutation() { return perm ? perm : (perm = new APermutation(_levels)); }
	
	// FIXME: we'll need to make it virtual in the super class ..
	virtual const char *stringAt(vsize_t i) {
		if (i >= _len) return NULL;
		int l = _data[i];
		return (l >= 0 && l < _levels) ?  _names[l] : NULL;
	}
	
	AUnivarTable *table() {
		if (!_tab) {
			_prof(profReport("^AFactorVector.table"))
			_tab = new AUnivarTable(_levels);
			for (vsize_t i = 0; i < _len; i++)
				_tab->add((vsize_t) _data[i]);
			if (_names) for (vsize_t i = 0; i < _levels; i++)
				_tab->setName(i, _names[i]);
			_prof(profReport("$AFactorVector.table"))
		}
		return _tab;
	}
	
	int levels() { return _levels; }
	const char **levelStrings() { return (const char**) _names; }
};

class APointVector : public ADataVector {
protected:
	APoint *_data;
public:
	APointVector(AMarker *m, const APoint *data, vsize_t len, bool copy) : ADataVector(m, len){
		_data = (APoint*)(copy?memdup(data, len * sizeof(APoint)):data); OCLASS(APointVector)
	}
	APointVector(const APoint *data, vsize_t len, bool copy) : ADataVector(0, len) {
		_data = (APoint*)(copy?memdup(data, len * sizeof(APoint)):data); OCLASS(APointVector)
	}
	APointVector(const APoint *data, vsize_t len) : ADataVector(0, len) {
		_data = (APoint*)memdup(data, len * sizeof(APoint)); OCLASS(APointVector)
	}
	
	virtual ~APointVector() {
		if (owned) AFree(_data);
		DCLASS(APointVector)
	}
	
	virtual const APoint* asPoints() { return _data; }
	
	bool isDataNull(){ 
		return _data == NULL;
	}
	
};




// TODO: mutable vectors ( + notification?)

#endif
