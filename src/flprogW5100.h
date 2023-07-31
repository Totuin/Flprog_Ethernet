
#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <IPAddress.h>

#if ARDUINO >= 156 && !defined(ARDUINO_ARCH_PIC32)
extern void yield(void);
#else
#define yield()
#endif

// Safe for all chips
#define SPI_ETHERNET_SETTINGS SPISettings(14000000, MSBFIRST, SPI_MODE0)

// Safe for W5200 and W5500, but too fast for W5100
// Uncomment this if you know you'll never need W5100 support.
//  Higher SPI clock only results in faster transfer to hosts on a LAN
//  or with very low packet latency.  With ordinary internet latency,
//  the TCP window size & packet loss determine your overall speed.
// #define SPI_ETHERNET_SETTINGS SPISettings(30000000, MSBFIRST, SPI_MODE0)

// Arduino 101's SPI can not run faster than 8 MHz.
#if defined(ARDUINO_ARCH_ARC32)
#undef SPI_ETHERNET_SETTINGS
#define SPI_ETHERNET_SETTINGS SPISettings(8000000, MSBFIRST, SPI_MODE0)
#endif

// Arduino Zero can't use W5100-based shields faster than 8 MHz
// https://github.com/arduino-libraries/Ethernet/issues/37#issuecomment-408036848
// W5500 does seem to work at 12 MHz.  Delete this if only using W5500
#if defined(__SAMD21G18A__)
#undef SPI_ETHERNET_SETTINGS
#define SPI_ETHERNET_SETTINGS SPISettings(8000000, MSBFIRST, SPI_MODE0)
#endif

#ifndef MAX_SOCK_NUM
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define MAX_SOCK_NUM 4
#else
#define MAX_SOCK_NUM 8
#endif
#endif

typedef uint8_t SOCKET;

#define FLPROG_SN_MR_CLOSE 0x00
#define FLPROG_SN_MR_TCP 0x21
#define FLPROG_SN_MR_UDP 0x02
#define FLPROG_SN_MR_MULTI 0x80

#define FLPROG_SOCK_CMD_OPEN 0x01
#define FLPROG_SOCK_CMD_CLOSE 0x10
#define FLPROG_SOCK_CMD_LISTEN 0x02
#define FLPROG_SOCK_CMD_CONNECT 0x04
#define FLPROG_SOCK_CMD_DISCON 0x08
#define FLPROG_SOCK_CMD_RECV 0x40
#define FLPROG_SOCK_CMD_SEND 0x20

#define FLPROG_SN_IR_SEND_OK 0x10
#define FLPROG_SN_IR_TIMEOUT 0x08

#define FLPROG_SN_SR_ESTABLISHED 0x17
#define FLPROG_SN_SR_CLOSE_WAIT 0x1C
#define FLPROG_SN_SR_LISTEN 0x14
#define FLPROG_SN_SR_CLOSED 0x00
#define FLPROG_SN_SR_FIN_WAIT 0x18
#define FLPROG_SN_SR_LAST_ACK 0x1D
#define FLPROG_SN_SR_TIME_WAIT 0x1B
#define FLPROG_SN_SR_FIN_WAIT 0x18
#define FLPROG_SN_SR_CLOSING 0x1A
#define FLPROG_SN_SR_INIT 0x13

#define FLPROG_GAR 0x0001            // Gateway IP address
#define FLPROG_SUBR 0x0005           // Subnet mask address
#define FLPROG_SHAR 0x0009           // Source MAC address
#define FLPROG_SIPR 0x000F           // Source IP address
#define FLPROG_RTR 0x0017            // Timeout address
#define FLPROG_SHAR 0x0009           // Source MAC address
#define FLPROG_RCR 0x0019            // Retry count
#define FLPROG_RMSR 0x001A           // Receive memory size (W5100 only)
#define FLPROG_TMSR 0x001B           // Transmit memory size (W5100 only)
#define FLPROG_MR 0x0000             // Mode
#define FLPROG_VERSIONR_W5200 0x001F // Chip Version Register (W5200 only)
#define FLPROG_VERSIONR_W5500 0x0039 // Chip Version Register (W5500 only)
#define FLPROG_PSTATUS_W5200 0x0035  // PHY Status
#define FLPROG_PHYCFGR_W5500 0x002E  // PHY Configuration register, default: 10111xxx

