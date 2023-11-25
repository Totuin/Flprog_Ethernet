#include "flprogAbstactEthernetClasses.h"

size_t FLProgAbstactEthernetUDPChanel::write(const uint8_t *buffer, size_t size)
{
    uint16_t bytes_written = _sourse->bufferDataSoket(_sockindex, _offset, buffer, size);
    _offset += bytes_written;
    return bytes_written;
}


int FLProgAbstactEthernetUDPChanel::parsePacket()
{
    while (_remaining)
    {
        read((uint8_t *)NULL, _remaining);
    }

    if (_sourse->availableSoket(_sockindex) > 0)
    {
        uint8_t tmpBuf[8];
        int ret = 0;
        ret = _sourse->recvSoket(_sockindex, tmpBuf, 8);
        if (ret > 0)
        {
            _remoteIP = tmpBuf;
            _remotePort = tmpBuf[4];
            _remotePort = (_remotePort << 8) + tmpBuf[5];
            _remaining = tmpBuf[6];
            _remaining = (_remaining << 8) + tmpBuf[7];
            ret = _remaining;
        }
        return ret;
    }
    return 0;
}

int FLProgAbstactEthernetUDPChanel::read()
{
    uint8_t byte;
    if ((_remaining > 0) && (_sourse->recvSoket(_sockindex, &byte, 1) > 0))
    {
        _remaining--;
        return byte;
    }
    return -1;
}

int FLProgAbstactEthernetUDPChanel::read(uint8_t *buffer, size_t len)
{
    if (_remaining > 0)
    {
        int got;
        if (_remaining <= len)
        {
            got = _sourse->recvSoket(_sockindex, buffer, _remaining);
        }
        else
        {
            got = _sourse->recvSoket(_sockindex, buffer, len);
        }
        if (got > 0)
        {
            _remaining -= got;
            return got;
        }
    }
    return -1;
}

int FLProgAbstactEthernetUDPChanel::endPacket()
{
    _sourse->sendUdpSoket(_sockindex);
    return FLPROG_SUCCESS;
}

void FLProgAbstactEthernetUDPChanel::stop()
{
    _sourse->closeSoket(_sockindex);
    _sockindex = FLPROG_ETHERNET_MAX_SOCK_NUM;
}

int FLProgAbstactEthernetUDPChanel::beginPacket(IPAddress ip, uint16_t port)
{
    _offset = 0;
    _status = FLPROG_READY_STATUS;
    uint8_t buffer[4];
    flprog::ipToArray(ip, buffer);
    if (_sourse->startUdpSoket(_sockindex, buffer, port))
    {
        _errorCode = FLPROG_NOT_ERROR;
        return FLPROG_SUCCESS;
    }
    _errorCode = FLPROG_ETHERNET_UDP_SOKET_START_ERROR;
    return FLPROG_ERROR;
}

uint8_t FLProgAbstactEthernetUDPChanel::begin(uint16_t port)
{
	if (!_sourse->isReady())
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR;
		return FLPROG_ERROR;
	}
	if (_port == port)
	{
		if (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
		{
			_status = FLPROG_READY_STATUS;
			_errorCode = FLPROG_NOT_ERROR;
			return FLPROG_SUCCESS;
		}
	}
	_sourse->closeSoket(_sockindex);
	_sockindex = _sourse->getUDPSoket(port);
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_ETHERNET_SOKET_INDEX_ERROR;
		return FLPROG_SUCCESS;
	}
	_port = port;
	_remaining = 0;
	_status = FLPROG_READY_STATUS;
	return FLPROG_SUCCESS;
}

int FLProgAbstactEthernetUDPChanel::peek()
{
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM || _remaining == 0)
	{
		return -1;
	}
	return _sourse->peekSoket(_sockindex);
}
