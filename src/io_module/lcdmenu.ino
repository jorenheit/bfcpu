#include "lcdmenu.h"
#include <EEPROM.h>

LCDMenu::LCDMenu(LCDScreen &scr, Settings &s, LCDBuffer &lcdBuf, KeyboardBuffer &kbBuf):
  _settings(s),
  _screen(scr),
  _actions(s, lcdBuf, kbBuf)
{
  _current = _menu.begin();
}

void LCDMenu::enter(){
  _lastActiveTime = millis();
  display();
}

bool LCDMenu::active() const{
  return (_lastActiveTime > 0 && (millis() - _lastActiveTime) < MENU_TIMEOUT);
}

void LCDMenu::handleButtons(ButtonState const up, ButtonState const down, ButtonState const both){
  if (_current == _menu.exit())  return exit();
  if (both == ButtonState::Hold) return exit();

  static bool bothReleased = true;
  if (bothReleased && both == ButtonState::JustPressed) {
    _lastActiveTime = millis();
    bothReleased = false;
    Settings const oldSettings = _settings;
    Menu::Pointer next = _current->highlighted()->select(_actions);
    if (_settings != oldSettings) saveSettings();
    if (next == _menu.exit()) return exit();
    _current = next;
    _selectedLine = 1;
    display();
  }
  else if (!bothReleased && both == ButtonState::Released) {
    bothReleased = true;
  }
  else if (up == ButtonState::JustReleased) {
    _lastActiveTime = millis();
    bool const wrap = _current->up();
    if (wrap) _selectedLine = min(_current->count(), VISIBLE_LINES - 1);
    else if (_selectedLine > 1) --_selectedLine;
    display();  
  }
  else if (down == ButtonState::JustReleased) {
    _lastActiveTime = millis();
    bool const wrap = _current->down();
    if (wrap) _selectedLine = 1;
    else if (_selectedLine < VISIBLE_LINES - 1) ++_selectedLine;
    display();  
  }
}

void LCDMenu::display(){
  static char buffer[VISIBLE_LINES][LINE_SIZE + 1];
  static char const *ptrs[VISIBLE_LINES];
  static bool initialized = false;
  if (!initialized) {
    for (uint8_t idx = 0; idx != VISIBLE_LINES; ++idx) {
      ptrs[idx] = buffer[idx];
    }
    initialized = true;
  }

  auto const copyToBuffer = [](char *dest, char const *src)  {
    while (*src) { *(dest++) = *(src++); } 
  };

  // Clear buffer
  for (uint8_t line = 0; line != VISIBLE_LINES; ++line) {
    memset(buffer[line], ' ', LINE_SIZE);
    buffer[line][LINE_SIZE] = 0;
  }
  
  // Fill buffer with menu contents
  if (VISIBLE_LINES == 1) {
    copyToBuffer(buffer[0], _current->highlighted()->getLabel());
  }
  else {
    static constexpr uint8_t OFFSET = 2;
    copyToBuffer(buffer[0], _current->getLabel());
    for (int8_t line = 1; line != VISIBLE_LINES; ++line) {
      Menu::Pointer item = _current->highlighted(line - _selectedLine);
      if (item) copyToBuffer(&buffer[line][OFFSET], item->getLabel());
    }
    buffer[_selectedLine][0] = '*';
  }

  _screen.displayTemp(ptrs, (uint8_t)VISIBLE_LINES, (size_t)MENU_TIMEOUT);
}

void LCDMenu::exit() {
  _current = _menu.home();
  _screen.abortTemp();
  _lastActiveTime = 0;
}


struct EEPROMSettings {
  Settings settings;
  uint8_t checksum;
  EEPROMSettings() = default;
  EEPROMSettings(Settings const &set):
    settings(set),
    checksum(computeChecksum())
  {}

  uint8_t computeChecksum() const {
    uint8_t cs = 0;
    for (uint8_t idx = 0; idx != sizeof(settings); ++idx) {
      cs += reinterpret_cast<uint8_t const*>(&settings)[idx];
    }
    return cs;
  }

  bool valid() const {
    return checksum == computeChecksum();
  }
 };


void LCDMenu::loadSettings() {
  uint8_t const validFlag = EEPROM.read(EEPROM_VALID_FLAG_ADDRESS);
  if (validFlag == EEPROM_VALID_FLAG_VALUE) {
    EEPROMSettings set = {};
    EEPROM.get(EEPROM_SETTINGS_ADDRESS, set);
    if (set.valid()) _settings = set.settings;
    else EEPROM.write(EEPROM_VALID_FLAG_ADDRESS, false);
  }
}

void LCDMenu::saveSettings() {
  EEPROMSettings const set = _settings;
  for (uint8_t idx = 0; idx != sizeof(set); ++idx) {
    EEPROM.update(EEPROM_SETTINGS_ADDRESS + idx, reinterpret_cast<byte const*>(&set)[idx]);
  }
  EEPROM.update(EEPROM_VALID_FLAG_ADDRESS, EEPROM_VALID_FLAG_VALUE);
}