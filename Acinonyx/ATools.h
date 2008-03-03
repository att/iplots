/*
 *  ATools.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/2/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

#ifndef A_TOOLS_H
#define A_TOOLS_H

#include <stdlib.h>
#include <string.h>

#include "ATypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void *memdup(const void *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif /* A_TOOLS_H */