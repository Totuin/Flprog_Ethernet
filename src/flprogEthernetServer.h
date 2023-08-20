#pragma once
#include <Arduino.h>
#include "hardware/flprogAbstractEthernetHardware.h"
#include "flprogAbstractEthernet.h"
#include "flprogDns.h"
#include "flprogEthernetClient.h"

// class FLProgEthernetClient;

class FLProgEthernetServer : public FLProgAbstractTcpServer
{
public:
    FLProgEthernetServer(FlprogAbstractEthernet *sourse, uint16_t port);
    FLProgEthernetClient accept();
    virtual void begin();
    virtual void begin(uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual operator bool();
    uint16_t server_port[FLPROG_ETHERNET_MAX_SOCK_NUM];
    virtual void setClient() { cl = accept(); };
    virtual Client *client() { return &cl; };
    FLProgEthernetClient available() { return accept(); };

private:
    uint16_t _port;
    FLProgAbstractEthernetHardware *_hardware;
    FLProgDNSClient *_dns;
    FLProgEthernetClient cl;
};
