#include <iostream>
#include "module.h"

class Power: public Module
{
public:

  enum OutputPins {
    GND, VCC,

    LOW = mask(GND),
    HIGH = mask(VCC)
  };
  
  Power(){
    Module::setOutput(0b10);
  }

  virtual int numberOfInputs() const override {
    return 0;
  }

  virtual int numberOfOutputs() const override {
    return 2;
  }

  virtual bool canBeClocked() const override {
    return false;
  }
};

Power *Module::power = nullptr;

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
  connectModules(*power, (en ? Power::HIGH : Power::LOW), *this, mask(pin), true);
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

void connectModulesByIndex(Module &outputModule, int const outputIndex, Module &inputModule, const int inputIndex, bool const disconnectOtherConnections)
{
  if (disconnectOtherConnections)
    inputModule.d_inputPins[inputIndex].clear();
  inputModule.d_inputPins[inputIndex].emplace_back(Module::Connection{&outputModule, outputIndex});
}

void connectModules(Module &outputModule, unsigned long const outputMask, Module &inputModule, unsigned long const inputMask, bool const disconnectOtherConnections)
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
      assert(currentInputPin < inputModule.numberOfInputs() && "mask contains invalid input pin");
      assert(currentOutputPin < outputModule.numberOfOutputs() && "mask contains invalid output pin");
      connectModulesByIndex(outputModule, currentOutputPin, inputModule, currentInputPin, disconnectOtherConnections);
    }
  }
  else if (n == 1) {
    // Case 2: multiple inputs connected to the same output.
    int const outputPin = nextPin(outputMask);
    assert(outputPin < outputModule.numberOfOutputs() && "mask contains invalid output pin");
    int currentInputPin = -1;
    for (size_t i = 0; i != m; ++i) {
      currentInputPin = nextPin(inputMask, currentInputPin);
      assert(currentInputPin < inputModule.numberOfInputs() && "mask contains invalid input pin");
      connectModulesByIndex(outputModule, outputPin, inputModule, currentInputPin, disconnectOtherConnections);
    }
  } else {
    assert(false && "Unreachable");
  }
}

