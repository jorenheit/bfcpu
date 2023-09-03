#include <iostream>
#include <bitset>
#include <vector>
#include <cassert>


class Decoder
{
  static constexpr int INSTRUCTION_BITS = 8;
  static constexpr int LOOKUP_TABLE_SIZE = (1 << INSTRUCTION_BITS);

  unsigned long table[LOOKUP_TABLE_SIZE];
  
public:

  void addInstruction(std::string const &bitPattern, unsigned long instruction)
  {
    assert(bitPattern.length() <= INSTRUCTION_BITS);
    for (char c: bitPattern)
      assert(c == '0' || c == '1' || c == 'x');
    
    // pattern = e.g. 001100xxx
    // where x can be any bit

    for (int i = 0; i != LOOKUP_TABLE_SIZE; ++i) {
      if (isMatch(bitPattern, i))
	table[i] = instruction;
    }
    
  }

  bool isMatch(std::string bitPattern, int value)
  {
    if (bitPattern.size() < INSTRUCTION_BITS) {
      int const diff = INSTRUCTION_BITS - bitPattern.size();
      bitPattern = std::string(diff, '0') + bitPattern;
    }
    std::string const val = std::bitset<INSTRUCTION_BITS>(value).to_string();

    for (int i = 0; i != INSTRUCTION_BITS; ++i) {
      if (bitPattern[i] != 'x' and bitPattern[i] != val[i])
	return false;
    }
    return true;
  }


  unsigned long get(int index) const {
    return table[index];
  }
};



int main(int argc, char **argv)
{
  Decoder dec;
  dec.addInstruction("111111xx", 42);

  for (int i = 0; i != 256; ++i)
    std::cout << i << ": " << dec.get(i) << '\n';
}
