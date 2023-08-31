#ifndef FIXEDOUTPUT_H
#define FIXEDOUTPUT_H

#include "module.h"

class FixedOutput: public Module
{
public:  
  enum Input {
    EN
  };
  
  FixedOutput(unsigned long const value = 0):
    Module(EN)
  {
    setData(value);
  }
  
  enum {
    LOW_BYTE = 0x00ff,
    HIGH_BYTE = 0xff00,
    DATA_OUT = 0xffff
  };
    
  virtual int numberOfInputs() const override {
    return 1;
  }
  
  virtual int numberOfOutputs() const override {
    return 16;
  }

  void setData(unsigned long const data) {
    Module::setOutput(data);
  }
};

#endif // FIXEDOUTPUT_H
