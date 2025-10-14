#include "lcdscreen.h"

LCDScreen::LCDScreen(Settings const &s):
  settings(s),
  lcd(LINE_SIZE, VISIBLE_LINES)
{}

void LCDScreen::begin(char const *msg) {
  lcd.begin();

  char line[LINE_SIZE + 1];
  uint8_t const msgLen = strlen(msg);
  uint8_t const padLeft = (LINE_SIZE - msgLen) / 2;
  uint8_t const padRight = LINE_SIZE - msgLen - padLeft;
  for (uint8_t i = 0; i != padLeft;  ++i) line[i] = ' ';
  for (uint8_t i = 0; i != msgLen;   ++i) line[padLeft + i] = msg[i];
  for (uint8_t i = 0; i != padRight; ++i) line[padLeft + msgLen + i] = ' ';
  line[LINE_SIZE] = 0;

  if (msg) displayTemp(TEMP_MESSAGE_TIMEOUT, line);
}

void LCDScreen::display(LCDBuffer::View const &view, bool forced) {
  static bool tempDisplayActive = false;
  static size_t lastViewID = 0;

  if (tempTimeout && (millis() < tempTimeout)) {
    tempDisplayActive = true;
    return;
  }

  if (tempDisplayActive) {
    // temp display was active but has now has timed out (or was cancelled) -> reset and force display update
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
    enableCursor();
  }
  else {
    // Cursor not in view -> turn off
    disableCursor();
  }
}

void LCDScreen::displayTemp(char const *lines[], uint8_t const n, size_t const timeout)
{
  static char emptyLine[LINE_SIZE + 1];
  static bool initialized = false;
  if (!initialized) {
    memset(emptyLine, ' ', LINE_SIZE);
    emptyLine[LINE_SIZE] = 0;
    initialized = true;
  } 

  for (uint8_t idx = 0; idx != VISIBLE_LINES; ++idx) {
    lcd.setCursor(0, idx);
    lcd.print(idx < n ? lines[idx] : emptyLine);
  }
  disableCursor();
  tempTimeout = millis() + timeout;
}

void LCDScreen::abortTemp() {
  tempTimeout = 0;
}

void LCDScreen::displayFrequency() {
  char floatBuffer[LINE_SIZE + 1];
  char stringBuffer[LINE_SIZE + 1];
  
  double kHz = measureFrequency();
  dtostrf(kHz, LINE_SIZE - 4, FREQUENCY_DISPLAY_PRECISION, floatBuffer);
  snprintf(stringBuffer, LINE_SIZE + 1, "%s kHz", floatBuffer);
  displayTemp(FREQUENCY_TIMEOUT, "Clock Frequency", stringBuffer);
}

void LCDScreen::enableCursor() {
  lcd.cursor();
  lcd.blink();
}

void LCDScreen::disableCursor() {
  lcd.noCursor();
  lcd.noBlink();
}








