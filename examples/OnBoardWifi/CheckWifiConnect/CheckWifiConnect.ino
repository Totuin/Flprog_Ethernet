#include <flprogEthernet.h> //подключаем библиотеку Ethernet

/*
  -------------------------------------------------------------------------------------------------
        Создание интерфейса для работы с чипом W5100(W5200,W5500)
        Шина SPI и пин CS берутся из  RT_HW_Base.device.spi.busETH и RT_HW_Base.device.spi.csETH
  -------------------------------------------------------------------------------------------------
*/
FLProgOnBoardWifiInterface WifiInterface;

/*
  -----------------------------------------------------------------------------------------
          Определение рабочих параметров и функций
  -----------------------------------------------------------------------------------------
*/
uint32_t blinkStartTime = 0;

uint8_t ethernetStatus = 255;
uint8_t ethernetError = 255;

bool isNeedClientSendConnectMessage = true;
bool isNeedClientSendDisconnectMessage = true;

bool isNeedApSendConnectMessage = true;
bool isNeedApSendDisconnectMessage = true;

//=================================================================================================
void setup()
{

    Serial.begin(115200);
    while (!Serial)
    {
    }
    Serial.print("Архитектура - ");
#if defined(RT_HW_ARCH_NAME)
    Serial.println(RT_HW_ARCH_NAME);
#else
    Serial.println("Архитектура не определенна!");
#endif

    Serial.print("Плата - ");
#if defined(RT_HW_BOARD_NAME)
    Serial.println(RT_HW_BOARD_NAME);
#else
    Serial.println("Плата не определенна!");
#endif

    flprog::printConsole();

    WifiInterface.clientOn();
    WifiInterface.apOn();
    WifiInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x28); //--Установка MAC-адрес контроллера (лучше адрес прошитый производителем);
    WifiInterface.setApSsid("Test-Esp-FLProg");
    WifiInterface.setApPassword("12345678");
    WifiInterface.setClientSsidd("totuin-router");
    WifiInterface.setClientPassword("12345678");

    pinMode(LED_BUILTIN, OUTPUT);
}
//=================================================================================================
void loop()
{
    WifiInterface.pool();
    printStatusMessages();
    blinkLed();
}

//=================================================================================================
void blinkLed()
{
    if (flprog::isTimer(blinkStartTime, 50))
    {
        blinkStartTime = millis();
        digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));
    }
}

void printStatusMessages()
{
    if (WifiInterface.getStatus() != ethernetStatus)
    {
        ethernetStatus = WifiInterface.getStatus();
        Serial.println();
        Serial.print("Статус интерфейса - ");
        Serial.println(flprog::flprogStatusCodeName(ethernetStatus));
    }
    if (WifiInterface.getError() != ethernetError)
    {
        ethernetError = WifiInterface.getError();
        if (ethernetError != FLPROG_NOT_ERROR)
        {
            Serial.println();
            Serial.print("Ошибка интерфейса - ");
            Serial.println(flprog::flprogErrorCodeName(ethernetError));
        }
    }
    printClientConnectMessages();
    printClientDisconnectMessages();
    printApConnectMessages();
    printApDisconnectMessages();
}

void printClientConnectMessages()
{
    if (!WifiInterface.clientIsReady())
    {
        return;
    }
    if (!isNeedClientSendConnectMessage)
    {
        return;
    }
    Serial.println("WiFiClient подключён!");
    Serial.print("Ssid - ");
    Serial.println(WifiInterface.clientSsid());
    Serial.print("Ip - ");
    Serial.println(WifiInterface.localIP());
    Serial.print("DNS - ");
    Serial.println(WifiInterface.dns());
    Serial.print("Subnet - ");
    Serial.println(WifiInterface.subnet());
    Serial.print("Gateway - ");
    Serial.println(WifiInterface.gateway());
    Serial.println();
    isNeedClientSendConnectMessage = false;
    isNeedClientSendDisconnectMessage = true;
}

void printApConnectMessages()
{
    if (!WifiInterface.apIsReady())
    {
        return;
    }
    if (!isNeedApSendConnectMessage)
    {
        return;
    }
    Serial.println("WiFi точка включенна!");
    Serial.print("Ssid - ");
    Serial.println(WifiInterface.apSsid());
    Serial.print("Ip - ");
    Serial.println(WifiInterface.apLocalIP());
    Serial.print("DNS - ");
    Serial.println(WifiInterface.apDns());
    Serial.print("Subnet - ");
    Serial.println(WifiInterface.apSubnet());
    Serial.print("Gateway - ");
    Serial.println(WifiInterface.apGateway());
    Serial.println();
    isNeedApSendConnectMessage = false;
    isNeedApSendDisconnectMessage = true;
}

void printClientDisconnectMessages()
{
    if (WifiInterface.clientIsReady())
    {
        return;
    }
    if (isNeedClientSendConnectMessage)
    {
        return;
    }
    if (!isNeedClientSendDisconnectMessage)
    {
        return;
    }
    Serial.println("WiFiClient отключён!");
    isNeedClientSendConnectMessage = true;
    isNeedClientSendDisconnectMessage = false;
}

void printApDisconnectMessages()
{
    if (WifiInterface.apIsReady())
    {
        return;
    }
    if (isNeedApSendConnectMessage)
    {
        return;
    }
    if (!isNeedApSendDisconnectMessage)
    {
        return;
    }
    Serial.println("WiFi точка отключёна!");
    isNeedApSendConnectMessage = true;
    isNeedApSendDisconnectMessage = false;
}