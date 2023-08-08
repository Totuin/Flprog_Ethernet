#pragma once
#include <Arduino.h>

#ifndef MAX_SOCK_NUM
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define MAX_SOCK_NUM 4
#else
#define MAX_SOCK_NUM 8
#endif
#endif

#define FLPROG_ETHERNET_LINK_UNKNOWN 0
#define FLPROG_ETHERNET_LINK_ON 1
#define FLPROG_ETHERNET_LINK_OFF 2

#define FLPROG_ETHERNET_NO_HARDWARE 0
#define FLPROG_ETHERNET_W5100 1
#define FLPROG_ETHERNET_W5200 2
#define FLPROG_ETHERNET_W5500 3

class FlprogAbstractEthernetHardware;
class FlprogDNSClient;

class FlprogAbstractEthernet 
{
public:
    virtual FlprogAbstractEthernetHardware *hardware() = 0;
    virtual FlprogDNSClient *dnsClient() = 0;
};