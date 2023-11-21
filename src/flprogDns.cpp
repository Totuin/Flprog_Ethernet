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
	status = FLPROG_READY_STATUS;
}

int FLProgDNSClient::getHostByName(const char *aHostname, uint8_t *aResult, uint16_t timeout)
{

	if (status == FLPROG_NOT_REDY_STATUS)
	{
		return status;
	}
	if (status == FLPROG_READY_STATUS)
	{

		Serial.println("Start1");

		IPAddress temp;
		temp[0] = aResult[0];
		temp[1] = aResult[1];
		temp[2] = aResult[2];
		temp[3] = aResult[3];

		if (flprog::inet_aton(aHostname, temp))
		{
			aResult[0] = temp[0];
			aResult[1] = temp[1];
			aResult[2] = temp[2];
			aResult[3] = temp[3];
			_wait_retries = 0;
			return FLPROG_SUCCESS;
		}
		Serial.println("Start2");

		if (iDNSServer == INADDR_NONE)
		{
			_wait_retries = 0;
			status = FLPROG_NOT_REDY_STATUS;
			return FLPROG_DNS_INVALID_SERVER;
		}

		Serial.println("Start3");

		if (!(_udp->begin(1024 + (millis() & 0xF))))
		{
			_udp->stop();
			_wait_retries = 0;
			return FLPROG_DNS_INVALID_UDP;
		}

		Serial.println("Start4");

		if ((_udp->beginPacket(iDNSServer, FLPROG_DNS_PORT)) == FLPROG_EHERNET_ERROR)
		{
			_udp->stop();
			_wait_retries = 0;
			return FLPROG_DNS_INVALID_UDP;
		}

		Serial.println("Start5");

		BuildRequest(aHostname);

		Serial.println("Start6");

		if (!_udp->endPacket())
		{
			_udp->stop();
			_wait_retries = 0;
			return FLPROG_DNS_INVALID_UDP;
		}

		Serial.println("Start7");

		_startTime = millis();
		_reqestStartTime = millis();
		status = FLPROG_WAIT_STATUS;
		return status;
	}
	uint8_t ret = ProcessResponse(timeout, aResult);
	if (ret == FLPROG_WAIT_STATUS)
	{
		return FLPROG_WAIT_STATUS;
		Serial.println("WAIT1");
	}
	if (ret == FLPROG_TIMED_OUT)
	{
		if (_wait_retries > 2)
		{
			_udp->stop();
			_wait_retries = 0;
			status = FLPROG_READY_STATUS;
			return FLPROG_TIMED_OUT;
		}
		_wait_retries++;
		_startTime = millis();
		_udp->stop();
		status = FLPROG_READY_STATUS;
		return FLPROG_WAIT_STATUS;
	}
	Serial.println("WAIT2");
	_udp->stop();
	_wait_retries = 0;
	status = FLPROG_READY_STATUS;
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
	return FLPROG_SUCCESS;
}

uint16_t FLProgDNSClient::ProcessResponse(uint16_t aTimeout, uint8_t *aAddress)
{

	if (flprog::isTimer(_startTime, aTimeout))
	{
		return FLPROG_TIMED_OUT;
	}
	if (!(flprog::isTimer(_reqestStartTime, 50)))
	{
		return FLPROG_WAIT_STATUS;
	}
	_reqestStartTime = millis();
	if (_udp->parsePacket() <= 0)
	{
		return FLPROG_WAIT_STATUS;
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
