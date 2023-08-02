#include "flprogEthernetServer.h"

FlprogEthernetServer::FlprogEthernetServer(FlprogAbstractEthernet *sourse, uint16_t port)
{
	_hardware = sourse->hardware();
	_dns = sourse->dnsClient();
	_port = port;
}

void FlprogEthernetServer::begin()
{
	uint8_t sockindex = _hardware->socketBegin(FLPROG_SN_MR_TCP, _port);
	if (sockindex < MAX_SOCK_NUM)
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

FlprogEthernetClient FlprogEthernetServer::available()
{
	bool listening = false;
	uint8_t sockindex = MAX_SOCK_NUM;
	uint8_t chip, maxindex = MAX_SOCK_NUM;
	chip = _hardware->getChip();
	if (!chip)
		return FlprogEthernetClient(_hardware, _dns, MAX_SOCK_NUM);
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			uint8_t stat = _hardware->socketStatus(i);
			if ((stat == FLPROG_SN_SR_ESTABLISHED) || (stat == FLPROG_SN_SR_CLOSE_WAIT))
			{
				if (_hardware->socketRecvAvailable(i) > 0)
				{
					sockindex = i;
				}
				else
				{
					// remote host closed connection, our end still open
					if (stat == FLPROG_SN_SR_CLOSE_WAIT)
					{
						_hardware->socketDisconnect(i);
						// status becomes LAST_ACK for short time
					}
				}
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
	{
		begin();
	}
	return FlprogEthernetClient(_hardware, _dns, sockindex);
}

FlprogEthernetClient FlprogEthernetServer::accept()
{
	bool listening = false;
	uint8_t sockindex = MAX_SOCK_NUM;
	uint8_t chip, maxindex = MAX_SOCK_NUM;

	chip = _hardware->getChip();
	if (!chip)
		return FlprogEthernetClient(_hardware, _dns, MAX_SOCK_NUM);
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			uint8_t stat = _hardware->socketStatus(i);
			if (sockindex == MAX_SOCK_NUM &&
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
	return FlprogEthernetClient(_hardware, _dns, sockindex);
}

FlprogEthernetServer::operator bool()
{
	uint8_t maxindex = MAX_SOCK_NUM;
#if MAX_SOCK_NUM > 4
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

#if 0
void FlprogEthernetServer::statusreport()
{
	Serial.printf("EthernetServer, port=%d\n", _port);
	for (uint8_t i=0; i < MAX_SOCK_NUM; i++) {
		uint16_t port = server_port[i];
		uint8_t stat = _hardware->socketStatus(i);
		const char *name;
		switch (stat) {
			case 0x00: name = "CLOSED"; break;
			case 0x13: name = "INIT"; break;
			case 0x14: name = "LISTEN"; break;
			case 0x15: name = "SYNSENT"; break;
			case 0x16: name = "SYNRECV"; break;
			case 0x17: name = "ESTABLISHED"; break;
			case 0x18: name = "FIN_WAIT"; break;
			case 0x1A: name = "CLOSING"; break;
			case 0x1B: name = "TIME_WAIT"; break;
			case 0x1C: name = "CLOSE_WAIT"; break;
			case 0x1D: name = "LAST_ACK"; break;
			case 0x22: name = "UDP"; break;
			case 0x32: name = "IPRAW"; break;
			case 0x42: name = "MACRAW"; break;
			case 0x5F: name = "PPPOE"; break;
			default: name = "???";
		}
		int avail = _hardware->socketRecvAvailable(i);
		Serial.printf("  %d: port=%d, status=%s (0x%02X), avail=%d\n",
			i, port, name, stat, avail);
	}
}
#endif

size_t FlprogEthernetServer::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FlprogEthernetServer::write(const uint8_t *buffer, size_t size)
{
	uint8_t chip, maxindex = MAX_SOCK_NUM;

	chip = _hardware->getChip();
	if (!chip)
		return 0;
#if MAX_SOCK_NUM > 4
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
