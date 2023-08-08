#include "flprogW5100.h"

void FlprogW5100Class::setNetSettings(uint8_t *mac, IPAddress ip)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	setMACAddress(mac);
	setIPAddress(ip);
	_spi->endTransaction();
}

void FlprogW5100Class::setNetSettings(IPAddress ip, IPAddress gateway, IPAddress subnet)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	setIPAddress(ip);
	setGatewayIp(gateway);
	setSubnetMask(subnet);
	_spi->endTransaction();
}

void FlprogW5100Class::setNetSettings(uint8_t *mac, IPAddress ip, IPAddress gateway, IPAddress subnet)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	setMACAddress(mac);
	setIPAddress(ip);
	setGatewayIp(gateway);
	setSubnetMask(subnet);
	_spi->endTransaction();
}

IPAddress FlprogW5100Class::localIP()
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = getIPAddress();
	_spi->endTransaction();
	return result;
}

IPAddress FlprogW5100Class::subnetMask()
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = getSubnetMask();
	_spi->endTransaction();
	return result;
}

IPAddress FlprogW5100Class::gatewayIP()
{
	IPAddress ret;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress result = getGatewayIp();
	_spi->endTransaction();
	return result;
}

void FlprogW5100Class::setOnlyMACAddress(const uint8_t *mac_address)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	setMACAddress(mac_address);
	_spi->endTransaction();
}

void FlprogW5100Class::setOnlyLocalIP(const IPAddress local_ip)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = local_ip;
	setIPAddress(ip);
	_spi->endTransaction();
}

void FlprogW5100Class::setOnlySubnetMask(const IPAddress subnet)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = subnet;
	setSubnetMask(ip);
	_spi->endTransaction();
}

void FlprogW5100Class::setOnlyGatewayIP(const IPAddress gateway)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = gateway;
	setGatewayIp(ip);
	_spi->endTransaction();
}

void FlprogW5100Class::MACAddress(uint8_t *mac_address)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	getMACAddress(mac_address);
	_spi->endTransaction();
}

