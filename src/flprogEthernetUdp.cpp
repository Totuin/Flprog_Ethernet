
#include "flprogEthernetUdp.h"

FLProgEthernetUDP::FLProgEthernetUDP(FLProgAbstractTcpInterface *sourse)
{
	setSourse(sourse);
}

void FLProgEthernetUDP::setSourse(FLProgAbstractTcpInterface *sourse)
{
	_dns.setSourse(sourse);
	_sourse = sourse;
}

int FLProgEthernetUDP::available()

	int FLProgEthernetUDP::beginPacket(const char *host, uint16_t port)
{
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
					return beginPacket(_dnsCachedIP, port);
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
	uint8_t result = _dns.getHostByName(host, remote_addr);
	if (result == FLPROG_WITE)
	{
		_status = FLPROG_WAIT_ETHERNET_DNS_STATUS;
		return FLPROG_WITE;
	}
	if (result = FLPROG_ERROR)
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = _dns.getError();
		return FLPROG_ERROR;
	}
	_dnsStartCachTime = millis();
	_dnsCachedHost = String(host);
	_dnsCachedIP = IPAddress(remote_addr[0], remote_addr[1], remote_addr[2], remote_addr[3]);
	return beginPacket(_dnsCachedIP, port);
}

uint8_t FLProgEthernetUDP::beginMulticast(IPAddress ip, uint16_t port)
{
	if (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_sourse->closeSoket(_sockindex);
	}
	_sockindex = _sourse->hardware()->socketBeginMulticast((FLPROG_SN_MR_UDP | FLPROG_SN_MR_MULTI), ip, port);
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		return 0;
	}
	_port = port;
	_remaining = 0;
	return 1;
}
