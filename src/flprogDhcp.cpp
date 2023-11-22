#include "flprogDhcp.h"

void FLProgDhcpClass::setUDP(FLProgEthernetUDP *udp)
{
	_udp = udp;
}

int FLProgDhcpClass::beginWithDHCP(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	if (_status == FLPROG_READY_STATUS)
	{
		_dhcpLeaseTime = 0;
		_dhcpT1 = 0;
		_dhcpT2 = 0;
		_timeout = timeout;
		_responseTimeout = responseTimeout;
		memset(_dhcpMacAddr, 0, 6);
		reset_DHCP_lease();
		memcpy((void *)_dhcpMacAddr, (void *)mac, 6);
		_dhcp_state = FLPROG_STATE_DHCP_START;
	}
	return request_DHCP_lease();
}

void FLProgDhcpClass::reset_DHCP_lease()
{
	memset(_dhcpLocalIp, 0, 20);
}

int FLProgDhcpClass::request_DHCP_lease()
{
	if (_status != FLPROG_WAIT_ETHERNET_DHCP_STATUS)
	{
		_status = FLPROG_WAIT_ETHERNET_DHCP_STATUS;
		_dhcpTransactionId = random(1UL, 2000UL);
		_dhcpInitialTransactionId = _dhcpTransactionId;
		_udp->stop();
		if ((_udp->begin(FLPROG_DHCP_CLIENT_PORT)) != FLPROG_SUCCESS)
		{
			_status = FLPROG_READY_STATUS;
			_errorCode = _udp->getError();
			return FLPROG_ERROR;
		}
	}
	uint8_t result = cheskStateMashine();
	if (result == FLPROG_WITE)
	{

		return FLPROG_WITE;
	}
	_udp->stop();
	_dhcpTransactionId++;
	_lastCheckLeaseMillis = millis();
	return result;
}

uint8_t FLProgDhcpClass::cheskStateMashine()
{
	uint8_t result;

	if (_dhcp_state == FLPROG_STATE_DHCP_START)
	{
		_dhcpTransactionId++;
		_startDhcpReqestTime = millis();
		send_DHCP_MESSAGE(FLPROG_DHCP_DISCOVER, ((millis() - _startDhcpReqestTime) / 1000));
		_lastCheckDhcpReqestTime = flprog::timeBack(100);
		_dhcp_state = FLPROG_STATE_DHCP_DISCOVER;
		return FLPROG_WITE;
	}
	if (_dhcp_state == FLPROG_STATE_DHCP_DISCOVER)
	{
		result = parseDHCPResponse();
		if (result == FLPROG_DHCP_TIMEOUT_MESSAGE_TYPE)
		{
			_dhcp_state = FLPROG_STATE_DHCP_START;
			_errorCode = FLPROG_ETHERNET_DHCP_DISCOVERY_TIMEOUT_ERROR;
			_status = FLPROG_READY_STATUS;
			return FLPROG_ERROR;
		}
		if (result == FLPROG_DHCP_ERROR_ID_MESSAGE_TYPE)
		{
			_dhcp_state = FLPROG_STATE_DHCP_START;
			_errorCode = FLPROG_ETHERNET_DHCP_DISCOVERY_ERROR_ID_ERROR;
			_status = FLPROG_READY_STATUS;
			return FLPROG_ERROR;
		}

		if (result == FLPROG_DHCP_WITE_CHECK_REQEST_MESSAGE_TYPE)
		{
			return FLPROG_WITE;
		}

		if (result == FLPROG_DHCP_OFFER)
		{
			_dhcpTransactionId = _respId;
			send_DHCP_MESSAGE(FLPROG_DHCP_REQUEST, ((millis() - _startDhcpReqestTime) / 1000));
			_lastCheckDhcpReqestTime = flprog::timeBack(100);
			_dhcp_state = FLPROG_STATE_DHCP_REQUEST;
		}
		return FLPROG_WITE;
	}

	if (_dhcp_state == FLPROG_STATE_DHCP_REREQUEST)
	{
		_dhcpTransactionId++;
		_startDhcpReqestTime = millis();
		send_DHCP_MESSAGE(FLPROG_DHCP_REQUEST, ((millis() - _startDhcpReqestTime) / 1000));
		_dhcp_state = FLPROG_STATE_DHCP_REQUEST;
		return FLPROG_WITE;
	}

	if (_dhcp_state == FLPROG_STATE_DHCP_REQUEST)
	{
		result = parseDHCPResponse();
		if (result == FLPROG_DHCP_WITE_CHECK_REQEST_MESSAGE_TYPE)
		{
			return FLPROG_WITE;
		}
		if (result == FLPROG_DHCP_TIMEOUT_MESSAGE_TYPE)
		{
			_dhcp_state = FLPROG_STATE_DHCP_START;
			_errorCode = FLPROG_ETHERNET_DHCP_REREQUEST_TIMEOUT_ERROR;
			_status = FLPROG_READY_STATUS;
			return FLPROG_ERROR;
		}

		if (result == FLPROG_DHCP_ERROR_ID_MESSAGE_TYPE)
		{
			_dhcp_state = FLPROG_STATE_DHCP_START;
			_errorCode = FLPROG_ETHERNET_DHCP_REREQUEST_ERROR_ID_ERROR;
			_status = FLPROG_READY_STATUS;
			return FLPROG_ERROR;
		}

		if (result == FLPROG_DHCP_ACK)
		{
			_dhcp_state = FLPROG_STATE_DHCP_START;
			_errorCode = FLPROG_NOT_ERROR;
			_status = FLPROG_READY_STATUS;
			return FLPROG_SUCCESS;
		}
		if (result == FLPROG_DHCP_NAK)
		{
			_errorCode = FLPROG_ETHERNET_DHCP_REREQUEST_NAK_ERROR;
			_dhcp_state = FLPROG_STATE_DHCP_START;
			_status = FLPROG_READY_STATUS;
			return FLPROG_ERROR;
		}
		return FLPROG_WITE;
	}
	_errorCode = FLPROG_ETHERNET_DHCP_NOT_DEFINED_ERROR;
	_dhcp_state = FLPROG_STATE_DHCP_START;
	_status = FLPROG_READY_STATUS;
	return FLPROG_ERROR;
}

