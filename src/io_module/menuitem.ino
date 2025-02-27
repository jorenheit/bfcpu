#include "menuitem.h"

MenuItem::MenuItem(char const *label, SelectFunction sel):
  label(label),
  selectFunc(sel)
{}
  
MenuItem *MenuItem::highlighted() {
  return (numChildren > 0) ? children[highlightedIndex] : this;
}

void MenuItem::up() {
  if (numChildren > 0) {
    highlightedIndex = (highlightedIndex + numChildren - 1) % numChildren;
  }
}

void MenuItem::down() {
  if (numChildren > 0) {
    highlightedIndex = (highlightedIndex + 1) % numChildren;
  }
}

MenuItem *MenuItem::select(LCDBuffer &buffer, LCDScreen &screen) {
  return selectFunc ? selectFunc(this, buffer, screen) 
                    : this;
}

void MenuItem::makeRoot() {
  setRoot(this);
}
  
char const *MenuItem::getLabel() const {
  return label;
}
  
char const *MenuItem::getNumberedLabel() const {
  return parent ? numberedLabel : label;
}
  
MenuItem *MenuItem::getParent() {
  return parent;
}

MenuItem *MenuItem::getRoot() {
  return root;
}


void MenuItem::setParent(MenuItem *par, uint8_t const pos) {
  parent = par;
  snprintf(numberedLabel, LINE_SIZE + 1, "%d. %s", pos, label); 
}

void MenuItem::setRoot(MenuItem *rt) {
  root = rt;
  for (uint8_t idx = 0; idx != numChildren; ++idx) {
    children[idx]->setRoot(root);
  }
}
