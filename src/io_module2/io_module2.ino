#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "keyboard.h"
#include "button.h"

LCDBuffer lcdBuffer;
Keyboard keyboard;
Button<SCROLL_UP_PIN>   scrollUpButton;
Button<SCROLL_DOWN_PIN> scrollDownButton;

void setup() {
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  setIOPinsToInput();
  
  pinMode(DISPLAY_MODE_PIN, INPUT);
  pinMode(SCROLL_UP_PIN, INPUT);
  pinMode(SCROLL_DOWN_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);

  keyboard.begin();
  lcdBuffer.begin("READY!\n");

  attachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN), onSystemClock, RISING);
}

void loop() {
  handleButtons();
  updateDisplay();
}

void updateDisplay() {
  static unsigned long lastUpdateTime = millis();
  unsigned long const currentTime = millis();

  DisplayMode const mode = digitalRead<DISPLAY_MODE_PIN>() 
      ? ASCII 
      : static_cast<DisplayMode>(NUMBER_MODE);

  bool const forced = (currentTime - lastUpdateTime) > FORCED_UPDATE_INTERVAL;
  if (lcdBuffer.send(mode, forced)) {
    lastUpdateTime = millis();
  }
}

void handleButtons() {
  if (scrollUpButton.pressed())   lcdBuffer.scrollUp();
  if (scrollDownButton.pressed()) lcdBuffer.scrollDown();
}

void onSystemClock() {
  enum KeyboardState {
    IDLE,
    WAIT,
    RESET
  };

  static KeyboardState kb_state = IDLE;
  
  if (kb_state == IDLE && digitalRead<DISPLAY_ENABLE_PIN>()) {
    lcdBuffer.push(readByteFromBus());
    return;
  }

  if (kb_state != IDLE || digitalRead<KEYBOARD_ENABLE_PIN>()) {
    switch (kb_state) {
      case IDLE: {
        setIOPinsToOutput();
        writeByteToBus(keyboard.get());
        kb_state = WAIT;
        return;
      }
      case WAIT: {
        kb_state = RESET;
        return;
      }
      case RESET: {
        setIOPinsToInput();
        kb_state = IDLE;
        return;
      }
    }
  }
}


