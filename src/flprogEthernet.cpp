#include "flprogEthernet.h"

void FlprogEthernetClass::pool()
{
	updateEthernetStatus();

	if (ethernetStatus == FLPROG_READY_STATUS)
	{
		return;
	}
	if (ethernetStatus == FLPROG_HARDWARE_INIT_STATUS)
	{
		uint8_t hardStatus = hardware()->init();
		if (hardStatus == FLPROG_EHERNET_ERROR)
		{
			ethernetStatus = FLPROG_NOT_REDY_STATUS;
			return;
		}
		if (hardStatus == FLPROG_WAIT_STATUS)
		{
			ethernetStatus = FLPROG_HARDWARE_INIT_STATUS;
			return;
		}
		hardware()->setMACAddress(macAddress);
		isNeedReconect = false;
	}

	if (!hardware()->isInit())
	{
		ethernetStatus = FLPROG_HARDWARE_INIT_STATUS;
		return;
	}

	if (ethernetStatus == FLPROG_ETHERNET_STATUS_WHITE_DHCP)
	{
		begin(macAddress);
		return;
	}
	if (isNeedReconect)
	{
		ethernetStatus = FLPROG_NOT_REDY_STATUS;
	}

	if (!flprog::isTimer(lastReconnectTime, reconnectEthernetPeriod))
	{
		return;
	}
	if (isDhcp)
	{
		begin(macAddress);
	}
	else
	{
		if (ip == IPAddress(0, 0, 0, 0))
		{
			begin(macAddress);
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
			begin(macAddress, ip, dnsIp, gatewayIp, subnetIp);
		}
	}
	lastReconnectTime = millis();
	lastCheckEthernetStatusTime = millis();
}

void FlprogEthernetClass::updateEthernetStatus()
{
	if (ethernetStatus == FLPROG_READY_STATUS)
	{

		if (flprog::isTimer(lastCheckEthernetStatusTime, checkEthernetStatusPeriod))
		{
			if ((hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE) || (linkStatus() == FLPROG_ETHERNET_LINK_OFF))
			{
				ethernetStatus = FLPROG_NOT_REDY_STATUS;
			}
			else
			{
				ethernetStatus = FLPROG_READY_STATUS;
			}
			lastCheckEthernetStatusTime = millis();
		}
	}
}

void FlprogEthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{

	if (ethernetStatus == FLPROG_READY_STATUS)
	{
		ethernetStatus = FLPROG_ETHERNET_STATUS_WHITE_DHCP;
	}
	int ret = _dhcp.beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 0)
	{
		ethernetStatus = FLPROG_NOT_REDY_STATUS;
		isNeedReconect - true;
		return;
	}
	if (ret == 2)
	{
		ethernetStatus = FLPROG_ETHERNET_STATUS_WHITE_DHCP;
		return;
	}
	ip = _dhcp.getLocalIp();
	gatewayIp = _dhcp.getGatewayIp();
	subnetIp = _dhcp.getSubnetMask();
	dnsIp = _dhcp.getDnsServerIp();
	hardware()->setNetSettings(ip, gatewayIp, subnetIp);
	_dns.begin(dnsIp);
	hardware()->socketPortRand(micros());
	ethernetStatus = FLPROG_READY_STATUS;
	isNeedReconect = false;
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip)
{
	IPAddress dns = ip;
	dns[3] = 1;
	begin(mac, ip, dns);
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
	IPAddress gateway = ip;
	gateway[3] = 1;
	begin(mac, ip, dns, gateway);
}

void FlprogEthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac, ip, dns, gateway, subnet);
}

void FlprogEthernetClass::begin(uint8_t *_mac, IPAddress _ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	ip = _ip;
	dnsIp = dns;
	subnetIp = subnet;
	gatewayIp = gateway;
	mac(_mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);
	_dns.begin(dnsIp);
	hardware()->setNetSettings(macAddress, ip, gatewayIp, subnetIp);
	hardware()->socketPortRand(micros());
	ethernetStatus = FLPROG_READY_STATUS;
	isNeedReconect = false;
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
	_udp.setSourse(this);
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
	_udp.setSourse(this);
	_udp.setDNS(&_dns);
	_dhcp.setUDP(&_udp);
	_dns.setUDP(&_udp);
}
