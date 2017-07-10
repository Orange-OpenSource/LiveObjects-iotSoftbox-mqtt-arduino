/**
 * \file config.h
 *
 * \brief Configuration options (set of defines)
 *
 *  This set of compile-time options may be used to enable
 *  or disable features selectively, and reduce the global
 *  memory footprint.
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef REDIRECT_MBEDTLS_CONFIG_H
#define REDIRECT_MBEDTLS_CONFIG_H

#if defined(ARDUINO_MTK_ONE)
#include <vmsys.h>
//#include "_config_mdk.h"
#include "_config_mdk_min.h"
#elif defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_ADK)
//#warning "mbedtls: ARDUINO_AVR_xxx"
#include "_config_mega.h"
#elif defined(ARDUINO_ARCH_SAMD)   // M0_PRO)
#include "_config_samd.h"
#else
#error "No config file for mbedtls !"
#endif

#if defined(TARGET_LIKE_MBED)
#include "mbedtls/target_config.h"
#endif

/*
 * Allow user to override any previous default.
 *
 * Use two macro names for that, as:
 * - with yotta the prefix YOTTA_CFG_ is forced
 * - without yotta is looks weird to have a YOTTA prefix.
 */
#if defined(YOTTA_CFG_MBEDTLS_USER_CONFIG_FILE)
#include YOTTA_CFG_MBEDTLS_USER_CONFIG_FILE
#elif defined(MBEDTLS_USER_CONFIG_FILE)
#include MBEDTLS_USER_CONFIG_FILE
#endif

#include "check_config.h"

#endif /* REDIRECT_MBEDTLS_CONFIG_H */
