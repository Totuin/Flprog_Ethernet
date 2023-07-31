#include <Arduino.h>
#include "flprogDns.h"

#define FLPROG_SOCKET_NONE 255
#define FLPROG_UDP_HEADER_SIZE 8
#define FLPROG_DNS_HEADER_SIZE 12
#define FLPROG_TTL_SIZE 4
#define FLPROG_QUERY_FLAG (0)
#define FLPROG_RESPONSE_FLAG (1 << 15)
#define FLPROG_QUERY_RESPONSE_MASK (1 << 15)
#define FLPROG_OPCODE_STANDARD_QUERY (0)
#define FLPROG_OPCODE_INVERSE_QUERY (1 << 11)
#define FLPROG_OPCODE_STATUS_REQUEST (2 << 11)
#define FLPROG_OPCODE_MASK (15 << 11)
#define FLPROG_AUTHORITATIVE_FLAG (1 << 10)
#define FLPROG_TRUNCATION_FLAG (1 << 9)
#define FLPROG_RECURSION_DESIRED_FLAG (1 << 8)
#define FLPROG_RECURSION_AVAILABLE_FLAG (1 << 7)
#define FLPROG_RESP_NO_ERROR (0)
#define FLPROG_RESP_FORMAT_ERROR (1)
#define FLPROG_RESP_SERVER_FAILURE (2)
#define FLPROG_RESP_NAME_ERROR (3)
#define FLPROG_RESP_NOT_IMPLEMENTED (4)
#define FLPROG_RESP_REFUSED (5)
#define FLPROG_RESP_MASK (15)
#define FLPROG_TYPE_A (0x0001)
#define FLPROG_CLASS_IN (0x0001)
#define FLPROG_LABEL_COMPRESSION_MASK (0xC0)
#define FLPROG_DNS_PORT 53

#define FLPROG_SUCCESS 1
#define FLPROG_TIMED_OUT -1
#define FLPROG_INVALID_SERVER -2
#define FLPROG_TRUNCATED -3
#define FLPROG_INVALID_RESPONSE -4

void FlprogDNSClient::setUDP(FlprogEthernetUDP *udp)
{
	_udp = udp;
}

void FlprogDNSClient::begin(const IPAddress &aDNSServer)
{
	iDNSServer = aDNSServer;
	iRequestId = 0;
}

int FlprogDNSClient::inet_aton(const char *address, IPAddress &result)
{
	uint16_t acc = 0; // Accumulator
	uint8_t dots = 0;
	while (*address)
	{
		char c = *address++;
		if (c >= '0' && c <= '9')
		{
			acc = acc * 10 + (c - '0');
			if (acc > 255)
			{
				return 0;
			}
		}
		else if (c == '.')
		{
			if (dots == 3)
			{
				return 0;
			}
			result[dots++] = acc;
			acc = 0;
		}
		else
		{
			return 0;
		}
	}
	if (dots != 3)
	{
		return 0;
	}
	result[3] = acc;
	return 1;
}

int FlprogDNSClient::getHostByName(const char *aHostname, IPAddress &aResult, uint16_t timeout)
{
	int ret = 0;
	if (inet_aton(aHostname, aResult))
	{
		return 1;
	}
	if (iDNSServer == INADDR_NONE)
	{
		return FLPROG_INVALID_SERVER;
	}
	if (_udp->begin(1024 + (millis() & 0xF)) == 1)
	{
		int retries = 0;
		ret = _udp->beginPacket(iDNSServer, FLPROG_DNS_PORT);
		if (ret != 0)
		{
			ret = BuildRequest(aHostname);
			if (ret != 0)
			{
				ret = _udp->endPacket();
				if (ret != 0)
				{
					int wait_retries = 0;
					ret = FLPROG_TIMED_OUT;
					while ((wait_retries < 3) && (ret == FLPROG_TIMED_OUT))
					{
						ret = ProcessResponse(timeout, aResult);
						wait_retries++;
					}
				}
			}
		}
		retries++;
		_udp->stop();
	}
	return ret;
}

uint16_t FlprogDNSClient::BuildRequest(const char *aName)
{
	iRequestId = millis(); // generate a random ID
	uint16_t twoByteBuffer;
	// FIXME We should also check that there's enough space available to write to, rather
	// FIXME than assume there's enough space (as the code does at present)
	_udp->write((uint8_t *)&iRequestId, sizeof(iRequestId));
	twoByteBuffer = flprogW5100Htons((int32_t)(FLPROG_QUERY_FLAG | FLPROG_OPCODE_STANDARD_QUERY | FLPROG_RECURSION_DESIRED_FLAG));
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = flprogW5100Htons(1); // One question record
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = 0;
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	const char *start = aName;
	const char *end = start;
	uint8_t len;
	while (*end)
	{
		end = start;
		while (*end && (*end != '.'))
		{
			end++;
		}
		if (end - start > 0)
		{
			len = end - start;
			_udp->write(&len, sizeof(len));
			_udp->write((uint8_t *)start, end - start);
		}
		start = end + 1;
	}
	len = 0;
	_udp->write(&len, sizeof(len));
	twoByteBuffer = flprogW5100Htons(FLPROG_TYPE_A);
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = flprogW5100Htons(FLPROG_CLASS_IN); // Internet class of question
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	return 1;
}

