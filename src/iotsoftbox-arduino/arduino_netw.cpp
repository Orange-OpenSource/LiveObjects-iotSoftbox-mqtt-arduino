/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file arduino_netw.c
 * @brief Network Interface.
 */

#include <arduino.h>

#include "liveobjects-Client/LiveObjectsClient_Config.h"
#include "liveobjects-sys/LiveObjectsClient_Platform.h"

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)

#define ARDUINO_ITF     1

#if (ARDUINO_CONN_ITF == 1)   /* GSM */
#include <GSM.h>
#elif (ARDUINO_CONN_ITF == 2) /* ETH1 */
#include <Ethernet.h>
#elif (ARDUINO_CONN_ITF == 3) /* ETH2 */
#include <Ethernet2.h>
#else
#error "No definition for interface"
#endif

#elif defined(ARDUINO_MEDIATEK)

/*
 * LinkIt/MediaTek Interfaces
 * See http://labs.mediatek.com/site/global/developer_tools/mediatek_linkit_assist_2502/api_references/C_STD.gsp p
 */
#include "vmsock.h"
#include "vmsys.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <errno.h>

#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include <stdint.h>

#else

#error "No definition for Arduino Architecture"

#endif

#include "liveobjects-sys/loc_trace.h"
#include "liveobjects-sys/mqtt_network_interface.h"

#include "iotsoftbox-core/loc_sock.h"
#include "iotsoftbox-core/netw_sock.h"

static socketHandle_t  _netw_socket = SOCKETHANDLE_NULL;
static uint8_t         _netw_bSockState;

#if defined(ARDUINO_ITF)

#if (ARDUINO_CONN_ITF == 1)                              /* GSM */
static GSMClient       _netw_client;
#elif (ARDUINO_CONN_ITF == 2) || (ARDUINO_CONN_ITF == 3) /* ETH1 or ETH2 */
static EthernetClient  _netw_client;
#endif

#endif

#define EXTERN_C  extern "C"

extern "C" uint64_t f_netw_be64toh(uint64_t x) {
	uint64_t r = x;
	LOTRACE_DBG1("f_netw_be64toh(x%lx)...", x);
	return r;
}

