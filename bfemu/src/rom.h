#ifndef ROM_H
#define ROM_H
#include <cstring>
#include "module.h"

class ROM: public Module
{
  static constexpr size_t CAPACITY = (1 << 16) - 1; // 16-bit addressable
  int d_memory[CAPACITY];
  
public:
  ROM() {
    memset(&d_memory[0], 0, CAPACITY * sizeof(int));
  }
  
  enum Input {
    A0, A1, A2, A3, A4, A5, A6, A7,
    A8, A9, A10, A11, A12, A13, A14, A15, // address lines (16-bit)

    N_INPUT,
    ADDRESS_IN_LOW  = mask(A0, A1, A2, A3, A4, A5, A6, A7),
    ADDRESS_IN_HIGH = mask(A8, A9, A10, A11, A12, A13, A14, A15),
    ADDRESS_IN = ADDRESS_IN_LOW | ADDRESS_IN_HIGH,
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, // output data
    N_OUTPUT,

    DATA_OUT = mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7)
  };

  virtual int numberOfInputs() const override {
    return N_INPUT;
  }
  virtual int numberOfOutputs() const override {
    return N_OUTPUT;
  }

  virtual void update() override {
    unsigned long const address = input(ADDRESS_IN);
    Module::setOutput(d_memory[address]);
  }

  virtual bool canBeClocked() const override {
    return false;
  }

  template <size_t N>
  void load(unsigned char const (&data)[N])
  {
    for (size_t i = 0; i != N; ++i)
      d_memory[i] = data[i];
  }

  void load(unsigned char const *data, size_t N)
  {
    for (size_t i = 0; i != N; ++i)
      d_memory[i] = data[i];
  }

  unsigned char at(size_t idx) const
  {
    assert(idx < CAPACITY && "index out of bounds");
    return d_memory[idx];
  }
  
};



#endif // RAM_H
