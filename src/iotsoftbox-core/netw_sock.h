/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file   netw_sock.h
 * @brief  Define network socket interface
 */

#ifndef __netw_sock_H_
#define __netw_sock_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#include "liveobjects-client/LiveObjectsClient_Config.h"

#include "liveobjects-sys/mqtt_network_interface.h"

#if (LOC_FEATURE_MBEDTLS == 1)     // MBEDTLS IMPLEMENTATTION

#include "mbedtls/net.h"
#include "mbedtls/ssl.h"

#define  NETW_ERR_NET_INVALID_CONTEXT    MBEDTLS_ERR_NET_INVALID_CONTEXT
#define  NETW_ERR_NET_CONN_RESET         MBEDTLS_ERR_NET_CONN_RESET
#define  NETW_ERR_NET_SEND_FAILED        MBEDTLS_ERR_NET_SEND_FAILED
#define  NETW_ERR_NET_RECV_FAILED        MBEDTLS_ERR_NET_RECV_FAILED

#define  NETW_ERR_SSL_WANT_READ          MBEDTLS_ERR_SSL_WANT_READ
#define  NETW_ERR_SSL_WANT_WRITE         MBEDTLS_ERR_SSL_WANT_WRITE
#define  NETW_ERR_SSL_TIMEOUT            MBEDTLS_ERR_SSL_TIMEOUT

#else

#define  NETW_ERR_NET_INVALID_CONTEXT    -1
#define  NETW_ERR_NET_CONN_RESET         -2
#define  NETW_ERR_NET_SEND_FAILED        -3
#define  NETW_ERR_NET_RECV_FAILED        -4

#define  NETW_ERR_SSL_WANT_READ          -5
#define  NETW_ERR_SSL_WANT_WRITE         -6
#define  NETW_ERR_SSL_TIMEOUT            -7

#endif

int f_netw_sock_init(Network *pNetwork, void* net_iface_handler);

uint8_t f_netw_sock_isOpen(Network *pNetwork);

uint8_t f_netw_sock_isLost(Network *pNetwork);

int f_netw_sock_setup(Network *pNetwork);

int f_netw_sock_close(Network *pNetwork);

int f_netw_sock_connect(Network *pNetwork, const char* RemoteHostAddress, uint16_t RemoteHostPort, uint32_t tmo_ms);

/*
 * mbetls compatible interface (see mbedtls_ssl_set_bio() function called in netw_wrapper.c)
 */
int f_netw_sock_send(void *pNetwork, const unsigned char *buf, size_t len);

int f_netw_sock_recv(void *pNetwork, unsigned char *buf, size_t len);

int f_netw_sock_recv_timeout(void *pNetwork, unsigned char *buf, size_t len, uint32_t tmo);

#if defined(__cplusplus)
}
#endif

#endif /* __netw_sock_H_ */
