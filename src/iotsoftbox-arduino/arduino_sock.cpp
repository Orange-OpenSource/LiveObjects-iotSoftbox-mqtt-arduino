/*
 * Copyright (C) 2016 Orange
 *
 * This software is distributed under the terms and conditions of the 'BSD-3-Clause'
 * license which can be found in the file 'LICENSE.txt' in this package distribution
 * or at 'https://opensource.org/licenses/BSD-3-Clause'.
 */

/**
 * @file  arduino_sock.c
 * @brief TCP Socket Interface used by loc_wget
 * @note  Check only one socket
 */

#include <Arduino.h>

#include "liveobjects-client/LiveObjectsClient_Config.h"

#if (LOC_FEATURE_LO_RESOURCES)

#include "liveobjects-sys/LiveObjectsClient_Platform.h"
#include "liveobjects-sys/loc_trace.h"

#include "iotsoftbox-core/loc_sock.h"
#include "iotsoftbox-core/netw_sock.h"

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)

#define ARDUINO_ITF     1

#define DNS_ENABLE   0

#include "config/liveobjects_dev_params.h"

#if (ARDUINO_CONN_ITF == 1)    /* GSM */
#include <GSM.h>
#elif (ARDUINO_CONN_ITF == 2)  /* ETH1 */
#include <Ethernet.h>
#elif (ARDUINO_CONN_ITF == 3)  /* ETH2 */
#include <Ethernet2.h>
#else
#error "No definition for interface"
#endif

#elif defined(ARDUINO_MEDIATEK)

#define DNS_ENABLE   1

#include <vmsock.h>
#include <vmsys.h>

#endif /* ARDUINO_MEDIATEK*/

#include <fcntl.h>
#include <errno.h>


#if defined(ARDUINO_ITF)

#if (ARDUINO_CONN_ITF == 1)                              /* GSM */
static GSMClient      _LO_sock_client;
#elif (ARDUINO_CONN_ITF == 2) || (ARDUINO_CONN_ITF == 3) /* ETH1 or ETH2 */
static EthernetClient _LO_sock_client;
#endif

#endif


static const char* sock_error_name(int err) {
#if defined(ARDUINO_MEDIATEK)
	switch (err) {
		case VM_E_SOC_ERROR :             return "VM_E_SOC_ERROR";
		case VM_E_SOC_WOULDBLOCK :        return "VM_E_SOC_WOULDBLOCK  not done yet";
		case VM_E_SOC_LIMIT_RESOURCE :    return "VM_E_SOC_LIMIT_RESOURCE";
		case VM_E_SOC_INVALID_SOCKET :    return "VM_E_SOC_INVALID_SOCKET";
		case VM_E_SOC_INVALID_ACCOUNT :   return "VM_E_SOC_INVALID_ACCOUNT";
		case VM_E_SOC_NAMETOOLONG :       return "VM_E_SOC_NAMETOOLONG";
		case VM_E_SOC_ALREADY :           return "VM_E_SOC_ALREADY";
		case VM_E_SOC_OPNOTSUPP :         return "VM_E_SOC_OPNOTSUPP";
		case VM_E_SOC_CONNABORTED :       return "VM_E_SOC_CONNABORTED";
		case VM_E_SOC_INVAL :             return "VM_E_SOC_INVAL";
		case VM_E_SOC_PIPE :              return "VM_E_SOC_PIPE";
		case VM_E_SOC_NOTCONN :           return "VM_E_SOC_NOTCONN";
		case VM_E_SOC_MSGSIZE :           return "VM_E_SOC_MSGSIZE";
		case VM_E_SOC_CONNRESET :         return "VM_E_SOC_CONNRESET";
		case VM_E_SOC_DHCP_ERROR :        return "VM_E_SOC_DHCP_ERROR";

		case VM_E_SOC_IP_CHANGED :        return "VM_E_SOC_IP_CHANGED";
		case VM_E_SOC_ADDRINUSE :         return "VM_E_SOC_ADDRINUSE";
		case VM_E_SOC_CANCEL_ACT_BEARER : return "VM_E_SOC_CANCEL_ACT_BEARER";

	}
	return "Unknown";
#else
	return "not_yet";
#endif
}

