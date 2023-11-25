#pragma once
// #include "flprogSPI.h"
#include "flprogAbstractEthernet.h"
#include "flprogAbstractEthernetHardware.h"

class FLProgWiznetClass : public FLProgAbstractEthernetHardware
{
public:
  virtual uint8_t init();
  uint8_t checkInit();
  void setPinCs(int pinCs);
  virtual uint8_t getLinkStatus();

  uint8_t SoketConnected(uint8_t soket);
  int readFromSoket(uint8_t soket);
  uint8_t readFromSoket(uint8_t soket, uint8_t *buf, int16_t len);
  size_t writeToSoket(const uint8_t *buffer, size_t size, uint8_t soket);
  uint8_t isConnectStatusSoket(uint8_t soket);
  uint8_t isCosedStatusSoket(uint8_t soket);

  virtual void setSpiBus(uint8_t bus) { _spiBus = bus; };
  virtual void setGatewayIp(IPAddress addr);
  virtual IPAddress getGatewayIp();
  virtual void setSubnetMask(IPAddress addr);
  virtual IPAddress getSubnetMask();
  virtual void setMACAddress(const uint8_t *addr) { write(FLPROG_SHAR, addr, 6); };
  virtual void getMACAddress(uint8_t *addr) { read(FLPROG_SHAR, addr, 6); };
  virtual void setIPAddress(IPAddress addr);
  virtual IPAddress getIPAddress();
  virtual void setRetransmissionTime(uint16_t timeout);
  virtual void setRetransmissionCount(uint8_t retry);
  virtual void execCmdSn(SOCKET s, uint8_t _cmd);
  virtual uint8_t getChip(void) { return _chip; }
  virtual uint16_t _CH_SIZE() { return CH_SIZE; };
  virtual uint16_t _SSIZE() { return SSIZE; };

  // W5100 Registers
  virtual uint16_t write(uint16_t addr, const uint8_t *buf, uint16_t len);
  virtual uint8_t write(uint16_t addr, uint8_t data) { return write(addr, &data, 1); };
  virtual void write16(uint16_t address, uint16_t _data);
  virtual uint16_t read(uint16_t addr, uint8_t *buf, uint16_t len);
  virtual uint8_t read(uint16_t addr);
  virtual uint16_t CH_BASE() { return _CH_BASE_MSB << 8; };
  uint8_t _CH_BASE_MSB; // 1 redundant byte, saves ~80 bytes code on AVR

  // W5100 Socket registers
  virtual uint8_t readSn(SOCKET s, uint16_t addr) { return read(CH_BASE() + s * CH_SIZE + addr); };
  virtual uint8_t writeSn(SOCKET s, uint16_t addr, uint8_t data) { return write(CH_BASE() + s * CH_SIZE + addr, data); };
  virtual uint16_t readSn(SOCKET s, uint16_t addr, uint8_t *buf, uint16_t len) { return read(CH_BASE() + s * CH_SIZE + addr, buf, len); };
  virtual uint16_t writeSn(SOCKET s, uint16_t addr, uint8_t *buf, uint16_t len) { return write(CH_BASE() + s * CH_SIZE + addr, buf, len); };
  virtual uint16_t readSn16(SOCKET _s, uint16_t address);
  virtual void writeSn16(SOCKET _s, uint16_t address, uint16_t _data);

#ifdef ETHERNET_LARGE_BUFFERS
  uint16_t SSIZE;
  uint16_t SMASK;
#else
  const uint16_t SSIZE = 2048;
  const uint16_t SMASK = 0x07FF;
#endif
  virtual uint16_t SBASE(uint8_t socknum);
  virtual uint16_t RBASE(uint8_t socknum);
  virtual bool hasOffsetAddressMapping(void);

  // утилиты
  virtual void setNetSettings(uint8_t *mac, IPAddress ip);
  virtual void setNetSettings(IPAddress ip, IPAddress gateway, IPAddress subnet);
  virtual void setNetSettings(uint8_t *mac, IPAddress ip, IPAddress gateway, IPAddress subnet);
  virtual void setOnlyMACAddress(const uint8_t *mac_address);
  virtual void setOnlyLocalIP(const IPAddress local_ip);
  virtual void setOnlySubnetMask(const IPAddress subnet);
  virtual void setOnlyGatewayIP(const IPAddress gateway);

  virtual IPAddress localIP();
  virtual IPAddress subnetMask();
  virtual IPAddress gatewayIP();
  virtual void MACAddress(uint8_t *mac_address);
  virtual uint16_t localPort(uint8_t soc);
  virtual IPAddress remoteIP(uint8_t soc);
  virtual uint16_t remotePort(uint8_t soc);

  // Сокет
  virtual void socketPortRand(uint16_t n);
  virtual uint8_t socketBegin(uint8_t protocol, uint16_t port);
  virtual uint8_t socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port);
  virtual uint8_t socketStatus(uint8_t s);
  virtual void socketClose(uint8_t s);
  virtual uint8_t socketListen(uint8_t s);
  virtual uint8_t socketConnect(uint8_t s, IPAddress ip, uint16_t port);
  virtual uint8_t socketDisconnect(uint8_t s);
  virtual uint16_t getSnRX_RSR(uint8_t s);
  virtual void read_data(uint8_t s, uint16_t src, uint8_t *dst, uint16_t len);
  virtual int socketRecv(uint8_t s, uint8_t *buf, int16_t len);
  virtual uint16_t socketRecvAvailable(uint8_t s);
  virtual uint8_t socketPeek(uint8_t s);
  virtual uint16_t getSnTX_FSR(uint8_t s);
  virtual void write_data(uint8_t s, uint16_t data_offset, const uint8_t *data, uint16_t len);
  virtual uint16_t socketSend(uint8_t s, const uint8_t *buf, uint16_t len);
  virtual uint16_t socketSendAvailable(uint8_t s);
  virtual uint16_t socketBufferData(uint8_t s, uint16_t offset, const uint8_t *buf, uint16_t len);
  virtual uint8_t socketStartUDP(uint8_t s, uint8_t *addr, uint16_t port);
  virtual uint8_t socketSendUDP(uint8_t s);
  virtual bool isInit() { return _status == FLPROG_READY_STATUS; };
  int pinCs();
  uint8_t spiBus();

private:
  uint8_t _chip = 0;
  uint32_t _startWhiteInitTime;

  uint16_t _local_port = 49152; // 49152 to 65535 TODO: randomize this when not using DHCP, but how?
  const uint16_t CH_SIZE = 0x0100;
  socketstate_t _state[FLPROG_ETHERNET_MAX_SOCK_NUM];
  uint8_t _spiBus = 255;
  int _pinCs = -1;
  uint8_t softReset(void);
  uint8_t isW5100(void);
  uint8_t isW5200(void);
  uint8_t isW5500(void);
  void initCs() { pinMode(pinCs(), OUTPUT); };
  void setCs() { digitalWrite(pinCs(), LOW); };
  void resetCs() { digitalWrite(pinCs(), HIGH); };
  void privateMaceSoket(uint8_t soc, uint8_t protocol, uint16_t port);
  void privateMaceSoketMulticast(uint8_t soc, uint8_t protocol, IPAddress ip, uint16_t port);
  void beginTransaction() { RT_HW_Base.spiBeginTransaction(SPI_ETHERNET_SPEED, 1, 0, _spiBus); };
  void endTransaction() { RT_HW_Base.spiEndTransaction(_spiBus); };
};
