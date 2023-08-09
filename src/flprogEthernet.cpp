#include "flprogEthernet.h"

uint8_t FlprogEthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{

	if (hardware()->init() == 0)
		return 0;
	hardware()->setNetSettings(mac, IPAddress(0, 0, 0, 0));
	int ret = _dhcp.beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1)
	{
		hardware()->setNetSettings(_dhcp.getLocalIp(), _dhcp.getGatewayIp(), _dhcp.getSubnetMask());
		_dnsServerAddress = _dhcp.getDnsServerIp();
		_dns.begin(_dnsServerAddress);
		hardware()->socketPortRand(micros());
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
	if (hardware()->init() == 0)
		return 0;
	hardware()->setNetSettings(mac, ip, gateway, subnet);
	_dnsServerAddress = dns;
	return 1;
}

uint8_t FlprogEthernetClass::hardwareStatus()
{
	switch (hardware()->getChip())
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
		hardware()->setNetSettings(_dhcp.getLocalIp(), _dhcp.getGatewayIp(), _dhcp.getSubnetMask());
		_dnsServerAddress = _dhcp.getDnsServerIp();
		break;
	default:
		break;
	}
	return rc;
}

//----------------------------FlprogW5100Ethernet-----------------------------

FlprogW5100Interface::FlprogW5100Interface()
{
	_hardware.setSsPin(10);
	_hardware.setSPI(new FLProgSPI(0));
	_udp.setHatdware(&_hardware);
	_udp.setDNS(&_dns);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}

FlprogW5100Interface::FlprogW5100Interface(FLProgSPI *spi, int pin)
{
	_hardware.setSsPin(pin);
	_hardware.setSPI(spi);
	_udp.setHatdware(&_hardware);
	_udp.setDNS(&_dns);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}

void FlprogW5100Interface::init(FLProgSPI *spi, int sspin)
{
	_hardware.setSsPin(sspin);
	_hardware.setSPI(spi);
	_udp.setHatdware(&_hardware);
	_udp.setDNS(&_dns);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}

bool FlprogW5100Interface::isReady()
{
	if (hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE)
	{
		return false;
	}
	if (localIP() == IPAddress(0, 0, 0, 0))
	{
		return false;
	}
	uint8_t link = linkStatus();
	if (link == FLPROG_ETHERNET_LINK_OFF)
	{
		return false;
	}
	return true;
}

bool FlprogW5100Interface::isBusy()
{
	if (!isReady())
	{
		return true;
	}

	return busy;
}
