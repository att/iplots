/*
 *  AValue.h - object wrapper around scalar values
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#include "AObject.h"

typedef enum { AVnull, AVchar, AVint, AVbool, AVfloat, AVdouble, AVlong, AVstr, AVptr, AVobj } AVtype_t;

class AValue : public AObject {
public:
	union AValue_u {
		int b;
		char c;
		int i;
		float f;
		double d;
		long l;
		char *s;
		void *v;
		AObject *o;
	} v;
	AVtype_t _type;
		
	AValue(int value) { v.i = value; _type = AVint; OCLASS(AValue) }
	AValue(AObject *obj) { v.o = obj; _type = AVobj; obj->retain(); OCLASS(AValue) }
	AValue(const char *str, bool copy) { v.s = (char*)(copy?strdup(str):str); _type = AVstr; OCLASS(AValue) }
	AValue(const char *str) { v.s = strdup(str); _type = AVstr;	OCLASS(AValue) }
	AValue(float value) { v.f = value; _type = AVfloat; OCLASS(AValue) }
	AValue(double value) { v.d = value; _type = AVdouble; OCLASS(AValue) }
	AValue(bool value) { v.b = value; _type = AVbool; OCLASS(AValue) }
	~AValue() {
		switch (_type) {
			case AVobj: if (v.o) v.o->release(); break;
			case AVstr: if (v.s) free(v.s); break;
		}
	}
	
	// FIXME: to/from string conversions
	int asInt() {
		switch (_type)  {
			case AVint: return v.i;
			case AVdouble: return (int)v.d;
			case AVfloat: return (int)v.f;
			case AVstr: return v.s?atoi(v.s):NA_int;
		}
		return NA_int;
	}
	
	float asFloat() {
		switch (_type) {
			case AVfloat: return v.f;
			case AVdouble: return (float)v.f;
			case AVint: return (float)v.i;
		}
		return 0.0f;
	}
	
	AObject *asObject() {
		return (_type == AVobj)?v.o:this;
	}
};
