#ifndef DECODER_H
#define DECODER_H
#include <map>
#include <bitset> // debug
#include "module.h"

class Decoder: public Module
{
  int d_cycleCount = 0;
  
public:
  enum Instructions {
    NOOP = 0,
    PLUS = 1,
    MINUS = 2,
    LEFT = 3,
    RIGHT = 4,
    IN = 5,
    OUT = 6,
    LOOP_START = 7,
    LOOP_END = 8
  };

  enum Flags {
    Z = (1 << 0),
    A = (1 << 1),
    V = (1 << 2)
  };
  
  enum Input {
    I0, I1, I2, I3, I4, I5, I6, I7, // instruction bits
    EN,
    N_INPUT,

    DATA_IN = Module::mask(I0, I1, I2, I3, I4, I5, I6, I7)
  };

  enum Output {
    D_EN,    // data register
    D_CNT,
    D_LD,
    D_DEC,

    DP_EN,   // datapointer register
    DP_CNT,
    DP_LD,
    DP_DEC,

    IP_EN,   // instruction pointer register
    IP_CNT,
    IP_LD,
    IP_DEC,

    SP_EN,   // stack pointer register
    SP_CNT,
    SP_LD,
    SP_DEC,

    RAM_EN,  // ram 
    RAM_WE,

    I_EN,    // instruction register
    I_LD,

    F_EN,    // flag register
    F_LD,

    SS_EN,   // stack size fixed output

    AF_OUT,  // pin to set the address-changed (A) flag with
    VF_OUT,  // pin to set the value-changed (V) flag with

    N_OUTPUT
  };
  
  virtual int numberOfInputs() const override {
    return N_INPUT;
  }

  virtual int numberOfOutputs() const override {
    return N_OUTPUT;
  }

  Decoder():
    Module(EN)
  {}
   
  unsigned long decode(unsigned char const command, unsigned char const flags, int const cycle)
  {
    switch (command){
    case PLUS: {
      if (cycle == 0) {
	return (flags & A) ?
	  mask(DP_EN, D_LD, RAM_EN) : // address changed -> Enable RAM and datapointer, then load into D
	  mask(D_EN, D_CNT, IP_CNT, F_LD, VF_OUT);  // value already in register -> Increment D and IP and set value-change flag
      }
      else if (cycle == 1) {
	assert((flags & A) && "Unreachable");
	return mask(D_EN, D_CNT, F_LD, IP_CNT, VF_OUT); // value now in register -> increment (see above). Also, store 0 back in A flag by enabling F_LD without setting A_OUT
      }
      else assert(false && "Unreachable");
    }
      
    case MINUS: {
      if (cycle == 0) {
	return (flags & A) ?
	  mask(DP_EN, D_LD, RAM_EN) :
	  mask(D_EN, D_CNT, D_DEC, IP_CNT, F_LD, VF_OUT);
      }
      else if (cycle == 1) {
	assert((flags & A) && "Unreachable");
	return mask(D_EN, D_CNT, D_DEC, F_LD, IP_CNT, VF_OUT);
      }
      else assert(false && "Unreachable");
    }
    case RIGHT: {
      if (cycle == 0) {
	return (flags & V) ?
	  mask(D_EN, DP_EN, RAM_WE) :  // value changed -> write contents of D back to ram before incrementing DP
	  mask(DP_CNT, IP_CNT, F_LD, AF_OUT); // value not changed -> increment DP and IP and set flag
      }
      else if (cycle == 1) {
	assert((flags & V) && "Unreachable");
	return mask(DP_CNT, IP_CNT, F_LD, AF_OUT);
      }
      else assert(false && "Unreachable");
    }
    case LEFT: {
      if (cycle == 0) {
	return (flags & V) ?
	  mask(D_EN, DP_EN, RAM_WE) :  // value changed -> write contents of D back to ram before incrementing DP
	  mask(DP_CNT, DP_DEC, IP_CNT, F_LD, AF_OUT); // value not changed -> increment DP and IP and set flag
      }
      else if (cycle == 1) {
	assert((flags & V) && "Unreachable");
	return mask(DP_CNT, DP_DEC, IP_CNT, F_LD, AF_OUT);
      }
      else assert(false && "Unreachable");
    }
    default: {
      assert(false && "Not implemented");
    }}
  }
    
  
  virtual void onClockRising() override
  {
    static int cycle = 0;
    unsigned long const ins = input(DATA_IN);
    unsigned char const cmd = (ins & 0b1111000) >> 3;
    unsigned char const flg = (ins & 0b0000111);

    
    unsigned long const config = decode(cmd, flg, cycle);
#if 0
    std::cerr << "Cycle: "  << cycle << '\n'
	      << "Command: " << std::bitset<4>(cmd) << '\n'
	      << "Flags: " << std::bitset<3>(flg) << '\n'
	      << "Config: " << std::bitset<N_OUTPUT>(config) <<'\n';
#endif    
    setOutput(config);
    cycle = (config & mask(IP_CNT)) ? 0 : cycle + 1;
  }
};


#endif //DECODER_H
