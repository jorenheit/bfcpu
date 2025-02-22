#pragma once

#include "settings.h"

#if USE_FAST_LIQUIDCRYSTAL_LIBRARY
#include "LiquidCrystal_74HC595.h"
#define LiquidCrystalSerial LiquidCrystal_74HC595
#else
#include <LiquidCrystalSerial.h>
#endif

#define RING_BUF_CAPACITY 256 // Do not change

class LCDBuffer {
  LiquidCrystalSerial lcd;
  
  char ringBuf[RING_BUF_CAPACITY];
  volatile uint8_t ringBufHead = 0;
  uint8_t ringBufTail = 0;

  char screenBuf[TOTAL_LINES][LINE_SIZE + 1];
  uint8_t pos = 0;
  uint8_t currentLine = 0;
  uint8_t topLine = 0;
  uint8_t bottomLine = TOTAL_LINES - 1;
  uint8_t topVisibleLine = 0;
  uint8_t bottomVisibleLine = VISIBLE_LINES - 1;
  
  bool changed = false;

  static_assert(TOTAL_LINES <= 128, "TOTAL_LINES cannot exceed 128.");
  static_assert(RING_BUF_CAPACITY == 256, "RING_BUF_CAPACITY should be 256 bytes.");
  static_assert(sizeof(ringBufHead) == 1, "sizeof(ringBufHead) should be 1 byte.");
  static_assert(sizeof(ringBufTail) == 1, "sizeof(ringBufTail) should be 1 byte.");

public:
  LCDBuffer();

  void begin(char const *msg = "");
  void push(byte const c);
  void push(char const *str);
  bool send(DisplayMode const mode, bool const forced = false);
  void scrollDown(uint8_t n = 1);
  void scrollUp(uint8_t n = 1);
  void reset();
  
private:
  void insertAsAscii(byte const c);
  void insertAsHex(byte const c);
  void insertAsDecimal(byte const c);
  void update(DisplayMode const mode);
  void clearLine(uint8_t const idx);
  void bringIntoView();
  void enter();
  void tab();
  void put(char const c);
  void newLine();

  uint8_t normalize(uint8_t line) const;
};
