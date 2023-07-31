#pragma once
#include <Arduino.h>
#include "Server.h"
#include "flprogW5100.h"
#include "flprogDns.h"
#include "flprogEthernet.h"
#include "flprogEthernetClient.h"


class FlprogEthernetServer : public Server
{
public:
    FlprogEthernetServer(FlprogEthernetClass *sourse, uint16_t port);
    FlprogEthernetClient available();
    FlprogEthernetClient accept();
    virtual void begin();
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual operator bool();
    using Print::write;
    uint16_t server_port[MAX_SOCK_NUM];

private:
    uint16_t _port;
    FlprogW5100Class *_hardware;
    FlprogDNSClient *_dns;
};
