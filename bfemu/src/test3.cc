#include <iostream>
#include <bitset>
#include <cassert>

template <typename ... Indices>
static constexpr unsigned long const mask(Indices ... indices) {
  assert(((indices < 64) && ...) && "invalid index");
  return ((1 << indices) | ...);
};


enum Input: unsigned long {
  A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, // address lines (16-bit)

  N_INPUT,
  ADDRESS_IN_LOW  = mask(A0, A1, A2, A3, A4, A5, A6, A7),
  ADDRESS_IN_HIGH = mask(A8, A9, A10, A11, A12, A13, A14, A15),
  ADDRESS_IN_FULL = ADDRESS_IN_LOW | ADDRESS_IN_HIGH
};


int main()
{
  std::cout << std::bitset<16>(ADDRESS_IN_LOW) << '\n';
}