void FLProgDhcpClass::send_DHCP_MESSAGE(uint8_t messageType, uint16_t secondsElapsed)
{
	uint8_t buffer[32];
	memset(buffer, 0, 32);
	IPAddress dest_addr(255, 255, 255, 255); // Broadcast address
	if ((_udp->beginPacket(dest_addr, FLPROG_DHCP_SERVER_PORT)) != FLPROG_SUCCESS)
	{
		return;
	}
	buffer[0] = FLPROG_DHCP_BOOTREQUEST;  // op
	buffer[1] = FLPROG_DHCP_HTYPE10MB;	  // htype
	buffer[2] = FLPROG_DHCP_HLENETHERNET; // hlen
	buffer[3] = FLPROG_DHCP_HOPS;		  // hops
	unsigned long xid = flprogW5100Htonl(_dhcpTransactionId);
	memcpy(buffer + 4, &(xid), 4);
	/*
	buffer[8] = ((secondsElapsed & 0xff00) >> 8);
	buffer[9] = (secondsElapsed & 0x00ff);
	*/
	unsigned short flags = flprogW5100Htons(FLPROG_DHCP_FLAGSBROADCAST);
	memcpy(buffer + 10, &(flags), 2);
	_udp->write(buffer, 28);
	memset(buffer, 0, 32);			 // clear local buffer
	memcpy(buffer, _dhcpMacAddr, 6); // chaddr
	_udp->write(buffer, 16);
	memset(buffer, 0, 32); // clear local buffer
	for (int i = 0; i < 6; i++)
	{
		_udp->write(buffer, 32);
	}
	buffer[0] = (uint8_t)((FLPROG_MAGIC_COOKIE >> 24) & 0xFF);
	buffer[1] = (uint8_t)((FLPROG_MAGIC_COOKIE >> 16) & 0xFF);
	buffer[2] = (uint8_t)((FLPROG_MAGIC_COOKIE >> 8) & 0xFF);
	buffer[3] = (uint8_t)(FLPROG_MAGIC_COOKIE & 0xFF);
	buffer[4] = dhcpMessageType;
	buffer[5] = 0x01;
	buffer[6] = messageType; // DHCP_REQUEST;
	buffer[7] = dhcpClientIdentifier;
	buffer[8] = 0x07;
	buffer[9] = 0x01;
	memcpy(buffer + 10, _dhcpMacAddr, 6);
	buffer[16] = hostName;
	buffer[17] = strlen(FLPROG_HOST_NAME) + 6; // length of hostname + last 3 bytes of mac address
	strcpy((char *)&(buffer[18]), FLPROG_HOST_NAME);
	printByte((char *)&(buffer[24]), _dhcpMacAddr[3]);
	printByte((char *)&(buffer[26]), _dhcpMacAddr[4]);
	printByte((char *)&(buffer[28]), _dhcpMacAddr[5]);
	_udp->write(buffer, 30);
	if (messageType == FLPROG_DHCP_REQUEST)
	{
		buffer[0] = dhcpRequestedIPaddr;
		buffer[1] = 0x04;
		buffer[2] = _dhcpLocalIp[0];
		buffer[3] = _dhcpLocalIp[1];
		buffer[4] = _dhcpLocalIp[2];
		buffer[5] = _dhcpLocalIp[3];

		buffer[6] = dhcpServerIdentifier;
		buffer[7] = 0x04;
		buffer[8] = _dhcpDhcpServerIp[0];
		buffer[9] = _dhcpDhcpServerIp[1];
		buffer[10] = _dhcpDhcpServerIp[2];
		buffer[11] = _dhcpDhcpServerIp[3];
		_udp->write(buffer, 12);
	}
	buffer[0] = dhcpParamRequest;
	buffer[1] = 0x06;
	buffer[2] = subnetMask;
	buffer[3] = routersOnSubnet;
	buffer[4] = dns;
	buffer[5] = domainName;
	buffer[6] = dhcpT1value;
	buffer[7] = dhcpT2value;
	buffer[8] = endOption;
	_udp->write(buffer, 9);
	_udp->endPacket();
}

