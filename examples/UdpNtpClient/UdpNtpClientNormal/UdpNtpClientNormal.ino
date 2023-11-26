/*
  =================================================================================================
                  Test Ethernet
                  Получение по UDP точного времени.
  =================================================================================================
*/
#include <flprogEthernet.h> //подключаем библиотеку Ethernet

/*
  -------------------------------------------------------------------------------------------------
        Создание интерфейса для работы с чипом W5100(W5200,W5500)
        Шина SPI и пин CS берутся из  RT_HW_Base.device.spi.busETH и RT_HW_Base.device.spi.csETH
  -------------------------------------------------------------------------------------------------
*/
FLProgWiznetInterface WiznetInterface;

/*
  -------------------------------------------------------------------------------------------------
        Второй вариант cоздания интерфейса для работы с чипом W5100(W5200,W5500).
       С непосредственной привязкой  пину.
        Пин CS - 10
        Шина SPI берётся из RT_HW_Base.device.spi.busETH
  -------------------------------------------------------------------------------------------------
*/
// FLProgWiznetInterface WiznetInterface(10);

/*
  -------------------------------------------------------------------------------------------------
        Третий вариант cоздания интерфейса для работы с чипом W5100(W5200,W5500).
        С непосредственной привязкой  пину и шине.
       Пин CS - 10
       Шина SPI - 0
  -------------------------------------------------------------------------------------------------
*/
// FLProgWiznetInterface WiznetInterface(10, 0);

/*
  -------------------------------------------------------------------------------------------------
         Задание параметров интернет соеденения и параметров UDP
  -------------------------------------------------------------------------------------------------
*/

uint32_t localPort = 8888; //--Определение порта для UDP пакетов (используется стандартный номер);

const char *timeServer = "ntp1.vniiftri.ru"; //--Имя NTP сервера - сервер точного времени;
// const char *timeServer = "time.nist.gov";
// const char *timeServer = "128.138.140.44";
// IPAddress  timeServer = IPAddress(128, 138, 140, 44);

const int NTP_PACKET_SIZE = 48;        //--Установка размера буфера (отметка времени NTP находится в первых 48 байтах сообщения);
uint8_t packetBuffer[NTP_PACKET_SIZE]; //--Создание буфера для хранения входящих и исходящих пакетов;
uint16_t cntGettingNTP = 0;            //--Cчетчик принятых пакетов;
bool isNeedSendConnectMessage = true;
bool isNeedSendDisconnectMessage = true;

/*
  -------------------------------------------------------------------------------------------------
         Создание объекта UDP для отправки и получения пакетов по UDP с привязкой к интерфейсу
  ------------------------------------------------------------------------------------------------
*/
FLProgUdpClient Udp(&WiznetInterface); //--Создание UDP клиента;

/*
  -----------------------------------------------------------------------------------------
         Определение рабочих параметров и функций
  -----------------------------------------------------------------------------------------
*/
uint32_t reqestPeriod = 5000;                             // Периодичность запроса времени от сервера
uint32_t sendPacadeTime = flprog::timeBack(reqestPeriod); // Время начала ожидания
bool isReplyInProcess = false;                            // Флаг ожидания ответа

uint8_t ethernetStatus = 255;
uint8_t ethernetError = 255;

uint8_t udpStatus = 255;
uint8_t udpError = 255;

uint32_t blinkStartTime = 0;
uint32_t printPointTime = 0;

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
  Serial.print("CS - ");
  Serial.println(WiznetInterface.pinCs());
  Serial.print("SPI BUS - ");
  Serial.println(WiznetInterface.spiBus());

  WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40); //--Установка MAC-адрес контроллера
  // WiznetInterface.localIP(192, 168, 199, 155);
  // WiznetInterface.resetDhcp();

  Udp.setDnsCacheStorageTime(600000); // Устанавливаем клиенту время хранаения DNS кэша 10 минут

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

//=================================================================================================
void loop()
{
  WiznetInterface.pool();
  printStatusMessages();
  blinkLed();
  workInUDP();
}

