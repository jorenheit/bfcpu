#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "keyboardbuffer.h"
#include "lcdmenu.h"
#include "button.h"
#include "ringbuffer.h"

LCDBuffer lcdBuffer;
KeyboardBuffer kbBuffer;
LCDScreen lcdScreen;
LCDMenu lcdMenu(lcdBuffer, lcdScreen);

Button<SCROLL_UP_PIN>   scrollUpButton;
Button<SCROLL_DOWN_PIN> scrollDownButton;
ButtonPair<PeekState> menuButton(scrollUpButton, scrollDownButton);

void setup() {
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);
  setIOPinsToInput();
  
  scrollUpButton.begin();
  scrollDownButton.begin();
  kbBuffer.begin();
  lcdBuffer.begin();
  lcdMenu.begin();
  lcdScreen.begin("READY!");

  attachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN), onSystemClock, RISING);
}

void loop() {
  handleButtons();
  kbBuffer.update();
  lcdBuffer.update();
  lcdScreen.display(lcdBuffer.view());
}

void handleButtons() {
  ButtonState const scrollUpState = scrollUpButton.state();
  ButtonState const scrollDownState = scrollDownButton.state();
  ButtonState const menuButtonState = menuButton.state();

  // Check if menu active. If so, forward button states to menu to handle
  if (lcdMenu.active()) {
    lcdMenu.handleButtons(scrollUpState, scrollDownState, menuButtonState);
    return;
  }
  
  if (menuButtonState == ButtonState::Rising) {
    lcdMenu.enter();
    return;
  }

  unsigned long currentTime = millis();
  static unsigned long previousScrollTime = 0;
  bool const scrollingAllowed = (currentTime - previousScrollTime) > NO_SCROLL_DELAY;

  if (scrollingAllowed && scrollUpState == ButtonState::High) {
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
    lcdBuffer.enqueue(readByteFromBus());
    return;
  }

  if (kb_state != IDLE || digitalRead<KEYBOARD_ENABLE_PIN>()) {
    switch (kb_state) {
      case IDLE: {
        setIOPinsToOutput();
        byte const val = kbBuffer.get();
        writeByteToBus(val);
        lcdBuffer.enqueueEcho(val);
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



