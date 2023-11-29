#pragma once
#include "flprogUtilites.h"

#ifdef ARDUINO_ARCH_ESP8266
#define FLPROG_CAN_USE_WIFI_SOKET
#include "ESP8266WiFi.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#define FLPROG_CAN_USE_WIFI_SOKET
#include "WiFi.h"
#include <esp_wifi.h>
#endif

#ifdef FLPROG_CAN_USE_WIFI_SOKET

#include <WiFiUdp.h>

#define FLPROG_WIFI_NOT_DEFINED_SOKET 0
#define FLPROG_WIFI_SERVER_SOKET 1
#define FLPROG_WIFI_CLIENT_SOKET 2
#define FLPROG_WIFI_UDP_SOKET 3

#define FLPROG_ON_BOARD_WIFI_MAX_SOCK_NUM 8

class FLProgWiFiServer : public WiFiServer
{
public:
    FLProgWiFiServer() : WiFiServer(0){};
};

class FLProgWifiSoket
{
public:
    FLProgWifiSoket(){};
    bool isUsed() { return _isUsed; };
    void disconnect();
    void close();
    void beServerTcp(uint16_t port);
    bool isListen();
    uint8_t connected();
    int available();
    int read();
    int read(uint8_t *buf, int16_t len);
    size_t write(const uint8_t *buffer, size_t len);
    uint8_t peek();

private:
    bool _isUsed = false;
    uint8_t _soketType = FLPROG_WIFI_NOT_DEFINED_SOKET;
    WiFiClient _client;
    FLProgWiFiServer _server;
    WiFiUDP _udp;
};

 #endif