uint16_t FlprogW5100Class::localPort(uint8_t soc)
{
	if (soc >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	uint16_t port;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	port = readSn16(soc, FLPROG_SN_PORT);
	_spi->endTransaction();
	return port;
}

IPAddress FlprogW5100Class::remoteIP(uint8_t soc)
{
	if (soc >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return IPAddress((uint32_t)0);
	uint8_t remoteIParray[4];
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	readSn(soc, FLPROG_SN_DIPR, remoteIParray, 4);
	_spi->endTransaction();
	return IPAddress(remoteIParray);
}

uint16_t FlprogW5100Class::remotePort(uint8_t soc)
{
	if (soc >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	uint16_t port;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	port = readSn16(soc, FLPROG_SN_DPORT);
	_spi->endTransaction();
	return port;
}

uint16_t FlprogW5100Class::SBASE(uint8_t socknum)
{
	if (chip == 51)
	{
		return socknum * SSIZE + 0x4000;
	}
	else
	{
		return socknum * SSIZE + 0x8000;
	}
}

uint16_t FlprogW5100Class::RBASE(uint8_t socknum)
{
	if (chip == 51)
	{
		return socknum * SSIZE + 0x6000;
	}
	else
	{
		return socknum * SSIZE + 0xC000;
	}
}

bool FlprogW5100Class::hasOffsetAddressMapping(void)
{
	if (chip == 55)
		return true;
	return false;
}

void FlprogW5100Class::setIPAddress(IPAddress addr)
{
	uint8_t buffer[4];
	buffer[0] = addr[0];
	buffer[1] = addr[1];
	buffer[2] = addr[2];
	buffer[3] = addr[3];
	write(FLPROG_SIPR, buffer, 4);
}

IPAddress FlprogW5100Class::getIPAddress()
{
	uint8_t buffer[4] = {0, 0, 0, 0};
	read(FLPROG_SIPR, buffer, 4);
	return IPAddress(buffer[0], buffer[1], buffer[2], buffer[3]);
}

void FlprogW5100Class::setGatewayIp(IPAddress addr)
{
	uint8_t buffer[4];
	buffer[0] = addr[0];
	buffer[1] = addr[1];
	buffer[2] = addr[2];
	buffer[3] = addr[3];
	write(FLPROG_GAR, buffer, 4);
}

IPAddress FlprogW5100Class::getGatewayIp()
{
	uint8_t buffer[4] = {0, 0, 0, 0};
	read(FLPROG_GAR, buffer, 4);
	return IPAddress(buffer[0], buffer[1], buffer[2], buffer[3]);
}

void FlprogW5100Class::setSubnetMask(IPAddress addr)
{
	uint8_t buffer[4];
	buffer[0] = addr[0];
	buffer[1] = addr[1];
	buffer[2] = addr[2];
	buffer[3] = addr[3];
	write(FLPROG_SUBR, buffer, 4);
}

IPAddress FlprogW5100Class::getSubnetMask()
{
	uint8_t buffer[4] = {0, 0, 0, 0};
	read(FLPROG_SUBR, buffer, 4);
	return IPAddress(buffer[0], buffer[1], buffer[2], buffer[3]);
}

void FlprogW5100Class::setRetransmissionTime(uint16_t timeout)
{
	if (timeout > 6553)
		timeout = 6553;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	write16(FLPROG_RTR, (timeout * 10));
	_spi->endTransaction();
}

void FlprogW5100Class::setRetransmissionCount(uint8_t retry)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	write(FLPROG_RCR, retry);
	_spi->endTransaction();
}

uint8_t FlprogW5100Class::read(uint16_t addr)
{
	uint8_t data;
	read(addr, &data, 1);
	return data;
}

uint16_t FlprogW5100Class::readSn16(SOCKET _s, uint16_t address)
{
	uint8_t buf[2];
	readSn(_s, address, buf, 2);
	return (buf[0] << 8) | buf[1];
}

void FlprogW5100Class::writeSn16(SOCKET _s, uint16_t address, uint16_t _data)
{
	uint8_t buf[2];
	buf[0] = _data >> 8;
	buf[1] = _data & 0xFF;
	writeSn(_s, address, buf, 2);
}

void FlprogW5100Class::write16(uint16_t address, uint16_t _data)
{
	uint8_t buf[2];
	buf[0] = _data >> 8;
	buf[1] = _data & 0xFF;
	write(address, buf, 2);
}

uint8_t FlprogW5100Class::init(void)
{
	uint8_t i;
	if (initialized)
		return 1;
	delay(560);
	_spi->begin();
	initSS();
	resetSS();
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	if (isW5200())
	{
		CH_BASE_MSB = 0x40;
#ifdef ETHERNET_LARGE_BUFFERS
#if FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
		SSIZE = 16384;
#elif FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
		SSIZE = 8192;
#elif FLPROG_ETHERNET_MAX_SOCK_NUM <= 4
		SSIZE = 4096;
#else
		SSIZE = 2048;
#endif
		SMASK = SSIZE - 1;
#endif
		for (i = 0; i < FLPROG_ETHERNET_MAX_SOCK_NUM; i++)
		{
			writeSn(i, FLPROG_SN_RX_SIZE, SSIZE >> 10);
			writeSn(i, FLPROG_SN_TX_SIZE, SSIZE >> 10);
		}
		for (; i < 8; i++)
		{
			writeSn(i, FLPROG_SN_RX_SIZE, 0);
			writeSn(i, FLPROG_SN_TX_SIZE, 0);
		}
	}
	else
	{
		if (isW5500())
		{
			CH_BASE_MSB = 0x10;
#ifdef ETHERNET_LARGE_BUFFERS
#if FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
			SSIZE = 16384;
#elif FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
			SSIZE = 8192;
#elif FLPROG_ETHERNET_MAX_SOCK_NUM <= 4
			SSIZE = 4096;
#else
			SSIZE = 2048;
#endif
			SMASK = SSIZE - 1;
			for (i = 0; i < FLPROG_ETHERNET_MAX_SOCK_NUM; i++)
			{
				writeSn(i, FLPROG_SN_RX_SIZE, SSIZE >> 10);
				writeSn(i, FLPROG_SN_TX_SIZE, SSIZE >> 10);
			}
			for (; i < 8; i++)
			{
				writeSn(i, FLPROG_SN_RX_SIZE, 0);
				writeSn(i, FLPROG_SN_TX_SIZE, 0);
			}
#endif
			// Try W5100 last.  This simple chip uses fixed 4 byte frames
			// for every 8 bit access.  Terribly inefficient, but so simple
			// it recovers from "hearing" unsuccessful W5100 or W5200
			// communication.  W5100 is also the only chip without a VERSIONR
			// register for identification, so we check this last.
		}
		else
		{
			if (isW5100())
			{
				CH_BASE_MSB = 0x04;
#ifdef ETHERNET_LARGE_BUFFERS
#if FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
				SSIZE = 8192;
				write(FLPROG_TMSR, 0x03);
				write(FLPROG_RMSR, 0x03);
#elif FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
				SSIZE = 4096;
				write(FLPROG_TMSR, 0x0A);
				write(FLPROG_RMSR, 0x0A);
#else
				SSIZE = 2048;
				write(FLPROG_TMSR, 0x55);
				write(FLPROG_RMSR, 0x55);
#endif
				SMASK = SSIZE - 1;
#else
				write(FLPROG_TMSR, 0x55);
				write(FLPROG_RMSR, 0x55);
#endif
				// No hardware seems to be present.  Or it could be a W5200
				// that's heard other SPI communication if its chip select
				// pin wasn't high when a SD card or other SPI chip was used.
			}
			else
			{
				// Serial.println("no chip :-(");
				chip = 0;
				_spi->endTransaction();
				return 0; // no known chip is responding :-(
			}
		}
	}
	_spi->endTransaction();
	initialized = true;
	return 1; // successful init
}

// Soft reset the Wiznet chip, by writing to its MR register reset bit
uint8_t FlprogW5100Class::softReset(void)
{
	uint16_t count = 0;

	// Serial.println("Wiznet soft reset");
	//  write to reset bit
	write(FLPROG_MR, 0x80);
	// then wait for soft reset to complete
	do
	{
		uint8_t mr = read(FLPROG_MR);
		// Serial.print("mr=");
		// Serial.println(mr, HEX);
		if (mr == 0)
			return 1;
		delay(1);
	} while (++count < 20);
	return 0;
}

uint8_t FlprogW5100Class::isW5100(void)
{
	chip = 51;
	// Serial.println("w5100.cpp: detect W5100 chip");
	if (!softReset())
		return 0;
	write(FLPROG_MR, 0x10);
	if (read(FLPROG_MR) != 0x10)
		return 0;
	write(FLPROG_MR, 0x12);
	if (read(FLPROG_MR) != 0x12)
		return 0;
	write(FLPROG_MR, 0x00);
	if (read(FLPROG_MR) != 0x00)
		return 0;
	// Serial.println("chip is W5100");
	return 1;
}

uint8_t FlprogW5100Class::isW5200(void)
{
	chip = 52;
	// Serial.println("w5100.cpp: detect W5200 chip");
	if (!softReset())
		return 0;
	write(FLPROG_MR, 0x08);
	if (read(FLPROG_MR) != 0x08)
		return 0;
	write(FLPROG_MR, 0x10);
	if (read(FLPROG_MR) != 0x10)
		return 0;
	write(FLPROG_MR, 0x00);
	if (read(FLPROG_MR) != 0x00)
		return 0;
	int ver = read(FLPROG_VERSIONR_W5200);
	Serial.print("version=");
	Serial.println(ver);
	if (ver != 3)
		return 0;
	// Serial.println("chip is W5200");
	return 1;
}

uint8_t FlprogW5100Class::isW5500(void)
{
	chip = 55;
	// Serial.println("w5100.cpp: detect W5500 chip");
	if (!softReset())
		return 0;
	write(FLPROG_MR, 0x08);
	if (read(FLPROG_MR) != 0x08)
		return 0;
	write(FLPROG_MR, 0x10);
	if (read(FLPROG_MR) != 0x10)
		return 0;
	write(FLPROG_MR, 0x00);
	if (read(FLPROG_MR) != 0x00)
		return 0;
	int ver = read(FLPROG_VERSIONR_W5500);
	// Serial.print("version=");
	// Serial.println(ver);
	if (ver != 4)
		return 0;
	// Serial.println("chip is W5500");
	return 1;
}

uint8_t FlprogW5100Class::getLinkStatus()
{
	uint8_t phystatus;

	if (!init())
		return FLPROG_ETHERNET_LINK_UNKNOWN;
	switch (chip)
	{
	case 52:
		_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
		phystatus = read(FLPROG_PSTATUS_W5200);
		_spi->endTransaction();
		if (phystatus & 0x20)
			return FLPROG_ETHERNET_LINK_ON;
		return FLPROG_ETHERNET_LINK_OFF;
	case 55:
		_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
		phystatus = read(FLPROG_PHYCFGR_W5500);
		_spi->endTransaction();
		if (phystatus & 0x01)
			return FLPROG_ETHERNET_LINK_ON;
		return FLPROG_ETHERNET_LINK_OFF;
	default:
		return FLPROG_ETHERNET_LINK_UNKNOWN;
	}
}

uint16_t FlprogW5100Class::write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
	uint8_t cmd[8];

	if (chip == 51)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			setSS();
			_spi->transfer(0xF0);
			_spi->transfer(addr >> 8);
			_spi->transfer(addr & 0xFF);
			addr++;
			_spi->transfer(buf[i]);
			resetSS();
		}
	}
	else if (chip == 52)
	{
		setSS();
		cmd[0] = addr >> 8;
		cmd[1] = addr & 0xFF;
		cmd[2] = ((len >> 8) & 0x7F) | 0x80;
		cmd[3] = len & 0xFF;
		_spi->transfer(cmd, 4);
#ifdef SPI_HAS_TRANSFER_BUF
		_spi->transfer(buf, NULL, len);
#else
		// TODO: copy 8 bytes at a time to cmd[] and block transfer
		for (uint16_t i = 0; i < len; i++)
		{
			_spi->transfer(buf[i]);
		}
#endif
		resetSS();
	}
	else
	{ // chip == 55
		setSS();
		if (addr < 0x100)
		{
			// common registers 00nn
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = 0x04;
		}
		else if (addr < 0x8000)
		{
			// socket registers  10nn, 11nn, 12nn, 13nn, etc
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 3) & 0xE0) | 0x0C;
		}
		else if (addr < 0xC000)
		{
			// transmit buffers  8000-87FF, 8800-8FFF, 9000-97FF, etc
			//  10## #nnn nnnn nnnn
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
#if defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
			cmd[2] = 0x14; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x14; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 4
			cmd[2] = ((addr >> 7) & 0x60) | 0x14; // 4K buffers
#else
			cmd[2] = ((addr >> 6) & 0xE0) | 0x14; // 2K buffers
#endif
		}
		else
		{
			// receive buffers
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
#if defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
			cmd[2] = 0x1C; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x1C; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 4
			cmd[2] = ((addr >> 7) & 0x60) | 0x1C; // 4K buffers
#else
			cmd[2] = ((addr >> 6) & 0xE0) | 0x1C; // 2K buffers
#endif
		}
		if (len <= 5)
		{
			for (uint8_t i = 0; i < len; i++)
			{
				cmd[i + 3] = buf[i];
			}
			_spi->transfer(cmd, len + 3);
		}
		else
		{
			_spi->transfer(cmd, 3);
#ifdef SPI_HAS_TRANSFER_BUF
			_spi->transfer(buf, NULL, len);
#else
			for (uint16_t i = 0; i < len; i++)
			{
				_spi->transfer(buf[i]);
			}
#endif
		}
		resetSS();
	}
	return len;
}

