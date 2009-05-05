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
#include <Rembedded.h>
public class REngine : public AObject {
	static REngine *_main;
	
public:
	REngine() {
		const char *argv[] = { "R", "--no-save", "--vanilla", 0 };
		if (!getenv("R_HOME")) {
			// FIXME: we use crude guesses for now
#if __APPLE__
			setenv("R_HOME","/Library/Frameworks/R.framework/Resources",1);
			if (rarch && !getenv("R_ARCH")) setenv("R_ARCH", rarch);
#else
			setenv("R_HOME","/usr/local/lib/R");
#endif
		}
		int stat = Rf_initialize_R(3, argv);
		setup_Rmainloop();
		if (!_main) _main = this;
	}
	
	virtual ~REngine() {
	}
	
	static REngine* mainEngine() {
		return _main ? _main : new Rengine();
	}
}

#endif
