/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
  * @file   loc_trace.h
  * @brief  Plug LiveObjects traces for arduino system
  */

#ifndef __loc_trace_H_
#define __loc_trace_H_

#include "liveobjects-sys/LiveObjectsClient_Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOTRACE_INIT(level)           lo_trace_init(level)

#define LOTRACE_LEVEL(level)          lo_trace_level(level)



#if defined(ARDUINO_MEDIATEK) || defined(ARDUINO_ARCH_SAMD)

#define LOTRACE_ERR_I(...)            lo_trace_log(1, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#if defined(ARDUINO_MEDIATEK)
#define LOTRACE_ERR(...)              lo_trace_log(1, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOTRACE_WARN(...)             lo_trace_log(2, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOTRACE_NOTICE(...)           lo_trace_log(3, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOTRACE_INF(...)              lo_trace_log(4, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOTRACE_DBG1(...)             lo_trace_log(5, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOTRACE_DBG2(...)             lo_trace_log(6, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOTRACE_ERR(...)              ((void)0)
#define LOTRACE_WARN(...)             ((void)0)
#define LOTRACE_NOTICE(...)           ((void)0)
#define LOTRACE_INF(...)              ((void)0)
#define LOTRACE_DBG1(...)             ((void)0)
#define LOTRACE_DBG2(...)             ((void)0)
#endif

#define LOTRACE_DBG_VERBOSE(...)      ((void)0)
//#define LOTRACE_DBG_VERBOSE(...)      lo_trace_log(7, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define LOTRACE_PRINTF                lo_trace_printf

#else

#define LOTRACE_ERR_I(...)            lo_trace_log(1, ##__VA_ARGS__)
#define LOTRACE_ERR(...)              ((void)0)
#define LOTRACE_WARN(...)             ((void)0)
#define LOTRACE_NOTICE(...)           ((void)0)
#define LOTRACE_INF(...)              ((void)0)
#define LOTRACE_DBG1(...)             ((void)0)
#define LOTRACE_DBG2(...)             ((void)0)
#define LOTRACE_DBG_VERBOSE(...)      ((void)0)

#define LOTRACE_PRINTF(...)           ((void)0)
//#define LOTRACE_PRINTF                lo_trace_printf

#endif

void lo_trace_init( int level );

void lo_trace_level( int level );

void lo_trace_log( int level,
#if defined(ARDUINO_MEDIATEK) || defined(ARDUINO_ARCH_SAMD)
    const char   *file,
    unsigned int line,
    const char   *function,
#endif
    char const   *format, ... );

void lo_trace_printf( char const   *format, ... );

#ifdef __cplusplus
}
#endif

#endif /* __loc_trace_H_ */
