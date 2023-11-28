#pragma once
#include "flprogUtilites.h"
#include "../../../flprogOnBoardWifiInterface.h"

// #ifdef FLPROG_ESP8266_WIFI_ON_BOARD_TCP_DEVICE

class FLProgOnBoardWifiInterface : public FLProgAbstracttWiFiInterface
{
public:
    virtual uint8_t pool();
    virtual bool isImitation() { return false; }
    virtual bool isReady();
    virtual bool clientIsReady();
    virtual bool apIsReady() { return _apiCurrentStatus; };

    virtual uint8_t type() { return FLPROG_ETHERNET_ON_BOARD_WIFI_ANON; };

    virtual uint8_t soketConnected(uint8_t soket) { return resetToVoidVar(soket); };
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
    uint8_t connect();
    uint8_t clientReconnect();
    uint8_t apReconnect();
    uint8_t checkConnectStatus();
    uint8_t resetToVoidVar(uint8_t soket);
    bool _apiCurrentStatus = false;
    bool _clientCurrentStatus = false;
    WiFiMode _mode = WIFI_AP_STA;
};

// #endif