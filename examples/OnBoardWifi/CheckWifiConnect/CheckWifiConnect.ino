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

bool isNeedSendConnectMessage = true;
bool isNeedSendDisconnectMessage = true;

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
  WifiInterface.clientOn();
  WifiInterface.setDhcp();
  WifiInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x28); //--Установка MAC-адрес контроллера (лучше адрес прошитый производителем);
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
  printConnectMessages();
  printDisconnectMessages();
}

void printConnectMessages()
{
  if (!WifiInterface.isReady())
  {
    return;
  }
  if (!isNeedSendConnectMessage)
  {
    return;
  }
  Serial.println("Ethernet подключён!");
  Serial.print("Ip - ");
  Serial.println(WifiInterface.localIP());
  Serial.print("DNS - ");
  Serial.println(WifiInterface.dns());
  Serial.print("Subnet - ");
  Serial.println(WifiInterface.subnet());
  Serial.print("Gateway - ");
  Serial.println(WifiInterface.gateway());
  isNeedSendConnectMessage = false;
  isNeedSendDisconnectMessage = true;
}

void printDisconnectMessages()
{
  if (WifiInterface.isReady())
  {
    return;
  }
  if (isNeedSendConnectMessage)
  {
    return;
  }
  if (!isNeedSendDisconnectMessage)
  {
    return;
  }
  Serial.println("Ethernet отключён!");
  isNeedSendConnectMessage = true;
  isNeedSendDisconnectMessage = false;
}