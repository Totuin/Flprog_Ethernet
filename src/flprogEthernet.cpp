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

		uint8_t result = hardware()->init();
		if (result == FLPROG_ERROR)
		{
			_status = FLPROG_NOT_REDY_STATUS;
			_errorCode = hardware()->getError();
			return;
		}
		if (result == FLPROG_WITE)
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
	if (_isDhcp || (_ip == FLPROG_INADDR_NONE))
	{
		begin();
		_lastReconnectTime = millis();
		_lastCheckEthernetStatusTime = millis();
		return;
	}
	else
	{
		if (_dnsIp == FLPROG_INADDR_NONE)
		{
			_dnsIp = _ip;
			_dnsIp[3] = 1;
		}
		if (_gatewayIp == FLPROG_INADDR_NONE)
		{
			_gatewayIp = _ip;
			_gatewayIp[3] = 1;
		}
		begin(_ip, _dnsIp, _gatewayIp, _subnetIp);
	}
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
	if ((hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE) || (linkStatus() != FLPROG_ETHERNET_LINK_ON))
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_HARDWARE_INIT_ERROR;
		_isNeedReconect = true;
	}
	_lastCheckEthernetStatusTime = millis();
}

void FlprogEthernetClass::begin(unsigned long timeout, unsigned long responseTimeout)
{

	if (linkStatus() != FLPROG_ETHERNET_LINK_ON)
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_LINK_OFF_ERROR;
		_isNeedReconect - true;
		return;
	}
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

	if ((_dhcp.getLocalIp()) == FLPROG_INADDR_NONE)
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
	if (linkStatus() != FLPROG_ETHERNET_LINK_ON)
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_LINK_OFF_ERROR;
		_isNeedReconect - true;
		return;
	}
	_ip = ip;
	_dnsIp = dns;
	_subnetIp = subnet;
	_gatewayIp = gateway;
	hardware()->setNetSettings(_macAddress, _ip, _gatewayIp, _subnetIp);
	hardware()->socketPortRand(micros());
	_status = FLPROG_READY_STATUS;
	_errorCode = FLPROG_NOT_ERROR;
	_isNeedReconect = false;
}

uint8_t FlprogEthernetClass::hardwareStatus()
{
	return hardware()->getChip();
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

FLProgWiznetInterface::FLProgWiznetInterface(int pinCs, uint8_t bus)
{
	init(pinCs, bus);
}

void FLProgWiznetInterface::init(int pinCs, uint8_t bus)
{
	_hardware.setPinCs(pinCs);
	_hardware.setSpiBus(bus);
	_dhcp.setSourse(this);
}
