
#include "flprogEthernet.h"

//=================================================================================================
//  Определяем целевую платформу
//=================================================================================================

#if defined(RT_HW_BOARD_NAME)
String boarbName = RT_HW_BOARD_NAME;
#else
String boarbName = "Not Defined";
#endif

#if defined(RT_HW_ARCH_NAME)
String archName = RT_HW_ARCH_NAME;
#else
String archName = "Not Defined";
#endif

#ifdef ARDUINO_ARCH_RP2040
#define CS_PIN 21
#define SPI_BUS 0
#elif ARDUINO_ARCH_STM32
#define CS_PIN PC2
#define SPI_BUS 0
#else
#define CS_PIN 10
#define SPI_BUS 0
#endif

//-------------------------------------------------------------------------------------------------
//         Вариант с  шиной (SPI0) и пином(10) по умолчаниюю. Пин потом можно поменять.
//         Но если на этой шине висит ещё какое то устройство лучше применять второй вариант
//-------------------------------------------------------------------------------------------------
// FLProgWiznetInterface WiznetInterface; //--Создание интерфейса для работы с чипом W5100(W5200,W5500) (по умолчанию CS pin - 10,  Шина SPI - 0);
//-------------------------------------------------------------------------------------------------
//        Второй вариант с непосредственной привязкой к шине и пину.
//-------------------------------------------------------------------------------------------------
FLProgWiznetInterface WiznetInterface(CS_PIN, SPI_BUS); //--Создание интерфейса для работы с чипом W5100(W5200,W5500)
// FLProgWiznetInterface WiznetInterface(CS_PIN); //--Создание интерфейса для работы с чипом W5100(W5200,W5500)

//-------------------------------------------------------------------------------------------------
//         Задание параметров интернет соеденения и параметров клиента
//-------------------------------------------------------------------------------------------------
const char *host = "djxmmx.net";
// const char *host = "flprog1.ru"; // Несуществующий домен для проверки DNS
// IPAddress  host = IPAddress(104, 230, 16, 86);
const uint16_t port = 17;

//-------------------------------------------------------------------------------------------------
//          1.2.Создание объекта клиента  с привязкой к интерфейсу
//-------------------------------------------------------------------------------------------------

FLProgEthernetClient client(&WiznetInterface);

//-----------------------------------------------------------------------------------------
//          1.3.Определение рабочих параметров и функций
//-----------------------------------------------------------------------------------------

uint8_t ethernetStatus = 255;
uint8_t ethernetError = 255;

uint8_t clientStatus = 255;
uint8_t clientError = 255;

uint32_t blinkStartTime = 0;

uint32_t reqestPeriod = 60000;
uint32_t startSendReqest = flprog::timeBack(reqestPeriod);

uint32_t reqestTimeout = 10000;

bool isWaitReqest = false;

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
  Serial.println(archName);
  Serial.print("Плата - ");
  Serial.println(boarbName);

  WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40); //--Установка MAC-адрес контроллера
  // WiznetInterface.localIP(192, 168, 199, 155);
  // WiznetInterface.resetDhcp();

  client.setDnsCacheStorageTime(600000); // Устанавливаем клиенту время хранаения DNS кэша 10 минут

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}
void loop()
{
  WiznetInterface.pool();
  printStatusMessages();
  blinkLed();
  sendReqest();
  ressiveData();
}

void ressiveData()
{
  if (!isWaitReqest)
  {
    return;
  }
  if (!WiznetInterface.isReady())
  {
    isWaitReqest = false;
    client.stop();
    return;
  }
  if (flprog::isTimer(startSendReqest, reqestTimeout))
  {
    isWaitReqest = false;
    Serial.println(">>> Client Timeout !");
    return;
  }
  if (client.available() == 0)
  {
    return;
  }
  while (client.available())
  {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }
  Serial.println();
  isWaitReqest = false;
}

void sendReqest()
{
  if (isWaitReqest)
  {
    return;
  }
  if (!(flprog::isTimer(startSendReqest, reqestPeriod)))
  {
    return;
  }
  if (!WiznetInterface.isReady())
  {
    client.stop();
    return;
  }

  uint8_t temp = client.connect(host, port);

  if (temp == FLPROG_WITE)
  {
    return;
  }
  if (temp == FLPROG_ERROR)
  {
    startSendReqest = millis();
    Serial.println("connection failed");
    return;
  }
  Serial.println();
  Serial.print("connecting to ");
  Serial.println();
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  Serial.println("sending data to server");
  Serial.println();
  if (client.connected())
  {
    client.println("hello from ESP8266");
  }
  startSendReqest = millis();
  isWaitReqest = true;
}

void printStatusMessages()
{
  if (WiznetInterface.getStatus() != ethernetStatus)
  {
    ethernetStatus = WiznetInterface.getStatus();
    Serial.print("Ethernet status -");
    Serial.println(flprog::flprogStatusCodeName(ethernetStatus));
  }
  if (WiznetInterface.getError() != ethernetError)
  {
    ethernetError = WiznetInterface.getError();
    Serial.print("Ethernet error - ");
    Serial.println(flprog::flprogErrorCodeName(ethernetError));
  }

  if (client.getStatus() != clientStatus)
  {
    clientStatus = client.getStatus();
    Serial.print("Client status -");
    Serial.println(flprog::flprogStatusCodeName(clientStatus));
  }
  if (client.getError() != clientError)
  {
    clientError = client.getError();
    Serial.print("Client error - ");
    Serial.println(flprog::flprogErrorCodeName(clientError));
  }
  printConnectMessages();
  printDisconnectMessages();
}

void printConnectMessages()
{
  if (!WiznetInterface.isReady())
  {
    return;
  }
  if (!isNeedSendConnectMessage)
  {
    return;
  }
  Serial.println("Ethernet подключён!");
  Serial.print("Ip - ");
  Serial.println(WiznetInterface.localIP());
  Serial.print("DNS - ");
  Serial.println(WiznetInterface.dns());
  Serial.print("Subnet - ");
  Serial.println(WiznetInterface.subnet());
  Serial.print("Gateway - ");
  Serial.println(WiznetInterface.gateway());
  isNeedSendConnectMessage = false;
  isNeedSendDisconnectMessage = true;
}

void printDisconnectMessages()
{
  if (WiznetInterface.isReady())
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

void blinkLed()
{
  if (flprog::isTimer(blinkStartTime, 50))
  {
    blinkStartTime = millis();
    digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));
  }
}