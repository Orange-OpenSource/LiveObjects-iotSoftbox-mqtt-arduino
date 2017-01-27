/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file timer_defs.h
 * @brief Timer struct definition.
 *
 */

#ifndef TIMER_DEFS_H_
#define TIMER_DEFS_H_

#include <Arduino.h>

#define LOC_TIMER_STRUCT        0

#if defined(__cplusplus)
 extern "C" {
#endif

#if LOC_TIMER_STRUCT
typedef struct {
	unsigned long  tv_sec;
	unsigned long  tv_usec;
} Timerval_t;
#else
typedef unsigned long Timerval_t;
#endif

struct Timer {
	Timerval_t  end_time;
};


#if defined(__cplusplus)
 }
#endif

#endif //TIMER_DEFS_H_
