#ifndef SCREEN_H
#define SCREEN_H
#include <iostream>
#include "module.h"

class Screen: public Module
{
  std::ostream &d_out;
public:
  Screen(std::ostream &out = std::cout):
    d_out(out)
  {}
  
  enum Input {
    D0, D1, D2, D3, D4, D5, D6, D7,
    N_INPUT,
    DATA_IN = Module::mask(D0, D1, D2, D3, D4, D5, D6, D7),
  };

  virtual int numberOfInputs() const override {
    return N_INPUT;
  }
  
  virtual int numberOfOutputs() const override {
    return 0;
  }

  virtual bool canBeClocked() const override {
    return false;
  }

  void refresh() {
    unsigned char const data = input(DATA_IN);
    d_out << data;
  }

  ~Screen() {
    d_out << '\n';
  }
};

#endif // SCREEN_H
