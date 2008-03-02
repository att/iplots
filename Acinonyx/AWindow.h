/*
 *  AWindow.h - abtract Acinonyx window class
 *  Acinonyx
 *
 *  Created by Simon Urbanek
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 *  lang: C++
 */

#ifndef A_WINDOW_H
#define A_WINDOW_H

#include "ATypes.h"

class AWindow
{
public:
	AWindow(ARect frame) {};
	
	virtual bool canClose() { return true; }
	virtual void close() {};

	virtual void setVisible(bool flag) {};
	virtual bool visible() { return false; }
};

#endif
