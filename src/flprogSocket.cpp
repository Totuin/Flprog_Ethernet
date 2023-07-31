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

#if ARDUINO >= 156 && !defined(ARDUINO_ARCH_PIC32)
extern void yield(void);
#else
#define yield()
#endif

// TODO: randomize this when not using DHCP, but how?
static uint16_t local_port = 49152; // 49152 to 65535

typedef struct
{
	uint16_t RX_RSR; // Number of bytes received
	uint16_t RX_RD;	 // Address to read
	uint16_t TX_FSR; // Free space ready for transmit
	uint8_t RX_inc;	 // how much have we advanced RX_RD
} socketstate_t;

static socketstate_t state[MAX_SOCK_NUM];

/*****************************************/
/*          Socket management            */
/*****************************************/

void FlprogEthernetClass::socketPortRand(uint16_t n)
{
	n &= 0x3FFF;
	local_port ^= n;
	// Serial.printf("socketPortRand %d, srcport=%d\n", n, local_port);
}

uint8_t FlprogEthernetClass::socketBegin(uint8_t protocol, uint16_t port)
{
	uint8_t s, status[MAX_SOCK_NUM], chip, maxindex = MAX_SOCK_NUM;

	// first check hardware compatibility
	chip = _hardware->getChip();
	if (!chip)
		return MAX_SOCK_NUM; // immediate error if no hardware detected
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	// Serial.printf("W5000socket begin, protocol=%d, port=%d\n", protocol, port);
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	// look at all the hardware sockets, use any that are closed (unused)
	for (s = 0; s < maxindex; s++)
	{
		status[s] = _hardware->readSn(s, FLPROG_SN_SR);
		if (status[s] == FLPROG_SN_SR_CLOSED)
			goto makesocket;
	}
	// Serial.printf("W5000socket step2\n");
	//  as a last resort, forcibly close any already closing
	for (s = 0; s < maxindex; s++)
	{
		uint8_t stat = status[s];
		if (stat == FLPROG_SN_SR_LAST_ACK)
			goto closemakesocket;
		if (stat == FLPROG_SN_SR_TIME_WAIT)
			goto closemakesocket;
		if (stat == FLPROG_SN_SR_FIN_WAIT)
			goto closemakesocket;
		if (stat == FLPROG_SN_SR_CLOSING)
			goto closemakesocket;
	}
	SPI.endTransaction();
	return MAX_SOCK_NUM; // all sockets are in use
closemakesocket:
	// Serial.printf("W5000socket close\n");
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
makesocket:
	// Serial.printf("W5000socket %d\n", s);
	FlprogEthernetServer::server_port[s] = 0;
	delayMicroseconds(250); // TODO: is this needed??
	_hardware->writeSn(s, FLPROG_SN_MR, protocol);
	_hardware->writeSn(s, FLPROG_SN_IR, 0xFF);
	if (port > 0)
	{
		_hardware->writeSn16(s, FLPROG_SN_PORT, port);
	}
	else
	{
		// if don't set the source port, set local_port number.
		if (++local_port < 49152)
			local_port = 49152;
		_hardware->writeSn16(s, FLPROG_SN_PORT, local_port);
	}
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_OPEN);
	state[s].RX_RSR = 0;
	state[s].RX_RD = _hardware->readSn16(s, FLPROG_SN_RX_RD); // always zero?
	state[s].RX_inc = 0;
	state[s].TX_FSR = 0;
	// Serial.printf("W5000socket prot=%d, RX_RD=%d\n",  _hardware->readSnMR(s), state[s].RX_RD);
	SPI.endTransaction();
	return s;
}

