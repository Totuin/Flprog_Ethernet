#pragma once
#include "flprogUtilites.h"
#include "../../abstract/flprogAbstractTcpInterface.h"

class FLProgAbstracttWiFiInterface : public FLProgAbstractTcpInterface
{
public:
    void setApSsid(String ssid);
    void setApPassword(String password);
    String apSsid() { return String(_apSsid); };
    void setClientSsidd(String ssid);
    void setClientPassword(String password);
    String clientSsid() { return String(_clientSsid); };

    void apMac(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5);
    uint8_t *apMac() { return _apMacaddress; };
    void apLocalIP(IPAddress ip);
    void apLocalIP(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { apLocalIP(IPAddress(ip0, ip1, ip2, ip3)); };
    IPAddress apLocalIP() { return _apIp; };
    void apDns(IPAddress ip);
    void apDns(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { apDns(IPAddress(ip0, ip1, ip2, ip3)); };
    IPAddress apDns() { return _apDnsIp; };
    void apSubnet(IPAddress ip);
    void apSubnet(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { apSubnet(IPAddress(ip0, ip1, ip2, ip3)); };
    IPAddress apSubnet() { return _apSubnetIp; };
    void apGateway(IPAddress ip);
    void apGateway(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3) { apGateway(IPAddress(ip0, ip1, ip2, ip3)); };
    IPAddress apGateway() { return _apGatewayIp; };

    virtual void clientOn();
    void clientOff();
    void clientMode(bool val);
    bool clientMode() { return _clientWorkStatus; };
    virtual bool clientIsReady() { return false; };
    virtual bool apIsReady() { return false; };
    virtual void apOn();
    void apOff();
    void apMode(bool val);
    bool apMode() { return _apWorkStatus; };
   

protected:
    uint8_t _apMacaddress[6] = {0, 0, 0, 0, 0, 0};
    char _apSsid[40] = "";
    char _apPassword[40] = "";
    char _clientSsid[40] = "";
    char _clientPassword[40] = "";

    IPAddress _apIp = FLPROG_INADDR_NONE;
    IPAddress _apDnsIp = FLPROG_INADDR_NONE;
    IPAddress _apSubnetIp = IPAddress(255, 255, 255, 0);
    IPAddress _apGatewayIp = FLPROG_INADDR_NONE;

    bool _apIsNeedReconect = false;
    bool _apWorkStatus = false;
    bool _clientWorkStatus = false;
    bool _clientStatus = false;
    bool _needUpdateClientData = false;
    bool _isCanStartServer = false;
};

#ifdef ARDUINO_ARCH_ESP8266
#define FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#define FLPROG_ESP8266_WIFI_ON_BOARD_TCP_DEVICE
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
extern "C"
{
#include "user_interface.h"
}
#include "variant/esp/esp8266/flprogEsp8266Wifi.h"
#endif

/*
#ifndef FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#ifdef ARDUINO_ARCH_ESP32
#define FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#define FLPROG_ESP32_WIFI_ON_BOARD_TCP_DEVICE
#include "WiFi.h"
#include <esp_wifi.h>
#include "variant/esp/esp32/flprogEsp32Wifi.h"
#endif
#endif

#ifndef FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#ifdef ARDUINO_ARCH_RP2040
#ifdef ARDUINO_RASPBERRY_PI_PICO_W
#define FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#define FLPROG_RP2040_WIFI_ON_BOARD_TCP_DEVICE
#include "variant/rp2040/flprogRP2040Wifi.h"
#include "WiFi.h"
#endif
#endif
#endif
*/
#ifndef FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#define FLPROG_WIFI_ON_BOARD_TCP_DEVICE
#define FLPROG_ANON_WIFI_ON_BOARD_TCP_DEVICE
#include "variant/anon/flprogAnonWifi.h"
#endif