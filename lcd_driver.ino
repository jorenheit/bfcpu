#include "lcdbuffer.h"
#include "settings.h"

LCDBuffer lcdBuffer;

void setup() {
  /*
  pinMode(CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(WRITE_ENABLE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onClock, RISING);

  for (int i = 0; i != 8; ++i) {
    pinMode(DATA_PINS[i], INPUT);
  }
  */

  Serial.begin(9600);
  lcdBuffer.setMode(ASCII); // TODO: set with HW switch/wire
}

void loop() {
  getInput(); // will be replaced by the interrupt handler
  update();
}

void update() {
  static unsigned long previous = millis();
  unsigned long current = millis();
  if (current - previous > REFRESH_DELAY) {
    previous = current;
    lcdBuffer.send();
  }
}

void getInput() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n')
      return;

    lcdBuffer.push(c);
  }
}

/*
void onClock() {
  if (digitalRead(WRITE_ENABLE_PIN)) {
    byte c = 0;
    for (int i = 0; i != 8; ++i) {
      c |= (digitalRead(DATA_PINS[i]) << i); 
    }
    lcdBuffer.push(c);
  }
}
*/