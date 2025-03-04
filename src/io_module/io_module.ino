#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "keyboardbuffer.h"
#include "button.h"
#include "lcdmenu.h"

KeyboardBuffer kbBuffer;
LCDBuffer      lcdBuffer;
LCDScreen      lcdScreen;
LCDMenu        lcdMenu(lcdBuffer, lcdScreen);

Button<SCROLL_UP_PIN>   scrollUpButton;
Button<SCROLL_DOWN_PIN> scrollDownButton;
ButtonPair<PeekState>   menuButton(scrollUpButton, scrollDownButton);

void setup() {
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);
  setIOPinsToInput();

  scrollUpButton.begin();
  scrollDownButton.begin();
  kbBuffer.begin();
  lcdBuffer.begin();
  lcdMenu.loadSettings();
  lcdScreen.begin("READY!");

  attachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN), onSystemClock, RISING);
}

void loop() {
  // Process raw data in the first ringbuffer of the keyboard. This
  // data will become available as ASCII characters through kbBuffer.get().
  kbBuffer.update();

  // Process raw data in the LCD ringbuffer and put it into the screen-buffer.
  lcdBuffer.update();

  // Send the view (visible window of the screen-buffer) to the screen.
  lcdScreen.display(lcdBuffer.view());
  
  // Handle buttons for scrolling, accessing the menu or displaying the frequency.
  handleButtons();
}

void handleButtons() {
  ButtonState const scrollUpState   = scrollUpButton.state();
  ButtonState const scrollDownState = scrollDownButton.state();
  ButtonState const menuButtonState = menuButton.state();

  if (lcdMenu.active())                       return lcdMenu.handleButtons(scrollUpState, scrollDownState, menuButtonState);
  if (menuButtonState == ButtonState::Rising) return lcdMenu.enter();
  if (menuButtonState == ButtonState::Hold)   return lcdScreen.displayFrequency();

  // No menu or frequency stuff going on -> buttons are used for scrolling the buffer
  scroll(scrollUpState, scrollDownState);
}

void scroll(ButtonState const up, ButtonState const down) {
  unsigned long currentTime = millis();
  static unsigned long previousScrollTime = 0;
  bool const scrollingAllowed = (currentTime - previousScrollTime) > NO_SCROLL_DELAY;

  if (scrollingAllowed && up == ButtonState::High) {
    lcdBuffer.scrollUp();
    previousScrollTime = currentTime;
  } else if (scrollingAllowed && down == ButtonState::High) {
    lcdBuffer.scrollDown();
    previousScrollTime = currentTime;
  }
}

volatile size_t tickCount = 0;

void onSystemClock() {
  ++tickCount;

  enum KeyboardState : uint8_t {
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
      case IDLE:
        {
          setIOPinsToOutput();
          byte const val = kbBuffer.get();
          writeByteToBus(val);
          lcdBuffer.enqueueEcho(val);
          kb_state = WAIT;
          return;
        }
      case WAIT:
        {
          kb_state = RESET;
          return;
        }
      case RESET:
        {
          setIOPinsToInput();
          kb_state = IDLE;
          return;
        }
    }
  }
}
