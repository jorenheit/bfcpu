#pragma once

class Random {
  uint32_t _value;

public:
  inline void begin(uint32_t seed) {
    _value = seed;
     while (get() == 0) {}
  }

  inline byte get() {
    _value ^= _value << 13;
    _value ^= _value >> 17;
    _value ^= _value << 5;
    return _value & 0xFF;
  }
};