#pragma once
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#endif

#include <ArduinoOTA.h>

class FLProgOTA
{
public:
    void setPassword(String password);
    String getPasssword() { return _password; };
    void setName(String name);
    String getName() { return _name; };

    void pool();

private:
    void init();
    String _password = "";
    String _name = "";
    bool _isInit = false;
};

#endif