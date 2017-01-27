/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file socket_defs.h
 * @brief Socket  definitions (handle, ..).
 *
 */

#ifndef SOCKET_DEFS_H_
#define SOCKET_DEFS_H_

#include <Arduino.h>

#include "liveobjects-sys/LiveObjectsClient_Platform.h"

#if defined(ARDUINO_MEDIATEK)
#include <vmdatetime.h>
#include <vmsock.h>
#endif /* ARDUINO_ARCH_MTK  or  ARDUINO_MTK_ONE */

#if defined(__cplusplus)
 extern "C" {
#endif

// Define the platform path to get the header file : MQTTPacket.h
// used by the header filme : paho-mqttclient-embedded-c/MQTTClient.h
#define MQTT_PACKET_HEADER_FILE   "MQTTPacket/MQTTPacket.h"

#if defined(ARDUINO_MEDIATEK)
// Socket handle 
#define SOCKETHANDLE_NULL      ((socketHandle_t)-1)
typedef int socketHandle_t;
#else
#define SOCKETHANDLE_NULL      ((socketHandle_t)0)
typedef void* socketHandle_t;
#endif

#if defined(__cplusplus)
 }
#endif

#endif // SOCKET_DEFS_H_