/* --------------------------------------------------------------------------------- */
/*  */
#if defined(ARDUINO_MEDIATEK)
static int sock_would_block(int sock_fd) {
	/*
	 * Never return 'WOULD BLOCK' on a non-blocking socket
	 */
	if ((fcntl(sock_fd, F_GETFL) & O_NONBLOCK) != O_NONBLOCK)
	return (0);

	switch (errno) {
#if defined EAGAIN
		case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
		case EWOULDBLOCK:
#endif
		return (1);
	}
	return (0);
}
#endif

/* --------------------------------------------------------------------------------- */
/*  */
#if DNS_ENABLE

static volatile char _dns_resolved = 0;
static uint32_t _dns_millis;
static char _dns_domain_name[100];

static vm_soc_dns_result _dns_result;
static VMCHAR _dns_ipv4_addr[16];

void app_dns_resolve_awakeup(char state);

static int dns_resolve_update(vm_soc_dns_result *p_dns_result, VMCHAR* ipv4_addr_ptr, int ipv4_addr_len) {
	int i;
	LOTRACE_INF("dns_resolve_update %p %p - num=%d", _dns_result, p_dns_result, p_dns_result->num);
	for (i = 0; i < p_dns_result->num; i++) {
#if 0
		const char* addr = inet_ntop(0, &p_dns_result->address[i], _dns_ipv4_addr, sizeof(_dns_ipv4_addr));
		if (addr == _dns_ipv4_addr) {
			LOTRACE_INF("DNS_ENTRY[%d] = %X %s", p_dns_result->address[i], _dns_ipv4_addr);
		}
		else {
			LOTRACE_INF("DNS_ENTRY[%d] = %X - inet_ntop ERROR", p_dns_result->address[i]);
		}
#else
		IN_ADDR ip_addr;
		ip_addr.S_un.s_addr = p_dns_result->address[i];
		char* addr = inet_ntoa(ip_addr);
		if ((addr) &&(*addr)) {
			_dns_resolved = 2;
			strncpy(ipv4_addr_ptr, addr, ipv4_addr_len);
			LOTRACE_INF("DNS_ENTRY[%d] = %X  %s", i, p_dns_result->address[i], _dns_ipv4_addr);
			return 0;
		}
		else
		LOTRACE_INF("DNS_ENTRY[%d] = x%X - inet_ntoa ERROR", ip_addr.S_un.s_addr);
#endif
	}
	return -1;
}

