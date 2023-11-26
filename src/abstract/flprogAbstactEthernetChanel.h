#pragma once
#include <Arduino.h>
#include "IPAddress.h"
#include "flprogUtilites.h"
#include "flprogAbstractTcpInterface.h"

#define flprogEthernetHtons(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0xFF))
#define flprogEthernetNtohs(x) flprogEthernetHtons(x)

#define flprogEthernetHtonl(x) (((x) << 24 & 0xFF000000UL) | \
                                ((x) << 8 & 0x00FF0000UL) |  \
                                ((x) >> 8 & 0x0000FF00UL) |  \
                                ((x) >> 24 & 0x000000FFUL))
#define flporgEthernetNtohl(x) flprogEthernetHtonl(x)

class FLProgAbstactEthernetChanel : public Print
{
public:
    virtual void setSourse(FLProgAbstractTcpInterface *sourse) { _sourse = sourse; };

    virtual size_t write(uint8_t byte) { return write(&byte, 1); };

    int read(char *buffer, size_t len) { return read((uint8_t *)buffer, len); };
    uint8_t getStatus() { return _status; };
    uint8_t getError() { return _errorCode; };

    void stop();
    void flush(){};

    virtual size_t write(const uint8_t *buf, size_t size) = 0;

protected:
    uint8_t checkReadySourse();
    FLProgAbstractTcpInterface *_sourse;
    uint8_t _errorCode = FLPROG_NOT_ERROR;
    uint8_t _status = FLPROG_NOT_REDY_STATUS;
    uint8_t _sockindex = 255;
};
