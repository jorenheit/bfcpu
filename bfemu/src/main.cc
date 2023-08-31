#include <iostream>
#include <bitset>

#include "power.h"
#include "clock.h"
#include "register.h"
#include "alu.h"
#include "ram.h"
#include "rom.h"


class Bus: public Module
{
  
public:
};

int main(int argc, char **argv) 
{
  Module::init();
  
  Register regA;
  Register regB;
  ALU alu;
  RAM ram;
  ROM rom;

  unsigned char const romData[] = {
    42, 69, 112
  };

  rom.load(romData, sizeof(romData));
  
  Clock clc;
  clc.connectModule(regA);
  //  clc.connectToModule(regB);
  
  regA.setData(-1); // address
  alu.connectInputTo(regA, Register::DATA_OUT, ALU::DATA_IN);
  rom.connectInputTo(alu, ALU::DATA_OUT, ROM::ADDRESS_IN_LOW);
  rom.tieInputToLow(ROM::ADDRESS_IN_HIGH);


  regA.setOutputEnabled(true);
  std::cout << regA.output(Register::DATA_OUT) << ' ' << rom.output(ROM::DATA_OUT) << '\n';


  Bus bus;
  bus.connectModule(regA, Register::DATA_OUT);
  bus.connectModule(regB, Register::DATA_OUT);

  regA.setData(0b01010101);
  regB.setData(0b10101010);
  regA.setOutputEnabled(true);
  regB.setOutputEnabled(true);

  std::cout << std::bitset<32>(bus.output()) << '\n';
}
