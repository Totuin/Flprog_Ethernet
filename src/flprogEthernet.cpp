#include "flprogEthernet.h"

FlprogEthernetClass::FlprogEthernetClass()
{
	_hardware.setSS(10);
	_udp.setHatdware(&_hardware);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	_dns.begin(_dnsServerAddress);
	if (_hardware.init() == 0)
		return 0;
	_hardware.setNetSettings(mac, IPAddress(0, 0, 0, 0));
	int ret = _dhcp.beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1)
	{
		_hardware.setNetSettings(_dhcp.getLocalIp(), _dhcp.getGatewayIp(), _dhcp.getSubnetMask());
		_dnsServerAddress = _dhcp.getDnsServerIp();
		_hardware.socketPortRand(micros());
	}
	return ret;
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip)
{
	IPAddress dns = ip;
	dns[3] = 1;
	return begin(mac, ip, dns);
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
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
	if (_hardware.init() == 0)
		return 0;
	_hardware.setNetSettings(mac, ip, gateway, subnet);
	_dnsServerAddress = dns;
	return 1;
}

void FlprogEthernetClass::init(uint8_t sspin)
{
	_hardware.setSS(sspin);
}

uint8_t FlprogEthernetClass::linkStatus()
{
	return _hardware.getLinkStatus();
}

uint8_t FlprogEthernetClass::hardwareStatus()
{
	switch (_hardware.getChip())
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
	rc = _dhcp.checkLease();
	switch (rc)
	{
	case FLPROG_DHCP_CHECK_NONE:
		// nothing done
		break;
	case FLPROG_DHCP_CHECK_RENEW_OK:
	case FLPROG_DHCP_CHECK_REBIND_OK:
		_hardware.setNetSettings(_dhcp.getLocalIp(), _dhcp.getGatewayIp(), _dhcp.getSubnetMask());
		_dnsServerAddress = _dhcp.getDnsServerIp();
		break;
	default:
		break;
	}
	return rc;
}

void FlprogEthernetClass::MACAddress(uint8_t *mac_address)
{
	_hardware.MACAddress(mac_address);
}

IPAddress FlprogEthernetClass::localIP()
{
	return _hardware.localIP();
}

IPAddress FlprogEthernetClass::subnetMask()
{
	return _hardware.subnetMask();
}

IPAddress FlprogEthernetClass::gatewayIP()
{
	return _hardware.gatewayIP();
}

void FlprogEthernetClass::setMACAddress(const uint8_t *mac_address)
{
	_hardware.setOnlyMACAddress(mac_address);
}

void FlprogEthernetClass::setLocalIP(const IPAddress local_ip)
{
	_hardware.setOnlyLocalIP(local_ip);
}

void FlprogEthernetClass::setSubnetMask(const IPAddress subnet)
{
	_hardware.setOnlySubnetMask(subnet);
}

void FlprogEthernetClass::setGatewayIP(const IPAddress gateway)
{
	_hardware.setOnlyGatewayIP(gateway);
}

void FlprogEthernetClass::setRetransmissionTimeout(uint16_t milliseconds)
{
	_hardware.setRetransmissionTime(milliseconds);
}

void FlprogEthernetClass::setRetransmissionCount(uint8_t num)
{
	_hardware.setRetransmissionCount(num);
}
