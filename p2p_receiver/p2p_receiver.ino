/*
  Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/receiver.ino, which corresponds to the code published
  in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks by Pradeeka Seneviratne (Apress, 2019)".

  The example source code was based on the original code published by Adafruit availabe at https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/rfm9x-test
  RadioHead Packet Radio library for embedded microprocessors - documentation: http://www.airspayce.com/mikem/arduino/RadioHead/

  tempChar pixel icon by https://www.hackster.io/jacoslabbert99/arduino-lcd-icons-custom-characters-548f38
*/
#include <SPI.h>
#include <RH_RF95.h>
#include <LiquidCrystal_I2C.h>

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED bar
int virtualVCC = 5;
int clockPin = 4;
int dataPin = 3;

int packageID = 0;
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

float lastTemp = 0.0;
long lastTempTime = 0;
  int lastTempLen = 0;
int lastPage = 0;

byte rawOutputChar[8] = {
  B01000,
  B01100,
  B01110,
  B01111,
  B01110,
  B01100,
  B01000,
  B00000
};

byte tempChar1[8] = {
  B00000,
  B00001,
  B00010,
  B00100,
  B00100,
  B00100,
  B00100,
  B00111,
};
byte tempChar2[8] = {
  B00111,
  B00111,
  B00111,
  B01111,
  B11111,
  B11111,
  B01111,
  B00011,
};
byte tempChar3[8] = {
  B00000,
  B10000,
  B01011,
  B00100,
  B00111,
  B00100,
  B00111,
  B11100,
};
byte tempChar4[8] = {
  B11111,
  B11100,
  B11100,
  B11110,
  B11111,
  B11111,
  B11110,
  B11000,
};

byte tempUnitChar[] = {
  B11100,
  B10100,
  B11100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, rawOutputChar);
  lcd.createChar(1,tempChar1);
  lcd.createChar(2,tempChar2);
  lcd.createChar(3,tempChar3);
  lcd.createChar(4,tempChar4);
  lcd.createChar(5,tempUnitChar);

  lcd.setCursor(0, 0);
  lcd.print("LoRaReceiver2021");
  lcd.setCursor(0, 1);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(virtualVCC, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // virtuelle Spannungsversorgung über pin 5
  digitalWrite(virtualVCC, HIGH);

  while (!Serial)
    ;
  Serial.begin(9600);
  delay(100);

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("Initializing failed");
    while (1)
      ;
  }
  Serial.println("Initialisation succeeded");


  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("Can't set the specifide frequency");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  Serial.println("----------------------------");

  rf95.setTxPower(23, false);

  lcd.print("by Florian Meyer");
  delay(1000);
}

void loop() {
  if (rf95.available()) {
    // read the availabe message
    if (rf95.recv(buf, &len)) {
      // print the message
      Serial.println((char*)buf);

      packageID++;

      parseTempValueFromRaw(buf);
      lastTempTime = millis();
      lastPage = -1; // force refresh

      updateLedBar(map(lastTemp * 10, 0, 400, 0, 16));  // 0.0 - 40.0 °C umgerechnet auf Skala 0-16
    } else {
      //If receive failed
      Serial.println("Receive failed");
    }
  }

int page = lastTempTime ? ((millis() - lastTempTime) / 5000) % 2 : 0;

    if (page == 1 && lastPage != 1) {
      lastPage = 1;
          lcd.clear();
  lcd.setCursor(0,0);
  lcd.write(1);
  lcd.setCursor(0,1);
  lcd.write(2);
  lcd.setCursor(1,0);
  lcd.write(3);
  lcd.setCursor(1,1);
  lcd.write(4);
  lcd.setCursor(3,0);
  lcd.print("Thermometer");
  lcd.setCursor(8 - (lastTempLen+1) / 2, 1);
  lcd.print(lastTemp);
  lcd.write(5);
  lcd.print("C");
    }

if (page == 0 && lastPage != 0) {
  lastPage = 0;
          lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ID#");
      lcd.print(packageID);

      lcd.setCursor(8, 0);
      lcd.print("RSSI");
      lcd.print(rf95.lastRssi());

      lcd.setCursor(0, 1);
      lcd.write(0);
      lcd.print("\"");
      lcd.print((char*)buf);
      lcd.print("\"");
    }



}

void parseTempValueFromRaw(char* raw) {
  // strings in C are guaranteed to be null-terminated
  bool started = false;
  bool isNegative = false;
  bool decimalPointReached = false;
  float decimalPositionFactor = 1.0;

  float value = 0.0;
  lastTempLen = 0;

  // data format: "d1|24.06|433"
  for (int i = 0; raw[i] != '\0'; i++) {
    if (raw[i] == '|') {
      started = !started;
      if (!started) {
        break;  // we are all done and can leave the loop...
      }
    } else if (started) {
      lastTempLen++;
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
  lastTemp = isNegative ? value * -1 : value;
}

int charToInt(char character) {
  // ascii values of the characters are subtracted from each other.
  // Since 0 comes directly before 1 in the ascii table (and so on until 9), the difference between the two characters is the number it represents.
  return character - '0';
}

void updateLedBar(int num) {
  num = constrain(num, 0, 16);  // limit to 0 - 16
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
  shiftOut(dataPin, clockPin, LSBFIRST, bytevalue);       // nimmt nur die niedrigsten 8 bit
  shiftOut(dataPin, clockPin, LSBFIRST, bytevalue >> 8);  // Verschiebung um 8 bit um die zweite hälfte darzustellen
}
