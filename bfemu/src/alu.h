#ifndef ALU_H
#define ALU_H
#include "module.h"

class ALU: public Module
{
public:

  enum Input {
    A0, A1, A2, A3, A4, A5, A6, A7,       // input data A
    A8, A9, A10, A11, A12, A13, A14, A15, // input data A
    
    SUB,
    N_INPUT,

    DATA_IN_A = Module::mask(A0, A1, A2, A3, A4, A5, A6, A7),
    DATA_IN_B = Module::mask(A8, A9, A10, A11, A12, A13, A14, A15),
    
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, // output data
    N_OUTPUT,

    DATA_OUT = Module::mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
  };

  ALU()
  {
    setSubEnabled(false);
  }

  virtual int numberOfInputs() const override {
    return N_INPUT;
  };

  virtual int numberOfOutputs() const override {
    return N_OUTPUT;
  };

  virtual void update() override
  {
    unsigned long valueAtInputA = Module::input(DATA_IN_A);
    unsigned long valueAtInputB = Module::input(DATA_IN_B);
    
    Module::setOutput(valueAtInputA + (subEnabled() ? -1 : 1) * valueAtInputB);
  }

  virtual bool canBeClocked() const override
  {
    return false;
  }

  DEFINE_CONTROL_PIN(SUB, setSubEnabled, subEnabled);
};


#endif // ALU_H
