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

auto scrollUpButton   = Button::create<SCROLL_UP_PIN>();
auto scrollDownButton = Button::create<SCROLL_DOWN_PIN>();
auto menuButton       = ButtonPair::create(scrollUpButton, scrollDownButton);

void setup() {
  // Setup for the pins
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);
  setIOPinsToInput();

  // Initialize buttons
  scrollUpButton.begin();
  scrollDownButton.begin();
  menuButton.begin();

  // Initialize buffers
  kbBuffer.begin();
  lcdBuffer.begin();
  
  // Load settings and initialize screen
  lcdMenu.loadSettings();
  lcdScreen.begin("READY!");

  // Start listening for incoming clocks
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
  // Update button-states
  scrollUpButton.update();
  scrollDownButton.update();
  menuButton.update();

  // If in the menu, forward button-states to the menu
  if (lcdMenu.active()) return lcdMenu.handleButtons(scrollUpButton.state(), 
                                                     scrollDownButton.state(), 
                                                     menuButton.state());
  
  if (menuButton.isJustPressed()) return lcdMenu.enter();
  if (menuButton.isHold())        return lcdScreen.displayFrequency();

  // No menu or frequency stuff going on -> buttons are used for scrolling the buffer
  scroll();
}

void scroll() {
  // assumes buttons have been updated recently
  unsigned long currentTime = millis();
  static unsigned long previousScrollTime = 0;
  bool const scrollingAllowed = (currentTime - previousScrollTime) > NO_SCROLL_DELAY;

  if (scrollingAllowed && scrollUpButton.isDown()) {
    lcdBuffer.scrollUp();
    previousScrollTime = currentTime;
  } else if (scrollingAllowed && scrollDownButton.isDown()) {
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
