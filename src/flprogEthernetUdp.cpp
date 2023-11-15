
#include "flprogEthernetUdp.h"

/* Start EthernetUDP socket, listening at local port PORT */

FLProgEthernetUDP::FLProgEthernetUDP(FlprogAbstractEthernet *sourse)
{
	_hardware = sourse->hardware();
	_dns = sourse->dnsClient();
}

void FLProgEthernetUDP::setHatdware(FLProgAbstractEthernetHardware *hardware)
{
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	_hardware = hardware;
}

uint8_t FLProgEthernetUDP::begin(uint16_t port)
{
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
		_hardware->socketClose(sockindex);
	sockindex = _hardware->socketBegin(FLPROG_SN_MR_UDP, port);
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	_port = port;
	_remaining = 0;
	return 1;
}

/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int FLProgEthernetUDP::available()
{
	return _remaining;
}

/* Release any resources being used by this EthernetUDP instance */
void FLProgEthernetUDP::stop()
{
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_hardware->socketClose(sockindex);
		sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	}
}

int FLProgEthernetUDP::beginPacket(const char *host, uint16_t port)
{
	int ret = 0;
	uint8_t remote_addr[4] = {0, 0, 0, 0};
	ret = _dns->getHostByName(host, remote_addr);
	if (ret != 1)
		return ret;
	return beginPacket(IPAddress(remote_addr[0], remote_addr[1], remote_addr[2], remote_addr[3]), port);
}

int FLProgEthernetUDP::beginPacket(IPAddress ip, uint16_t port)
{
	_offset = 0;
	uint8_t buffer[4];
	buffer[0] = ip[0];
	buffer[1] = ip[1];
	buffer[2] = ip[2];
	buffer[3] = ip[3];
	return _hardware->socketStartUDP(sockindex, buffer, port);
}

int FLProgEthernetUDP::endPacket()
{
	return _hardware->socketSendUDP(sockindex);
}

size_t FLProgEthernetUDP::write(uint8_t byte)
{
	return write(&byte, 1);
}

size_t FLProgEthernetUDP::write(const uint8_t *buffer, size_t size)
{
	
	uint16_t bytes_written = _hardware->socketBufferData(sockindex, _offset, buffer, size);
	_offset += bytes_written;
	return bytes_written;
}

int FLProgEthernetUDP::parsePacket()
{
	// discard any remaining bytes in the last packet
	while (_remaining)
	{
		// could this fail (loop endlessly) if _remaining > 0 and recv in read fails?
		// should only occur if recv fails after telling us the data is there, lets
		// hope the w5100 always behaves :)
		read((uint8_t *)NULL, _remaining);
	}

	if (_hardware->socketRecvAvailable(sockindex) > 0)
	{
		// HACK - hand-parse the UDP packet using TCP recv method
		uint8_t tmpBuf[8];
		int ret = 0;
		// read 8 header bytes and get IP and port from it
		ret = _hardware->socketRecv(sockindex, tmpBuf, 8);
		if (ret > 0)
		{
			_remoteIP = tmpBuf;
			_remotePort = tmpBuf[4];
			_remotePort = (_remotePort << 8) + tmpBuf[5];
			_remaining = tmpBuf[6];
			_remaining = (_remaining << 8) + tmpBuf[7];

			// When we get here, any remaining bytes are the data
			ret = _remaining;
		}
		return ret;
	}
	// There aren't any packets available
	return 0;
}

int FLProgEthernetUDP::read()
{
	uint8_t byte;

	if ((_remaining > 0) && (_hardware->socketRecv(sockindex, &byte, 1) > 0))
	{
		// We read things without any problems
		_remaining--;
		return byte;
	}

	// If we get here, there's no data available
	return -1;
}

int FLProgEthernetUDP::read(uint8_t *buffer, size_t len)
{
	if (_remaining > 0)
	{
		int got;
		if (_remaining <= len)
		{
			// data should fit in the buffer
			got = _hardware->socketRecv(sockindex, buffer, _remaining);
		}
		else
		{
			// too much data for the buffer,
			// grab as much as will fit
			got = _hardware->socketRecv(sockindex, buffer, len);
		}
		if (got > 0)
		{
			_remaining -= got;
			return got;
		}
	}
	// If we get here, there's no data available or recv failed
	return -1;
}

int FLProgEthernetUDP::peek()
{
	// Unlike recv, peek doesn't check to see if there's any data available, so we must.
	// If the user hasn't called parsePacket yet then return nothing otherwise they
	// may get the UDP header
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM || _remaining == 0)
		return -1;
	return _hardware->socketPeek(sockindex);
}

void FLProgEthernetUDP::flush()
{
	// TODO: we should wait for TX buffer to be emptied
}

/* Start EthernetUDP socket, listening at local port PORT */
uint8_t FLProgEthernetUDP::beginMulticast(IPAddress ip, uint16_t port)
{
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
		_hardware->socketClose(sockindex);
	sockindex = _hardware->socketBeginMulticast((FLPROG_SN_MR_UDP | FLPROG_SN_MR_MULTI), ip, port);
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	_port = port;
	_remaining = 0;
	return 1;
}
