#include "flprogWiznetInterface.h"

FLProgWiznetInterface::FLProgWiznetInterface(int pinCs, uint8_t bus)
{
    init(pinCs, bus);
}

FLProgWiznetInterface::FLProgWiznetInterface()
{
    init(-1, 255);
}
void FLProgWiznetInterface::init(int pinCs, uint8_t bus)
{
    _hardware.setPinCs(pinCs);
    _hardware.setSpiBus(bus);
    _dhcp.setSourse(this);
}

void FLProgWiznetInterface::pool()
{
    updateEthernetStatus();

    if (_status == FLPROG_READY_STATUS)
    {
        if (_isNeedReconect)
        {
            _status = FLPROG_NOT_REDY_STATUS;
        }
        maintain();
        return;
    }

    if (_status == FLPROG_WAIT_ETHERNET_HARDWARE_INIT_STATUS)
    {
        uint8_t result = _hardware.init();
        if (result == FLPROG_ERROR)
        {
            _status = FLPROG_NOT_REDY_STATUS;
            _errorCode = _hardware.getError();
            return;
        }
        if (result == FLPROG_WITE)
        {
            _status = FLPROG_WAIT_ETHERNET_HARDWARE_INIT_STATUS;
            return;
        }
        _hardware.setMACAddress(_macAddress);
        _isNeedReconect = false;
    }
    if (!_hardware.isInit())
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
    if (isDhcp())
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

void FLProgWiznetInterface::updateEthernetStatus()
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

void FLProgWiznetInterface::begin()
{
    if (linkStatus() != FLPROG_ETHERNET_LINK_ON)
    {
        _status = FLPROG_NOT_REDY_STATUS;
        _errorCode = FLPROG_ETHERNET_LINK_OFF_ERROR;
        _isNeedReconect = true;
        return;
    }
    if (_status != FLPROG_WAIT_ETHERNET_DHCP_STATUS)
    {
        _status = FLPROG_WAIT_ETHERNET_DHCP_STATUS;
    }

    uint8_t result = _dhcp.beginWithDHCP(_timeout, _responseTimeout);
    if (result == FLPROG_ERROR)
    {
        _status = FLPROG_NOT_REDY_STATUS;
        _errorCode = _dhcp.getError();
        _isNeedReconect = true;
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
        _isNeedReconect = true;
        return;
    }
    _ip = _dhcp.getLocalIp();
    _gatewayIp = _dhcp.getGatewayIp();
    _subnetIp = _dhcp.getSubnetMask();
    _dnsIp = _dhcp.getDnsServerIp();
    _hardware.setNetSettings(_ip, _gatewayIp, _subnetIp);
    _hardware.socketPortRand(micros());
    _status = FLPROG_READY_STATUS;
    _errorCode = FLPROG_NOT_ERROR;
    _isNeedReconect = false;
    _startMaintainTime = millis();
}

uint8_t FLProgWiznetInterface::maintain()
{
    if (!isDhcp())
    {
        return FLPROG_SUCCESS;
    }
    if (!_isMaintainMode)
    {
        if (!flprog::isTimer(_startMaintainTime, _maintainPeriod))
        {
            return FLPROG_SUCCESS;
        }
    }
    if (linkStatus() != FLPROG_ETHERNET_LINK_ON)
    {
        return FLPROG_SUCCESS;
    }
    _isMaintainMode = true;
    uint8_t result = _dhcp.beginWithDHCP(_timeout, _responseTimeout);
    if (result == FLPROG_ERROR)
    {
        _isMaintainMode = false;
        _errorCode = _dhcp.getError();
        _startMaintainTime = millis();
        return FLPROG_SUCCESS;
    }
    if (result == FLPROG_WITE)
    {
        return FLPROG_SUCCESS;
    }
    if ((_dhcp.getLocalIp()) != FLPROG_INADDR_NONE)
    {
        _ip = _dhcp.getLocalIp();
        _gatewayIp = _dhcp.getGatewayIp();
        _subnetIp = _dhcp.getSubnetMask();
        _dnsIp = _dhcp.getDnsServerIp();
        _hardware.setNetSettings(_ip, _gatewayIp, _subnetIp);
        _hardware.socketPortRand(micros());
    }
    _startMaintainTime = millis();
    _isMaintainMode = false;
    return FLPROG_SUCCESS;
}

void FLProgWiznetInterface::begin(IPAddress ip)
{
    IPAddress dns = ip;
    dns[3] = 1;
    begin(ip, dns);
}

void FLProgWiznetInterface::begin(IPAddress ip, IPAddress dns)
{
    IPAddress gateway = ip;
    gateway[3] = 1;
    begin(ip, dns, gateway);
}

void FLProgWiznetInterface::begin(IPAddress ip, IPAddress dns, IPAddress gateway)
{
    IPAddress subnet(255, 255, 255, 0);
    begin(ip, dns, gateway, subnet);
}

void FLProgWiznetInterface::begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
    if (linkStatus() != FLPROG_ETHERNET_LINK_ON)
    {
        _status = FLPROG_NOT_REDY_STATUS;
        _errorCode = FLPROG_ETHERNET_LINK_OFF_ERROR;
        _isNeedReconect = true;
        return;
    }
    _ip = ip;
    _dnsIp = dns;
    _subnetIp = subnet;
    _gatewayIp = gateway;
    _hardware.setNetSettings(_macAddress, _ip, _gatewayIp, _subnetIp);
    _hardware.socketPortRand(micros());
    _status = FLPROG_READY_STATUS;
    _errorCode = FLPROG_NOT_ERROR;
    _isNeedReconect = false;
}

uint8_t FLProgWiznetInterface::hardwareStatus()
{
    return _hardware.getChip();
}
