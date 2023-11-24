#include "flprogEthernetServer.h"

FLProgEthernetServer::FLProgEthernetServer(FlprogAbstractEthernet *sourse, uint16_t port)
{
	_sourse = sourse;
	_port = port;
}

uint8_t FLProgEthernetServer::setPort(uint16_t port)
{
	if (_port == port)
	{
		return;
	}
	_port = port;
	_status = FLPROG_NOT_REDY_STATUS;
	return FLPROG_SUCCESS;
}

uint8_t FLProgEthernetServer::pool()
{
	if (_callbackFunction == 0)
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_SERVER_NOT_CALLBACK_ERROR;
		return FLPROG_ERROR;
	}
	if (!_sourse->isReady())
	{
		_status = FLPROG_NOT_REDY_STATUS;
		return FLPROG_SUCCESS;
	}
	if (_status == FLPROG_NOT_REDY_STATUS)
	{
		return begin();
	}
	if (!available())
	{
		return FLPROG_SUCCESS;
	}
	_callbackFunction();
	return FLPROG_SUCCESS;
}

uint8_t FLProgEthernetServer::begin()
{
	if (_sockindex < FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_sourse->hardware()->socketDisconnect(_sockindex);
	}
	_sockindex = _sourse->hardware()->socketBegin(FLPROG_SN_MR_TCP, _port);
	if (_sockindex >= FLPROG_ETHERNET_MAX_SOCK_NUM)
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_SOKET_INDEX_ERROR;
		return FLPROG_ERROR;
	}
	if (_sourse->hardware()->socketListen(_sockindex))
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_NOT_ERROR;
		return FLPROG_SUCCESS;
	}
	_sourse->hardware()->socketDisconnect(_sockindex);
	_status = FLPROG_NOT_REDY_STATUS;
	_errorCode = FLPROG_ETHERNET_SOKET_NOT_INIT_ERROR;
	return FLPROG_ERROR;
}

size_t FLProgEthernetServer::write(uint8_t b)
{
	return write(&b, 1);
}
size_t FLProgEthernetServer::write(const uint8_t *buffer, size_t size)
{
	if (_sourse->hardware()->socketStatus(_sockindex) == FLPROG_SN_SR_ESTABLISHED)
	{
		_sourse->hardware()->socketSend(_sockindex, buffer, size);
	}
	return size;
}

int FLProgEthernetServer::available()
{
	return _sourse->hardware()->socketRecvAvailable(_sockindex);
}

int FLProgEthernetServer::read()
{
	uint8_t b;
	if (_sourse->hardware()->socketRecv(_sockindex, &b, 1) > 0)
	{
		return b;
	}
	return -1;
}

void FLProgEthernetServer::println(String s)
{
	print(s);
	print("\n");
}