/* --------------------------------------------------------------------------------- */
/*  */
VMINT dns_resolve_callback(vm_soc_dns_result *p_dns_result) {
	if ((p_dns_result) &&(p_dns_result->num > 0)) {
		if (p_dns_result == &_dns_result)
			_dns_resolved = 1;
		else
			_dns_resolved = -2;
	}
	else {
		_dns_resolved = -1;
	}
#if 0 //TODO : require to have a callback to solve the issue during link
	app_dns_resolve_awakeup(_dns_resolved);
#endif
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
static int LO_sock_getHostByName(const char *domain_name) //, int tmo, char* pAddr, int len )
{
	VMINT apn;
	IN_ADDR addr;
	VMINT ret;

#if 0
	if (vm_wifi_is_connected()) {
		apn = VM_TCP_APN_WIFI;
	}
	else {
		apn = VM_APN_USER_DEFINE; // VM_TCP_APN_CMNET;
	}
#else
	apn = VM_APN_USER_DEFINE;
#endif

	strncpy(_dns_domain_name, domain_name, sizeof(_dns_domain_name));
	//_dns_domain_name = domain_name;
	_dns_resolved = 0;
	_dns_millis = millis();

	LOTRACE_INF("LO_sock_getHostByName : %s (apn=%d) at %u ms", _dns_domain_name, apn, _dns_millis);

	ret = vm_soc_get_host_by_name(apn, _dns_domain_name, &_dns_result, dns_resolve_callback);
	if (ret > 0) {
#if 0
		int tmo = 60; // 60 seconds max.
		int cnt = 0;
		while ((_dns_resolved == 0) && (cnt < tmo)) {
			LOTRACE_INF("LO_sock_getHostByName : ret=%d => Not Yet %d/%d", ret, cnt, tmo);
			cnt++;
			delay(1000);
		}
		if (_dns_resolved <= 0) { // Timeout
			LOTRACE_NOTICE("LO_sock_getHostByName : _dns_resolved=% => TIMEOUT or ERROR  !!", _dns_resolved);
			return -1;
		}
		LOTRACE_INF("LO_sock_getHostByName : RESOLVED (cnt=%d/%d) !!", cnt, tmo);
#else
		LOTRACE_INF("LO_sock_getHostByName : ret=%d => Not Yet ", ret);
#endif
		return ret;
	}

	if (ret == VM_E_SOC_SUCCESS) {
		LOTRACE_NOTICE("getHostByName : IN CACHE");
		ret = dns_resolve_callback(&_dns_result);
		return ret;
	}
	LOTRACE_ERR("Error, ret= %d %s", ret, sock_error_name(ret));
	return -1;
}
#endif

/* --------------------------------------------------------------------------------- */
/*  */
int LO_sock_dnsSetFQDN(const char* domain_name, const char* ip_address) {
	if ((domain_name) &&(*domain_name)) {
		if (ip_address == NULL) {
#if DNS_ENABLE
			int ret;
			ret = LO_sock_getHostByName(domain_name);
			return ret;
#else
			return -1;
#endif
		}

#if defined(ARDUINO_MEDIATEK)
		_dns_result.address[0] = inet_addr(ip_address);
		if (_dns_result.address[0]) {
			_dns_resolved = 2;
			_dns_result.num = 1;
			strncpy(_dns_domain_name, domain_name, sizeof(_dns_domain_name));
			strncpy(_dns_ipv4_addr, ip_address, sizeof(_dns_ipv4_addr));
			return 0;
		}
#endif
	}
	return -1;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" void LO_sock_disconnect(socketHandle_t *pHdl) {
	if (pHdl) {
		if (*pHdl != SOCKETHANDLE_NULL) {
#if defined(ARDUINO_MEDIATEK)
			vm_closesocket(*pHdl);
#endif
#if defined(ARDUINO_ITF)
			_LO_sock_client.stop();
#endif
		}
		*pHdl = SOCKETHANDLE_NULL;
	}
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int LO_sock_connect(short retry, const char* remoteHostAddress, uint16_t remoteHostPort, socketHandle_t *pHdl) {
	int i;
	int ret;
#if defined(ARDUINO_MEDIATEK)
	int sock_fd;
	SOCKADDR_IN t_addr_in = {0};
	uint32_t t_ai_addrlen; /* Length of socket address. */
	const char* remote_host;

	if (pHdl)
	*pHdl = SOCKETHANDLE_NULL;

	if (remoteHostAddress == NULL) {
		return -1;
	}
	LOTRACE_INF("Connecting to server %s:%d (retry=%d) ...", remoteHostAddress, remoteHostPort, retry);

	if ((*remoteHostAddress >= '0') && (*remoteHostAddress <= '9')) {
		remote_host = remoteHostAddress;
	}
	else {
#if DNS_ENABLE
		int cnt = 60;
		LOTRACE_INF("Call DNS to resolve %s   ...", remoteHostAddress);
#if 0
		ret = LO_sock_getHostByName(remoteHostAddress);
		if (ret < 0) {
			LOTRACE_INF("ERROR to resolve %s !!", remoteHostAddress);
			return ret;
		}
#else
		if (strcmp(remoteHostAddress, _dns_domain_name)) {
			LOTRACE_NOTICE("ERROR - unknown host %s !! (%s)", remoteHostAddress, _dns_domain_name);
			return -1;
		}
#endif
		if (_dns_resolved == 0) {
			LOTRACE_INF("%u (%u)- Wait for DNS to resolve %s   ...", millis(), millis() - _dns_millis,
					remoteHostAddress);
			while ((_dns_resolved == 0) && (--cnt > 0)) {
				delay(1000);
			}
		}
		if (_dns_resolved <= 0) {
			LOTRACE_NOTICE("%u (%u) - ERROR (%d) to resolve %s !!", millis(), millis() - _dns_millis, _dns_resolved,
					remoteHostAddress);
			return -1;
		}
		if (_dns_resolved == 1) {
			LOTRACE_INF("%u (%u)- Update DNS %s   ...", millis(), millis() - _dns_millis, remoteHostAddress);
			ret = dns_resolve_update(&_dns_result, _dns_ipv4_addr, sizeof(_dns_ipv4_addr));
			if (ret) {
				LOTRACE_NOTICE("ERROR (%d) to update dns result !!", ret);
				return -1;
			}
		}
		remote_host = _dns_ipv4_addr;

		LOTRACE_INF("%u (%u) - IP Resolver %s %s", millis(), millis() - _dns_millis, remoteHostAddress, remote_host);
#else
		// DNS resolver ?
		if (!strcmp(remoteHostAddress,"liveobjects.orange-business.com")) {
			remote_host = "84.39.42.214";
		}
		else {
			remote_host = "84.39.42.208";
		}
		LOTRACE_INF("NO DNS resolver : %s ->  %s ...", remoteHostAddress, remote_host);
#endif
	}

	t_addr_in.sin_family = PF_INET;  //IPv4
	t_addr_in.sin_addr.S_un.s_addr = inet_addr(remote_host);
	t_addr_in.sin_port = htons(remoteHostPort);

	t_ai_addrlen = sizeof(SOCKADDR);

	LOTRACE_INF(" vm_socket ...");
	sock_fd = (int) vm_socket(t_addr_in.sin_family, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		LOTRACE_ERR("Error, ret= %d %s", sock_fd, sock_error_name(sock_fd));
		return sock_fd;
	}

	if (retry <= 0)
	retry = 1;

	ret = -1;
	for (i = 0; i < retry; i++) {
		LOTRACE_INF(" vm_connect(%d, %X : %d) ...", sock_fd, t_addr_in.sin_addr.S_un.s_addr, remoteHostPort);
		ret = vm_connect(sock_fd, (SOCKADDR*) &t_addr_in, sizeof(SOCKADDR));
		if (ret == 0)
		break;
		LOTRACE_ERR("vm_connect error, ret= %d %s (sock_fd=%d retry=%d)", ret, sock_error_name(ret), sock_fd, i);
		delay(200);
	}
	LOTRACE_INF("Connected to server %s:%d - sock=%d", remoteHostAddress, remoteHostPort, sock_fd);

	if (pHdl)
	*pHdl = sock_fd;
#endif
#if  defined(ARDUINO_ITF)

	if (_LO_sock_client.connected()) {
		_LO_sock_client.stop();
		delay(500);
	}

	ret = _LO_sock_client.connect(remoteHostAddress, remoteHostPort);
	if (ret == 1) {
		ret = 0;
	}
	else {
		ret = -1;
	}
	if (pHdl)
	*pHdl = &_LO_sock_client;
#endif
	return ret;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int LO_sock_send(socketHandle_t hdl, const char* buf_ptr) {
	int len, ret;
	const char* pc = buf_ptr;
	if ((hdl < 0) || (pc == NULL)) {
		LOTRACE_ERR("Invalid parameter hdl=%d buf=%p", hdl, pc);
		return -1;
	}
	len = strlen(buf_ptr);

	LOTRACE_DBG1("len=%d\r\n%s", len, buf_ptr);
#if defined(ARDUINO_MEDIATEK)
	while (len > 0) {
		ret = (int) vm_send(hdl, pc, len, 0);
		if (ret < 0) {
			if (sock_would_block(hdl) != 0) {
				LOTRACE_NOTICE("WOULD_BLOCK !!");
				continue;
			}
			if (errno == EINTR) {
				LOTRACE_NOTICE("INTERRUPT !!");
				continue;
			}
			if (errno == EPIPE || errno == ECONNRESET) {
				LOTRACE_NOTICE("Closed by peer");
				return -1;
			}

			LOTRACE_NOTICE("ERROR (ret=%d errno=%d) while sending data , len=%d/%d", ret, errno,
					strlen(buf_ptr) - len, strlen(buf_ptr));
			return -1;
		}
		if (ret == 0) {
			LOTRACE_NOTICE("ret=0 => WOULD_BLOCK or Closed by peer ???");
		}
		pc += ret;
		len -= ret;
	}
#endif
#if defined(ARDUINO_ITF)
	if (hdl != &_LO_sock_client) {
		LOTRACE_ERR("Invalid context %p", &_LO_sock_client);
		return ( NETW_ERR_NET_INVALID_CONTEXT);
	}
	if (_LO_sock_client.connected()) {
#if (ARDUINO_CONN_ITF == 1) /* GSM */
		_LO_sock_client.beginWrite(true);
#endif
		ret = _LO_sock_client.write((uint8_t*)buf_ptr, len);
#if (ARDUINO_CONN_ITF == 1) /* GSM */
		_LO_sock_client.endWrite(true);
#endif
		if (ret != len) {
			LOTRACE_ERR("ERROR %d returnd by _LO_sock_client.write(len=%d)", ret, len);
		}
	}
	else {
		LOTRACE_ERR("ERROR - not connected to send % bytes", len);
		ret = NETW_ERR_NET_CONN_RESET;
	}
#endif
	LOTRACE_DBG1("send_data: OK");
	return 0;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int LO_sock_recv(socketHandle_t hdl, char* buf_ptr, int buf_len) {
	int ret;
	if ((hdl < 0) || (buf_ptr == NULL) || (buf_len <= 0)) {
		LOTRACE_ERR("Invalid parameters - hdl=%d buf_ptr=%p buf_len=%d", hdl, buf_ptr, buf_len);
		return -1;
	}
#if defined(ARDUINO_MEDIATEK)
	ret = (int) vm_recv(hdl, buf_ptr, buf_len, 0);
	if (ret < 0) {
#if 0
		if( sock_would_block( _netw_socket ) != 0 ) {
			LOTRACE_INF("(_netw_socket=%d len=%x) ret=%d x%x", _netw_socket, len, ret, MBEDTLS_ERR_SSL_WANT_READ);
			return( MBEDTLS_ERR_SSL_WANT_READ );
		}
		if( errno == EINTR ) {
			LOTRACE_INF("(ctx=%p _netw_socket=%d len=%x) ret=%d x%x", _netw_socket, len, ret, MBEDTLS_ERR_SSL_WANT_READ);
			return( MBEDTLS_ERR_SSL_WANT_READ );
		}
		if( errno == EPIPE || errno == ECONNRESET ) {
			_netw_bSockState |= 0x02;
			LOTRACE_ERR("(_netw_socket=%d len=%x) ret=%d errno=%d x%x",_netw_socket, len, ret, errno, MBEDTLS_ERR_NET_CONN_RESET);
			return( MBEDTLS_ERR_NET_CONN_RESET );
		}
#endif
		LOTRACE_ERR("(len=%d) ret=%d errno=%d - %s", buf_len, ret, errno, sock_error_name(ret));
		return -1;
	}

	buf_ptr[ret] = 0;

	if (ret == 0) {
		LOTRACE_ERR("(len=%d) ->  ret = 0 !!", buf_len);
		return 0;
	}
#endif
#if defined(ARDUINO_ITF)
	if (_LO_sock_client.connected()) {
		ret = _LO_sock_client.available();
		if (ret > 0) {
			ret = _LO_sock_client.read((uint8_t*)buf_ptr, buf_len);
		}
		else {
			ret = NETW_ERR_SSL_WANT_READ;
		}
	}
	else {
		ret = NETW_ERR_NET_CONN_RESET;
	}
#endif
	LOTRACE_DBG1("LO_sock_recv(len=%d)", ret);
	return ret;
}

/* --------------------------------------------------------------------------------- */
/*  */
extern "C" int LO_sock_read_line(socketHandle_t hdl, char* buf_ptr, int buf_len) {
	int len = 0;
	short retry = 0;
#if defined(ARDUINO_MEDIATEK)
	char cc;
#else
	int cc;
#endif

	if ((hdl < 0) || (buf_ptr == NULL) || (buf_len <= 0)) {
		LOTRACE_ERR("LO_sock_recv: Invalid parameters - hdl=%d buf_ptr=%p buf_len=%d", hdl, buf_ptr, buf_len);
		return -1;
	}

	cc = 0;
	while (1) {
#if defined(ARDUINO_MEDIATEK)
		int ret = (int) vm_recv(hdl, &cc, 1, 0);
		if (ret < 0) {
			LOTRACE_ERR("(1) (len=%d/%d) -> ERROR ret=%d errno=%d - %s", len, buf_len, ret, errno,
					sock_error_name(ret));
			if (sock_would_block(hdl) != 0) {
				LOTRACE_INF("(len=%d) retry=%d -> ERROR_WOULD_BLOCK (%d)", len, retry, ret);
				if (++retry < 6) {
					delay(200);
					continue;
				}
			}
			return ret;
		}
		if (ret == 0) {
			LOTRACE_ERR("(len=%d) ->  ret=0 -> Closed by peer  !!", len);
			return -1;
		}
#endif
#if defined(ARDUINO_ITF)
		if (_LO_sock_client.connected()) {
			cc = _LO_sock_client.read();
			if ((cc == -1) || (cc == 0)) {
				if (_LO_sock_client.connected()) {
					if (++retry < 6) {
						LOTRACE_INF("(len=%d) retry=%d -> ERROR_WOULD_BLOCK", len, retry);
						delay(200);
						continue;
					}
					LOTRACE_WARN("(len=%d) retry=%d -> ERROR_WOULD_BLOCK", len, retry);
					return NETW_ERR_SSL_WANT_READ;
				}
				LOTRACE_ERR("(len=%d) ->  ret=%d -> Closed by peer  !!", len, cc);
				return NETW_ERR_NET_CONN_RESET;
			}
		}
		else {
			LOTRACE_ERR("(len=%d) ->  ret=0 -> Closed by peer  !!", len);
			return NETW_ERR_NET_CONN_RESET;
		}
#endif
		if (cc == '\n') {
			LOTRACE_DBG1("(len=%d) -> EOL", len);
			break;
		}

		buf_ptr[len++] = cc;
		if (len >= buf_len) {
			LOTRACE_ERR("(len=%d) ->  TOO SHORT  !!", len);
			return -1;
		}
		retry = 0;
	}

	if ((cc == '\n') && (len >= 1) && (buf_ptr[len - 1] == '\r')) {
		len--;
		if (len == 0)
		LOTRACE_DBG1("->  BODY  !!");
	}
	buf_ptr[len] = 0;

	return len;
}

#endif /* LOC_FEATURE_LO_RESOURCES */
