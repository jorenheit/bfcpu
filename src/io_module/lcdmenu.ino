 #include "lcdmenu.h"
 
LCDMenu::LCDMenu(LCDBuffer &buf, LCDScreen &scr):
  _buffer(buf), 
  _screen(scr)
{}

void LCDMenu::enter(){
  _lastActiveTime = millis();
  _current = menu.getPointer();
  display();
}

bool LCDMenu::active() const{
  return (_lastActiveTime > 0 && (millis() - _lastActiveTime) < MENU_TIMEOUT);
}

void LCDMenu::handleButtons(ButtonState const up, ButtonState const down, ButtonState const both){
  static bool bothReleased = true;
  if (bothReleased && both == ButtonState::Rising) {
    _lastActiveTime = millis();
    bothReleased = false;
    MenuItem *next = _current->highlighted()->select(_buffer);
    if (next == nullptr) {
      _screen.clear();
      _lastActiveTime = 0;
      return;
    }
    _current = next;
    display();
  }
  else if (!bothReleased && both == ButtonState::Low) {
    bothReleased = true;
  }
  else if (up == ButtonState::Falling) {
    _lastActiveTime = millis();
    _current->up();
    display();  
  }
  else if (down == ButtonState::Falling) {
    _lastActiveTime = millis();
    _current->down();
    display();  
  }
}

void LCDMenu::display(){
  _screen.displayTemp(MENU_TIMEOUT, _current->getLabel(), _current->highlighted()->getNumberedLabel());
}