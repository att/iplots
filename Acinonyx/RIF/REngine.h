/*
 *  REngine.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 5/5/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef A_RENGINE_H
#define A_RENGINE_H

/* for getenv/setenv */
#include <stdlib.h>

/* must come before R due to clash between GL in mingw-w64 and R */
#include "ARenderer.h"

#ifdef __i386__
static const char *rarch = "/i386";
#elif defined __x86_64__
static const char *rarch = "/x86_64";
#elif defined __ppc__
static const char *rarch = "/ppc";
#elif defined __ppc64__
static const char *rarch = "/ppc64";
#elif defined __arm__
static const char *rarch = "/arm";
#else
static const char *rarch = 0;
#endif

#include "RObject.h"
#include <Rversion.h>
#include <Rembedded.h>
#include <R_ext/Parse.h>

#if R_VERSION < R_Version(2,5,0)
#define RS_ParseVector R_ParseVector
#else
#define RS_ParseVector(A,B,C) R_ParseVector(A,B,C,R_NilValue)
#endif


class REngine : public AObject {
	static REngine *_main;
	
public:
	REngine(bool init_new=true) {
		if (init_new) {
			const char *argv[] = { "R", "--no-save", "--vanilla", 0 };
			if (!getenv("R_HOME")) {
			// FIXME: we use crude guesses for now
#if __APPLE__
				setenv("R_HOME", "/Library/Frameworks/R.framework/Resources", 1);
				if (rarch && !getenv("R_ARCH")) setenv("R_ARCH", rarch, 1);
#else
#ifndef WIN32
				setenv("R_HOME", "/usr/local/lib/R", 1);
#endif
#endif
			}
			int stat = Rf_initialize_R(3, (char**) argv);
			if (stat < 0)
				fprintf(stderr, "ERROR: cannot start R: %d\n", stat);
			setup_Rmainloop();
		}
		if (!_main) _main = this;
	}
	
	virtual ~REngine() {
	}
	
	static REngine* mainEngine() {
		return _main ? _main : new REngine();
	}
	
	RObject *parse(const char *text, int count, int *status) {
		ParseStatus ps;
		SEXP p = RS_ParseVector(Rf_mkString(text), count, &ps);
		if (status) status[0] = (int) ps;
		if (p == R_NilValue) return NULL;
		return new RObject(p);
	}
	
	RObject *eval(RObject *obj, int *status) {
		if (!obj) return NULL;
		SEXP res = NULL;
		int er = 0;
		SEXP exp = obj->value();
		if (TYPEOF(exp) == EXPRSXP) { // vector - evaluate all
			unsigned int i = 0;
			while (i < LENGTH(exp)) {
				er = 0;
				res = R_tryEval(VECTOR_ELT(exp, i), R_GlobalEnv, &er);
				if (er != 0) break;
				i++;
			}
		} else
			res = R_tryEval(exp, R_GlobalEnv, &er);
		if (status)
			status[0] = er;
		return res ? new RObject(res) : NULL;
	}
	
	RObject *parseAndEval(const char *text) {
		RObject *o = parse(text, 1, NULL);
		if (o) {
			RObject *r = eval(o, NULL);
			o->release();
			return r; 
		}
		return NULL;
	}
};

#endif
