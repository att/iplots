/*
 *  AColorMap.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 6/24/10.
 *  Copyright 2010 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_COLOR_MAP_H__
#define A_COLOR_MAP_H__

#include "AObject.h"
#include <math.h>

typedef int cmindex_t;

#define NilColor AMkColor(1.0, 1.0, 1.0, 0.0)
#define IsNilColor(X) AColorsAreEqual(X, NilColor)

#define displayGamma 2.2

class AColorMap : public AObject {
public:
	AColorMap() { OCLASS(AColorMap) }
	
	virtual AColor color(cmindex_t which) {
		return NilColor;
	}
};

/* although the separation is noble, we may combine both for speed [color() is virtual] unless we come up with another color map implementation */

class AIndexColorMap : public AColorMap {
protected:
	AColor *colors_;
	vsize_t n_colors_;

public:
	/* if colors_ is NULL and n_colors is not the vector is allocated */
	AIndexColorMap(AColor *colors, vsize_t n_colors, bool copy=true) : colors_(0), n_colors_(0) {
		colors_ = (colors && n_colors && copy) ? (AColor*) memdup(colors, sizeof(AColor) * n_colors) : 0;
		if (!colors_ && n_colors)
			colors_ = (AColor*) calloc(sizeof(AColor), n_colors);
		if (colors_) n_colors_ = n_colors;
		OCLASS(AIndexColorMap)
	}
	
	virtual ~AIndexColorMap() {
		if (colors_)
			free(colors_);
		DCLASS(AIndexColorMap)
	}

	virtual AColor color(cmindex_t which) {
		if (which >= n_colors_ || which < 0)
			return NilColor;
		return colors_[which];
	}
	
	const AColor *colors() {
		return colors_;
	}
	
	vsize_t count() {
		return n_colors_;
	}
	
	AColor *mutableColors() {
		return colors_;
	}
	
	/* it's currently a no-op but we may used it if we decide to lock on mutation */
	void unlock() {
	}
};

#define COL_BASIC 0
#define COL_CB1   16
#define COL_HCL   32
#define COL_NHCL  128 /* number of HCL colors */

static const unsigned char init_default_colors[] = {
	/* fully saturated basic colors; first color is the default non-brushed color */
	0,0,0, 0,0,0, 255,0,0, 0,205,0, 0,0,255, 0,255,255, 255,0,255, 255,255,0, 190,190,190, 128,0,0, 0,128,0, 0,0,128, 0,128,128, 128,0,128, 128,128,0,
	/* CBset1 9 class */
	0xef,0x76,0x77, 0x87,0xb2,0xd4, 0xff,0xb2,0x66, 0xc1,0x95,0xc8, 0x94,0xcf,0x92, 0xaf,0xaf,0x45, 0xca,0x9a,0x7e, 0xfa,0xb3,0xd9, 0xc2,0xc2,0xc2,
	/* random darker extensions  -FIXME*/
	0xcf,0x56,0x57, 0x67,0xa2,0xb4, 0xcf,0xa2,0x40, 0xa1,0x75,0xa8, 0x74,0xaf,0x72, 0xd0,0xd0,0x55, 0xaa,0x7a,0x55 };

class ADefaultColorMap : public AIndexColorMap {
public:
	ADefaultColorMap() : AIndexColorMap(0, 1024) {
		for (vsize_t i = 0; i < sizeof(init_default_colors); i += 3)
			colors_[i / 3] = AMkColor(((AFloat)init_default_colors[i])/255.0,
						  ((AFloat)init_default_colors[i + 1])/255.0,
						  ((AFloat)init_default_colors[i + 2])/255.0, 1.0);
		setHCLcolors(COL_HCL, COL_NHCL, 55.0, 75.0);
	}
	
	void setHCLcolors(vsize_t first, vsize_t n, AFloat chroma, AFloat luminance) {
		for (int i = 0; i < n; i++)
			colors_[i + first] = getHCLcolor(((AFloat) i) * 360.0/((AFloat) n), chroma, luminance);
	}

	/** adjusts RGB value according to the specified display gamma setting */
	static double gammaAdjust(double u) {
		return (u > 0.00304) ? 1.055 * pow(u,(1 / displayGamma)) - 0.055 : 12.92 * u;
	}
	
	/** transforms color defined in HCL space into RGB color
	 @param hue - hue (in degrees, between 0.0 and 360.0). basic colors are at angles 0, 120, 240
	 @param chroma - colorfullness of the color - unlike the saturation, chroma is an absolute value (default=35)
	 @param luminance - brightness of the color relative to while (white=100; default=85)
	 @return color object in RGB representation suitable for use in graphics */
	static AColor getHCLcolor(AFloat hue, AFloat chroma, AFloat luminance) {
		//function(hue, chroma = 35, luminance = 85, correct = FALSE, gamma = 2.2)
		//  Assume a D65 whitepoint with luminance 100.
		//  Ultimately, this should be a parameter.
		//  These are the CIE XYZ values.
		
		double XN =  95.047;
		double YN = 100.000;
		double ZN = 108.883;
		
		//  uN and vN are the corresponding LUV chromaticities
		
		double tmp = XN + YN + ZN;
		double xN = XN / tmp;
		double yN = YN / tmp;
		double uN = 2 * xN /(6 * yN - xN + 1.5);
		double vN = 4.5 * yN / (6 * yN - xN + 1.5);
		
		//  Convert from polar coordinates to u and v.
		//  Hue is take to be in degrees and needs to be converted.
		
		double U = chroma * cos(0.01745329251994329576 * hue);
		double V = chroma * sin(0.01745329251994329576 * hue);
		
		// Convert from L*u*v* to CIE-XYZ
		
		double Y = YN * ((luminance > 7.999592) ? pow((luminance + 16)/116,3) : luminance/903.3);
		double u = U / (13 * luminance) + uN;
		double v = V / (13 * luminance) + vN;
		double X = 9.0 * Y * u / (4 * v);
		double Z = - X / 3 - 5 * Y + 3 * Y / v;
		
		//  Map to ``gamma dependent'' RGB
		
		AFloat r = (AFloat) (gammaAdjust(( 3.240479 * X - 1.537150 * Y - 0.498535 * Z) / YN));
		AFloat g = (AFloat) (gammaAdjust((-0.969256 * X + 1.875992 * Y + 0.041556 * Z) / YN));
		AFloat b = (AFloat) (gammaAdjust(( 0.055648 * X - 0.204043 * Y + 1.057311 * Z) / YN));
		
		if (r < 0.0) r = 0.0; if (r > 1.0) r = 1.0;
		if (g < 0.0) g = 0.0; if (g > 1.0) g = 1.0;
		if (b < 0.0) b = 0.0; if (b > 1.0) b = 1.0;
		return AMkColor(r, g, b, 1.0);
	}
};

#endif
