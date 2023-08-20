
#pragma once
#include <Arduino.h>
#include "flprogEthernetUdp.h"

class FLProgEthernetUDP;

class FLProgDNSClient
{
public:
	void setUDP(FLProgEthernetUDP *udp);
	void begin(const IPAddress aDNSServer);
	int inet_aton(const char *aIPAddrString, IPAddress &aResult);
	int getHostByName(const char *aHostname, uint8_t *aResult, uint16_t timeout = 5000);

protected:
	uint16_t BuildRequest(const char *aName);
	uint16_t ProcessResponse(uint16_t aTimeout, uint8_t *aAddress);
	FLProgEthernetUDP *_udp;

	IPAddress iDNSServer;
	uint16_t iRequestId;
};
