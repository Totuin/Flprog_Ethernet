
#include "flprogEthernetUdp.h"

FLProgEthernetUDP::FLProgEthernetUDP(FlprogAbstractEthernet *sourse)
{
	_sourse = sourse;
	_dns = new FLProgDNSClient;
	_dns->setUDP(new FLProgEthernetUDP);
	_dns->udp()->setSourse(sourse);
}

void FLProgEthernetUDP::setSourse(FlprogAbstractEthernet *sourse)
{
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	_sourse = sourse;
}

uint8_t FLProgEthernetUDP::begin(uint16_t port)
{

	if (_port == port)
	{
		if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
		{
			return;
		}
	}
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_sourse->hardware()->socketClose(sockindex);
	}
	sockindex = _sourse->hardware()->socketBegin(FLPROG_SN_MR_UDP, port);
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		return 0;
	}
	_port = port;
	_remaining = 0;
	return 1;
}

int FLProgEthernetUDP::available()
{
	return _remaining;
}

void FLProgEthernetUDP::stop()
{
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_sourse->hardware()->socketClose(sockindex);
		sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	}
}

int FLProgEthernetUDP::beginPacket(const char *host, uint16_t port)
{
	uint8_t remote_addr[4] = {0, 0, 0, 0};
	_dns->begin(_sourse->dns());
	uint8_t ret = _dns->getHostByName(host, remote_addr);
	if (ret == FLPROG_WAIT_STATUS)
	{
		status = FLPROG_WAIT_STATUS;
		return status;
	}
	if (ret == FLPROG_SUCCESS)
	{
		return beginPacket(IPAddress(remote_addr[0], remote_addr[1], remote_addr[2], remote_addr[3]), port);
	}
	status = FLPROG_READY_STATUS;
	return ret;
}

int FLProgEthernetUDP::beginPacket(IPAddress ip, uint16_t port)
{
	_offset = 0;
	uint8_t buffer[4];
	buffer[0] = ip[0];
	buffer[1] = ip[1];
	buffer[2] = ip[2];
	buffer[3] = ip[3];
	if (_sourse->hardware()->socketStartUDP(sockindex, buffer, port))
	{
		return FLPROG_SUCCESS;
	}
	return FLPROG_EHERNET_ERROR;
}

int FLProgEthernetUDP::endPacket()
{
	return _sourse->hardware()->socketSendUDP(sockindex);
}

size_t FLProgEthernetUDP::write(uint8_t byte)
{
	return write(&byte, 1);
}

size_t FLProgEthernetUDP::write(const uint8_t *buffer, size_t size)
{
	uint16_t bytes_written = _sourse->hardware()->socketBufferData(sockindex, _offset, buffer, size);
	_offset += bytes_written;
	return bytes_written;
}

int FLProgEthernetUDP::parsePacket()
{
	while (_remaining)
	{
		read((uint8_t *)NULL, _remaining);
	}

	if (_sourse->hardware()->socketRecvAvailable(sockindex) > 0)
	{

		uint8_t tmpBuf[8];
		int ret = 0;
		ret = _sourse->hardware()->socketRecv(sockindex, tmpBuf, 8);
		if (ret > 0)
		{
			_remoteIP = tmpBuf;
			_remotePort = tmpBuf[4];
			_remotePort = (_remotePort << 8) + tmpBuf[5];
			_remaining = tmpBuf[6];
			_remaining = (_remaining << 8) + tmpBuf[7];
			ret = _remaining;
		}
		return ret;
	}
	return 0;
}

int FLProgEthernetUDP::read()
{
	uint8_t byte;
	if ((_remaining > 0) && (_sourse->hardware()->socketRecv(sockindex, &byte, 1) > 0))
	{
		_remaining--;
		return byte;
	}
	return -1;
}

int FLProgEthernetUDP::read(uint8_t *buffer, size_t len)
{
	if (_remaining > 0)
	{
		int got;
		if (_remaining <= len)
		{
			got = _sourse->hardware()->socketRecv(sockindex, buffer, _remaining);
		}
		else
		{
			got = _sourse->hardware()->socketRecv(sockindex, buffer, len);
		}
		if (got > 0)
		{
			_remaining -= got;
			return got;
		}
	}
	return -1;
}

int FLProgEthernetUDP::peek()
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM || _remaining == 0)
	{
		return -1;
	}
	return _sourse->hardware()->socketPeek(sockindex);
}

void FLProgEthernetUDP::flush()
{
	// TODO: we should wait for TX buffer to be emptied
}

uint8_t FLProgEthernetUDP::beginMulticast(IPAddress ip, uint16_t port)
{
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_sourse->hardware()->socketClose(sockindex);
	}
	sockindex = _sourse->hardware()->socketBeginMulticast((FLPROG_SN_MR_UDP | FLPROG_SN_MR_MULTI), ip, port);
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		return 0;
	}
	_port = port;
	_remaining = 0;
	return 1;
}
