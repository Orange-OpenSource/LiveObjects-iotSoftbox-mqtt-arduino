/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file  netw_wrapper.cp
 * @brief Network Interface.
 */

#include <stdbool.h>
#include <string.h>

#include "netw_wrapper.h"
#include "netw_sock.h"

#include "liveobjects-client/LiveObjectsClient_Config.h"

#include "liveobjects-sys/loc_trace.h"
#include "liveobjects-sys/LiveObjectsClient_Platform.h"
#include "platform_default.h"

#if (LOC_MQTT_DUMP_MSG & 0x02)
void LOCC_mqtt_dump_msg(const unsigned char* p_buf);
#endif

/* --------------------------------------------------------------------------------- */
/*  */
void netw_disconnect(Network *pNetwork, int mode) {
	if (f_netw_sock_isOpen(pNetwork)) {
		f_netw_sock_close(pNetwork);
	}
	LOTRACE_INF("RESET");
}

/* --------------------------------------------------------------------------------- */
/*  */
unsigned char netw_isLost(Network *pNetwork) {
	if (pNetwork) {
		return f_netw_sock_isLost(pNetwork);
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int netw_mqtt_write(Network *pNetwork, unsigned char *pMsg, int len, int timeout_ms) {
	int written = 0;
	LOTRACE_DBG1("(%p/%p, len=%d,timeout_ms=%d) ...", pNetwork, pNetwork->my_socket, len, timeout_ms);

#if (LOC_MQTT_DUMP_MSG & 0x02)
	LOCC_mqtt_dump_msg(pMsg);
#endif

	written = f_netw_sock_send(pNetwork, pMsg, len);
	if (written < 0) {
		LOTRACE_ERR("(len=%d,timeout_ms=%d) ERROR %d", len, timeout_ms, written);
		return written;
	}

	LOTRACE_DBG1("(len=%d,timeout_ms=%d) -> written=%d", len, timeout_ms, written);
	return written;
}

/* --------------------------------------------------------------------------------- */
/*  */
int netw_mqtt_read(Network *pNetwork, unsigned char *pMsg, int len, int timeout_ms) {
	int ret = -1;

	/* LOTRACE_DBG_VERBOSE("(%p/%p, len=%d,timeout_ms=%d) ...",  pNetwork, pNetwork->my_socket, len, timeout_ms); */

	ret = f_netw_sock_recv_timeout(pNetwork, pMsg, len, timeout_ms);
    if (ret < 0) {
        if ((ret != NETW_ERR_NET_RECV_WANT_READ) && (ret != NETW_ERR_NET_RECV_TIMEOUT)) {
            LOTRACE_ERR("f_netw_sock_recv_timeout(len=%d) -> ERROR %d x%x", len, ret, ret);
        }
        return ret;
    }

	LOTRACE_DBG_VERBOSE("netw_mqtt_read(len=%d,timeout_ms=%d) ret=%d", len, timeout_ms, ret);

	return ret;
}

/* --------------------------------------------------------------------------------- */
/*  */
int netw_init(Network *pNetwork, void* net_iface_handler) {

	LOTRACE_DBG1("netw_init(%p,%p)", pNetwork, net_iface_handler);

	f_netw_sock_init(pNetwork, net_iface_handler);

	LOTRACE_DBG1("netw_init: OK");

	if (pNetwork) {
		pNetwork->my_socket = SOCKETHANDLE_NULL;
		pNetwork->mqttread = netw_mqtt_read;
		pNetwork->mqttwrite = netw_mqtt_write;
		/* pNetwork->disconnect = netw_mqtt_disconnect; */
	}

	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
int netw_connect(Network* pNetwork, LiveObjectsNetConnectParams_t* params) {
	int ret;
	LOTRACE_INF("Connecting to server %s:%d tmo=%u ...", params->RemoteHostAddress, params->RemoteHostPort,
			params->TimeoutMs);

	if (f_netw_sock_isOpen(pNetwork)) {
		netw_disconnect(pNetwork, 0);
	}

	ret = f_netw_sock_connect(pNetwork, params->RemoteHostAddress, params->RemoteHostPort, params->TimeoutMs);
	if (ret) {
		LOTRACE_ERR("Failed to create TCP socket");
		return -1;
	}
	LOTRACE_INF("Connected to server %s:%d OK", params->RemoteHostAddress, params->RemoteHostPort);

	ret = 0;

	f_netw_sock_setup(pNetwork);

	return ret;
}
