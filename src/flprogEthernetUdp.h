#pragma once
#include <Arduino.h>
#include "flprogUtilites.h"
#include "flprogDns.h"



#define FLPROG_UDP_TX_PACKET_MAX_SIZE 24

#define FLPROG_UDP_TIMED_OUT 4

class FLProgEthernetUDP
{
public:
	FLProgEthernetUDP(){};
	FLProgEthernetUDP(FlprogAbstractEthernet *sourse);
	void setSourse(FlprogAbstractEthernet *sourse);
	virtual uint8_t begin(uint16_t);
	virtual uint8_t beginMulticast(IPAddress, uint16_t);
	virtual void stop();
	virtual int beginPacket(IPAddress ip, uint16_t port);
	virtual int beginPacket(const char *host, uint16_t port);
	virtual int endPacket();
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buffer, size_t size);
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
	uint8_t getStatus() { return _status; };
	uint8_t getError() { return _errorCode; }

	void setDnsCacheStorageTime(uint32_t time) { _dnsCacheStorageTime = time; }

protected:
	uint8_t _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	uint16_t _remaining;
	uint8_t _status = FLPROG_NOT_REDY_STATUS;
	uint8_t _errorCode = FLPROG_NOT_ERROR;

private:
	uint16_t _port;
	IPAddress _remoteIP;
	uint16_t _remotePort;
	uint16_t _offset;
	FlprogAbstractEthernet *_sourse;
	FLProgDNSClient _dns;

	String _dnsCachedHost = "";
	IPAddress _dnsCachedIP = FLPROG_INADDR_NONE;
	uint32_t _dnsStartCachTime;
	uint32_t _dnsCacheStorageTime = 60000;
};
