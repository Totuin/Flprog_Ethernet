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

uint8_t flporgConvertTmpBytes[4];
uint8_t *flprogConvertIp(IPAddress adress)
{
	flporgConvertTmpBytes[0] = adress[0];
	flporgConvertTmpBytes[1] = adress[1];
	flporgConvertTmpBytes[2] = adress[2];
	flporgConvertTmpBytes[3] = adress[3];
	return flporgConvertTmpBytes;
};

IPAddress FlprogEthernetClass::_dnsServerAddress;
FlprogDhcpClass *FlprogEthernetClass::_dhcp = NULL;

int FlprogEthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	static FlprogDhcpClass s_dhcp;
	_dhcp = &s_dhcp;

	// Initialise the basic info
	if (FlprogW5100.init() == 0)
		return 0;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	FlprogW5100.setMACAddress(mac);
	FlprogW5100.setIPAddress(IPAddress(0, 0, 0, 0));
	// FlprogW5100.setIPAddress(flprogConvertIp(IPAddress(0, 0, 0, 0)));
	SPI.endTransaction();

	// Now try to get our config info from a DHCP server
	int ret = _dhcp->beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1)
	{
		// We've successfully found a DHCP server and got our configuration
		// info, so set things accordingly
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		// FlprogW5100.setIPAddress(flprogConvertIp(_dhcp->getLocalIp()));
		FlprogW5100.setIPAddress(_dhcp->getLocalIp());
		FlprogW5100.setGatewayIp(_dhcp->getGatewayIp());
		FlprogW5100.setSubnetMask(_dhcp->getSubnetMask());
		SPI.endTransaction();
		_dnsServerAddress = _dhcp->getDnsServerIp();
		socketPortRand(micros());
	}
	return ret;
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip)
{
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns = ip;
	dns[3] = 1;
	begin(mac, ip, dns);
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = ip;
	gateway[3] = 1;
	begin(mac, ip, dns, gateway);
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac, ip, dns, gateway, subnet);
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	if (FlprogW5100.init() == 0)
		return;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	FlprogW5100.setMACAddress(mac);
#if ARDUINO > 106 || TEENSYDUINO > 121
	FlprogW5100.setIPAddress(flprogConvertIp(ip));
	FlprogW5100.setGatewayIp(flprogConvertIp(gateway));
	FlprogW5100.setSubnetMask(flprogConvertIp(subnet));
#else
	// FlprogW5100.setIPAddress(flprogConvertIp(ip));
	FlprogW5100.setIPAddress(ip);
	FlprogW5100.setGatewayIp(gateway);
	FlprogW5100.setSubnetMask(subnet);
#endif
	SPI.endTransaction();
	_dnsServerAddress = dns;
}

void FlprogEthernetClass::init(uint8_t sspin)
{
	FlprogW5100.setSS(sspin);
}

FlprogEthernetLinkStatus FlprogEthernetClass::linkStatus()
{
	switch (FlprogW5100.getLinkStatus())
	{
	case UNKNOWN:
		return Unknown;
	case LINK_ON:
		return LinkON;
	case LINK_OFF:
		return LinkOFF;
	default:
		return Unknown;
	}
}

FlprogEthernetHardwareStatus FlprogEthernetClass::hardwareStatus()
{
	switch (FlprogW5100.getChip())
	{
	case 51:
		return EthernetW5100;
	case 52:
		return EthernetW5200;
	case 55:
		return EthernetW5500;
	default:
		return EthernetNoHardware;
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
			// FlprogW5100.setIPAddress(flprogConvertIp(_dhcp->getLocalIp()));
			FlprogW5100.setIPAddress(_dhcp->getLocalIp());
			FlprogW5100.setGatewayIp(_dhcp->getGatewayIp());
			FlprogW5100.setSubnetMask(_dhcp->getSubnetMask());
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
	FlprogW5100.getMACAddress(mac_address);
	SPI.endTransaction();
}

IPAddress FlprogEthernetClass::localIP()
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = FlprogW5100.getIPAddress();
	SPI.endTransaction();
	return result;
}

IPAddress FlprogEthernetClass::subnetMask()
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = FlprogW5100.getSubnetMask();
	SPI.endTransaction();
	return result;
}

IPAddress FlprogEthernetClass::gatewayIP()
{
	IPAddress ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = FlprogW5100.getGatewayIp();
	SPI.endTransaction();
	return result;
}

void FlprogEthernetClass::setMACAddress(const uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	FlprogW5100.setMACAddress(mac_address);
	SPI.endTransaction();
}

void FlprogEthernetClass::setLocalIP(const IPAddress local_ip)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = local_ip;
	// FlprogW5100.setIPAddress(flprogConvertIp(ip));
	FlprogW5100.setIPAddress(ip);
	SPI.endTransaction();
}

void FlprogEthernetClass::setSubnetMask(const IPAddress subnet)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = subnet;
	FlprogW5100.setSubnetMask(ip);
	SPI.endTransaction();
}

void FlprogEthernetClass::setGatewayIP(const IPAddress gateway)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = gateway;
	FlprogW5100.setGatewayIp(ip);
	SPI.endTransaction();
}

void FlprogEthernetClass::setRetransmissionTimeout(uint16_t milliseconds)
{
	if (milliseconds > 6553)
		milliseconds = 6553;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	FlprogW5100.setRetransmissionTime(milliseconds * 10);
	SPI.endTransaction();
}

void FlprogEthernetClass::setRetransmissionCount(uint8_t num)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	FlprogW5100.setRetransmissionCount(num);
	SPI.endTransaction();
}

FlprogEthernetClass FlprogEthernet;
