#include <iomanip>
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
	      decoder,
	      scr);

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

  connectModulesByIndex( decoder, Decoder::SCR_LD, scr, Screen::LD);
  
  // PERMANENT CONNECTIONS
  decoder.setOutputEnabled(true);
  instructionRegister.setOutputEnabled(true);
}


void Computer::show(std::ostream &out)
{
  int const pointerPosition = dataPointerRegister.peek();
  int const window = 5;
  int const first = (pointerPosition > window) ? (pointerPosition - window) : 0;
  int const last = first + 2 * window + 1;

  out << "\n=========================================================\n";
  // Memory (around datapointer):
  out << "Memory: |";
  for (int i = first; i != last; ++i) {
    out << ' ' << std::setw(2) << std::setfill('0') << std::hex << (int)ram1.at(i) << " |";
  }
  out << "\n        |";
  for (int i = first; i != last; ++i) {
    out << ' ' << std::setw(2) << std::setfill('0') << std::hex << (int)ram2.at(i) << " |";
  }
  out << '\n' << std::string(11 + (pointerPosition - first) * 5, ' ') << "^\n";

  // Instruction Pointer:
  int const ip = instructionPointerRegister.peek(Register<16>::DATA_OUT);
  int const cmd = (ip < (1 << 16)) ? rom.at(ip) : 0;
  //  int const cmd = rom.at(ip);
  out << "IP   : " << std::dec << ip << " (" << ("0+-<>.,[]SH"[cmd]) << ")\n";
  // Data Pointer
  out << "DP   : " << std::dec << pointerPosition << '\n';
  // D-Reg
  out << "D-REG: " << std::setw(2) << std::setfill('0') << std::hex << (int)dataRegister.peek(Register<8>::DATA_OUT) << '\n';
  // L-Reg
  out << "L-REG: " << std::setw(4) << std::setfill('0') << std::hex << (int)loopRegister.peek(Register<16>::DATA_OUT) << '\n';
  // SP
  int const sp = stackPointerRegister.peek(Register<8>::DATA_OUT);
  int const addr = ram1.at(sp) + (ram2.at(sp) << 8);
  out << "SP   : " << std::setw(4) << std::setfill('0') << std::hex << sp  << " (" << std::dec << addr << ")\n"; 

  // I-Reg
  int const instr = instructionRegister.peek(Register<8>::DATA_OUT);
  out << "I-REG: " << std::bitset<4>(instr) << " | " << std::bitset<4>(instr >> 4);
  out << "\n=========================================================\n";
  
  
}

void Computer::load(std::string const &prog)
{
  std::vector<unsigned char> program;
  for (char c: prog) {
    switch (c) {
    case '+': program.push_back(Decoder::PLUS); break;
    case '-': program.push_back(Decoder::MINUS); break;
    case '<': program.push_back(Decoder::LEFT); break;
    case '>': program.push_back(Decoder::RIGHT); break;
    case '[': program.push_back(Decoder::LOOP_START); break;
    case ']': program.push_back(Decoder::LOOP_END); break;
    case '.': program.push_back(Decoder::OUT); break;
      //case ',': program.push_back(Decoder::LOOP_END); break;
    default: continue;
    }
  }
  
  program.push_back(Decoder::HLT);
  rom.load(program.data(), program.size());
}

void Computer::run()
{
  while (!clc.haltEnabled()) {
    //    std::cin.get();
    clc.pulse();
    //    show();
  }
}

void Computer::reset()
{
  dataPointerRegister.reset();
  dataRegister.reset();
  instructionRegister.reset();
  instructionPointerRegister.reset();
  loopRegister.reset();
  flagRegister.reset();
  stackPointerRegister.reset();
  decoder.reset();
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
    Decoder::RIGHT,
    Decoder::HLT
  };
  rom.load(program);

    
  while (!clc.haltEnabled()) {
    std::cin.get();
    clc.pulse();
    show();
  }

}
