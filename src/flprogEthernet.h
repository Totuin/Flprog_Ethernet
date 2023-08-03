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
	uint8_t begin(uint8_t *mac, unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
	int maintain();
	uint8_t linkStatus();
	uint8_t hardwareStatus();

	// Manaul configuration
	uint8_t begin(uint8_t *mac, IPAddress ip);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);

	void MACAddress(uint8_t *mac_address);
	IPAddress localIP();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsServerIP() { return _dnsServerAddress; }

	void setMACAddress(const uint8_t *mac_address);
	void setLocalIP(const IPAddress local_ip);
	void setSubnetMask(const IPAddress subnet);
	void setGatewayIP(const IPAddress gateway);
	void setDnsServerIP(const IPAddress dns_server) { _dnsServerAddress = dns_server; }
	void setRetransmissionTimeout(uint16_t milliseconds);
	void setRetransmissionCount(uint8_t num);
	virtual FlprogDNSClient *dnsClient() { return &_dns; };

protected:
	IPAddress _dnsServerAddress;
	FlprogEthernetUDP _udp;
	FlprogDhcpClass _dhcp;
	FlprogDNSClient _dns;
};

class FlprogW5100Interface : public FlprogEthernetClass
{
public:
	FlprogW5100Interface(){};
	FlprogW5100Interface(FLProgSPI *spi, uint8_t pin = 10);
	virtual FlprogAbstractEthernetHardware *hardware()
	{
		return &_hardware;
	};
	void init(FLProgSPI *spi, uint8_t sspin = 10);

protected:
	FlprogW5100Class _hardware;
};
