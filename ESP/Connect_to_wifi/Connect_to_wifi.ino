/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.

    Must have same baud rate on serial monitor
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "RaspberryPiNetwork"
#define STAPSK  "password"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

const char* host = "192.168.4.1";
const uint16_t port = 80;

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

    HTTPClient http;
    String user, uid;
    user = "16327";
    uid = "1";

    String httpRequestData = "user=" + user + "&uid=" + uid ;

    http.begin("http://192.168.4.1/insert?user=16327&id=1");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    


    Serial.print("httpRequestData: ");
    Serial.print(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode>0) {
      Serial.print("\nHTTP Response code: ");
      Serial.print(httpResponseCode);
    }
    else {
      Serial.print("\nError code :");
      Serial.print(httpResponseCode);
      }

      http.end();
      delay(2000);
  }