//=================================================================================================
void printStatusMessages()
{
  if (WiznetInterface.getStatus() != ethernetStatus)
  {
    ethernetStatus = WiznetInterface.getStatus();
    Serial.println();
    Serial.print("Статус интерфейса - ");
    Serial.println(flprog::flprogStatusCodeName(ethernetStatus));
  }
  if (WiznetInterface.getError() != ethernetError)
  {
    ethernetError = WiznetInterface.getError();
    if (ethernetError != FLPROG_NOT_ERROR)
    {
      Serial.println();
      Serial.print("Ошибка интерфейса - ");
      Serial.println(flprog::flprogErrorCodeName(ethernetError));
    }
  }
  if (Udp.getStatus() != udpStatus)
  {
    udpStatus = Udp.getStatus();
    Serial.print("Статус UDP - ");
    Serial.println(flprog::flprogStatusCodeName(udpStatus));
  }
  if (Udp.getError() != udpError)
  {
    udpError = Udp.getError();
    if (udpError != FLPROG_NOT_ERROR)
    {
      Serial.print("Ошибка UDP - ");
      Serial.println(flprog::flprogErrorCodeName(udpError));
    }
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

void workInUDP()
{
  if (!WiznetInterface.isReady())
  {
    isReplyInProcess = false;
    return;
  }
  if (!isReplyInProcess)
  {
    sendNTPpacket();
    return;
  }
  processingResponse();
}

/*
  #################################################################################################
  =================================================================================================
                         ФУНКЦИЯ sendNTPpacket()
     Формирование запроса в буфере packetBuffer и его отправка в NTP сервер
  =================================================================================================
*/
void sendNTPpacket()
{
  if (!flprog::isTimer(sendPacadeTime, reqestPeriod)) //--Проверяем - прошло ли время после последнего запроса
  {
    if (flprog::isTimer(printPointTime, 1000))
    {
      Serial.print(".");
      printPointTime = millis();
    }
    return; //-- Если нет - выходим
  }

  Udp.begin(localPort);
  uint8_t result = Udp.beginPacket(timeServer, 123);
  if (result == FLPROG_WITE)
  {
    return;
  }

  memset(packetBuffer, 0, NTP_PACKET_SIZE); //--Очистка буфера
  packetBuffer[0] = 0b11100011;             // LI, Version, Mode
  packetBuffer[1] = 0;                      // Stratum, or type of clock
  packetBuffer[2] = 6;                      // Polling Interval
  packetBuffer[3] = 0xEC;                   // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  if (result == FLPROG_SUCCESS)
  {
    if (Udp.write(packetBuffer, NTP_PACKET_SIZE))
    {
      if (Udp.endPacket())
      {
        isReplyInProcess = true;
      }
      else
      {
        Serial.println("UDP Ошибка отправки!");
      }
    }
    else
    {
      Serial.println("UDP Ошибка записи!");
    }
  }
  sendPacadeTime = millis();
}

/*
  #################################################################################################
  =================================================================================================
               ФУНКЦИЯ processingResponse()
               обработка ответа из NTP сервера
  =================================================================================================
*/
void processingResponse()
{
  if (flprog::isTimer(sendPacadeTime, 15000)) // проверяем прошло ли время ожидания ответа
  {
    isReplyInProcess = false;
    Udp.stop();
    Serial.println("Нет ответа от сервера!");
    return;
  }
  if (Udp.parsePacket() <= 0)
  {
    return;
  }
  Udp.read(packetBuffer, NTP_PACKET_SIZE);
  Udp.stop();
  cntGettingNTP++;
  uint16_t highWord = word(packetBuffer[40], packetBuffer[41]);
  uint16_t lowWord = word(packetBuffer[42], packetBuffer[43]);
  uint32_t secsSince1900 = ((uint32_t)highWord << 16) | lowWord;
  /*
    -------------------------------------------------------------------------------------------------
                Unix-time время в сек от 01.01.1970,
                    что соответствует 2208988800;
    -------------------------------------------------------------------------------------------------
  */
  uint32_t epoch = secsSince1900 - 2208988800UL;
  uint32_t vr;
  Serial.print(F("\nUTC(Greenwich)="));
  vr = (epoch % 86400L) / 3600;
  if (vr < 10)
  {
    Serial.print('0');
  }
  Serial.print(vr); //--Вывод часов (86400 сек в сутках);
  vr = (epoch % 3600) / 60;
  Serial.print(':');
  if (vr < 10)
  {
    Serial.print('0');
  }
  Serial.print(vr); //--Вывод минут (3600 сек в минуте);
  vr = epoch % 60;
  Serial.print(':');
  if (vr < 10)
  {
    Serial.print('0');
  }
  Serial.print(vr); //--Вывод сек;
  Serial.print(F(";  Time 01.01.1900="));
  Serial.print(secsSince1900); //--Вывод абсолютного времени в сек(с 01.01.1990);
  Serial.print(F(";  Time Unix="));
  Serial.print(epoch); //--Вывод UNIX времени (с 01.01.1970)
  Serial.print(F(";  Счетчик="));
  Serial.print(cntGettingNTP);
  Serial.println(';'); //--Вывод счетчика принятых пакетов;
  isReplyInProcess = false;
};