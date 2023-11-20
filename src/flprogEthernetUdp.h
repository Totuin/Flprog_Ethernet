#pragma once
#include <Arduino.h>
#include "Udp.h"
#include "flprogDns.h"
#include "hardware/flprogAbstractEthernetHardware.h"
#include "flprogAbstractEthernet.h"

#ifndef FLPROG_ETHERNET_MAX_SOCK_NUM
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define FLPROG_ETHERNET_MAX_SOCK_NUM 4
#else
#define FLPROG_ETHERNET_MAX_SOCK_NUM 8
#endif
#endif

#define FLPROG_UDP_TX_PACKET_MAX_SIZE 24

class FLProgDNSClient;

class FLProgEthernetUDP : public UDP
{
public:
	FLProgEthernetUDP(){};
	FLProgEthernetUDP(FlprogAbstractEthernet *sourse);
	void setSourse(FlprogAbstractEthernet *sourse);
	void setDNS(FLProgDNSClient *dns) { _dns = dns; };
	virtual uint8_t begin(uint16_t);
	virtual uint8_t beginMulticast(IPAddress, uint16_t);
	virtual void stop();
	virtual int beginPacket(IPAddress ip, uint16_t port);
	virtual int beginPacket(const char *host, uint16_t port);
	virtual int endPacket();
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buffer, size_t size);

	using Print::write;
	virtual int parsePacket();
	virtual int available();
	virtual int read();
	virtual int read(uint8_t *buffer, size_t len);
	virtual int read(char *buffer, size_t len) { return read((uint8_t *)buffer, len); };
	virtual int peek();
	virtual void flush();
	virtual IPAddress remoteIP() { return _remoteIP; };
	virtual uint16_t remotePort() { return _remotePort; };
	virtual uint16_t localPort() { return _port; }

protected:
	uint8_t sockindex;
	uint16_t _remaining;

private:
	uint16_t _port;
	IPAddress _remoteIP;
	uint16_t _remotePort;
	uint16_t _offset;
	FlprogAbstractEthernet *_sourse;
	FLProgDNSClient *_dns;
};
