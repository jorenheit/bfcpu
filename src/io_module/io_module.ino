#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "keyboard.h"
#include "button.h"

LCDBuffer lcdBuffer;
Keyboard keyboard;
Button<SCROLL_UP_PIN>   scrollUpButton;
Button<SCROLL_DOWN_PIN> scrollDownButton;
ButtonPair<PeekState> modeChangeButton(scrollUpButton, scrollDownButton);

void setup() {
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);
  setIOPinsToInput();
  
  scrollUpButton.begin();
  scrollDownButton.begin();
  keyboard.begin();
  lcdBuffer.begin("READY!");

  attachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN), onSystemClock, RISING);
}

void loop() {
  handleButtons();
  lcdBuffer.send();
}

void handleButtons() {
  ButtonState const scrollUpState = scrollUpButton.state();
  ButtonState const scrollDownState = scrollDownButton.state();
  ButtonState const modeChangeState = modeChangeButton.state();

  unsigned long currentTime = millis();
  static unsigned long previousScrollTime = 0;
  bool const scrollingAllowed = (currentTime - previousScrollTime) > NO_SCROLL_DELAY;
  
  static unsigned long holdToClearStartTime = 0;
  static bool clearingAllowed = false;

  if (modeChangeState == ButtonState::Rising) {
    lcdBuffer.nextMode(true);
    holdToClearStartTime = currentTime;
    clearingAllowed = true;
  }
  else if (modeChangeState == ButtonState::High) {
    if (clearingAllowed && (currentTime - holdToClearStartTime) > HOLD_TO_CLEAR_TIME) {
      lcdBuffer.clear("CLEAR!");
      lcdBuffer.previousMode(false);
      clearingAllowed = false;
    }
  }
  else if (scrollingAllowed && scrollUpState == ButtonState::High) {
    lcdBuffer.scrollUp();
    previousScrollTime = currentTime;
  }
  else if (scrollingAllowed && scrollDownState == ButtonState::High) {
    lcdBuffer.scrollDown();
    previousScrollTime = currentTime;
  }
}

void onSystemClock() {  
  enum KeyboardState: uint8_t {
    IDLE,
    WAIT,
    RESET
  };
  static volatile KeyboardState kb_state = IDLE;

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



