#ifndef POWER_H
#define POWER_H
#include "module.h"

class Power: public Module
{
public:

  enum OutputPins {
    GND, VCC,

    LOW = Module::mask(GND),
    HIGH = Module::mask(VCC)
  };
  
  Power(){
    Module::setOutput(0b10);
  }

  virtual int numberOfInputs() const override {
    return 0;
  }

  virtual int numberOfOutputs() const override {
    return 2;
  }

  virtual bool canBeClocked() const override {
    return false;
  }
};

#endif // POWER_H
