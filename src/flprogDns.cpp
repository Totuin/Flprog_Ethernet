#include <Arduino.h>
#include "flprogDns.h"

void FLProgDNSClient::setSourse(FLProgAbstractTcpInterface *sourse)
{
	_sourse = sourse;
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
		if (_sourse->dns() == FLPROG_INADDR_NONE)
		{
			_wait_retries = 0;
			_status = FLPROG_NOT_REDY_STATUS;
			_errorCode = FLPROG_ETHERNET_DNS_INVALID_SERVER;
			return FLPROG_ERROR;
		}
		if (begin() != FLPROG_SUCCESS)
		{
			stop();
			_wait_retries = 0;
			return FLPROG_ERROR;
		}
		if (beginPacket() != FLPROG_SUCCESS)
		{
			stop();
			_wait_retries = 0;
			_errorCode = FLPROG_ETHERNET_UDP_SOKET_START_ERROR;
			return FLPROG_ERROR;
		}
		buildRequest(aHostname);
		if ((_sourse->sendUdpSoket(_sockindex)) != FLPROG_SUCCESS)
		{
			_wait_retries = 0;
			_errorCode = FLPROG_ETHERNET_UDP_SOKET_START_ERROR;
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
	stop();
	_wait_retries = 0;
	_status = FLPROG_READY_STATUS;
	return result;
}

uint8_t FLProgDNSClient::begin()
{
	if (!_sourse->isReady())
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR;
		return FLPROG_ERROR;
	}
	_sourse->closeSoket(_sockindex);
	_sockindex = _sourse->getUDPSoket(1024 + (millis() & 0xF));
	if (!(_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM))
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_SOKET_INDEX_ERROR;
		return FLPROG_ERROR;
	}
	_status = FLPROG_READY_STATUS;
	return FLPROG_SUCCESS;
}

