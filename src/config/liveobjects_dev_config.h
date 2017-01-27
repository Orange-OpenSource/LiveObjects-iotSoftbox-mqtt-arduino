/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 *
 * This file is part of LiveObjects iotsoftbox-mqtt library.
 */

/**
 * @file  liveobjects_dev_config.h
 *
 * @brief User parameters used to overwrite the default LiveObjects parameters
 *        defined in header file : LiveObjectsClient_Config.h
 *
 */

#ifndef __liveobjects_dev_config_H_
#define __liveobjects_dev_config_H_

#if defined(ARDUINO_ARCH_AVR)

#define ARDUINO_CONN_ETH                     1

#define LOC_MQTT_DUMP_MSG                    0

#define LOC_FEATURE_MBEDTLS                  0

//#define LOC_FEATURE_LO_STATUS                0
//#define LOC_FEATURE_LO_PARAMS                0
//#define LOC_FEATURE_LO_DATA                  0
//#define LOC_FEATURE_LO_COMMANDS              0
#define LOC_FEATURE_LO_RESOURCES             0

//#define LOC_MQTT_API_KEEPALIVEINTERVAL_SEC   30
#define LOC_MQTT_DEF_COMMAND_TIMEOUT           10000
#define LOC_MQTT_DEF_SND_SZ                  (250)
#define LOC_MQTT_DEF_RCV_SZ                  (250)

#define LOC_MQTT_DEF_TOPIC_NAME_SZ           12
#define LOC_MQTT_DEF_DEV_ID_SZ               20
#define LOC_MQTT_DEF_NAME_SPACE_SZ           20

#define LOC_MQTT_DEF_PENDING_MSG_MAX         2
//#define LOC_MAX_OF_COMMAND_ARGS              5
#define LOC_MAX_OF_DATA_SET                  1

#define LOM_PUSH_ASYNC                       1
#define LOM_JSON_BUF_SZ                      200
#define LOM_JSON_BUF_USER_SZ                 200

#define LOM_SETOFDATA_STREAM_ID_SZ           40
//#define LOM_SETOFDATA_MODEL_SZ               20
#define LOM_SETOFDATA_TAGS_SZ                0

#elif defined(ARDUINO_ARCH_MTK) || defined(ARDUINO_MTK_ONE)

//#define LOC_MQTT_DUMP_MSG                    0

//#define LOC_FEATURE_MBEDTLS                  0

//#define LOC_FEATURE_LO_STATUS                0
//#define LOC_FEATURE_LO_PARAMS                0
//#define LOC_FEATURE_LO_DATA                  0
//#define LOC_FEATURE_LO_COMMANDS              0
//#define LOC_FEATURE_LO_RESOURCES             0

//#define LOC_MQTT_API_KEEPALIVEINTERVAL_SEC   30
#define LOC_MQTT_DEF_COMMAND_TIMEOUT           60000
//#define LOC_MQTT_DEF_SND_SZ                  (1024*2)
//#define LOC_MQTT_DEF_RCV_SZ                  (1024*2)

//#define LOC_MQTT_DEF_TOPIC_NAME_SZ           40
//#define LOC_MQTT_DEF_DEV_ID_SZ               20
//#define LOC_MQTT_DEF_NAME_SPACE_SZ           20

//#define LOC_MQTT_DEF_PENDING_MSG_MAX         5
//#define LOC_MAX_OF_COMMAND_ARGS              5
//#define LOC_MAX_OF_DATA_SET                  5

//#define LOM_JSON_BUF_SZ                      1024
//#define LOM_JSON_BUF_USER_SZ                 200

#else

# "ERROR : Arduino platform not defined "

#endif

#endif /* __liveobjects_dev_config_H_ */
