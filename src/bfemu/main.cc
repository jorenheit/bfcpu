#include <iostream>
#include "computer.h"

int main(int argc, char **argv)
{
  std::string prog = argv[1];
  Computer comp;
  comp.load(prog);
  comp.run();
}
