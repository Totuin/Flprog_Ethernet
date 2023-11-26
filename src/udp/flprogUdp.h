#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "../abstract/flprogAbstactEthernetUDPChanel.h"
#include "flprogDns.h"

#define FLPROG_UDP_TX_PACKET_MAX_SIZE 24

#define FLPROG_UDP_TIMED_OUT 4

class FLProgUdpClient : public FLProgAbstactEthernetUDPChanel
{
public:
	FLProgUdpClient(){};
	FLProgUdpClient(FLProgAbstractTcpInterface *sourse);
	virtual void setSourse(FLProgAbstractTcpInterface *sourse);
	virtual uint8_t beginMulticast(IPAddress, uint16_t);
	virtual int beginPacket(const char *host, uint16_t port);
	void setDnsCacheStorageTime(uint32_t time) { _dns.setDnsCacheStorageTime(time); };

private:
	FLProgDNSClient _dns;
};
