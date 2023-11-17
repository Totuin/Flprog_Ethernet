#include "flprogW5100.h"

uint8_t FLProgWiznetClass::init(void)
{
	uint8_t i;
	if (initialized)
		return 1;
	delay(560);
	RT_HW_Base.spiBegin(spiBus);
	initSS();
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	if (isW5200())
	{
		CH_BASE_MSB = 0x40;
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
		}
		else
		{
			if (isW5100())
			{
				CH_BASE_MSB = 0x04;
			}
			else
			{
				chip = 0;
				RT_HW_Base.spiEndTransaction(spiBus);
				return 0;
			}
		}
	}
	RT_HW_Base.spiEndTransaction(spiBus);
	initialized = true;
	return 1;
}

void FLProgWiznetClass::setSsPin(int sspin)
{

	if (sspin < 0)
	{
		return;
	}
	if (_pinSS == sspin)
	{
		return;
	}
	_pinSS = sspin;
	initSS();
}

void FLProgWiznetClass::setNetSettings(uint8_t *mac, IPAddress ip)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	setMACAddress(mac);
	setIPAddress(ip);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::setNetSettings(IPAddress ip, IPAddress gateway, IPAddress subnet)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	setIPAddress(ip);
	setGatewayIp(gateway);
	setSubnetMask(subnet);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::setNetSettings(uint8_t *mac, IPAddress ip, IPAddress gateway, IPAddress subnet)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	setMACAddress(mac);
	setIPAddress(ip);
	setGatewayIp(gateway);
	setSubnetMask(subnet);
	RT_HW_Base.spiEndTransaction(spiBus);
}

IPAddress FLProgWiznetClass::localIP()
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	IPAddress result = getIPAddress();
	RT_HW_Base.spiEndTransaction(spiBus);
	return result;
}

IPAddress FLProgWiznetClass::subnetMask()
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	IPAddress result = getSubnetMask();
	RT_HW_Base.spiEndTransaction(spiBus);
	return result;
}

IPAddress FLProgWiznetClass::gatewayIP()
{
	IPAddress ret;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	IPAddress result = getGatewayIp();
	RT_HW_Base.spiEndTransaction(spiBus);
	return result;
}

void FLProgWiznetClass::setOnlyMACAddress(const uint8_t *mac_address)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	setMACAddress(mac_address);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::setOnlyLocalIP(const IPAddress local_ip)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	IPAddress ip = local_ip;
	setIPAddress(ip);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::setOnlySubnetMask(const IPAddress subnet)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	IPAddress ip = subnet;
	setSubnetMask(ip);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::setOnlyGatewayIP(const IPAddress gateway)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	IPAddress ip = gateway;
	setGatewayIp(ip);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::MACAddress(uint8_t *mac_address)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	getMACAddress(mac_address);
	RT_HW_Base.spiEndTransaction(spiBus);
}