// multicast version to set fields before open  thd
uint8_t FlprogEthernetClass::socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port)
{
	uint8_t s, status[MAX_SOCK_NUM], chip, maxindex = MAX_SOCK_NUM;

	// first check hardware compatibility
	chip = _hardware->getChip();
	if (!chip)
		return MAX_SOCK_NUM; // immediate error if no hardware detected
#if MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	// Serial.printf("W5000socket begin, protocol=%d, port=%d\n", protocol, port);
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	// look at all the hardware sockets, use any that are closed (unused)
	for (s = 0; s < maxindex; s++)
	{
		status[s] = _hardware->readSn(s, FLPROG_SN_SR);
		if (status[s] == FLPROG_SN_SR_CLOSED)
			goto makesocket;
	}
	// Serial.printf("W5000socket step2\n");
	//  as a last resort, forcibly close any already closing
	for (s = 0; s < maxindex; s++)
	{
		uint8_t stat = status[s];
		if (stat == FLPROG_SN_SR_LAST_ACK)
			goto closemakesocket;
		if (stat == FLPROG_SN_SR_TIME_WAIT)
			goto closemakesocket;
		if (stat == FLPROG_SN_SR_FIN_WAIT)
			goto closemakesocket;
		if (stat == FLPROG_SN_SR_CLOSING)
			goto closemakesocket;
	}
	SPI.endTransaction();
	return MAX_SOCK_NUM; // all sockets are in use
closemakesocket:
	// Serial.printf("W5000socket close\n");
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
makesocket:
	// Serial.printf("W5000socket %d\n", s);
	FlprogEthernetServer::server_port[s] = 0;
	delayMicroseconds(250); // TODO: is this needed??
	_hardware->writeSn(s, FLPROG_SN_MR, protocol);
	_hardware->writeSn(s, FLPROG_SN_IR, 0xFF);
	if (port > 0)
	{
		_hardware->writeSn16(s, FLPROG_SN_PORT, port);
	}
	else
	{
		// if don't set the source port, set local_port number.
		if (++local_port < 49152)
			local_port = 49152;
		_hardware->writeSn16(s, FLPROG_SN_PORT, local_port);
	}
	// Calculate MAC address from Multicast IP Address
	byte mac[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
	mac[3] = ip[1] & 0x7F;
	mac[4] = ip[2];
	mac[5] = ip[3];
	uint8_t buf1[4];
	buf1[0] = ip[0];
	buf1[1] = ip[1];
	buf1[2] = ip[2];
	buf1[3] = ip[3];
	_hardware->writeSn(s, FLPROG_SN_DIPR, buf1, 4); // 239.255.0.1

	_hardware->writeSn16(s, FLPROG_SN_DPORT, port);
	_hardware->writeSn(s, FLPROG_SN_DHAR, mac, 6);
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_OPEN);
	state[s].RX_RSR = 0;
	state[s].RX_RD = _hardware->readSn16(s, FLPROG_SN_RX_RD); // always zero?
	state[s].RX_inc = 0;
	state[s].TX_FSR = 0;
	// Serial.printf("W5000socket prot=%d, RX_RD=%d\n",  _hardware->readSnMR(s), state[s].RX_RD);
	SPI.endTransaction();
	return s;
}
// Return the socket's status
//
uint8_t FlprogEthernetClass::socketStatus(uint8_t s)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	uint8_t status = _hardware->readSn(s, FLPROG_SN_SR);
	SPI.endTransaction();
	return status;
}

// Immediately close.  If a TCP connection is established, the
// remote host is left unaware we closed.
//
void FlprogEthernetClass::socketClose(uint8_t s)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
	SPI.endTransaction();
}

// Place the socket in listening (server) mode
//
uint8_t FlprogEthernetClass::socketListen(uint8_t s)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	if (_hardware->readSn(s, FLPROG_SN_SR) != FLPROG_SN_SR_INIT)
	{
		SPI.endTransaction();
		return 0;
	}
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_LISTEN);
	SPI.endTransaction();
	return 1;
}

// establish a TCP connection in Active (client) mode.
//
void FlprogEthernetClass::socketConnect(uint8_t s, uint8_t *addr, uint16_t port)
{
	// set destination IP
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->writeSn(s, FLPROG_SN_DIPR, addr, 4);
	_hardware->writeSn16(s, FLPROG_SN_DPORT, port);
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_CONNECT);
	SPI.endTransaction();
}

// Gracefully disconnect a TCP connection.
//
void FlprogEthernetClass::socketDisconnect(uint8_t s)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_DISCON);
	SPI.endTransaction();
}

/*****************************************/
/*    Socket Data Receive Functions      */
/*****************************************/

uint16_t FlprogEthernetClass::getSnRX_RSR(uint8_t s)
{
	uint16_t val, prev;
	prev = _hardware->readSn16(s, FLPROG_SN_RX_RSR);
	while (1)
	{
		val = _hardware->readSn16(s, FLPROG_SN_RX_RSR);
		if (val == prev)
		{
			return val;
		}
		prev = val;
	}
}

