#ifndef DECODER_H
#define DECODER_H
#include <map>
#include <bitset> // debug
#include "module.h"

class Decoder: public Module
{
  static constexpr int INSTRUCTION_BITS = 11;
  static constexpr int LOOKUP_TABLE_SIZE = 1 << INSTRUCTION_BITS;
  unsigned long d_lookupTable[LOOKUP_TABLE_SIZE];
  int d_cycle = 0;
  
public:
  enum Instructions {
    NOOP	= 0x00,
    PLUS	= 0x01,
    MINUS	= 0x02,
    LEFT	= 0x03,
    RIGHT	= 0x04,
    IN		= 0x05,
    OUT		= 0x06,
    LOOP_START	= 0x07,
    LOOP_END	= 0x08,
    PROG_START	= 0x0e,
    HLT         = 0x0f
  };
  
  enum Input {
    I0, I1, I2, I3, Z_IN, L_IN, A_IN, V_IN,
    EN,
    N_INPUT,

    INS_IN = mask(I0, I1, I2, I3),
    FLAGS_IN = mask(Z_IN, L_IN, A_IN, V_IN),
    DATA_IN = INS_IN | FLAGS_IN,
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

    I_EN,    // Instruction register
    I_LD,

    F_EN,    // Flag Register
    F_LD,
    
    L_EN,    // loopcount register
    L_CNT,
    L_DEC,

    SCR_LD,  // screen
    
    AF_OUT,  // pin to set the address-changed (A) flag with
    VF_OUT,  // pin to set the value-changed (V) flag with

    HLT_EN, // pin connected to halt of clock
    
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
  {
    for (auto &val: d_lookupTable)
      val = -1;
    setupDecoderTable();
  }    
  
  virtual void onClockRising() override
  {
    unsigned char const instruction = input(DATA_IN);
    unsigned long const config = d_lookupTable[(instruction << 3) | d_cycle];

    ASSERT(config != (unsigned long)-1,
	   "Invalid index to the lookup table.\nInstruction = ", std::bitset<8>(instruction),
	   "\nCycle = ", d_cycle);
    
    setOutput(config);
    d_cycle = (config & mask(IP_CNT)) ? 0 : d_cycle + 1;
  }

  virtual void reset() override
  {
    d_cycle = 0;
    setOutput(0);
  }

private:
  void setupDecoderTable();
  void addInstruction(std::string const &pattern, unsigned long instruction);
  
};


#endif //DECODER_H
