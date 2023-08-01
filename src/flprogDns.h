
#pragma once
#include <Arduino.h>
#include "flprogEthernetUdp.h"

class FlprogEthernetUDP;

class FlprogDNSClient
{
public:
	void setUDP(FlprogEthernetUDP *udp);
	void begin(const IPAddress &aDNSServer);

	/** Convert a numeric IP address string into a four-byte IP address.
		@param aIPAddrString IP address to convert
		@param aResult IPAddress structure to store the returned IP address
		@result 1 if aIPAddrString was successfully converted to an IP address,
				else error code
	*/
	int inet_aton(const char *aIPAddrString, IPAddress &aResult);

	/** Resolve the given hostname to an IP address.
		@param aHostname Name to be resolved
		@param aResult IPAddress structure to store the returned IP address
		@result 1 if aIPAddrString was successfully converted to an IP address,
				else error code
	*/
	int getHostByName(const char *aHostname,  uint8_t *aResult, uint16_t timeout = 5000);

protected:
	uint16_t BuildRequest(const char *aName);
	uint16_t ProcessResponse(uint16_t aTimeout,  uint8_t *aAddress);
	FlprogEthernetUDP *_udp;

	IPAddress iDNSServer;
	uint16_t iRequestId;
};
