#ifndef CLOCK_H
#define CLOCK_H
#include <vector>
#include "module.h"

class Clock: public Module
{
  std::vector<Module*> d_connectedModules;
  
public:

  template <typename ... Rest>
  void connect(Module &first, Rest& ... rest)
  {
    assert(first.canBeClocked() && "Module cannot be clocked");
    d_connectedModules.push_back(&first);

    if constexpr (sizeof ... (rest) > 0) {
      connect(rest ...);
    }
  }

  
  enum Input {
    HLT,
    N_INPUT
  };

  // output is handled differently from other modules,
  // by calling the virtual functions in order. See pulse()
  
  void pulse()
  {
    if (haltEnabled())
      return;

    for (Module *module: d_connectedModules)
      module->onClockRising();

    for (Module *module: d_connectedModules)
      module->onClockHigh();

    for (Module *module: d_connectedModules)
      module->onClockFalling();

    for (Module *module: d_connectedModules)
      module->onClockLow();
  }

  virtual int numberOfInputs() const override {
    return N_INPUT;
  }
  
  virtual int numberOfOutputs() const override {
    return 0;
  }

  virtual bool canBeClocked() const override {
    return false;
  }
  
  DEFINE_CONTROL_PIN(HLT, setHaltEnabled, haltEnabled);
  
};


#endif // CLOCK_H
