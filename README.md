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
// Установка непосредственно числами
WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40);

// Установка путём передачи массива
uint8_t macAddr[6] = {0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40};
WiznetInterface.mac(macAddr);
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

<br>

- *Доступ к параметрам соеденения.*

```cpp
// Внимание если не отключить режим DHCP то заданные IP адреса при соеденении будут замененны полученными от сервера DHCP!

// Задание Ip адреса устройства.
// В качестве значения могут перетдаться как обект IPAddress так и четыре цифры
WiznetInterface.localIP(IPAddress(192, 168, 1, 100));
WiznetInterface.localIP(192, 168, 1, 100);

// Получение IP адреса устройства.
// Возвращает объект класса IPAddress
IPAddress ip = WiznetInterface.localIP();

// Задание Ip адреса DNS сервера.
// В качестве значения могут перетдаться как обект IPAddress так и четыре цифры
WiznetInterface.dns(IPAddress(192, 168, 1, 1));
WiznetInterface.dns(192, 168, 1, 1);

// Получение IP адреса DNS сервера
// Возвращает объект класса IPAddress
IPAddress ip = WiznetInterface.dns();

// Задание Ip адреса шлюза.
// В качестве значения могут перетдаться как обект IPAddress так и четыре цифры
WiznetInterface.gateway(IPAddress(192, 168, 1, 1));
WiznetInterface.gateway(192, 168, 1, 1);

// Получение IP адреса шлюза
// Возвращает объект класса IPAddress
IPAddress ip = WiznetInterface.gateway();

// Задание маски подсети. По умолчанию установленна 255.255.255.0
// В качестве значения могут перетдаться как обект IPAddress так и четыре цифры
WiznetInterface.subnet(IPAddress(255, 255, 255, 0));
WiznetInterface.subnet(255, 255, 255, );

// Получение маски подсети
// Возвращает объект класса IPAddress
IPAddress ip = WiznetInterface.subnet();
```
<br>

## **Управление интерфейсом**

```cpp
// Цикл работы интерфейса. 
// Обязательно вызывать один раз в секции loop().
// Возвращает результат выполнения цикла (описания значений результатов ниже).
uint8_t result = WiznetInterface.pool();

// Получение типа интерфейса (описания значений типов интерфейса ниже).
uint8_t type = WiznetInterface.type();

// Флаг указывающий что испольуемый интерфейс не поддерживается на данной платформе (true - интерфейс не поддерживается) 
bool isImitation = WiznetInterface.isImitation();

// Получение текущего статуса интерфейса (описания значений статусов ниже).
uint8_t type = WiznetInterface.getStatus();

// Получение текущей ошибка интерфейса (описания значений кодов ошибок ниже).
uint8_t type = WiznetInterface.getError();

 ```