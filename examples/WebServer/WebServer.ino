#include "flprogEthernet.h"

//-------------------------------------------------------------------------------------------------
//         Вариант с  шиной (SPI0) и пином(10) по умолчаниюю. Пин потом можно поменять.
//         Но если на этой шине висит ещё какое то устройство лучше применять второй вариант
//-------------------------------------------------------------------------------------------------
FLProgWiznetInterface WiznetInterface; //--Создание интерфейса для работы с чипом W5100(W5200,W5500) (по умолчанию CS pin - 10,  Шина SPI - 0);

//-------------------------------------------------------------------------------------------------
//        Второй вариант с непосредственной привязкой к шине и пину.
//-------------------------------------------------------------------------------------------------
 //FLProgWiznetInterface WiznetInterface(10, 0); //--Создание интерфейса для работы с чипом W5100(W5200,W5500) CS pin - 10, Шина SPI - 0;
 //FLProgWiznetInterface WiznetInterface(10); //--Создание интерфейса для работы с чипом W5100(W5200,W5500) CS pin - 10, (по умолчанию Шина SPI - 0);

FLProgEthernetServer server(&WiznetInterface, 80);

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

  WiznetInterface.mac(0x78, 0xAC, 0xC0, 0x0D, 0x5B, 0x86);
}

void loop()
{
  WiznetInterface.pool();
  if (WiznetInterface.isReady())
  {
    if (isNeedSendConnectMessage)
    {
      Serial.println("Ethernet подключён!");
      Serial.print("Ip - ");
      Serial.println(WiznetInterface.localIP());
      Serial.print("Subnet mask - ");
      Serial.println(WiznetInterface.subnet());
      Serial.print("Gateway IP - ");
      Serial.println(WiznetInterface.gateway());
      Serial.print("Dns IP - ");
      Serial.println(WiznetInterface.dns());
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

  FLProgEthernetClient client = server.accept();
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