#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "flprogAbstactEthernetClasses.h"

class FLProgEthernetServer : public FLProgAbstactEthernetTCPChanel
{
public:
    FLProgEthernetServer(FLProgAbstractTcpInterface *sourse, uint16_t port = 80);
    uint8_t pool();
    uint8_t begin();
    uint8_t setPort(uint16_t port);
    uint8_t connected();
    int available();
    void stopConnection();
    
    


    void setCallback(void (*func)(void)) { _callbackFunction = func; };

private:
    uint16_t _port = 0;
    uint16_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
    void (*_callbackFunction)(void) = 0;
    uint8_t checkReadySourse();
};