uint16_t FLProgWiznetClass::localPort(uint8_t soc)
{
	if (soc >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	uint16_t port;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	port = readSn16(soc, FLPROG_SN_PORT);
	RT_HW_Base.spiEndTransaction(spiBus);
	return port;
}

IPAddress FLProgWiznetClass::remoteIP(uint8_t soc)
{
	if (soc >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return IPAddress((uint32_t)0);
	uint8_t remoteIParray[4];
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	readSn(soc, FLPROG_SN_DIPR, remoteIParray, 4);
	RT_HW_Base.spiEndTransaction(spiBus);
	return IPAddress(remoteIParray);
}

uint16_t FLProgWiznetClass::remotePort(uint8_t soc)
{
	if (soc >= FLPROG_ETHERNET_MAX_SOCK_NUM)
		return 0;
	uint16_t port;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	port = readSn16(soc, FLPROG_SN_DPORT);
	RT_HW_Base.spiEndTransaction(spiBus);
	return port;
}

uint16_t FLProgWiznetClass::SBASE(uint8_t socknum)
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

uint16_t FLProgWiznetClass::RBASE(uint8_t socknum)
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

bool FLProgWiznetClass::hasOffsetAddressMapping(void)
{
	if (chip == 55)
		return true;
	return false;
}

void FLProgWiznetClass::setIPAddress(IPAddress addr)
{
	uint8_t buffer[4];
	buffer[0] = addr[0];
	buffer[1] = addr[1];
	buffer[2] = addr[2];
	buffer[3] = addr[3];
	write(FLPROG_SIPR, buffer, 4);
}

IPAddress FLProgWiznetClass::getIPAddress()
{
	uint8_t buffer[4] = {0, 0, 0, 0};
	read(FLPROG_SIPR, buffer, 4);
	return IPAddress(buffer[0], buffer[1], buffer[2], buffer[3]);
}

void FLProgWiznetClass::setGatewayIp(IPAddress addr)
{
	uint8_t buffer[4];
	buffer[0] = addr[0];
	buffer[1] = addr[1];
	buffer[2] = addr[2];
	buffer[3] = addr[3];
	write(FLPROG_GAR, buffer, 4);
}

IPAddress FLProgWiznetClass::getGatewayIp()
{
	uint8_t buffer[4] = {0, 0, 0, 0};
	read(FLPROG_GAR, buffer, 4);
	return IPAddress(buffer[0], buffer[1], buffer[2], buffer[3]);
}

void FLProgWiznetClass::setSubnetMask(IPAddress addr)
{
	uint8_t buffer[4];
	buffer[0] = addr[0];
	buffer[1] = addr[1];
	buffer[2] = addr[2];
	buffer[3] = addr[3];
	write(FLPROG_SUBR, buffer, 4);
}

IPAddress FLProgWiznetClass::getSubnetMask()
{
	uint8_t buffer[4] = {0, 0, 0, 0};
	read(FLPROG_SUBR, buffer, 4);
	return IPAddress(buffer[0], buffer[1], buffer[2], buffer[3]);
}

void FLProgWiznetClass::setRetransmissionTime(uint16_t timeout)
{
	if (timeout > 6553)
		timeout = 6553;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	write16(FLPROG_RTR, (timeout * 10));
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::setRetransmissionCount(uint8_t retry)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	write(FLPROG_RCR, retry);
	RT_HW_Base.spiEndTransaction(spiBus);
}

uint8_t FLProgWiznetClass::read(uint16_t addr)
{
	uint8_t data;
	read(addr, &data, 1);
	return data;
}

uint16_t FLProgWiznetClass::readSn16(SOCKET _s, uint16_t address)
{
	uint8_t buf[2];
	readSn(_s, address, buf, 2);
	return (buf[0] << 8) | buf[1];
}

void FLProgWiznetClass::writeSn16(SOCKET _s, uint16_t address, uint16_t _data)
{
	uint8_t buf[2];
	buf[0] = _data >> 8;
	buf[1] = _data & 0xFF;
	writeSn(_s, address, buf, 2);
}

void FLProgWiznetClass::write16(uint16_t address, uint16_t _data)
{
	uint8_t buf[2];
	buf[0] = _data >> 8;
	buf[1] = _data & 0xFF;
	write(address, buf, 2);
}

uint8_t FLProgWiznetClass::softReset(void)
{
	uint16_t count = 0;
	write(FLPROG_MR, 0x80);
	do
	{
		uint8_t mr = read(FLPROG_MR);

		if (mr == 0)
			return 1;
		delay(1);
	} while (++count < 20);
	return 0;
}

uint8_t FLProgWiznetClass::isW5100(void)
{
	chip = 51;
	if (!softReset())
	{
		return 0;
	}
	write(FLPROG_MR, 0x10);
	if (read(FLPROG_MR) != 0x10)
	{
		return 0;
	}
	write(FLPROG_MR, 0x12);
	if (read(FLPROG_MR) != 0x12)
	{
		return 0;
	}
	write(FLPROG_MR, 0x00);
	if (read(FLPROG_MR) != 0x00)
	{
		return 0;
	}
	return 1;
}

uint8_t FLProgWiznetClass::isW5200(void)
{
	chip = 52;
	if (!softReset())
	{
		return 0;
	}
	write(FLPROG_MR, 0x08);
	if (read(FLPROG_MR) != 0x08)
	{
		return 0;
	}
	write(FLPROG_MR, 0x10);
	if (read(FLPROG_MR) != 0x10)
	{
		return 0;
	}
	write(FLPROG_MR, 0x00);
	if (read(FLPROG_MR) != 0x00)
	{
		return 0;
	}
	int ver = read(FLPROG_VERSIONR_W5200);
	if (ver != 3)
	{
		return 0;
	}
	return 1;
}

uint8_t FLProgWiznetClass::isW5500(void)
{
	chip = 55;
	if (!softReset())
	{
		return 0;
	}
	write(FLPROG_MR, 0x08);
	if (read(FLPROG_MR) != 0x08)
	{
		return 0;
	}
	write(FLPROG_MR, 0x10);
	if (read(FLPROG_MR) != 0x10)
	{
		return 0;
	}
	write(FLPROG_MR, 0x00);
	if (read(FLPROG_MR) != 0x00)
		return 0;
	int ver = read(FLPROG_VERSIONR_W5500);
	if (ver != 4)
	{
		return 0;
	}
	return 1;
}

uint8_t FLProgWiznetClass::getLinkStatus()
{
	uint8_t phystatus;
	if (!init())
		return FLPROG_ETHERNET_LINK_UNKNOWN;
	switch (chip)
	{
	case 52:
		RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
		phystatus = read(FLPROG_PSTATUS_W5200);
		RT_HW_Base.spiEndTransaction(spiBus);
		if (phystatus & 0x20)
			return FLPROG_ETHERNET_LINK_ON;
		return FLPROG_ETHERNET_LINK_OFF;
	case 55:
		RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
		phystatus = read(FLPROG_PHYCFGR_W5500);
		RT_HW_Base.spiEndTransaction(spiBus);
		if (phystatus & 0x01)
			return FLPROG_ETHERNET_LINK_ON;
		return FLPROG_ETHERNET_LINK_OFF;
	default:
		return FLPROG_ETHERNET_LINK_UNKNOWN;
	}
}

uint16_t FLProgWiznetClass::write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
	uint8_t cmd[8];
	if (chip == 51)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			setSS();
			RT_HW_Base.spiTransfer(0xF0, spiBus);
			RT_HW_Base.spiTransfer((addr >> 8), spiBus);
			RT_HW_Base.spiTransfer((addr & 0xFF), spiBus);
			addr++;
			RT_HW_Base.spiTransfer((buf[i]), spiBus);
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
		for (uint8_t i = 0; i < 4; i++)
		{
			RT_HW_Base.spiTransfer(cmd[i], spiBus);
		}
		for (uint16_t i = 0; i < len; i++)
		{
			RT_HW_Base.spiTransfer((buf[i]), spiBus);
		}
		resetSS();
	}
	else
	{ // chip == 55
		setSS();
		if (addr < 0x100)
		{
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = 0x04;
		}
		else if (addr < 0x8000)
		{
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 3) & 0xE0) | 0x0C;
		}
		else if (addr < 0xC000)
		{
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 6) & 0xE0) | 0x14; // 2K buffers
		}
		else
		{
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 6) & 0xE0) | 0x1C; // 2K buffers
		}
		if (len <= 5)
		{
			for (uint8_t i = 0; i < len; i++)
			{
				cmd[i + 3] = buf[i];
			}
			for (uint16_t i = 0; i < (len + 3); i++)
			{
				RT_HW_Base.spiTransfer(cmd[i], spiBus);
			}
		}
		else
		{
			for (uint8_t i = 0; i < 3; i++)
			{
				RT_HW_Base.spiTransfer(cmd[i], spiBus);
			}
			for (uint16_t i = 0; i < len; i++)
			{
				RT_HW_Base.spiTransfer(buf[i], spiBus);
			}
		}
		resetSS();
	}
	return len;
}

