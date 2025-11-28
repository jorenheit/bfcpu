#include "settings.h"
#include "common.h"
#include "lcdbuffer.h"
#include "lcdscreen.h"
#include "keyboardbuffer.h"
#include "button.h"
#include "lcdmenu.h"
#include "random.h"
#include "timer1.h"

// Global objects
Settings settings;
KeyboardBuffer kbBuffer(settings);
LCDBuffer lcdBuffer(settings);
LCDScreen lcdScreen(settings);
LCDMenu lcdMenu(lcdScreen, settings, lcdBuffer, kbBuffer);
Random rng;

// Pointer to RNG seed value needed to update from menu
int *rngSeedPtr = &settings.rngSeed;
int *slotPtr = &settings.programSlot;

// Errors
namespace Error {
  volatile enum ErrorType : uint8_t { 
    NONE,
    REACHED_DEFAULT_1,
    REACHED_DEFAULT_2,
    REACHED_DEFAULT_3
  } errno = NONE;

  void assert(ErrorType err = Error::NONE) {
    if (Error::errno == err) return;
    lcdScreen.displayTemp(-1, "I/O Error: %d", Error::errno);
    while (true) {};
  }
}

// Buttons
auto scrollUpButton = Button::create<SCROLL_UP_PIN>();
auto scrollDownButton = Button::create<SCROLL_DOWN_PIN>();
auto menuButton = ButtonPair::create(scrollUpButton, scrollDownButton);

namespace ISR {
  void onTimer();
  void onKCleared();
}

void setup() {
  // Setup for the pins
  pinMode(DISPLAY_ENABLE_PIN, INPUT);
  pinMode(KEYBOARD_ENABLE_PIN, INPUT);
  pinMode(K_OUT_PIN, OUTPUT);
  pinMode(K_IN_PIN, INPUT);
  digitalWrite<K_OUT_PIN, LOW>();
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

  // Start ISR timer and attach K-flag interrupt
  attachInterrupt(digitalPinToInterrupt(K_IN_PIN), ISR::onKCleared, FALLING);
  Timer1::start<ISR_FREQUENCY, ISR::onTimer>();
}

void loop() {
  Error::assert();

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
  if (menuButton.isHold())        return /* no effect */;

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
  __asm__ __volatile__("nop\n\t");
}

void handshake() {
  delay(HANDSHAKE_STARTUP_DELAY_MILLIS); // TODO: check if necessary

  // Phase 1: wait for CPU to finish initialization (cannot claim the bus while it is still at INIT)
  setK();
  while(digitalRead<K_IN_PIN>()) {}

  // Phase 2: CPU has cleared K -> it is waiting for the program slot
  setIOPinsToOutput();
  writeByteToBus(settings.programSlot);
  setK();
  while(digitalRead<K_IN_PIN>()) {}

  // Phase 3: CPU has cleared K again -> done
  setIOPinsToInput();
}


//----------isr_begin----------
namespace ISR {
  void panic(Error::ErrorType err) {
    Timer1::stop();
    detachInterrupt(digitalPinToInterrupt(K_IN_PIN));
    Error::errno = err;
  }

  static volatile enum : uint8_t {
    IDLE,
    WAIT_FOR_KEYBOARD,
    WAIT_AFTER_WRITE,
    WAIT_AFTER_READ
  } state = IDLE;

  void onKCleared() {
    switch (state) {
      case IDLE: return;
      case WAIT_AFTER_WRITE: setIOPinsToInput(); break;
      case WAIT_AFTER_READ:  break;
      default: return panic(Error::REACHED_DEFAULT_1);
    }
    state = IDLE;
    Timer1::restart();
  }

  void writeByteToBusAndWait(uint8_t const value) {
    Timer1::stop();
    setIOPinsToOutput();
    writeByteToBus(value);
    setK();
    state = WAIT_AFTER_WRITE;
  };

  void enqueueByteFromBusAndWait() {
    Timer1::stop();
    lcdBuffer.enqueue(readByteFromBus());
    setK();
    state = WAIT_AFTER_READ;
  };

  void onTimer() {
    switch (state) {
      case IDLE: break;
      case WAIT_FOR_KEYBOARD: {
        if (kbBuffer.available()) {
          writeByteToBusAndWait(kbBuffer.get());
        }
        return;     
      }
      default: return panic(Error::REACHED_DEFAULT_2);
    }

    bool const EN_OUT = digitalRead<DISPLAY_ENABLE_PIN>();
    bool const EN_IN = digitalRead<KEYBOARD_ENABLE_PIN>();

    switch (EN_IN << 1 | EN_OUT) {
      case 0b00: return;
      case 0b01: return enqueueByteFromBusAndWait();
      case 0b10: {
        if (settings.inputMode == IMMEDIATE || kbBuffer.available()) {
          return writeByteToBusAndWait(kbBuffer.get());  
        } else {
          state = WAIT_FOR_KEYBOARD;
          return;
        }
      }
      case 0b11: return writeByteToBusAndWait(rng.get());
      default:   return panic(Error::REACHED_DEFAULT_3);
    }
  }
} // namespace ISR
//----------isr_end----------