uint16_t FlprogW5100Class::read(uint16_t addr, uint8_t *buf, uint16_t len)
{
	uint8_t cmd[4];

	if (chip == 51)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			setSS();
#if 1
			_spi->transfer(0x0F);
			_spi->transfer(addr >> 8);
			_spi->transfer(addr & 0xFF);
			addr++;
			buf[i] = _spi->transfer(0);
#else
			cmd[0] = 0x0F;
			cmd[1] = addr >> 8;
			cmd[2] = addr & 0xFF;
			cmd[3] = 0;
			_spi->transfer(cmd, 4); // TODO: why doesn't this work?
			buf[i] = cmd[3];
			addr++;
#endif
			resetSS();
		}
	}
	else if (chip == 52)
	{
		setSS();
		cmd[0] = addr >> 8;
		cmd[1] = addr & 0xFF;
		cmd[2] = (len >> 8) & 0x7F;
		cmd[3] = len & 0xFF;
		_spi->transfer(cmd, 4);
		memset(buf, 0, len);
		_spi->transfer(buf, len);
		resetSS();
	}
	else
	{ // chip == 55
		setSS();
		if (addr < 0x100)
		{
			// common registers 00nn
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = 0x00;
		}
		else if (addr < 0x8000)
		{
			// socket registers  10nn, 11nn, 12nn, 13nn, etc
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 3) & 0xE0) | 0x08;
		}
		else if (addr < 0xC000)
		{
			// transmit buffers  8000-87FF, 8800-8FFF, 9000-97FF, etc
			//  10## #nnn nnnn nnnn
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
#if defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
			cmd[2] = 0x10; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x10; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 4
			cmd[2] = ((addr >> 7) & 0x60) | 0x10; // 4K buffers
#else
			cmd[2] = ((addr >> 6) & 0xE0) | 0x10; // 2K buffers
#endif
		}
		else
		{
			// receive buffers
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
#if defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 1
			cmd[2] = 0x18; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x18; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && FLPROG_ETHERNET_MAX_SOCK_NUM <= 4
			cmd[2] = ((addr >> 7) & 0x60) | 0x18; // 4K buffers
#else
			cmd[2] = ((addr >> 6) & 0xE0) | 0x18; // 2K buffers
#endif
		}
		_spi->transfer(cmd, 3);
		memset(buf, 0, len);
		_spi->transfer(buf, len);
		resetSS();
	}
	return len;
}

