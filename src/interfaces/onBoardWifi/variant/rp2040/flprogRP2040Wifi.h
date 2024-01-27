#pragma once
#ifdef ARDUINO_ARCH_RP2040
#ifdef ARDUINO_RASPBERRY_PI_PICO_W

#include "flprogUtilites.h"
#include "WiFi.h"
#include "flprogRP2040WifiSoket.h"
#include "../../flprogOnBoardWifiInterface.h"

class FLProgOnBoardWifiInterface : public FLProgAbstracttWiFiInterface
{
public:
    virtual uint8_t pool();
    virtual bool isImitation() { return false; };
    virtual bool isReady();
    virtual bool clientIsReady();
    virtual bool apIsReady() { return _apiCurrentStatus; };

    virtual uint8_t type() { return FLPROG_ETHERNET_ON_BOARD_WIFI_ANON; };
    virtual void disconnecSoket(uint8_t soket);
    virtual uint8_t getServerTCPSoket(uint16_t port);
    virtual bool isListenSoket(uint8_t soket);
    virtual void closeSoket(uint8_t soket);
    uint8_t soketConnected(uint8_t soket);
    virtual int availableSoket(uint8_t soket);
    virtual int readFromSoket(uint8_t soket);
    virtual int readFromSoket(uint8_t soket, uint8_t *buf, int16_t len);
    virtual size_t writeToSoket(uint8_t soket, const uint8_t *buffer, size_t size);
    virtual uint8_t peekSoket(uint8_t soket);
    virtual uint8_t getUDPSoket(uint16_t port);
    virtual uint8_t startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port);
    virtual uint8_t sendUdpSoket(uint8_t soket);
    int parsePacketSocet(uint8_t soket);
    virtual uint8_t getClientTCPSoket(uint16_t port);
    virtual uint8_t connectSoket(uint8_t soket, IPAddress ip, uint16_t port);
    virtual uint8_t statusSoket(uint8_t soket);
    virtual uint8_t isConnectStatusSoket(uint8_t soket);
    virtual uint8_t isCosedStatusSoket(uint8_t soket);

    virtual uint8_t maxSoketNum() { return FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM; };
    virtual bool isInit() { return true; };

    // Необходимые заглушки
    virtual uint16_t bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len);
    virtual int recvSoket(uint8_t soket, uint8_t *buf, int16_t len);

    // Заглушки которые надо допилить.....
    virtual uint8_t beginMulticastSoket(IPAddress ip, uint16_t port);
    virtual uint16_t localPortSoket(uint8_t soket) { return resetToVoidVar(soket); };
    virtual IPAddress remoteIPSoket(uint8_t soket);
    virtual uint16_t remotePortSoket(uint8_t soket) { return resetToVoidVar(soket); };

private:
    uint8_t connect();
    uint8_t clientReconnect();
    uint8_t apReconnect();
    uint8_t checkConnectStatus();

    uint8_t getFreeSoketIndex();
    bool checkOnUseSoket(uint8_t soket);

    uint8_t resetToVoidVar(uint8_t soket);
    bool _apiCurrentStatus = false;
    bool _clientCurrentStatus = false;
    WiFiMode_t _mode = WIFI_AP_STA;

    FLProgWifiSoket _sokets[FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM];
};

#endif
#endif