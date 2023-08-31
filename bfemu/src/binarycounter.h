#ifndef BINARYCOUNTER_H
#define BINARYCOUNTER_H
#include "module.h"

class BinaryCounter: public Module
{
  uint16_t d_value = 0;
  uint16_t const d_resetValue;
  bool d_carry = false;
  bool d_zero = true;
  
public:
  BinaryCounter(uint16_t const resetValue = 0);

  
  enum Input {
    D0, D1, D2, D3, D4, D5, D6, D7, 
    D8, D9, D10, D11, D12, D13, D14, D15, // input data
    
    EN, LD, CNT, DEC, // output enable, load enable, count enable, decrement enable
    N_INPUT,
    DATA_IN_LOW = Module::mask(D0, D1, D2, D3, D4, D5, D6, D7),
    DATA_IN_HIGH = Module::mask(D8, D9, D10, D11, D12, D13, D14, D15),
    DATA_IN = DATA_IN_HIGH | DATA_IN_LOW
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, 
    Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15, // output data
    
    CA, // carry-flag; set on over-/underflow
    Z,  // zero flag

    N_OUTPUT,
    DATA_OUT_LOW = Module::mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
    DATA_OUT_HIGH = Module::mask(Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15),
    DATA_OUT = DATA_OUT_HIGH | DATA_OUT_LOW
  };
  
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

  DEFINE_CONTROL_PIN(LD, setLoadEnabled, loadEnabled);
  DEFINE_CONTROL_PIN(CNT, setCountEnabled, countEnabled);
  DEFINE_CONTROL_PIN(DEC, setDecEnabled, decEnabled);
  
  void setData(unsigned long data); // for testing

};


#endif // BINARYCOUNTER_H
