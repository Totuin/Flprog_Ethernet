#include "flprogRP2040Wifi.h"
#ifdef ARDUINO_ARCH_RP2040
#ifdef ARDUINO_RASPBERRY_PI_PICO_W

//-----------------------------------------------FLProgOnBoardWifiInterface----------------------------------------------------------------

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
    if (_apWorkStatus)
    {
        _mode = WIFI_AP;
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
        return FLPROG_WAIT;
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
        WiFi.macAddress(_macAddress);
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
        WiFi.config(_ip, _gatewayIp, _subnetIp, _dnsIp);
    }
    WiFi.begin(_clientSsid, _clientPassword);
    _clientCurrentStatus = true;
    _status = FLPROG_WAIT_WIFI_CLIENT_CONNECT_STATUS;
    return FLPROG_WAIT;
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
        WiFi.macAddress(_apMacaddress);
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
        return;
    }
    _sokets[soket].disconnect();
}

void FLProgOnBoardWifiInterface::closeSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return;
    }
    _sokets[soket].close();
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
    _sokets[result].beServerTcp(port);
    return result;
}

uint8_t FLProgOnBoardWifiInterface::getClientTCPSoket(uint16_t port)
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
    _sokets[result].beCliendTcp(port);
    return result;
}

bool FLProgOnBoardWifiInterface::isListenSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return false;
    }
    return _sokets[soket].isListen();
}

uint8_t FLProgOnBoardWifiInterface::soketConnected(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].connected();
}

uint8_t FLProgOnBoardWifiInterface::connectSoket(uint8_t soket, IPAddress ip, uint16_t port)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }

    return _sokets[soket].connect(ip, port);
}

uint8_t FLProgOnBoardWifiInterface::isConnectStatusSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return false;
    }
    uint8_t status = _sokets[soket].status();
    if (status == 4)
    {
        return true;
    }
    if (status == 7)
    {
        return true;
    }
    return false;
}

uint8_t FLProgOnBoardWifiInterface::isCosedStatusSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return true;
    }
    return ((_sokets[soket].status()) == 0);
}

uint8_t FLProgOnBoardWifiInterface::statusSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].status();
}

int FLProgOnBoardWifiInterface::availableSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].available();
}

int FLProgOnBoardWifiInterface::readFromSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return -1;
    }
    return _sokets[soket].read();
}

int FLProgOnBoardWifiInterface::readFromSoket(uint8_t soket, uint8_t *buf, int16_t len)
{
    if (!checkOnUseSoket(soket))
    {
        return -1;
    }
    return _sokets[soket].read(buf, len);
}

size_t FLProgOnBoardWifiInterface::writeToSoket(uint8_t soket, const uint8_t *buf, size_t size)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].write(buf, size);
}

uint8_t FLProgOnBoardWifiInterface::peekSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].peek();
}

uint8_t FLProgOnBoardWifiInterface::getUDPSoket(uint16_t port)
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
    _sokets[result].beUDP(port);
    return result;
}

uint8_t FLProgOnBoardWifiInterface::startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].beginIpUDPPacket(addr, port);
}

uint8_t FLProgOnBoardWifiInterface::sendUdpSoket(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].endUDPPacket();
}

int FLProgOnBoardWifiInterface::parsePacketSocet(uint8_t soket)
{
    if (!checkOnUseSoket(soket))
    {
        return 0;
    }
    return _sokets[soket].parsePacket();
}

int FLProgOnBoardWifiInterface::recvSoket(uint8_t soket, uint8_t *buf, int16_t len)
{
    return readFromSoket(soket, buf, len);
}

uint8_t FLProgOnBoardWifiInterface::getFreeSoketIndex()
{
    for (uint8_t i = 0; i < FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM; i++)
    {
        if (!_sokets[i].isUsed())
        {
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
    return _sokets[soket].isUsed();
}

uint8_t FLProgOnBoardWifiInterface::resetToVoidVar(uint8_t soket)
{
    (void)soket;
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

uint8_t FLProgOnBoardWifiInterface::beginMulticastSoket(IPAddress ip, uint16_t port)
{
    (void)port;
    (void)ip;
    return 0;
}

IPAddress FLProgOnBoardWifiInterface::remoteIPSoket(uint8_t soket)
{
    (void)soket;
    return FLPROG_INADDR_NONE;
}

#endif
#endif