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

- **Atmega328**
- **Mega2560**
- **STM32**
- **Raspberry Pi Pico** *(RP 2040)*
- **ESP8266**
<br>

<br>

# Класс FLProgWiznetInterface

## **Конструктор** <br> *Создание интерфейса для работы с чипом W5100 (W5200, W5500)*

- *Шина SPI и пин CS берутся из  **RT_HW_Base.device.spi.busETH** и **RT_HW_Base.device.spi.csETH**.*

 ```cpp
FLProgWiznetInterface WiznetInterface;
```

<br>

- *С непосредственной привязкой  пину.<br>
Пин CS - 10.<br>
Шина SPI берётся из **RT_HW_Base.device.spi.busETH**.*

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

## **Настройка интерфейса**

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
int16_t pin = WiznetInterface.pinCs() ;
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

- *Настройка и получение параметров DHCP*

```cpp
// Задание  значения периода попыток переподлючения при отсутствии соеденения с DHCP сервером в миллисекундах.
// Может вызываться в любой момент времени.
// Значение по умолчанию - 5000
WiznetInterface.setReconnectionPeriod(6000);

// Получение значения периода попыток переподлючения при отсутствии соеденения с DHCP сервером в миллисекундах.
uint32_t period = WiznetInterface.reconnectionPeriod();

// Задание  значения  таймаута на ответ DHCP сервера после окончания которого будет отправлен повторный запрос, в миллисекундах.
// Может вызываться в любой момент времени.<br>
// Значение по умолчанию - 6000.
WiznetInterface.setResponseDhcpTimeout(2000);

// Получение значения  таймаута на ответ DHCP сервера после окончания которого будет отправлен повторный запрос, в миллисекундах.
uint32_t timeout = WiznetInterface.responseDhcpTimeout();

// Задание значения таймаута на весь процесс запроса к серверу DHCP в миллисекундах.
// Может вызываться в любой момент времени<br>
// Значение по умолчанию - 20000.
WiznetInterface.setDhcpTimeout(30000);

// Получение значения таймаута на весь процесс запроса к серверу DHCP в миллисекундах.
uint32_t timeout = WiznetInterface.dhcpTimeout();

// Задание значения периода обновления данных с сервера DHCP в миллисекундах.
// Может вызываться в любой момент времени
// Значение по умолчанию - 1800000.
WiznetInterface.setMaintainPeriod(200000);

// Получение значения периода обновления данных с сервера DHCP в миллисекундах.
uint32_t period = WiznetInterface.maintainPeriod();

// Включение режима получения IP адреса через DHCP 
WiznetInterface.setDhcp();

// Отключение режима получения IP адреса через DHCP 
WiznetInterface.resetDhcp();

// Управление режимом получения IP адреса через DHCP (true - включён, false - выключен)
WiznetInterface.dhcpMode(true);

// Получение текущего состояния режима получения IP адреса через DHCP 
bool mode =  WiznetInterface.isDhcp();
```
