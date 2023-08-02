#include <flprogEthernet.h>

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
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Создаём IP адрес для интерфейса
IPAddress ip(192, 168, 199, 177);
IPAddress myDns(192, 168, 199, 1);

/*
   Создаем обект клиента и передаем ему ссылку на обект интерфейса с которым он будет работать
*/
FlprogEthernetClient client(&W5100_Interface);

char server[] = "www.arduino.cc";  // also change the Host line in httpRequest()
//IPAddress server(104,18,13,241);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10*1000;  // delay between updates, in milliseconds

void setup() {

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (W5100_Interface.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (W5100_Interface.hardwareStatus() == FLPROG_ETHERNET_NO_HARDWARE) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (W5100_Interface.linkStatus() == FLPROG_ETHERNET_LINK_OFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    W5100_Interface.begin(mac, ip, myDns);
    Serial.print("My IP address: ");
    Serial.println(W5100_Interface.localIP());
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(W5100_Interface.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:
    client.println("GET /latest.txt HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

