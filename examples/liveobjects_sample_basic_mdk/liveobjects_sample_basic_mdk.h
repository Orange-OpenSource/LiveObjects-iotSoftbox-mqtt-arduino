/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
  * @file  liveobjects_sample_basic.h
  * @brief Interfaces used by this Live Objects Basic Sample
  */
#ifndef __liveobjects_sample_H__
#define __liveobjects_sample_H__

// Definitions for this board or os.
#include "liveobjects-sys/LiveObjectsClient_Platform.h"

//set to use Wifi or GPRS
#define WIFI_USED 0 //false  //true (Wifi) or false (GPRS)

/* change GPRS settings here */
#define GPRS_APN       "orange.m2m.spec"
#define GPRS_USERNAME  "orange"
#define GPRS_PASSWORD  "orange"


#endif // __liveobjects_sample_H__

