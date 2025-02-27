#include "lcdmenu.h"

namespace MenuTree {
  struct MenuItem;

  using SelectFunction = MenuItem* (*)(MenuItem *self, LCDBuffer &buffer, LCDScreen &screen);

  class MenuItem {
    static constexpr uint8_t MAX_CHILDREN = 5; // arbirtary, but this fits in a single digit
    MenuItem* parent = nullptr;
    MenuItem* root = nullptr;
    MenuItem* children[MAX_CHILDREN];
    uint8_t numChildren = 0;
    
    const char *label;
    char numberedLabel[LINE_SIZE + 1];
    
    uint8_t highlightedIndex = 0;
    SelectFunction selectFunc = nullptr;

  public:
    // Final node -> do something according to the select function
    MenuItem(char const *label, SelectFunction sel):
      label(label),
      selectFunc(sel)
    {}
    
    // Submenu node
    template <typename ... Others>
    MenuItem(char const *label, MenuItem &child, Others& ... others):
      children{&child, (&others)...},
      numChildren(1 + sizeof ... (others)),
      label(label)
    {
      static_assert(sizeof ... (others) < MAX_CHILDREN, "Too many child-nodes passed to MenuItem constructor");
      
      // Tell my children what position they have in my list, so they can add this number to their label
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        children[idx]->setParent(this, idx + 1);
      }
    }

    MenuItem *highlighted() {
      return (numChildren > 0) ? children[highlightedIndex] : this;
    }

    void up() {
      if (numChildren > 0) {
        highlightedIndex = (highlightedIndex + numChildren - 1) % numChildren;
      }
    }

    void down() {
      if (numChildren > 0) {
        highlightedIndex = (highlightedIndex + 1) % numChildren;
      }
    }

    MenuItem *select(LCDBuffer &buffer, LCDScreen &screen) {
      return selectFunc ? selectFunc(this, buffer, screen) 
                        : this;
    }


    void makeRoot() {
      setRoot(this);
    }
    
    char const *getLabel() const {
      return label;
    }
    
    char const *getNumberedLabel() const {
      return parent ? numberedLabel : label;
    }
    
    MenuItem *getParent() {
      return parent;
    }

    MenuItem *getRoot() {
      return root;
    }

  private:
    void setParent(MenuItem *par, uint8_t const pos) {
      parent = par;
      snprintf(numberedLabel, LINE_SIZE + 1, "%d. %s", pos, label); 
    }

    void setRoot(MenuItem *r) {
      root = r;
      for (uint8_t idx = 0; idx != numChildren; ++idx) {
        children[idx]->setRoot(root);
      }
    }
  };


  MenuItem clearItem("Clear", [](MenuItem*, LCDBuffer &buffer, LCDScreen &) -> MenuItem* {
    buffer.clear();
    return nullptr;
  });

  MenuItem echoOnItem("On", [](MenuItem *self, LCDBuffer &buffer, LCDScreen &) -> MenuItem* {
    buffer.setEchoEnabled(true);
    return self->getRoot();
  });

  MenuItem echoOffItem("Off", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setEchoEnabled(false);
    return self->getRoot();
  });

  MenuItem autoscrollOnItem("On", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setAutoScroll(true);
    return self->getRoot();
  });

  MenuItem autoscrollOffItem("Off", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setAutoScroll(false);
    return self->getRoot();
  });

  MenuItem hexModeItem("Hexadecimal", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setMode(HEXADECIMAL);
    return self->getRoot();
  });

  MenuItem decModeItem("Decimal", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setMode(DECIMAL);
    return self->getRoot();
  });

  MenuItem textModeItem("Text", [](MenuItem *self, LCDBuffer &buffer, LCDScreen&) -> MenuItem* {
    buffer.setMode(ASCII);
    return self->getRoot();
  });

  MenuItem exitItem("Exit", [](MenuItem*, LCDBuffer&, LCDScreen &) -> MenuItem* {
    return nullptr;
  });

  MenuItem echoMenu("Echo", echoOnItem, echoOffItem);
  MenuItem autoscrollMenu("Autoscroll", autoscrollOnItem, autoscrollOffItem);
  MenuItem modeMenu("Display Mode", textModeItem, decModeItem, hexModeItem);
  MenuItem rootMenu("Main Menu", clearItem, echoMenu, autoscrollMenu, modeMenu, exitItem);
}


LCDMenu::LCDMenu(LCDBuffer &buf, LCDScreen &scr): 
  _buffer(buf), 
  _screen(scr)
{}

void LCDMenu::begin() {
  MenuTree::rootMenu.makeRoot();
}

void LCDMenu::enter() {
  _lastActiveTime = millis();
  _current = &MenuTree::rootMenu;
  display();
}

bool LCDMenu::active() const {
  return (_lastActiveTime > 0 && (millis() - _lastActiveTime) < MENU_TIMEOUT);
}

void LCDMenu::handleButtons(ButtonState const up, ButtonState const down, ButtonState const both) {
  static bool bothReleased = true;
  if (bothReleased && both == ButtonState::Rising) {
    _lastActiveTime = millis();
    bothReleased = false;
    MenuTree::MenuItem *next = _current->highlighted()->select(_buffer, _screen);
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

void LCDMenu::display() {
  _screen.displayTemp(MENU_TIMEOUT, _current->getLabel(), _current->highlighted()->getNumberedLabel());
}