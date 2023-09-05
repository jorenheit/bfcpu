#ifndef COMPUTER_H
#define COMPUTER_H
#include "allmodules.h"

#if 0
#include <gtest/gtest.h>
class Computer: public ::testing::Test
#else
class Computer
#endif
{
protected:
  struct ModuleInitializer {
    ModuleInitializer() {
      Module::init();
    }
  } init;
  
  Clock clc;
  RAM ram1;
  RAM ram2;
  ROM rom;

  Register<16> dataPointerRegister;
  Register<16> instructionPointerRegister;
  Register<16> loopRegister;
  Register<16>  stackPointerRegister; 
  Register<8>  dataRegister;
  Register<8>  instructionRegister;
  Register<4>  flagRegister;
  
  Decoder decoder;
  Screen scr;

  static constexpr size_t STACK_SIZE = 4;
  
  void build();

public:
  Computer():
    dataPointerRegister(STACK_SIZE)
  {
    build();
  }
  void doIt();

};

#endif
