#ifndef REGISTER_H
#define REGISTER_H
#include "module.h"

class Register: public Module
{
  uint16_t d_value;
  uint16_t const d_resetValue;
  
public:

  enum Input {
    D0, D1, D2, D3, D4, D5, D6, D7, 
    D8, D9, D10, D11, D12, D13, D14, D15, // input data
    
    EN, LD, // output enable & load enable
    N_INPUT,

    DATA_IN_LOW = Module::mask(D0, D1, D2, D3, D4, D5, D6, D7),
    DATA_IN_HIGH = Module::mask(D8, D9, D10, D11, D12, D13, D14, D15),
    DATA_IN = DATA_IN_HIGH | DATA_IN_LOW
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7,
    Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15, // output data
    
    N_OUTPUT,

    DATA_OUT_LOW = Module::mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
    DATA_OUT_HIGH = Module::mask(Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15),
    DATA_OUT = DATA_OUT_HIGH | DATA_OUT_LOW
  };
  
  Register(uint16_t const resetValue = 0);

  virtual int numberOfInputs() const override {
    return N_INPUT;
  }

  virtual int numberOfOutputs() const override {
    return N_OUTPUT;
  }

  virtual void reset() override {
    setOutput(d_resetValue);
  }
  
  virtual void onClockFalling() override;  
  virtual void update() override;

  void setData(unsigned long data); // for testing

  DEFINE_CONTROL_PIN(LD, setLoadEnabled, loadEnabled);
};

#endif // REGISTER_H