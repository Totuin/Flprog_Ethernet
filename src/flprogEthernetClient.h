#pragma once
#include <Arduino.h>
#include "Client.h"
#include "hardware/flprogAbstractEthernetHardware.h"
#include "flprogAbstractEthernet.h"
#include "flprogDns.h"

class FLProgEthernetClient : public Client
{
public:
    FLProgEthernetClient(){};
    FLProgEthernetClient(FlprogAbstractEthernet *sourse);
    FLProgEthernetClient(FlprogAbstractEthernet *sourse, uint8_t s);

    virtual void init(FlprogAbstractEthernet *sourse);
    uint8_t status();
    virtual int connect(IPAddress ip, uint16_t port);
    virtual int connect(const char *host, uint16_t port);
    virtual int availableForWrite(void);
    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);
    virtual int available();
    virtual int read();
    virtual int read(uint8_t *buf, size_t size);
    virtual int peek();
    virtual void flush();
    virtual void stop();
    virtual uint8_t connected();
    virtual operator bool() { return _sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM; }
    virtual bool operator==(const bool value) { return bool() == value; }
    virtual bool operator!=(const bool value) { return bool() != value; }
    virtual bool operator==(const FLProgEthernetClient &);
    virtual bool operator!=(const FLProgEthernetClient &rhs) { return !this->operator==(rhs); }
    uint8_t getSocketNumber() const { return _sockindex; }
    virtual uint16_t localPort();
    virtual IPAddress remoteIP();
    virtual uint16_t remotePort();
    virtual void setConnectionTimeout(uint16_t timeout) { _timeout = timeout; }
    friend class FLProgEthernetServer;
    using Print::write;

    void setDnsCacheStorageTime(uint32_t time) { _dnsCacheStorageTime = time; }
    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; }

private:
    FlprogAbstractEthernet *_sourse;
    FLProgDNSClient *_dns;

    uint8_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM; // FLPROG_ETHERNET_MAX_SOCK_NUM means client not in use
    uint32_t _timeout = 1000;
    uint32_t _startConnectTime;
    bool isInit = false;

    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _errorCode = FLPROG_NOT_ERROR;

    String _dnsCachedHost = "";
    IPAddress _dnsCachedIP = FLPROG_INADDR_NONE;
    uint32_t _dnsStartCachTime;
    uint32_t _dnsCacheStorageTime = 60000;
};
