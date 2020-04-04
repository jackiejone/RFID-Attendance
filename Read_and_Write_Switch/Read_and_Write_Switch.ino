/*This code switches the RFID-RC522 between read and write mode when a switch conntected to pin ####### is switched.
 *The code for reading and writing to the RFID tags is derived from examples from a RFID custom library 
 *by miguelbalboa from his RFID github Repository (https://github.com/miguelbalboa/rfid.git)
 *by Jackie Jone
 */

// Importing custom libraries
#include <SPI.h>
#include <MFRC522.h>

// Defining pin numbers
#define RST_PIN 9
#define SS_PIN 10
#define Switch_PIN 8 // Defining pin which the switch is connected to

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance for RFID scanner

void setup() {
  pinMode(Switch_PIN, INPUT);                                   // Sets switch pin to an input pin
  Serial.begin(9600);                                           // Initialize serial monitor
  SPI.begin();                                                  // Initialize SPI bus
  mfrc522.PCD_Init();                                           // Initialize MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(Switch_PIN) == LOW) {
    return;
  } else if (digitalRead(Switch_PIN) == HIGH) {
    return;
  }
}
