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


/* ================================================================================= */
/* Private Functions
 * -----------------
 */

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

