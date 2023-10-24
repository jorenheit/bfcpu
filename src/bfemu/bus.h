#ifndef BUS_H
#define BUS_H

#include "module.h"

class Bus: public Module
{
public:
  enum Bits {
    B0, B1, B2, B3, B4, B5, B6, B7, // low bits
    B8, B9, B10, B11, B12, B13, B14, B15, // high bits

    N_BITS,
    LOW_BYTE  = Module::mask(B0, B1, B2, B3, B4, B5, B6, B7),
    HIGH_BYTE = Module::mask(B8, B9, B10, B11, B12, B13, B14, B15),
    FULL = LOW_BYTE | HIGH_BYTE
  };
  
  virtual int numberOfInputs() const override {
    return N_BITS;
  }
  
  virtual int numberOfOutputs() const override {
    return N_BITS;
  }

  virtual bool canBeClocked() const override {
    return false;
  }

  virtual void update() override {
    Module::setOutput(input());
  }
};

#endif // BUS_H
