#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "flprogW5100.h"
#include "flprogEthernetUdp.h"
#include "flprogDns.h"
#include "flprogDhcp.h"

#ifndef MAX_SOCK_NUM
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define MAX_SOCK_NUM 4
#else
#define MAX_SOCK_NUM 8
#endif
#endif

#define FLPROG_ETHERNET_LINK_UNKNOWN 0
#define FLPROG_ETHERNET_LINK_ON 1
#define FLPROG_ETHERNET_LINK_OFF 2

#define FLPROG_ETHERNET_NO_HARDWARE 0
#define FLPROG_ETHERNET_W5100 1
#define FLPROG_ETHERNET_W5200 2
#define FLPROG_ETHERNET_W5500 3

class FlprogEthernetClass
{
public:
	FlprogEthernetClass(SPIClass *spi, uint8_t pin = 10);
	uint8_t begin(uint8_t *mac, unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
	int maintain();
	uint8_t linkStatus();
	uint8_t hardwareStatus();

	// Manaul configuration
	uint8_t begin(uint8_t *mac, IPAddress ip);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
	uint8_t begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);
	void init(uint8_t sspin = 10);

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
	FlprogDNSClient *dnsClient() { return &_dns; };
	FlprogW5100Class *hardware() { return &_hardware; };

private:
	IPAddress _dnsServerAddress;
	FlprogW5100Class _hardware;
	FlprogEthernetUDP _udp;
	FlprogDhcpClass _dhcp;
	FlprogDNSClient _dns;
};

#include "flprogEthernetClient.h"
#include "flprogEthernetServer.h"
