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

	virtual FLProgDNSClient *dnsClient() { return &_dns; };

	uint32_t checkLinePeriod() { return _checkEthernetStatusPeriod; };
	void checkLinePeriod(uint32_t period) { _checkEthernetStatusPeriod = period; };
	uint32_t reconnectPeriod() { return _reconnectEthernetPeriod; };
	void reconnectPeriod(uint32_t period) { _reconnectEthernetPeriod = period; };
	virtual FLProgAbstractTcpServer *getServer(int port) { return new FLProgEthernetServer(this, port); };
	virtual Client *getClient() { return new FLProgEthernetClient(this); };

protected:
	FLProgEthernetUDP _udp;
	FLProgDhcpClass _dhcp;
	FLProgDNSClient _dns;

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

	virtual uint8_t socetConnected(uint8_t socet) { return _hardware.socetConnected(socet); };
	virtual int readFromSocet(uint8_t socet) { return _hardware.readFromSocet(socet); };
	virtual size_t writeToSocet(const uint8_t *buffer, size_t size, uint8_t socet) { return _hardware.writeToSocet(buffer, size, socet); };
	virtual int availableSocet(uint8_t socet) { return _hardware.availableSocet(socet); };
	virtual void disconnecSocet(uint8_t socet) { _hardware.disconnecSocet(socet); };
	virtual uint8_t getTCPSocet(uint16_t port) { return _hardware.getTCPSocet(port); };
	virtual bool isListenSocet(uint8_t socet) { return _hardware.isListenSocet(socet); };

	virtual FLProgAbstractEthernetHardware *hardware() { return &_hardware; };
	void init(int pinCs, uint8_t bus);
	void setPinCs(int pinCs) { _hardware.setPinCs(pinCs); };
	virtual bool isImitation() { return false; }
	int pinCs() { return _hardware.pinCs(); }
	uint8_t spiBus() { return _hardware.spiBus(); }

protected:
	FLProgWiznetClass _hardware;
};
