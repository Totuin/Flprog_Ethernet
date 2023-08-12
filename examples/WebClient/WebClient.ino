
#include "flprogEthernet.h"

FLProgSPI spiBus(0);
FlprogW5100Interface W5100_Interface(&spiBus, 10);

const char *host = "djxmmx.net";
const uint16_t port = 17;

void setup()
{
  Serial.begin(115200);

  W5100_Interface.mac(0x78, 0xAC, 0xC0, 0x2C, 0x3E, 0x28); //--Установка MAC-адрес контроллера 
  while (!W5100_Interface.isReady())
  {
    W5100_Interface.pool();
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Ethernet connected");
  Serial.println("IP address: ");
  Serial.println(W5100_Interface.localIP());
}

void loop()
{
  W5100_Interface.pool();

  static bool wait = false;

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  FlprogEthernetClient client(&W5100_Interface);
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    delay(5000);
    return;
  }

  Serial.println("sending data to server");
  if (client.connected())
  {
    client.println("hello from ESP8266");
  }
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }
  Serial.println("receiving from remote server");
  while (client.available())
  {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  Serial.println();
  Serial.println("closing connection");
  client.stop();

  if (wait)
  {
    delay(300000); // execute once every 5 minutes, don't flood remote service
  }
  wait = true;
}