uint16_t FLProgWiznetClass::read(uint16_t addr, uint8_t *buf, uint16_t len)
{
	uint8_t cmd[4];
	if (chip == 51)
	{
		for (uint16_t i = 0; i < len; i++)
		{
			setSS();
			RT_HW_Base.spiTransfer(0x0F, spiBus);
			RT_HW_Base.spiTransfer((addr >> 8), spiBus);
			RT_HW_Base.spiTransfer((addr & 0xFF), spiBus);
			addr++;
			buf[i] = RT_HW_Base.spiTransfer(0, spiBus);
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
		for (uint8_t i = 0; i < 4; i++)
		{
			RT_HW_Base.spiTransfer(cmd[i], spiBus);
		}
		for (uint16_t i = 0; i < len; i++)
		{
			buf[i] = RT_HW_Base.spiTransfer(0, spiBus);
		}
		resetSS();
	}
	else
	{ // chip == 55
		setSS();
		if (addr < 0x100)
		{
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = 0x00;
		}
		else if (addr < 0x8000)
		{
			cmd[0] = 0;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 3) & 0xE0) | 0x08;
		}
		else if (addr < 0xC000)
		{
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 6) & 0xE0) | 0x10; // 2K buffers
		}
		else
		{
			cmd[0] = addr >> 8;
			cmd[1] = addr & 0xFF;
			cmd[2] = ((addr >> 6) & 0xE0) | 0x18; // 2K buffers
		}
		for (uint8_t i = 0; i < 3; i++)
		{
			RT_HW_Base.spiTransfer(cmd[i], spiBus);
		}
		for (uint16_t i = 0; i < len; i++)
		{
			buf[i] = RT_HW_Base.spiTransfer(0, spiBus);
		}
		resetSS();
	}
	return len;
}

