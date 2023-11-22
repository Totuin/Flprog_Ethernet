#include <Arduino.h>
#include "flprogDns.h"

void FLProgDNSClient::setUDP(FLProgEthernetUDP *udp)
{
	_udp = udp;
}

void FLProgDNSClient::begin(const IPAddress aDNSServer)
{
	if (_iDNSServer == FLPROG_INADDR_NONE)
	{
		if (aDNSServer != FLPROG_INADDR_NONE)
		{
			_status = FLPROG_READY_STATUS;
		}
	}
	_iDNSServer = aDNSServer;
}

int FLProgDNSClient::getHostByName(const char *aHostname, uint8_t *aResult, uint16_t timeout)
{

	if (_status == FLPROG_NOT_REDY_STATUS)
	{
		_errorCode = FLPROG_ETHERNET_DNS_NOT_READY_ERROR;
		return FLPROG_ERROR;
	}

	if (_status == FLPROG_READY_STATUS)
	{
		IPAddress temp(aResult[0], aResult[1], aResult[2], aResult[3]);
		if (flprog::inet_aton(aHostname, temp))
		{
			aResult[0] = temp[0];
			aResult[1] = temp[1];
			aResult[2] = temp[2];
			aResult[3] = temp[3];
			_wait_retries = 0;
			_errorCode = FLPROG_NOT_ERROR;
			return FLPROG_SUCCESS;
		}
		if (_iDNSServer == FLPROG_INADDR_NONE)
		{
			_wait_retries = 0;
			_status = FLPROG_NOT_REDY_STATUS;
			_errorCode = FLPROG_ETHERNET_DNS_INVALID_SERVER;
			return FLPROG_ERROR;
		}
		if ((_udp->begin(1024 + (millis() & 0xF))) != FLPROG_SUCCESS)
		{
			_udp->stop();
			_wait_retries = 0;
			_errorCode = _udp->getError();
			return FLPROG_ERROR;
		}
		if ((_udp->beginPacket(_iDNSServer, FLPROG_DNS_PORT)) != FLPROG_SUCCESS)
		{
			_udp->stop();
			_wait_retries = 0;
			_errorCode = _udp->getError();
			return FLPROG_ERROR;
		}
		buildRequest(aHostname);
		if ((_udp->endPacket()) != FLPROG_SUCCESS)
		{
			_udp->stop();
			_wait_retries = 0;
			_errorCode = _udp->getError();
			return FLPROG_ERROR;
		}
		_startTime = millis();
		_reqestStartTime = millis();
		_status = FLPROG_WAIT_ETHERNET_UDP_STATUS;
		_errorCode = FLPROG_NOT_ERROR;
		return FLPROG_WITE;
	}
	uint8_t result = processResponse(timeout, aResult);
	if (result == FLPROG_WITE)
	{
		return FLPROG_WITE;
	}
	_udp->stop();
	_wait_retries = 0;
	_status = FLPROG_READY_STATUS;
	return result;
}

uint16_t FLProgDNSClient::buildRequest(const char *aName)
{
	_iRequestId = millis();
	uint16_t twoByteBuffer;
	_udp->write((uint8_t *)&_iRequestId, sizeof(_iRequestId));
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
	return FLPROG_SUCCESS;
}

uint16_t FLProgDNSClient::processResponse(uint16_t aTimeout, uint8_t *aAddress)
{

	if (flprog::isTimer(_startTime, aTimeout))
	{
		if (_wait_retries > 2)
		{
			_wait_retries = 0;
			_status = FLPROG_READY_STATUS;
			_errorCode = FLPROG_ETHERNET_UDP_TIMEOUT_ERROR;
			return FLPROG_ERROR;
		}
		_wait_retries++;
		_startTime = millis();
		_udp->stop();
		_status = FLPROG_READY_STATUS;
		return FLPROG_WITE;
	}
	if (!(flprog::isTimer(_reqestStartTime, 50)))
	{
		return FLPROG_WITE;
	}
	_reqestStartTime = millis();
	if (_udp->parsePacket() <= 0)
	{
		return FLPROG_WITE;
	}
	union
	{
		uint8_t byte[FLPROG_DNS_HEADER_SIZE];
		uint16_t word[FLPROG_DNS_HEADER_SIZE / 2];
	} header;

	if ((_iDNSServer != _udp->remoteIP()) || (_udp->remotePort() != FLPROG_DNS_PORT))
	{
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_SERVER;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	if (_udp->available() < FLPROG_DNS_HEADER_SIZE)
	{
		_errorCode = FLPROG_ETHERNET_DNS_TRUNCATED;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	_udp->read(header.byte, FLPROG_DNS_HEADER_SIZE);
	uint16_t header_flags = flprogW5100Htons(header.word[1]);
	if ((_iRequestId != (header.word[0])) ||
		((header_flags & FLPROG_DNS_QUERY_RESPONSE_MASK) != (uint16_t)FLPROG_DNS_RESPONSE_FLAG))
	{
		_udp->flush(); // FIXME
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	if ((header_flags & FLPROG_DNS_TRUNCATION_FLAG) || (header_flags & FLPROG_DNS_RESP_MASK))
	{
		_udp->flush(); // FIXME
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	uint16_t answerCount = flprogW5100Htons(header.word[3]);
	if (answerCount == 0)
	{
		_udp->flush(); // FIXME
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
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
				_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
				_status = FLPROG_READY_STATUS;
				return FLPROG_ERROR;
			}
			_udp->read(aAddress, 4);
			return FLPROG_SUCCESS;
		}
		else
		{
			_udp->read((uint8_t *)NULL, flprogW5100Htons(header_flags));
		}
	}
	_udp->flush(); // FIXME
	_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
	_status = FLPROG_READY_STATUS;
	return FLPROG_ERROR;
}
