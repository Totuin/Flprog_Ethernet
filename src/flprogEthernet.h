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

	uint8_t begin(uint8_t *mac, unsigned long timeout = 20000, unsigned long responseTimeout = 4000);

	int maintain();

	uint8_t linkStatus() { return hardware()->getLinkStatus(); };
	uint8_t hardwareStatus();

	uint8_t begin(uint8_t *mac, IPAddress ip);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);

	void setRetransmissionTimeout(uint16_t milliseconds) { hardware()->setRetransmissionTime(milliseconds); };
	void setRetransmissionCount(uint8_t num) { hardware()->setRetransmissionCount(num); };

	virtual FLProgDNSClient *dnsClient() { return &_dns; };

	uint32_t checkLinePeriod() { return checkLineStatusPeriod; };
	void checkLinePeriod(uint32_t period) { checkLineStatusPeriod = period; };
	uint32_t reconnectPeriod() { return reconnectLinePeriod; };
	void reconnectPeriod(uint32_t period) { reconnectLinePeriod = period; };
	virtual FLProgAbstractTcpServer *getServer(int port) { return new FLProgEthernetServer(this, port); };
	virtual Client *getClient() { return new FLProgEthernetClient(this); };

protected:
	FLProgEthernetUDP _udp;
	FLProgDhcpClass _dhcp;
	FLProgDNSClient _dns;

private:
	uint8_t checkLineStatus();
	uint32_t checkLineStatusPeriod = 1000;
	uint32_t lastCheckLineStatusTime = flprog::timeBack(checkLineStatusPeriod);
	uint32_t reconnectLinePeriod = 5000;
	uint32_t lastReconnectTime = flprog::timeBack(reconnectLinePeriod);
};

class FLProgWiznetInterface : public FlprogEthernetClass
{
public:
	FLProgWiznetInterface(int pin = -1, uint8_t bus = 0 );
	virtual FLProgAbstractEthernetHardware *hardware() { return &_hardware; };
	void init(int pin = -1, uint8_t bus = 0);
	void setSsPin(int sspin) { _hardware.setSsPin(sspin); };
	virtual bool isImitation() { return false; }

protected:
	FLProgWiznetClass _hardware;
};