void FLProgWiznetClass::execCmdSn(SOCKET s, uint8_t _cmd)
{
	writeSn(s, FLPROG_SN_CR, _cmd);
	while (readSn(s, FLPROG_SN_CR))
		;
}

/*****************************************/
/*          Socket management            */
/*****************************************/

void FLProgWiznetClass::socketPortRand(uint16_t n)
{
	n &= 0x3FFF;
	local_port ^= n;
}

uint8_t FLProgWiznetClass::socketBegin(uint8_t protocol, uint16_t port)
{
	uint8_t s, status[FLPROG_ETHERNET_MAX_SOCK_NUM], maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	if (!chip)
	{
		return FLPROG_ETHERNET_MAX_SOCK_NUM; // immediate error if no hardware detected
	}
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	for (s = 0; s < maxindex; s++)
	{
		status[s] = readSn(s, FLPROG_SN_SR);
		if (status[s] == FLPROG_SN_SR_CLOSED)
		{
			privateMaceSocet(s, protocol, port);
			RT_HW_Base.spiEndTransaction(spiBus);
			return s;
		}
	}
	for (s = 0; s < maxindex; s++)
	{
		if ((status[s] == FLPROG_SN_SR_LAST_ACK) || (status[s] == FLPROG_SN_SR_TIME_WAIT) || (status[s] == FLPROG_SN_SR_CLOSING))
		{
			execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
			privateMaceSocet(s, protocol, port);
			RT_HW_Base.spiEndTransaction(spiBus);
			return s;
		}
	}
	RT_HW_Base.spiEndTransaction(spiBus);
	return FLPROG_ETHERNET_MAX_SOCK_NUM; // all sockets are in use
}

void FLProgWiznetClass::privateMaceSocet(uint8_t soc, uint8_t protocol, uint16_t port)
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