void FlprogW5100Class::execCmdSn(SOCKET s, uint8_t _cmd)
{
	writeSn(s, FLPROG_SN_CR, _cmd);
	while (readSn(s, FLPROG_SN_CR))
		;
}

/*****************************************/
/*          Socket management            */
/*****************************************/

void FlprogW5100Class::socketPortRand(uint16_t n)
{
	n &= 0x3FFF;
	local_port ^= n;
}

uint8_t FlprogW5100Class::socketBegin(uint8_t protocol, uint16_t port)
{
	uint8_t s, status[FLPROG_ETHERNET_MAX_SOCK_NUM], maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	if (!chip)
		return FLPROG_ETHERNET_MAX_SOCK_NUM; // immediate error if no hardware detected
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	for (s = 0; s < maxindex; s++)
	{
		status[s] = readSn(s, FLPROG_SN_SR);
		if (status[s] == FLPROG_SN_SR_CLOSED)
		{
			privateMaceSocet(s, protocol, port);
			_spi->endTransaction();
			return s;
		}
	}
	for (s = 0; s < maxindex; s++)
	{
		if ((status[s] == FLPROG_SN_SR_LAST_ACK) || (status[s] == FLPROG_SN_SR_TIME_WAIT) || (status[s] == FLPROG_SN_SR_CLOSING))
		{
			execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
			privateMaceSocet(s, protocol, port);
			_spi->endTransaction();
			return s;
		}
	}
	_spi->endTransaction();
	return FLPROG_ETHERNET_MAX_SOCK_NUM; // all sockets are in use
}

