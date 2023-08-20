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

#define FLPROG_ETHERNET_LINK_UNKNOWN 0
#define FLPROG_ETHERNET_LINK_ON 1
#define FLPROG_ETHERNET_LINK_OFF 2

#define FLPROG_ETHERNET_NO_HARDWARE 0
#define FLPROG_ETHERNET_W5100 1
#define FLPROG_ETHERNET_W5200 2
#define FLPROG_ETHERNET_W5500 3

#define FLPROG_ETHERNET_STATUS_NOTREADY 0
#define FLPROG_ETHERNET_STATUS_READY 1
#define FLPROG_ETHERNET_STATUS_WHITE_DHCP 2

class FLProgAbstractEthernetHardware;
class FLProgDNSClient;

class FlprogAbstractEthernet : public FLProgAbstractTcpInterface
{
public:
    virtual FLProgAbstractEthernetHardware *hardware() = 0;
    virtual FLProgDNSClient *dnsClient() = 0;
    virtual bool isReady() { return lineStatus == FLPROG_ETHERNET_STATUS_READY; };
    virtual uint8_t type() { return FLPROG_ETHERNET_INTERFACE; }

protected:
    uint8_t lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
    bool needUpdateData = false;
};