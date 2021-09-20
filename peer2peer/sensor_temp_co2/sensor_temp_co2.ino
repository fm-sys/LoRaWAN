/*
Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/sensor.ino, which corresponds to the code published 
in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks by Pradeeka Seneviratne (Apress, 2019)".

Temp messure DS18B20: https://www.youtube.com/watch?v=UY4zOXchK4w

*/
#include <DallasTemperature.h>
#include <OneWire.h>
#include <RH_RF95.h>

//Radio pinout setup
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

//temp messure pins
float currentTemperature = 0.0;  // define the current temperature variable
int oneWireBusPin = 13;          // define the 1 wire bus pin


OneWire oneWireBus(oneWireBusPin);                 // create an instance of the OneWire Library and define the oneWireBusPin in it
DallasTemperature temperatureSensor(&oneWireBus);  // create an instance of the Dallastemperature Library and define the OneWire instance oneWireBus in it
DeviceAddress temperatureSensorAddress;            // temperatureDeviceAddress is an uint8_t array [8]

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial)
    ;
  Serial.begin(9600);
  delay(100);

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("initializing…");
  }
  Serial.println("initialisation succeeded");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);




  temperatureSensor.begin();  // start the bus
  temperatureSensor.getDeviceCount();
  temperatureSensor.getAddress(temperatureSensorAddress, 0);


  if (temperatureSensor.setResolution(temperatureSensorAddress, 12, false) == true) {  // set resolution of a device to 9, 10, 11, or 12 bits. If new resolution is out of range, 9 bits is used.
    Serial.print("Resolution successfully set to ");
    Serial.print(temperatureSensor.getResolution(temperatureSensorAddress));  // get the resolution of a device with a given address
    Serial.println(" bit");
  } else {
    Serial.println("Resolution configuration failed ");
    Serial.print("Resolution automatically set to ");
    Serial.print(temperatureSensor.getResolution(temperatureSensorAddress));  // get the resolution of a device with a given address
    Serial.println(" bit");
  }
}

void loop() {

  temperatureSensor.requestTemperaturesByAddress(temperatureSensorAddress);   // command for a device with a specific address to perform a temperature conversion (to read the temperature)
  currentTemperature = temperatureSensor.getTempC(temperatureSensorAddress);  // Get temperature for device index in C (Slow process)

  char radiopacket[20];                            // create buffer
  dtostrf(currentTemperature, 5, 2, radiopacket);  // write to buffer

  Serial.print("Send '");
  Serial.print(radiopacket);
  Serial.println("'");


  Serial.println("Sending...");
  delay(10);

  rf95.send((uint8_t *)radiopacket, 20);

  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();


  delay(30 * 1000);
}
