#pragma once
#include <Arduino.h>

#if !(defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266))

class FLProgOTA
{
public:
    void setPassword(String password) { (void)password; };
    String getPasssword() { return ""; };
    void setName(String name) { (void)name; };
    String getName() { return ""; };
    void pool() {};
};

#endif