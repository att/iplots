/*
 *  ALinearProjection.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/24/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#include "AVector.h"

typedef double coeff_t;

// currently we're basing this on ADoubleVector because that's what we use for the final result, but is could as well be AFloatVector ...
class ALinearProjection : public ADoubleVector {
protected:
	vsize_t n_coeff;
	coeff_t *coeff;
	AObjectVector *vars;
	
public:
	ALinearProjection(AMarker *m, AObjectVector *variables) : ADoubleVector(m, NULL, 0, false), n_coeff(0), coeff(0) {
		// FIXME: we should really make a copy of the vars vector since it might change while we use it
		vars = variables;
		if (vars) {
			vars->retain();
			n_coeff = vars->length();
			if (n_coeff) {
				coeff = (coeff_t*) calloc(sizeof(coeff_t), n_coeff);
				AVector *v = (AVector*) vars->objectAt(0);
				if (v) {
					_len = v->length();
					_data = (double*) malloc(sizeof(double) * _len);
				}
			}
		}
		project();
		OCLASS(ALinearProjection)
	}
	
	virtual ~ALinearProjection() {
		if (coeff) free(coeff);
		if (vars) vars->release();
		DCLASS(ALinearProjection)
	}
	
	void project() {
		if (!n_coeff || !_len) return;
		memset(_data, 0, _len * sizeof(double)); // reset all to 0
		for(vsize_t i = 0; i < n_coeff; i++) { // each variable at a time add it to the _data
			AVector *v = (AVector*) vars->objectAt(i);
			if (v) {
				vsize_t n = _len;
				if (v->length() < n) n = v->length();
				const double *d = v->asDoubles();
				if (d)
					for (vsize_t j = 0; j < n; j++)
						_data[j] += coeff[i] * d[j];
			}
		}
	}
	
	void setCoefficient(vsize_t i, coeff_t value, bool update=true) {
		if (i >= n_coeff) return;
		coeff[i] = value;
		if (update) project();
	}
	
	void setCoefficients(coeff_t *c, bool update=true) {
		memcpy(coeff, c, sizeof(coeff_t) * n_coeff);
		if (update) project();
	}
	
	vsize_t nCoefficients() {
		return n_coeff;
	}
	
	ADataVector *variable(vsize_t i) {
		return (ADataVector*) vars->objectAt(i);
	}
};
