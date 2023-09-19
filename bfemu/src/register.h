#ifndef REGISTER_H
#define REGISTER_H
#include "module.h"
#include "registerpins.h"

template <size_t N>
class Register: public Module, public RegisterPins<N>
{
  using Pins = RegisterPins<N>;
  using ValueType = typename RegisterPins<N>::ValueType;
  
  uint16_t d_value = 0;
  uint16_t const d_resetValue;
  bool d_carry = false;
  bool d_zero = true;
  
public:
  Register(uint16_t const resetValue = 0):
    Module(Pins::EN, Pins::Z, Pins::CA),
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
    return Pins::N_INPUT;
  }

  virtual int numberOfOutputs() const override
  {
    return Pins::N_OUTPUT;
  }

  virtual void reset() override
  {
    d_value = d_resetValue;
    d_zero = (d_value == 0);
    d_carry = false;
  }
  
  virtual void onClockFalling() override
  {
    if (loadEnabled()) {
      d_value = input(Pins::DATA_IN);
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
    setOutput(static_cast<unsigned long>(d_value) | (d_carry << Pins::CA) | (d_zero << Pins::Z));
  }

  ValueType value() const
  {
    return d_value;
  }

  int relativeValue() const
  {
    return (int)d_value - (int)d_resetValue;
  }
  
  bool zero() const
  {
    return d_zero;
  }

  bool carry() const
  {
    return d_carry;
  }
  
  DEFINE_CONTROL_PIN(Pins::LD, setLoadEnabled, loadEnabled);
  DEFINE_CONTROL_PIN(Pins::CNT, setCountEnabled, countEnabled);
  DEFINE_CONTROL_PIN(Pins::DEC, setDecEnabled, decEnabled);
};


#endif // REGISTER_H
