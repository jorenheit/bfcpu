#pragma once

#include "settings.h"
#include "ringbuffer.h"

class LCDBuffer {
  RingBuffer<char, 256> ringBuf;
  char screenBuf[TOTAL_LINES][LINE_SIZE + 1];

  uint8_t pos = 0;
  uint8_t currentLine = 0;
  uint8_t topLine = 0;
  uint8_t bottomLine = TOTAL_LINES - 1;
  uint8_t topVisibleLine = 0;
  uint8_t bottomVisibleLine = VISIBLE_LINES - 1;
  
  mutable bool changed = false;
  DisplayMode mode = ASCII;
  bool autoScroll = true;
  bool echoEnabled = false;

  static_assert(TOTAL_LINES <= 256, "TOTAL_LINES cannot exceed 256.");

public:
  void begin();
  void update();
  void enqueue(byte const c);
  void enqueueEcho(byte const c);
  
  void scrollDown(uint8_t n = 1);
  void scrollUp(uint8_t n = 1);
  void clear();

  DisplayMode setMode(DisplayMode const mode);
  DisplayMode nextMode();
  DisplayMode previousMode();
  DisplayMode currentMode() const;

  void setAutoScroll(bool const val);
  void setEchoEnabled(bool const val);

  struct View {
    char const *lines[VISIBLE_LINES];
    uint8_t cursorRow;
    uint8_t cursorCol;
    size_t id;
  };

  View view() const;
  
private:
  void insertAsAscii(byte const c);
  void insertAsHex(byte const c);
  void insertAsDecimal(byte const c);
  void clearLine(uint8_t const idx);
  void bringIntoView();
  void enter();
  void tab();
  void put(char const c);
  void newLine();

  uint8_t normalize(uint8_t line) const;
};
