/* This code Connects to a WiFi Network and sends a JSON request with data to a HTTP server
 * By Jackie Jone
 */

// Importing external libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Defining WiFi Network variables
#ifndef STASSID
#define STASSID "RaspberryPiNetwork"
#define STAPSK  "password"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

// Defining HTTP server variables
const char* host = "192.168.4.1";
const uint16_t port = 80;


void setup() {
  // Setting up serial monitor
  Serial.begin(115200);
  
  // We Connecting to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);                    // Setting ESP to act as a WiFi client isntead of an Access point (default) or both client and access point
  WiFi.begin(ssid, password);             // Connecting to WiFi network

  while (WiFi.status() != WL_CONNECTED) { // Loop to print "." in serial monitor while connecting to the WiFi network
    delay(500);
    Serial.print(".");
  }

  // Printing network information to serial monitor
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {     // Loop to send data to the web server or HTTP server with 5 seconds of delay after sending and getting data
    send_data();  // Runs function to send data to web server
    delay(5000);  // Waits 5 seconds
    get_data();   // Runs function to get data from web server
    delay(5000);  // Waits 5 seconds
  }

// Function to send data to webserver
void send_data() {
  // Initilising HTTP client
  HTTPClient http;
  // Defining data to be sent to the HTTP server
  String user, uid;
  user = "16327";
  uid = "1";
  String httpRequestData = "user=" + user + "&uid=" + uid ;

  // Preparing address of HTTP server and JSON information
  http.begin("http://192.168.4.1/insert");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  Serial.print("\nhttpRequestData: ");
  Serial.print(httpRequestData);

  // Sending HTTP Post request to server with data
  int httpResponseCode = http.POST(httpRequestData);

  // Checking the HTTP response code received from the server 
  if (httpResponseCode>0) {
    Serial.print("\nHTTP Response code: ");
    Serial.print(httpResponseCode);
  }
  // If response code is less than or equal to 0 then something has gone wrong 
  // and the response code will be printed to the serial monitor for debugging
  else {
    Serial.print("\nError code :");
    Serial.print(httpResponseCode);
    }

    http.end();
  }

// Function to get data from web server
void get_data() {
  // Initilising HTTP client
  HTTPClient http;
  // Defining data which is put in the link to the HTTP server to get data back
  String userCode;
  userCode = "16327";

  // Preparing address of HTTP server to get data from
  http.begin("http://192.168.4.1/get_data/" + userCode);
  
  // Getting response code from webserver
  int httpCode = http.GET();

  // Getting data from webserver
  String payload = http.getString();
  Serial.println("\nReturned Data: " + payload);

  http.end();
  }
