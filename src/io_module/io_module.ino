#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "keyboardbuffer.h"
#include "button.h"
#include "lcdmenu.h"
#include "random.h"
//#include <PinChangeInterrupt.h>

// Global objects
Settings settings;
KeyboardBuffer kbBuffer(settings);
LCDBuffer lcdBuffer(settings);
LCDScreen lcdScreen(settings);
LCDMenu lcdMenu(lcdScreen, settings, lcdBuffer, kbBuffer);
Random rng;

// Pointer to RNG seed value needed to update from menu
int *rngSeedPtr = &settings.rngSeed;

// Buttons
auto scrollUpButton = Button::create<SCROLL_UP_PIN>();
auto scrollDownButton = Button::create<SCROLL_DOWN_PIN>();
auto menuButton = ButtonPair::create(scrollUpButton, scrollDownButton);

void setup() {
  // Setup for the pins
  pinMode(SYSTEM_CLOCK_INTERRUPT_PIN, INPUT);
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);
  pinMode(K_OUT_PIN, OUTPUT);
  pinMode(K_IN_PIN, INPUT);
  digitalWrite<K_OUT_PIN, LOW>();
  //attachPCINT(digitalPinToPCINT(K_IN_PIN), setKReceivedFlag, FALLING);
  setIOPinsToInput();

  // Initialize buttons
  scrollUpButton.begin();
  scrollDownButton.begin();
  menuButton.begin();

  // Initialize buffers
  kbBuffer.begin();
  lcdBuffer.begin();

  // Load settings from EEPROM
  lcdMenu.loadSettings();

  // Initialize RNG using loaded seed value
  rng.begin(settings.rngSeed);

  // Initialize screen
  lcdScreen.begin();

  // Block until the handshake has been completed
  handshake();

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
  if (menuButton.isHold()) return lcdScreen.displayFrequency();

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

void echo(char const c) {
  lcdBuffer.enqueue(c);
}

void setK() {
  digitalWrite<K_OUT_PIN, HIGH>();
  __asm__ __volatile__("nop\n\t");
  digitalWrite<K_OUT_PIN, LOW>();
}

void handshake() {
  delay(HANDSHAKE_STARTUP_DELAY_MILLIS);
  setK();
  while (digitalRead<K_IN_PIN>()) {}
}

volatile size_t tickCount = 0;
//----------isr_begin----------
void onSystemClock() {
  ++tickCount;

  static enum : uint8_t {
    IDLE,
    WAIT_SYS,
    WAIT_KB
  } state = IDLE;

  auto const writeByteToBusAndWait = [](uint8_t const value, uint8_t const newState) -> void [[always_inline]] {
    setIOPinsToOutput();
    writeByteToBus(value);
    setK();
    state = static_cast<decltype(state)>(newState);
  };

  auto const enqueueByteFromBusAndWait = [](uint8_t const newState) -> void [[always_inline]] {
    lcdBuffer.enqueue(readByteFromBus());
    setK();
    state = static_cast<decltype(state)>(newState);
  };

  if (state == WAIT_SYS) {
    if (!digitalRead<K_IN_PIN>()) {
      setIOPinsToInput();
      state = IDLE;
    }
    return;
  } else if (state == WAIT_KB) {
    if (kbBuffer.available()) {
      writeByteToBusAndWait(kbBuffer.get(), WAIT_SYS);
    }
    return;
  }

  bool const EN_OUT = digitalRead<DISPLAY_ENABLE_PIN>();
  bool const EN_IN = digitalRead<KEYBOARD_ENABLE_PIN>();

  switch (EN_IN << 1 | EN_OUT) {
    case 0b01: {
      return enqueueByteFromBusAndWait(WAIT_SYS);
    }
    case 0b10: {
      if (settings.inputMode == IMMEDIATE || kbBuffer.available()) {
        writeByteToBusAndWait(kbBuffer.get(), WAIT_SYS);
      } else {
        state = WAIT_KB;
      }
      return;
    }
    case 0b11: {
      return writeByteToBusAndWait(rng.get(), WAIT_SYS);
    }
    default: return; /* UNREACHABLE */
  }
}
//----------isr_end----------
