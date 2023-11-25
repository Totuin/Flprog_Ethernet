#pragma once
#include <Arduino.h>
#include "flprogEthernetUdp.h"

/* DHCP state machine. */
#define FLPROG_STATE_DHCP_START 0
#define FLPROG_STATE_DHCP_DISCOVER 1

#define FLPROG_STATE_DHCP_REQUEST 3
#define FLPROG_STATE_DHCP_LEASED 4
#define FLPROG_STATE_DHCP_REREQUEST 5
#define FLPROG_STATE_DHCP_RELEASE 6

#define FLPROG_DHCP_FLAGSBROADCAST 0x8000

/* UDP port numbers for DHCP */
#define FLPROG_DHCP_SERVER_PORT 67 /* from server to client */
#define FLPROG_DHCP_CLIENT_PORT 68 /* from client to server */

/* DHCP message OP code */
#define FLPROG_DHCP_BOOTREQUEST 1
#define FLPROG_DHCP_BOOTREPLY 2

#define FLPROG_DHCP_HTYPE10MB 1
#define FLPROG_DHCP_HTYPE100MB 2

#define FLPROG_DHCP_HLENETHERNET 6
#define FLPROG_DHCP_HOPS 0
#define FLPROG_DHCP_SECS 0

#define FLPROG_MAGIC_COOKIE 0x63825363
#define FLPROG_MAX_DHCP_OPT 16

#define FLPROG_HOST_NAME "WIZnet"
#define FLPROG_DEFAULT_LEASE 900 // default lease time in seconds

#define FLPROG_DHCP_CHECK_NONE 0
#define FLPROG_DHCP_CHECK_RENEW_FAIL 1
#define FLPROG_DHCP_CHECK_RENEW_OK 2
#define FLPROG_DHCP_CHECK_REBIND_FAIL 3
#define FLPROG_DHCP_CHECK_REBIND_OK 4

// типы сообщений при ответе сервера
#define FLPROG_DHCP_DISCOVER 1
#define FLPROG_DHCP_OFFER 2
#define FLPROG_DHCP_REQUEST 3
#define FLPROG_DHCP_DECLINE 4
#define FLPROG_DHCP_ACK 5
#define FLPROG_DHCP_NAK 6
#define FLPROG_DHCP_RELEASE 7
#define FLPROG_DHCP_INFORM 8
#define FLPROG_DHCP_WITE_CHECK_REQEST_MESSAGE_TYPE 254
#define FLPROG_DHCP_TIMEOUT_MESSAGE_TYPE 255
#define FLPROG_DHCP_ERROR_ID_MESSAGE_TYPE 253

enum
{
	padOption = 0,
	subnetMask = 1,
	timerOffset = 2,
	routersOnSubnet = 3,
	/* timeServer		=	4,
	nameServer		=	5,*/
	dns = 6,
	/*logServer		=	7,
	cookieServer		=	8,
	lprServer		=	9,
	impressServer		=	10,
	resourceLocationServer	=	11,*/
	hostName = 12,
	/*bootFileSize		=	13,
	meritDumpFile		=	14,*/
	domainName = 15,
	/*swapServer		=	16,
	rootPath		=	17,
	extentionsPath		=	18,
	IPforwarding		=	19,
	nonLocalSourceRouting	=	20,
	policyFilter		=	21,
	maxDgramReasmSize	=	22,
	defaultIPTTL		=	23,
	pathMTUagingTimeout	=	24,
	pathMTUplateauTable	=	25,
	ifMTU			=	26,
	allSubnetsLocal		=	27,
	broadcastAddr		=	28,
	performMaskDiscovery	=	29,
	maskSupplier		=	30,
	performRouterDiscovery	=	31,
	routerSolicitationAddr	=	32,
	staticRoute		=	33,
	trailerEncapsulation	=	34,
	arpCacheTimeout		=	35,
	ethernetEncapsulation	=	36,
	tcpDefaultTTL		=	37,
	tcpKeepaliveInterval	=	38,
	tcpKeepaliveGarbage	=	39,
	nisDomainName		=	40,
	nisServers		=	41,
	ntpServers		=	42,
	vendorSpecificInfo	=	43,
	netBIOSnameServer	=	44,
	netBIOSdgramDistServer	=	45,
	netBIOSnodeType		=	46,
	netBIOSscope		=	47,
	xFontServer		=	48,
	xDisplayManager		=	49,*/
	dhcpRequestedIPaddr = 50,
	dhcpIPaddrLeaseTime = 51,
	/*dhcpOptionOverload	=	52,*/
	dhcpMessageType = 53,
	dhcpServerIdentifier = 54,
	dhcpParamRequest = 55,
	/*dhcpMsg			=	56,
	dhcpMaxMsgSize		=	57,*/
	dhcpT1value = 58,
	dhcpT2value = 59,
	/*dhcpClassIdentifier	=	60,*/
	dhcpClientIdentifier = 61,
	endOption = 255
};

typedef struct _FLPROG_RIP_MSG_FIXED
{
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint8_t ciaddr[4];
	uint8_t yiaddr[4];
	uint8_t siaddr[4];
	uint8_t giaddr[4];
	uint8_t chaddr[6];
} FLPROG_RIP_MSG_FIXED;

class FLProgDhcpClass
{
public:
	IPAddress getLocalIp();
	IPAddress getSubnetMask();
	IPAddress getGatewayIp();
	IPAddress getDhcpServerIp();
	IPAddress getDnsServerIp();
	int beginWithDHCP(uint8_t *, unsigned long timeout = 20000, unsigned long responseTimeout = 4000);
	int checkLease();
	uint8_t getStatus() { return _status; };
	uint8_t getError() { return _errorCode; }

private:
	uint32_t _dhcpInitialTransactionId;
	uint32_t _dhcpTransactionId;
	uint8_t _dhcpMacAddr[6];
	FLProgEthernetUDP _udp;
	uint8_t _errorCode = FLPROG_NOT_ERROR;
	uint8_t _status = FLPROG_NOT_REDY_STATUS;
#ifdef __arm__
	uint8_t _dhcpLocalIp[4] __attribute__((aligned(4)));
	uint8_t _dhcpSubnetMask[4] __attribute__((aligned(4)));
	uint8_t _dhcpGatewayIp[4] __attribute__((aligned(4)));
	uint8_t _dhcpDhcpServerIp[4] __attribute__((aligned(4)));
	uint8_t _dhcpDnsServerIp[4] __attribute__((aligned(4)));
#else
	uint8_t _dhcpLocalIp[4];
	uint8_t _dhcpSubnetMask[4];
	uint8_t _dhcpGatewayIp[4];
	uint8_t _dhcpDhcpServerIp[4];
	uint8_t _dhcpDnsServerIp[4];
#endif
	uint32_t _dhcpLeaseTime;
	uint32_t _dhcpT1, _dhcpT2;
	uint32_t _renewInSec;
	uint32_t _rebindInSec;
	uint32_t _timeout;
	uint32_t _responseTimeout;
	uint32_t _lastCheckLeaseMillis;
	uint8_t _dhcp_state;
	uint32_t _startDhcpReqestTime;
	uint32_t _lastCheckDhcpReqestTime;
	uint32_t _respId;

	int request_DHCP_lease();
	void reset_DHCP_lease();

	uint8_t cheskStateMashine();
	void send_DHCP_MESSAGE(uint8_t, uint16_t);
	void printByte(char *, uint8_t);

	uint8_t parseDHCPResponse();
};
