/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file  arduino_sys.c
 * @brief System Interface : Mutex , Thread, ..
 */

#include "iotsoftbox-core/loc_sys.h"

#include "liveobjects-client/LiveObjectsClient_Config.h"
#include "liveobjects-client/LiveObjectsClient_Core.h"

#include "liveobjects-sys/LiveObjectsClient_Platform.h"
#include "liveobjects-sys/loc_trace.h"

#if defined(ARDUINO_ARCH_SAMD)
#include <stdlib.h>
#include <malloc.h>
#endif

/* ================================================================================= */
/* Private Functions
 * -----------------
 */
#if defined(ARDUINO_ARCH_SAMD)
static size_t   _lo_sys_mem_info_arena;
static size_t   _lo_sys_mem_info_uordblks;
static size_t   _lo_sys_mem_max_block_size;
/* --------------------------------------------------------------------------------- */
/*  */
static void _LO_sys_mem_check(void)
{
	struct mallinfo m = mallinfo();
	if ((m.arena) && (m.arena > _lo_sys_mem_info_arena)) {
		LOTRACE_WARN("HEAP -- total:   %u (+%u)", m.arena, m.arena-_lo_sys_mem_info_arena);
		_lo_sys_mem_info_arena = m.arena;
	}
	if ((m.uordblks) && (m.uordblks > _lo_sys_mem_info_uordblks)) {
		LOTRACE_WARN("HEAP -- in-used: %u / %u (+%u)",
				m.uordblks, m.arena, m.uordblks-_lo_sys_mem_info_uordblks);
		_lo_sys_mem_info_uordblks = m.uordblks;
	}
}
#endif

/* --------------------------------------------------------------------------------- */
/*  */
static void _LO_sys_threadExec(void const *argument) {
	LOTRACE_DBG1("THREAD %p...", argument);

	LiveObjectsClient_Run((LiveObjectsD_CallbackState_t) argument);

	LOTRACE_WARN("THREAD EXIT");
}



/* ================================================================================= */
/* Public Functions
 * ----------------
 */

/* --------------------------------------------------------------------------------- */
/*  Initialization */
void LO_sys_init(void) {
	LOTRACE_ERR_I("LO_sys_init");
#if defined(ARDUINO_ARCH_SAMD)
	_lo_sys_mem_info_arena = 0;
	_lo_sys_mem_info_uordblks = 0;
	_lo_sys_mem_max_block_size = 0;
#endif
}

/* ================================================================================= */
/* MUTEX
 * -----
 */
uint8_t LO_sys_mutex_lock(uint8_t idx) {
	LOTRACE_DBG_VERBOSE("LO_sys_mutex_lock(%d)", idx);
	return 0;
}

void LO_sys_mutex_unlock(uint8_t idx) {
	LOTRACE_DBG_VERBOSE("LO_sys_mutex_unlock(%d)", idx);
}

/* ================================================================================= */
/* THREAD
 * ------
 */

/* --------------------------------------------------------------------------------- */
/*  */
void LO_sys_threadRun(void) {
	LOTRACE_DBG1("LO_sys_threadRun()");
}

/* --------------------------------------------------------------------------------- */
/*  */
uint8_t LO_sys_threadIsLiveObjectsClient(void) {
	/* There is only one thread. So it is always the LiveObjects thread ! */
	LOTRACE_DBG1("LO_sys_threadIsLiveObjectsClient()");
	return 1;
}

/* --------------------------------------------------------------------------------- */
/*  */
int LO_sys_threadStart(void const *argument) {
	LOTRACE_DBG1("LO_sys_threadStart()");
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
void LO_sys_threadCheck(void) {
	LOTRACE_DBG1("LO_sys_threadCheck()");
}

#if defined(ARDUINO_ARCH_SAMD)

void LO_sys_mem_info(void)
{
	struct mallinfo m = mallinfo();
	if (m.arena)    LOTRACE_ERR_I("total:     %u",m.arena);
	if (m.ordblks)  LOTRACE_ERR_I("ordblks:   %u",m.ordblks);
	if (m.hblkhd)   LOTRACE_ERR_I("hblks:     %u",m.hblks);
	if (m.hblks)    LOTRACE_ERR_I("hblkhd:    %u",m.hblkhd);
	if (m.uordblks) LOTRACE_ERR_I("in-used:   %u",m.uordblks);
	if (m.fordblks) LOTRACE_ERR_I("free:      %u",m.fordblks);
	if (m.fordblks && m.uordblks)
		LOTRACE_ERR_I("t:         %u (%u)",
				m.uordblks+m.fordblks, m.arena-(m.uordblks+m.fordblks));
	if (m.keepcost) LOTRACE_ERR_I("keepcost   %u",m.keepcost);
}

void  LO_sys_mem_free(void * ptr)
{
	LOTRACE_DBG1("LO_sys_mem_free(%p) len=%u", ptr, malloc_usable_size (ptr));
	free(ptr);
	_LO_sys_mem_check();
}

void* LO_sys_mem_calloc(size_t n, size_t l)
{
	void* ptr;
	if ((n*l) > _lo_sys_mem_max_block_size ) {
		LOTRACE_ERR_I("HEAP -- max_block_size = %u (%u)", (n*l), _lo_sys_mem_max_block_size);
		_lo_sys_mem_max_block_size = (n*l);
	}
	ptr = calloc(n,l);
	if (ptr == NULL) {
		LOTRACE_ERR_I("LO_sys_mem_calloc(%u,%u) len=%u => ERROR", n, l, n*l);
	}
	else  LOTRACE_DBG1("LO_sys_mem_calloc(%u,%u) len=%u => %p", n, l, n*l, ptr);
	_LO_sys_mem_check();
	return ptr;
}
#endif
