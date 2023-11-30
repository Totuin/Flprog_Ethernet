#include "flprogESP8266WifiSoket.h"
 #ifdef ARDUINO_ARCH_ESP8266

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

void FLProgWifiSoket::beUDP(uint16_t port)
{
    close();
    _udp.begin(port);
    _soketType = FLPROG_WIFI_UDP_SOKET;
    _isUsed = true;
}

void FLProgWifiSoket::beCliendTcp(uint16_t port)
{
    close();
    _client.setLocalPortStart(port);
    _soketType = FLPROG_WIFI_CLIENT_SOKET;
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

int FLProgWifiSoket::connect(IPAddress ip, uint16_t port)
{

    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType == FLPROG_WIFI_CLIENT_SOKET)
    {
        return _client.connect(ip, port);
    }
    return 0;
}

uint8_t FLProgWifiSoket::status()
{
    /*  CLOSED      = 0,
      LISTEN      = 1,
      SYN_SENT    = 2,
      SYN_RCVD    = 3,
      ESTABLISHED = 4,
      FIN_WAIT_1  = 5,
      FIN_WAIT_2  = 6,
      CLOSE_WAIT  = 7,
      CLOSING     = 8,
      LAST_ACK    = 9,
      TIME_WAIT   = 10
    */
    if (!_isUsed)
    {
        return CLOSED;
    }

    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return CLOSED;
    }

    if (_soketType == FLPROG_WIFI_CLIENT_SOKET)
    {
        return _client.status();
    }
    return _server.status();
}

int FLProgWifiSoket::available()
{
    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType == FLPROG_WIFI_UDP_SOKET)
    {
        return _udp.available();
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

int FLProgWifiSoket::beginIpUDPPacket(uint8_t *addr, uint16_t port)
{
    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType != FLPROG_WIFI_UDP_SOKET)
    {
        return 0;
    }
    IPAddress ip = IPAddress(addr[0], addr[1], addr[2], addr[3]);
    if (_udp.beginPacket(ip, port))
    {
        return FLPROG_SUCCESS;
    }
    return FLPROG_ERROR;
}

int FLProgWifiSoket::endUDPPacket()
{
    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType != FLPROG_WIFI_UDP_SOKET)
    {
        return 0;
    }
    return _udp.endPacket();
}

int FLProgWifiSoket::parsePacket()
{
    if (!_isUsed)
    {
        return 0;
    }
    if (_soketType != FLPROG_WIFI_UDP_SOKET)
    {
        return 0;
    }
    return _udp.parsePacket();
}

#endif