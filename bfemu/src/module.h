#ifndef MODULE_H
#define MODULE_H
#include <vector>
#include <cassert>
#include <bitset>


class Module
{
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
  static Module *power;
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
  
  template <typename ... Indices>
  static constexpr unsigned long const mask(Indices ... indices) {
    assert((((int)indices < (int)N_PINS) && ...) && "invalid index");
    return ((1 << indices) | ...);
  };

  static void init();
  virtual int numberOfInputs() const = 0;
  virtual int numberOfOutputs() const = 0;

  virtual void update() {}
  virtual void reset() { setOutput(0); };
  virtual void onClockRising() {}
  virtual void onClockFalling() {}
  virtual void onClockHigh() {}
  virtual void onClockLow() {}

  virtual bool canBeClocked() const { return true; }

  void setOutputEnabled(bool const en);
  bool outputEnabled();

  unsigned long input(unsigned long mask = -1);
  unsigned long output(unsigned long mask = -1, bool const peek = false);
  unsigned long peek(unsigned long mask = -1);
  bool inputByIndex(int const index);
  bool outputByIndex(int const index, bool const update = true, bool const peek = false);  
  void connectInputTo(Module &other, unsigned long const outputMask, unsigned long const inputMask, bool const disconnectOtherConnections = false);
  void connectInputToIndex(Module &other, int const outputIndex, int const inputIndex, bool const disconnectOtherConnections = false);

protected:
  void set(int const pin, bool const en);
};


#define DEFINE_CONTROL_PIN(P, setter, getter)		\
  void setter(bool const en) { Module::set(P, en); }	\
  bool getter() { return Module::inputByIndex(P); }

#endif // MODULE_H
