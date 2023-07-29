/* Copyright 2018 Paul Stoffregen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "flprogFlprogEthernet.h"
#include "FlprogDns.h"
#include "utility/flprogW5100.h"

int FlprogEthernetClient::connect(const char * host, uint16_t port)
{
	DNSClient dns; // Look up the host first
	IPAddress remote_addr;

	if (sockindex < MAX_SOCK_NUM) {
		if (FlprogEthernet.socketStatus(sockindex) != FlprogSnSR::CLOSED) {
			FlprogEthernet.socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = MAX_SOCK_NUM;
	}
	dns.begin(FlprogEthernet.dnsServerIP());
	if (!dns.getHostByName(host, remote_addr)) return 0; // TODO: use _timeout
	return connect(remote_addr, port);
}

int FlprogEthernetClient::connect(IPAddress ip, uint16_t port)
{
	if (sockindex < MAX_SOCK_NUM) {
		if (FlprogEthernet.socketStatus(sockindex) != FlprogSnSR::CLOSED) {
			FlprogEthernet.socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = MAX_SOCK_NUM;
	}
#if defined(ESP8266) || defined(ESP32)
	if (ip == IPAddress((uint32_t)0) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#else
	if (ip == IPAddress(0ul) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#endif
	sockindex = FlprogEthernet.socketBegin(FlprogSnMR::TCP, 0);
	if (sockindex >= MAX_SOCK_NUM) return 0;
	FlprogEthernet.socketConnect(sockindex, rawIPAddress(ip), port);
	uint32_t start = millis();
	while (1) {
		uint8_t stat = FlprogEthernet.socketStatus(sockindex);
		if (stat == FlprogSnSR::ESTABLISHED) return 1;
		if (stat == FlprogSnSR::CLOSE_WAIT) return 1;
		if (stat == FlprogSnSR::CLOSED) return 0;
		if (millis() - start > _timeout) break;
		delay(1);
	}
	FlprogEthernet.socketClose(sockindex);
	sockindex = MAX_SOCK_NUM;
	return 0;
}

int FlprogEthernetClient::availableForWrite(void)
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	return FlprogEthernet.socketSendAvailable(sockindex);
}

size_t FlprogEthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FlprogEthernetClient::write(const uint8_t *buf, size_t size)
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	if (FlprogEthernet.socketSend(sockindex, buf, size)) return size;
	setWriteError();
	return 0;
}

int FlprogEthernetClient::available()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	return FlprogEthernet.socketRecvAvailable(sockindex);
	// TODO: do the Wiznet chips automatically retransmit TCP ACK
	// packets if they are lost by the network?  Someday this should
	// be checked by a man-in-the-middle test which discards certain
	// packets.  If ACKs aren't resent, we would need to check for
	// returning 0 here and after a timeout do another Sock_RECV
	// command to cause the Wiznet chip to resend the ACK packet.
}

int FlprogEthernetClient::read(uint8_t *buf, size_t size)
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	return FlprogEthernet.socketRecv(sockindex, buf, size);
}

int FlprogEthernetClient::peek()
{
	if (sockindex >= MAX_SOCK_NUM) return -1;
	if (!available()) return -1;
	return FlprogEthernet.socketPeek(sockindex);
}

int FlprogEthernetClient::read()
{
	uint8_t b;
	if (FlprogEthernet.socketRecv(sockindex, &b, 1) > 0) return b;
	return -1;
}

void FlprogEthernetClient::flush()
{
	while (sockindex < MAX_SOCK_NUM) {
		uint8_t stat = FlprogEthernet.socketStatus(sockindex);
		if (stat != FlprogSnSR::ESTABLISHED && stat != FlprogSnSR::CLOSE_WAIT) return;
		if (FlprogEthernet.socketSendAvailable(sockindex) >= W5100.SSIZE) return;
	}
}

void FlprogEthernetClient::stop()
{
	if (sockindex >= MAX_SOCK_NUM) return;

	// attempt to close the connection gracefully (send a FIN to other side)
	FlprogEthernet.socketDisconnect(sockindex);
	unsigned long start = millis();

	// wait up to a second for the connection to close
	do {
		if (FlprogEthernet.socketStatus(sockindex) == FlprogSnSR::CLOSED) {
			sockindex = MAX_SOCK_NUM;
			return; // exit the loop
		}
		delay(1);
	} while (millis() - start < _timeout);

	// if it hasn't closed, close it forcefully
	FlprogEthernet.socketClose(sockindex);
	sockindex = MAX_SOCK_NUM;
}

uint8_t FlprogEthernetClient::connected()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;

	uint8_t s = FlprogEthernet.socketStatus(sockindex);
	return !(s == FlprogSnSR::LISTEN || s == FlprogSnSR::CLOSED || s == FlprogSnSR::FIN_WAIT ||
		(s == FlprogSnSR::CLOSE_WAIT && !available()));
}

uint8_t FlprogEthernetClient::status()
{
	if (sockindex >= MAX_SOCK_NUM) return FlprogSnSR::CLOSED;
	return FlprogEthernet.socketStatus(sockindex);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.
bool FlprogEthernetClient::operator==(const EthernetClient& rhs)
{
	if (sockindex != rhs.sockindex) return false;
	if (sockindex >= MAX_SOCK_NUM) return false;
	if (rhs.sockindex >= MAX_SOCK_NUM) return false;
	return true;
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/937bce1a0bb2567f6d03b15df79525569377dabd
uint16_t FlprogEthernetClient::localPort()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	uint16_t port;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = W5100.readSnPORT(sockindex);
	SPI.endTransaction();
	return port;
}

// https://github.com/per1234/EthernetMod
// returns the remote IP address: http://forum.arduino.cc/index.php?topic=82416.0
IPAddress FlprogEthernetClient::remoteIP()
{
	if (sockindex >= MAX_SOCK_NUM) return IPAddress((uint32_t)0);
	uint8_t remoteIParray[4];
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.readSnDIPR(sockindex, remoteIParray);
	SPI.endTransaction();
	return IPAddress(remoteIParray);
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/ca37de4ba4ecbdb941f14ac1fe7dd40f3008af75
uint16_t FlprogEthernetClient::remotePort()
{
	if (sockindex >= MAX_SOCK_NUM) return 0;
	uint16_t port;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = W5100.readSnDPORT(sockindex);
	SPI.endTransaction();
	return port;
}


