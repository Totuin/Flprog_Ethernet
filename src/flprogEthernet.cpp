#include "flprogEthernet.h"

void FlprogEthernetClass::pool()
{

	lineStatus = checkLineStatus();
	if (isNeedReconect)
	{
		lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
	}
	if (lineStatus == FLPROG_ETHERNET_STATUS_READY)
	{
		return;
	}
	if (!flprog::isTimer(lastReconnectTime, reconnectLinePeriod))
	{
		return;
	}
	if (isDhcp)
	{
		if (begin(macAddress))
		{
			lineStatus = FLPROG_ETHERNET_STATUS_READY;
		}
		else
		{
			lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
		}
	}
	else
	{
		if (ip == IPAddress(0, 0, 0, 0))
		{
			if (begin(macAddress))
			{
				lineStatus = FLPROG_ETHERNET_STATUS_READY;
			}
			else
			{
				lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
			}
		}
		else
		{
			if (dnsIp == IPAddress(0, 0, 0, 0))
			{
				dnsIp = ip;
				dnsIp[3] = 1;
			}
			if (gatewayIp == IPAddress(0, 0, 0, 0))
			{
				gatewayIp = ip;
				gatewayIp[3] = 1;
			}
			if (begin(macAddress, ip, dnsIp, gatewayIp, subnetIp))
			{
				lineStatus = FLPROG_ETHERNET_STATUS_READY;
			}
			else
			{
				lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
			}
		}
	}
	if (lineStatus == FLPROG_ETHERNET_STATUS_READY)
	{
		isNeedReconect = false;
	}
	lastReconnectTime = millis();
	lastCheckLineStatusTime = millis();
}

uint8_t FlprogEthernetClass::checkLineStatus()
{
	if (lineStatus == FLPROG_ETHERNET_STATUS_WHITE_DHCP)
	{
		return lineStatus;
	}
	if (flprog::isTimer(lastCheckLineStatusTime, checkLineStatusPeriod))
	{
		if ((hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE) || (linkStatus() == FLPROG_ETHERNET_LINK_OFF))
		{
			lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
		}
		else
		{
			lineStatus = FLPROG_ETHERNET_STATUS_READY;
		}

		lastCheckLineStatusTime = millis();
	}
	return lineStatus;
}

uint8_t FlprogEthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	if (hardware()->init() == 0)
		return 0;
	hardware()->setNetSettings(mac, IPAddress(0, 0, 0, 0));
	int ret = _dhcp.beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1)
	{
		ip = _dhcp.getLocalIp();
		gatewayIp = _dhcp.getGatewayIp();
		subnetIp = _dhcp.getSubnetMask();
		dnsIp = _dhcp.getDnsServerIp();
		hardware()->setNetSettings(ip, gatewayIp, subnetIp);
		_dns.begin(dnsIp);
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
	(void)dns;
	if (hardware()->init() == 0)
		return 0;
	hardware()->setNetSettings(mac, ip, gateway, subnet);
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
		ip = _dhcp.getLocalIp();
		gatewayIp = _dhcp.getGatewayIp();
		subnetIp = _dhcp.getSubnetMask();
		dnsIp = _dhcp.getDnsServerIp();
		hardware()->setNetSettings(ip, gatewayIp, subnetIp);
		break;
	default:
		break;
	}
	return rc;
}

//----------------------------FlprogW5100Ethernet-----------------------------

FLProgWiznetInterface::FLProgWiznetInterface(int pin, uint8_t bus)
{
	if (pin == -1)
	{
		_hardware.setSsPin(RT_HW_Base.device.spi.cs0);
	}
	else
	{
		_hardware.setSsPin(pin);
	}
	_hardware.setSpiBus(bus);
	_udp.setHatdware(&_hardware);
	_udp.setDNS(&_dns);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}

void FLProgWiznetInterface::init(int pin, uint8_t bus)
{
	if (pin == -1)
	{
		_hardware.setSsPin(RT_HW_Base.device.spi.cs0);
	}
	else
	{
		_hardware.setSsPin(pin);
	}
	_hardware.setSpiBus(bus);
	_udp.setHatdware(&_hardware);
	_udp.setDNS(&_dns);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}
