#include "flprogEthernetClient.h"

FLProgEthernetClient::FLProgEthernetClient(FlprogAbstractEthernet *sourse)
{
	init(sourse);
}

FLProgEthernetClient::FLProgEthernetClient(FlprogAbstractEthernet *sourse, uint8_t s)
{
	init(sourse);
	_sockindex = s;
}

void FLProgEthernetClient::init(FlprogAbstractEthernet *sourse)
{
	if (isInit)
	{
		return;
	}
	_sourse = sourse;
	_dns = new FLProgDNSClient;
	_dns->setUDP(new FLProgEthernetUDP);
	_dns->udp()->setSourse(sourse);
	_status = FLPROG_READY_STATUS;
	isInit = true;
}

int FLProgEthernetClient::connect(const char *host, uint16_t port)
{
	if (!_sourse->isReady())
	{
		stop();
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR;
		return FLPROG_ERROR;
	}
	if (_status != FLPROG_WAIT_ETHERNET_DNS_STATUS)
	{
		if (_dnsCachedHost.equals(String(host)))
		{
			if (flprog::isTimer(_dnsStartCachTime, _dnsCacheStorageTime))
			{
				_dnsCachedHost = "";
				_dnsCachedIP = FLPROG_INADDR_NONE;
			}
			else
			{
				if (_dnsCachedIP == FLPROG_INADDR_NONE)
				{
					_dnsCachedHost = "";
				}
				else
				{
					return connect(_dnsCachedIP, port);
				}
			}
		}
		else
		{
			_dnsCachedHost = "";
			_dnsCachedIP = FLPROG_INADDR_NONE;
		}
	}
	uint8_t remote_addr[4] = {0, 0, 0, 0};
	_dns->begin(_sourse->dns());
	uint8_t result = _dns->getHostByName(host, remote_addr);
	if (result == FLPROG_WITE)
	{
		_status = FLPROG_WAIT_ETHERNET_DNS_STATUS;
		return FLPROG_WITE;
	}
	if (result == FLPROG_ERROR)
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = _dns->getError();
		return FLPROG_ERROR;
	}

	_dnsStartCachTime = millis();
	_dnsCachedHost = String(host);
	_dnsCachedIP = IPAddress(remote_addr[0], remote_addr[1], remote_addr[2], remote_addr[3]);
	return connect(_dnsCachedIP, port);
}

int FLProgEthernetClient::connect(IPAddress ip, uint16_t port)
{
	if (!_sourse->isReady())
	{
		_sourse->hardware()->socketClose(_sockindex);
		_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR;
		return FLPROG_ERROR;
	}
	if (_status != FLPROG_WAIT_ETHERNET_CLIENT_CONNECT_STATUS)
	{
		if (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
		{
			if (_sourse->hardware()->socketStatus(_sockindex) != FLPROG_SN_SR_CLOSED)
			{
				_sourse->hardware()->socketDisconnect(_sockindex); // TODO: should we call stop()?
			}
			_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
		}
		_sockindex = _sourse->hardware()->socketBegin(FLPROG_SN_MR_TCP, 0);
		if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		{
			_status = FLPROG_READY_STATUS;
			_errorCode = FLPROG_ETHERNET_CLIENT_SOKET_START_ERROR;
			return FLPROG_ERROR;
		}
		_sourse->hardware()->socketConnect(_sockindex, ip, port);
		_startConnectTime = millis();
		_status = FLPROG_WAIT_ETHERNET_CLIENT_CONNECT_STATUS;
	}
	if (flprog::isTimer(_startConnectTime, _timeout))
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_CLIENT_CONNECT_TIMEOUT_ERROR;
		_sourse->hardware()->socketClose(_sockindex);
		_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
		return FLPROG_ERROR;
	}
	uint8_t stat = _sourse->hardware()->socketStatus(_sockindex);
	if ((stat == FLPROG_SN_SR_ESTABLISHED) || (stat == FLPROG_SN_SR_CLOSE_WAIT))
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_NOT_ERROR;
		return FLPROG_SUCCESS;
	}
	if (stat == FLPROG_SN_SR_CLOSED)
	{
		stop();
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_CLIENT_SOKET_CLOSED_ERROR;
		return FLPROG_ERROR;
	}
	return FLPROG_WITE;
}

int FLProgEthernetClient::availableForWrite(void)
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _sourse->hardware()->socketSendAvailable(_sockindex);
}

size_t FLProgEthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FLProgEthernetClient::write(const uint8_t *buf, size_t size)
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	if (_sourse->hardware()->socketSend(_sockindex, buf, size))
		return size;
	setWriteError();
	return 0;
}

int FLProgEthernetClient::available()
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _sourse->hardware()->socketRecvAvailable(_sockindex);
}

int FLProgEthernetClient::read(uint8_t *buf, size_t size)
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _sourse->hardware()->socketRecv(_sockindex, buf, size);
}

int FLProgEthernetClient::peek()
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return -1;
	if (!available())
		return -1;
	return _sourse->hardware()->socketPeek(_sockindex);
}

int FLProgEthernetClient::read()
{
	uint8_t b;
	if (_sourse->hardware()->socketRecv(_sockindex, &b, 1) > 0)
		return b;
	return -1;
}

void FLProgEthernetClient::flush()
{
	while (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		uint8_t stat = _sourse->hardware()->socketStatus(_sockindex);
		if ((stat != FLPROG_SN_SR_ESTABLISHED) && (stat != FLPROG_SN_SR_CLOSE_WAIT))
			return;
		if (_sourse->hardware()->socketSendAvailable(_sockindex) >= _sourse->hardware()->_SSIZE())
			return;
	}
}

void FLProgEthernetClient::stop()
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		return;
	}
	_sourse->hardware()->socketClose(_sockindex);
	_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
}

uint8_t FLProgEthernetClient::connected()
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	uint8_t s = _sourse->hardware()->socketStatus(_sockindex);
	return !((s == FLPROG_SN_SR_LISTEN) || (s == FLPROG_SN_SR_CLOSED) || (s == FLPROG_SN_SR_FIN_WAIT) ||
			 ((s == FLPROG_SN_SR_CLOSE_WAIT) && !available()));
}

uint8_t FLProgEthernetClient::status()
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return FLPROG_SN_SR_CLOSED;
	return _sourse->hardware()->socketStatus(_sockindex);
}

bool FLProgEthernetClient::operator==(const FLProgEthernetClient &rhs)
{
	if (_sockindex != rhs._sockindex)
		return false;
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return false;
	if (rhs._sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return false;
	return true;
}

uint16_t FLProgEthernetClient::localPort()
{
	return _sourse->hardware()->localPort(_sockindex);
}

IPAddress FLProgEthernetClient::remoteIP()
{
	return _sourse->hardware()->remoteIP(_sockindex);
}

uint16_t FLProgEthernetClient::remotePort()
{
	return _sourse->hardware()->remotePort(_sockindex);
}
