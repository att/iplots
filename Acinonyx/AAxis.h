/*
 *  AAxis.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_AXIS_H_
#define A_AXIS_H_

#include "AVisual.h"
#include "AQuery.h"

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
	
	virtual void draw(vsize_t layer) {
		if (layer != LAYER_ROOT) return;
		color(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.8);
		//rect(_frame);
		//color(0.0, 0.0, 1.0, 0.3);
		rectO(_frame);
		color(0.0, 0.0, 0.0, 1.0);
		line(_frame.x, _frame.y + _frame.height, _frame.x + _frame.width, _frame.y + _frame.height);
		line(_frame.x, _frame.y + _frame.height / 2, _frame.x, _frame.y + _frame.height);
		line(_frame.x + _frame.width, _frame.y + _frame.height / 2, _frame.x + _frame.width, _frame.y + _frame.height);
		
		// show outer labels
		ADataRange dr = _scale->dataRange();
		text(AMkPoint(_frame.x, _frame.y + _frame.height / 2), _scale->stringForDoubleValue(dr.begin), AMkPoint(0.0, 0.5));
		text(AMkPoint(_frame.x + _frame.width, _frame.y + _frame.height / 2), _scale->stringForDoubleValue(dr.begin + dr.length), AMkPoint(1.0, 0.5));
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
		AMEM(_name);
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
	ADiscreteXAxis(AContainer *parent, ARect frame, int flags, AScale *scale) : AAxis(parent, frame, flags, scale) { OCLASS(ADiscreteXAxis) }
	virtual ~ADiscreteXAxis() { DCLASS(ADiscreteXAxis) }

	virtual void draw(vsize_t layer) {
		if (layer != LAYER_ROOT) return;
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
			ARange dr = _scale->discreteRange(i);
			AFloat c = dr.begin + (dr.length * 0.5);
			char *txt = 0, buf[32];
			line(c, _frame.y + _frame.height, c, _frame.y + _frame.height * 0.8);
			if (tab && tab->name(i))
				txt = tab->name(i);
			else if (_names > i && _name[i])
				txt = _name[i];
			else
				snprintf(txt = buf, 32, "%d", i);
			if (txt && dr.length > 10 && (strlen(txt) < 3 || dr.length > 40))
				text(AMkPoint(c, _frame.y + _frame.height * 0.4), txt, centerAdj);
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
	
	virtual void draw(vsize_t layer) {
		if (layer != LAYER_ROOT) return;
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
		text(AMkPoint(_frame.x + _frame.width / 2, _frame.y), buf, AMkPoint(1.0, 0.5), -90.0);
		snprintf(buf, 64, "%g", dr.begin + dr.length);
		text(AMkPoint(_frame.x + _frame.width / 2, _frame.y + _frame.height), buf, AMkPoint(0.0, 0.5), -90.0);
	}
};

#endif;