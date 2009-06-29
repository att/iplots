/*
 *  AAxis.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "AVisual.h"

class AAxis : public AVisual {
protected:
	AScale *_scale;
public:
	AAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AVisual(parent, frame, flags), _scale(scale) { scale->retain(); OCLASS(AAxis) }
	virtual ~AAxis() {
		if (_scale) _scale->release();
		DCLASS(AAxis)
	}
};

class AXAxis : public AAxis {
public:
	AXAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		//rect(_frame);
		//color(0.0, 0.0, 1.0, 0.3);
		rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x, _frame.y + _frame.height / 2, _frame.x, _frame.y + _frame.height);
		line(_frame.x + _frame.width, _frame.y + _frame.height / 2, _frame.x + _frame.width, _frame.y + _frame.height);
		
		// show outer labels
		char buf[64];
		ADataRange dr = _scale->dataRange();
		snprintf(buf, 64, "%g", dr.begin);
		text(AMkPoint(_frame.x, _frame.y + _frame.height / 2), buf, AMkPoint(0.0, 0.5));
		snprintf(buf, 64, "%g", dr.begin + dr.length);
		text(AMkPoint(_frame.x + _frame.width, _frame.y + _frame.height / 2), buf, AMkPoint(1.0, 0.5));
	}
};

class ANamedAxis {
protected:
	char **_name;
	vsize_t _names;
	
public:
	ANamedAxis() : _names(0), _name(0) { }
	
	virtual ~ANamedAxis() {
		if (_names)
			for (vsize_t i = 0; i < _names; i++)
				if (_name[i]) free(_name[i]);
		if (_name)
			free(_name);
	}
	
	/* it is allowed to use (NULL, n) to allocate names and then use setName */
	void setNames(const char **name, vsize_t names) {
		if (_name) free(_name);
		_name = (char **) calloc(names, sizeof(char*));
		if (name)
			for (vsize_t i = 0; i < names; i++)
				if (name[i]) _name[i] = strdup(name[i]);
		_names = names;
	}
	
	/* name can be NULL. currently it only allows modification of already allocated names - names beyond allocated space will be silently dropped. */
	void setName(vsize_t ix, const char *name) {
		if (ix >= _names) return;
		if (_name[ix]) free(_name[ix]);
		_name[ix] = name ? strdup(name) : 0;
	}
	
	const char *name(vsize_t ix) {
		return (ix >= _names) ? 0 : _name[ix];
	}
};

class ADiscreteXAxis : public AAxis, public ANamedAxis {
public:
	ADiscreteXAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }

	virtual void draw() {
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		rect(_frame);
		//color(0.0, 0.0, 1.0, 0.3);
		//rectO(_frame);

		ADataRange r = _scale->dataRange();
		vsize_t n = (vsize_t) r.length;
		APoint centerAdj = AMkPoint(0.5, 0.5);
		color(pointColor);
		AUnivarTable *tab = NULL;
		AVector *dv = _scale->data();
		if (dv && dv->isFactor())
			tab = ((AFactorVector*)dv)->table();
		for (vsize_t i = 0;i <= n; i++) {
			AFloat c = _scale->discreteCenter(i);
			line(c, _frame.y + _frame.height, c, _frame.y + _frame.height * 0.8);
			if (tab && tab->name(i))
				text(AMkPoint(c, _frame.y + _frame.height * 0.4), tab->name(i), centerAdj);
			else if (_names > i && _name[i])
				text(AMkPoint(c, _frame.y + _frame.height * 0.4), _name[i], centerAdj);
			else {
				char buf[32];
				snprintf(buf, 32, "%d", i);
				text(AMkPoint(c, _frame.y + _frame.height * 0.4), buf, centerAdj);
			}				
		}
		/*
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x, _frame.y + _frame.height / 2, _frame.x, _frame.y + _frame.height);
		line(_frame.x + _frame.width, _frame.y + _frame.height / 2, _frame.x + _frame.width, _frame.y + _frame.height);
		
		// show outer labels
		char buf[64];
		ADataRange dr = _scale->dataRange();
		snprintf(buf, 64, "%g", dr.begin);
		text(AMkPoint(_frame.x, _frame.y + _frame.height / 2), AMkPoint(0.0, 0.5), buf);
		snprintf(buf, 64, "%g", dr.begin + dr.length);
		text(AMkPoint(_frame.x + _frame.width, _frame.y + _frame.height / 2), AMkPoint(1.0, 0.5), buf);
		 */
	}
};


class AYAxis : public AAxis, public ANamedAxis {
public:
	AYAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { }
	
	virtual void draw() {
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		rect(_frame);
		//color(1.0, 0.0, 0.0, 0.3);
		//rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x + _frame.width, _frame.y, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x + _frame.width / 2, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x + _frame.width / 2, _frame.y, _frame.x + _frame.width, _frame.y);

		// show outer labels
		char buf[64];
		ADataRange dr = _scale->dataRange();
		snprintf(buf, 64, "%g", dr.begin);
		text(AMkPoint(_frame.x + _frame.width / 2, _frame.y), buf, AMkPoint(0.5, 1.0), -90.0);
		snprintf(buf, 64, "%g", dr.begin + dr.length);
		text(AMkPoint(_frame.x + _frame.width / 2, _frame.y + _frame.height), buf, AMkPoint(0.5, 0.0), -90.0);
	}
};
