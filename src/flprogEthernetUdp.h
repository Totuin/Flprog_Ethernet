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

class FlprogDNSClient;

class FlprogEthernetUDP : public UDP
{
public:
	FlprogEthernetUDP(){};
	FlprogEthernetUDP(FlprogAbstractEthernet *sourse);
	void setHatdware(FlprogAbstractEthernetHardware *hardware);
	void setDNS(FlprogDNSClient *dns) { _dns = dns; };
	virtual uint8_t begin(uint16_t);					 // initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
	virtual uint8_t beginMulticast(IPAddress, uint16_t); // initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
	virtual void stop();								 // Finish with the UDP socket
	virtual int beginPacket(IPAddress ip, uint16_t port);
	virtual int beginPacket(const char *host, uint16_t port);
	virtual int endPacket();
	// Write a single byte into the packet
	virtual size_t write(uint8_t);
	// Write size bytes from buffer into the packet
	virtual size_t write(const uint8_t *buffer, size_t size);

	using Print::write;

	// Start processing the next available incoming packet
	// Returns the size of the packet in bytes, or 0 if no packets are available
	virtual int parsePacket();
	// Number of bytes remaining in the current packet
	virtual int available();
	// Read a single byte from the current packet
	virtual int read();
	// Read up to len bytes from the current packet and place them into buffer
	// Returns the number of bytes read, or 0 if none are available
	virtual int read(uint8_t *buffer, size_t len);
	// Read up to len characters from the current packet and place them into buffer
	// Returns the number of characters read, or 0 if none are available
	virtual int read(char *buffer, size_t len) { return read((uint8_t *)buffer, len); };
	// Return the next byte from the current packet without moving on to the next byte
	virtual int peek();
	virtual void flush(); // Finish reading the current packet

	// Return the IP address of the host who sent the current incoming packet
	virtual IPAddress remoteIP() { return _remoteIP; };
	// Return the port of the host who sent the current incoming packet
	virtual uint16_t remotePort() { return _remotePort; };
	virtual uint16_t localPort() { return _port; }

protected:
	uint8_t sockindex;
	uint16_t _remaining; // remaining bytes of incoming packet yet to be processed

private:
	uint16_t _port;		  // local port to listen on
	IPAddress _remoteIP;  // remote IP address for the incoming packet whilst it's being processed
	uint16_t _remotePort; // remote port for the incoming packet whilst it's being processed
	uint16_t _offset;	  // offset into the packet being sent
	FlprogAbstractEthernetHardware *_hardware;
	FlprogDNSClient *_dns;
};
