
#pragma once
#include <Arduino.h>
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

#ifndef FLPROG_ETHERNET_MAX_SOCK_NUM
#if defined(RAMEND) && defined(RAMSTART) && ((RAMEND - RAMSTART) <= 2048)
#define FLPROG_ETHERNET_MAX_SOCK_NUM 4
#else
#define FLPROG_ETHERNET_MAX_SOCK_NUM 8
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

class FLProgAbstractEthernetHardware
{
public:
  virtual uint8_t init() = 0;
  virtual uint8_t getLinkStatus() = 0;
  virtual void setGatewayIp(IPAddress addr) = 0;
  virtual IPAddress getGatewayIp() = 0;
  virtual void setSubnetMask(IPAddress addr);
  virtual IPAddress getSubnetMask() = 0;
  virtual void setMACAddress(const uint8_t *addr) = 0;
  virtual void getMACAddress(uint8_t *addr) = 0;
  virtual void setIPAddress(IPAddress addr) = 0;
  virtual IPAddress getIPAddress() = 0;
  virtual void setRetransmissionTime(uint16_t timeout) = 0;
  virtual void setRetransmissionCount(uint8_t retry) = 0;
  virtual void execCmdSn(SOCKET s, uint8_t _cmd) = 0;
  virtual uint8_t getChip() = 0;
  virtual uint16_t _CH_SIZE() = 0;
  virtual uint16_t _SSIZE() = 0;

  // утилиты
  virtual void setNetSettings(uint8_t *mac, IPAddress ip) = 0;
  virtual void setNetSettings(IPAddress ip, IPAddress gateway, IPAddress subnet) = 0;
  virtual void setNetSettings(uint8_t *mac, IPAddress ip, IPAddress gateway, IPAddress subnet) = 0;
  virtual void setOnlyMACAddress(const uint8_t *mac_address) = 0;
  virtual void setOnlyLocalIP(const IPAddress local_ip) = 0;
  virtual void setOnlySubnetMask(const IPAddress subnet) = 0;
  virtual void setOnlyGatewayIP(const IPAddress gateway) = 0;

  virtual IPAddress localIP() = 0;
  virtual IPAddress subnetMask() = 0;
  virtual IPAddress gatewayIP() = 0;
  virtual void MACAddress(uint8_t *mac_address) = 0;
  virtual uint16_t localPort(uint8_t soc) = 0;
  virtual IPAddress remoteIP(uint8_t soc) = 0;
  virtual uint16_t remotePort(uint8_t soc) = 0;

  // Сокет
  virtual void socketPortRand(uint16_t n) = 0;
  virtual uint8_t socketBegin(uint8_t protocol, uint16_t port) = 0;
  virtual uint8_t socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port) = 0;
  virtual uint8_t socketStatus(uint8_t s) = 0;
  virtual void socketClose(uint8_t s) = 0;
  virtual uint8_t socketListen(uint8_t s) = 0;
  virtual void socketConnect(uint8_t s, IPAddress ip, uint16_t port) = 0;
  virtual void socketDisconnect(uint8_t s) = 0;
  virtual uint16_t getSnRX_RSR(uint8_t s) = 0;
  virtual void read_data(uint8_t s, uint16_t src, uint8_t *dst, uint16_t len) = 0;
  virtual int socketRecv(uint8_t s, uint8_t *buf, int16_t len) = 0;
  virtual uint16_t socketRecvAvailable(uint8_t s) = 0;
  virtual uint8_t socketPeek(uint8_t s) = 0;
  virtual uint16_t getSnTX_FSR(uint8_t s) = 0;
  virtual void write_data(uint8_t s, uint16_t data_offset, const uint8_t *data, uint16_t len) = 0;
  virtual uint16_t socketSend(uint8_t s, const uint8_t *buf, uint16_t len) = 0;
  virtual uint16_t socketSendAvailable(uint8_t s) = 0;
  virtual uint16_t socketBufferData(uint8_t s, uint16_t offset, const uint8_t *buf, uint16_t len) = 0;
  virtual bool socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port) = 0;
  virtual bool socketSendUDP(uint8_t s) = 0;
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

#include "flprogW5100.h"