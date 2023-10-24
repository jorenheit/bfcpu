#ifndef MODULE_H
#define MODULE_H
#include <vector>
#include <cassert>
#include <iostream>

template <typename ... Args>
void ASSERT(bool const condition, Args ... args)
{
  if (condition) return;

  std::cerr << "Assertion failed: ";
  (std::cerr << ... << args) << '\n';
  std::exit(1);
}


template <typename ... Indices>
static constexpr unsigned long const mask(Indices ... indices) {
  return ((1 << indices) | ...);
};

class Power;
class Module;

void connectModules(Module &outModule, unsigned long const outputMask,
		    Module &inModule, unsigned long const inputMask,
		    bool const disconnectOtherConnections = false);

void connectModulesByIndex(Module &outputModule, int const outputIndex,
			   Module &inputModule, const int inputIndex,
			   bool const disconnectOtherConnections = false);

class Module
{
  friend void connectModulesByIndex(Module &, int const, Module &, const int, bool const);
  friend void connectModules(Module &, unsigned long const, Module &, unsigned long const, bool const);
  
  enum PinState { LOW, HIGH };
  
  struct Connection
  {
    Module *other = nullptr;
    int index;
  };

  static constexpr size_t N_PINS = 8 * sizeof(unsigned long);
  bool d_outputPins[N_PINS];
  bool d_peek[N_PINS];
  std::vector<Connection> d_inputPins[N_PINS];
  static Power *power;
  int const d_outputEnablePin;

protected:
  void setOutput(unsigned long const data)
  {
    for (int i = 0; i != this->numberOfOutputs(); ++i) {
      d_outputPins[i] = data & (1 << i);
    }
  }
  
public:
  Module(int const enPin = -1):
    d_outputEnablePin(enPin)
  {
    for (int i = 0; i != N_PINS; ++i) {
      d_outputPins[i] = false;
      d_peek[i] = false;
    }
  }

  template <typename ... Rest>
  Module(int const enPin, int const peekPin1, Rest const ... rest):
    Module(enPin)
  {
    int const peekPins[] = {peekPin1, rest ...};
    for (int i = 0; i != 1 + sizeof ... (rest); ++i) {
      assert(peekPins[i] < (int)N_PINS && "peekpin index out of bounds");
      d_peek[peekPins[i]] = true;
    }
  }
  

  static void init();
  virtual int numberOfInputs() const = 0;
  virtual int numberOfOutputs() const = 0;

  virtual void update() {}
  virtual void onClockRising() {}
  virtual void onClockFalling() {}
  virtual void onClockHigh() {}
  virtual void onClockLow() {}
  virtual bool canBeClocked() const { return true; }
  virtual void reset() { setOutput(0); };
  
  void setOutputEnabled(bool const en);
  bool outputEnabled();

  unsigned long input(unsigned long mask = -1);
  unsigned long output(unsigned long mask = -1, bool const peek = false);
  unsigned long peek(unsigned long mask = -1);
  bool inputByIndex(int const index);
  bool outputByIndex(int const index, bool const update = true, bool const peek = false);  

protected:
  void set(int const pin, bool const en);
};


#define DEFINE_CONTROL_PIN(P, setter, getter)		\
  void setter(bool const en) { Module::set(P, en); }	\
  bool getter() { return Module::inputByIndex(P); }

#endif // MODULE_H
