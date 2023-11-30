#pragma once
#include "flprogUtilites.h"
#include "../../flprogOnBoardWifiInterface.h"

#ifdef FLPROG_ANON_WIFI_ON_BOARD_TCP_DEVICE

class FLProgOnBoardWifiInterface : public FLProgAbstracttWiFiInterface
{
public:
    virtual uint8_t pool() { return FLPROG_SUCCESS; };
    virtual bool isImitation() { return true; }
    virtual bool isReady() { return false; };
    virtual uint8_t type() { return FLPROG_ETHERNET_ON_BOARD_WIFI_ANON; };

    virtual uint8_t soketConnected(uint8_t soket) { return resetToVoidVar(soket); };

    int parsePacketSocet(uint8_t soket);
    virtual int readFromSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual int readFromSoket(uint8_t soket, uint8_t *buf, int16_t len);
    virtual size_t writeToSoket(uint8_t soket, const uint8_t *buffer, size_t size);
    virtual int availableSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual void disconnecSoket(uint8_t soket) { (void)soket; };
    virtual uint8_t getClientTCPSoket(uint16_t port) { return resetToVoidVar(port); };
    virtual uint8_t getServerTCPSoket(uint16_t port) { return resetToVoidVar(port); };
    virtual uint8_t getUDPSoket(uint16_t port) { return resetToVoidVar(port); };
    virtual bool isListenSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual void closeSoket(uint8_t soket) { (void)soket; };
    virtual uint8_t startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port);
    virtual uint8_t sendUdpSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual uint16_t bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len);
    virtual int recvSoket(uint8_t soket, uint8_t *buf, int16_t len);
    virtual uint8_t peekSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual uint8_t beginMulticastSoket(IPAddress ip, uint16_t port);
    virtual uint8_t connectSoket(uint8_t soket, IPAddress ip, uint16_t port);
    virtual uint8_t isConnectStatusSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual uint8_t isCosedStatusSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual uint8_t statusSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual uint16_t localPortSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual IPAddress remoteIPSoket(uint8_t soket);
    virtual uint16_t remotePortSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual uint8_t maxSoketNum() { return 0; };
    virtual bool isInit() { return false; };

private:
    uint8_t resetToVoidVar(uint8_t soket);
};

#endif