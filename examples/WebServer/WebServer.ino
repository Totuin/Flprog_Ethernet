#include "flprogEthernet.h"

//Создаем объект шины 
FLProgSPI spiBus(0);

/*
   Создаём обект интерфейса на чипе W5100 (поддерживаются W5200 и W5500)
   В конструкторе передаём ссылку на шину к которой он подключён и номер пина SS
   стандартныйе номера пинов
   10 - Arduino shield
   5 - MKR ETH shield
   0 - Teensy 2.0
   20 - Teensy++ 2.0
   15 - ESP8266 with Adafruit Featherwing Ethernet
   33 - ESP32 with Adafruit Featherwing Ethernet
*/
FlprogW5100Interface W5100_Interface(&spiBus, 10);



// Создаём массив с MAC адресом
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Создаём IP адрес для интерфейса
IPAddress ip(192, 168, 199, 177);

/*
   Создаём обект сервера
   В конструкторе передаём ссылку на объект интерфейса на которм этот сервер будет слушать передаваемый порт
*/

FlprogEthernetServer server(&W5100_Interface, 80);

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // Ожидаем инициализацию порта
  }
  Serial.println("Ethernet WebServer Example");

  // Запускаем интерфейс
  W5100_Interface.begin(mac, ip);

  // Проверяем работоспособность интерфейса
  if (W5100_Interface.hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1);
    }
  }

  // Проверяем - находится ли в сети интерфейс
  if (W5100_Interface.linkStatus() == FLPROG_ETHERNET_LINK_OFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }

  // Стартуем сервер
  server.begin();
  Serial.print("server is at ");
  Serial.println(W5100_Interface.localIP()); // Показываем Ip интерфейса
}

void loop()
{
  // listen for incoming clients
  FlprogEthernetClient client = server.available();
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