/* --------------------------------------------------------------------------------- */
/*  */
#if defined(ARDUINO_MEDIATEK)
extern "C" int netw_would_block( int sock_fd )
{
	/*
	 * Never return 'WOULD BLOCK' on a non-blocking socket
	 */
	if( ( fcntl( sock_fd, F_GETFL ) & O_NONBLOCK ) != O_NONBLOCK )
	return( 0 );

	switch( errno )
	{
#if defined EAGAIN
		case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
		case EWOULDBLOCK:
#endif
		return( 1 );
	}
	return( 0 );
}
#endif

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_init(Network *pNetwork, void* net_iface_handler) {
	(void)net_iface_handler;
	if (pNetwork) {
		pNetwork->my_socket = SOCKETHANDLE_NULL;
		pNetwork->mqttread = NULL;
		pNetwork->mqttwrite = NULL;
	}
	_netw_socket = SOCKETHANDLE_NULL;
	_netw_bSockState = 0;
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" uint8_t f_netw_sock_isOpen(Network *pNetwork) {
	(void)pNetwork;
	if (_netw_bSockState == 0x01) {
		return 1;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" uint8_t f_netw_sock_isLost(Network *pNetwork) {
	(void)pNetwork;
	if (_netw_bSockState & 0x01) {
		if (_netw_bSockState & 0x02)
			LOTRACE_INF("bSockState= x%0x ", _netw_bSockState);
		return _netw_bSockState & 0x02;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_close(Network *pNetwork) {
	LOTRACE_ERR_I("f_netw_sock_close(%" PRIsock " %p)...", _netw_socket, pNetwork);

#if defined(ARDUINO_MEDIATEK)
	if (_netw_socket >= 0) {
		vm_closesocket(_netw_socket);
	}
#endif
#if defined(ARDUINO_ITF)
	if ((_netw_socket != SOCKETHANDLE_NULL) && _netw_client.connected()) {
		_netw_client.stop();
	}
#endif

	_netw_socket = SOCKETHANDLE_NULL;
	_netw_bSockState = 0;
	if (pNetwork) {
		pNetwork->my_socket = SOCKETHANDLE_NULL;
	}
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_setup(Network *pNetwork) {
	(void)pNetwork;
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_connect(Network *pNetwork, const char* RemoteHostAddress, uint16_t RemoteHostPort,
		uint32_t tmo_ms) {
	int ret;
	LOTRACE_DBG1("(RemoteHostAddress=%s RemoteHostPort=%u tmo_ms=%u) (_netw_socket=%" PRIsock ") ...",
			RemoteHostAddress, RemoteHostPort, tmo_ms, _netw_socket);

#if defined(ARDUINO_MEDIATEK)
	if (_netw_socket >= 0) {
		vm_closesocket(_netw_socket);
	}
	_netw_socket = -1;
	_netw_bSockState = 0;

	ret = LO_sock_connect(1, RemoteHostAddress, RemoteHostPort, &_netw_socket);
	if ((ret == 0) && (_netw_socket >= 0)) {
		_netw_bSockState = 0x01;
	}
#endif /* ARDUINO_MEDIATEK */

#if defined(ARDUINO_ITF)
	if (_netw_client.connected()) {
		LOTRACE_ERR_I("Closing the connection ..");
		_netw_client.stop();
		delay(500);
	}
	_netw_socket = NULL;
	_netw_bSockState = 0;
	LOTRACE_ERR_I("Connecting to %s:%u ..", RemoteHostAddress, RemoteHostPort);
	ret = _netw_client.connect(RemoteHostAddress, RemoteHostPort);
	if (ret == 1) {
		_netw_bSockState = 0x01;
		_netw_socket = &_netw_client;
		ret = 0;
		LOTRACE_ERR_I("ARDUINO_ITF - Connected %" PRIsock, _netw_socket);
	}
	else {
		LOTRACE_ERR_I("Error(%d) while connecting to %s:%u", ret, RemoteHostAddress, RemoteHostPort);
		ret = -1;
	}
#endif /* ARDUINO_ITF */
	if (pNetwork) {
		pNetwork->my_socket = _netw_socket;
	}
	return ret;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_recv(void *pNetwork, unsigned char* buf, size_t len) {
	int ret;

#if defined(ARDUINO_MEDIATEK)
	if (_netw_socket < 0) {
		return ( NETW_ERR_NET_INVALID_CONTEXT);
	}

	LOTRACE_DBG_VERBOSE("(pNetwork=%p _netw_socket=%" PRIsock " buf=%p len=%d) ...", pNetwork, _netw_socket, buf, len);
	ret = (int) vm_recv(_netw_socket, (char*)buf, len, 0);
	if (ret < 0) {
		if (netw_would_block(_netw_socket) != 0) {
			LOTRACE_INF("(_netw_socket=%" PRIsock " len=%x) ret=%d x%x", _netw_socket, len, ret, NETW_ERR_SSL_WANT_READ);
			return ( NETW_ERR_SSL_WANT_READ);
		}
		if (errno == EINTR) {
			LOTRACE_INF("(ctx=%p _netw_socket=%" PRIsock " len=%x) ret=%d x%x", _netw_socket, len, ret,
					NETW_ERR_SSL_WANT_READ);
			return ( NETW_ERR_SSL_WANT_READ);
		}
		if (errno == EPIPE || errno == ECONNRESET) {
			_netw_bSockState |= 0x02;
			LOTRACE_ERR_I("(_netw_socket=%" PRIsock " len=%x) ret=%d errno=%d x%x", _netw_socket, len, ret, errno,
					NETW_ERR_NET_CONN_RESET);
			return ( NETW_ERR_NET_CONN_RESET);
		}
		LOTRACE_ERR_I("(_netw_socket=%" PRIsock " len=%x) ret=%d errno=%d x%x", _netw_socket, len, ret, errno,
				NETW_ERR_NET_RECV_FAILED);
		return ( NETW_ERR_NET_RECV_FAILED);
	}
#endif
#if defined(ARDUINO_ITF)
	if (_netw_client.connected()) {
		ret = _netw_client.available();
		if (ret > 0) {
			LOTRACE_DBG_VERBOSE("RCV: connected and available=%d ...", ret);
			ret = _netw_client.read(buf, len);
		}
		else {
			ret = NETW_ERR_SSL_WANT_READ;
		}
	}
	else {
		LOTRACE_ERR_I("Disconnected");
		ret = NETW_ERR_NET_CONN_RESET;
		_netw_bSockState |= 0x02;
	}
#endif
	LOTRACE_DBG_VERBOSE("(_netw_socket=%" PRIsock " len=%d) ret=%d", _netw_socket, len, ret);
	return (ret);
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_recv_timeout(void *pNetwork, unsigned char *buf, size_t len, uint32_t timeout) {
#if defined(ARDUINO_MEDIATEK)
	int ret;
	timeval tv;
	fd_set read_fds;

	LOTRACE_DBG_VERBOSE("(pNetwork=%p _netw_socket=%" PRIsock " buf=%p len=%d tmo=%u)...", pNetwork, _netw_socket, buf, len,
			timeout);

	if (_netw_socket < 0) {
		LOTRACE_ERR_I("Invalid context %d", _netw_socket);
		return ( NETW_ERR_NET_INVALID_CONTEXT);
	}

	FD_ZERO(&read_fds);
	FD_SET(_netw_socket, &read_fds);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	const timeval tv_const = tv;

	ret = vm_select(_netw_socket + 1, &read_fds, NULL, NULL, timeout == ((uint32_t) -1) ? NULL : &tv_const);
	/* Zero fds ready means we timed out */
	if (ret == 0) {
		LOTRACE_DBG_VERBOSE("TIMEOUT (sock=%" PRIsock " len=%d tmo=%u) => x%x!", _netw_socket, len, timeout,
				NETW_ERR_SSL_TIMEOUT);
		return ( NETW_ERR_SSL_TIMEOUT);
	}

	if (ret < 0) {
		if (errno == EINTR) {
			LOTRACE_NOTICE("SELECT INTERRUPT (sock=%" PRIsock " tmo=%u) %d !", _netw_socket, timeout, ret);
			return ( NETW_ERR_SSL_WANT_READ);
		}
		LOTRACE_NOTICE("SELECT ERR (sock=%" PRIsock " tmo=%u) %d !", _netw_socket, timeout, ret);
		return ( NETW_ERR_NET_RECV_FAILED);
	}
#endif
#if defined(ARDUINO_ITF)
	LOTRACE_DBG2("RCV(len=%u, tmo=%u)...", len, timeout);
	if (timeout > 0) {
		uint32_t dt = 0;
		while (_netw_client.connected() && !_netw_client.available() && (dt < timeout)) {
			delay(100);
			dt += 100;
		}
		LOTRACE_DBG2("available=%u ...", _netw_client.available());
		if (!_netw_client.connected()) {
			LOTRACE_ERR_I("Disconnected ! (tmo=%u dt=%u)", timeout, dt);
			_netw_bSockState |= 0x02;
			return (NETW_ERR_NET_RECV_FAILED);
		}
		if (!_netw_client.available()) {
			LOTRACE_DBG2("TIMEOUT ! (tmo=%u dt=%u)", timeout, dt);
			return (NETW_ERR_SSL_TIMEOUT);
		}
	}
#endif

	/* This call will not block */
	return (f_netw_sock_recv(pNetwork, buf, len));
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int f_netw_sock_send(void *pNetwork, const unsigned char *buf, size_t len) {
	int ret;

	LOTRACE_DBG2("(pNetwork=%p _netw_socket=%" PRIsock " buf=%p len=%d)...", pNetwork, _netw_socket, buf, len);

#if defined(ARDUINO_MEDIATEK)
	if (_netw_socket < 0) {
		LOTRACE_ERR_I("Invalid context %d", _netw_socket);
		return ( NETW_ERR_NET_INVALID_CONTEXT);
	}
	ret = (int) vm_send(_netw_socket, (const char *)buf, len, 0);
	if (ret < 0) {
		if (netw_would_block(_netw_socket) != 0)
		return ( NETW_ERR_SSL_WANT_WRITE);

		if (errno == EINTR)
		return ( NETW_ERR_SSL_WANT_WRITE);

		LOTRACE_ERR_I("ERROR %d (errno=%d) returnd by vm_send(len=%d)", ret, errno, len);

		if (errno == EPIPE || errno == ECONNRESET)
		return ( NETW_ERR_NET_CONN_RESET);
		return ( NETW_ERR_NET_SEND_FAILED);
	}
#endif
#if defined(ARDUINO_ITF)
	if (_netw_socket != &_netw_client) {
		LOTRACE_ERR_I("Invalid context %" PRIsock, _netw_socket);
		return ( NETW_ERR_NET_INVALID_CONTEXT);
	}
	if (_netw_client.connected()) {
#if (ARDUINO_CONN_ITF == 1) /* GSM */
		_netw_client.beginWrite(true);
#endif
		ret = _netw_client.write(buf, len);
#if (ARDUINO_CONN_ITF == 1) /* GSM */
		_netw_client.endWrite(true);
#endif
		if (ret != len) {
			LOTRACE_ERR_I("ERROR %d returned by _netw_client.write(len=%d)", ret, len);
		}
	}
	else {
		LOTRACE_ERR_I("ERROR - not connected to send % bytes", len);
		ret = NETW_ERR_NET_CONN_RESET;
	}
#endif
	LOTRACE_DBG_VERBOSE("(_netw_socket=%" PRIsock " len=%d) ret= %d", _netw_socket, len, ret);
	return (ret);
}

