#include "flprogDhcp.h"

void FlprogDhcpClass::setUDP(FlprogEthernetUDP *udp)
{
	_udp = udp;
}

int FlprogDhcpClass::beginWithDHCP(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
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
	return request_DHCP_lease();
}

void FlprogDhcpClass::reset_DHCP_lease()
{
	memset(_dhcpLocalIp, 0, 20);
}

int FlprogDhcpClass::request_DHCP_lease()
{
	uint8_t messageType = 0;
	_dhcpTransactionId = random(1UL, 2000UL);
	_dhcpInitialTransactionId = _dhcpTransactionId;
	_udp->stop();
	if (_udp->begin(FLPROG_DHCP_CLIENT_PORT) == 0)
	{
		return 0;
	}
	presend_DHCP();
	int result = 0;
	unsigned long startTime = millis();
	while (_dhcp_state != FLPROG_STATE_DHCP_LEASED)
	{
		if (_dhcp_state == FLPROG_STATE_DHCP_START)
		{
			_dhcpTransactionId++;
			send_DHCP_MESSAGE(FLPROG_DHCP_DISCOVER, ((millis() - startTime) / 1000));
			_dhcp_state = FLPROG_STATE_DHCP_DISCOVER;
		}
		else if (_dhcp_state == FLPROG_STATE_DHCP_REREQUEST)
		{
			_dhcpTransactionId++;
			send_DHCP_MESSAGE(FLPROG_DHCP_REQUEST, ((millis() - startTime) / 1000));
			_dhcp_state = FLPROG_STATE_DHCP_REQUEST;
		}
		else if (_dhcp_state == FLPROG_STATE_DHCP_DISCOVER)
		{
			uint32_t respId;
			messageType = parseDHCPResponse(_responseTimeout, respId);
			if (messageType == FLPROG_DHCP_OFFER)
			{
				// We'll use the transaction ID that the offer came with,
				// rather than the one we were up to
				_dhcpTransactionId = respId;
				send_DHCP_MESSAGE(FLPROG_DHCP_REQUEST, ((millis() - startTime) / 1000));
				_dhcp_state = FLPROG_STATE_DHCP_REQUEST;
			}
		}
		else if (_dhcp_state == FLPROG_STATE_DHCP_REQUEST)
		{
			uint32_t respId;
			messageType = parseDHCPResponse(_responseTimeout, respId);
			if (messageType == FLPROG_DHCP_ACK)
			{
				_dhcp_state = FLPROG_STATE_DHCP_LEASED;
				result = 1;
				// use default lease time if we didn't get it
				if (_dhcpLeaseTime == 0)
				{
					_dhcpLeaseTime = FLPROG_DEFAULT_LEASE;
				}
				// Calculate T1 & T2 if we didn't get it
				if (_dhcpT1 == 0)
				{
					// T1 should be 50% of _dhcpLeaseTime
					_dhcpT1 = _dhcpLeaseTime >> 1;
				}
				if (_dhcpT2 == 0)
				{
					// T2 should be 87.5% (7/8ths) of _dhcpLeaseTime
					_dhcpT2 = _dhcpLeaseTime - (_dhcpLeaseTime >> 3);
				}
				_renewInSec = _dhcpT1;
				_rebindInSec = _dhcpT2;
			}
			else if (messageType == FLPROG_DHCP_NAK)
			{
				_dhcp_state = FLPROG_STATE_DHCP_START;
			}
		}

		if (messageType == 255)
		{
			messageType = 0;
			_dhcp_state = FLPROG_STATE_DHCP_START;
		}

		if (result != 1 && ((millis() - startTime) > _timeout))
			break;
	}

	// We're done with the socket now
	_udp->stop();
	_dhcpTransactionId++;
	_lastCheckLeaseMillis = millis();
	return result;
}

void FlprogDhcpClass::presend_DHCP()
{
}

void FlprogDhcpClass::send_DHCP_MESSAGE(uint8_t messageType, uint16_t secondsElapsed)
{
	uint8_t buffer[32];
	memset(buffer, 0, 32);
	IPAddress dest_addr(255, 255, 255, 255); // Broadcast address
	if (_udp->beginPacket(dest_addr, FLPROG_DHCP_SERVER_PORT) == -1)
	{
		return;
	}
	buffer[0] = FLPROG_DHCP_BOOTREQUEST;  // op
	buffer[1] = FLPROG_DHCP_HTYPE10MB;	  // htype
	buffer[2] = FLPROG_DHCP_HLENETHERNET; // hlen
	buffer[3] = FLPROG_DHCP_HOPS;		  // hops
	unsigned long xid = flprogW5100Htonl(_dhcpTransactionId);
	memcpy(buffer + 4, &(xid), 4);
	buffer[8] = ((secondsElapsed & 0xff00) >> 8);
	buffer[9] = (secondsElapsed & 0x00ff);
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

uint8_t FlprogDhcpClass::parseDHCPResponse(unsigned long responseTimeout, uint32_t &transactionId)
{
	uint8_t type = 0;
	uint8_t opt_len = 0;
	unsigned long startTime = millis();
	while (_udp->parsePacket() <= 0)
	{
		if ((millis() - startTime) > responseTimeout)
		{
			return 255;
		}
		delay(50);
	}
	FLPROG_RIP_MSG_FIXED fixedMsg;
	_udp->read((uint8_t *)&fixedMsg, sizeof(FLPROG_RIP_MSG_FIXED));
	if (fixedMsg.op == FLPROG_DHCP_BOOTREPLY && _udp->remotePort() == FLPROG_DHCP_SERVER_PORT)
	{
		transactionId = flporgW5100Ntohl(fixedMsg.xid);
		if (memcmp(fixedMsg.chaddr, _dhcpMacAddr, 6) != 0 ||
			(transactionId < _dhcpInitialTransactionId) ||
			(transactionId > _dhcpTransactionId))
		{
			_udp->flush(); // FIXME
			return 0;
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
int FlprogDhcpClass::checkLease()
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

IPAddress FlprogDhcpClass::getLocalIp()
{
	return IPAddress(_dhcpLocalIp);
}

IPAddress FlprogDhcpClass::getSubnetMask()
{
	return IPAddress(_dhcpSubnetMask);
}

IPAddress FlprogDhcpClass::getGatewayIp()
{
	return IPAddress(_dhcpGatewayIp);
}

IPAddress FlprogDhcpClass::getDhcpServerIp()
{
	return IPAddress(_dhcpDhcpServerIp);
}

IPAddress FlprogDhcpClass::getDnsServerIp()
{
	return IPAddress(_dhcpDnsServerIp);
}

void FlprogDhcpClass::printByte(char *buf, uint8_t n)
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
