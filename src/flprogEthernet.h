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
	uint8_t linkStatus() { return hardware()->getLinkStatus(); };
	uint8_t hardwareStatus();

	// Manaul configuration
	uint8_t begin(uint8_t *mac, IPAddress ip);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);

	void MACAddress(uint8_t *mac_address) { hardware()->MACAddress(mac_address); };
	IPAddress localIP() { return hardware()->localIP(); };
	IPAddress subnetMask() { return hardware()->subnetMask(); };
	IPAddress gatewayIP() { return hardware()->gatewayIP(); };
	IPAddress dnsServerIP() { return _dnsServerAddress; }

	void setMACAddress(const uint8_t *mac_address) { hardware()->setOnlyMACAddress(mac_address); };
	void setLocalIP(const IPAddress local_ip) { hardware()->setOnlyLocalIP(local_ip); };
	void setSubnetMask(const IPAddress subnet) { hardware()->setOnlySubnetMask(subnet); };
	void setGatewayIP(const IPAddress gateway) { hardware()->setOnlyGatewayIP(gateway); };
	void setDnsServerIP(const IPAddress dns_server) { _dnsServerAddress = dns_server; }
	void setRetransmissionTimeout(uint16_t milliseconds) { hardware()->setRetransmissionTime(milliseconds); };
	void setRetransmissionCount(uint8_t num) { hardware()->setRetransmissionCount(num); };
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
	FlprogW5100Interface();
	FlprogW5100Interface(FLProgSPI *spi, int pin = 10);
	virtual FlprogAbstractEthernetHardware *hardware() { return &_hardware; };
	void init(FLProgSPI *spi, int sspin = 10);
	void setSsPin(int sspin) { _hardware.setSsPin(sspin); };
	virtual bool isReady();
	virtual bool isBusy();

protected:
	FlprogW5100Class _hardware;
};
