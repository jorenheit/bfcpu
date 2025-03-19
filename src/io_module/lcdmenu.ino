#include "lcdmenu.h"
#include <EEPROM.h>

LCDMenu::LCDMenu(LCDBuffer &buf, LCDScreen &scr):
  _buffer(buf), 
  _screen(scr),
  _actions(buf)
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
    LCDBuffer::Settings const oldSettings = _buffer.getSettings();
    Menu::Pointer next = _current->highlighted()->select(_actions);
    if (_buffer.getSettings() != oldSettings) saveSettings();
    if (next == _menu.exit()) return exit();
    _current = next;
    display();
  }
  else if (!bothReleased && both == ButtonState::Released) {
    bothReleased = true;
  }
  else if (up == ButtonState::JustReleased) {
    _lastActiveTime = millis();
    _current->up();
    display();  
  }
  else if (down == ButtonState::JustReleased) {
    _lastActiveTime = millis();
    _current->down();
    display();  
  }
}

void LCDMenu::display(){
  _screen.displayTemp(MENU_TIMEOUT, true, _current->getLabel(), _current->highlighted()->getNumberedLabel());
}

void LCDMenu::exit() {
  _current = _menu.home();
  _screen.clear();
  _lastActiveTime = 0;
}


struct EEPROMSettings {
  LCDBuffer::Settings settings;
  uint8_t checksum;
  EEPROMSettings() = default;
  EEPROMSettings(LCDBuffer::Settings const &set):
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
    if (set.valid()) _buffer.setSettings(set.settings);
    else EEPROM.write(EEPROM_VALID_FLAG_ADDRESS, false);
  }
}

void LCDMenu::saveSettings() {
  EEPROMSettings const set = _buffer.getSettings();
  for (uint8_t idx = 0; idx != sizeof(set); ++idx) {
    EEPROM.update(EEPROM_SETTINGS_ADDRESS + idx, reinterpret_cast<byte const*>(&set)[idx]);
  }
  EEPROM.update(EEPROM_VALID_FLAG_ADDRESS, EEPROM_VALID_FLAG_VALUE);
}