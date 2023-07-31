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
#include "utility/flprogW5100.h"

uint16_t FlprogEthernetServer::server_port[MAX_SOCK_NUM];

FlprogEthernetServer::FlprogEthernetServer(FlprogEthernetClass *sourse, uint16_t port)
{
	ethernet = sourse;
	_port = port;
}

void FlprogEthernetServer::begin()
{
	uint8_t sockindex = ethernet->socketBegin(FLPROG_SN_MR_TCP, _port);
	if (sockindex < MAX_SOCK_NUM)
	{
		if (ethernet->socketListen(sockindex))
		{
			server_port[sockindex] = _port;
		}
		else
		{
			ethernet->socketDisconnect(sockindex);
		}
	}
}

FlprogEthernetClient FlprogEthernetServer::available()
{
	bool listening = false;
	uint8_t sockindex = MAX_SOCK_NUM;
	uint8_t chip, maxindex = MAX_SOCK_NUM;
	chip = ethernet->hardware()->getChip();
	if (!chip)
		return FlprogEthernetClient(ethernet, MAX_SOCK_NUM);
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			uint8_t stat = ethernet->socketStatus(i);
			if ((stat == FLPROG_SN_SR_ESTABLISHED) || (stat == FLPROG_SN_SR_CLOSE_WAIT))
			{
				if (ethernet->socketRecvAvailable(i) > 0)
				{
					sockindex = i;
				}
				else
				{
					// remote host closed connection, our end still open
					if (stat == FLPROG_SN_SR_CLOSE_WAIT)
					{
						ethernet->socketDisconnect(i);
						// status becomes LAST_ACK for short time
					}
				}
			}
			else if (stat == FLPROG_SN_SR_LISTEN)
			{
				listening = true;
			}
			else if (stat == FLPROG_SN_SR_CLOSED)
			{
				server_port[i] = 0;
			}
		}
	}
	if (!listening)
	{
		begin();
	}
	return FlprogEthernetClient(ethernet, sockindex);
}

FlprogEthernetClient FlprogEthernetServer::accept()
{
	bool listening = false;
	uint8_t sockindex = MAX_SOCK_NUM;
	uint8_t chip, maxindex = MAX_SOCK_NUM;

	chip = ethernet->hardware()->getChip();
	if (!chip)
		return FlprogEthernetClient(ethernet, MAX_SOCK_NUM);
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			uint8_t stat = ethernet->socketStatus(i);
			if (sockindex == MAX_SOCK_NUM &&
				((stat == FLPROG_SN_SR_ESTABLISHED) || (stat == FLPROG_SN_SR_CLOSE_WAIT)))
			{
				// Return the connected client even if no data received.
				// Some protocols like FTP expect the server to send the
				// first data.
				sockindex = i;
				server_port[i] = 0; // only return the client once
			}
			else if (stat == FLPROG_SN_SR_LISTEN)
			{
				listening = true;
			}
			else if (stat == FLPROG_SN_SR_CLOSED)
			{
				server_port[i] = 0;
			}
		}
	}
	if (!listening)
		begin();
	return FlprogEthernetClient(ethernet, sockindex);
}

FlprogEthernetServer::operator bool()
{
	uint8_t maxindex = MAX_SOCK_NUM;
#if MAX_SOCK_NUM > 4
	if (W5100.getChip() == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			if (ethernet->socketStatus(i) == FLPROG_SN_SR_LISTEN)
			{
				return true; // server is listening for incoming clients
			}
		}
	}
	return false;
}

#if 0
void FlprogEthernetServer::statusreport()
{
	Serial.printf("EthernetServer, port=%d\n", _port);
	for (uint8_t i=0; i < MAX_SOCK_NUM; i++) {
		uint16_t port = server_port[i];
		uint8_t stat = ethernet->socketStatus(i);
		const char *name;
		switch (stat) {
			case 0x00: name = "CLOSED"; break;
			case 0x13: name = "INIT"; break;
			case 0x14: name = "LISTEN"; break;
			case 0x15: name = "SYNSENT"; break;
			case 0x16: name = "SYNRECV"; break;
			case 0x17: name = "ESTABLISHED"; break;
			case 0x18: name = "FIN_WAIT"; break;
			case 0x1A: name = "CLOSING"; break;
			case 0x1B: name = "TIME_WAIT"; break;
			case 0x1C: name = "CLOSE_WAIT"; break;
			case 0x1D: name = "LAST_ACK"; break;
			case 0x22: name = "UDP"; break;
			case 0x32: name = "IPRAW"; break;
			case 0x42: name = "MACRAW"; break;
			case 0x5F: name = "PPPOE"; break;
			default: name = "???";
		}
		int avail = ethernet->socketRecvAvailable(i);
		Serial.printf("  %d: port=%d, status=%s (0x%02X), avail=%d\n",
			i, port, name, stat, avail);
	}
}
#endif

size_t FlprogEthernetServer::write(uint8_t b)
{
	return write(&b, 1);
}

size_t FlprogEthernetServer::write(const uint8_t *buffer, size_t size)
{
	uint8_t chip, maxindex = MAX_SOCK_NUM;

	chip = ethernet->hardware()->getChip();
	if (!chip)
		return 0;
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	available();
	for (uint8_t i = 0; i < maxindex; i++)
	{
		if (server_port[i] == _port)
		{
			if (ethernet->socketStatus(i) == FLPROG_SN_SR_ESTABLISHED)
			{
				ethernet->socketSend(i, buffer, size);
			}
		}
	}
	return size;
}
