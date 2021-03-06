/*
  Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/receiver.ino, which corresponds to the code published
  in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks by Pradeeka Seneviratne (Apress, 2019)".

  The example source code was based on the original code published by Adafruit availabe at https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/rfm9x-test
  RadioHead Packet Radio library for embedded microprocessors - documentation: http://www.airspayce.com/mikem/arduino/RadioHead/
*/
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define LED 13

void setup() {
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
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}

void loop() {
  if (rf95.available()) {
    //Read the availabe message
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      //Receive the message
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: '");
      Serial.print((char*)buf);
      Serial.println("'");

      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      // Send a reply
      uint8_t data[] = "Hi";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Reply was sent");
      digitalWrite(LED, LOW);
    } else {
      //If receive failed
      Serial.println("Receive failed");
    }
  }
}
