#include "flprogAnonWifi.h"

#ifdef FLPROG_ANON_WIFI_ON_BOARD_TCP_DEVICE

int FLProgOnBoardWifiInterface::parsePacketSocet(uint8_t soket)
{
    (void)soket;
    return 0;
}
uint8_t FLProgOnBoardWifiInterface::resetToVoidVar(uint8_t soket)
{
    (void)soket;
    return 0;
}

int FLProgOnBoardWifiInterface::readFromSoket(uint8_t soket, uint8_t *buf, int16_t len)
{
    (void)soket;
    (void)buf;
    (void)len;
    return -1;
}

size_t FLProgOnBoardWifiInterface::writeToSoket(uint8_t soket, const uint8_t *buffer, size_t size)
{
    (void)soket;
    (void)buffer;
    (void)size;
    return 0;
}

uint8_t FLProgOnBoardWifiInterface::startUdpSoket(uint8_t soket, uint8_t *addr, uint16_t port)
{
    (void)soket;
    (void)addr;
    (void)port;
    return 0;
}

uint16_t FLProgOnBoardWifiInterface::bufferDataSoket(uint8_t soket, uint16_t offset, const uint8_t *buf, uint16_t len)
{
    (void)soket;
    (void)offset;
    (void)buf;
    (void)len;
    return 0;
}

int FLProgOnBoardWifiInterface::recvSoket(uint8_t soket, uint8_t *buf, int16_t len)
{
    (void)soket;
    (void)buf;
    (void)len;
    return -1;
}

uint8_t FLProgOnBoardWifiInterface::beginMulticastSoket(IPAddress ip, uint16_t port)
{
    (void)port;
    (void)ip;
    return 0;
}

uint8_t FLProgOnBoardWifiInterface::connectSoket(uint8_t soket, IPAddress ip, uint16_t port)
{
    (void)soket;
    (void)port;
    (void)ip;
    return 0;
}

IPAddress FLProgOnBoardWifiInterface::remoteIPSoket(uint8_t soket)
{
    (void)soket;
    return FLPROG_INADDR_NONE;
}
#endif