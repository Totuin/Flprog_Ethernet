#include "flprogEthernetDiagnosticsReporter.h"

FLProgEthernetDiagnosticsReporter::FLProgEthernetDiagnosticsReporter(FLProgAbstractTcpInterface *interface)
{
    _interface = interface;
}

void FLProgEthernetDiagnosticsReporter::pool()
{
    checkStatus();
    report();
}

void FLProgEthernetDiagnosticsReporter::checkStatus()
{
    uint8_t temp = _interface->getStatus();
    if (temp != _status)
    {
        _status = temp;
        _hasUpdateStatus = true;
    }

    temp = _interface->getError();
    if (temp != _errorCode)
    {
        _errorCode = temp;
        _hasUpdateError = true;
    }

    if (_interface->isReady())
    {
        if (_connectStatus != 1)
        {
            _connectStatus = _interface->isReady();
            _hasUpdateConnect = true;
        }
        else
        {
            if (_connectStatus != 0)
            {
                _connectStatus = _interface->isReady();
                _hasUpdateConnect = true;
            }
        }
    }
}

void FLProgEthernetDiagnosticsReporter::report()
{
    if (!RT_HW_console.getRun())
    {
        return;
    }
    if (_isNeedSemdMainHeader)
    {
        _isNeedSemdMainHeader = false;
        RT_HW_console.outHead(String(F("ETHERNET")), '=');
        RT_HW_console.outCR();
    }

    if (_hasUpdateStatus)
    {
        statusReport();
        _hasUpdateStatus = false;
    }
    if (_hasUpdateError)
    {
        errorReport();
        _hasUpdateError = false;
    }
    if (_hasUpdateConnect)
    {
        connectReport();
        _hasUpdateConnect = false;
    }
}

void FLProgEthernetDiagnosticsReporter::statusReport()
{
    headReport();
    RT_HW_console.outVar(String(F(" status->")));
    RT_HW_console.outVar(_status, ':', 0, 'U');
    RT_HW_console.outBlank(2);
    if (_status == FLPROG_NOT_REDY_STATUS)
    {
        RT_HW_console.outVar(String(F("No ready chip")));
    }
    if (_status == FLPROG_READY_STATUS)
    {
        RT_HW_console.outVar(String(F("Chip ready")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_HARDWARE_INIT_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait init chip")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_DHCP_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait DHCP")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_DNS_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait DNS")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_UDP_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait UDP")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_CLIENT_CONNECT_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait link with client Ethernet")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_LINK_ON_STATUS)
    {
        RT_HW_console.outVar(String(F("Link ok")));
    }
    if (_status == FLPROG_ETHERNET_HARDWARE_INIT_STATUS)
    {
        RT_HW_console.outVar(String(F("Chip Ethernet init ok")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_CONNECT_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait link Ethernet")));
    }
    if (_status == FLPROG_WAIT_WIFI_CLIENT_CONNECT_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait link with client Wi-Fi")));
    }
    if (_status == FLPROG_WAIT_ETHERNET_START_CLIENT_CONNECT_STATUS)
    {
        RT_HW_console.outVar(String(F("Wait link with client Ethernet")));
    }
    if (_status == FLPROG_WAIT_WEB_SERVER_READ_REQEST)
    {
        RT_HW_console.outVar(String(F("Wait answer from Web-server")));
    }
    if (_status == FLPROG_WAIT_WEB_SERVER_SEND_ANSVER)
    {
        RT_HW_console.outVar(String(F("Wait send  to Web-server")));
    }
    if (_status == FLPROG_WAIT_SEND_UDP_PACAGE)
    {
        RT_HW_console.outVar(String(F("Wait send packet to UDP-server")));
    }
    RT_HW_console.outChar('.');
    RT_HW_console.outCR();
}

void FLProgEthernetDiagnosticsReporter::errorReport()
{
    headReport();
    RT_HW_console.outVar(String(F(" error->")));
    RT_HW_console.outVar(_errorCode, ':', 0, 'U');
    RT_HW_console.outBlank(2);
    if (_errorCode == FLPROG_NOT_ERROR)
    {
        RT_HW_console.outVar(String(F("Error reset, no error")));
    }
    if (_errorCode == FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR)
    {
        RT_HW_console.outVar(String(F("Error ready Ethernet")));
    }
    if (_errorCode == FLPROG_ETHERNET_HARDWARE_INIT_ERROR)
    {
        RT_HW_console.outVar(String(F("Error init  Ethernet")));
    }
    RT_HW_console.outCR();
}

void FLProgEthernetDiagnosticsReporter::connectReport()
{
    headReport();
    if (_connectStatus == 1)
    {
        RT_HW_console.outVar(String(F(" connect=1:  ETHERNET LINK! ")), '~');
        RT_HW_console.outCR();
        RT_HW_console.outTest(String(F("MAC")), RT_HW_Base.ethMacAdrToString(_interface->mac()), ';');
        RT_HW_console.outBlank(1);
        RT_HW_console.outTest(String(F("IP")), RT_HW_Base.ethIpToString(_interface->localIP()), 'E');
        RT_HW_console.outTest(String(F("DNS")), RT_HW_Base.ethIpToString(_interface->dns()), ';');
        RT_HW_console.outBlank(2);
        RT_HW_console.outTest(String(F("Subnet")), RT_HW_Base.ethIpToString(_interface->subnet()), ';');
        RT_HW_console.outBlank(2);
        RT_HW_console.outTest(String(F("Gateway")), RT_HW_Base.ethIpToString(_interface->gateway()), 'E');
    }
    if (_connectStatus == 0)
    {
        RT_HW_console.outVar(String(F(" connect=0:  ETHERNET OFF!!! ")));
        RT_HW_console.outCR(2);
    }
}

void FLProgEthernetDiagnosticsReporter::headReport()
{
    RT_HW_console.outVar(String(F("Eth")));
    RT_HW_console.outVar((_interface->busNumber()), '=');
    RT_HW_console.outVar(_messageNumber, '~', 0, 'U');
    RT_HW_console.outTest(String(F(" cs")), ((uint8_t)_interface->pinCs()), ';', 'P');
    RT_HW_console.outTest(String(F(" bus")), ((uint8_t)_interface->busNumber()), ';', 'U');
    _messageNumber++;
}