void FlprogW5100Class::privateMaceSocet(uint8_t soc, uint8_t protocol, uint16_t port)
{
	delayMicroseconds(250); // TODO: is this needed??
	writeSn(soc, FLPROG_SN_MR, protocol);
	writeSn(soc, FLPROG_SN_IR, 0xFF);
	if (port > 0)
	{
		writeSn16(soc, FLPROG_SN_PORT, port);
	}
	else
	{
		if (++local_port < 49152)
			local_port = 49152;
		writeSn16(soc, FLPROG_SN_PORT, local_port);
	}
	execCmdSn(soc, FLPROG_SOCK_CMD_OPEN);
	state[soc].RX_RSR = 0;
	state[soc].RX_RD = readSn16(soc, FLPROG_SN_RX_RD); // always zero?
	state[soc].RX_inc = 0;
	state[soc].TX_FSR = 0;
}

uint8_t FlprogW5100Class::socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port)
{
	uint8_t s, status[FLPROG_ETHERNET_MAX_SOCK_NUM], maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	if (!chip)
		return FLPROG_ETHERNET_MAX_SOCK_NUM;
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4;
#endif
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	for (s = 0; s < maxindex; s++)
	{
		status[s] = readSn(s, FLPROG_SN_SR);
		if (status[s] == FLPROG_SN_SR_CLOSED)
		{
			privateMaceSocetMulticast(s, protocol, ip, port);
			_spi->endTransaction();
			return s;
		}
	}
	for (s = 0; s < maxindex; s++)
	{
		if ((status[s] == FLPROG_SN_SR_LAST_ACK) || (status[s] == FLPROG_SN_SR_TIME_WAIT) || (status[s] == FLPROG_SN_SR_CLOSING))
		{
			execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
			privateMaceSocetMulticast(s, protocol, ip, port);
			_spi->endTransaction();
			return s;
		}
	}
	_spi->endTransaction();
	return FLPROG_ETHERNET_MAX_SOCK_NUM;
}

