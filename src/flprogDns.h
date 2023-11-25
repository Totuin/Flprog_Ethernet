
#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"

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

class FLProgDNSClient
{
public:
	void setSourse(FLProgAbstractTcpInterface *sourse);
	int getHostByName(const char *aHostname, uint8_t *aResult, uint16_t timeout = 5000);
	uint8_t getStatus() { return _status; };
	uint8_t getError() { return _errorCode; }

protected:
	uint8_t begin();
	int beginPacket();
	void stop();
	uint16_t buildRequest(const char *aName);
	uint16_t processResponse(uint16_t aTimeout, uint8_t *aAddress);
	size_t write(const uint8_t *buffer, size_t size);
	size_t write(uint8_t byte);
	int parsePacket();
	int read();
	int read(uint8_t *buffer, size_t len);
	uint16_t _offset;
	IPAddress _iDNSServer;
	uint16_t _iRequestId;
	uint8_t _status = FLPROG_NOT_REDY_STATUS;
	uint8_t _errorCode = FLPROG_ETHERNET_DNS_NOT_READY_ERROR;
	uint8_t _wait_retries = 0;
	uint32_t _startTime;
	uint32_t _reqestStartTime;
	uint16_t _remaining;
	FLProgAbstractTcpInterface *_sourse;
	uint8_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
};
