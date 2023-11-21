
#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "flprogEthernetUdp.h"

#define FLPROG_DNS_SOCKET_NONE 255
#define FLPROG_DNS_UDP_HEADER_SIZE 8
#define FLPROG_DNS_HEADER_SIZE 12
#define FLPROG_DNS_TTL_SIZE 4
#define FLPROG_DNS_QUERY_FLAG (0)
#define FLPROG_DNS_RESPONSE_FLAG (1 << 15)
#define FLPROG_DNS_QUERY_RESPONSE_MASK (1 << 15)
#define FLPROG_DNS_OPCODE_STANDARD_QUERY (0)
#define FLPROG_DNS_OPCODE_INVERSE_QUERY (1 << 11)
#define FLPROG_DNS_OPCODE_STATUS_REQUEST (2 << 11)
#define FLPROG_DNS_OPCODE_MASK (15 << 11)
#define FLPROG_DNS_AUTHORITATIVE_FLAG (1 << 10)
#define FLPROG_DNS_TRUNCATION_FLAG (1 << 9)
#define FLPROG_DNS_RECURSION_DESIRED_FLAG (1 << 8)
#define FLPROG_DNS_RECURSION_AVAILABLE_FLAG (1 << 7)
#define FLPROG_DNS_RESP_NO_ERROR (0)
#define FLPROG_DNS_RESP_FORMAT_ERROR (1)
#define FLPROG_DNS_RESP_SERVER_FAILURE (2)
#define FLPROG_DNS_RESP_NAME_ERROR (3)
#define FLPROG_DNS_RESP_NOT_IMPLEMENTED (4)
#define FLPROG_DNS_RESP_REFUSED (5)
#define FLPROG_DNS_RESP_MASK (15)
#define FLPROG_DNS_TYPE_A (0x0001)
#define FLPROG_DNS_CLASS_IN (0x0001)
#define FLPROG_DNS_LABEL_COMPRESSION_MASK (0xC0)
#define FLPROG_DNS_PORT 53



class FLProgEthernetUDP;

class FLProgDNSClient
{
public:
	void setUDP(FLProgEthernetUDP *udp);
	void begin(const IPAddress aDNSServer);
	int getHostByName(const char *aHostname, uint8_t *aResult, uint16_t timeout = 5000);
	FLProgEthernetUDP *udp() { return _udp; }

protected:
	uint16_t BuildRequest(const char *aName);
	uint16_t ProcessResponse(uint16_t aTimeout, uint8_t *aAddress);
	FLProgEthernetUDP *_udp;
	IPAddress iDNSServer;
	uint16_t iRequestId;
	uint8_t status = FLPROG_NOT_REDY_STATUS;
	uint8_t _wait_retries = 0;
	uint32_t _startTime;
	uint32_t _reqestStartTime;
};
