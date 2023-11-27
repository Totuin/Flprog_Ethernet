#include "flprogOnBoardWifiInterface.h"

void FLProgAbstracttWiFiInterface::setApSsid(String ssid)
{
    if (ssid.equals(String(_apSsid)))
    {
        return;
    }
    ssid.toCharArray(_apSsid, 40);
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::setApPassword(String password)
{
    if (password.equals(String(_apPassword)))
    {
        return;
    }
    password.toCharArray(_apPassword, 40);
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::setClientSsidd(String ssid)
{
    if (ssid.equals(String(_clientSsid)))
    {
        return;
    }
    ssid.toCharArray(_clientSsid, 40);
    _isNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::setClientPassword(String password)
{
    if (password.equals(String(_clientPassword)))
    {
        return;
    }
    password.toCharArray(_clientPassword, 40);
    _isNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::apMac(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5)
{
    if (flprog::applyMac(m0, m1, m2, m3, m4, m5, _apMacaddress))
    {
        _apIsNeedReconect = true;
    }
}

void FLProgAbstracttWiFiInterface::apLocalIP(IPAddress ip)
{
    if (ip == _apIp)
    {
        return;
    }
    _apIp = ip;
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::apDns(IPAddress ip)
{
    if (ip == _apDnsIp)
    {
        return;
    }
    _apDnsIp = ip;
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::apSubnet(IPAddress ip)
{
    if (ip == _apSubnetIp)
    {
        return;
    }
    _apSubnetIp = ip;
    _apIsNeedReconect = true;
}


void FLProgAbstracttWiFiInterface::apGateway(IPAddress ip)
{
    if (ip == _apGatewayIp)
    {
        return;
    }
    _apGatewayIp = ip;
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::clientOn()
{
    if (_clientWorkStatus)
    {
        return;
    }
    _clientWorkStatus = true;
    _isNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::clientOff()
{
    if (!_clientWorkStatus)
    {
        return;
    }
    _clientWorkStatus = false;
    _isNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::clientMode(bool val)
{
    if (!_clientWorkStatus == val)
    {
        return;
    }
    _clientWorkStatus = val;
    _isNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::apOn()
{
    if (_apWorkStatus)
    {
        return;
    }
    _apWorkStatus = true;
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::apOff()
{
    if (!_apWorkStatus)
    {
        return;
    }
    _apWorkStatus = false;
    _apIsNeedReconect = true;
}

void FLProgAbstracttWiFiInterface::apMode(bool val)
{
    if (_apWorkStatus == val)
    {
        return;
    }
    _apWorkStatus = val;
    _apIsNeedReconect = true;
}



