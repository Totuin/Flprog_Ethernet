#pragma once
#include "hardware/flprogAbstractEthernetHardware.h"
#include "flprogAbstractEthernet.h"
#include "flprogEthernetUdp.h"
#include "flprogDns.h"
#include "flprogDhcp.h"
#include "flprogEthernetClient.h"
#include "flprogEthernetServer.h"

class FlprogEthernetClass : public FlprogAbstractEthernet
{
public:
	virtual void pool();

	void begin(unsigned long timeout = 20000, unsigned long responseTimeout = 4000);

	int maintain();

	uint8_t linkStatus() { return hardware()->getLinkStatus(); };
	uint8_t hardwareStatus();

	void begin(IPAddress ip);
	void begin(IPAddress ip, IPAddress dns);
	void begin(IPAddress ip, IPAddress dns, IPAddress gateway);
	void begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);

	void setRetransmissionTimeout(uint16_t milliseconds) { hardware()->setRetransmissionTime(milliseconds); };
	void setRetransmissionCount(uint8_t num) { hardware()->setRetransmissionCount(num); };

	uint32_t checkLinePeriod() { return _checkEthernetStatusPeriod; };
	void checkLinePeriod(uint32_t period) { _checkEthernetStatusPeriod = period; };
	uint32_t reconnectPeriod() { return _reconnectEthernetPeriod; };
	void reconnectPeriod(uint32_t period) { _reconnectEthernetPeriod = period; };

protected:
	FLProgDhcpClass _dhcp;

private:
	void updateEthernetStatus();
	uint32_t _checkEthernetStatusPeriod = 1000;
	uint32_t _lastCheckEthernetStatusTime = flprog::timeBack(_checkEthernetStatusPeriod);
	uint32_t _reconnectEthernetPeriod = 5000;
	uint32_t _lastReconnectTime = flprog::timeBack(_reconnectEthernetPeriod);
};

class FLProgWiznetInterface : public FlprogEthernetClass
{
public:
	FLProgWiznetInterface(int pinCs = -1, uint8_t bus = 255);

	virtual uint8_t SoketConnected(uint8_t soket) { return _hardware.SoketConnected(soket); };
	virtual int readFromSoket(uint8_t soket) { return _hardware.readFromSoket(soket); };
	virtual int readFromSoket(uint8_t soket, uint8_t *buf, int16_t len) { return _hardware.readFromSoket(uint8_t soket, buf, len); };

	virtual size_t writeToSoket(const uint8_t *buffer, size_t size, uint8_t soket) { return _hardware.writeToSoket(buffer, size, soket); };
	virtual int availableSoket(uint8_t soket) { return _hardware.socketRecvAvailable(soket); };
	virtual void disconnecSoket(uint8_t soket) { _hardware.socketDisconnect(soket); };
	virtual uint8_t getTCPSoket(uint16_t port) { return _hardware.socketBegin(FLPROG_SN_MR_TCP, port); };
	virtual uint8_t getUDPSoket(uint16_t port) { return _hardware.socketBegin(FLPROG_SN_MR_UDP, port); };
	virtual bool isListenSoket(uint8_t soket) { return _hardware.socketListen(soket); };
	virtual void closeSoket(uint8_t soket) { _hardware.socketClose(soket); };
	virtual uint8_t sendUdpSoket(uint8_t soket) { return _hardware.socketSendUDP(soket); };

	virtual uint8_t startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port) { return _hardware.socketStartUDP(soket, addr, port); };
	virtual uint16_t bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len) { return _hardware.socketBufferData(soket, offset, buf, len); };
	virtual int recvSoket(uint8_t soket, uint8_t *buf, int16_t len) { return _hardware.socketRecv(soket, buf, len); };
	virtual uint8_t peekSoket(uint8_t soket) { return _hardware.socketPeek(soket); };
	virtual uint8_t beginMulticastSoket(IPAddress ip, uint16_t port) { return _hardware.socketBeginMulticast((FLPROG_SN_MR_UDP | FLPROG_SN_MR_MULTI), ip, port); };
	virtual uint8_t connectSoket(uint8_t soket, IPAddress ip, uint16_t port) { return _hardware.socketConnect(uint8_t soket, IPAddress ip, uint16_t port); };
	virtual uint8_t isConnectStatusSoket(uint8_t soket) { return _hardware.isConnectStatusSoket(soket); };
	virtual uint8_t isCosedStatusSoket(uint8_t soket) { return _hardware.isCosedStatusSoket(soket); };
	virtual uint8_t peekSoket(uint8_t soket) { return _hardware.socketPeek(soket); };
	virtual uint8_t statusSoket(uint8_t soket) { return _hardware.socketStatus(soket); };
	virtual uint16_t localPortSoket(uint8_t soket) { return _hardware.localPort(soket); };
	virtual IPAddress remoteIPSoket(uint8_t soket) { return _hardware.remoteIP(soket); };
	virtual uint16_t remotePortSoket(uint8_t soket) { return _hardware.remotePort(soket); };

	virtual FLProgAbstractEthernetHardware *hardware() { return &_hardware; };
	void init(int pinCs, uint8_t bus);
	void setPinCs(int pinCs) { _hardware.setPinCs(pinCs); };
	virtual bool isImitation() { return false; }
	int pinCs() { return _hardware.pinCs(); }
	uint8_t spiBus() { return _hardware.spiBus(); }

protected:
	FLProgWiznetClass _hardware;
};
