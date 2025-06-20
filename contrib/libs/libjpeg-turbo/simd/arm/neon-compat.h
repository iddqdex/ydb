/*
 * Copyright (C) 2020, 2024, D. R. Commander.  All Rights Reserved.
 * Copyright (C) 2020-2021, Arm Limited.  All Rights Reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

// #cmakedefine HAVE_VLD1_S16_X3
// #cmakedefine HAVE_VLD1_U16_X2
// #cmakedefine HAVE_VLD1Q_U8_X4

/* Define compiler-independent count-leading-zeros and byte-swap macros */
#if defined(_MSC_VER) && !defined(__clang__)
#define BUILTIN_CLZ(x)  _CountLeadingZeros(x)
#define BUILTIN_CLZLL(x)  _CountLeadingZeros64(x)
#define BUILTIN_BSWAP64(x)  _byteswap_uint64(x)
#elif defined(__clang__) || defined(__GNUC__)
#define BUILTIN_CLZ(x)  __builtin_clz(x)
#define BUILTIN_CLZLL(x)  __builtin_clzll(x)
#define BUILTIN_BSWAP64(x)  __builtin_bswap64(x)
#else
#error "Unknown compiler"
#endif

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#pragma clang diagnostic ignored "-Wc99-extensions"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
