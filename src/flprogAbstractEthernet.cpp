#include "flprogAbstractEthernet.h"

void FlprogAbstractEthernet::localIP(IPAddress _ip)
{
    if (ip == _ip)
    {
        return;
    }
    ip = _ip;
    isNeedReconect = true;
}

void FlprogAbstractEthernet::dns(IPAddress _ip)
{
    if (dnsIp == _ip)
    {
        return;
    }
    dnsIp = _ip;
    isNeedReconect = true;
}

void FlprogAbstractEthernet::subnet(IPAddress _ip)
{
    if (subnetIp == _ip)
    {
        return;
    }
    subnetIp = _ip;
    isNeedReconect = true;
}

void FlprogAbstractEthernet::gateway(IPAddress _ip)
{
    if (gatewayIp == _ip)
    {
        return;
    }
    gatewayIp = _ip;
    isNeedReconect = true;
}

void FlprogAbstractEthernet::mac(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5)
{
    if (flprog::applyMac(m0, m1, m2, m3, m4, m5, macAddress))
    {
        isNeedReconect = true;
    }
}

void FlprogAbstractEthernet::mac(uint8_t *mac_address)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        mac_address[i] = macAddress[i];
    }
}

void FlprogAbstractEthernet::setDhcp()
{
    if (isDhcp)
    {
        return;
    }
    isDhcp = true;
    isNeedReconect = true;
}

void FlprogAbstractEthernet::resetDhcp()
{
    if (!isDhcp)
    {
        return;
    }
    isDhcp = false;
    isNeedReconect = true;
}

void FlprogAbstractEthernet::dhcpMode(bool val)
{
    if (isDhcp == val)
    {
        return;
    }
    isDhcp = val;
    isNeedReconect = true;
}