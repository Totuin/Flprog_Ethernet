#include "flprogEthernetClient.h"

FLProgEthernetClient::FLProgEthernetClient(FLProgAbstractTcpInterface *sourse)
{
	init(sourse);
}

FLProgEthernetClient::FLProgEthernetClient(FLProgAbstractTcpInterface *sourse, uint8_t s)
{
	init(sourse);
	_sockindex = s;
}

void FLProgEthernetClient::setServerData(uint8_t s)
{
	_sockindex = s;
}

void FLProgEthernetClient::init(FLProgAbstractTcpInterface *sourse)
{
	if (isInit)
	{
		return;
	}
	_sourse = sourse;
	_dns.setSourse(sourse);
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
		_sourse->closeSoket(_sockindex);
		_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR;
		return FLPROG_ERROR;
	}
	if (_status != FLPROG_WAIT_ETHERNET_CLIENT_CONNECT_STATUS)
	{
		if (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
		{
			_sourse->closeSoket(_sockindex);
			_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
		}
		_sockindex = _sourse->getTCPSoket(0);
		if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		{
			_status = FLPROG_READY_STATUS;
			_errorCode = FLPROG_ETHERNET_CLIENT_SOKET_START_ERROR;
			return FLPROG_ERROR;
		}
		_sourse->connectSoket(_sockindex, ip, port);
		_startConnectTime = millis();
		_status = FLPROG_WAIT_ETHERNET_CLIENT_CONNECT_STATUS;
	}
	if (flprog::isTimer(_startConnectTime, _timeout))
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_CLIENT_CONNECT_TIMEOUT_ERROR;
		_sourse->closeSoket(_sockindex);
		_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
		return FLPROG_ERROR;
	}
	if (_sourse->isConnectStatusSoket(_sockindex))
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_NOT_ERROR;
		return FLPROG_SUCCESS;
	}
	else
	{
		if (_sourse->isCosedStatusSoket())
		{
			stop();
			_status = FLPROG_READY_STATUS;
			_errorCode = FLPROG_ETHERNET_CLIENT_SOKET_CLOSED_ERROR;
			return FLPROG_ERROR;
		}
	}
	return FLPROG_WITE;
}

size_t FLProgEthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FLProgEthernetClient::write(const uint8_t *buf, size_t size)
{
	uint8_t res = _sourse->writeToSoket(buf, size, _sockindex);
	if (res)
	{
		return res;
	}
	setWriteError();
	return 0;
}

int FLProgEthernetClient::available()
{
	return _sourse->availableSoket(_sockindex);
}

int FLProgEthernetClient::read(uint8_t *buf, size_t size)
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	return _sourse->readFromSoket(_sockindex, buf, size);
}

int FLProgEthernetClient::peek()
{
	return _sourse->peekSoket(_sockindex);
}

int FLProgEthernetClient::read()
{
	return _sourse->readFromSoket(_sockindex);
}

void FLProgEthernetClient::flush()
{ /*
	 while (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	 {
		 uint8_t stat = _sourse->hardware()->socketStatus(_sockindex);
		 if ((stat != FLPROG_SN_SR_ESTABLISHED) && (stat != FLPROG_SN_SR_CLOSE_WAIT))
			 return;
		 if (_sourse->hardware()->socketSendAvailable(_sockindex) >= _sourse->hardware()->_SSIZE())
			 return;
	 }
	 */
}

void FLProgEthernetClient::stop()
{
	_sourse->closeSoket(_sockindex);
	_sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
}

uint8_t FLProgEthernetClient::connected()
{
	return _sourse->socetConnected(_sockindex);
}

uint8_t FLProgEthernetClient::status()
{
	return _sourse->statusSoket(_sockindex);
}

uint16_t FLProgEthernetClient::localPort()
{
	return _sourse->localPortSoket(_sockindex);
}

IPAddress FLProgEthernetClient::remoteIP()
{
	return _sourse->remoteIPSoket(_sockindex);
}

uint16_t FLProgEthernetClient::remotePort()
{
	return _sourse->remotePortSoket(_sockindex);
}
