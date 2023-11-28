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
Может вызываться в любой момент времени. <br>
Значение по умолчанию - 5000.*
```cpp
 WiznetInterface.setReconnectionPeriod(6000);
 ```
-   *Задание периода проверки состояния соеденения и состояния чипа в миллисекундах.<br> 
Может вызываться в любой момент времени.<br>
Значение по умолчанию - 1000.*
```cpp
 WiznetInterface.setCheckStatusPeriod(2000);
 ```

- *Задание таймаута на ответ DHCP сервера после окончания которого будет отправлен повторный запрос, в миллисекундах.<br> 
Может вызываться в любой момент времени.<br>
Значение по умолчанию - 6000.*
```cpp
 WiznetInterface.setResponseDhcpTimeout(2000);
 ```

   *Задание таймаута весь процесст запроса к серверу DHCP, в миллисекундах.<br> 
Может вызываться в любой момент времени.<br>
Значение по умолчанию - 20000.*
```cpp
 WiznetInterface.setDhcpTimeout(30000);
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
