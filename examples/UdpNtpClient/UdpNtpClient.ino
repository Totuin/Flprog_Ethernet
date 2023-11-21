//=================================================================================================
//                              1.Test Ethernet
//  Получение по UDP точного времени.
//=================================================================================================
#include <flprogEthernet.h> //подключаем библиотеку Ethernet

//=================================================================================================
//  Определяем целевую платформу
//=================================================================================================

#ifdef ARDUINO_ARCH_AVR
#define TEST_BOARD "AVR"
#elif ARDUINO_ARCH_SAM
#define TEST_BOARD "SAM"
#elif ARDUINO_ARCH_ESP8266
#define TEST_BOARD "ESP8266"
#elif ARDUINO_ARCH_ESP32
#define TEST_BOARD "ESP32"
#elif ARDUINO_ARCH_RP2040
#define TEST_BOARD "RP2040"
#elif ARDUINO_ARCH_STM32
#define TEST_BOARD "STM32"
#else
#define TEST_BOARD "ANON"
#endif

//-------------------------------------------------------------------------------------------------
//         Вариант с  шиной (SPI0) и пином(10) по умолчаниюю. Пин потом можно поменять.
//         Но если на этой шине висит ещё какое то устройство лучше применять второй вариант
//-------------------------------------------------------------------------------------------------
//FLProgWiznetInterface WiznetInterface; //--Создание интерфейса для работы с чипом W5100(W5200,W5500) (по умолчанию CS pin - 10,  Шина SPI - 0);

//-------------------------------------------------------------------------------------------------
//        Второй вариант с непосредственной привязкой к шине и пину.
//-------------------------------------------------------------------------------------------------
//FLProgWiznetInterface WiznetInterface(10, 0); //--Создание интерфейса для работы с чипом W5100(W5200,W5500) CS pin - 10, Шина SPI - 0;
FLProgWiznetInterface WiznetInterface(10); //--Создание интерфейса для работы с чипом W5100(W5200,W5500) CS pin - 10, (по умолчанию Шина SPI - 0);


//-------------------------------------------------------------------------------------------------
//         Задание параметров интернет соеденения и параметров UDP
//-------------------------------------------------------------------------------------------------

uint32_t localPort = 8888;                            //--Определение порта для UDP пакетов (используется стандартный номер);
const char timeServer[] = "time.nist.gov";
//const char timeServer[] = "128.138.140.44";
//const char timeServer[] = "192.168.0.101";
//const char timeServer[] = "ntp1.vniiftri.ru";
//--Имя NTP сервера - сервер точного времени;

const int NTP_PACKET_SIZE = 48;                       //--Установка размера буфера (отметка времени NTP находится в первых 48 байтах сообщения);
uint8_t packetBuffer[NTP_PACKET_SIZE];                //--Создание буфера для хранения входящих и исходящих пакетов;
uint16_t cntGettingNTP = 0;                           //--Cчетчик принятых пакетов;
bool isNeedSendConnectMessage = true;
bool isNeedSendDisconnectMessage = true;

//-------------------------------------------------------------------------------------------------
//          1.2.Создание объекта UDP для отправки и получения пакетов по UDP с привязкой к интерфейсу
//-------------------------------------------------------------------------------------------------
FLProgEthernetUDP Udp(&WiznetInterface); //--Создание UDP клиента;

//-----------------------------------------------------------------------------------------
//          1.3.Определение рабочих параметров и функций
//-----------------------------------------------------------------------------------------
uint32_t  sendPacadeTime;   // Время начала ожидания
uint32_t reqestPeriod = 5000;         // Периодичность запроса времени от сервера
bool isReplyInProcess = false;        // Флаг ожидания ответа

void sendNTPpacket(const char *nameSever); //--Предопределение функции;
void processingResponse();                 //--Предопределение функции;

uint8_t ethernetStatus = 255;
uint8_t oldUdpStatus = 255;
uint32_t blinkStartTime = 0;

//=================================================================================================
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }
  Serial.print(String(F("\n-----Тест UdpNtpClient для платы ")));
  Serial.print(String(F(TEST_BOARD)));
  Serial.println(String(F("-----")));

  WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40); //--Установка MAC-адрес контроллера (лучше адрес прошитый производителем);
  // WiznetInterface.localIP(192, 168, 199, 155);
  // WiznetInterface.resetDhcp();


  sendPacadeTime = flprog::timeBack(reqestPeriod);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}