uint8_t FLProgWiznetClass::socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port)
{
	uint8_t s, status[FLPROG_ETHERNET_MAX_SOCK_NUM], maxindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
	if (!chip)
		return FLPROG_ETHERNET_MAX_SOCK_NUM;
#if FLPROG_ETHERNET_MAX_SOCK_NUM > 4
	if (chip == 51)
		maxindex = 4;
#endif
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	for (s = 0; s < maxindex; s++)
	{
		status[s] = readSn(s, FLPROG_SN_SR);
		if (status[s] == FLPROG_SN_SR_CLOSED)
		{
			privateMaceSocetMulticast(s, protocol, ip, port);
			RT_HW_Base.spiEndTransaction(spiBus);
			return s;
		}
	}
	for (s = 0; s < maxindex; s++)
	{
		if ((status[s] == FLPROG_SN_SR_LAST_ACK) || (status[s] == FLPROG_SN_SR_TIME_WAIT) || (status[s] == FLPROG_SN_SR_CLOSING))
		{
			execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
			privateMaceSocetMulticast(s, protocol, ip, port);
			RT_HW_Base.spiEndTransaction(spiBus);
			return s;
		}
	}
	RT_HW_Base.spiEndTransaction(spiBus);
	return FLPROG_ETHERNET_MAX_SOCK_NUM;
}

void FLProgWiznetClass::privateMaceSocetMulticast(uint8_t soc, uint8_t protocol, IPAddress ip, uint16_t port)
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

uint8_t FLProgWiznetClass::socketStatus(uint8_t s)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	uint8_t status = readSn(s, FLPROG_SN_SR);
	RT_HW_Base.spiEndTransaction(spiBus);
	return status;
}

void FLProgWiznetClass::socketClose(uint8_t s)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	execCmdSn(s, FLPROG_SOCK_CMD_CLOSE);
	RT_HW_Base.spiEndTransaction(spiBus);
}

uint8_t FLProgWiznetClass::socketListen(uint8_t s)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	if (readSn(s, FLPROG_SN_SR) != FLPROG_SN_SR_INIT)
	{
		RT_HW_Base.spiEndTransaction(spiBus);
		return 0;
	}
	execCmdSn(s, FLPROG_SOCK_CMD_LISTEN);
	RT_HW_Base.spiEndTransaction(spiBus);
	return 1;
}

void FLProgWiznetClass::socketConnect(uint8_t s, IPAddress ip, uint16_t port)
{
	uint8_t buffer[4];
	buffer[0] = ip[0];
	buffer[1] = ip[1];
	buffer[2] = ip[2];
	buffer[3] = ip[3];
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	writeSn(s, FLPROG_SN_DIPR, buffer, 4);
	writeSn16(s, FLPROG_SN_DPORT, port);
	execCmdSn(s, FLPROG_SOCK_CMD_CONNECT);
	RT_HW_Base.spiEndTransaction(spiBus);
}

void FLProgWiznetClass::socketDisconnect(uint8_t s)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	execCmdSn(s, FLPROG_SOCK_CMD_DISCON);
	RT_HW_Base.spiEndTransaction(spiBus);
}

/*****************************************/
/*    Socket Data Receive Functions      */
/*****************************************/

uint16_t FLProgWiznetClass::getSnRX_RSR(uint8_t s)
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

void FLProgWiznetClass::read_data(uint8_t s, uint16_t src, uint8_t *dst, uint16_t len)
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

int FLProgWiznetClass::socketRecv(uint8_t s, uint8_t *buf, int16_t len)
{
	int ret = state[s].RX_RSR;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
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
	RT_HW_Base.spiEndTransaction(spiBus);
	return ret;
}

uint16_t FLProgWiznetClass::socketRecvAvailable(uint8_t s)
{
	uint16_t ret = state[s].RX_RSR;
	if (ret == 0)
	{
		RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
		uint16_t rsr = getSnRX_RSR(s);
		RT_HW_Base.spiEndTransaction(spiBus);
		ret = rsr - state[s].RX_inc;
		state[s].RX_RSR = ret;
	}
	return ret;
}

uint8_t FLProgWiznetClass::socketPeek(uint8_t s)
{
	uint8_t b;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	uint16_t ptr = state[s].RX_RD;
	read((ptr & SMASK) + RBASE(s), &b, 1);
	RT_HW_Base.spiEndTransaction(spiBus);
	return b;
}

