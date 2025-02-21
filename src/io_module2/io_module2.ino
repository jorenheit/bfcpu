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
  static byte kb_cycle = 0; 
  
  if (kb_cycle == 0 && digitalRead<DISPLAY_ENABLE_PIN>()) {
    lcdBuffer.push(readByteFromBus());
    return;
  }

  if (kb_cycle != 0 || digitalRead<KEYBOARD_ENABLE_PIN>()) {
    switch (kb_cycle) {
      case 0: {
        // Prepare outputs
        setIOPinsToOutput();
        writeByteToBus(keyboard.get());
        ++kb_cycle;
        return;
      }
      case 1: {
        // Data is being read, do nothing
        ++kb_cycle;
        return;
      }
      case 2: {
        // Reset
        setIOPinsToInput();
        kb_cycle = 0;
        return;
      }
    }
  }
}


