#ifndef RAM_H
#define RAM_H
#include <cstring>
#include "module.h"
class RAM: public Module
{
  static constexpr size_t CAPACITY = (1 << 16) - 1; // 16-bit addressable
  int d_memory[CAPACITY];

public:
  
  enum Input {
    A0, A1, A2, A3, A4, A5, A6, A7,          // address lines (16 bit)
    A8, A9, A10, A11, A12, A13, A14, A15, 

    D0, D1, D2, D3, D4, D5, D6, D7,          // data lines (8-bit)
    EN, WE,                                  // output/write enable

    N_INPUT,
    ADDRESS_IN_LOW  = Module::mask(A0, A1, A2, A3, A4, A5, A6, A7),
    ADDRESS_IN_HIGH = Module::mask(A8, A9, A10, A11, A12, A13, A14, A15),
    ADDRESS_IN = ADDRESS_IN_LOW | ADDRESS_IN_HIGH,
    
    DATA_IN = Module::mask(D0, D1, D2, D3, D4, D5, D6, D7),
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, // output data
    N_OUTPUT,

    DATA_OUT = Module::mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7)
  };

  RAM():
    Module(EN)
  {
    memset(&d_memory[0], 0, CAPACITY * sizeof(int));
  }
  
  
  virtual int numberOfInputs() const override {
    return N_INPUT;
  }
  virtual int numberOfOutputs() const override {
    return N_OUTPUT;
  }

  virtual void onClockFalling() override {
    if (writeEnabled()) {
      unsigned long const address = input(ADDRESS_IN);
      unsigned long const data = input(DATA_IN);
      d_memory[address] = data;
    }
  }

  virtual void update() override {
    unsigned long const address = input(ADDRESS_IN);
    Module::setOutput(d_memory[address]);
  }

  template <size_t N>
  void load(unsigned char const (&data)[N])
  {
    for (size_t i = 0; i != N; ++i)
      d_memory[i] = data[i];
  }

  unsigned char at(size_t idx) const
  {
    assert(idx < CAPACITY && "index out of bounds");
    return d_memory[idx];
  }
  
  DEFINE_CONTROL_PIN(WE, setWriteEnabled, writeEnabled);
};



#endif // RAM_H
