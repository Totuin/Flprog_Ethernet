#include "flprogWifiSoket.h"
#ifdef FLPROG_CAN_USE_WIFI_SOKET

void FLProgWifiSoket::disconnect()
{
    if (_soketType == FLPROG_WIFI_SERVER_SOKET)
    {
        _client.stop();
        _client = _server.accept();
        return;
    }
    close();
}

void FLProgWifiSoket::close()
{
    _client.stop();
    _server.stop();
    _udp.stop();
    _isUsed = false;
    _soketType = FLPROG_WIFI_NOT_DEFINED_SOKET;
}

void FLProgWifiSoket::beServerTcp(uint16_t port)
{
    close();
    _server.begin(port);
    _soketType = FLPROG_WIFI_SERVER_SOKET;
    _isUsed = true;
}

bool FLProgWifiSoket::isListen()
{
    if (!_isUsed)
    {
        return false;
    }
    if (_soketType == FLPROG_WIFI_SERVER_SOKET)
    {
        return (_server.status() != CLOSED);
    }
    return true;
}

uint8_t FLProgWifiSoket::connected()
{
    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType == FLPROG_WIFI_SERVER_SOKET)
    {
        if (_client.connected())
        {
            return _client.connected();
        }
        _client.stop();
        _client = _server.accept();
        return _client.available();
    }
    if (_soketType == FLPROG_WIFI_CLIENT_SOKET)
    {
        return _client.connected();
    }
    return 0;
}

int FLProgWifiSoket::available()
{
    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return 0;
    }
    if (_soketType == FLPROG_WIFI_CLIENT_SOKET)
    {
        return _client.available();
    }
    if (_client.available())
    {
        return _client.available();
    }
    _client.stop();
    _client = _server.accept();
    return _client.available();
}

int FLProgWifiSoket::read()
{
    if (!_isUsed)
    {
        return -1;
    }
    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return _udp.read();
    }
    return _client.read();
}

int FLProgWifiSoket::read(uint8_t *buf, int16_t len)
{
    if (!_isUsed)
    {
        return -1;
    }
    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return _udp.read(buf, len);
    }
    return _client.read(buf, len);
}

size_t FLProgWifiSoket::write(const uint8_t *buf, size_t len)
{
    if (!_isUsed)
    {
        return -1;
    }
    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return _udp.write(buf, len);
    }
    return _client.write(buf, len);
}

uint8_t FLProgWifiSoket::peek()
{
    if (!_isUsed)
    {
        return -1;
    }
    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return _udp.peek();
    }
    return _client.peek();
}

#endif