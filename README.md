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

- *Шина SPI и пин CS берутся из  __RT_HW_Base.device.spi.busETH__ и __RT_HW_Base.device.spi.csETH__.*
 ```cpp
FLProgWiznetInterface WiznetInterface;
```
<br>

- *С непосредственной привязкой  пину.<br>
Пин CS - 10.<br>
Шина SPI берётся из __RT_HW_Base.device.spi.busETH__.*
```cpp
FLProgWiznetInterface WiznetInterface(10);
```
<br>

- *С непосредственной привязкой  пину и шине. <br>
Пин CS - 10.<br>
Шина SPI - 0.*
```cpp
FLProgWiznetInterface WiznetInterface(10, 0);
```

## __Настройка интерфейса__

- *Установка MAC-адрес контроллера.<br>
Обязательно вызывается в секции setup().*
```cpp
WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40);
```
<br>

- *Получение текущего MAC-адрес контроллера.<br>
Возврщает ссылку на массив из 6 элементов типа **uint8_t**.*
```cpp
uint8_t *macAddres = WiznetInterface.mac();
```
<br>

- *Задание пина Cs и получение текущего значения этого пина.<br>
Может вызываться в любой момент времени.*
```cpp
// Задание пина
WiznetInterface.setPinCs(10);
  
// Получение номера пина  
int = WiznetInterface.pinCs() ;
```
<br>

- *Задание номера шины SPI, и получение текущего значение номера шины.<br>
Может вызываться в любой момент времени.*
```cpp
// Задание номера шины
WiznetInterface.setSpiBus(0);

// Получение номера шины
uint8_t bus = WiznetInterface.spiBus()
 ```
<br>

- *Задание и получение значения периода попыток переподлючения при отсутствии соеденения с DHCP сервером в миллисекундах.<br>
Может вызываться в любой момент времени. <br>
Значение по умолчанию - 5000.*
```cpp
// Задание периода
WiznetInterface.setReconnectionPeriod(6000);

// Получение периода
uint32_t period = WiznetInterface.reconnectionPeriod();
 ```
<br>

- *Задание и получение значения  периода проверки  соеденения и состояния чипа в миллисекундах.<br>
Может вызываться в любой момент времени.<br>
Значение по умолчанию - 1000.*
```cpp
// Задание периода
WiznetInterface.setCheckStatusPeriod(2000);

// Получение периода
uint32_t period = WiznetInterface.checkStatusPeriod();
 ```
<br>

- *Задание и получение значения  таймаута на ответ DHCP сервера после окончания которого будет отправлен повторный запрос, в миллисекундах.<br>
Может вызываться в любой момент времени.<br>
Значение по умолчанию - 6000.*
```cpp
// Задание значения
WiznetInterface.setResponseDhcpTimeout(2000);

// Получение значения
uint32_t timeout = WiznetInterface.responseDhcpTimeout();
 ```
<br>

- *Задание и получение значения таймаута на весь процесс запроса к серверу DHCP в миллисекундах.<br>
Может вызываться в любой момент времени<br>
Значение по умолчанию - 20000.*
```cpp
// Задание значения
WiznetInterface.setDhcpTimeout(30000);

// Получение значения
uint32_t timeout = WiznetInterface.dhcpTimeout();
 ```
<br>

- *Задание и получение значения периода обновления данных с сервера DHCP в миллисекундах.<br>
Может вызываться в любой момент времени<br>
Значение по умолчанию - 1800000.*
```cpp
// Задание значения
WiznetInterface.setMaintainPeriod(200000);

// Получение значения
uint32_t period = WiznetInterface.maintainPeriod();
 ```
