/**
 * @file timer.c
 * @brief Linux implementation of the timer interface.
 */

#include <Arduino.h>

#include <stddef.h>
#include <sys/types.h>

#include "paho-mqttclient-embedded-c/timer_interface.h"

#include "liveobjects-sys/LiveObjectsClient_Platform.h"
#include "liveobjects-sys/loc_trace.h"

//#define TIMER_DEBUG

typedef unsigned long  time_ms_t;

#if defined(ARDUINO_MEDIATEK)
#if 0
#include <vmdatetime.h>
#include <vmsock.h>
#define GET_TIME_MS()  ((time_ms_t)(vm_ust_get_current_time()/1000UL))
#else
#define GET_TIME_MS()   millis()
#endif
#else
#define GET_TIME_MS()   millis()
#endif /* ARDUINO_MEDIATEK */

#if LOC_TIMER_STRUCT
/*
 * sub time from timer
 */
void Ltimersub(const Timerval_t* tvp, const Timerval_t* uvp, Timerval_t* vvp) {
	if ((tvp->tv_sec < uvp->tv_sec) || ((tvp->tv_sec == uvp->tv_sec) && (tvp->tv_usec <= uvp->tv_usec))) {
		vvp->tv_sec = 0;
		vvp->tv_usec = 0;
		return;
	}
	vvp->tv_sec = tvp->tv_sec - uvp->tv_sec;
	if (tvp->tv_usec >= uvp->tv_usec) {
		vvp->tv_usec = tvp->tv_usec - uvp->tv_usec;
	}
	else {
		--vvp->tv_sec;
		vvp->tv_usec = (1000000 + tvp->tv_usec) - uvp->tv_usec;
	}
}

/*
 * add time to timer
 */
void Ltimeradd(const Timerval_t* tvp, const Timerval_t* uvp, Timerval_t* vvp) {
	vvp->tv_sec = tvp->tv_sec + uvp->tv_sec;
	vvp->tv_usec = tvp->tv_usec + uvp->tv_usec;
	if (vvp->tv_usec >= 1000000) {
		++vvp->tv_sec;
		vvp->tv_usec -= 1000000;
	}
}
#endif


/*
 * count timer and return true or false if the time ends up
 */
char TimerIsExpired(Timer* timer) {
	time_ms_t start;
#if !LOC_TIMER_STRUCT
	char expired;
	start = GET_TIME_MS();
	expired = ((timer->end_time) && (timer->end_time < start)) ? 1 : 0;
#ifdef TIMER_DEBUG
	if (expired) {
		LOTRACE_DBG1("T%p: TimerIsExpired  %lu < %lu",
				timer, timer->end_time,  start);
	}
#endif
	return expired;
#else
	Timerval_t now, res;
	start = GET_TIME_MS();
	now.tv_sec = start / 1000;
	now.tv_usec = (start % (now.tv_sec * 1000)) * 1000;
	Ltimersub(&timer->end_time, &now, &res);
#ifdef TIMER_DEBUG
	if (res.tv_sec == 0 && res.tv_usec <= 2000)
	{
		LOTRACE_DBG1("T%p: %s n=%d.%06d e=%d.%06d => %d.%06d", timer,
				(res.tv_sec == 0 && res.tv_usec <= 2000) ? "YES" : "",
				now.tv_sec, now.tv_usec, timer->end_time.tv_sec, timer->end_time.tv_usec,
				res.tv_sec, res.tv_usec);
	}
#endif
	return (res.tv_sec == 0 && res.tv_usec <= 2000);
#endif
}

//add time to the timer in microseconds level
void TimerCountdownMS(Timer* timer, unsigned int timeout) {
	time_ms_t start;
#if !LOC_TIMER_STRUCT
	start = GET_TIME_MS();
	timer->end_time = start + timeout;
#ifdef TIMER_DEBUG
	LOTRACE_DBG1("T%p: %u ms (%lu -> %lu)",
			timer, timeout, start,  timer->end_time);
#endif
#else
	Timerval_t now;
	Timerval_t interval;

	interval.tv_sec = timeout / 1000;
	interval.tv_usec = (timeout % 1000) * 1000;

	start = GET_TIME_MS();
	now.tv_sec = start / 1000;
	now.tv_usec = (start % 1000) * 1000;

	Ltimeradd(&now, &interval, &timer->end_time);

#ifdef TIMER_DEBUG
	LOTRACE_DBG1("T%p: %u ms => e=%lu.%06lu i=%lu.%06lu n=%lu.%06lu", timer, timeout,
			timer->end_time.tv_sec, timer->end_time.tv_usec,
			interval.tv_sec, interval.tv_usec,
			now.tv_sec, now.tv_usec);
#endif
#endif
}

//add time to the timer in seconds level
void TimerCountdown(Timer* timer, unsigned int timeout) {
	time_ms_t start;
#if !LOC_TIMER_STRUCT
	start = GET_TIME_MS();
	timer->end_time = start + timeout * 1000;
#ifdef TIMER_DEBUG
	LOTRACE_DBG1("T%p: %u s (%lu -> %lu)",
			timer, timeout, start,  timer->end_time);
#endif
#else
	Timerval_t now;
	start = GET_TIME_MS();
	now.tv_sec = start / 1000;
	now.tv_usec = (start % 1000) * 1000;
	Timerval_t interval = { timeout, 0 };

	Ltimeradd(&now, &interval, &timer->end_time);

	LOTRACE_DBG1("T%p: %u seconds => n=%lu.%06lu  e=%lu.%06lu", timer, timeout,
			now.tv_sec, now.tv_usec,
			timer->end_time.tv_sec, timer->end_time.tv_usec);
#endif
}

//calculate the left time in microseconds level
int TimerLeftMS(Timer* timer) {
	time_ms_t start;
#if !LOC_TIMER_STRUCT
	start = GET_TIME_MS();
	long left = timer->end_time - start;
#ifdef TIMER_DEBUG
	LOTRACE_DBG1("T%p: left %ld ms (%lu %lu)", timer, left, timer->end_time, start);
#endif
	return (left <= 0) ? 0 : left;
#else
	Timerval_t now, res;
	start = GET_TIME_MS();
	now.tv_sec = start / 1000;
	now.tv_usec = (start % 1000) * 1000;
	Ltimersub(&timer->end_time, &now, &res);

	LOTRACE_DBG1("T%p: left %lu.%lu => %u ms (n= %lu.%lu  e= %lu.%lu)", timer,
			res.tv_sec, res.tv_usec,
			(res.tv_sec * 1000) + (res.tv_usec / 1000),
			now.tv_sec, now.tv_usec,
			timer->end_time.tv_sec, timer->end_time.tv_usec);

	return (res.tv_sec * 1000) + (res.tv_usec / 1000);
#endif
}

void TimerInit(Timer* timer) {
#if !LOC_TIMER_STRUCT
	timer->end_time = 0;
#else
	timer->end_time = (Timerval_t) { 0, 0 };
#endif
}
