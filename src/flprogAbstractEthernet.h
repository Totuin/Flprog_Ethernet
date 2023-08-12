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

class FlprogAbstractEthernetHardware;
class FlprogDNSClient;

class FlprogAbstractEthernet
{
public:
    virtual FlprogAbstractEthernetHardware *hardware() = 0;
    virtual FlprogDNSClient *dnsClient() = 0;

    virtual void pool(){};
    virtual bool isBusy() { return busy; };
    virtual void isBusy(bool val) { busy = val; };
    void setBusy() { busy = true; };
    void resetBusy() { busy = false; };
    virtual bool isReady() { return lineStatus == FLPROG_ETHERNET_STATUS_READY; };

    IPAddress localIP() { return ip; };
    void localIP(IPAddress _ip);
    IPAddress dns() { return dnsIp; };
    void dns(IPAddress _ip);
    IPAddress subnet() { return subnetIp; };
    void subnet(IPAddress _ip);
    IPAddress gateway() { return gatewayIp; };
    void gateway(IPAddress _ip);

    void mac(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5);
    uint8_t *mac() { return macAddress; };
    virtual void mac(uint8_t *mac_address);

    void setDhcp();
    void resetDhcp();
    void dhcpMode(bool val);
    bool dhcpMode() { return isDhcp; };

protected:
    IPAddress ip = IPAddress(0, 0, 0, 0);
    IPAddress dnsIp = IPAddress(0, 0, 0, 0);
    IPAddress subnetIp = IPAddress(255, 255, 255, 0);
    IPAddress gatewayIp = IPAddress(0, 0, 0, 0);
    uint8_t macAddress[6] = {0, 0, 0, 0, 0, 0};
    uint8_t lineStatus = FLPROG_ETHERNET_STATUS_NOTREADY;
    bool isDhcp = true;
    bool needUpdateData = false;
    bool isNeedReconect = false;
    bool busy = false;
};