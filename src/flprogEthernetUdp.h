#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "flprogAbstactEthernetClasses.h"
#include "flprogDns.h"

#define FLPROG_UDP_TX_PACKET_MAX_SIZE 24

#define FLPROG_UDP_TIMED_OUT 4

class FLProgEthernetUDP : public FLProgAbstactEthernetUDPChanel
{
public:
	FLProgEthernetUDP(){};
	FLProgEthernetUDP(FLProgAbstractTcpInterface *sourse);
	virtual void setSourse(FLProgAbstractTcpInterface *sourse);
	virtual uint8_t beginMulticast(IPAddress, uint16_t);
	virtual virtual int beginPacket(const char *host, uint16_t port);
	void setDnsCacheStorageTime(uint32_t time) { _dnsCacheStorageTime = time; }

private:
	FLProgDNSClient _dns;
	String _dnsCachedHost = "";
	IPAddress _dnsCachedIP = FLPROG_INADDR_NONE;
	uint32_t _dnsStartCachTime;
	uint32_t _dnsCacheStorageTime = 60000;
};
