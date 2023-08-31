#include <iostream>
#include "register.h"

Register::Register(uint16_t const resetValue):
  Module(EN),
  d_value(resetValue),
  d_resetValue(resetValue)
{
  setLoadEnabled(false);
  setOutputEnabled(false);
}

void Register::onClockFalling() 
{
  // Latch data from input
  if (loadEnabled())
    d_value = input(DATA_IN);
}

void Register::setData(unsigned long data) // for testing
{
  d_value = data;
}

void Register::update()
{
  Module::setOutput(d_value);
}
