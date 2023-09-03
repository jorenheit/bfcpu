#include <iostream>
#include "module.h"
#include "power.h"

Module *Module::power = nullptr;

void Module::init()
{
  static Power s_power;
  static bool initialized = false;
  if (initialized)
    return;
  
  Module::power = &s_power;
}

void Module::set(int const pin, bool const en)
{
  connectInputTo(*power, (en ? Power::HIGH : Power::LOW), mask(pin), true);
}

void Module::setOutputEnabled(bool const en)
{
  assert(d_outputEnablePin != -1 && "Module has no EN pin");
  set(d_outputEnablePin, en);
}

bool Module::outputEnabled()
{
  return d_outputEnablePin == -1 ? true : inputByIndex(d_outputEnablePin);
}

bool Module::outputByIndex(int const index, bool const update, bool const peek) 
{
  assert(index < this->numberOfOutputs() && "index out of range");
  if (update)
    this->update();

  return (peek || d_peek[index] || this->outputEnabled()) ? d_outputPins[index] : LOW;
}
  
unsigned long Module::output(unsigned long mask, bool const peek)
{
  this->update();
  unsigned long result = 0;
  for (int i = 0; i != this->numberOfOutputs(); ++i) {
    if (mask & (1 << i))
      result |= outputByIndex(i, false, peek) << i;
  }
  
  return result;
}

unsigned long Module::peek(unsigned long mask)
{
  return output(mask, true);
}


bool Module::inputByIndex(int const index)
{
  assert(index < this->numberOfInputs() && "index out of range");

  // return OR of all outputs connected to this input
  bool result = false;
  for (Connection const &connection: d_inputPins[index]) {
    Module *other = connection.other;
    assert(other && "input connected to NULL module");
    result = result || other->outputByIndex(connection.index);
  }
  return result;
}
  
unsigned long Module::input(unsigned long mask)
{
  unsigned long result = 0;
  int count = 0;
  for (int i = 0; i != this->numberOfInputs(); ++i) {
    if (mask & (1 << i)) {
      bool const value  = inputByIndex(i);
      result |= (static_cast<unsigned long>(value) << count++);
    }
  }
  return result;
}

void Module::connectInputToIndex(Module &other, int const outputIndex, int const inputIndex, bool const disconnectOtherConnections)
{
  if (disconnectOtherConnections)
    d_inputPins[inputIndex].clear();
  d_inputPins[inputIndex].emplace_back(Connection{&other, outputIndex});
}

void Module::connectInputTo(Module &other, unsigned long const outputMask, unsigned long const inputMask, bool const disconnectOtherConnections)
{
  auto const countOnes = [](unsigned long mask) {
    size_t count = mask & 1;
    while (mask >>= 1) count += (mask & 1);
    return count;
  };

  size_t const n = countOnes(outputMask);
  size_t const m = countOnes(inputMask);
    
  assert((n == 1 || n == m) && "masks do not match");

  auto const nextPin = [](unsigned long mask, int current = -1) {
    mask >>= (current + 1);
    int count = 1;
    while (!(mask & 1)) {
      mask >>= 1;
      ++count;
    }
    return current + count;
  };

  if (n == m) {
    // Case 1: equal number of pins in both masks. Match up from LSB -> MSB.
    int currentOutputPin = -1;
    int currentInputPin = -1;
    for (size_t i = 0; i != n; ++i) {
      currentOutputPin = nextPin(outputMask, currentOutputPin);
      currentInputPin = nextPin(inputMask, currentInputPin);
      assert(currentInputPin < this->numberOfInputs() && "mask contains invalid input pin");
      assert(currentOutputPin < other.numberOfOutputs() && "mask contains invalid output pin");
      connectInputToIndex(other, currentOutputPin, currentInputPin, disconnectOtherConnections);
    }
  }
  else if (n == 1) {
    // Case 2: multiple inputs connected to the same output.
    int const outputPin = nextPin(outputMask);
    assert(outputPin < other.numberOfOutputs() && "mask contains invalid output pin");
    int currentInputPin = -1;
    for (size_t i = 0; i != m; ++i) {
      currentInputPin = nextPin(inputMask, currentInputPin);
      assert(currentInputPin < this->numberOfInputs() && "mask contains invalid input pin");
      connectInputToIndex(other, outputPin, currentInputPin, disconnectOtherConnections);
    }
  } else {
    assert(false && "Unreachable");
  }
}