uint8_t FLProgDhcpClass::parseDHCPResponse()
{
	if (flprog::isTimer(_startDhcpReqestTime, _responseTimeout))
	{
		return FLPROG_DHCP_TIMEOUT_MESSAGE_TYPE;
	}
	if (!(flprog::isTimer(_lastCheckDhcpReqestTime, 50)))
	{
		return FLPROG_DHCP_WITE_CHECK_REQEST_MESSAGE_TYPE;
	}
	if (_udp->parsePacket() <= 0)
	{
		_lastCheckDhcpReqestTime = millis();
		return FLPROG_DHCP_WITE_CHECK_REQEST_MESSAGE_TYPE;
	}
	uint8_t type = 0;
	uint8_t opt_len = 0;
	FLPROG_RIP_MSG_FIXED fixedMsg;
	_udp->read((uint8_t *)&fixedMsg, sizeof(FLPROG_RIP_MSG_FIXED));
	if (fixedMsg.op == FLPROG_DHCP_BOOTREPLY && _udp->remotePort() == FLPROG_DHCP_SERVER_PORT)
	{
		_respId = flporgW5100Ntohl(fixedMsg.xid);
		if (memcmp(fixedMsg.chaddr, _dhcpMacAddr, 6) != 0 ||
			(_respId < _dhcpInitialTransactionId) ||
			(_respId > _dhcpTransactionId))
		{
			_udp->flush(); // FIXME
			return FLPROG_DHCP_ERROR_ID_MESSAGE_TYPE;
		}
		memcpy(_dhcpLocalIp, fixedMsg.yiaddr, 4);
		_udp->read((uint8_t *)NULL, 240 - (int)sizeof(FLPROG_RIP_MSG_FIXED));
		while (_udp->available() > 0)
		{
			switch (_udp->read())
			{
			case endOption:
				break;

			case padOption:
				break;

			case dhcpMessageType:
				opt_len = _udp->read();
				type = _udp->read();
				break;

			case subnetMask:
				opt_len = _udp->read();
				_udp->read(_dhcpSubnetMask, 4);
				break;

			case routersOnSubnet:
				opt_len = _udp->read();
				_udp->read(_dhcpGatewayIp, 4);
				_udp->read((uint8_t *)NULL, opt_len - 4);
				break;

			case dns:
				opt_len = _udp->read();
				_udp->read(_dhcpDnsServerIp, 4);
				_udp->read((uint8_t *)NULL, opt_len - 4);
				break;

			case dhcpServerIdentifier:
				opt_len = _udp->read();
				if (IPAddress(_dhcpDhcpServerIp) == IPAddress((uint32_t)0) ||
					IPAddress(_dhcpDhcpServerIp) == _udp->remoteIP())
				{
					_udp->read(_dhcpDhcpServerIp, sizeof(_dhcpDhcpServerIp));
				}
				else
				{
					// Skip over the rest of this option
					_udp->read((uint8_t *)NULL, opt_len);
				}
				break;

			case dhcpT1value:
				opt_len = _udp->read();
				_udp->read((uint8_t *)&_dhcpT1, sizeof(_dhcpT1));
				_dhcpT1 = flporgW5100Ntohl(_dhcpT1);
				break;

			case dhcpT2value:
				opt_len = _udp->read();
				_udp->read((uint8_t *)&_dhcpT2, sizeof(_dhcpT2));
				_dhcpT2 = flporgW5100Ntohl(_dhcpT2);
				break;

			case dhcpIPaddrLeaseTime:
				opt_len = _udp->read();
				_udp->read((uint8_t *)&_dhcpLeaseTime, sizeof(_dhcpLeaseTime));
				_dhcpLeaseTime = flporgW5100Ntohl(_dhcpLeaseTime);
				_renewInSec = _dhcpLeaseTime;
				break;

			default:
				opt_len = _udp->read();
				// Skip over the rest of this option
				_udp->read((uint8_t *)NULL, opt_len);
				break;
			}
		}
	}
	_udp->flush(); // FIXME
	return type;
}