uint16_t FLProgDNSClient::buildRequest(const char *aName)
{
	_iRequestId = millis();
	uint16_t twoByteBuffer;
	write((uint8_t *)&_iRequestId, sizeof(_iRequestId));
	twoByteBuffer = flprogW5100Htons((int32_t)(FLPROG_DNS_QUERY_FLAG | FLPROG_DNS_OPCODE_STANDARD_QUERY | FLPROG_DNS_RECURSION_DESIRED_FLAG));
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = flprogW5100Htons(1);
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = 0;
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
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
			write(&len, sizeof(len));
			write((uint8_t *)start, end - start);
		}
		start = end + 1;
	}
	len = 0;
	write(&len, sizeof(len));
	twoByteBuffer = flprogW5100Htons(FLPROG_DNS_TYPE_A);
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
	twoByteBuffer = flprogW5100Htons(FLPROG_DNS_CLASS_IN);
	write((uint8_t *)&twoByteBuffer, sizeof(twoByteBuffer));
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
		stop();
		_status = FLPROG_READY_STATUS;
		return FLPROG_WITE;
	}
	if (!(flprog::isTimer(_reqestStartTime, 50)))
	{
		return FLPROG_WITE;
	}
	_reqestStartTime = millis();
	if (parsePacket() <= 0)
	{
		return FLPROG_WITE;
	}
	union
	{
		uint8_t byte[FLPROG_DNS_HEADER_SIZE];
		uint16_t word[FLPROG_DNS_HEADER_SIZE / 2];
	} header;
	if (_remaining < FLPROG_DNS_HEADER_SIZE)
	{
		_errorCode = FLPROG_ETHERNET_DNS_TRUNCATED;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	read(header.byte, FLPROG_DNS_HEADER_SIZE);
	uint16_t header_flags = flprogW5100Htons(header.word[1]);
	if ((_iRequestId != (header.word[0])) ||
		((header_flags & FLPROG_DNS_QUERY_RESPONSE_MASK) != (uint16_t)FLPROG_DNS_RESPONSE_FLAG))
	{
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	if ((header_flags & FLPROG_DNS_TRUNCATION_FLAG) || (header_flags & FLPROG_DNS_RESP_MASK))
	{
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	uint16_t answerCount = flprogW5100Htons(header.word[3]);
	if (answerCount == 0)
	{
		_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
		_status = FLPROG_READY_STATUS;
		return FLPROG_ERROR;
	}
	for (uint16_t i = 0; i < flprogW5100Htons(header.word[2]); i++)
	{
		uint8_t len;
		do
		{
			read(&len, sizeof(len));
			if (len > 0)
			{
				read((uint8_t *)NULL, (size_t)len);
			}
		} while (len != 0);
		read((uint8_t *)NULL, 4);
	}
	for (uint16_t i = 0; i < answerCount; i++)
	{
		uint8_t len;
		do
		{
			read(&len, sizeof(len));
			if ((len & FLPROG_DNS_LABEL_COMPRESSION_MASK) == 0)
			{
				if (len > 0)
				{
					read((uint8_t *)NULL, len);
				}
			}
			else
			{
				read((uint8_t *)NULL, 1); // we don't care about the byte
				len = 0;
			}
		} while (len != 0);
		uint16_t answerType;
		uint16_t answerClass;
		read((uint8_t *)&answerType, sizeof(answerType));
		read((uint8_t *)&answerClass, sizeof(answerClass));
		read((uint8_t *)NULL, FLPROG_DNS_TTL_SIZE); // don't care about the returned bytes
		read((uint8_t *)&header_flags, sizeof(header_flags));
		if ((flprogW5100Htons(answerType) == FLPROG_DNS_TYPE_A) && (flprogW5100Htons(answerClass) == FLPROG_DNS_CLASS_IN))
		{
			if (flprogW5100Htons(header_flags) != 4)
			{
				_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
				_status = FLPROG_READY_STATUS;
				return FLPROG_ERROR;
			}
			read(aAddress, 4);
			return FLPROG_SUCCESS;
		}
		else
		{
			read((uint8_t *)NULL, flprogW5100Htons(header_flags));
		}
	}
	_errorCode = FLPROG_ETHERNET_DNS_INVALID_RESPONSE;
	_status = FLPROG_READY_STATUS;
	return FLPROG_ERROR;
}

void FLProgDNSClient::stop()
{
	_sourse->closeSoket(_sockindex);
	_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
}

int FLProgDNSClient::beginPacket()
{
	_offset = 0;
	_status = FLPROG_READY_STATUS;
	uint8_t buffer[4];
	flprog::ipToArray(_sourse->dns(), buffer);
	if (_sourse->startUdpSoket(_sockindex, buffer, FLPROG_DNS_PORT))
	{
		_errorCode = FLPROG_NOT_ERROR;
		return FLPROG_SUCCESS;
	}
	_errorCode = FLPROG_ETHERNET_UDP_SOKET_START_ERROR;
	return FLPROG_ERROR;
}

size_t FLProgDNSClient::write(const uint8_t *buffer, size_t size)
{
	uint16_t bytes_written = _sourse->bufferDataSoket(_sockindex, _offset, buffer, size);
	_offset += bytes_written;
	return bytes_written;
}

size_t FLProgDNSClient::write(uint8_t byte)
{
	return write(&byte, 1);
}

int FLProgDNSClient::parsePacket()
{
	while (_remaining)
	{
		read((uint8_t *)NULL, _remaining);
	}
	if (_sourse->availableSoket(_sockindex) > 0)
	{

		uint8_t tmpBuf[8];
		int ret = 0;
		ret = _sourse->recvSoket(_sockindex, tmpBuf, 8);
		if (ret > 0)
		{
			_remaining = tmpBuf[6];
			_remaining = (_remaining << 8) + tmpBuf[7];
			ret = _remaining;
		}
		return ret;
	}
	return 0;
}

int FLProgDNSClient::read()
{
	uint8_t byte;
	if ((_remaining > 0) && (_sourse->recvSoket(_sockindex, &byte, 1) > 0))
	{
		_remaining--;
		return byte;
	}
	return -1;
}

int FLProgDNSClient::read(uint8_t *buffer, size_t len)
{
	if (_remaining > 0)
	{
		int got;
		if (_remaining <= len)
		{
			got = _sourse->recvSoket(_sockindex, buffer, _remaining);
		}
		else
		{
			got = _sourse->recvSoket(_sockindex, buffer, len);
		}
		if (got > 0)
		{
			_remaining -= got;
			return got;
		}
	}
	return -1;
}
