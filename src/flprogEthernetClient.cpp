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
#include "flprogEthernet.h"
#include "flprogDns.h"
#include "utility/flprogW5100.h"

FlprogEthernetClient::FlprogEthernetClient(FlprogEthernetClass *sourse)
{
	ethernet = sourse;
	sockindex = MAX_SOCK_NUM;
	_timeout = 1000;
}

FlprogEthernetClient::FlprogEthernetClient(FlprogEthernetClass *sourse, uint8_t s)
{
	ethernet = sourse;
	sockindex = s;
	_timeout = 1000;
}

int FlprogEthernetClient::connect(const char *host, uint16_t port)
{
	IPAddress remote_addr;
	if (sockindex < MAX_SOCK_NUM)
	{
		if (ethernet->socketStatus(sockindex) != FLPROG_SN_SR_CLOSED)
		{
			ethernet->socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = MAX_SOCK_NUM;
	}
	if (!ethernet->dnsClient()->getHostByName(host, remote_addr))
		return 0; // TODO: use _timeout
	return connect(remote_addr, port);
}

int FlprogEthernetClient::connect(IPAddress ip, uint16_t port)
{
	if (sockindex < MAX_SOCK_NUM)
	{
		if (ethernet->socketStatus(sockindex) != FLPROG_SN_SR_CLOSED)
		{
			ethernet->socketDisconnect(sockindex); // TODO: should we call stop()?
		}
		sockindex = MAX_SOCK_NUM;
	}
#if defined(ESP8266) || defined(ESP32)
	if (ip == IPAddress((uint32_t)0) || ip == IPAddress(0xFFFFFFFFul))
		return 0;
#else
	if (ip == IPAddress(0ul) || ip == IPAddress(0xFFFFFFFFul))
		return 0;
#endif
	sockindex = ethernet->socketBegin(FLPROG_SN_MR_TCP, 0);
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	ethernet->socketConnect(sockindex, rawIPAddress(ip), port);
	uint32_t start = millis();
	while (1)
	{
		uint8_t stat = ethernet->socketStatus(sockindex);
		if (stat == FLPROG_SN_SR_ESTABLISHED)
			return 1;
		if (stat == FLPROG_SN_SR_CLOSE_WAIT)
			return 1;
		if (stat == FLPROG_SN_SR_CLOSED)
			return 0;
		if (millis() - start > _timeout)
			break;
		delay(1);
	}
	ethernet->socketClose(sockindex);
	sockindex = MAX_SOCK_NUM;
	return 0;
}

int FlprogEthernetClient::availableForWrite(void)
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	return ethernet->socketSendAvailable(sockindex);
}

size_t FlprogEthernetClient::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FlprogEthernetClient::write(const uint8_t *buf, size_t size)
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	if (ethernet->socketSend(sockindex, buf, size))
		return size;
	setWriteError();
	return 0;
}

int FlprogEthernetClient::available()
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	return ethernet->socketRecvAvailable(sockindex);
	// TODO: do the Wiznet chips automatically retransmit TCP ACK
	// packets if they are lost by the network?  Someday this should
	// be checked by a man-in-the-middle test which discards certain
	// packets.  If ACKs aren't resent, we would need to check for
	// returning 0 here and after a timeout do another Sock_RECV
	// command to cause the Wiznet chip to resend the ACK packet.
}

int FlprogEthernetClient::read(uint8_t *buf, size_t size)
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	return ethernet->socketRecv(sockindex, buf, size);
}

int FlprogEthernetClient::peek()
{
	if (sockindex >= MAX_SOCK_NUM)
		return -1;
	if (!available())
		return -1;
	return ethernet->socketPeek(sockindex);
}

int FlprogEthernetClient::read()
{
	uint8_t b;
	if (ethernet->socketRecv(sockindex, &b, 1) > 0)
		return b;
	return -1;
}

void FlprogEthernetClient::flush()
{
	while (sockindex < MAX_SOCK_NUM)
	{
		uint8_t stat = ethernet->socketStatus(sockindex);
		if ((stat != FLPROG_SN_SR_ESTABLISHED) && (stat != FLPROG_SN_SR_CLOSE_WAIT))
			return;
		if (ethernet->socketSendAvailable(sockindex) >= ethernet->hardware()->SSIZE)
			return;
	}
}

void FlprogEthernetClient::stop()
{
	if (sockindex >= MAX_SOCK_NUM)
		return;

	// attempt to close the connection gracefully (send a FIN to other side)
	ethernet->socketDisconnect(sockindex);
	unsigned long start = millis();

	// wait up to a second for the connection to close
	do
	{
		if (ethernet->socketStatus(sockindex) == FLPROG_SN_SR_CLOSED)
		{
			sockindex = MAX_SOCK_NUM;
			return; // exit the loop
		}
		delay(1);
	} while (millis() - start < _timeout);

	// if it hasn't closed, close it forcefully
	ethernet->socketClose(sockindex);
	sockindex = MAX_SOCK_NUM;
}

uint8_t FlprogEthernetClient::connected()
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	uint8_t s = ethernet->socketStatus(sockindex);
	return !((s == FLPROG_SN_SR_LISTEN) || (s == FLPROG_SN_SR_CLOSED) || (s == FLPROG_SN_SR_FIN_WAIT) ||
			 ((s == FLPROG_SN_SR_CLOSE_WAIT) && !available()));
}

uint8_t FlprogEthernetClient::status()
{
	if (sockindex >= MAX_SOCK_NUM)
		return FLPROG_SN_SR_CLOSED;
	return ethernet->socketStatus(sockindex);
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.
bool FlprogEthernetClient::operator==(const FlprogEthernetClient &rhs)
{
	if (sockindex != rhs.sockindex)
		return false;
	if (sockindex >= MAX_SOCK_NUM)
		return false;
	if (rhs.sockindex >= MAX_SOCK_NUM)
		return false;
	return true;
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/937bce1a0bb2567f6d03b15df79525569377dabd
uint16_t FlprogEthernetClient::localPort()
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	uint16_t port;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = ethernet->hardware()->readSn16(sockindex, FLPROG_SN_PORT);
	SPI.endTransaction();
	return port;
}

// https://github.com/per1234/EthernetMod
// returns the remote IP address: http://forum.arduino.cc/index.php?topic=82416.0
IPAddress FlprogEthernetClient::remoteIP()
{
	if (sockindex >= MAX_SOCK_NUM)
		return IPAddress((uint32_t)0);
	uint8_t remoteIParray[4];
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	ethernet->hardware()->readSn(sockindex, FLPROG_SN_DIPR, remoteIParray, 4);
	SPI.endTransaction();
	return IPAddress(remoteIParray);
}

// https://github.com/per1234/EthernetMod
// from: https://github.com/ntruchsess/Arduino-1/commit/ca37de4ba4ecbdb941f14ac1fe7dd40f3008af75
uint16_t FlprogEthernetClient::remotePort()
{
	if (sockindex >= MAX_SOCK_NUM)
		return 0;
	uint16_t port;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	port = ethernet->hardware()->readSn16(sockindex, FLPROG_SN_DPORT);
	SPI.endTransaction();
	return port;
}
