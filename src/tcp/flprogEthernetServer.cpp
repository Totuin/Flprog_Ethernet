#include "flprogEthernetServer.h"

FLProgEthernetServer::FLProgEthernetServer(FLProgAbstractTcpInterface *sourse, uint16_t port)
{
	_sourse = sourse;
	_port = port;
}

uint8_t FLProgEthernetServer::setPort(uint16_t port)
{
	if (_port == port)
	{
		return FLPROG_SUCCESS;
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
	if (!connected())
	{
		return FLPROG_SUCCESS;
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
	if (checkReadySourse() == FLPROG_ERROR)
	{
		return FLPROG_ERROR;
	}
	_sourse->disconnecSoket(_sockindex);
	_sockindex = _sourse->getServerTCPSoket(_port);
	if (_sockindex >= _sourse->maxSoketNum())
	{
		_status = FLPROG_NOT_REDY_STATUS;
		_errorCode = FLPROG_ETHERNET_SOKET_INDEX_ERROR;
		return FLPROG_ERROR;
	}
	if (_sourse->isListenSoket(_sockindex))
	{
		_status = FLPROG_READY_STATUS;
		_errorCode = FLPROG_NOT_ERROR;
		return FLPROG_SUCCESS;
	}
	_sourse->disconnecSoket(_sockindex);
	_status = FLPROG_NOT_REDY_STATUS;
	_errorCode = FLPROG_ETHERNET_SOKET_NOT_INIT_ERROR;
	return FLPROG_ERROR;
}

uint8_t FLProgEthernetServer::connected()
{
	if (checkReadySourse() == FLPROG_ERROR)
	{
		return 0;
	}
	if (_status == FLPROG_NOT_REDY_STATUS)
	{
		return begin();
	}
	return _sourse->soketConnected(_sockindex);
}

void FLProgEthernetServer::stopConnection()
{
	begin();
}
