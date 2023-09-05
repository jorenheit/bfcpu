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

  enum Flags {
    Z = (1 << 0),
    L = (1 << 1),
    A = (1 << 2),
    V = (1 << 3)
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
    
    /*
      Instruction: VALZ IIII CCC
      C = cycle bits
      I = instruction bit
      Z = zero-flag
      L = loop-flag
      A = Address change flag
      V = Value change flag
     */
    
    // Cycle 0: always fetch new instruction from ROM.
    addInstruction("xxxx xxxx 000", mask(F_EN, IP_EN, I_LD)); 

    // PROG_START
    addInstruction("xxxx 1110 001", mask(D_LD, RAM_EN, DP_EN, IP_CNT));

    // HLT
    addInstruction("xxxx 1111 001", mask(HLT_EN));
    
    // PLUS
    addInstruction("x01x 0001 001", mask(D_CNT, VF_OUT, F_LD, IP_CNT));		// A-FLAG NOT SET
    addInstruction("x11x 0001 001", mask(DP_EN, D_LD, RAM_EN));			// A-FLAG SET, CYCLE 1
    addInstruction("x11x 0001 010", mask(D_CNT, VF_OUT, F_LD, IP_CNT));		// A-FLAG SET, CYCLE 2
    addInstruction("xx0x 0001 001", mask(IP_CNT));				// L-FLAG LOW -> SKIP

    // MINUS
    addInstruction("x01x 0010 001", mask(D_CNT, D_DEC, VF_OUT, F_LD, IP_CNT));	// A-FLAG NOT SET
    addInstruction("x11x 0010 001", mask(DP_EN, D_LD, RAM_EN));			// A-FLAG SET, CYCLE 1
    addInstruction("x11x 0010 010", mask(D_CNT, D_DEC, VF_OUT, F_LD, IP_CNT));	// A-FLAG SET, CYCLE 2
    addInstruction("xx0x 0010 001", mask(IP_CNT));				// LOOP SKIP

    // LEFT
    addInstruction("0x1x 0011 001", mask(DP_CNT, DP_DEC, AF_OUT, F_LD, IP_CNT)); // V-FLAG NOT SET
    addInstruction("1x1x 0011 001", mask(D_EN, DP_EN, RAM_WE));                  // V-FLAG SET, CYCLE 1
    addInstruction("0x1x 0011 010", mask(DP_CNT, DP_DEC, AF_OUT, F_LD, IP_CNT)); // V-FLAG SET, CYCLE 2
    addInstruction("xx0x 0011 001", mask(IP_CNT));                               // LOOP SKIP

    // RIGHT
    addInstruction("0x1x 0100 001", mask(DP_CNT, AF_OUT, F_LD, IP_CNT));         // V-FLAG NOT SET
    addInstruction("1x1x 0100 001", mask(D_EN, DP_EN, RAM_WE));                  // V-FLAG SET, CYCLE 1
    addInstruction("1x1x 0100 010", mask(DP_CNT, AF_OUT, F_LD, IP_CNT));         // V-FLAG SET, CYCLE 2
    addInstruction("xx0x 0100 001", mask(IP_CNT));                               // LOOP SKIP
    
    // LOOP START
    // Scenario 1: cell is already loaded and 0
    addInstruction("x011 0111 001", mask(L_CNT, IP_CNT));                        // CYCLE 1: increment loop register (skip loop)

    // Scenario 2: cell is already loaded and nonzero
    addInstruction("x010 0111 001", mask(SP_CNT));                               // CYCLE 1: increment SP
    addInstruction("x010 0111 010", mask(RAM_WE, SP_EN, IP_EN));                 // CYCLE 2: write current IP to stack
    addInstruction("x010 0111 011", mask(IP_CNT));                               // CYCLE 3: next instruction

    // Scenario 3: cell is not yet loaded (A-flag)
    addInstruction("x11x 0111 001", mask(DP_EN, RAM_EN, D_LD));                  // CYCLE 1:  load from ram
    addInstruction("x111 0111 010", mask(L_CNT, IP_CNT));                        // CYCLE 2A: Data was just loaded and zero -> skip loop -> DONE
    addInstruction("x110 0111 010", mask(SP_CNT));                               // CYCLE 2B: Data was just loaded and nonzero -> increment SP
    addInstruction("x110 0111 011", mask(RAM_WE, SP_EN, IP_EN));                 // CYCLE 3:  write current IP to stack
    addInstruction("x110 0111 100", mask(IP_CNT));                               // CYCLE 4:  next instruction

    // On skip
    addInstruction("xx0x 0111 001", mask(L_CNT, IP_CNT));                        // Increment loop count

    // LOOP END
    // Scenario 1: cell is already loaded and 0
    addInstruction("x011 1000 001", mask(SP_CNT, SP_DEC, IP_CNT));               // CYCLE 1: exit loop -> decrement SP and go to next instruction

    // Scenario 2: cell is already loaded and nonzero
    addInstruction("x010 1000 001", mask(SP_EN, IP_LD, RAM_EN));                 // CYCLE 1: load IP from stack
    addInstruction("x010 1000 010", mask(IP_CNT));                               // CYCLE 2: next instruction (first of the loop)

    // Scenario 3: cell is not yet loaded (A-flag)
    addInstruction("x11x 1000 001", mask(DP_EN, RAM_EN, D_LD));                  // CYCLE 1:  load from ram
    addInstruction("x111 1000 010", mask(SP_CNT, SP_DEC, IP_CNT));               // CYCLE 2A: data was just loaded and zero -> exit loop
    addInstruction("x110 1000 010", mask(SP_EN, IP_LD, RAM_EN));                 // CYCLE 2B: data was just loaded and nonzero -> loop back
    addInstruction("x110 1000 011", mask(IP_CNT));                               // CYCLE 3:  next instruction

    // On skip
    addInstruction("xx0x 1000 001", mask(L_CNT, L_DEC, IP_CNT));
  }


  void addInstruction(std::string const &pattern, unsigned long instruction)
  {
    // pattern = e.g. 001100xxx, where x can be any bit
    std::string bitPattern;
    for (char c: pattern)
      if (c == '0' || c == '1' || c == 'x')
	bitPattern += c;
    assert(bitPattern.length() <= INSTRUCTION_BITS);

    auto isMatch = [&](std::string pattern, int value) -> bool {
      if (pattern.size() < INSTRUCTION_BITS) {
	int const diff = INSTRUCTION_BITS - bitPattern.size();
	bitPattern = std::string(diff, '0') + bitPattern;
      }
      std::string const val = std::bitset<INSTRUCTION_BITS>(value).to_string();

      for (int i = 0; i != INSTRUCTION_BITS; ++i) {
	if (bitPattern[i] != 'x' and bitPattern[i] != val[i])
	  return false;
      }
      return true;
    };

    for (int i = 0; i != LOOKUP_TABLE_SIZE; ++i) {
      if (isMatch(bitPattern, i))
	d_lookupTable[i] = instruction;
    }
  }
  
  
  unsigned long decode(unsigned char const command, unsigned char const flags, int const cycle)
  {
    int index = cycle | (command << 3) | (flags << 7);
    unsigned long result = d_lookupTable[index];
    return result;
  }
    
  
  virtual void onClockRising() override
  {
    static int cycle = 0;
    unsigned char const instruction = input(DATA_IN);
    unsigned long const config = d_lookupTable[(instruction << 3) | cycle];
    assert(config != (unsigned long)-1 && "Invalid index to the lookup table -> no instruction found");
    setOutput(config);
    cycle = (config & mask(IP_CNT)) ? 0 : cycle + 1;
  }
};


#endif //DECODER_H