/*****************************************/
/*    Socket Data Transmit Functions     */
/*****************************************/

uint16_t FLProgWiznetClass::getSnTX_FSR(uint8_t s)
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

void FLProgWiznetClass::write_data(uint8_t s, uint16_t data_offset, const uint8_t *data, uint16_t len)
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
		uint16_t size = SSIZE - offset;
		write(dstAddr, data, size);
		write(SBASE(s), data + size, len - size);
	}
	ptr += len;
	writeSn16(s, FLPROG_SN_TX_WR, ptr);
}

uint16_t FLProgWiznetClass::socketSend(uint8_t s, const uint8_t *buf, uint16_t len)
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
		RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
		freesize = getSnTX_FSR(s);
		status = readSn(s, FLPROG_SN_SR);
		RT_HW_Base.spiEndTransaction(spiBus);
		if ((status != FLPROG_SN_SR_ESTABLISHED) && (status != FLPROG_SN_SR_CLOSE_WAIT))
		{
			ret = 0;
			break;
		}
		yield();
	} while (freesize < ret);
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	write_data(s, 0, (uint8_t *)buf, ret);
	execCmdSn(s, FLPROG_SOCK_CMD_SEND);
	while ((readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_SEND_OK) != FLPROG_SN_IR_SEND_OK)
	{
		if (readSn(s, FLPROG_SN_SR) == FLPROG_SN_SR_CLOSED)
		{
			RT_HW_Base.spiEndTransaction(spiBus);
			return 0;
		}
		RT_HW_Base.spiEndTransaction(spiBus);
		yield();
		RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	}
	/* +2008.01 bj */
	writeSn(s, FLPROG_SN_IR, FLPROG_SN_IR_SEND_OK);
	RT_HW_Base.spiEndTransaction(spiBus);
	return ret;
}

uint16_t FLProgWiznetClass::socketSendAvailable(uint8_t s)
{
	uint8_t status = 0;
	uint16_t freesize = 0;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	freesize = getSnTX_FSR(s);
	status = readSn(s, FLPROG_SN_SR);
	RT_HW_Base.spiEndTransaction(spiBus);
	if ((status == FLPROG_SN_SR_ESTABLISHED) || (status == FLPROG_SN_SR_CLOSE_WAIT))
	{
		return freesize;
	}
	return 0;
}

uint16_t FLProgWiznetClass::socketBufferData(uint8_t s, uint16_t offset, const uint8_t *buf, uint16_t len)
{
	uint16_t ret = 0;
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
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
	RT_HW_Base.spiEndTransaction(spiBus);
	return ret;
}

bool FLProgWiznetClass::socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port)
{
	if (((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		((port == 0x00)))
	{
		return false;
	}
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	writeSn(s, FLPROG_SN_DIPR, addr, 4);
	writeSn16(s, FLPROG_SN_DPORT, port);
	RT_HW_Base.spiEndTransaction(spiBus);
	return true;
}

bool FLProgWiznetClass::socketSendUDP(uint8_t s)
{
	RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	execCmdSn(s, FLPROG_SOCK_CMD_SEND);
	while ((readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_SEND_OK) != FLPROG_SN_IR_SEND_OK)
	{
		if (readSn(s, FLPROG_SN_IR) & FLPROG_SN_IR_TIMEOUT)
		{
			writeSn(s, FLPROG_SN_IR, (FLPROG_SN_IR_SEND_OK | FLPROG_SN_IR_TIMEOUT));
			RT_HW_Base.spiEndTransaction(spiBus);
			return false;
		}
		RT_HW_Base.spiEndTransaction(spiBus);
		yield();
		RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, spiBus);
	}
	writeSn(s, FLPROG_SN_IR, FLPROG_SN_IR_SEND_OK);
	RT_HW_Base.spiEndTransaction(spiBus);
	return true;
}
