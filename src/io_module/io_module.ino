#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "keyboardbuffer.h"
#include "button.h"
#include "lcdmenu.h"
#include "random.h"

// Global objects
Settings       settings;
KeyboardBuffer kbBuffer(settings);
LCDBuffer      lcdBuffer(settings);
LCDScreen      lcdScreen(settings);
LCDMenu        lcdMenu(lcdScreen, settings, lcdBuffer, kbBuffer);
Random         rng;

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

  // Initialize RNG
  rng.begin(123); // TODO: implement seed settings

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

void echo(char const c) {
  lcdBuffer.enqueue(c);
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

//----------isr_begin----------
void onSystemClock() {
  ++tickCount;

  static enum : uint8_t {
    IDLE, WAIT, RESET
  } bus_state = IDLE;

  if (bus_state != IDLE) {
    switch (bus_state) {
      case WAIT: {
        bus_state = RESET;
        return;
      }
      case RESET:
        setIOPinsToInput(); [[fallthrough]];
      default: {
        bus_state = IDLE;
        return;
      }
    }
  }

  #define WRITE_TO_BUS_AND_WAIT(X) { \
    setIOPinsToOutput();             \
    writeByteToBus(X);               \
    bus_state = WAIT;                \
    return;                          \
  }

  #define DISPLAY_BYTE_FROM_BUS() {       \
    lcdBuffer.enqueue(readByteFromBus()); \
    return;                               \
  }

  bool const EN_OUT = digitalRead<DISPLAY_ENABLE_PIN>();
  bool const EN_IN  = digitalRead<KEYBOARD_ENABLE_PIN>();

  switch (EN_IN << 1 | EN_OUT) {
    case 0b01: DISPLAY_BYTE_FROM_BUS();
    case 0b10: WRITE_TO_BUS_AND_WAIT(kbBuffer.get());
    case 0b11: WRITE_TO_BUS_AND_WAIT(rng.get());
    default: return;
  }
}
//----------isr_end----------