/*
	returns:
	0/FLPROG_DHCP_CHECK_NONE: nothing happened
	1/FLPROG_DHCP_CHECK_RENEW_FAIL: renew failed
	2/FLPROG_DHCP_CHECK_RENEW_OK: renew success
	3/FLPROG_DHCP_CHECK_REBIND_FAIL: rebind fail
	4/FLPROG_DHCP_CHECK_REBIND_OK: rebind success
*/
int FLProgDhcpClass::checkLease()
{
	int rc = FLPROG_DHCP_CHECK_NONE;

	unsigned long now = millis();
	unsigned long elapsed = now - _lastCheckLeaseMillis;

	// if more then one sec passed, reduce the counters accordingly
	if (elapsed >= 1000)
	{
		// set the new timestamps
		_lastCheckLeaseMillis = now - (elapsed % 1000);
		elapsed = elapsed / 1000;

		// decrease the counters by elapsed seconds
		// we assume that the cycle time (elapsed) is fairly constant
		// if the remainder is less than cycle time * 2
		// do it early instead of late
		if (_renewInSec < elapsed * 2)
		{
			_renewInSec = 0;
		}
		else
		{
			_renewInSec -= elapsed;
		}
		if (_rebindInSec < elapsed * 2)
		{
			_rebindInSec = 0;
		}
		else
		{
			_rebindInSec -= elapsed;
		}
	}

	// if we have a lease but should renew, do it
	if (_renewInSec == 0 && _dhcp_state == FLPROG_STATE_DHCP_LEASED)
	{
		_dhcp_state = FLPROG_STATE_DHCP_REREQUEST;
		rc = 1 + request_DHCP_lease();
	}

	// if we have a lease or is renewing but should bind, do it
	if (_rebindInSec == 0 && (_dhcp_state == FLPROG_STATE_DHCP_LEASED ||
							  _dhcp_state == FLPROG_STATE_DHCP_START))
	{
		// this should basically restart completely
		_dhcp_state = FLPROG_STATE_DHCP_START;
		reset_DHCP_lease();
		rc = 3 + request_DHCP_lease();
	}
	return rc;
}

IPAddress FLProgDhcpClass::getLocalIp()
{
	return IPAddress(_dhcpLocalIp);
}

IPAddress FLProgDhcpClass::getSubnetMask()
{
	return IPAddress(_dhcpSubnetMask);
}

IPAddress FLProgDhcpClass::getGatewayIp()
{
	return IPAddress(_dhcpGatewayIp);
}

IPAddress FLProgDhcpClass::getDhcpServerIp()
{
	return IPAddress(_dhcpDhcpServerIp);
}

IPAddress FLProgDhcpClass::getDnsServerIp()
{
	return IPAddress(_dhcpDnsServerIp);
}

void FLProgDhcpClass::printByte(char *buf, uint8_t n)
{
	char *str = &buf[1];
	buf[0] = '0';
	do
	{
		unsigned long m = n;
		n /= 16;
		char c = m - 16 * n;
		*str-- = c < 10 ? c + '0' : c + 'A' - 10;
	} while (n);
}
