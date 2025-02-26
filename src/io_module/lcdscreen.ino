#include "lcdscreen.h"

LCDScreen::LCDScreen():
  lcd(LINE_SIZE, VISIBLE_LINES)
{}

void LCDScreen::begin(char const *msg) {
  lcd.begin();
  lcd.cursor();
  lcd.blink();
  if (msg) displayTemp(TEMP_MESSAGE_TIMEOUT, msg);
}

void LCDScreen::display(LCDBuffer::View const &view, bool forced) {
  static bool tempDisplayActive = false;
  static size_t lastViewID = 0;

  if (tempTimeout && (millis() < tempTimeout))
  {
    tempDisplayActive = true;
    return;
  }

  if (tempDisplayActive) {
    // temp display was active but has now has timed out -> reset and force display update
    tempDisplayActive = false;
    tempTimeout = 0; 
    forced = true;
  }

  if (view.id == lastViewID && !forced) 
    return;

  lastViewID = view.id;
  for (uint8_t line = 0; line != VISIBLE_LINES; ++line) {
    lcd.setCursor(0, line);
    lcd.print(view.lines[line]);
  }

  // Display cursor at the position where the next character will be printed (if in view).
  if (view.cursorRow < VISIBLE_LINES) {
    lcd.setCursor(view.cursorCol, view.cursorRow);
    lcd.cursor(); 
  }
  else {
    // Cursor not in view -> turn off
    lcd.noCursor();
  }
}

void LCDScreen::displayTemp(char const *lines[], uint8_t const n, size_t const timeout)
{
  lcd.clear();
  for (uint8_t idx = 0; idx != min(n, static_cast<uint8_t>(VISIBLE_LINES)); ++idx) {
    lcd.setCursor(0, idx);
    lcd.print(lines[idx]);
  }
  lcd.noCursor();
  tempTimeout = millis() + timeout;
}
