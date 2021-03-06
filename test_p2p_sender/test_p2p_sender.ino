/*
  Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/sensor.ino, which corresponds to the code published
  in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks" by Pradeeka Seneviratne (Apress, 2019).

*/
#include <RH_RF95.h> //RadioHead RFM9x library

//Radio pinout setup
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

//Frequency
#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);


void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);

  // reset LoRa Shield
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("Initializing failed");
    while (1);
  }
  Serial.println("initialisation succeeded");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}

void loop() {
  char radiopacket[20] = "hello world";

  Serial.print("Send '");
  Serial.print(radiopacket);
  Serial.println("'");
  Serial.println("Sending...");
  delay(10);

  long t1 = millis();
  rf95.send((uint8_t *) radiopacket, 20);

  Serial.println("Waiting to complete...");
  delay(10);
  rf95.waitPacketSent();
  Serial.print(millis() - t1);
  Serial.println("ms Übertragungsdauer");


  // Waiting for the reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  delay(10);
  if (rf95.waitAvailableTimeout(1000)) {
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: '");
      Serial.print((char*)buf);
      Serial.println("'");
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    } else {
      Serial.println("Receive failed");
    }
  } else {
    Serial.println("No reply");
  }
  delay(30 * 1000);
}
