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

  BinaryCounter dataPointerRegister;
  BinaryCounter instructionPointerRegister;
  BinaryCounter stackPointerRegister; 
  BinaryCounter dataRegister;
  BinaryCounter loopRegister;
  Register instructionRegister;
  Register flagRegister;
  
  Decoder decoder;
  Screen scr;

  static constexpr size_t STACK_SIZE = 4;
  
  void connectModules();

public:
  Computer():
    dataPointerRegister(STACK_SIZE)
  {
    connectModules();
  }
  void doIt();

};

#endif
