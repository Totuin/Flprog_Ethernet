#include <Arduino.h>
#include "flprogDns.h"

void FLProgDNSClient::setUDP(FLProgEthernetUDP *udp)
{
	_udp = udp;
}

void FLProgDNSClient::begin(const IPAddress aDNSServer)
{
	iDNSServer = aDNSServer;
	iRequestId = 0;
}

int FLProgDNSClient::inet_aton(const char *address, IPAddress &result)
{
	uint16_t acc = 0; 
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

int FLProgDNSClient::getHostByName(const char *aHostname, uint8_t *aResult, uint16_t timeout)
{
	int ret = 0;
	IPAddress temp;
	temp[0] = aResult[0];
	temp[1] = aResult[1];
	temp[2] = aResult[2];
	temp[3] = aResult[3];
	if (inet_aton(aHostname, temp))
	{
		return 1;
	}
	if (iDNSServer == INADDR_NONE)
	{
		return FLPROG_DNS_INVALID_SERVER;
	}
	if (_udp->begin(1024 + (millis() & 0xF)) == 1)
	{
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
					ret = FLPROG_DNS_TIMED_OUT;
					while ((wait_retries < 3) && (ret == FLPROG_DNS_TIMED_OUT))
					{
						ret = ProcessResponse(timeout, aResult);
						wait_retries++;
					}
				}
			}
		}
		_udp->stop();
	}
	return ret;
}

uint16_t FLProgDNSClient::BuildRequest(const char *aName)
{
	iRequestId = millis();
	uint16_t twoByteBuffer;
	_udp->write((uint8_t *)&iRequestId, sizeof(iRequestId));
	twoByteBuffer = flprogW5100Htons((int32_t)(FLPROG_DNS_QUERY_FLAG | FLPROG_DNS_OPCODE_STANDARD_QUERY | FLPROG_DNS_RECURSION_DESIRED_FLAG));
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = flprogW5100Htons(1);
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
	twoByteBuffer = flprogW5100Htons(FLPROG_DNS_TYPE_A);
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = flprogW5100Htons(FLPROG_DNS_CLASS_IN);
	_udp->write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	return 1;
}

uint16_t FLProgDNSClient::ProcessResponse(uint16_t aTimeout, uint8_t *aAddress)
{
	uint32_t startTime = millis();
	while (_udp->parsePacket() <= 0)
	{
		if ((millis() - startTime) > aTimeout)
		{
			return FLPROG_DNS_TIMED_OUT;
		}
		delay(50);
	}
	union
	{
		uint8_t byte[FLPROG_DNS_HEADER_SIZE];
		uint16_t word[FLPROG_DNS_HEADER_SIZE / 2];
	} header;

	if ((iDNSServer != _udp->remoteIP()) || (_udp->remotePort() != FLPROG_DNS_PORT))
	{
		return FLPROG_DNS_INVALID_SERVER;
	}

	if (_udp->available() < FLPROG_DNS_HEADER_SIZE)
	{
		return FLPROG_DNS_TRUNCATED;
	}
	_udp->read(header.byte, FLPROG_DNS_HEADER_SIZE);
	uint16_t header_flags = flprogW5100Htons(header.word[1]);
	if ((iRequestId != (header.word[0])) ||
		((header_flags & FLPROG_DNS_QUERY_RESPONSE_MASK) != (uint16_t)FLPROG_DNS_RESPONSE_FLAG))
	{
		_udp->flush(); // FIXME
		return FLPROG_DNS_INVALID_RESPONSE;
	}
	if ((header_flags & FLPROG_DNS_TRUNCATION_FLAG) || (header_flags & FLPROG_DNS_RESP_MASK))
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
			if ((len & FLPROG_DNS_LABEL_COMPRESSION_MASK) == 0)
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
		_udp->read((uint8_t *)NULL, FLPROG_DNS_TTL_SIZE); // don't care about the returned bytes
		_udp->read((uint8_t *)&header_flags, sizeof(header_flags));
		if ((flprogW5100Htons(answerType) == FLPROG_DNS_TYPE_A) && (flprogW5100Htons(answerClass) == FLPROG_DNS_CLASS_IN))
		{
			if (flprogW5100Htons(header_flags) != 4)
			{
				_udp->flush(); // FIXME
				return -9;	   // INVALID_RESPONSE;
			}
			_udp->read(aAddress, 4);
			return FLPROG_DNS_SUCCESS;
		}
		else
		{
			_udp->read((uint8_t *)NULL, flprogW5100Htons(header_flags));
		}
	}
	_udp->flush(); // FIXME
	return -10;
}
