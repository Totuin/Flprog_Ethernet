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

void FLProgOnBoardWifiInterface::disconnecSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        clearSoket(soket);
        return;
    }

    if (_sokets[soket].soketType == FLPROG_WIFI_SERVER_SOKET)
    {
        if (_sever[_sokets[soket].indexOnTypeArray] != 0)
        {
            _sever[_sokets[soket].indexOnTypeArray]->stop();
        }
    }
    if (_sokets[soket].soketType == FLPROG_WIFI_CLIENT_SOKET)
    {
        if (_client[_sokets[soket].indexOnTypeArray] != 0)
        {
            _client[_sokets[soket].indexOnTypeArray]->stop();
        }
    }
    if (_sokets[soket].soketType == FLPROG_WIFI_UDP_SOKET)
    {
        if (_udp[_sokets[soket].indexOnTypeArray] != 0)
        {
            _udp[_sokets[soket].indexOnTypeArray]->stop();
        }
    }
    clearSoket(soket);
}

uint8_t FLProgOnBoardWifiInterface::getServerTCPSoket(uint16_t port)
{
    if (!isReady())
    {
        return FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM;
    }
    uint8_t result = getFreeSoketIndex();
    if (result >= FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM)
    {
        return FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM;
    }
    uint8_t serverIndex = 0;
    while ((serverIndex < FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM) && (_sever[serverIndex] == 0))
    {
        serverIndex++;
    }
    if (serverIndex >= FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM)
    {
        return FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM;
    }
    _sever[serverIndex] = new WiFiServer(port);
    _sokets[result].soketType = FLPROG_WIFI_SERVER_SOKET;
    _sokets[result].indexOnTypeArray = serverIndex;
    _sokets[result].isUsed = true;
    return result;
};

bool FLProgOnBoardWifiInterface::isListenSoket(uint8_t soket)
{
    return checkOnUseSoket(soket);
}

uint8_t FLProgOnBoardWifiInterface::getFreeSoketIndex()
{
    for (uint8_t i = 0; i < FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM; i++)
    {
        if (!checkOnUseSoket(i))
        {
            clearSoket(i);
            return i;
        }
    }
    return FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM;
}

bool FLProgOnBoardWifiInterface::checkOnUseSoket(uint8_t soket)
{
    if (soket >= FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM)
    {
        return false;
    }
    if (!_sokets[soket].isUsed)
    {
        return false;
    }
    uint8_t type = _sokets[soket].soketType;
    if (type == FLPROG_WIFI_NOT_DEFINED_SOKET)
    {
        return false;
    }
    uint8_t index = _sokets[soket].indexOnTypeArray;
    if (index >= FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM)
    {
        return false;
    }
    if (type == FLPROG_WIFI_SERVER_SOKET)
    {
        return _sever[index] != 0;
    }
    if (type == FLPROG_WIFI_CLIENT_SOKET)
    {
        return _client[index] != 0;
    }
    if (type == FLPROG_WIFI_UDP_SOKET)
    {
        return _udp[index] != 0;
    }
    return false;
}

void FLProgOnBoardWifiInterface::clearSoket(uint8_t soket)
{
    if (soket >= FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM)
    {
        return;
    }
    _sokets[soket].isUsed = false;
    uint8_t type = _sokets[soket].soketType;
    if (type != FLPROG_WIFI_NOT_DEFINED_SOKET)
    {
        uint8_t index = _sokets[soket].indexOnTypeArray;
        if (index < FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM)
        {
            if (type == FLPROG_WIFI_SERVER_SOKET)
            {
                _sever[index] = 0;
            }
            if (type == FLPROG_WIFI_CLIENT_SOKET)
            {
                _client[index] = 0;
            }
            if (type == FLPROG_WIFI_UDP_SOKET)
            {
                _udp[index] = 0;
            }
        }
        _sokets[soket].soketType = FLPROG_WIFI_NOT_DEFINED_SOKET;
    }
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