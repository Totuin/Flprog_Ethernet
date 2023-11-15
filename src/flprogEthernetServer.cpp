#include "flprogEthernetServer.h"

FLProgEthernetServer::FLProgEthernetServer(FlprogAbstractEthernet *sourse, uint16_t port)
{
	_hardware = sourse->hardware();
	_dns = sourse->dnsClient();
	_port = port;
}

void FLProgEthernetServer::begin(uint16_t port)
{
	_port = port;
	begin();
}

void FLProgEthernetServer::begin()
{
	uint8_t sockindex = _hardware->socketBegin(FLPROG_SN_MR_TCP, _port);
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		if (_hardware->socketListen(sockindex))
		{
			server_port[sockindex] = _port;
		}
		else
		{
			_hardware->socketDisconnect(sockindex);
		}
	}
}

FLProgEthernetClient FLProgEthernetServer::accept()
{
	bool listening = false;
	uint8_t sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	uint8_t chip, maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;

	chip = _hardware->getChip();
	if (!chip)
		return FLProgEthernetClient(_hardware, _dns, FLPROG_ETHERNET_MAX_SOCK_NUM);
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			uint8_t stat = _hardware->socketStatus(i);
			if (sockindex == FLPROG_ETHERNET_MAX_SOCK_NUM &&
				((stat == FLPROG_SN_SR_ESTABLISHED) || (stat == FLPROG_SN_SR_CLOSE_WAIT)))
			{
				sockindex = i;
				server_port[i] = 0; // only return the client once
			}
			else if (stat == FLPROG_SN_SR_LISTEN)
			{
				listening = true;
			}
			else if (stat == FLPROG_SN_SR_CLOSED)
			{
				server_port[i] = 0;
			}
		}
	}
	if (!listening)
		begin();
	return FLProgEthernetClient(_hardware, _dns, sockindex);
}

FLProgEthernetServer::operator bool()
{
	uint8_t maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (_hardware->getChip() == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			if (_hardware->socketStatus(i) == FLPROG_SN_SR_LISTEN)
			{
				return true; // server is listening for incoming clients
			}
		}
	}
	return false;
}


size_t FLProgEthernetServer::write(uint8_t b)
{
	return write(&b, 1);
}
size_t FLProgEthernetServer::write(const uint8_t *buffer, size_t size)
{
	uint8_t chip, maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;

	chip = _hardware->getChip();
	if (!chip)
		return 0;
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	available();
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			if (_hardware->socketStatus(i) == FLPROG_SN_SR_ESTABLISHED)
			{
				_hardware->socketSend(i, buffer, size);
			}
		}
	}
	return size;
}


