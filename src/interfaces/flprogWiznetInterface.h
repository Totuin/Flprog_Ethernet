#pragma once
#include "flprogUtilites.h"
#include "../abstract/flprogAbstractTcpInterface.h"
#include "../hardware/flprogWizNet.h"
#include "../udp/flprogDhcp.h"

class FLProgWiznetInterface : public FLProgAbstractTcpInterface
{
public:
    FLProgWiznetInterface();
    FLProgWiznetInterface(int pinCs, uint8_t bus = 255);

    virtual uint8_t pool();

    virtual uint8_t soketConnected(uint8_t soket) { return _hardware.soketConnected(soket); };
    virtual int readFromSoket(uint8_t soket) { return _hardware.readFromSoket(soket); };
    virtual int readFromSoket(uint8_t soket, uint8_t *buf, int16_t len) { return _hardware.readFromSoket(soket, buf, len); };

    virtual size_t writeToSoket(uint8_t soket, const uint8_t *buffer, size_t size) { return _hardware.writeToSoket(soket, buffer, size); };
    virtual int availableSoket(uint8_t soket) { return _hardware.socketRecvAvailable(soket); };
    virtual void disconnecSoket(uint8_t soket) { _hardware.socketDisconnect(soket); };
    virtual uint8_t getTCPSoket(uint16_t port) { return _hardware.getTCPSoket(port); };
    virtual uint8_t getUDPSoket(uint16_t port) { return _hardware.getUDPSoket(port); };
    virtual bool isListenSoket(uint8_t soket) { return _hardware.socketListen(soket); };
    virtual void closeSoket(uint8_t soket) { _hardware.socketClose(soket); };
    virtual uint8_t sendUdpSoket(uint8_t soket) { return _hardware.socketSendUDP(soket); };

    virtual uint8_t startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port) { return _hardware.socketStartUDP(soket, addr, port); };
    virtual uint16_t bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len) { return _hardware.socketBufferData(soket, offset, buf, len); };
    virtual int recvSoket(uint8_t soket, uint8_t *buf, int16_t len) { return _hardware.socketRecv(soket, buf, len); };
    virtual uint8_t peekSoket(uint8_t soket) { return _hardware.socketPeek(soket); };
    virtual uint8_t beginMulticastSoket(IPAddress ip, uint16_t port) { return _hardware.beginMulticastSoket(ip, port); };
    virtual uint8_t connectSoket(uint8_t soket, IPAddress ip, uint16_t port) { return _hardware.socketConnect(soket, ip, port); };
    virtual uint8_t isConnectStatusSoket(uint8_t soket) { return _hardware.isConnectStatusSoket(soket); };
    virtual uint8_t isCosedStatusSoket(uint8_t soket) { return _hardware.isCosedStatusSoket(soket); };

    virtual uint8_t statusSoket(uint8_t soket) { return _hardware.socketStatus(soket); };
    virtual uint16_t localPortSoket(uint8_t soket) { return _hardware.localPort(soket); };
    virtual IPAddress remoteIPSoket(uint8_t soket) { return _hardware.remoteIP(soket); };
    virtual uint16_t remotePortSoket(uint8_t soket) { return _hardware.remotePort(soket); };

    void begin(IPAddress ip);
    void begin(IPAddress ip, IPAddress dns);
    void begin(IPAddress ip, IPAddress dns, IPAddress gateway);
    void begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);
    void begin();
    uint8_t maintain();
    uint8_t linkStatus() { return _hardware.getLinkStatus(); };
    uint8_t hardwareStatus();
    void setRetransmissionTimeout(uint16_t milliseconds) { _hardware.setRetransmissionTime(milliseconds); };
    void setRetransmissionCount(uint8_t num) { _hardware.setRetransmissionCount(num); };
    uint32_t checkLinePeriod() { return _checkEthernetStatusPeriod; };
    void checkLinePeriod(uint32_t period) { _checkEthernetStatusPeriod = period; };
    uint32_t reconnectPeriod() { return _reconnectEthernetPeriod; };
    void reconnectPeriod(uint32_t period) { _reconnectEthernetPeriod = period; };
    void init(int pinCs, uint8_t bus);
    void setPinCs(int pinCs) { _hardware.setPinCs(pinCs); };
    virtual bool isImitation() { return false; }
    int pinCs() { return _hardware.pinCs(); }
    uint8_t spiBus() { return _hardware.spiBus(); }
    virtual uint8_t type() { return FLPROG_ETHERNET_INTERFACE; }
    void updateEthernetStatus();
    virtual uint8_t maxSoketNum() { return _hardware.maxSoketNum(); };
    virtual bool isInit() { return _hardware.isInit(); };

private:
    FLProgWiznetClass _hardware;
    FLProgDhcp _dhcp;

    uint32_t _maintainPeriod = 1800000;
    uint32_t _startMaintainTime;
    bool _isMaintainMode = false;

    uint32_t _timeout = 20000;
    uint32_t _responseTimeout = 6000;

    uint32_t _checkEthernetStatusPeriod = 1000;
    uint32_t _lastCheckEthernetStatusTime = flprog::timeBack(_checkEthernetStatusPeriod);
    uint32_t _reconnectEthernetPeriod = 5000;
    uint32_t _lastReconnectTime = flprog::timeBack(_reconnectEthernetPeriod);
};
