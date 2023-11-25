#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "flprogAbstactEthernetClasses.h"
#include "flprogDns.h"

class FLProgEthernetClient : public FLProgAbstactEthernetTCPChanel
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
   
    int available();
 
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


private:
    FLProgDNSClient _dns;

    uint8_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM; 
    uint32_t _timeout = 1000;
    uint32_t _startConnectTime;
    bool isInit = false;

    String _dnsCachedHost = "";
    IPAddress _dnsCachedIP = FLPROG_INADDR_NONE;
    uint32_t _dnsStartCachTime;
    uint32_t _dnsCacheStorageTime = 60000;
};
