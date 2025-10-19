#include "lcdbuffer.h"

void LCDBuffer::begin() {
  clear();
}

void LCDBuffer::enqueue(byte const c) {
  ringBuf.put(c);
}

void LCDBuffer::update() {
  // Select the insertion function
  using InsertFuncPtr = void (LCDBuffer::*)(byte const);
  InsertFuncPtr const insert = 
    (settings.displayMode == ASCII)    ? &LCDBuffer::insertAsAscii   :
    (settings.displayMode == DECIMAL)  ? &LCDBuffer::insertAsDecimal
                                       : &LCDBuffer::insertAsHex;

  // Copy new data from ringBuf into the screenBuf using the selected insert function.
  auto bytesAvailable = ringBuf.available();
  for (decltype(bytesAvailable) i = 0; i != bytesAvailable; ++i) {
    (this->*insert)(ringBuf.get().value);
  }
  if (bytesAvailable && settings.autoscrollEnabled) bringIntoView();
}

LCDBuffer::View LCDBuffer::view() const {
  static size_t count = 0;

  View view;
  for (uint8_t line = 0; line != VISIBLE_LINES; ++line) {
    view.lines[line] = screenBuf[(topVisibleLine + line) % TOTAL_LINES];
  }
  view.cursorRow = normalize(currentLine) - normalize(topVisibleLine); 
  view.cursorCol = pos;
  view.id = changed ? ++count : count;

  changed = false;
  return view;
}

void LCDBuffer::insertAsHex(byte const c) {
  static char const hexChars[] = "0123456789ABCDEF";
  insertAsAscii(hexChars[(c >> 4) & 0x0f]); // high nibble
  insertAsAscii(hexChars[(c >> 0) & 0x0f]); // low nibble 
  insertAsAscii(settings.delimiter);
}

void LCDBuffer::insertAsDecimal(byte const c) {
  insertAsAscii('0' + (c / 100));
  insertAsAscii('0' + (c % 100) / 10);
  insertAsAscii('0' + (c % 10));
  insertAsAscii(settings.delimiter);
}

void LCDBuffer::insertAsAscii(byte const c) {
  changed = true;
  switch (c) {
    case '\n': {
      enter();
      return;
    }
    case '\t': {
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
  n = min(normalize(bottomLine) - normalize(bottomVisibleLine), n);
  topVisibleLine = (topVisibleLine + n) % TOTAL_LINES;
  bottomVisibleLine = (bottomVisibleLine + n) % TOTAL_LINES;
  changed = true;
}

void LCDBuffer::scrollUp(uint8_t n) {
  n = min(normalize(topVisibleLine) - normalize(topLine), n);
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

void LCDBuffer::clear() {
  for (uint8_t i = 0; i != TOTAL_LINES; ++i) 
    clearLine(i);

  currentLine = 0;
  topVisibleLine = 0;
  bottomVisibleLine = VISIBLE_LINES - 1;
  topLine = 0;
  bottomLine = TOTAL_LINES - 1;
  pos = 0;
  ringBuf.clear();
}

uint8_t LCDBuffer::normalize(uint8_t const line) const {
  // returns a line number between 0 and TOTAL_LINES - 1 as offset with repect to the top line.
  return (line - topLine + TOTAL_LINES) % TOTAL_LINES;
}
