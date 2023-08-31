#include <iostream>
#include "allmodules.h"
#include "computer.h"



int main()
{
#if 1
  Computer comp;
  comp.test();
  
#else
  Module::init();
  Clock clc;
  RAM ram1;
  RAM ram2;
  ROM rom;

  size_t const STACK_SIZE = 4;
  BinaryCounter dataPointerRegister(STACK_SIZE);
  BinaryCounter instructionPointerRegister;
  BinaryCounter stackPointerRegister; // only need low byte
  BinaryCounter dataRegister;
  Register flagRegister;

  Screen scr;
  Decoder decoder;
  
  clc.connectModule(ram1,
		    ram2,
		    dataRegister,
		    flagRegister,
		    stackPointerRegister,
		    dataPointerRegister,
		    instructionPointerRegister,
		    decoder);

  ram1.connectInputTo(dataPointerRegister, BinaryCounter::DATA_OUT, RAM::ADDRESS_IN);
  ram1.connectInputTo(stackPointerRegister, BinaryCounter::DATA_OUT, RAM::ADDRESS_IN);
  ram1.connectInputTo(dataRegister, BinaryCounter::DATA_OUT_LOW, RAM::DATA_IN); 
  ram1.connectInputTo(instructionPointerRegister, BinaryCounter::DATA_OUT_LOW, RAM::DATA_IN);
 
  ram2.connectInputTo(dataPointerRegister, BinaryCounter::DATA_OUT, RAM::ADDRESS_IN);
  ram2.connectInputTo(stackPointerRegister, BinaryCounter::DATA_OUT, RAM::ADDRESS_IN);
  ram2.connectInputTo(dataRegister, BinaryCounter::DATA_OUT_HIGH, RAM::DATA_IN); 
  ram2.connectInputTo(instructionPointerRegister, BinaryCounter::DATA_OUT_HIGH, RAM::DATA_IN);

  dataRegister.connectInputTo(ram1, RAM::DATA_OUT, BinaryCounter::DATA_IN_LOW);
  dataRegister.connectInputTo(ram2, RAM::DATA_OUT, BinaryCounter::DATA_IN_HIGH);
  
  
  
  rom.connectInputTo(instructionPointerRegister, BinaryCounter::DATA_OUT, ROM::ADDRESS_IN);

  instructionPointerRegister.connectInputTo(ram1, RAM::DATA_OUT, BinaryCounter::DATA_IN_LOW);
  instructionPointerRegister.connectInputTo(ram2, RAM::DATA_OUT, BinaryCounter::DATA_IN_HIGH);

  // Decoder <- Flags
  decoder.connectInputToIndex(flagRegister, Register::Q0, Decoder::I0); // Z
  decoder.connectInputToIndex(flagRegister, Register::Q1, Decoder::I1); // A
  decoder.connectInputToIndex(flagRegister, Register::Q2, Decoder::I2); // V
  // Decoder <- ROM
  decoder.connectInputToIndex(rom, ROM::Q0, Decoder::I3);
  decoder.connectInputToIndex(rom, ROM::Q1, Decoder::I4);
  decoder.connectInputToIndex(rom, ROM::Q2, Decoder::I5);
  decoder.connectInputToIndex(rom, ROM::Q3, Decoder::I6);
  
  // Flags
  flagRegister.connectInputToIndex(decoder, Decoder::F_EN, Register::EN);
  flagRegister.connectInputToIndex(decoder, Decoder::F_LD, Register::LD);
  flagRegister.connectInputToIndex(dataRegister, BinaryCounter::Z, Register::D0);
  flagRegister.connectInputToIndex(decoder, Decoder::AF_OUT, Register::D1);
  flagRegister.connectInputToIndex(decoder, Decoder::VF_OUT, Register::D2);

  // Screen
  scr.connectInputTo(dataRegister, BinaryCounter::DATA_OUT_LOW, Screen::DATA_IN);
  

  // Connect Decoder
  dataRegister.connectInputToIndex(decoder, Decoder::D_EN, BinaryCounter::EN);
  dataRegister.connectInputToIndex(decoder, Decoder::D_CNT, BinaryCounter::CNT);
  dataRegister.connectInputToIndex(decoder, Decoder::D_LD, BinaryCounter::LD);
  dataRegister.connectInputToIndex(decoder, Decoder::D_DEC, BinaryCounter::DEC);

  dataPointerRegister.connectInputToIndex(decoder, Decoder::DP_EN, BinaryCounter::EN);
  dataPointerRegister.connectInputToIndex(decoder, Decoder::DP_CNT, BinaryCounter::CNT);
  dataPointerRegister.connectInputToIndex(decoder, Decoder::DP_LD, BinaryCounter::LD);
  dataPointerRegister.connectInputToIndex(decoder, Decoder::DP_DEC, BinaryCounter::DEC);

  instructionPointerRegister.connectInputToIndex(decoder, Decoder::IP_EN, BinaryCounter::EN);
  instructionPointerRegister.connectInputToIndex(decoder, Decoder::IP_CNT, BinaryCounter::CNT);
  instructionPointerRegister.connectInputToIndex(decoder, Decoder::IP_LD, BinaryCounter::LD);
  instructionPointerRegister.connectInputToIndex(decoder, Decoder::IP_DEC, BinaryCounter::DEC);

  stackPointerRegister.connectInputToIndex(decoder, Decoder::SP_EN, BinaryCounter::EN);
  stackPointerRegister.connectInputToIndex(decoder, Decoder::SP_CNT, BinaryCounter::CNT);
  stackPointerRegister.connectInputToIndex(decoder, Decoder::SP_LD, BinaryCounter::LD);
  stackPointerRegister.connectInputToIndex(decoder, Decoder::SP_DEC, BinaryCounter::DEC);

  ram1.connectInputToIndex(decoder, Decoder::RAM_EN, RAM::EN);
  ram2.connectInputToIndex(decoder, Decoder::RAM_EN, RAM::EN);
  ram1.connectInputToIndex(decoder, Decoder::RAM_WE, RAM::WE);
  ram2.connectInputToIndex(decoder, Decoder::RAM_WE, RAM::WE);

  

  // Starting state
  unsigned char memory1[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 12, 'b', 'c', 'd'};
  unsigned char memory2[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  
  ram1.load(memory1);
  ram2.load(memory2);
  decoder.setOutputEnabled(true);
  instructionPointerRegister.setOutputEnabled(true);
  flagRegister.setOutputEnabled(true);

  dataRegister.setData(0);
  instructionPointerRegister.setData(0);
  flagRegister.setData(0);

  
  // Run
  unsigned char program[] = {Decoder::PLUS, Decoder::PLUS, Decoder::PLUS, Decoder::PLUS, Decoder::PLUS, Decoder::PLUS};
  rom.load(program);

  for (int i = 0; i != 10; ++i){
    clc.pulse();
    std::cout << dataRegister.peek(BinaryCounter::DATA_OUT) << '\n';
  }
#endif      
  
}
