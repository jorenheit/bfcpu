#ifndef COMPUTER_H
#define COMPUTER_H
#include <gtest/gtest.h>
#include "allmodules.h"

class Computer: public ::testing::Test
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
  Register flagRegister;
  Decoder decoder;
  Screen scr;

  static constexpr size_t STACK_SIZE = 4;
  
  void connectModules();
  void test();

public:
  Computer():
    dataPointerRegister(STACK_SIZE)
  {
    connectModules();
  }

};

#endif
