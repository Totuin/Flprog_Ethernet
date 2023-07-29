// DHCP Library v0.3 - April 25, 2009
// Author: Jordan Terrell - blog.jordanterrell.com

#ifndef FlprogDhcp_h
#define FlprogDhcp_h

/* DHCP state machine. */
#define FLPROG_STATE_DHCP_START	0
#define	FLPROG_STATE_DHCP_DISCOVER	1
#define	FLPROG_STATE_DHCP_REQUEST	2
#define	FLPROG_STATE_DHCP_LEASED	3
#define	FLPROG_STATE_DHCP_REREQUEST	4
#define	FLPROG_STATE_DHCP_RELEASE	5

#define FLPROG_DHCP_FLAGSBROADCAST	0x8000

/* UDP port numbers for DHCP */
#define	FLPROG_DHCP_SERVER_PORT	67	/* from server to client */
#define FLPROG_DHCP_CLIENT_PORT	68	/* from client to server */

/* DHCP message OP code */
#define FLPROG_DHCP_BOOTREQUEST	1
#define FLPROG_DHCP_BOOTREPLY		2

/* DHCP message type */
#define	FLPROG_DHCP_DISCOVER		1
#define FLPROG_DHCP_OFFER		2
#define	FLPROG_DHCP_REQUEST		3
#define	FLPROG_DHCP_DECLINE		4
#define	FLPROG_DHCP_ACK		5
#define FLPROG_DHCP_NAK		6
#define	FLPROG_DHCP_RELEASE		7
#define FLPROG_DHCP_INFORM		8

#define FLPROG_DHCP_HTYPE10MB		1
#define FLPROG_DHCP_HTYPE100MB		2

#define FLPROG_DHCP_HLENETHERNET	6
#define FLPROG_DHCP_HOPS		0
#define FLPROG_DHCP_SECS		0

#define FLPROG_MAGIC_COOKIE		0x63825363
#define FLPROG_MAX_DHCP_OPT		16

#define FLPROG_HOST_NAME "WIZnet"
#define FLPROG_DEFAULT_LEASE	(900) //default lease time in seconds

#define FLPROG_DHCP_CHECK_NONE         (0)
#define FLPROG_DHCP_CHECK_RENEW_FAIL   (1)
#define FLPROG_DHCP_CHECK_RENEW_OK     (2)
#define FLPROG_DHCP_CHECK_REBIND_FAIL  (3)
#define FLPROG_DHCP_CHECK_REBIND_OK    (4)

enum
{
	padOption		=	0,
	subnetMask		=	1,
	timerOffset		=	2,
	routersOnSubnet		=	3,
	/* timeServer		=	4,
	nameServer		=	5,*/
	dns			=	6,
	/*logServer		=	7,
	cookieServer		=	8,
	lprServer		=	9,
	impressServer		=	10,
	resourceLocationServer	=	11,*/
	hostName		=	12,
	/*bootFileSize		=	13,
	meritDumpFile		=	14,*/
	domainName		=	15,
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
	dhcpRequestedIPaddr	=	50,
	dhcpIPaddrLeaseTime	=	51,
	/*dhcpOptionOverload	=	52,*/
	dhcpMessageType		=	53,
	dhcpServerIdentifier	=	54,
	dhcpParamRequest	=	55,
	/*dhcpMsg			=	56,
	dhcpMaxMsgSize		=	57,*/
	dhcpT1value		=	58,
	dhcpT2value		=	59,
	/*dhcpClassIdentifier	=	60,*/
	dhcpClientIdentifier	=	61,
	endOption		=	255
};

typedef struct _FLPROG_RIP_MSG_FIXED
{
	uint8_t  op;
	uint8_t  htype;
	uint8_t  hlen;
	uint8_t  hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint8_t  ciaddr[4];
	uint8_t  yiaddr[4];
	uint8_t  siaddr[4];
	uint8_t  giaddr[4];
	uint8_t  chaddr[6];
} FLPROG_RIP_MSG_FIXED;

#endif
