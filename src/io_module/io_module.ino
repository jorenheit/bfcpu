#include "lcdbuffer.h"
#include "keyboard.h"
#include "common.h"
#include "settings.h"

LCDBuffer lcdBuffer;
Keyboard keyboard;

volatile bool update_display = false;

void setup() {
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  setIOPinsTo(INPUT);
  
  pinMode(DISPLAY_MODE_PIN, INPUT);
  pinMode(SCROLL_UP_PIN, INPUT);
  pinMode(SCROLL_DOWN_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN), onSystemClock, RISING);
  keyboard.begin();
}

void loop() {
  if (!keyboard.enabled) {
    handleButtons();
    updateDisplay();
  }
  handleKeyboard();
}

void updateDisplay() {
  if (!update_display) return;
  lcdBuffer.setMode(digitalRead(DISPLAY_MODE_PIN) ? ASCII : HEXADECIMAL);
  lcdBuffer.send();
  update_display = false;
}

void onSystemClock() {
  if (digitalRead(DISPLAY_ENABLE_PIN)) {
    byte c = 0;
    for (int i = 0; i != 8; ++i) {
      c |= (digitalRead(DATA_PINS[i]) << i); 
    }
    lcdBuffer.push(c);
    update_display = true;
  }

  if (keyboard.enabled && !keyboard.ready) {
    // data was read before ready
    keyboard.interrupted = true;
  }
}

void handleButtons() {
  if (digitalRead(SCROLL_UP_PIN)) {
    lcdBuffer.scrollUp();
    delay(BUTTON_DEBOUNCE_DELAY);
    update_display = true;
  }
  else if (digitalRead(SCROLL_DOWN_PIN)) {
    lcdBuffer.scrollDown();
    delay(BUTTON_DEBOUNCE_DELAY);
    update_display = true;
  }
}

void handleKeyboard() {
  int pin_value = analogRead(KEYBOARD_ENABLE_PIN);
  if (!keyboard.enabled && pin_value > 512) {
    keyboard.enabled = true;
    setIOPinsTo(OUTPUT);
    keyboard.send();
  }
  else if (keyboard.enabled && pin_value <= 512) {
    keyboard.enabled = false;
    setIOPinsTo(INPUT);
  }
  else if (keyboard.enabled && !keyboard.ready) {
    keyboard.send();
  }  
}