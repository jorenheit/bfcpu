#include "lcdbuffer.h"

LCDBuffer::LCDBuffer():
  lcd(SH_CP, DS, ST_CP)
{
  lcd.begin(CHARS, VISIBLE_LINES);
  lcd.cursor();
  lcd.blink();
  clear();
}

void LCDBuffer::push(byte const c) {
  if (mode == ASCII)
    pushAscii(c);
  else
    pushNumber(c);

  changed = true;
}

void LCDBuffer::send() {
  if (not changed) 
    return;
    
  lcd.setCursor(0, 0);
  lcd.print(buf[topVisibleLine]);
  lcd.setCursor(0, 1);
  lcd.print(buf[topVisibleLine + 1]);
 
  if (line - topVisibleLine > 1 || line < topVisibleLine) {
    lcd.noCursor();
  }
  else {
    lcd.cursor();
    lcd.setCursor(pos, line - topVisibleLine);
  }
  changed = false;
}

void LCDBuffer::setMode(DisplayMode m) {
  mode = m;
}

DisplayMode LCDBuffer::getMode() const {
  return mode;
}

void LCDBuffer::pushAscii(byte const c) {
  switch (c) {
    case '\n': {
      ++line;
      pos = 0;
      break;
    }
    case '\t': {
      pos += TAB;
      break;
    }
    default: {
      buf[line][pos] = c;
      ++pos;
      break;
    }
  }

  if (pos >= CHARS) {
    ++line;
    pos = 0;
  }
  
  while (line >= TOTAL_LINES) {
    for (int i = 0; i != TOTAL_LINES - 1; ++i) {
      memcpy(&buf[i][0], &buf[i + 1][0], CHARS);
    }
    clearLine(TOTAL_LINES - 1);
    --line;
  }

  while (line > topVisibleLine + 1) scrollDown();
  while (line < topVisibleLine)     scrollUp();
}

void LCDBuffer::scroll(int amount) {
  topVisibleLine += amount;
  topVisibleLine = min(topVisibleLine, TOTAL_LINES - 2);
  topVisibleLine = max(topVisibleLine, 0);
  changed = true;
}

void LCDBuffer::scrollDown() {
  scroll(1);
}

void LCDBuffer::scrollUp() {
  scroll(-1);
}

void LCDBuffer::pushNumber(byte const c, char const separator) {
  String s(c, mode);
  for (int i = 0; i != s.length(); ++i)
    pushAscii(s[i]);
  pushAscii(separator);
}

void LCDBuffer::clearLine(int const idx, char const fill) {
  for (int i = 0; i != CHARS; ++i) {
    buf[idx][i] = fill;
  }
  buf[idx][CHARS] = 0;
}

void LCDBuffer::clear(char const fill) {
  for (int i = 0; i != TOTAL_LINES; ++i) {
    clearLine(i, fill);
  }
  line = 0;
  pos = 0;
  topVisibleLine = 0;
  changed = true;
}
