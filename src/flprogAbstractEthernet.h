#pragma once
#include <Arduino.h>
#include "IPAddress.h"
#include "flprogUtilites.h"

#ifndef FLPROG_ETHERNET_MAX_SOCK_NUM
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define FLPROG_ETHERNET_MAX_SOCK_NUM 4
#else
#define FLPROG_ETHERNET_MAX_SOCK_NUM 8
#endif
#endif

class FLProgAbstractEthernetHardware;
class FLProgDNSClient;

class FlprogAbstractEthernet : public FLProgAbstractTcpInterface
{
public:
    virtual FLProgAbstractEthernetHardware *hardware() = 0;
    virtual FLProgDNSClient *dnsClient() = 0;
    uint8_t getStatus() { return ethernetStatus; }
    virtual bool isReady() { return ethernetStatus == FLPROG_READY_STATUS; };
    virtual uint8_t type() { return FLPROG_ETHERNET_INTERFACE; }

protected:
    uint8_t ethernetStatus = FLPROG_NOT_REDY_STATUS;
    bool needUpdateData = false;
};