void FlprogW5100Class::privateMaceSocetMulticast(uint8_t soc, uint8_t protocol, IPAddress ip, uint16_t port)
{
	delayMicroseconds(250); // TODO: is this needed??
	writeSn(soc, FLPROG_SN_MR, protocol);
	writeSn(soc, FLPROG_SN_IR, 0xFF);
	if (port > 0)
	{
		writeSn16(soc, FLPROG_SN_PORT, port);
	}
	else
	{
		if (++local_port < 49152)
			local_port = 49152;
		writeSn16(soc, FLPROG_SN_PORT, local_port);
	}
	byte mac[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
	mac[3] = ip[1] & 0x7F;
	mac[4] = ip[2];
	mac[5] = ip[3];
	uint8_t buf1[4];
	buf1[0] = ip[0];
	buf1[1] = ip[1];
	buf1[2] = ip[2];
	buf1[3] = ip[3];
	writeSn(soc, FLPROG_SN_DIPR, buf1, 4); // 239.255.0.1
	writeSn16(soc, FLPROG_SN_DPORT, port);
	writeSn(soc, FLPROG_SN_DHAR, mac, 6);
	execCmdSn(soc, FLPROG_SOCK_CMD_OPEN);
	state[soc].RX_RSR = 0;
	state[soc].RX_RD = readSn16(soc, FLPROG_SN_RX_RD); // always zero?
	state[soc].RX_inc = 0;
	state[soc].TX_FSR = 0;
}

uint8_t FlprogW5100Class::socketStatus(uint8_t s)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	uint8_t status = readSn(s, FLPROG_SN_SR);
	_spi->endTransaction();
	return status;
}

void FlprogW5100Class::socketClose(uint8_t s)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
	_spi->endTransaction();
}

uint8_t FlprogW5100Class::socketListen(uint8_t s)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	if (readSn(s, FLPROG_SN_SR) != FLPROG_SN_SR_INIT)
	{
		_spi->endTransaction();
		return 0;
	}
	execCmdSn(s, FLPROG_SOCK_CMD_LISTEN);
	_spi->endTransaction();
	return 1;
}

