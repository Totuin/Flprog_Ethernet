#include <Arduino.h>
#include "flprogEthernet.h"
#include "flprogW5100.h"

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
	write16(FLPROG_RTR, timeout);
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
	Serial.print("Init3 - ");
	Serial.println(initialized);
	if (initialized)
		return 1;
	delay(560);
	// Serial.println("w5100 init");
	Serial.print("Init4");
	Serial.println(initialized);
	SPI.begin();
	initSS();
	resetSS();
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	if (isW5200())
	{
		CH_BASE_MSB = 0x40;
#ifdef ETHERNET_LARGE_BUFFERS
#if MAX_SOCK_NUM <= 1
		SSIZE = 16384;
#elif MAX_SOCK_NUM <= 2
		SSIZE = 8192;
#elif MAX_SOCK_NUM <= 4
		SSIZE = 4096;
#else
		SSIZE = 2048;
#endif
		SMASK = SSIZE - 1;
#endif
		for (i = 0; i < MAX_SOCK_NUM; i++)
		{
			writeSn(i, FLPROG_SN_RX_SIZE, SSIZE >> 10);
			writeSn(i, FLPROG_SN_TX_SIZE, SSIZE >> 10);
		}
		for (; i < 8; i++)
		{
			writeSn(i, FLPROG_SN_RX_SIZE, 0);
			writeSn(i, FLPROG_SN_TX_SIZE, 0);
		}
		// Try W5500 next.  Wiznet finally seems to have implemented
		// SPI well with this chip.  It appears to be very resilient,
		// so try it after the fragile W5200
	}
	else
	{
		if (isW5500())
		{
			CH_BASE_MSB = 0x10;
#ifdef ETHERNET_LARGE_BUFFERS
#if MAX_SOCK_NUM <= 1
			SSIZE = 16384;
#elif MAX_SOCK_NUM <= 2
			SSIZE = 8192;
#elif MAX_SOCK_NUM <= 4
			SSIZE = 4096;
#else
			SSIZE = 2048;
#endif
			SMASK = SSIZE - 1;
			for (i = 0; i < MAX_SOCK_NUM; i++)
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
#if MAX_SOCK_NUM <= 1
				SSIZE = 8192;
				write(FLPROG_TMSR, 0x03);
				write(FLPROG_RMSR, 0x03);
#elif MAX_SOCK_NUM <= 2
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
				SPI.endTransaction();
				return 0; // no known chip is responding :-(
			}
		}
	}
	SPI.endTransaction();
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
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		phystatus = read(FLPROG_PSTATUS_W5200);
		SPI.endTransaction();
		if (phystatus & 0x20)
			return FLPROG_ETHERNET_LINK_ON;
		return FLPROG_ETHERNET_LINK_OFF;
	case 55:
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		phystatus = read(FLPROG_PHYCFGR_W5500);
		SPI.endTransaction();
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
			SPI.transfer(0xF0);
			SPI.transfer(addr >> 8);
			SPI.transfer(addr & 0xFF);
			addr++;
			SPI.transfer(buf[i]);
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
		SPI.transfer(cmd, 4);
#ifdef SPI_HAS_TRANSFER_BUF
		SPI.transfer(buf, NULL, len);
#else
		// TODO: copy 8 bytes at a time to cmd[] and block transfer
		for (uint16_t i = 0; i < len; i++)
		{
			SPI.transfer(buf[i]);
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
#if defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 1
			cmd[2] = 0x14; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x14; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 4
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
#if defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 1
			cmd[2] = 0x1C; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x1C; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 4
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
			SPI.transfer(cmd, len + 3);
		}
		else
		{
			SPI.transfer(cmd, 3);
#ifdef SPI_HAS_TRANSFER_BUF
			SPI.transfer(buf, NULL, len);
#else
			// TODO: copy 8 bytes at a time to cmd[] and block transfer
			for (uint16_t i = 0; i < len; i++)
			{
				SPI.transfer(buf[i]);
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
			SPI.transfer(0x0F);
			SPI.transfer(addr >> 8);
			SPI.transfer(addr & 0xFF);
			addr++;
			buf[i] = SPI.transfer(0);
#else
			cmd[0] = 0x0F;
			cmd[1] = addr >> 8;
			cmd[2] = addr & 0xFF;
			cmd[3] = 0;
			SPI.transfer(cmd, 4); // TODO: why doesn't this work?
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
		SPI.transfer(cmd, 4);
		memset(buf, 0, len);
		SPI.transfer(buf, len);
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
#if defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 1
			cmd[2] = 0x10; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x10; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 4
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
#if defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 1
			cmd[2] = 0x18; // 16K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 2
			cmd[2] = ((addr >> 8) & 0x20) | 0x18; // 8K buffers
#elif defined(ETHERNET_LARGE_BUFFERS) && MAX_SOCK_NUM <= 4
			cmd[2] = ((addr >> 7) & 0x60) | 0x18; // 4K buffers
#else
			cmd[2] = ((addr >> 6) & 0xE0) | 0x18; // 2K buffers
#endif
		}
		SPI.transfer(cmd, 3);
		memset(buf, 0, len);
		SPI.transfer(buf, len);
		resetSS();
	}
	return len;
}

void FlprogW5100Class::execCmdSn(SOCKET s, uint8_t _cmd)
{
	// Send command to socket
	writeSn(s, FLPROG_SN_CR, _cmd);
	// Wait for command to complete
	while (readSn(s, FLPROG_SN_CR))
		;
}
