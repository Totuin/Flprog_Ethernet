#pragma once
#include <Arduino.h>
#include "Server.h"
#include "hardware/flprogAbstractEthernetHardware.h"
#include "flprogAbstractEthernet.h"
#include "flprogDns.h"
#include "flprogEthernetClient.h"

// class FlprogEthernetClient;

class FlprogEthernetServer : public Server
{
public:
    FlprogEthernetServer(FlprogAbstractEthernet *sourse, uint16_t port);
    FlprogEthernetClient available();
    FlprogEthernetClient accept();
    virtual void begin();
    virtual void begin(uint16_t port);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual operator bool();
    using Print::write;
    uint16_t server_port[FLPROG_ETHERNET_MAX_SOCK_NUM];

private:
    uint16_t _port;
    FlprogAbstractEthernetHardware *_hardware;
    FlprogDNSClient *_dns;
};