void FlprogW5100Class::socketConnect(uint8_t s, IPAddress ip, uint16_t port)
{

	uint8_t buffer[4];
	buffer[0] = ip[0];
	buffer[1] = ip[1];
	buffer[2] = ip[2];
	buffer[3] = ip[3];
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	writeSn(s, FLPROG_SN_DIPR, buffer, 4);
	writeSn16(s, FLPROG_SN_DPORT, port);
	execCmdSn(s, FLPROG_SOCK_CMD_CONNECT);
	_spi->endTransaction();
}

void FlprogW5100Class::socketDisconnect(uint8_t s)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	execCmdSn(s, FLPROG_SOCK_CMD_DISCON);
	_spi->endTransaction();
}

/*****************************************/
/*    Socket Data Receive Functions      */
/*****************************************/

uint16_t FlprogW5100Class::getSnRX_RSR(uint8_t s)
{
	uint16_t val, prev;
	prev = readSn16(s, FLPROG_SN_RX_RSR);
	while (1)
	{
		val = readSn16(s, FLPROG_SN_RX_RSR);
		if (val == prev)
		{
			return val;
		}
		prev = val;
	}
}

void FlprogW5100Class::read_data(uint8_t s, uint16_t src, uint8_t *dst, uint16_t len)
{
	uint16_t size;
	uint16_t src_mask;
	uint16_t src_ptr;
	src_mask = ((uint16_t)src) & SMASK;
	src_ptr = RBASE(s) + src_mask;
	if (hasOffsetAddressMapping() || src_mask + len <= SSIZE)
	{
		read(src_ptr, dst, len);
	}
	else
	{
		size = SSIZE - src_mask;
		read(src_ptr, dst, size);
		dst += size;
		read(RBASE(s), dst, len - size);
	}
}

int FlprogW5100Class::socketRecv(uint8_t s, uint8_t *buf, int16_t len)
{
	int ret = state[s].RX_RSR;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	if (ret < len)
	{
		uint16_t rsr = getSnRX_RSR(s);
		ret = rsr - state[s].RX_inc;
		state[s].RX_RSR = ret;
	}
	if (ret == 0)
	{
		uint8_t status = readSn(s, FLPROG_SN_CR);
		if ((status == FLPROG_SN_SR_LISTEN) || (status == FLPROG_SN_SR_CLOSED) ||
			(status == FLPROG_SN_SR_CLOSE_WAIT))
		{
			ret = 0;
		}
		else
		{
			ret = -1;
		}
	}
	else
	{
		if (ret > len)
			ret = len;
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
			writeSn16(s, FLPROG_SN_RX_RD, ptr);
			execCmdSn(s, FLPROG_SOCK_CMD_RECV);
		}
		else
		{
			state[s].RX_inc = inc;
		}
	}
	_spi->endTransaction();
	return ret;
}

uint16_t FlprogW5100Class::socketRecvAvailable(uint8_t s)
{
	uint16_t ret = state[s].RX_RSR;
	if (ret == 0)
	{
		_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
		uint16_t rsr = getSnRX_RSR(s);
		_spi->endTransaction();
		ret = rsr - state[s].RX_inc;
		state[s].RX_RSR = ret;
	}
	return ret;
}

uint8_t FlprogW5100Class::socketPeek(uint8_t s)
{
	uint8_t b;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	uint16_t ptr = state[s].RX_RD;
	read((ptr & SMASK) + RBASE(s), &b, 1);
	_spi->endTransaction();
	return b;
}

/*****************************************/
/*    Socket Data Transmit Functions     */
/*****************************************/

uint16_t FlprogW5100Class::getSnTX_FSR(uint8_t s)
{
	uint16_t val, prev;
	prev = readSn16(s, FLPROG_SN_TX_FSR);
	while (1)
	{
		val = readSn16(s, FLPROG_SN_TX_FSR);
		if (val == prev)
		{
			state[s].TX_FSR = val;
			return val;
		}
		prev = val;
	}
}

