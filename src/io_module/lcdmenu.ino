 #include "lcdmenu.h"
 
LCDMenu::LCDMenu(LCDBuffer &buf, LCDScreen &scr):
  _buffer(buf), 
  _screen(scr),
  _actions(buf)
{}

void LCDMenu::enter(){
  _current = _menu.home();
  _lastActiveTime = millis();
  display();
}

bool LCDMenu::active() const{
  return (_lastActiveTime > 0 && (millis() - _lastActiveTime) < MENU_TIMEOUT);
}

void LCDMenu::handleButtons(ButtonState const up, ButtonState const down, ButtonState const both){
  if (_current == _menu.exit())   return exit();
  if (both == ButtonState::Hold ) return exit();


  static bool bothReleased = true;
  if (bothReleased && both == ButtonState::Rising) {
    _lastActiveTime = millis();
    bothReleased = false;
    Menu::BasePtr next = _current->highlighted()->select(_actions);
    if (next == _menu.exit()) return exit();
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
  _screen.displayTemp(MENU_TIMEOUT, true, _current->getLabel(), _current->highlighted()->getNumberedLabel());
}

void LCDMenu::exit() {
  _current = _menu.home();
  _screen.clear();
  _lastActiveTime = 0;
}