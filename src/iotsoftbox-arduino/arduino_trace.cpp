/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file arduino_trace.cpp
 * @brief Trace/Log Interface.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include <string.h>

#include <Arduino.h>

#include "liveobjects-sys/LiveObjectsClient_Platform.h"
#include "liveobjects-sys/loc_trace.h"

#define LOGP_STACK_CHECK
#define LOGP_MSG_TAG  0x5A

#define TRACE_LEVELS_MAX      7
#define LOGP_MAX_MSG_SIZE     250
#define LOGP_TAIL_MSG_SIZE    5

static uint8_t _trace_console = 1;
static uint32_t _trace_max_msg_size = 0;
static uint32_t _trace_index = 0;
static int _trace_level_current = 4;
static char _trace_str[LOGP_MAX_MSG_SIZE + LOGP_TAIL_MSG_SIZE];

static const char _trace_TraceLib[TRACE_LEVELS_MAX + 2] = "-EWNIDdV";

extern "C" void lo_trace_init(int level) {
	_trace_level_current = level;
}

extern "C" void lo_trace_level(int level) {
	_trace_level_current = level;
}

extern "C" void lo_trace_log(int level,
#if defined(ARDUINO_MEDIATEK) || defined(ARDUINO_ARCH_SAMD)
		const char *file,
		unsigned int line,
		const char *function,
#endif
		const char* format, ...) {
	if (level > 0) {
		va_list ap;
		va_start(ap, format);
		if (level > TRACE_LEVELS_MAX)
			level = TRACE_LEVELS_MAX;
		if (level <= _trace_level_current) {
#if 0
			struct tm timestamp;
#endif
			char *pt_str = _trace_str;
			char *end_str = _trace_str + LOGP_MAX_MSG_SIZE;

#ifdef LOGP_STACK_CHECK
			char *end_buf = pt_str + sizeof(_trace_str);
			char* pc = end_str;
			while (pc < end_buf)
				*pc++ = LOGP_MSG_TAG;
#endif
			*end_str = LOGP_MSG_TAG;

			pt_str += snprintf(pt_str, end_str - pt_str, "%" PRIu32, ++_trace_index);

#if 0
			if (pt_str < end_str) {
				/*Get timestamp*/
				tb_getTime(&timestamp);

				/*Convert timestamp into date string in YearDay-HH:MM:SS format*/
				pt_str += tb_convertTimeToDateString(&timestamp, pt_str, end_str-pt_str);
			}
#endif

#if 1
			/*Add  milliseconds since the program started */
			if (pt_str < end_str) {
				unsigned long ms = millis();
				pt_str += snprintf(pt_str, end_str - pt_str, ":%lu", ms);
			}
#endif
#if 0
			/* And add Task Id (tid)*/
			if (pt_str < end_str) {
				pt_str += snprintf(pt_str, end_str-pt_str, ":t%02d", os_tsk_self());
			}
#endif

			/*Add debug level to log*/
			if (pt_str < end_str)
				pt_str += snprintf(pt_str, end_str - pt_str, ":%c:", *(_trace_TraceLib + level));

#if defined(ARDUINO_MEDIATEK) || defined(ARDUINO_ARCH_SAMD)
			/*Add file:line: */
			if (pt_str < end_str) {
				const char* name = strrchr(file, '\\');
				if ((name) && (*name == '\\')) name++;
				else {
					name = strrchr(file, '/');
					if ((name) && (*name == '/')) name++;
					else name = file;
				}
				pt_str += snprintf(pt_str, end_str-pt_str, "%s:%u:%s:", name, line, function);
			}
#endif

			if (pt_str < end_str) {
				// add user data
				pt_str += vsnprintf(pt_str, end_str - pt_str, format, ap);
				{ // Store the size of the biggest trace
					uint32_t msg_size = pt_str - _trace_str;
					if (_trace_max_msg_size < msg_size)
						_trace_max_msg_size = msg_size;
				}
			}

			if (pt_str >= end_str) {
				// Truncated message
				pt_str = end_str - 1;
#ifdef LOGP_STACK_CHECK
				if (*pt_str) {
					; //LogBug(0);
				}
				pc = end_str;
				while (pc < end_buf) {
					if (*pc != LOGP_MSG_TAG) {
						; //LogBug(1);
					}
					pc++;
				}
#endif
				*pt_str = 0;
			}

			if (*end_str != LOGP_MSG_TAG) {
#if 0
				fprintf(stderr,"\nLogRecord: Corrupted stack end_ptr=%p 0x%X != 0x%X", end_str, *end_str, LOGP_MSG_TAG);
#endif
				//CATCH_SOFT_EXCEPTION(EXCEPTION_CODE_LOG_OVERLOAD); // Don't LOG Macro because it calls  LOG_... function !!
				return;
			}

			if (*(pt_str - 1) != '\n') {
				*pt_str++ = '\n';
				*pt_str = 0;
			}
			else if ((*(pt_str) != 0))
				*pt_str = 0;

			if (_trace_console) {
				Serial.print(_trace_str);
			}
		}
		va_end(ap);
	}
}

extern "C" void lo_trace_printf(const char* format, ...) {
	va_list ap;
	va_start(ap, format);

	if (_trace_console) {
		char *pt_str = _trace_str;
		char *end_str = _trace_str + LOGP_MAX_MSG_SIZE;

		*end_str = LOGP_MSG_TAG;

		// add user data
		pt_str += vsnprintf(pt_str, end_str - pt_str, format, ap);
		{ // Store the size of the biggest trace
			uint32_t msg_size = pt_str - _trace_str;
			if (_trace_max_msg_size < msg_size)
				_trace_max_msg_size = msg_size;
		}

		if (pt_str >= end_str) {
			// Truncated message
			pt_str = end_str - 1;
			*pt_str = 0;
		}

		if (*end_str != LOGP_MSG_TAG) {
#if 0
			fprintf(stderr,"\nLogRecord: Corrupted stack end_ptr=%p 0x%X != 0x%X", end_str, *end_str, LOGP_MSG_TAG);
#endif
			//CATCH_SOFT_EXCEPTION(EXCEPTION_CODE_LOG_OVERLOAD); // Don't LOG Macro because it calls  LOG_... function !!
			return;
		}

		Serial.print(_trace_str);
	}

	va_end(ap);
}
