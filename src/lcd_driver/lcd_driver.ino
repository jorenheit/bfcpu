#include "lcdbuffer.h"
#include "settings.h"

LCDBuffer lcdBuffer;

void setup() {
  pinMode(CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(WRITE_ENABLE_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onClock, RISING);

  for (int i = 0; i != 8; ++i) {
    pinMode(DATA_PINS[i], INPUT);
  }
  
  pinMode(DISPLAY_MODE_PIN, INPUT);
  pinMode(SCROLL_UP_PIN, INPUT);
  pinMode(SCROLL_DOWN_PIN, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  handleButtons();
  update();
}

void update() {
  static unsigned long previous = millis();
  unsigned long current = millis();
  if (current - previous > REFRESH_DELAY) {
    previous = current;
    if (digitalRead(DISPLAY_MODE_PIN) == 0)
      lcdBuffer.setMode(HEXADECIMAL);
    else
      lcdBuffer.setMode(ASCII);

    lcdBuffer.send();
  }
}

void onClock() {
  if (digitalRead(WRITE_ENABLE_PIN)) {
    byte c = 0;
    for (int i = 0; i != 8; ++i) {
      c |= (digitalRead(DATA_PINS[i]) << i); 
    }
    lcdBuffer.push(c);
  }
}

void handleButtons() {
  if (digitalRead(SCROLL_UP_PIN)) {
    lcdBuffer.scrollUp();
    delay(BUTTON_DEBOUNCE_DELAY);
  }
  else if (digitalRead(SCROLL_DOWN_PIN)) {
    lcdBuffer.scrollDown();
    delay(BUTTON_DEBOUNCE_DELAY);
  }

  // TODO: clear when both buttons pushed at the same time
  // CLEAR_HOLD_TIME was already defined but not used yet.
}
