#include "flprogEthernet.h"

void FlprogEthernetClass::pool()
{
	updateEthernetStatus();
	if (_status == FLPROG_READY_STATUS)
	{
		if (_isNeedReconect)
		{
			_status = FLPROG_NOT_REDY_STATUS;
		}
		return;
	}
	if (_status == FLPROG_WAIT_ETHERNET_HARDWARE_INIT_STATUS)
	{
		uint8_t hardStatus = hardware()->init();
		if (hardStatus == FLPROG_ERROR)
		{
			_status = FLPROG_NOT_REDY_STATUS;
			_errorCode = hardware()->getError();
			return;
		}
		if (hardStatus == FLPROG_WITE)
		{
			_status = FLPROG_WAIT_ETHERNET_HARDWARE_INIT_STATUS;
			return;
		}
		hardware()->setMACAddress(_macAddress);
		_isNeedReconect = false;
	}
	if (!hardware()->isInit())
	{

		_status = FLPROG_WAIT_ETHERNET_HARDWARE_INIT_STATUS;
		return;
	}
	if (_status == FLPROG_WAIT_ETHERNET_DHCP_STATUS)
	{
		begin();
		return;
	}

	if (!flprog::isTimer(_lastReconnectTime, _reconnectEthernetPeriod))
	{
		return;
	}
	if (_isDhcp || (_ip == INADDR_NONE))
	{
		begin();
		_lastReconnectTime = millis();
		_lastCheckEthernetStatusTime = millis();
		return;
	}
	if (_dnsIp == INADDR_NONE)
	{
		_dnsIp = _ip;
		_dnsIp[3] = 1;
	}
	if (_gatewayIp == INADDR_NONE)
	{
		_gatewayIp = _ip;
		_gatewayIp[3] = 1;
	}
	begin(_ip, _dnsIp, _gatewayIp, _subnetIp);
	_lastReconnectTime = millis();
	_lastCheckEthernetStatusTime = millis();
}

void FlprogEthernetClass::updateEthernetStatus()
{
	if (_status != FLPROG_READY_STATUS)
	{
		return;
	}
	if (!(flprog::isTimer(_lastCheckEthernetStatusTime, _checkEthernetStatusPeriod)))
	{
		return;
	}
	if ((hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE) || (linkStatus() == FLPROG_ETHERNET_LINK_OFF))
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_HARDWARE_INIT_ERROR;
		_isNeedReconect = true;
	}
	_lastCheckEthernetStatusTime = millis();
}

void FlprogEthernetClass::begin(unsigned long timeout, unsigned long responseTimeout)
{

	if (_status != FLPROG_WAIT_ETHERNET_DHCP_STATUS)
	{
		_status = FLPROG_WAIT_ETHERNET_DHCP_STATUS;
	}

	int result = _dhcp.beginWithDHCP(_macAddress, timeout, responseTimeout);

	if (result == FLPROG_ERROR)
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = _dhcp.getError();
		_isNeedReconect - true;
		return;
	}
	if (result == FLPROG_WITE)
	{
		_status = FLPROG_WAIT_ETHERNET_DHCP_STATUS;
		return;
	}

	if ((_dhcp.getLocalIp()) == INADDR_NONE)
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_DHCP_NOT_CORRECT_RESULT_ERROR;
		_isNeedReconect - true;
		return;
	}
	_ip = _dhcp.getLocalIp();
	_gatewayIp = _dhcp.getGatewayIp();
	_subnetIp = _dhcp.getSubnetMask();
	_dnsIp = _dhcp.getDnsServerIp();
	hardware()->setNetSettings(_ip, _gatewayIp, _subnetIp);
	_dns.begin(_dnsIp);
	hardware()->socketPortRand(micros());
	_status = FLPROG_READY_STATUS;
	_errorCode = FLPROG_NOT_ERROR;
	_isNeedReconect = false;
}

void FlprogEthernetClass::begin(IPAddress ip)
{
	IPAddress dns = ip;
	dns[3] = 1;
	begin(ip, dns);
}

void FlprogEthernetClass::begin(IPAddress ip, IPAddress dns)
{
	IPAddress gateway = ip;
	gateway[3] = 1;
	begin(ip, dns, gateway);
}

void FlprogEthernetClass::begin(IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(ip, dns, gateway, subnet);
}

void FlprogEthernetClass::begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	_ip = _ip;
	_dnsIp = dns;
	_subnetIp = subnet;
	_gatewayIp = gateway;
	// mac(_mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);
	_dns.begin(_dnsIp);
	hardware()->setNetSettings(_macAddress, _ip, _gatewayIp, _subnetIp);
	hardware()->socketPortRand(micros());
	_status = FLPROG_READY_STATUS;
	_errorCode = FLPROG_NOT_ERROR;
	_isNeedReconect = false;
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
		_ip = _dhcp.getLocalIp();
		_gatewayIp = _dhcp.getGatewayIp();
		_subnetIp = _dhcp.getSubnetMask();
		_dnsIp = _dhcp.getDnsServerIp();
		hardware()->setNetSettings(_ip, _gatewayIp, _subnetIp);
		break;
	default:
		break;
	}
	return rc;
}

//----------------------------FlprogW5100Ethernet-----------------------------

FLProgWiznetInterface::FLProgWiznetInterface(int pin, uint8_t bus)
{
	init(pin, bus);
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
