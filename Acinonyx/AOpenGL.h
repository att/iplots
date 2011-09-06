/*
 *  AOpenGL.h
 *  Acinonyx
 *  This file is used to load OpenGL API in a centralized manner
 *
 *  Created by Simon Urbanek on 9/1/11.
 *  Copyright 2011 Simon Urbanek. All rights reserved.
 *
 *  IMPORTANT: due to clashes is some headers files (w64-mingw) with R this
 *             file must be included *before* any R header files!
 *
 */

#ifndef A_OPEN_GL
#define A_OPEN_GL

#if ( defined __APPLE__ ) && (! defined USE_X11 ) /* ACocoaWindow */
#include <OpenGL/gl.h>
#include <OpenGL/glu.h> /* for polygon tessellation */
#else /* this includes X11 on OS X */ 
#include <GL/gl.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include <GL/glu.h>

/* non-exact textures using AFreeType are currently broken so disable them for now */
#ifdef GL_TEXTURE_RECTANGLE_EXT
#undef GL_TEXTURE_RECTANGLE_EXT
#endif
#ifdef GL_TEXTURE_RECTANGLE_ARB
#undef GL_TEXTURE_RECTANGLE_ARB
#endif

#endif /* X11 or Win32 */

/* #define TEXTURE_RECTANGLE_ARB 0x84F5  (name "GL_ARB_texture_rectangle") -- it is equivalent to GL_TEXTURE_RECTANGLE_EXT (name "GL_EXT_texture_rectangle") on OS X */

#ifdef GL_TEXTURE_RECTANGLE_EXT
#define A_TEXTURE_TYPE GL_TEXTURE_RECTANGLE_EXT
#define A_EXACT_TEXTURE 1
#elif defined GL_TEXTURE_RECTANGLE_ARB
#define A_TEXTURE_TYPE GL_TEXTURE_RECTANGLE_ARB
#define A_EXACT_TEXTURE 1
#else
#define A_TEXTURE_TYPE GL_TEXTURE_2D
#define A_EXACT_TEXTURE 0
#endif

#endif
