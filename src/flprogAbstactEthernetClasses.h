#pragma once
#include <Arduino.h>
#include "IPAddress.h"
#include "flprogUtilites.h"

#ifndef FLPROG_EHERNET_UTIL_H
#define FLPROG_EHERNET_UTIL_H

#define flprogEthernetHtons(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0xFF))
#define flprogEthernetNtohs(x) flprogEthernetHtons(x)

#define flprogEthernetHtonl(x) (((x) << 24 & 0xFF000000UL) | \
                                ((x) << 8 & 0x00FF0000UL) |  \
                                ((x) >> 8 & 0x0000FF00UL) |  \
                                ((x) >> 24 & 0x000000FFUL))
#define flporgEthernetNtohl(x) flprogEthernetHtonl(x)
#endif

class FLProgAbstractEthernetHardware
{
public:
    virtual uint8_t init() = 0;
    virtual uint8_t getLinkStatus() = 0;
    virtual void setGatewayIp(IPAddress addr) = 0;
    virtual IPAddress getGatewayIp() = 0;
    virtual void setSubnetMask(IPAddress addr) = 0;
    virtual IPAddress getSubnetMask() = 0;
    virtual void setMACAddress(const uint8_t *addr) = 0;
    virtual void getMACAddress(uint8_t *addr) = 0;
    virtual void setIPAddress(IPAddress addr) = 0;
    virtual IPAddress getIPAddress() = 0;
    virtual void setRetransmissionTime(uint16_t timeout) = 0;
    virtual void setRetransmissionCount(uint8_t retry) = 0;
    virtual uint8_t getChip() = 0;
    virtual bool isInit() { return false; }

    // утилиты
    virtual void setNetSettings(uint8_t *mac, IPAddress ip) = 0;
    virtual void setNetSettings(IPAddress ip, IPAddress gateway, IPAddress subnet) = 0;
    virtual void setNetSettings(uint8_t *mac, IPAddress ip, IPAddress gateway, IPAddress subnet) = 0;
    virtual void setOnlyMACAddress(const uint8_t *mac_address) = 0;
    virtual void setOnlyLocalIP(const IPAddress local_ip) = 0;
    virtual void setOnlySubnetMask(const IPAddress subnet) = 0;
    virtual void setOnlyGatewayIP(const IPAddress gateway) = 0;
    virtual IPAddress localIP() = 0;
    virtual IPAddress subnetMask() = 0;
    virtual IPAddress gatewayIP() = 0;
    virtual void MACAddress(uint8_t *mac_address) = 0;

    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; }

protected:
    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _errorCode = FLPROG_ETHERNET_HARDWARE_INIT_ERROR;
};

class FLProgAbstractTcpInterface
{
public:
    void setDhcp();
    void resetDhcp();
    void dhcpMode(bool val);
    bool dhcpMode() { return _isDhcp; };

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

    virtual bool isBusy() { return _busy; };
    virtual void isBusy(bool busy) { _busy = busy; };

    void setBusy() { _busy = true; };
    void resetBusy() { _busy = false; };

    virtual uint8_t type() { return FLPROG_ANON_INTERFACE; };
    virtual bool isImitation() { return true; }

    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; };
    virtual bool isReady() { return _status == FLPROG_READY_STATUS; };

    // API обязательное для реализации наследниками
    virtual void pool() = 0;
    virtual uint8_t soketConnected(uint8_t soket) = 0;
    virtual int readFromSoket(uint8_t soket) = 0;
    virtual int readFromSoket(uint8_t soket, uint8_t *buf, int16_t len) = 0;
    virtual size_t writeToSoket(const uint8_t *buffer, size_t size, uint8_t soket) = 0;
    virtual int availableSoket(uint8_t soket) = 0;
    virtual void disconnecSoket(uint8_t soket) = 0;
    virtual uint8_t getTCPSoket(uint16_t port) = 0;
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
    virtual uint8_t peekSoket(uint8_t soket) = 0;
    virtual uint8_t statusSoket(uint8_t soket) = 0;
    virtual uint16_t localPortSoket(uint8_t soket) = 0;
    virtual IPAddress remoteIPSoket(uint8_t soket) = 0;
    virtual uint16_t remotePortSoket(uint8_t soket) = 0;

protected:
    bool _busy = false;
    bool _isDhcp = true;
    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _errorCode = FLPROG_NOT_ERROR;
    bool _needUpdateData = false;
    IPAddress _ip = INADDR_NONE;
    IPAddress _dnsIp = INADDR_NONE;
    IPAddress _subnetIp = IPAddress(255, 255, 255, 0);
    IPAddress _gatewayIp = INADDR_NONE;
    uint8_t _macAddress[6] = {0, 0, 0, 0, 0, 0};
    bool _isNeedReconect = true;
};

class FLProgAbstactEthernetChanel : public Print
{
public:
    virtual void setSourse(FLProgAbstractTcpInterface *sourse) { _sourse = sourse; };
    virtual size_t write(uint8_t byte) { return write(&byte, 1); };

    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; };

    void flush(){};

protected:
    FLProgAbstractTcpInterface *_sourse;
    uint8_t _errorCode = FLPROG_NOT_ERROR;
    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
};

class FLProgAbstactEthernetTCPChanel : public FLProgAbstactEthernetChanel
{
public:
    virtual size_t write(const uint8_t *buffer, size_t size) { return _sourse->writeToSoket(buffer, size, _sockindex); };
    int read() { return _sourse->readFromSoket(_sockindex); };

protected:
};

class FLProgAbstactEthernetUDPChanel : public FLProgAbstactEthernetChanel
{
public:
    virtual size_t write(const uint8_t *buffer, size_t size);

    int read();
    int read(uint8_t *buffer, size_t len);
    int read(char *buffer, size_t len) { return read((uint8_t *)buffer, len); };
    virtual int available() { return _remaining; };

    IPAddress remoteIP() { return _remoteIP; };
    uint16_t remotePort() { return _remotePort; };
    uint16_t localPort() { return _port; }

    uint8_t begin(uint16_t port);
    int beginPacket(IPAddress ip, uint16_t port);
    int parsePacket();
    int endPacket();
    void stop();
    int peek();

protected:
    uint16_t _offset;
    uint16_t _remaining;
    IPAddress _remoteIP;
    uint16_t _remotePort;
    uint16_t _port;
};