void FlprogEthernetClass::read_data(uint8_t s, uint16_t src, uint8_t *dst, uint16_t len)
{
	uint16_t size;
	uint16_t src_mask;
	uint16_t src_ptr;

	// Serial.printf("read_data, len=%d, at:%d\n", len, src);
	src_mask = (uint16_t)src & _hardware->SMASK;
	src_ptr = _hardware->RBASE(s) + src_mask;

	if (_hardware->hasOffsetAddressMapping() || src_mask + len <= _hardware->SSIZE)
	{
		_hardware->read(src_ptr, dst, len);
	}
	else
	{
		size = _hardware->SSIZE - src_mask;
		_hardware->read(src_ptr, dst, size);
		dst += size;
		_hardware->read(_hardware->RBASE(s), dst, len - size);
	}
}

// Receive data.  Returns size, or -1 for no data, or 0 if connection closed
//
int FlprogEthernetClass::socketRecv(uint8_t s, uint8_t *buf, int16_t len)
{
	// Check how much data is available
	int ret = state[s].RX_RSR;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	if (ret < len)
	{
		uint16_t rsr = getSnRX_RSR(s);
		ret = rsr - state[s].RX_inc;
		state[s].RX_RSR = ret;
		// Serial.printf("Sock_RECV, RX_RSR=%d, RX_inc=%d\n", ret, state[s].RX_inc);
	}
	if (ret == 0)
	{
		// No data available.
		uint8_t status = _hardware->readSn(s, FLPROG_SN_CR);
		if ((status == FLPROG_SN_SR_LISTEN) || (status == FLPROG_SN_SR_CLOSED) ||
			(status == FLPROG_SN_SR_CLOSE_WAIT))
		{
			// The remote end has closed its side of the connection,
			// so this is the eof state
			ret = 0;
		}
		else
		{
			// The connection is still up, but there's no data waiting to be read
			ret = -1;
		}
	}
	else
	{
		if (ret > len)
			ret = len; // more data available than buffer length
		uint16_t ptr = state[s].RX_RD;
		if (buf)
			read_data(s, ptr, buf, ret);
		ptr += ret;
		state[s].RX_RD = ptr;
		state[s].RX_RSR -= ret;
		uint16_t inc = state[s].RX_inc + ret;
		if (inc >= 250 || state[s].RX_RSR == 0)
		{
			state[s].RX_inc = 0;
			_hardware->writeSn16(s, FLPROG_SN_RX_RD, ptr);
			_hardware->execCmdSn(s, FLPROG_SOCK_CMD_RECV);
			// Serial.printf("Sock_RECV cmd, RX_RD=%d, RX_RSR=%d\n",
			//   state[s].RX_RD, state[s].RX_RSR);
		}
		else
		{
			state[s].RX_inc = inc;
		}
	}
	SPI.endTransaction();
	// Serial.printf("socketRecv, ret=%d\n", ret);
	return ret;
}

uint16_t FlprogEthernetClass::socketRecvAvailable(uint8_t s)
{
	uint16_t ret = state[s].RX_RSR;
	if (ret == 0)
	{
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		uint16_t rsr = getSnRX_RSR(s);
		SPI.endTransaction();
		ret = rsr - state[s].RX_inc;
		state[s].RX_RSR = ret;
		// Serial.printf("sockRecvAvailable s=%d, RX_RSR=%d\n", s, ret);
	}
	return ret;
}

// get the first byte in the receive queue (no checking)
//
uint8_t FlprogEthernetClass::socketPeek(uint8_t s)
{
	uint8_t b;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	uint16_t ptr = state[s].RX_RD;
	_hardware->read((ptr & _hardware->SMASK) + _hardware->RBASE(s), &b, 1);
	SPI.endTransaction();
	return b;
}

/*****************************************/
/*    Socket Data Transmit Functions     */
/*****************************************/

uint16_t FlprogEthernetClass::getSnTX_FSR(uint8_t s)
{
	uint16_t val, prev;

	prev = _hardware->readSn16(s, FLPROG_SN_TX_FSR);
	while (1)
	{
		val = _hardware->readSn16(s, FLPROG_SN_TX_FSR);
		if (val == prev)
		{
			state[s].TX_FSR = val;
			return val;
		}
		prev = val;
	}
}

