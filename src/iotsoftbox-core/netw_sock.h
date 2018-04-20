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

#define  NETW_ERR_NET_INVALID_CONTEXT    -1
#define  NETW_ERR_NET_CONN_RESET         -2
#define  NETW_ERR_NET_SEND_FAILED        -3
#define  NETW_ERR_NET_RECV_FAILED        -4
#define  NETW_ERR_NET_RECV_TIMEOUT       -5
#define  NETW_ERR_NET_RECV_WANT_READ     -6
#define  NETW_ERR_NET_RECV_WANT_WRITE    -7

int f_netw_sock_init(Network *pNetwork, void* net_iface_handler);

uint8_t f_netw_sock_isOpen(Network *pNetwork);

uint8_t f_netw_sock_isLost(Network *pNetwork);

int f_netw_sock_setup(Network *pNetwork);

int f_netw_sock_close(Network *pNetwork);

int f_netw_sock_connect(Network *pNetwork, const char* RemoteHostAddress, uint16_t RemoteHostPort, uint32_t tmo_ms);

int f_netw_sock_send(void *pNetwork, const unsigned char *buf, size_t len);

int f_netw_sock_recv(void *pNetwork, unsigned char *buf, size_t len);

int f_netw_sock_recv_timeout(void *pNetwork, unsigned char *buf, size_t len, uint32_t tmo);

#if defined(__cplusplus)
}
#endif

#endif /* __netw_sock_H_ */
