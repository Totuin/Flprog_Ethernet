#pragma once
#include "flprogEthernet.h"
#include "RT_HW_BASE.h"
#include "RT_HW_CONSOLE.h"

class FLProgEthernetDiagnosticsReporter
{

public:
    FLProgEthernetDiagnosticsReporter(FLProgAbstractTcpInterface *interface);
    void pool();

private:
    void checkStatus();
    void report();
    void statusReport();
    void errorReport();
    void connectReport();
    void headReport();
    bool _isNeedSemdMainHeader = true;
    bool _hasUpdateStatus = false;
    bool _hasUpdateError = false;
    bool _hasUpdateConnect = false;

    uint8_t _status = 255;
    uint16_t _messageNumber = 1;
    uint8_t _errorCode = FLPROG_NOT_ERROR;
    uint8_t _connectStatus = 0;
    FLProgAbstractTcpInterface *_interface;
};
