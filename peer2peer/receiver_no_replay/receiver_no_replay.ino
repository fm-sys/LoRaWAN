/*
Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/receiver.ino, which corresponds to the code published 
in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks by Pradeeka Seneviratne (Apress, 2019)".

The example source code was based on the original code published by Adafruit availabe at https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/rfm9x-test
RadioHead Packet Radio library for embedded microprocessors - documentation: http://www.airspayce.com/mikem/arduino/RadioHead/
*/
#include <SPI.h>
#include <RH_RF95.h>
#include <LiquidCrystal_I2C.h>
 
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
 
#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);
LiquidCrystal_I2C lcd(0x27, 20, 4);


#define LED 13

int packageID = 0;
uint8_t old_buf[RH_RF95_MAX_MESSAGE_LEN];
 
void setup() 
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Init...");

  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  while (!Serial);
  Serial.begin(9600);
  delay(100);
 
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("Initializing failed");
    while (1);
  }
  Serial.println("Initialisation succeeded");
 

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("Can't set the specifide frequency");
    while (1);
  }
  Serial.print("Set Freq to: "); 
  Serial.println(RF95_FREQ);

  Serial.println("----------------------------");
 
  rf95.setTxPower(23, false);

  lcd.print("Ready!");
}
 
void loop()
{
  if (rf95.available())
  {
    //Read the availabe message
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
      //Receive the message
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      //Serial.print("Got: '");
      Serial.println((char*)buf);
      //Serial.println("'");

      packageID++;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ID: #");
      lcd.print(packageID);

      lcd.print("  RSSI: ");
      lcd.print(rf95.lastRssi());

            //Serial.print("RSSI: ");
      //Serial.println(rf95.lastRssi(), DEC);


      lcd.setCursor(0,1);
      //lcd.print("\"");
      lcd.print("Daten: ");
      lcd.print((char*)buf);
      //lcd.print("\"");

      if (packageID > 1) {
        lcd.setCursor(0,2);
      lcd.print("Vorheriges Paket:");
      lcd.setCursor(0,3);
      //lcd.print("\"");
      lcd.print((char*)old_buf);
      //lcd.print("\"");
      }

      // reset old buffer
      for(int i = strlen((char*)old_buf); i>=0; i--) {
        old_buf[i] = '\0'; 
      }
      for(int i = strlen((char*)buf); i>=0; i--) {
        old_buf[i] = buf[i]; 
      }


    }
    else
    {
      //If receive failed
      Serial.println("Receive failed");
    }
  }
}
