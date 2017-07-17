/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file  platform_default.h
 * @brief default platform definitions; only used for coding style
 */

#ifndef __platform_default_H_
#define __platform_default_H_

#if !defined(PRIu32) &&  defined(CODING_STYLE)

#include <inttypes.h>
#include <stdlib.h>

#ifndef PRIu32
#define PRIu32    "u"
#endif

#ifndef PRIi32
#define PRIi32    "d"
#endif

#ifndef PRIu64
#define PRIu64    "lu"
#endif

#ifndef PRIi64
#define PRIi64    "ld"
#endif

#define MEM_ALLOC(len)        ((char*) malloc(len))

#define MEM_FREE(p)            free((void*)(p))

#define WAIT_MS(dt_ms)         usleep(dt_ms*1000)
#endif

#ifdef __cplusplus
}
#endif
#endif /* __platform_default_H_ */
