/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file  LiveObjectsClient_Platform.h
 * @brief Specific definitions for LiveObjects Client library
 */

#ifndef __LiveObjectsClient_Paltform_H_
#define __LiveObjectsClient_Paltform_H_

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

#ifndef LOC_MQT_DUMP_STATIC_BUFFER_SIZE
// see LOGP_MAX_MSG_SIZE defined in arduino_trace.cpp
#define LOC_MQT_DUMP_STATIC_BUFFER_SIZE    ((250/3)-3)
#endif

#define MEM_ALLOC(len)        ((char*) malloc(len))

#define MEM_FREE(p)            free((void*)(p))

#define WAIT_MS(dt_ms)         delay(dt_ms)

#if defined(ARDUINO_ARCH_MTK) || defined(ARDUINO_MTK_ONE)
#define ARDUINO_MEDIATEK   1
#define ARDUINO_BOARD      "MDK"
#elif defined(ARDUINO_ARCH_AVR)  // ARDUINO_AVR_MEGA2560 , ARDUINO_AVR_ADK
#define ARDUINO_DTOSTRE    1
#if defined(ARDUINO_AVR_MEGA2560)
#define ARDUINO_BOARD      "MEGA"
#elif defined(ARDUINO_AVR_ADK)
#define ARDUINO_BOARD      "ADK"
#else
#define ARDUINO_BOARD      "???"
#endif
#elif defined(ARDUINO_ARCH_SAMD) // M0 PRO
#define ARDUINO_DTOSTRF    1
#define ARDUINO_BOARD      "SAMD"
#include "avr/dtostrf.h"
#else
#error "Unknown Arduino platform"
#endif

#if defined(ARDUINO_MEDIATEK)
typedef enum
{
	VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_NONE_PROXY_APN = 1,  /* only use gprs apn which donot have proxy */
	VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_PROXY_APN = 2,       /* only use gprs apn which has a proxy */
	VM_BEARER_DATA_ACCOUNT_TYPE_WLAN = 3,                 /* use wlan to connect */
	VM_BEARER_DATA_ACCOUNT_TYPE_GPRS_CUSTOMIZED_APN = 6,  /* use customized apn */
	VM_BEARER_DATA_ACCOUNT_TYPE_MAX = 0x7FFFFFFF
}VM_BEARER_DATA_ACCOUNT_TYPE;

typedef enum
{
	VM_BEARER_DEACTIVATED = 0x01,                         /* bearer deactivated */
	VM_BEARER_ACTIVATING = 0x02,                          /* bearer activating */
	VM_BEARER_ACTIVATED = 0x04,                           /* bearer activated */
	VM_BEARER_DEACTIVATING = 0x08,                        /* bearer deactivating */
	VM_BEARER_BEARER_STATE_MAX = 0x7FFFFFFF
}VM_BEARER_STATE;

/* network bearer result */
typedef enum
{
	VM_BEARER_OK = 0,                                      /* success */
	VM_BEARER_ERROR = -1,                                  /* general error */
	VM_BEARER_WOULDBLOCK = -2,                             /* would block */
	VM_BEARER_LIMIT_RESOURCE = -3,                         /* limited resource */
	VM_BEARER_INVALID_ACCOUNT_ID = -4,                     /* invalid account id*/
	VM_BEARER_BEARER_FAIL = -7,                            /* bearer fail */
	VM_BEARER_DHCP_ERROR = -8,                             /* DHCP get IP error */
	VM_BEARER_IPV6_ERROR = -11,                            /* IP get V6 address error */
	VM_BEARER_RESULT_MAX = 0x7FFFFFFF
}VM_BEARER_RESULT;

#endif

#endif /* __LiveObjectsClient_Paltform_H_ */
