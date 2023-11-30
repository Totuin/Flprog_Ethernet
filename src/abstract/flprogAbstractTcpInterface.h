#pragma once
#include <Arduino.h>
#include "IPAddress.h"
#include "flprogUtilites.h"

class FLProgAbstractTcpInterface
{
public:
    // Основные методы для пользователей
    void setDhcp();
    void resetDhcp();
    void dhcpMode(bool val);
    bool isDhcp();

    IPAddress localIP() { return _ip; };
    void localIP(IPAddress ip);
    void localIP(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { localIP(IPAddress(ip0, ip1, ip2, ip3)); };

    IPAddress dns() { return _dnsIp; };
    void dns(IPAddress ip);
    void dns(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { dns(IPAddress(ip0, ip1, ip2, ip3)); };

    IPAddress subnet() { return _subnetIp; };
    void subnet(IPAddress ip);
    void subnet(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { subnet(IPAddress(ip0, ip1, ip2, ip3)); };

    IPAddress gateway() { return _gatewayIp; };
    void gateway(IPAddress ip);
    void gateway(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { gateway(IPAddress(ip0, ip1, ip2, ip3)); };

    void mac(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5);
    uint8_t *mac() { return _macAddress; };
    virtual void mac(uint8_t *mac_address);

    virtual uint8_t type() { return FLPROG_ANON_INTERFACE; };
    virtual bool isImitation() { return true; }

    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; };
    virtual bool isReady() { return _status == FLPROG_READY_STATUS; };

    // Внутрение методы библиотеки
    bool checkMac(uint8_t *mac);

    // API обязательное для реализации наследниками
    virtual uint8_t pool() = 0;
    virtual uint8_t soketConnected(uint8_t soket) = 0;
    virtual int readFromSoket(uint8_t soket) = 0;
    virtual int readFromSoket(uint8_t soket, uint8_t *buf, int16_t len) = 0;
    virtual size_t writeToSoket(uint8_t soket, const uint8_t *buffer, size_t size) = 0;
    virtual int parsePacketSocet(uint8_t soket) = 0;
    virtual int availableSoket(uint8_t soket) = 0;
    virtual void disconnecSoket(uint8_t soket) = 0;
    virtual uint8_t getClientTCPSoket(uint16_t port) = 0;
    virtual uint8_t getServerTCPSoket(uint16_t port) = 0;
    virtual uint8_t getUDPSoket(uint16_t port) = 0;
    virtual bool isListenSoket(uint8_t soket) = 0;
    virtual void closeSoket(uint8_t soket) = 0;
    virtual uint8_t startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port) = 0;
    virtual uint8_t sendUdpSoket(uint8_t soket) = 0;
    virtual uint16_t bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len) = 0;
    virtual int recvSoket(uint8_t soket, uint8_t *buf, int16_t len) = 0;
    virtual uint8_t peekSoket(uint8_t soket) = 0;
    virtual uint8_t beginMulticastSoket(IPAddress ip, uint16_t port) = 0;
    virtual uint8_t connectSoket(uint8_t soket, IPAddress ip, uint16_t port) = 0;
    virtual uint8_t isConnectStatusSoket(uint8_t soket) = 0;
    virtual uint8_t isCosedStatusSoket(uint8_t soket) = 0;
    virtual uint8_t statusSoket(uint8_t soket) = 0;
    virtual uint16_t localPortSoket(uint8_t soket) = 0;
    virtual IPAddress remoteIPSoket(uint8_t soket) = 0;
    virtual uint16_t remotePortSoket(uint8_t soket) = 0;
    virtual uint8_t maxSoketNum() = 0;
    virtual bool isInit() = 0;

protected:
    bool _isDhcp = true;
    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _errorCode = FLPROG_NOT_ERROR;
    bool _needUpdateData = false;
    IPAddress _ip = FLPROG_INADDR_NONE;
    IPAddress _dnsIp = FLPROG_INADDR_NONE;
    IPAddress _subnetIp = IPAddress(255, 255, 255, 0);
    IPAddress _gatewayIp = FLPROG_INADDR_NONE;
    uint8_t _macAddress[6] = {0, 0, 0, 0, 0, 0};
    bool _isNeedReconect = true;
};