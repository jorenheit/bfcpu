#include <iostream>
#include <functional>
#include <vector>
#include <cassert>

#include "clock.h"
#include "register.h"




int main(int argc, char **argv)
{
  Register regA;
  regA.setOutputEnabled(true);
  regA.setData(0b10101010);
  
  Register regB;
  regB.setOutputEnabled(true);
  regB.setData(0b01010101);


  Clock clc;
  clc.connectToModule(regA);
  clc.connectToModule(regB);
  
  for (int i = 0; i != 8; ++i)
    regA.connectInputTo(regB, i, i);
  regA.setLoadEnabled(true);

  std::cout << regA.output(0xff) << " " << regB.output(0xff) << '\n';
  clc.pulse();
  std::cout << regA.output(0xff) << " " << regB.output(0xff) << '\n';
  
}
