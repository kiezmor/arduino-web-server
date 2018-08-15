#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <SimpleDHT.h>

#define REQ_BUF_SZ 40

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 45);
EthernetServer server(80);
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0};
char req_index = 0;

int pinDHT11 = 2;
SimpleDHT11 dht11;

void setup() {
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);

  Serial.println("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;
  }
  Serial.println("SUCCESS - SD card initialized.");
  if (!SD.exists("index.htm")) {
    Serial.println("ERROR - Can't find index.htm file!");
    return;
  } Serial.println("SUCCESS - Found index.htm file.");

  Serial.print("Server open on : ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;
          req_index++;
        }
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          if (StrContains(HTTP_req, "ajax_tmp")) {
            temp(client);
          }
          else {
            webFile = SD.open("index.htm");
            if (webFile) {
              while (webFile.available()) {
                client.write(webFile.read());
              } webFile.close();
            }
          }
          Serial.println(HTTP_req);
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void StrClear(char *str, char length) {
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

char StrContains(char *str, char *sfind) {
  char found = 0;
  char index = 0;
  char len;
  len = strlen(str);
  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }
}

void temp(EthernetClient cl) {
  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data))
  {
    Serial.print("Read DHT11 ko\n");
    return;
  }
  Serial.print((int)temperature); Serial.print(" *C, ");
  Serial.print((int)humidity); Serial.println(" %");
  cl.println((int)temperature); cl.println(" *C, ");
  cl.println((int)humidity); cl.println(" %");
}
