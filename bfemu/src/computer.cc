#include "computer.h"

void Computer::build()
{
  clc.connect(ram1,
	      ram2,
	      dataRegister,
	      instructionRegister,
	      flagRegister,
	      loopRegister,
	      stackPointerRegister,
	      dataPointerRegister,
	      instructionPointerRegister,
	      decoder);

   // Connections to RAM
  connectModules( dataPointerRegister, Register<16>::DATA_OUT, ram1, RAM::ADDRESS_IN );
  connectModules( stackPointerRegister, Register<16>::DATA_OUT, ram1, RAM::ADDRESS_IN );
  connectModules( dataRegister, Register<8>::DATA_OUT, ram1, RAM::DATA_IN );
  connectModules( instructionPointerRegister, Register<16>::DATA_OUT_LOW, ram1, RAM::DATA_IN );

  connectModules( stackPointerRegister, Register<16>::DATA_OUT, ram2, RAM::ADDRESS_IN );
  connectModules( instructionPointerRegister, Register<16>::DATA_OUT_HIGH, ram2, RAM::DATA_IN );

  // Connections to ROM (PROGMEM)
  connectModules( instructionPointerRegister, Register<16>::DATA_OUT, rom, ROM::ADDRESS_IN);
  
  // Connections to D-REG
  connectModules( ram1, RAM::DATA_OUT, dataRegister, Register<8>::DATA_IN );

  // Connections to F-REG
  connectModulesByIndex( decoder, Decoder::AF_OUT, flagRegister, Register<4>::D0 );
  connectModulesByIndex( decoder, Decoder::VF_OUT, flagRegister, Register<4>::D1 );
  
  // Connections to I-REG
  // command-bits
  connectModulesByIndex( rom, ROM::Q0, instructionRegister, Register<8>::D0 ); 
  connectModulesByIndex( rom, ROM::Q1, instructionRegister, Register<8>::D1 ); 
  connectModulesByIndex( rom, ROM::Q2, instructionRegister, Register<8>::D2 ); 
  connectModulesByIndex( rom, ROM::Q3, instructionRegister, Register<8>::D3 ); 
  
  // state-bits (flags)
  connectModulesByIndex( dataRegister, Register<8>::Z, instructionRegister, Register<8>::D4 ); 
  connectModulesByIndex( loopRegister, Register<16>::Z, instructionRegister, Register<8>::D5 ); 
  connectModulesByIndex( flagRegister, Register<4>::Q0, instructionRegister, Register<8>::D6 ); 
  connectModulesByIndex( flagRegister, Register<4>::Q1, instructionRegister, Register<8>::D7 ); 

  // Connections to IP-REG
  connectModules( ram1, RAM::DATA_OUT, instructionPointerRegister, Register<16>::DATA_IN_LOW );
  connectModules( ram2, RAM::DATA_OUT, instructionPointerRegister, Register<16>::DATA_IN_HIGH );
  
  // L-REG
  // No incoming connections into loop register
  
  // DECODER
  connectModules( instructionRegister, Register<8>::DATA_OUT, decoder, Decoder::DATA_IN );
  
  // SCREEN
  connectModules( dataRegister, Register<8>::DATA_OUT, scr, Screen::DATA_IN);

  // CONTROL SIGNALS
  connectModulesByIndex( decoder, Decoder::HLT_EN, clc, Clock::HLT );

  connectModulesByIndex( decoder, Decoder::D_EN, dataRegister, Register<8>::EN );
  connectModulesByIndex( decoder, Decoder::D_CNT, dataRegister, Register<8>::CNT );
  connectModulesByIndex( decoder, Decoder::D_LD, dataRegister, Register<8>::LD );
  connectModulesByIndex( decoder, Decoder::D_DEC, dataRegister, Register<8>::DEC );

  connectModulesByIndex( decoder, Decoder::DP_EN, dataPointerRegister, Register<16>::EN );
  connectModulesByIndex( decoder, Decoder::DP_CNT, dataPointerRegister, Register<16>::CNT );
  connectModulesByIndex( decoder, Decoder::DP_LD, dataPointerRegister, Register<16>::LD );
  connectModulesByIndex( decoder, Decoder::DP_DEC, dataPointerRegister, Register<16>::DEC );

  connectModulesByIndex( decoder, Decoder::IP_EN, instructionPointerRegister, Register<16>::EN);
  connectModulesByIndex( decoder, Decoder::IP_CNT, instructionPointerRegister, Register<16>::CNT);
  connectModulesByIndex( decoder, Decoder::IP_LD, instructionPointerRegister, Register<16>::LD);
  connectModulesByIndex( decoder, Decoder::IP_DEC, instructionPointerRegister, Register<16>::DEC);

  connectModulesByIndex( decoder, Decoder::SP_EN, stackPointerRegister, Register<16>::EN);
  connectModulesByIndex( decoder, Decoder::SP_CNT, stackPointerRegister, Register<16>::CNT);
  connectModulesByIndex( decoder, Decoder::SP_LD, stackPointerRegister, Register<16>::LD);
  connectModulesByIndex( decoder, Decoder::SP_DEC, stackPointerRegister, Register<16>::DEC);

  connectModulesByIndex( decoder, Decoder::F_EN, flagRegister, Register<4>::EN);
  connectModulesByIndex( decoder, Decoder::F_LD, flagRegister, Register<4>::LD);

  connectModulesByIndex( decoder, Decoder::I_EN, instructionRegister, Register<8>::EN);
  connectModulesByIndex( decoder, Decoder::I_LD, instructionRegister, Register<8>::LD);

  connectModulesByIndex( decoder, Decoder::L_EN, loopRegister, Register<16>::EN);
  connectModulesByIndex( decoder, Decoder::L_CNT, loopRegister, Register<16>::CNT);
  connectModulesByIndex( decoder, Decoder::L_DEC, loopRegister, Register<16>::DEC);

  connectModulesByIndex( decoder, Decoder::RAM_EN, ram1, RAM::EN);
  connectModulesByIndex( decoder, Decoder::RAM_EN, ram2, RAM::EN);
  connectModulesByIndex( decoder, Decoder::RAM_WE, ram1, RAM::WE);
  connectModulesByIndex( decoder, Decoder::RAM_WE, ram2, RAM::WE);

  // PERMANENT CONNECTIONS
  decoder.setOutputEnabled(true);
  instructionRegister.setOutputEnabled(true);
}


void Computer::doIt()
{
  // Starting state
  unsigned char memory1[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  unsigned char memory2[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  
  ram1.load(memory1);
  ram2.load(memory2);

  dataRegister.setInternalState(0);
  instructionPointerRegister.setInternalState(0);
  
  // Run
  unsigned char program[] = {
    Decoder::PLUS,
    Decoder::PLUS,
    Decoder::LOOP_START,
    Decoder::MINUS,
    Decoder::LOOP_END,
    Decoder::HLT
  };
  rom.load(program);

    
  while (!clc.haltEnabled()) {
    std::cin.get();
    clc.pulse();
    std::cout << dataRegister.peek(Register<8>::DATA_OUT) << '\n';
    std::cout << loopRegister.peek(Register<8>::DATA_OUT) << '\n';
    std::cout << (int)ram1.at(4) << ' ' << (int)ram1.at(5) << ' ' << (int)ram1.at(6) << '\n';
  }

}
