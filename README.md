Библиотека FLPROG Ethernet
===

Предназначена для реализации работы по сети Ethernet

## Контактная информация

- Title:  `FLPROG Ethernet`
- Authors: [`Глушенко Сергей`](@Totuin)
- Сайт: [FLProg](http://flprog.ru)
- Почта: [support@flprog.ru](mailto:support@flprog.ru)

## Зависимости

- [Flprog_Utilites](https://github.com/Totuin/Flprog_Utilites)
- RT_00_HW_BASE




## Поддерживаемые контроллеры

  ```
  Atmega328
  Mega2560
  STM32
  RP2040
  ESP8266
  ```

<br>
<br>

# Класс FLProgWiznetInterface

## __Конструктор__ <br> _Создание интерфейса для работы с чипом W5100(W5200,W5500)_

-   *Шина SPI и пин CS берутся из  __RT_HW_Base.device.spi.busETH__ и __RT_HW_Base.device.spi.csETH__.*

 ```cpp
 FLProgWiznetInterface WiznetInterface;
```
<br>

-   *С непосредственной привязкой  пину.<br> 
Пин CS - 10.<br> 
Шина SPI берётся из __RT_HW_Base.device.spi.busETH__.*
```cpp
  FLProgWiznetInterface WiznetInterface(10);
```
<br>

-   *С непосредственной привязкой  пину и шине. <br> 
Пин CS - 10.<br> 
Шина SPI - 0.*
```cpp
  FLProgWiznetInterface WiznetInterface(10, 0);
```

## __Настройка интерфейса__

-   *Установка MAC-адрес контроллера.<br> 
Обязательно вызывается в секции setup().*
```cpp
 WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40);
```
-   *Задание пина Cs.<br> 
Может вызываться в любой момент времени.*
```cpp
 WiznetInterface.setPinCs(10);
```
-   *Задание номера шины SPI.<br> 
Может вызываться в любой момент времени.*
```cpp
 WiznetInterface.setSpiBus(0);
 ```

-   *Задание периода попыток переподлючения при отсутствии соеденения с DHCP сервером в миллисекундах.<br> 
Может вызываться в любой момент времени <br>
Значение по умолчанию - 5000*
```cpp
 WiznetInterface.setReconnectionPeriod(6000);
 ```












## Состав библиотеки
```
|—— examples
|    |—— CheckWizNetConnect
|        |—— CheckWizNetConnectMinimal
|            |—— CheckWizNetConnectMinimal.ino
|        |—— CheckWizNetConnectNormal
|            |—— CheckWizNetConnectNormal.ino
|    |—— UdpNtpClient
|        |—— UdpNtpClienMinimal
|            |—— UdpNtpClienMinimal.ino
|        |—— UdpNtpClientNormal
|            |—— UdpNtpClientNormal.ino
|    |—— WebClient
|        |—— WebClientMinimal
|            |—— WebClientMinimal.ino
|        |—— WebClientNormal
|            |—— WebClientNormal.ino
|    |—— WebServer
|        |—— WebServerClassicMethod
|            |—— WebServerClassicMethod.ino
|        |—— WebServerClassicMethodMinimal
|            |—— WebServerClassicMethodMinimal.ino
|        |—— WebServerPollMethod
|            |—— WebServerPollMethod.ino
|—— src
|    |—— abstract
|        |—— flprogAbstactEthernetChanel.cpp
|        |—— flprogAbstactEthernetChanel.h
|        |—— flprogAbstactEthernetTCPChanel.h
|        |—— flprogAbstactEthernetUDPChanel.cpp
|        |—— flprogAbstactEthernetUDPChanel.h
|        |—— flprogAbstractEthernetHardware.h
|        |—— flprogAbstractTcpInterface.cpp
|        |—— flprogAbstractTcpInterface.h
|    |—— flprogEthernet.h
|    |—— hardware
|        |—— flprogWizNet.cpp
|        |—— flprogWizNet.h
|    |—— interfaces
|        |—— flprogWiznetInterface.cpp
|        |—— flprogWiznetInterface.h
|    |—— tcp
|        |—— flprogEthernetClient.cpp
|        |—— flprogEthernetClient.h
|        |—— flprogEthernetServer.cpp
|        |—— flprogEthernetServer.h
|    |—— udp
|        |—— flprogDhcp.cpp
|        |—— flprogDhcp.h
|        |—— flprogDns.cpp
|        |—— flprogDns.h
|        |—— flprogUdp.cpp
|        |—— flprogUdp.h
|—— keywords.txt
```

## References

- [paper-1]()
- [paper-2]()
- [code-1](https://github.com)
- [code-2](https://github.com)
  
## License

## Citing

If you use xxx,please use the following BibTeX entry.

```
```
