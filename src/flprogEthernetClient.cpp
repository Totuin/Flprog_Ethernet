#include "flprogEthernetClient.h"

FlprogEthernetClient::FlprogEthernetClient(FlprogAbstractEthernet *sourse)
{
	_hardware = sourse->hardware();
	_dns = sourse->dnsClient();
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	_timeout = 1000;
	isInit = true;
}

void FlprogEthernetClient::init(FlprogAbstractEthernet *sourse)
{
	if (isInit)
	{
		return;
	}
	_hardware = sourse->hardware();
	_dns = sourse->dnsClient();
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	_timeout = 1000;
	isInit = true;
}

FlprogEthernetClient::FlprogEthernetClient(FlprogAbstractEthernetHardware *hardware, FlprogDNSClient *dns)
{
	_hardware = hardware;
	_dns = dns;
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	_timeout = 1000;
	isInit = true;
}

FlprogEthernetClient::FlprogEthernetClient(FlprogAbstractEthernetHardware *hardware, FlprogDNSClient *dns, uint8_t s)
{
	_hardware = hardware;
	_dns = dns;
	sockindex = s;
	_timeout = 1000;
	isInit = true;
}

int FlprogEthernetClient::connect(const char *host, uint16_t port)
{
	uint8_t remote_addr[4] = {0, 0, 0, 0};
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		if (_hardware->socketStatus(sockindex) != FLPROG_SN_SR_CLOSED)
		{
			_hardware->socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	}

	if (!_dns->getHostByName(host, remote_addr))
	{
		return 0;
	} // TODO: use _timeout
	return connect(IPAddress(remote_addr[0], remote_addr[1], remote_addr[2], remote_addr[3]), port);
}

int FlprogEthernetClient::connect(IPAddress ip, uint16_t port)
{
	if (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		if (_hardware->socketStatus(sockindex) != FLPROG_SN_SR_CLOSED)
		{
			_hardware->socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	}
#if defined(ESP8266) || defined(ESP32)
	if (ip == IPAddress((uint32_t)0) || ip == IPAddress(0xFFFFFFFFul))
		return 0;
#else
	if (ip == IPAddress(0ul) || ip == IPAddress(0xFFFFFFFFul))
		return 0;
#endif
	sockindex = _hardware->socketBegin(FLPROG_SN_MR_TCP, 0);
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	_hardware->socketConnect(sockindex, ip, port);
	uint32_t start = millis();
	while (1)
	{
		uint8_t stat = _hardware->socketStatus(sockindex);
		if (stat == FLPROG_SN_SR_ESTABLISHED)
			return 1;
		if (stat == FLPROG_SN_SR_CLOSE_WAIT)
			return 1;
		if (stat == FLPROG_SN_SR_CLOSED)
			return 0;
		if (millis() - start > _timeout)
			break;
		delay(1);
	}
	_hardware->socketClose(sockindex);
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	return 0;
}

int FlprogEthernetClient::availableForWrite(void)
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _hardware->socketSendAvailable(sockindex);
}

size_t FlprogEthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FlprogEthernetClient::write(const uint8_t *buf, size_t size)
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	if (_hardware->socketSend(sockindex, buf, size))
		return size;
	setWriteError();
	return 0;
}

int FlprogEthernetClient::available()
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _hardware->socketRecvAvailable(sockindex);
	// TODO: do the Wiznet chips automatically retransmit TCP ACK
	// packets if they are lost by the network?  Someday this should
	// be checked by a man-in-the-middle test which discards certain
	// packets.  If ACKs aren't resent, we would need to check for
	// returning 0 here and after a timeout do another Sock_RECV
	// command to cause the Wiznet chip to resend the ACK packet.
}

int FlprogEthernetClient::read(uint8_t *buf, size_t size)
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _hardware->socketRecv(sockindex, buf, size);
}

int FlprogEthernetClient::peek()
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return -1;
	if (!available())
		return -1;
	return _hardware->socketPeek(sockindex);
}

int FlprogEthernetClient::read()
{
	uint8_t b;
	if (_hardware->socketRecv(sockindex, &b, 1) > 0)
		return b;
	return -1;
}

void FlprogEthernetClient::flush()
{
	while (sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		uint8_t stat = _hardware->socketStatus(sockindex);
		if ((stat != FLPROG_SN_SR_ESTABLISHED) && (stat != FLPROG_SN_SR_CLOSE_WAIT))
			return;
		if (_hardware->socketSendAvailable(sockindex) >= _hardware->_SSIZE())
			return;
	}
}

void FlprogEthernetClient::stop()
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return;

	// attempt to close the connection gracefully (send a FIN to other side)
	_hardware->socketDisconnect(sockindex);
	unsigned long start = millis();

	// wait up to a second for the connection to close
	do
	{
		if (_hardware->socketStatus(sockindex) == FLPROG_SN_SR_CLOSED)
		{
			sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
			return; // exit the loop
		}
		delay(1);
	} while (millis() - start < _timeout);

	// if it hasn't closed, close it forcefully
	_hardware->socketClose(sockindex);
	sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
}

uint8_t FlprogEthernetClient::connected()
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	uint8_t s = _hardware->socketStatus(sockindex);
	return !((s == FLPROG_SN_SR_LISTEN) || (s == FLPROG_SN_SR_CLOSED) || (s == FLPROG_SN_SR_FIN_WAIT) ||
			 ((s == FLPROG_SN_SR_CLOSE_WAIT) && !available()));
}

uint8_t FlprogEthernetClient::status()
{
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return FLPROG_SN_SR_CLOSED;
	return _hardware->socketStatus(sockindex);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.
bool FlprogEthernetClient::operator==(const FlprogEthernetClient &rhs)
{
	if (sockindex != rhs.sockindex)
		return false;
	if (sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return false;
	if (rhs.sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return false;
	return true;
}

uint16_t FlprogEthernetClient::localPort()
{
	return _hardware->localPort(sockindex);
}

IPAddress FlprogEthernetClient::remoteIP()
{
	return _hardware->remoteIP(sockindex);
}

uint16_t FlprogEthernetClient::remotePort()
{
	return _hardware->remotePort(sockindex);
}
