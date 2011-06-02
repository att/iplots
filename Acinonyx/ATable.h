/*
 *  ATable.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 7/8/10.
 *  Copyright 2010 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_TABLE_H
#define A_TABLE_H

#include "AObject.h"

class AUnivarTable : public AObject {
protected:
	vsize_t *_counts;
	vsize_t _size;
	vsize_t _other;
	vsize_t _max;
	char **_names;
public:
	AUnivarTable(vsize_t size, bool named=true) : _size(size), _names(NULL), _other(0), _max(0) {
		_counts = (vsize_t*) AZAlloc(_size, sizeof(vsize_t));
		AMEM(_counts);
		OCLASS(AUnivarTable)
	}
	
	virtual ~AUnivarTable() {
		AFree(_counts);
		DCLASS(AUnivarTable);
	}
	
	virtual AUnivarTable *copy() {
		AUnivarTable *c = new AUnivarTable(_size, true); 
		memcpy(c->_counts, _counts, sizeof(vsize_t) * _size);
		c->_other = _other;
		if (_names)
			for (vsize_t i = 0; i < _size; i++)
				c->setName(i, _names[i]);
		return c;
	}
	
	vsize_t size() { return _size; }
	
	const vsize_t *counts() { return _counts; }
	
	vsize_t other() { return _other; }
	
	vsize_t maxCount() { return _max; }
	
	vsize_t count(vsize_t index) { return (index < _size) ? _counts[index] : 0; }
	
	char **names() { return _names; }
	
	char *name(vsize_t index) { return (_names && index < _size) ? _names[index] : 0; }
	
	void setName(vsize_t index, const char *name) {
		if (!_names)
			_names = (char**) AZAlloc(_size, sizeof(char*));
		AMEM(_names);
		if (index < _size) {
			if (_names[index] && !strcmp(name, _names[index])) return;
			if (_names[index]) AFree(_names[index]);
			_names[index] = strdup(name);
		}
	}
	
	void reset() { memset(_counts, 0, sizeof(vsize_t) * _size); _other = 0; _max = 0; }
	
	void add(vsize_t entry) {
		if (entry < _size) {
			vsize_t c = ++_counts[entry];
			if (c > _max) _max = c;
		} else _other++;
	}
};

#if 0

typedef struct cross_level {
	struct cross_level *next;
	AFactorVector *factor;
	vsize_t n;
	void *ptr;
} cross_level_t;

class ACrossTable : public Object {
protected:
	vsize_t n_factors;
	cross_level_t *levels;
	AFactorVector **factors;
public:
	ACrossTable(AObjectVector *factors) {
		(factorVars = factors)->retain();
		factors = (AFactorVector**) factors->asObjects();
		n_factors = factors->length();
		buildLevels();
		OCLASS(ACrossTable)
	}
	
	virtual void buildLevels() {};
	
	virtual vsize_t at(vsize_t i0, ...) {
		return 0;
	}
};

class ADenseCrossTable : public ACrossTable {
public:
	ADenseCrossTable(AObjectVector *factors) : ACrossTable(factors) {
	}

	virtual void buildLevels() {
		
	}	
};

#endif

#endif
