#pragma once
#include <Arduino.h>
#include "hardware/flprogAbstractEthernetHardware.h"
#include "flprogAbstractEthernet.h"
#include "flprogDns.h"
#include "flprogEthernetClient.h"

// class FLProgEthernetClient;

class FLProgEthernetServer : public FLProgAbstractTcpServer
{
public:
    FLProgEthernetServer(FlprogAbstractEthernet *sourse, uint16_t port);
    virtual uint8_t pool();

    virtual uint8_t begin();
    virtual uint8_t setPort(uint16_t port);
    int available();
    int read();

    void print(String s) { write(s.c_str(), s.length()); };
    void println() { print("\n"); };
    void println(String s);

    virtual size_t write(uint8_t);
    virtual size_t write(const uint8_t *buf, size_t size);

    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; }

    void setCallback(size_t func) { _callbackFunction = func; };

private:
    uint16_t _port;
    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _errorCode = FLPROG_NOT_ERROR;
    uint16_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
    FlprogAbstractEthernet *_sourse;
    void (*_callbackFunction)(void) = 0;
};