void FlprogW5100Class::write_data(uint8_t s, uint16_t data_offset, const uint8_t *data, uint16_t len)
{
	uint16_t ptr = readSn16(s, FLPROG_SN_TX_WR);
	ptr += data_offset;
	uint16_t offset = ptr & SMASK;
	uint16_t dstAddr = offset + SBASE(s);

	if (hasOffsetAddressMapping() || offset + len <= SSIZE)
	{
		write(dstAddr, data, len);
	}
	else
	{
		// Wrap around circular buffer
		uint16_t size = SSIZE - offset;
		write(dstAddr, data, size);
		write(SBASE(s), data + size, len - size);
	}
	ptr += len;
	writeSn16(s, FLPROG_SN_TX_WR, ptr);
}

uint16_t FlprogW5100Class::socketSend(uint8_t s, const uint8_t *buf, uint16_t len)
{
	uint8_t status = 0;
	uint16_t ret = 0;
	uint16_t freesize = 0;
	if (len > SSIZE)
	{
		ret = SSIZE; // check size not to exceed MAX size.
	}
	else
	{
		ret = len;
	}
	do
	{
		_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
		freesize = getSnTX_FSR(s);
		status = readSn(s, FLPROG_SN_SR);
		_spi->endTransaction();
		if ((status != FLPROG_SN_SR_ESTABLISHED) && (status != FLPROG_SN_SR_CLOSE_WAIT))
		{
			ret = 0;
			break;
		}
		yield();
	} while (freesize < ret);
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	write_data(s, 0, (uint8_t *)buf, ret);
	execCmdSn(s, FLPROG_SOCK_CMD_SEND);
	while ((readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_SEND_OK) != FLPROG_SN_IR_SEND_OK)
	{
		if (readSn(s, FLPROG_SN_SR) == FLPROG_SN_SR_CLOSED)
		{
			_spi->endTransaction();
			return 0;
		}
		_spi->endTransaction();
		yield();
		_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	}
	/* +2008.01 bj */
	writeSn(s, FLPROG_SN_IR, FLPROG_SN_IR_SEND_OK);
	_spi->endTransaction();
	return ret;
}

uint16_t FlprogW5100Class::socketSendAvailable(uint8_t s)
{
	uint8_t status = 0;
	uint16_t freesize = 0;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	freesize = getSnTX_FSR(s);
	status = readSn(s, FLPROG_SN_SR);
	_spi->endTransaction();
	if ((status == FLPROG_SN_SR_ESTABLISHED) || (status == FLPROG_SN_SR_CLOSE_WAIT))
	{
		return freesize;
	}
	return 0;
}

uint16_t FlprogW5100Class::socketBufferData(uint8_t s, uint16_t offset, const uint8_t *buf, uint16_t len)
{
	uint16_t ret = 0;
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	uint16_t txfree = getSnTX_FSR(s);
	if (len > txfree)
	{
		ret = txfree;
	}
	else
	{
		ret = len;
	}
	write_data(s, offset, buf, ret);
	_spi->endTransaction();
	return ret;
}

bool FlprogW5100Class::socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port)
{
	if (((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		((port == 0x00)))
	{
		return false;
	}
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	writeSn(s, FLPROG_SN_DIPR, addr, 4);
	writeSn16(s, FLPROG_SN_DPORT, port);
	_spi->endTransaction();
	return true;
}

bool FlprogW5100Class::socketSendUDP(uint8_t s)
{
	_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	execCmdSn(s, FLPROG_SOCK_CMD_SEND);
	while ((readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_SEND_OK) != FLPROG_SN_IR_SEND_OK)
	{
		if (readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_TIMEOUT)
		{
			writeSn(s, FLPROG_SN_IR, (FLPROG_SN_IR_SEND_OK | FLPROG_SN_IR_TIMEOUT));
			_spi->endTransaction();
			return false;
		}
		_spi->endTransaction();
		yield();
		_spi->beginTransaction(SPI_ETHERNET_SETTINGS);
	}
	writeSn(s, FLPROG_SN_IR, FLPROG_SN_IR_SEND_OK);
	_spi->endTransaction();
	return true;
}
