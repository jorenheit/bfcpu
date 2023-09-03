#include <iostream>
#include "binarycounter.h"

BinaryCounter::BinaryCounter(uint16_t const resetValue):
  Module(EN, Z, CA),
  d_resetValue(resetValue)
{
  setData(resetValue);
}

void BinaryCounter::setData(unsigned long data) // for testing
{
  d_value = data;
  d_carry = 0;
  d_zero = (data == 0);
}

void BinaryCounter::onClockFalling()
{
  // Datasheet for SN74F163A says that when load is enabled,
  // it will not increment/decrement. The value stored should agree
  // with the input data after the clock.
  
  if (loadEnabled()) {
    d_value = input(DATA_IN);
    d_zero = (d_value == 0);
    d_carry = 0;
    return;
  }

  if (!countEnabled())
    return;

  bool const inc = !decEnabled();
  d_carry = (d_value == 0xffff && inc) || (d_value == 0 && !inc);
  d_value += (inc ? 1 : -1);
  d_zero = (d_value == 0);
}


void BinaryCounter::update()
{
  setOutput(static_cast<unsigned long>(d_value) | (d_carry << CA) | (d_zero << Z));
}
