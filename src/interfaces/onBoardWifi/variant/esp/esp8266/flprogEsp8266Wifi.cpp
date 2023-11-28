#include "flprogEsp8266Wifi.h"

// #ifdef FLPROG_ESP8266_WIFI_ON_BOARD_TCP_DEVICE

bool FLProgOnBoardWifiInterface::isReady()
{
    if (_apWorkStatus)
    {
        if (_apiCurrentStatus)
        {
            return true;
        }
    }
    return _status == FLPROG_READY_STATUS;
}

bool FLProgOnBoardWifiInterface::clientIsReady()
{
    if (!_clientCurrentStatus)
    {
        return false;
    }
    return _status == FLPROG_READY_STATUS;
}

uint8_t FLProgOnBoardWifiInterface::pool()
{
    if (_apIsNeedReconect)
    {
        connect();
    }
    if (_isNeedReconect)
    {
        return connect();
    }
    return checkConnectStatus();
}

uint8_t FLProgOnBoardWifiInterface::checkConnectStatus()
{
    if (_clientWorkStatus)
    {
        if (_clientCurrentStatus)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                _isNeedReconect = true;
            }
        }
        else
        {
            _isNeedReconect = true;
        }
    }
    else
    {
        if (_clientCurrentStatus)
        {
            _isNeedReconect = true;
        }
    }
    if (_apiCurrentStatus != _apWorkStatus)
    {
        _apIsNeedReconect = true;
    }
    return FLPROG_SUCCESS;
}

uint8_t FLProgOnBoardWifiInterface::connect()
{
    if (WiFi.getAutoConnect() != true)
    {
        WiFi.setAutoConnect(true);
    }
    WiFi.setAutoReconnect(true);

    if (_apWorkStatus)
    {
        if (_clientWorkStatus)
        {
            _mode = WIFI_AP_STA;
        }
        else
        {
            _mode = WIFI_AP;
        }
    }
    else
    {
        if (_clientWorkStatus)
        {
            _mode = WIFI_STA;
        }
        else
        {
            _mode = WIFI_OFF;
        }
    }
    WiFi.mode(_mode);
    apReconnect();
    return clientReconnect();
}

uint8_t FLProgOnBoardWifiInterface::clientReconnect()
{
    if (_status == FLPROG_WAIT_WIFI_CLIENT_CONNECT_STATUS)
    {
        if (_mode == WIFI_AP)
        {
            if (_clientCurrentStatus)
            {
                WiFi.disconnect();
                _clientCurrentStatus = false;
            }
            _status = FLPROG_READY_STATUS;
            _isNeedReconect = false;
            return FLPROG_SUCCESS;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            _ip = WiFi.localIP();
            _subnetIp = WiFi.subnetMask();
            _gatewayIp = WiFi.gatewayIP();
            _dnsIp = WiFi.dnsIP();
            _status = FLPROG_READY_STATUS;
            _isNeedReconect = false;
            return FLPROG_SUCCESS;
        }
        return FLPROG_WITE;
    }
    if (_mode == WIFI_AP)
    {
        if (_clientCurrentStatus)
        {
            _clientCurrentStatus = false;
            WiFi.disconnect();
        }
        _isNeedReconect = false;
        return FLPROG_SUCCESS;
    }
    if (!_isNeedReconect)
    {
        return FLPROG_SUCCESS;
    }
    WiFi.disconnect();
    if (checkMac(_macAddress))
    {
        wifi_set_macaddr(STATION_IF, _macAddress);
    }
    if (isDhcp())
    {
        WiFi.config(FLPROG_INADDR_NONE, FLPROG_INADDR_NONE, FLPROG_INADDR_NONE, FLPROG_INADDR_NONE);
    }
    else
    {

        if (_gatewayIp == FLPROG_INADDR_NONE)
        {
            _gatewayIp = _ip;
            _gatewayIp[3] = 1;
        }
        if (_dnsIp == FLPROG_INADDR_NONE)
        {
            _dnsIp = _ip;
            _dnsIp[3] = 1;
        }
        WiFi.config(_ip, _gatewayIp, _subnetIp, _dnsIp, _dnsIp);
    }
    WiFi.begin(_clientSsid, _clientPassword);
    _clientCurrentStatus = true;
    _status = FLPROG_WAIT_WIFI_CLIENT_CONNECT_STATUS;
    return FLPROG_WITE;
}

uint8_t FLProgOnBoardWifiInterface::apReconnect()
{
    if (_mode == WIFI_STA)
    {
        if (_apiCurrentStatus)
        {
            _apiCurrentStatus = false;
            WiFi.softAPdisconnect();
        }
        _apIsNeedReconect = false;
        return FLPROG_SUCCESS;
    }
    if (!_apIsNeedReconect)
    {
        return FLPROG_SUCCESS;
    }
    WiFi.softAPdisconnect();
    if (checkMac(_apMacaddress))
    {
        wifi_set_macaddr(SOFTAP_IF, _apMacaddress);
    }
    if (_apIp == FLPROG_INADDR_NONE)
    {
        _apIp = IPAddress(192, 168, 0, 1);
    }
    if (_apGatewayIp == FLPROG_INADDR_NONE)
    {
        _apGatewayIp = _apIp;
        _apGatewayIp[3] = 1;
    }
    if (_apDnsIp == FLPROG_INADDR_NONE)
    {
        _apDnsIp = _apIp;
        _apDnsIp[3] = 1;
    }
    WiFi.softAPConfig(_apIp, _apGatewayIp, _apSubnetIp);
    WiFi.softAP(_apSsid, _apPassword);
    _apIsNeedReconect = false;
    _apiCurrentStatus = true;
    return FLPROG_SUCCESS;
}

uint8_t FLProgOnBoardWifiInterface::resetToVoidVar(uint8_t soket)
{
    (void)soket;
    return 0;
}

int FLProgOnBoardWifiInterface::readFromSoket(uint8_t soket, uint8_t *buf, int16_t len)
{
    (void)soket;
    (void)buf;
    (void)len;
    return -1;
}

size_t FLProgOnBoardWifiInterface::writeToSoket(uint8_t soket, const uint8_t *buffer, size_t size)
{
    (void)soket;
    (void)buffer;
    (void)size;
    return 0;
}

uint8_t FLProgOnBoardWifiInterface::startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port)
{
    (void)soket;
    (void)addr;
    (void)port;
    return 0;
}

uint16_t FLProgOnBoardWifiInterface::bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len)
{
    (void)soket;
    (void)offset;
    (void)buf;
    (void)len;
    return 0;
}

int FLProgOnBoardWifiInterface::recvSoket(uint8_t soket, uint8_t *buf, int16_t len)
{
    (void)soket;
    (void)buf;
    (void)len;
    return -1;
}

uint8_t FLProgOnBoardWifiInterface::beginMulticastSoket(IPAddress ip, uint16_t port)
{
    (void)port;
    (void)ip;
    return 0;
}

uint8_t FLProgOnBoardWifiInterface::connectSoket(uint8_t soket, IPAddress ip, uint16_t port)
{
    (void)soket;
    (void)port;
    (void)ip;
    return 0;
}

IPAddress FLProgOnBoardWifiInterface::remoteIPSoket(uint8_t soket)
{
    (void)soket;
    return FLPROG_INADDR_NONE;
}

// #endif