uint16_t FlprogDNSClient::ProcessResponse(uint16_t aTimeout, IPAddress &aAddress)
{
	uint32_t startTime = millis();
	while (_udp->parsePacket() <= 0)
	{
		if ((millis() - startTime) > aTimeout)
		{
			return FLPROG_TIMED_OUT;
		}
		delay(50);
	}
	union
	{
		uint8_t byte[FLPROG_DNS_HEADER_SIZE]; // Enough space to reuse for the DNS header
		uint16_t word[FLPROG_DNS_HEADER_SIZE / 2];
	} header;
	if ((iDNSServer != _udp->remoteIP()) || (_udp->remotePort() != FLPROG_DNS_PORT))
	{
		return FLPROG_INVALID_SERVER;
	}
	if (_udp->available() < FLPROG_DNS_HEADER_SIZE)
	{
		return FLPROG_TRUNCATED;
	}
	_udp->read(header.byte, FLPROG_DNS_HEADER_SIZE);
	uint16_t header_flags = flprogW5100Htons(header.word[1]);
	if ((iRequestId != (header.word[0])) ||
		((header_flags & FLPROG_QUERY_RESPONSE_MASK) != (uint16_t)FLPROG_RESPONSE_FLAG))
	{
		_udp->flush(); // FIXME
		return FLPROG_INVALID_RESPONSE;
	}
	if ((header_flags & FLPROG_TRUNCATION_FLAG) || (header_flags & FLPROG_RESP_MASK))
	{
		_udp->flush(); // FIXME
		return -5;	   // INVALID_RESPONSE;
	}
	uint16_t answerCount = flprogW5100Htons(header.word[3]);
	if (answerCount == 0)
	{
		_udp->flush(); // FIXME
		return -6;	   // INVALID_RESPONSE;
	}
	for (uint16_t i = 0; i < flprogW5100Htons(header.word[2]); i++)
	{
		uint8_t len;
		do
		{
			_udp->read(&len, sizeof(len));
			if (len > 0)
			{
				_udp->read((uint8_t *)NULL, (size_t)len);
			}
		} while (len != 0);
		_udp->read((uint8_t *)NULL, 4);
	}
	for (uint16_t i = 0; i < answerCount; i++)
	{
		uint8_t len;
		do
		{
			_udp->read(&len, sizeof(len));
			if ((len & FLPROG_LABEL_COMPRESSION_MASK) == 0)
			{
				if (len > 0)
				{
					_udp->read((uint8_t *)NULL, len);
				}
			}
			else
			{
				_udp->read((uint8_t *)NULL, 1); // we don't care about the byte
				len = 0;
			}
		} while (len != 0);
		uint16_t answerType;
		uint16_t answerClass;
		_udp->read((uint8_t *)&answerType, sizeof(answerType));
		_udp->read((uint8_t *)&answerClass, sizeof(answerClass));
		_udp->read((uint8_t *)NULL, FLPROG_TTL_SIZE); // don't care about the returned bytes
		_udp->read((uint8_t *)&header_flags, sizeof(header_flags));
		if ((flprogW5100Htons(answerType) == FLPROG_TYPE_A) && (flprogW5100Htons(answerClass) == FLPROG_CLASS_IN))
		{
			if (flprogW5100Htons(header_flags) != 4)
			{
				_udp->flush(); // FIXME
				return -9;	   // INVALID_RESPONSE;
			}
			uint8_t flporgConvertTmpBytes[4];
			flporgConvertTmpBytes[0] = aAddress[0];
			flporgConvertTmpBytes[1] = aAddress[1];
			flporgConvertTmpBytes[2] = aAddress[2];
			flporgConvertTmpBytes[3] = aAddress[3];
			_udp->read(flporgConvertTmpBytes, 4);
			aAddress[0] = flporgConvertTmpBytes[0];
			aAddress[1] = flporgConvertTmpBytes[1];
			aAddress[2] = flporgConvertTmpBytes[2];
			aAddress[3] = flporgConvertTmpBytes[3];
			return FLPROG_SUCCESS;
		}
		else
		{
			_udp->read((uint8_t *)NULL, flprogW5100Htons(header_flags));
		}
	}
	_udp->flush(); // FIXME
	return -10;
}
