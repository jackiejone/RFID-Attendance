/*This code switches the RFID-RC522 between read and write mode when a switch conntected to pin D4 is pressed.
  The code for reading and writing to the RFID tags is derived from examples from a RFID custom library
  by miguelbalboa from his RFID github Repository (https://github.com/miguelbalboa/rfid.git)
  The code also sends the data scanned from an RFID tag to a server over WIFI
  by Jackie Jone

  When uploading code to Wemos, you need to disconnect pins D0, D4
*/

// Importing custom libraries
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LiquidCrystal_I2C.h>

// Setting LCD address for I2C
LiquidCrystal_I2C lcd(0x27,20,4);

// Defining pin numbers
#define RST_PIN D3  // Reset Pin for MFRC522
#define SS_PIN D8   // Slave Select Pin
#define RED_LED D0  // LED pin for indicating read or write mode

// Defining network names and passwords
#ifndef STASSID
#define STASSID "RaspberryPiNetwork"
#define STAPSK  "password"
#endif

// Seting networking variables
const char* ssid     = STASSID;
const char* password = STAPSK;

// Sever IP and Port
const char* host = "192.168.4.1";
const uint16_t port = 80;

#define statePin D; // Defining pin which the switch is connected to
volatile byte state = LOW;    // Defining which mode the system is in, read or write for RFID

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance for RFID scanner

void ICACHE_RAM_ATTR switchState(); // Don't know what this does but it makes the code work

void setup() {
  Serial.begin(115200);                                         // Initialize serial monitor
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  String message = "Connecting to: " + String(ssid); 
  lcd.print(message);
  // Connecting to WIFI network as a client
  Serial.print("\nConnecting to: ");
  Serial.println(ssid);

  
  
  WiFi.mode(WIFI_STA); // Setting ESP8266 as a wifi client
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    lcd.scrollDisplayLeft();
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,0);

  pinMode(statePin, INPUT);                          // Sets interrupt pin
  SPI.begin();                                                  // Initialize SPI bus
  mfrc522.PCD_Init();                                           // Initialize MFRC522 card
  pinMode(RED_LED, OUTPUT);                                     // Sets LED pin
  Serial.println("\nSet to Read");                              // Shows that system is in read mode
}

void loop() {
  // Checks for a high or low signal to switch between reading RFID tags using the
  // MFRC522 or writing to RFID tags
  if (digitalRead(statePin) == LOW) { // Checks state of variable to define whether it should be in read or write mode
    digitalWrite(RED_LED, LOW);
    RFID_read(); // Runs function for reading RFID tag
    lcd.clear();
    lcd.print("Mode: Read");
    delay(500);
  } else if (digitalRead(statePin) == HIGH) {
    digitalWrite(RED_LED, HIGH);
    RFID_write(); // Runs function for writing to RFID tag
    lcd.clear();
    lcd.print("Mode: Write");
    delay(500);
  }
}


// Function for sending data to server
void send_data(const String uid, const String user) {
    HTTPClient http; // Begins HTTP client
    Serial.print(uid);
    Serial.print(user);
    String httpRequestData = "user=" + user + "&uid=" + uid;
    http.begin("http://192.168.4.1/insert");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    Serial.print("\nhttpRequestData: ");
    Serial.print(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode>0) {
      Serial.println("HTTP Response code: ");
      Serial.print(httpResponseCode);
    }
    else {
      Serial.println("Error code: ");
      Serial.print(httpResponseCode);
      }
  }

byte get_data() {
  
  }

// Defining function for reading the RFID chip
void RFID_read() {

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card


  //------------------------------------------- GET STUDENT ID
  Serial.print(F("Student ID: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //STUDENT ID
  char usrid[] = "";
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      usrid[i] = buffer1[i];
      Serial.write(buffer1[i]);
    }
  }
  Serial.print("\nUSRID string: ");
  Serial.print(usrid);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("User ID: ");
  lcd.print(usrid);
  Serial.print("\n");
  String userid;
  userid = String(usrid);

  //---------------------------------------- GET FIRST NAME

  Serial.print(F("Name: \n"));

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT FIRST
  char uname[] = "";
  for (uint8_t i = 0; i < 16; i++) {
    uname[i] = buffer2[i];
    Serial.write(buffer2[i]);
  }
  Serial.print("\nFist name: ");
  Serial.println(uname);
  lcd.setCursor(0,1);
  lcd.print("Username: ");
  lcd.print(uname);
  lcd.setCursor(0,0);
  String username;
  username = String(uname);
  send_data(userid, username);
  delay(1000);
  //----------------------------------------

  Serial.println(F("\n**End Reading**\n"));
  delay(500); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}






// Defining function for writing to the RFID chip
void RFID_write() {
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  // Ask personal data: Name
  Serial.println(F("Type name, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ; // read family name from serial
  for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  // Ask personal data: student number
  Serial.println(F("Type student number, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 20) ; // read first name from serial
  for (byte i = len; i < 20; i++) buffer[i] = ' ';     // pad with spaces

  block = 4;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 5;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));


  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}