#define FLPROG_SN_CR 0x0001      // Command
#define FLPROG_SN_SR 0x0003      // Status
#define FLPROG_SN_MR 0x0000      // Mode
#define FLPROG_SN_IR 0x0002      // Interrupt
#define FLPROG_SN_PORT 0x0004    // Source Port
#define FLPROG_SN_RX_RD 0x0028   // RX Read Pointer
#define FLPROG_SN_RX_SIZE 0x001E // RX Memory Size (W5200 only)
#define FLPROG_SN_RX_RSR 0x0026  // RX Free Size
#define FLPROG_SN_TX_SIZE 0x001F // RX Memory Size (W5200 only)
#define FLPROG_SN_TX_FSR 0x0020  // TX Free Size
#define FLPROG_SN_TX_WR 0x0024   // TX Write Pointer
#define FLPROG_SN_DIPR 0x000C    // Destination IP Addr
#define FLPROG_SN_DPORT 0x0010   // Destination Port
#define FLPROG_SN_DHAR 0x0006    // Destination Hardw Addr

#define SS_PIN_DEFAULT 10

typedef struct
{
  uint16_t RX_RSR; // Number of bytes received
  uint16_t RX_RD;  // Address to read
  uint16_t TX_FSR; // Free space ready for transmit
  uint8_t RX_inc;  // how much have we advanced RX_RD
} socketstate_t;

class FlprogW5100Class
{
public:
  uint8_t init(void);
  uint8_t getLinkStatus();
  void setGatewayIp(IPAddress addr);
  IPAddress getGatewayIp();
  void setSubnetMask(IPAddress addr);
  IPAddress getSubnetMask();
  void setMACAddress(const uint8_t *addr) { write(FLPROG_SHAR, addr, 6); };
  void getMACAddress(uint8_t *addr) { read(FLPROG_SHAR, addr, 6); };
  void setIPAddress(IPAddress addr);
  IPAddress getIPAddress();
  void setRetransmissionTime(uint16_t timeout);
  void setRetransmissionCount(uint8_t retry);
  void execCmdSn(SOCKET s, uint8_t _cmd);
  uint8_t getChip(void) { return chip; }

  // W5100 Registers
  uint16_t write(uint16_t addr, const uint8_t *buf, uint16_t len);
  uint8_t write(uint16_t addr, uint8_t data) { return write(addr, &data, 1); };
  void write16(uint16_t address, uint16_t _data);
  uint16_t read(uint16_t addr, uint8_t *buf, uint16_t len);
  uint8_t read(uint16_t addr);
  uint16_t CH_BASE(void) { return CH_BASE_MSB << 8; };
  uint8_t CH_BASE_MSB; // 1 redundant byte, saves ~80 bytes code on AVR
  const uint16_t CH_SIZE = 0x0100;

  // W5100 Socket registers
  uint8_t readSn(SOCKET s, uint16_t addr) { return read(CH_BASE() + s * CH_SIZE + addr); };
  uint8_t writeSn(SOCKET s, uint16_t addr, uint8_t data) { return write(CH_BASE() + s * CH_SIZE + addr, data); };
  uint16_t readSn(SOCKET s, uint16_t addr, uint8_t *buf, uint16_t len) { return read(CH_BASE() + s * CH_SIZE + addr, buf, len); };
  uint16_t writeSn(SOCKET s, uint16_t addr, uint8_t *buf, uint16_t len) { return write(CH_BASE() + s * CH_SIZE + addr, buf, len); };
  uint16_t readSn16(SOCKET _s, uint16_t address);
  void writeSn16(SOCKET _s, uint16_t address, uint16_t _data);
  uint8_t _pinSS;

#ifdef ETHERNET_LARGE_BUFFERS
  uint16_t SSIZE;
  uint16_t SMASK;
#else
  const uint16_t SSIZE = 2048;
  const uint16_t SMASK = 0x07FF;
#endif
  uint16_t SBASE(uint8_t socknum);
  uint16_t RBASE(uint8_t socknum);
  bool hasOffsetAddressMapping(void);
  void setSS(uint8_t pin) { _pinSS = pin; };

