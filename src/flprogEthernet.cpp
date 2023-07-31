/* Copyright 2018 Paul Stoffregen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "flprogEthernet.h"
#include "utility/flprogW5100.h"
#include "flprogDhcp.h"
#include "flprogDns.h"

uint8_t FlprogEthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	_dhcp = FlprogDhcpClass(this);
	_dns = FlprogDNSClient(this);
	_udp = FlprogEthernetUDP(this);
	_hardware = FlprogW5100Class(this);
	_dns->begin(_dnsServerAddress);
	// Initialise the basic info
	if (_hardware->init() == 0)
		return 0;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->setMACAddress(mac);
	_hardware->setIPAddress(IPAddress(0, 0, 0, 0));
	SPI.endTransaction();

	// Now try to get our config info from a DHCP server
	int ret = _dhcp->beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1)
	{
		// We've successfully found a DHCP server and got our configuration
		// info, so set things accordingly
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		// _hardware->setIPAddress(flprogConvertIp(_dhcp->getLocalIp()));
		_hardware->setIPAddress(_dhcp->getLocalIp());
		_hardware->setGatewayIp(_dhcp->getGatewayIp());
		_hardware->setSubnetMask(_dhcp->getSubnetMask());
		SPI.endTransaction();
		_dnsServerAddress = _dhcp->getDnsServerIp();
		socketPortRand(micros());
	}
	return ret;
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip)
{
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns = ip;
	dns[3] = 1;
	return begin(mac, ip, dns);
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = ip;
	gateway[3] = 1;
	return begin(mac, ip, dns, gateway);
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	return begin(mac, ip, dns, gateway, subnet);
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	if (_hardware->init() == 0)
		return 0;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->setMACAddress(mac);
	_hardware->setIPAddress(ip);
	_hardware->setGatewayIp(gateway);
	_hardware->setSubnetMask(subnet);
	SPI.endTransaction();
	_dnsServerAddress = dns;
	return 1;
}
/*
void FlprogEthernetClass::init(uint8_t sspin)
{
	_hardware->setSS(sspin);
}
*/
uint8_t FlprogEthernetClass::linkStatus()
{
	return _hardware->getLinkStatus();
}

uint8_t FlprogEthernetClass::hardwareStatus()
{
	switch (_hardware->getChip())
	{
	case 51:
		return FLPROG_ETHERNET_W5100;
	case 52:
		return FLPROG_ETHERNET_W5200;
	case 55:
		return FLPROG_ETHERNET_W5500;
	default:
		return FLPROG_ETHERNET_NO_HARDWARE;
	}
}

int FlprogEthernetClass::maintain()
{
	int rc = FLPROG_DHCP_CHECK_NONE;
	if (_dhcp != NULL)
	{
		// we have a pointer to dhcp, use it
		rc = _dhcp->checkLease();
		switch (rc)
		{
		case FLPROG_DHCP_CHECK_NONE:
			// nothing done
			break;
		case FLPROG_DHCP_CHECK_RENEW_OK:
		case FLPROG_DHCP_CHECK_REBIND_OK:
			// we might have got a new IP.
			SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
			// _hardware->setIPAddress(flprogConvertIp(_dhcp->getLocalIp()));
			_hardware->setIPAddress(_dhcp->getLocalIp());
			_hardware->setGatewayIp(_dhcp->getGatewayIp());
			_hardware->setSubnetMask(_dhcp->getSubnetMask());
			SPI.endTransaction();
			_dnsServerAddress = _dhcp->getDnsServerIp();
			break;
		default:
			// this is actually an error, it will retry though
			break;
		}
	}
	return rc;
}

void FlprogEthernetClass::MACAddress(uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->getMACAddress(mac_address);
	SPI.endTransaction();
}

IPAddress FlprogEthernetClass::localIP()
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = _hardware->getIPAddress();
	SPI.endTransaction();
	return result;
}

IPAddress FlprogEthernetClass::subnetMask()
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = _hardware->getSubnetMask();
	SPI.endTransaction();
	return result;
}

IPAddress FlprogEthernetClass::gatewayIP()
{
	IPAddress ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = _hardware->getGatewayIp();
	SPI.endTransaction();
	return result;
}

void FlprogEthernetClass::setMACAddress(const uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->setMACAddress(mac_address);
	SPI.endTransaction();
}

void FlprogEthernetClass::setLocalIP(const IPAddress local_ip)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = local_ip;
	// _hardware->setIPAddress(flprogConvertIp(ip));
	_hardware->setIPAddress(ip);
	SPI.endTransaction();
}

void FlprogEthernetClass::setSubnetMask(const IPAddress subnet)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = subnet;
	_hardware->setSubnetMask(ip);
	SPI.endTransaction();
}

void FlprogEthernetClass::setGatewayIP(const IPAddress gateway)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = gateway;
	_hardware->setGatewayIp(ip);
	SPI.endTransaction();
}

void FlprogEthernetClass::setRetransmissionTimeout(uint16_t milliseconds)
{
	if (milliseconds > 6553)
		milliseconds = 6553;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->setRetransmissionTime(milliseconds * 10);
	SPI.endTransaction();
}

void FlprogEthernetClass::setRetransmissionCount(uint8_t num)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->setRetransmissionCount(num);
	SPI.endTransaction();
}

FlprogEthernetClass FlprogEthernet;
