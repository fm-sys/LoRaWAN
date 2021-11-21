/*
   Test sketch for 74HC164

   https://www.instructables.com/The-74HC164-Shift-Register-and-your-Arduino/
   https://www.arduino.cc/reference/de/language/functions/advanced-io/shiftout/
*/

int virtualVCC = 5;
int clockPin = 4;
int dataPin = 3;

void setup() {
  pinMode(virtualVCC, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // virtuelle Spannungsversorgung über pin 5
  digitalWrite(virtualVCC, HIGH);


  for (int i = 0; i <= 16; i++) {
    updateLedBar(i);
    delay(500);
  }
}

void loop() {
  for (int i = 0; i < 256; i++) { // wenn BYTE genutzt wird, kann die for-Schleife niemals enden, da nach 255 wieder die 0 kommt.
    for (int j = 0; j < 256; j++) {
      shiftOut(dataPin, clockPin, LSBFIRST, j); // gibt jeweils ein byte aus
      shiftOut(dataPin, clockPin, LSBFIRST, i);
      delay(100);
    }
  }
}

void updateLedBar(int num) { /* 0 - 16 */
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
