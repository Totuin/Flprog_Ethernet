#pragma once
#include <Arduino.h>

#include "flprogDns.h"

class FLProgEthernetClient : public Print
{
public:
    FLProgEthernetClient(){};
    FLProgEthernetClient(FLProgAbstractTcpInterface *sourse);
    FLProgEthernetClient(FLProgAbstractTcpInterface *sourse, uint8_t s);
    void setServerData(uint8_t s);
    void init(FLProgAbstractTcpInterface *sourse);
    uint8_t status();
    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    int available();
    int read();
    int read(uint8_t *buf, size_t size);
    int peek();
    void flush();
    void stop();
    uint8_t connected();
    uint8_t getSocketNumber() const { return _sockindex; }
    uint16_t localPort();
    IPAddress remoteIP();
    uint16_t remotePort();
    void setConnectionTimeout(uint16_t timeout) { _timeout = timeout; }
    void setDnsCacheStorageTime(uint32_t time) { _dnsCacheStorageTime = time; }
    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; }

private:
    FLProgAbstractTcpInterface *_sourse;
    FLProgDNSClient _dns;

    uint8_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM; 
    uint32_t _timeout = 1000;
    uint32_t _startConnectTime;
    bool isInit = false;

    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _errorCode = FLPROG_NOT_ERROR;

    String _dnsCachedHost = "";
    IPAddress _dnsCachedIP = FLPROG_INADDR_NONE;
    uint32_t _dnsStartCachTime;
    uint32_t _dnsCacheStorageTime = 60000;
};