  // утилиты
  void setNetSettings(uint8_t *mac, IPAddress ip);
  void setNetSettings(IPAddress ip, IPAddress gateway, IPAddress subnet);
  void setNetSettings(uint8_t *mac, IPAddress ip, IPAddress gateway, IPAddress subnet);
  void setOnlyMACAddress(const uint8_t *mac_address);
  void setOnlyLocalIP(const IPAddress local_ip);
  void setOnlySubnetMask(const IPAddress subnet);
  void setOnlyGatewayIP(const IPAddress gateway);

  IPAddress localIP();
  IPAddress subnetMask();
  IPAddress gatewayIP();
  void MACAddress(uint8_t *mac_address);

  // Сокет
  void socketPortRand(uint16_t n);
  uint8_t socketBegin(uint8_t protocol, uint16_t port);
  uint8_t socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port);
  uint8_t socketStatus(uint8_t s);
  void socketClose(uint8_t s);
  uint8_t socketListen(uint8_t s);
  void socketConnect(uint8_t s, uint8_t *addr, uint16_t port);
  void socketDisconnect(uint8_t s);
  uint16_t getSnRX_RSR(uint8_t s);
  void read_data(uint8_t s, uint16_t src, uint8_t *dst, uint16_t len);
  int socketRecv(uint8_t s, uint8_t *buf, int16_t len);
  uint16_t socketRecvAvailable(uint8_t s);
  uint8_t socketPeek(uint8_t s);
  uint16_t getSnTX_FSR(uint8_t s);
  void write_data(uint8_t s, uint16_t data_offset, const uint8_t *data, uint16_t len);
  uint16_t socketSend(uint8_t s, const uint8_t *buf, uint16_t len);
  uint16_t socketSendAvailable(uint8_t s);
  uint16_t socketBufferData(uint8_t s, uint16_t offset, const uint8_t *buf, uint16_t len);
  bool socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port);
  bool socketSendUDP(uint8_t s);

private:
  uint8_t chip = 0;
  uint16_t local_port = 49152; // 49152 to 65535 TODO: randomize this when not using DHCP, but how?
  bool initialized = false;
  socketstate_t state[MAX_SOCK_NUM];
  uint8_t softReset(void);
  uint8_t isW5100(void);
  uint8_t isW5200(void);
  uint8_t isW5500(void);
  void initSS() { pinMode(_pinSS, OUTPUT); };
  void setSS() { digitalWrite(_pinSS, LOW); };
  void resetSS() { digitalWrite(_pinSS, HIGH); };
  void privateMaceSocet(uint8_t soc, uint8_t protocol, uint16_t port);
  void privateMaceSocetMulticast(uint8_t soc, uint8_t protocol, IPAddress ip, uint16_t port);
};

#ifndef FLPROG_W5100_UTIL_H
#define FLPROG_W5100_UTIL_H

#define flprogW5100Htons(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0xFF))
#define flprogW5100Ntohs(x) flprogW5100Htons(x)

#define flprogW5100Htonl(x) (((x) << 24 & 0xFF000000UL) | \
                             ((x) << 8 & 0x00FF0000UL) |  \
                             ((x) >> 8 & 0x0000FF00UL) |  \
                             ((x) >> 24 & 0x000000FFUL))
#define flporgW5100Ntohl(x) flprogW5100Htonl(x)
#endif