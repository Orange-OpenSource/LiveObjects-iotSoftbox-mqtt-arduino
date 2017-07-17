/*
 *  Platform-specific and custom entropy polling functions
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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_ENTROPY_C)
#include <Arduino.h>


#include "mbedtls/entropy.h"
#include "mbedtls/entropy_poll.h"

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

void randomSeed( uint32_t dwSeed )
{
  if ( dwSeed != 0 )
  {
    srand( dwSeed ) ;
  }
}


long random1( long howbig )
{
  if ( howbig == 0 )
  {
    return 0 ;
  }

  return rand() % howbig;
}

long random2( long howsmall, long howbig )
{
  if (howsmall >= howbig)
  {
    return howsmall;
  }

  long diff = howbig - howsmall;

  return random1(diff) + howsmall;
}




int mbedtls_hardware_poll( void *data,
                           unsigned char *output, size_t len, size_t *olen )
{

    randomSeed(2147483647);
    unsigned long random_seed = random2(0, 2147483647);

    ((void) data);
    *olen = 0;

    if( len < sizeof(unsigned long) )
        return( 0 );

    memcpy( output, &random_seed, sizeof(unsigned long) );
    *olen = sizeof(unsigned long);

    return( 0 );
}


#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */

#endif /* MBEDTLS_ENTROPY_C */
