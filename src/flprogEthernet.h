#pragma once
#include "flprogSPI.h"
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
	
	virtual FlprogDNSClient *dnsClient() { return &_dns; };

	uint32_t checkLinePeriod() { return checkLineStatusPeriod; };
	void checkLinePeriod(uint32_t period) { checkLineStatusPeriod = period; };
	uint32_t reconnectPeriod() { return reconnectLinePeriod; };
	void reconnectPeriod(uint32_t period) { reconnectLinePeriod = period; };

protected:
	FlprogEthernetUDP _udp;
	FlprogDhcpClass _dhcp;
	FlprogDNSClient _dns;

private:
	uint8_t checkLineStatus();
	uint32_t checkLineStatusPeriod = 1000;
	uint32_t lastCheckLineStatusTime = flprog::timeBack(checkLineStatusPeriod);
	uint32_t reconnectLinePeriod = 5000;
	uint32_t lastReconnectTime = flprog::timeBack(reconnectLinePeriod);
};

class FlprogW5100Interface : public FlprogEthernetClass
{
public:
	FlprogW5100Interface();
	FlprogW5100Interface(FLProgSPI *spi, int pin = 10);
	virtual FlprogAbstractEthernetHardware *hardware() { return &_hardware; };
	void init(FLProgSPI *spi, int sspin = 10);
	void setSsPin(int sspin) { _hardware.setSsPin(sspin); };

protected:
	FlprogW5100Class _hardware;
};
