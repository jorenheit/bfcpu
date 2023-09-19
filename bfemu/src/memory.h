#ifndef MEMORY_H
#define MEMORY_H
#include "module.h"

struct RAM {};
struct ROM {};


template <typename MemoryType>
struct MemoryPins;

template <>
struct MemoryPins<RAM>
{
  enum Input {
    A0, A1, A2, A3, A4, A5, A6, A7,          // address lines (16 bit)
    A8, A9, A10, A11, A12, A13, A14, A15, 

    D0, D1, D2, D3, D4, D5, D6, D7,          // data lines (8-bit)
    EN, WE,                                  // output/write enable

    N_INPUT,
    ADDRESS_IN_LOW  = mask(A0, A1, A2, A3, A4, A5, A6, A7),
    ADDRESS_IN_HIGH = mask(A8, A9, A10, A11, A12, A13, A14, A15),
    ADDRESS_IN = ADDRESS_IN_LOW | ADDRESS_IN_HIGH,
    
    DATA_IN = mask(D0, D1, D2, D3, D4, D5, D6, D7),
  };

  enum Output {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, // output data
    N_OUTPUT,

    DATA_OUT = mask(Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7)
  };
};

template <>
struct MemoryPins<ROM>
{
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
};

template <typename MemoryType>
class Memory: public Module,
	      public MemoryPins<MemoryType>
{
  using Pins = MemoryPins<MemoryType>;
  
  static constexpr size_t CAPACITY = (1 << 16) - 1; // 16-bit addressable
  int d_memory[CAPACITY];

  
public:
  Memory():
    Module(Pins::EN)
  {
    memset(&d_memory[0], 0, CAPACITY * sizeof(int));
  }

  virtual int numberOfInputs() const override
  {
    return Pins::N_INPUT;
  }
  
  virtual int numberOfOutputs() const override
  {
    return Pins::N_OUTPUT;
  }
  
  virtual void onClockFalling() override
  {
    if constexpr (std::is_same_v<MemoryType, RAM>) {
      if (writeEnabled()) {
	unsigned long const address = input(ADDRESS_IN);
	unsigned long const data = input(DATA_IN);
	d_memory[address] = data;
      }
    } else {

    }
  }


};


#endif // MEMORY_H
