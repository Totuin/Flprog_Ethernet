#include "flprogEthernet.h"

FLProgSPI spiBus(0);
FlprogW5100Interface W5100_Interface(&spiBus, 10);
FlprogEthernetServer server(&W5100_Interface, 80);

bool isNeedSendConnectMessage = true;
bool isNeedSendDisconnectMessage = true;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // Ожидаем инициализацию порта
  }
  Serial.println("WebServer демонстрация");

  W5100_Interface.mac(0x78, 0xAC, 0xC0, 0x0D, 0x5B, 0x86);
}

void loop()
{
  W5100_Interface.pool();
  if (W5100_Interface.isReady())
  {
    if (isNeedSendConnectMessage)
    {
      Serial.println("Ethernet подключён!");
      Serial.print("Ip - ");
      Serial.println(W5100_Interface.localIP());
      Serial.print("Subnet mask - ");
      Serial.println(W5100_Interface.subnet());
      Serial.print("Gateway IP - ");
      Serial.println(W5100_Interface.gateway());
      Serial.print("Dns IP - ");
      Serial.println(W5100_Interface.dns());
      isNeedSendConnectMessage = false;
      isNeedSendDisconnectMessage = true;
    }
  }
  else
  {
    if (isNeedSendDisconnectMessage)
    {
      Serial.println("Ethernet отключён!");
      isNeedSendConnectMessage = true;
      isNeedSendDisconnectMessage = false;
    }
  }

  FlprogEthernetClient client = server.accept();
  if (client)
  {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close"); // the connection will be closed after completion of the response
          client.println("Refresh: 5");        // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++)
          {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    // client.stop();
    Serial.println("client disconnected");
  }
  delay(500);
}