void FlprogEthernetClass::write_data(uint8_t s, uint16_t data_offset, const uint8_t *data, uint16_t len)
{
	uint16_t ptr = _hardware->readSn16(s, FLPROG_SN_TX_WR);
	ptr += data_offset;
	uint16_t offset = ptr & _hardware->SMASK;
	uint16_t dstAddr = offset + _hardware->SBASE(s);

	if (_hardware->hasOffsetAddressMapping() || offset + len <= _hardware->SSIZE)
	{
		_hardware->write(dstAddr, data, len);
	}
	else
	{
		// Wrap around circular buffer
		uint16_t size = _hardware->SSIZE - offset;
		_hardware->write(dstAddr, data, size);
		_hardware->write(_hardware->SBASE(s), data + size, len - size);
	}
	ptr += len;
	_hardware->writeSn16(s, FLPROG_SN_TX_WR, ptr);
}

/**
 * @brief	This function used to send the data in TCP mode
 * @return	1 for success else 0.
 */
uint16_t FlprogEthernetClass::socketSend(uint8_t s, const uint8_t *buf, uint16_t len)
{
	uint8_t status = 0;
	uint16_t ret = 0;
	uint16_t freesize = 0;

	if (len > _hardware->SSIZE)
	{
		ret = _hardware->SSIZE; // check size not to exceed MAX size.
	}
	else
	{
		ret = len;
	}

	// if freebuf is available, start.
	do
	{
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		freesize = getSnTX_FSR(s);
		status = _hardware->readSn(s, FLPROG_SN_SR);
		SPI.endTransaction();
		if ((status != FLPROG_SN_SR_ESTABLISHED) && (status != FLPROG_SN_SR_CLOSE_WAIT))
		{
			ret = 0;
			break;
		}
		yield();
	} while (freesize < ret);

	// copy data
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	write_data(s, 0, (uint8_t *)buf, ret);
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_SEND);

	/* +2008.01 bj */
	while ((_hardware->readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_SEND_OK) != FLPROG_SN_IR_SEND_OK)
	{
		/* m2008.01 [bj] : reduce code */
		if (_hardware->readSn(s, FLPROG_SN_SR) == FLPROG_SN_SR_CLOSED)
		{
			SPI.endTransaction();
			return 0;
		}
		SPI.endTransaction();
		yield();
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	}
	/* +2008.01 bj */
	_hardware->writeSn(s, FLPROG_SN_IR, FLPROG_SN_IR_SEND_OK);
	SPI.endTransaction();
	return ret;
}

uint16_t FlprogEthernetClass::socketSendAvailable(uint8_t s)
{
	uint8_t status = 0;
	uint16_t freesize = 0;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	freesize = getSnTX_FSR(s);
	status = _hardware->readSn(s, FLPROG_SN_SR);
	SPI.endTransaction();
	if ((status == FLPROG_SN_SR_ESTABLISHED) || (status == FLPROG_SN_SR_CLOSE_WAIT))
	{
		return freesize;
	}
	return 0;
}

uint16_t FlprogEthernetClass::socketBufferData(uint8_t s, uint16_t offset, const uint8_t *buf, uint16_t len)
{
	// Serial.printf("  bufferData, offset=%d, len=%d\n", offset, len);
	uint16_t ret = 0;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	uint16_t txfree = getSnTX_FSR(s);
	if (len > txfree)
	{
		ret = txfree; // check size not to exceed MAX size.
	}
	else
	{
		ret = len;
	}
	write_data(s, offset, buf, ret);
	SPI.endTransaction();
	return ret;
}

bool FlprogEthernetClass::socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port)
{
	if (((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		((port == 0x00)))
	{
		return false;
	}
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->writeSn(s, FLPROG_SN_DIPR, addr, 4);
	_hardware->writeSn16(s, FLPROG_SN_DPORT, port);
	SPI.endTransaction();
	return true;
}

bool FlprogEthernetClass::socketSendUDP(uint8_t s)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	_hardware->execCmdSn(s, FLPROG_SOCK_CMD_SEND);

	/* +2008.01 bj */
	while ((_hardware->readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_SEND_OK) != FLPROG_SN_IR_SEND_OK)
	{
		if (_hardware->readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_TIMEOUT)
		{
			/* +2008.01 [bj]: clear interrupt */
			_hardware->writeSn(s, FLPROG_SN_IR, (FLPROG_SN_IR_SEND_OK | FLPROG_SN_IR_TIMEOUT));
			SPI.endTransaction();
			// Serial.printf("sendUDP timeout\n");
			return false;
		}
		SPI.endTransaction();
		yield();
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	}

	/* +2008.01 bj */
	_hardware->writeSn(s, FLPROG_SN_IR, FLPROG_SN_IR_SEND_OK);
	SPI.endTransaction();

	// Serial.printf("sendUDP ok\n");
	/* Sent ok */
	return true;
}
