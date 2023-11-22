#include <flprogEthernet.h> //подключаем библиотеку Ethernet

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
    Serial.println(archName);
    Serial.print("Плата - ");
    Serial.println(boarbName);

    WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x40); //--Установка MAC-адрес контроллера
    // WiznetInterface.localIP(192, 168, 199, 155);
    // WiznetInterface.resetDhcp();

    pinMode(LED_BUILTIN, OUTPUT);
}
//=================================================================================================
void loop()
{
    WiznetInterface.pool(); // Цикл работы интерфейса
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
