#include "flprogAbstactEthernetChanel.h"

void FLProgAbstactEthernetChanel::stop()
{
    _sourse->closeSoket(_sockindex);
    _sockindex = _sourse->maxSoketNum();
    _status = FLPROG_NOT_REDY_STATUS;
}

uint8_t FLProgAbstactEthernetChanel::checkReadySourse()
{
    if (_sourse->isReady())
    {
        return FLPROG_SUCCESS;
    }
    _errorCode = FLPROG_ETHERNET_INTERFACE_NOT_READY_ERROR;
    _status = FLPROG_NOT_REDY_STATUS;
    return FLPROG_ERROR;
}
