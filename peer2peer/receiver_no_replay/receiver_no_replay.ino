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

// LED bar
int virtualVCC = 5;
int clockPin = 4;
int dataPin = 3;

int packageID = 0;
uint8_t old_buf[RH_RF95_MAX_MESSAGE_LEN];

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Init...");

  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(virtualVCC, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // virtuelle Spannungsversorgung über pin 5
  digitalWrite(virtualVCC, HIGH);

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

void loop() {
  if (rf95.available()) {
    //Read the availabe message
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      //Receive the message
      digitalWrite(LED, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      //Serial.print("Got: '");
      Serial.println((char*)buf);
      //Serial.println("'");

      packageID++;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ID: #");
      lcd.print(packageID);

      lcd.print("  RSSI: ");
      lcd.print(rf95.lastRssi());

      //Serial.print("RSSI: ");
      //Serial.println(rf95.lastRssi(), DEC);


      lcd.setCursor(0, 1);
      //lcd.print("\"");
      lcd.print("Daten: ");
      lcd.print((char*)buf);

      updateLedBar(map(parseTempValueFromRaw(buf) * 10, 0, 400, 0, 16)); // 0.0 - 40.0 °C umgerechnet auf Skala 0-16
      //lcd.print("\"");

      if (packageID > 1) {
        lcd.setCursor(0, 2);
        lcd.print("Vorheriges Paket:");
        lcd.setCursor(0, 3);
        //lcd.print("\"");
        lcd.print((char*)old_buf);
        //lcd.print("\"");
      }

      // reset old buffer
      for (int i = strlen((char*)old_buf); i >= 0; i--) {
        old_buf[i] = '\0';
      }
      for (int i = strlen((char*)buf); i >= 0; i--) {
        old_buf[i] = buf[i];
      }


    }
    else {
      //If receive failed
      Serial.println("Receive failed");
    }
  }
}

float parseTempValueFromRaw(char* raw) {
  // strings in C are guaranteed to be null-terminated
  bool started = false;
  bool isNegative = false;
  bool decimalPointReached = false;
  float decimalPositionFactor = 1.0;

  float value = 0.0;

  // data format: "d1|24.06|433"
  for (int i = 0; raw[i] != '\0'; i++) {
    if (raw[i] == '|') {
      started = !started;
      if (!started) {
        break; // we are all done and can leave the loop...
      }
    } else if (started) {
      switch (raw[i]) {
        case '-':
          isNegative = true;
          break;
        case '.':
          decimalPointReached = true;
          break;
        default:
          if (!decimalPointReached) {
            // move existing number one step to the left
            value *= 10;
          } else {
            // don't change parsed value, but change decimalPositionFactor instead
            decimalPositionFactor /= 10;
          }
          value += charToInt(raw[i]) * decimalPositionFactor;
          break;
      }
    }
  }
  return isNegative ? value * -1 : value;
}

int charToInt(char character) {
  // ascii values of the characters are subtracted from each other.
  // Since 0 comes directly before 1 in the ascii table (and so on until 9), the difference between the two characters is the number it represents.
  return character - '0';
}

void updateLedBar(int num) {
  num = constrain(num, 0, 16); // limit to 0 - 16
  num--;
  unsigned int bytevalue = 0;

  if (num >= 0) {
    for (int i = 0; i <= num; i++) {
      // bit an Stelle i auf 1 setzen
      bytevalue |= (1 << i);
    }
  }
  writeToLedBar(bytevalue);
}

void writeToLedBar(unsigned int bytevalue) {
  // gibt jeweils ein byte an das Schieberegister aus
  shiftOut(dataPin, clockPin, LSBFIRST, bytevalue); // nimmt nur die niedrigsten 8 bit
  shiftOut(dataPin, clockPin, LSBFIRST, bytevalue >> 8); // Verschiebung um 8 bit um die zweite hälfte darzustellen
}
