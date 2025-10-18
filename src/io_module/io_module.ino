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

// Pointer to RNG seed value needed to update from menu
int *rngSeedPtr = &settings.rngSeed;

// Buttons
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


// ----------- HANDSHAKE ROUTINE ---------------
volatile bool handshakeCompleted = false;

void handshake() {
  setIOPinsToOutput();
  writeByteToBus(HANDSHAKE_MAGIC_VALUE);
  attachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN), onSystemClockDuringHandshake, RISING);
  while (!handshakeCompleted) {}
  setIOPinsToInput();
}

void onSystemClockDuringHandshake() {
  bool const EN_IN = digitalRead<KEYBOARD_ENABLE_PIN>();
  bool const EN_OUT = digitalRead<DISPLAY_ENABLE_PIN>();
  if (EN_IN && EN_OUT) {
    handshakeCompleted = true;
    detachInterrupt(digitalPinToInterrupt(SYSTEM_CLOCK_INTERRUPT_PIN));
  }
}


//----------isr_begin----------
volatile size_t tickCount = 0;
void onSystemClock()  {
  ++tickCount;

  static enum : uint8_t {
    IDLE, WAIT, RESET
  } state = IDLE;

  if (state == WAIT) {
    state = RESET;
    return;
  }
  else if (state == RESET) {
    setIOPinsToInput();
    state = IDLE;
    return;
  }
 
  auto writeByteToBusAndWait = [](uint8_t value) -> void [[always_inline]] {
    setIOPinsToOutput();             
    writeByteToBus(value);               
    state = WAIT;                 
  };

  bool const EN_OUT = digitalRead<DISPLAY_ENABLE_PIN>();
  bool const EN_IN  = digitalRead<KEYBOARD_ENABLE_PIN>();

  switch (EN_IN << 1 | EN_OUT) {
    case 0b01: return lcdBuffer.enqueue(readByteFromBus());
    case 0b10: return writeByteToBusAndWait(kbBuffer.get());
    case 0b11: return writeByteToBusAndWait(rng.get());
    default:   return; /* UNREACHABLE */
  }
}
//----------isr_end----------