//=================================================================================================
void loop()
{

  if (flprog::isTimer(blinkStartTime, 200))
  {
    blinkStartTime = millis();
    digitalWrite(LED_BUILTIN, !( digitalRead(LED_BUILTIN)));
  }

  WiznetInterface.pool(); // Цикл работы интерфейса

  if ( WiznetInterface.getStatus() != ethernetStatus)
  {
    ethernetStatus = WiznetInterface.getStatus();
    Serial.print("Ethernet ");
    Serial.print(flprog::flprogCodeName(ethernetStatus));
    Serial.print(" - ");
    Serial.print(ethernetStatus);
    Serial.println();

  }
  //-------------------------------------------------------------------------------------------------
  //                                           Основная логика
  //-------------------------------------------------------------------------------------------------
  if ( WiznetInterface.isReady())
  {
    if (isNeedSendConnectMessage)
    {
      //-------------------------------------------------------------------------------------------------
      //                      Если интерфейс готов то вывод сообщения о подключении
      //-------------------------------------------------------------------------------------------------
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
    //-------------------------------------------------------------------------------------------------
    //                          работаем с NTP сервером
    //-------------------------------------------------------------------------------------------------
    if (isReplyInProcess)
    {
      //--Если  ждём ответа проверяем ответ
      processingResponse();
    }
    else
    {
      //--Если не ждём ответа отправляем запрос
      sendNTPpacket(timeServer);
    }
  }
  else
  {
    //-------------------------------------------------------------------------------------------------
    //      Если интерфейс не готов то  выводим один раз сообщение об обрыве связи
    //-------------------------------------------------------------------------------------------------
    if (isNeedSendDisconnectMessage)
    {
      Serial.println("Ethernet отключён!");
      isNeedSendConnectMessage = true;
      isNeedSendDisconnectMessage = false;
    }
  }

} //======END loop();

// #################################################################################################
//=================================================================================================
//                         1.ФУНКЦИЯ sendNTPpacket(*nameServer)
//     Формирование запроса в буфере packetBuffer и его отправка в NTP сервер
//=================================================================================================
void sendNTPpacket(const char *nameServer)
{


  //-------------------------------------------------------------------------------------------------
  //                 Отправка запроса на сервер точного времени
  //------------------------------------------------------------------------------------------------ -
  if (!flprog::isTimer(sendPacadeTime, reqestPeriod)) //--Проверяем - прошло ли время после последнего запроса
  {
    return; //-- Если нет - выходим
  }
  //-------------------------------------------------------------------------------------------------
  //  1.1.Инициализация буфера для отправки запросра требуемой формы в  NTP сервер
  //    (see URL above for details on the packets)
  //    Байты 4-11: 8 bytes of zero for Root Delay & Root Dispersion
  //-------------------------------------------------------------------------------------------------

  memset(packetBuffer, 0, NTP_PACKET_SIZE); //--Очистка буфера
  packetBuffer[0] = 0b11100011;             // LI, Version, Mode
  packetBuffer[1] = 0;                      // Stratum, or type of clock
  packetBuffer[2] = 6;                      // Polling Interval
  packetBuffer[3] = 0xEC;                   // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  //-------------------------------------------------------------------------------------------------
  //   1.2.Отправка подготовленного  пакет с запросом метки времени
  //-------------------------------------------------------------------------------------------------

  Udp.begin(localPort);
  uint8_t udpStatus = Udp.beginPacket(nameServer, 123);
  if (oldUdpStatus != udpStatus)
  {
    oldUdpStatus = udpStatus;
    Serial.print("UDP ");
    Serial.print(flprog::flprogCodeName(ethernetStatus));
    Serial.print(" - ");
    Serial.print(ethernetStatus);
    Serial.println();
  }

  if (udpStatus == FLPROG_WAIT_STATUS)
  {
    return;
  }

  if (udpStatus == FLPROG_SUCCESS)
  {
    if (!Udp.write(packetBuffer, NTP_PACKET_SIZE)) //--Отправка запроса
    {
      Serial.println("UDP Ошибка записи!");
      sendPacadeTime = millis();
      oldUdpStatus = 255;
      return;
    }
    if (!Udp.endPacket()) //--Завершение запроса;
    {
      Serial.println("UDP Ошибка отправки!");
      sendPacadeTime = millis();
      oldUdpStatus = 255;
      return;
    }

    isReplyInProcess = true;   //--Вешаем флаг ожидания ответа
  }
  sendPacadeTime = millis(); // --Запоминаем время отправки запроса
  oldUdpStatus = 255;
};                           //======END sendNTPpacket();

// #################################################################################################
//=================================================================================================
//                2.ФУНКЦИЯ processingResponse()
//                обработка ответа из NTP сервера
//=================================================================================================
void processingResponse()
{
  //-------------------------------------------------------------------------------------------------
  //                Ожидание ответа и его обработка
  //-------------------------------------------------------------------------------------------------
  if (flprog::isTimer(sendPacadeTime, 15000)) // проверяем прошло ли время ожидания ответа
  {
    isReplyInProcess = false;
    Udp.stop();
    Serial.println("Нет ответа от сервера!");
    return;
  }

  //--Проверка наличия ответного пакета от сервера;
  if (Udp.parsePacket() <= 0)
  {
    return; //--если нет ответа - выходим
  }
  Udp.read(packetBuffer, NTP_PACKET_SIZE); //--Чтение пакета в буфер.read. В байтах 40-43 находятся сведения о времени;
  Udp.stop();
  cntGettingNTP++;                         //--Считаем пакеты

  //--Вывод текущего, абсолютного и UNIX времени;
  uint16_t highWord = word(packetBuffer[40], packetBuffer[41]);
  uint16_t lowWord = word(packetBuffer[42], packetBuffer[43]);
  uint32_t secsSince1900 = ((uint32_t)highWord << 16) | lowWord; //--Получения времени в сек от 01.01.1900;

  //-------------------------------------------------------------------------------------------------
  //              Unix-time время в сек от 01.01.1970,
  //                  что соответствует 2208988800;
  //-------------------------------------------------------------------------------------------------
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