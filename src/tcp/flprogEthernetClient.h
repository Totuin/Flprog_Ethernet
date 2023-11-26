#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "../abstract/flprogAbstactEthernetTCPChanel.h"
#include "../udp/flprogDns.h"

class FLProgEthernetClient : public FLProgAbstactEthernetTCPChanel
{
public:
    FLProgEthernetClient(){};
    FLProgEthernetClient(FLProgAbstractTcpInterface *sourse);

    void init(FLProgAbstractTcpInterface *sourse);
    uint8_t status();
    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);

    uint8_t connected();
    uint8_t getSocketNumber() const { return _sockindex; };
    void setConnectionTimeout(uint16_t timeout) { _timeout = timeout; };
    void setDnsCacheStorageTime(uint32_t time) { _dns.setDnsCacheStorageTime(time); };

private:
    FLProgDNSClient _dns;
    uint32_t _timeout = 1000;
    uint32_t _startConnectTime;
    bool isInit = false;
};
