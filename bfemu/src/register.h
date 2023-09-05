#ifndef REGISTER_H
#define REGISTER_H
#include "module.h"


namespace Impl_ {

  template <size_t N>
  struct RegisterPins;

  template <>
  struct RegisterPins<4>
  {
    enum Input {
      D0, D1, D2, D3, 
      EN, LD, CNT, DEC,
      N_INPUT,
      DATA_IN = mask(D0, D1, D2, D3),
    };

    enum Output {
      Q0, Q1, Q2, Q3, 
      CA,
      Z, 
      N_OUTPUT,
      DATA_OUT = mask(Q0, Q1, Q2, Q3),
    };
  };

  template <>
  struct RegisterPins<8>
  {
    enum Input {
      D0, D1, D2, D3, D4, D5, D6, D7, 
      EN, LD, CNT, DEC,
      N_INPUT,
      DATA_IN = mask(D0, D1, D2, D3, D4, D5, D6, D7),
    };

    enum Output {
      Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, 
      CA,
      Z, 
      N_OUTPUT,
      DATA_OUT = mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
    };
  };

  template <>
  struct RegisterPins<16>
  {
    enum Input {
      D0, D1, D2, D3, D4, D5, D6, D7, 
      D8, D9, D10, D11, D12, D13, D14, D15,
    
      EN, LD, CNT, DEC,
      N_INPUT,
      DATA_IN_LOW = mask(D0, D1, D2, D3, D4, D5, D6, D7),
      DATA_IN_HIGH = mask(D8, D9, D10, D11, D12, D13, D14, D15),
      DATA_IN = DATA_IN_HIGH | DATA_IN_LOW
    };

    enum Output {
      Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, 
      Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15,
      CA,
      Z, 

      N_OUTPUT,
      DATA_OUT_LOW = mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7),
      DATA_OUT_HIGH = mask(Q8, Q9, Q10, Q11, Q12, Q13, Q14, Q15),
      DATA_OUT = DATA_OUT_HIGH | DATA_OUT_LOW
    };
  };  
}

template <size_t N>
class Register: public Module, public Impl_::RegisterPins<N>
{
  using Pin = Impl_::RegisterPins<N>;
  
  uint16_t d_value = 0;
  uint16_t const d_resetValue;
  bool d_carry = false;
  bool d_zero = true;
  
public:
  Register(uint16_t const resetValue = 0):
    Module(Pin::EN, Pin::Z, Pin::CA),
    d_resetValue(resetValue)
  {
    setInternalState(resetValue);
  }

  void setInternalState(unsigned long data)
  {
    d_value = data;
    d_carry = 0;
    d_zero = (data == 0);
  }
  
  virtual int numberOfInputs() const override
  {
    return Pin::N_INPUT;
  }

  virtual int numberOfOutputs() const override
  {
    return Pin::N_OUTPUT;
  }

  virtual void reset() override
  {
    setOutput(d_resetValue);
  }
  
  virtual void onClockFalling() override
  {
    // Datasheet for SN74F163A says that when load is enabled,
    // it will not increment/decrement. The value stored should agree
    // with the input data after the clock.
  
    if (loadEnabled()) {
      d_value = input(Pin::DATA_IN);
      d_zero = (d_value == 0);
      d_carry = 0;
      return;
    }

    if (!countEnabled())
      return;

    bool const inc = !decEnabled();
    if (inc && d_value == ((1L << N) - 1)) {
      d_value = 0;
      d_zero = true;
      d_carry = true;
    }
    else if (!inc && d_value == 0) {
      d_value = (1L << N) - 1;
      d_zero = false;
      d_carry = true;
    }
    else {
      d_value += (inc ? 1 : -1);
      d_zero = (d_value == 0);
      d_carry = false;
    }
  }

  virtual void update() override
  {
    setOutput(static_cast<unsigned long>(d_value) | (d_carry << Pin::CA) | (d_zero << Pin::Z));
  }

  DEFINE_CONTROL_PIN(Pin::LD, setLoadEnabled, loadEnabled);
  DEFINE_CONTROL_PIN(Pin::CNT, setCountEnabled, countEnabled);
  DEFINE_CONTROL_PIN(Pin::DEC, setDecEnabled, decEnabled);
};


#endif // REGISTER_H
