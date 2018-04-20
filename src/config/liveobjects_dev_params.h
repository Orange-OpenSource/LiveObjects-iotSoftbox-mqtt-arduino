/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 *
 * This file is a part of LiveObjects iotsoftbox-mqtt library.
 */

/**
 * @file  liveobjects_dev_params.h
 * @brief Default user parameters used to configure this device as a Live Objects Device
 */

#ifndef __liveobjects_dev_params_H_
#define __liveobjects_dev_params_H_

// The flag define the modem to use
//#define ARDUINO_CONN_ITF  -1 // Internal modem
#define ARDUINO_CONN_ITF  4 // External modem

// Only used to overwrite the LiveOjects Server settings :
// IP address, TCP port, Connection timeout in milliseconds.
//#define LOC_SERV_IP_ADDRESS                  "XXXX"
//#define LOC_SERV_PORT                        8883
#define SECURITY_ENABLED                     1
//#define LOC_SERV_TIMEOUT                     XXXX

// When there is an issue with DNS to resolve FQDN liveobjects.orange-business.com ?
#if defined(ARDUINO_ARCH_MTK) || defined(ARDUINO_MTK_ONE)
#define LOC_SERV_IP_ADDRESS                  "84.39.42.208"
#else
//#define LOC_SERV_IP_ADDRESS                  "84.39.42.214"
#endif

#endif //__liveobjects_dev_params_H_
