#pragma once

class LCDScreen;
class LCDBuffer;
class MenuItem;

using SelectFunction = MenuItem* (*)(MenuItem*, LCDBuffer&, LCDScreen&);

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
  MenuItem(char const *label, SelectFunction sel);
  
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

  MenuItem *highlighted();
  void up();
  void down();
  MenuItem *select(LCDBuffer &buffer, LCDScreen &screen);
  void makeRoot();
  char const *getLabel() const;
  char const *getNumberedLabel() const;
  MenuItem *getParent();
  MenuItem *getRoot();

private:
  void setParent(MenuItem *par, uint8_t const pos);
  void setRoot(MenuItem *rt);
};
