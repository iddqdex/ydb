/*
 * rdppm-8.c
 *
 * Copyright (C) 2025, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file is a wrapper for compiling rdppm.c to support 2 to 8 bits of
 * data precision.  rdppm.c should not be compiled directly.
 */

#define BITS_IN_JSAMPLE  8

#include "../rdppm.c"
