#include "lcdbuffer.h"

LCDBuffer::LCDBuffer():
#if USE_FAST_LIQUIDCRYSTAL_LIBRARY
  lcd(LINE_SIZE, VISIBLE_LINES)
#else
  lcd(SH_CP, DS, ST_CP)
#endif
{}

void LCDBuffer::begin(char const *msg) {
#if USE_FAST_LIQUIDCRYSTAL_LIBRARY
  lcd.begin();
#else
  lcd.begin(LINE_SIZE, VISIBLE_LINES);
#endif
  lcd.cursor();
  lcd.blink();
  
  reset();
  push(msg);
  send(ASCII, true);
}

void LCDBuffer::push(byte const c) {
  ringBuf[ringBufHead++] = c;
}

void LCDBuffer::push(char const *str) {
  uint8_t idx = 0;
  while (str[idx] != 0) push(str[idx++]);
}

void LCDBuffer::update(DisplayMode const mode) {
  // Select the insertion function
  using InsertFuncPtr = void (LCDBuffer::*)(byte const);
  InsertFuncPtr const insert = 
    (mode == ASCII)       ? &LCDBuffer::insertAsAscii   :
    (mode == DECIMAL)     ? &LCDBuffer::insertAsDecimal
                          : &LCDBuffer::insertAsHex;

  // Update screen-buffer by pushing all new bytes from the ring-buffer into it. 
  uint8_t const head = ringBufHead; // local copy, ringBufHead might be changed by interrupt
  while (ringBufTail != head) (this->*insert)(ringBuf[ringBufTail++]);
  if (changed) bringIntoView();
}

bool LCDBuffer::send(DisplayMode const mode, bool const forced) {
  // Update screen-buffer
  update(mode); 
  if (!changed && !forced) return false;

  // Screen-buffer is now up-to-date -> send visible lines to screen
  for (uint8_t line = 0; line != VISIBLE_LINES; ++line) {
    lcd.setCursor(0, line);
    lcd.print(screenBuf[(topVisibleLine + line) % TOTAL_LINES]);
  }
  // Display cursor at the position where the next character will be printed (if in view).
  int8_t const offset = normalize(currentLine) - normalize(topVisibleLine);
  if (offset >= 0 && offset < VISIBLE_LINES) {
    lcd.setCursor(pos, offset);
    lcd.cursor();
  }
  else {
    // Cursor not in view -> turn off
    lcd.noCursor();
  }

  // Reset changed flag and return
  changed = false;
  return true;
}

void LCDBuffer::insertAsHex(byte const c) {
  static char const hexChars[] = "0123456789ABCDEF";
  insertAsAscii(hexChars[(c >> 4) & 0x0f]); // high nibble
  insertAsAscii(hexChars[(c >> 0) & 0x0f]); // low nibble 
  insertAsAscii(NUMBER_SEPARATOR);
}

void LCDBuffer::insertAsDecimal(byte const c) {
  insertAsAscii('0' + (c / 100));
  insertAsAscii('0' + (c % 100) / 10);
  insertAsAscii('0' + (c % 10));
  insertAsAscii(NUMBER_SEPARATOR);
}

void LCDBuffer::insertAsAscii(byte const c) {
  changed = true;
  switch (c) {
    case '\n':
    case PS2_ENTER: {
      enter();
      return;
    }
    case PS2_TAB: {
	    tab();
      return;
    }
    default: {
	    put(c);
      return;
    }
  }
}

void LCDBuffer::enter() {
  newLine();
  pos = 0;  
}

void LCDBuffer::tab() {
  pos += TAB_WIDTH;
  if (pos >= LINE_SIZE) {
    newLine();
    pos -= LINE_SIZE;
  }
}

void LCDBuffer::put(char const c) {
  screenBuf[currentLine][pos++] = c;
  if (pos >= LINE_SIZE) {
    newLine();
    pos = 0;
  }
}

void LCDBuffer::newLine() {
  currentLine = (currentLine + 1) % TOTAL_LINES;
  if (currentLine == bottomLine) {
    bottomLine = (bottomLine + 1) % TOTAL_LINES;
    topLine = (topLine + 1) % TOTAL_LINES;
    clearLine(bottomLine);
  }
}

void LCDBuffer::scrollDown(uint8_t n) {
  n = min(bottomLine - bottomVisibleLine, n);
  topVisibleLine = (topVisibleLine + n) % TOTAL_LINES;
  bottomVisibleLine = (bottomVisibleLine + n) % TOTAL_LINES;
  changed = true;
}

void LCDBuffer::scrollUp(uint8_t n) {
  n = min(topVisibleLine - topLine, n);
  topVisibleLine = (topVisibleLine - n + TOTAL_LINES) % TOTAL_LINES;  
  bottomVisibleLine = (bottomVisibleLine - n + TOTAL_LINES) % TOTAL_LINES;
  changed = true;
}

void LCDBuffer::bringIntoView() {
  uint8_t const current = normalize(currentLine);
  uint8_t const top     = normalize(topVisibleLine);
  uint8_t const bottom  = normalize(bottomVisibleLine);

  if (current < top) {
    scrollUp(top - current);
  }
  else if (current > bottom) {
    scrollDown(current - bottom);
  }
}

void LCDBuffer::clearLine(uint8_t const idx) {
  memset(screenBuf[idx], ' ', LINE_SIZE);
  screenBuf[idx][LINE_SIZE] = '\0';
  changed = true;
}

void LCDBuffer::reset() {
  for (uint8_t i = 0; i != TOTAL_LINES; ++i) 
    clearLine(i);

  currentLine = 0;
  topVisibleLine = 0;
  bottomVisibleLine = VISIBLE_LINES - 1;
  topLine = 0;
  bottomLine = TOTAL_LINES - 1;
  pos = 0;
}

uint8_t LCDBuffer::normalize(uint8_t const line) const {
  // returns a line number between 0 and TOTAL_LINES - 1 as offset with repect to the top line.
  return (line - topLine + TOTAL_LINES) % TOTAL_LINES;
}
