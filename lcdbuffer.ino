#include "lcdbuffer.h"

LCDBuffer::LCDBuffer():
  lcd(SH_CP, DS, ST_CP)
{
  lcd.begin(CHARS, LINES);
  lcd.cursor();
  lcd.blink();
  buf[0].reserve(CHARS);
  buf[1].reserve(CHARS);
  clear(buf[0]);
  clear(buf[1]);
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
  lcd.print(buf[0]);
  lcd.setCursor(0, 1);
  lcd.print(buf[1]);
  lcd.setCursor(pos, line);
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
      pos += CHARS;
      break;
    }
    case '\t': {
      pos += TAB;
      break;
    }
    default: {
      buf[line][pos] = c;
      pos += 1;
      break;
    }
  }

  if (pos >= CHARS) {
    line += 1;
    pos = 0;
  }
  if (line > 1) {
    line = 1;
    buf[0] = buf[1];
    clear(buf[1]);
  }
}

void LCDBuffer::pushNumber(byte const c, char const separator) {
  String s(c, mode);
  for (int i = 0; i != s.length(); ++i)
    pushAscii(s[i]);
  pushAscii(separator);
}

void LCDBuffer::clear(String &buf, char const fill) {
  buf = "";
  for (int i = 0; i != CHARS; ++i) {
    buf += fill;
  }
}

